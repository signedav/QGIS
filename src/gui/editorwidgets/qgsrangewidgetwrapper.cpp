/***************************************************************************
    qgsrangewidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrangewidgetwrapper.h"

#include "qgsapplication.h"
#include "qgsdial.h"
#include "qgsdoublespinbox.h"
#include "qgsslider.h"
#include "qgsspinbox.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QString>

#include "moc_qgsrangewidgetwrapper.cpp"

using namespace Qt::StringLiterals;

QgsRangeWidgetWrapper::QgsRangeWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{}

int QgsRangeWidgetWrapper::defaultFieldPrecision( const QgsField &field )
{
  constexpr int DEFAULT_PRECISION_DOUBLE = 4;
  const int fieldPrecision = field.precision();
  switch ( field.type() )
  {
    case QMetaType::Type::Float:
    case QMetaType::Type::Double:
      return fieldPrecision > 0 ? fieldPrecision : DEFAULT_PRECISION_DOUBLE;

    // we use the double spin box for long long fields in order to get sufficient range of min/max values
    case QMetaType::Type::LongLong:
      return 0;

    default:
      break;
  }

  return fieldPrecision;
}

QWidget *QgsRangeWidgetWrapper::createWidget( QWidget *parent )
{
  QWidget *editor = nullptr;

  if ( config( u"Style"_s ).toString() == "Dial"_L1 )
  {
    editor = new QgsTolerantDial( parent );
  }
  else if ( config( u"Style"_s ).toString() == "Slider"_L1 )
  {
    editor = new QgsTolerantSlider( Qt::Horizontal, parent );
  }
  else
  {
    switch ( layer()->fields().at( fieldIdx() ).type() )
    {
      case QMetaType::Type::Double:
      // for long long field types we have to use a double spin box with 0 decimal places,
      // as the min/max value supported by QSpinBox is not large enough
      case QMetaType::Type::LongLong:
      {
        editor = new QgsTolerantDoubleSpinBox( parent );
        static_cast<QgsTolerantDoubleSpinBox *>( editor )->setLineEditAlignment( Qt::AlignRight );
        break;
      }

      case QMetaType::Type::Int:
      default:
        editor = new QgsTolerantSpinBox( parent );
        static_cast<QgsTolerantSpinBox *>( editor )->setLineEditAlignment( Qt::AlignRight );
        break;
    }
  }

  return editor;
}

template<class T> static void setupIntEditor( const QVariant &min, const QVariant &max, const QVariant &step, T *slider, QgsRangeWidgetWrapper *wrapper )
{
  // must use a template function because those methods are overloaded and not inherited by some classes
  slider->setMinimum( min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest() );
  slider->setMaximum( max.isValid() ? max.toInt() : std::numeric_limits<int>::max() );
  slider->setSingleStep( step.isValid() ? step.toInt() : 1 );
  QObject::connect( slider, qOverload<int>( &T::valueChanged ), wrapper, &QgsRangeWidgetWrapper::emitValueChanged );
}

template<class T> static void setupVariantEditor( const QVariant &min, const QVariant &max, const QVariant &step, T *slider, QgsRangeWidgetWrapper *wrapper )
{
  // must use a template function because those methods are overloaded and not inherited by some classes
  slider->setMinimum( min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest() );
  slider->setMaximum( max.isValid() ? max.toInt() : std::numeric_limits<int>::max() );
  slider->setSingleStep( step.isValid() ? step.toInt() : 1 );
  QObject::connect( slider, qOverload<const QVariant &>( &T::valueChanged ), wrapper, &QgsRangeWidgetWrapper::emitValueChanged );
}

void QgsRangeWidgetWrapper::initWidget( QWidget *editor )
{
  mDoubleSpinBox = qobject_cast<QDoubleSpinBox *>( editor );
  mIntSpinBox = qobject_cast<QSpinBox *>( editor );
  mQgsTolerantIntSpinBox = qobject_cast<QgsTolerantSpinBox *>( editor );
  mQgsTolerantDoubleSpinBox = qobject_cast<QgsTolerantDoubleSpinBox *>( editor );

  mDial = qobject_cast<QDial *>( editor );
  mSlider = qobject_cast<QSlider *>( editor );
  mQgsDial = qobject_cast<QgsDial *>( editor );
  mQgsSlider = qobject_cast<QgsSlider *>( editor );
  mQgsTolerantDial = qobject_cast<QgsTolerantDial *>( editor );
  mQgsTolerantSlider = qobject_cast<QgsTolerantSlider *>( editor );

  const bool allowNull = config( u"AllowNull"_s, true ).toBool();

  QVariant min( config( u"Min"_s ) );
  QVariant max( config( u"Max"_s ) );
  QVariant step( config( u"Step"_s ) );
  const QVariant precision( config( u"Precision"_s ) );

  if ( mQgsTolerantDoubleSpinBox )
  {
    const double stepval = step.isValid() ? step.toDouble() : 1.0;
    double minval = min.isValid() ? min.toDouble() : std::numeric_limits<double>::lowest();
    const double maxval = max.isValid() ? max.toDouble() : std::numeric_limits<double>::max();

    const QgsField field = layer()->fields().at( fieldIdx() );
    const int precisionval = precision.isValid() ? precision.toInt() : defaultFieldPrecision( field );

    mQgsTolerantDoubleSpinBox->setDecimals( precisionval );
    mQgsTolerantDoubleSpinBox->setShowClearButton( allowNull );
    // Make room for null value: lower the minimum to allow for NULL special values
    if ( allowNull )
    {
      double decr;
      if ( precisionval > 0 )
      {
        decr = std::pow( 10, -precisionval );
      }
      else
      {
        decr = stepval;
      }
      minval -= decr;
      // Note: call setMinimum here or setValue won't work
      mQgsTolerantDoubleSpinBox->setMinimum( minval );
      mQgsTolerantDoubleSpinBox->setValue( minval );
      mQgsTolerantDoubleSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    mQgsTolerantDoubleSpinBox->setMinimum( minval );
    mQgsTolerantDoubleSpinBox->setMaximum( maxval );
    mQgsTolerantDoubleSpinBox->setSingleStep( stepval );
    if ( config( u"Suffix"_s ).isValid() )
      mQgsTolerantDoubleSpinBox->setSuffix( config( u"Suffix"_s ).toString() );

    connect( mQgsTolerantDoubleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) { emitValueChanged(); } );
  }
  else if ( mDoubleSpinBox )
  {
    const double stepval = step.isValid() ? step.toDouble() : 1.0;
    double minval = min.isValid() ? min.toDouble() : std::numeric_limits<double>::lowest();
    const double maxval = max.isValid() ? max.toDouble() : std::numeric_limits<double>::max();

    const QgsField field = layer()->fields().at( fieldIdx() );
    const int precisionval = precision.isValid() ? precision.toInt() : defaultFieldPrecision( field );
    mDoubleSpinBox->setDecimals( precisionval );

    QgsTolerantDoubleSpinBox *qgsWidget = qobject_cast<QgsTolerantDoubleSpinBox *>( mDoubleSpinBox );


    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    // Make room for null value: lower the minimum to allow for NULL special values
    if ( allowNull )
    {
      double decr;
      if ( precisionval > 0 )
      {
        decr = std::pow( 10, -precisionval );
      }
      else
      {
        decr = stepval;
      }
      minval -= decr;
      // Note: call setMinimum here or setValue won't work
      mDoubleSpinBox->setMinimum( minval );
      mDoubleSpinBox->setValue( minval );
      QgsTolerantDoubleSpinBox *doubleSpinBox( qobject_cast<QgsTolerantDoubleSpinBox *>( mDoubleSpinBox ) );
      if ( doubleSpinBox )
        doubleSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
      else
        mDoubleSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    mDoubleSpinBox->setMinimum( minval );
    mDoubleSpinBox->setMaximum( maxval );
    mDoubleSpinBox->setSingleStep( stepval );
    if ( config( u"Suffix"_s ).isValid() )
      mDoubleSpinBox->setSuffix( config( u"Suffix"_s ).toString() );

    connect( mDoubleSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) { emitValueChanged(); } );
  }
  else if ( mQgsTolerantIntSpinBox )
  {
    mQgsTolerantIntSpinBox->setShowClearButton( allowNull );
    int minval = min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest();
    const int maxval = max.isValid() ? max.toInt() : std::numeric_limits<int>::max();
    const uint stepval = step.isValid() ? step.toUInt() : 1;
    if ( allowNull )
    {
      // make sure there is room for a new value (i.e. signed integer does not overflow)
      const int minvalOverflow = uint( minval ) - stepval;
      if ( minvalOverflow < minval )
      {
        minval = minvalOverflow;
      }
      mQgsTolerantIntSpinBox->setTolerantValue( minval );
      mQgsTolerantIntSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    setupIntEditor( minval, maxval, stepval, mQgsTolerantIntSpinBox, this );
    if ( config( u"Suffix"_s ).isValid() )
      mQgsTolerantIntSpinBox->setSuffix( config( u"Suffix"_s ).toString() );
  }
  else if ( mIntSpinBox )
  {
    QgsSpinBox *qgsWidget = qobject_cast<QgsSpinBox *>( mIntSpinBox );
    if ( qgsWidget )
      qgsWidget->setShowClearButton( allowNull );
    int minval = min.isValid() ? min.toInt() : std::numeric_limits<int>::lowest();
    const int maxval = max.isValid() ? max.toInt() : std::numeric_limits<int>::max();
    const uint stepval = step.isValid() ? step.toUInt() : 1;
    if ( allowNull )
    {
      // make sure there is room for a new value (i.e. signed integer does not overflow)
      const int minvalOverflow = uint( minval ) - stepval;
      if ( minvalOverflow < minval )
      {
        minval = minvalOverflow;
      }
      mIntSpinBox->setValue( minval );
      QgsSpinBox *intSpinBox( qobject_cast<QgsSpinBox *>( mIntSpinBox ) );
      if ( intSpinBox )
        intSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
      else
        mIntSpinBox->setSpecialValueText( QgsApplication::nullRepresentation() );
    }
    setupIntEditor( minval, maxval, stepval, mIntSpinBox, this );
    if ( config( u"Suffix"_s ).isValid() )
      mIntSpinBox->setSuffix( config( u"Suffix"_s ).toString() );
  }
  else
  {
    ( void ) field().convertCompatible( min );
    ( void ) field().convertCompatible( max );
    ( void ) field().convertCompatible( step );
    if ( mQgsTolerantDial )
      setupVariantEditor( min, max, step, mQgsTolerantDial, this );
    else if ( mQgsTolerantSlider )
      setupVariantEditor( min, max, step, mQgsTolerantSlider, this );
    else if ( mQgsDial )
      setupVariantEditor( min, max, step, mQgsDial, this );
    else if ( mQgsSlider )
      setupVariantEditor( min, max, step, mQgsSlider, this );
    else if ( mDial )
      setupIntEditor( min, max, step, mDial, this );
    else if ( mSlider )
      setupIntEditor( min, max, step, mSlider, this );
  }
}

bool QgsRangeWidgetWrapper::valid() const
{
  return mSlider || mDial || mQgsDial || mQgsSlider || mQgsTolerantDial || mQgsTolerantSlider || mIntSpinBox || mQgsTolerantIntSpinBox || mDoubleSpinBox || mQgsTolerantDoubleSpinBox;
}

void QgsRangeWidgetWrapper::valueChangedVariant( const QVariant &v )
{
  if ( v.userType() == QMetaType::Type::Int )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toInt() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toInt() );
  }
  else if ( v.userType() == QMetaType::Type::LongLong )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toLongLong() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toLongLong() );
  }
  else if ( v.userType() == QMetaType::Type::Double )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( v.toDouble() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( v.toDouble() );
  }
}

QVariant QgsRangeWidgetWrapper::value() const
{
  QVariant value;

  if ( mQgsTolerantDoubleSpinBox )
  {
    const QMetaType::Type fieldType = field().type();
    switch ( fieldType )
    {
      case QMetaType::Type::Double:
        value = mQgsTolerantDoubleSpinBox->value();
        break;

      case QMetaType::Type::LongLong:
        value = static_cast<long long>( mQgsTolerantDoubleSpinBox->value() );
        break;

      default:
        break;
    }

    if ( value == mQgsTolerantDoubleSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( fieldType );
    }
  }
  else if ( mDoubleSpinBox )
  {
    const QMetaType::Type fieldType = field().type();
    switch ( fieldType )
    {
      case QMetaType::Type::Double:
        value = mDoubleSpinBox->value();
        break;

      case QMetaType::Type::LongLong:
        value = static_cast<long long>( mDoubleSpinBox->value() );
        break;

      default:
        break;
    }

    if ( value == mDoubleSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( fieldType );
    }
  }
  else if ( mQgsTolerantIntSpinBox )
  {
    value = mQgsTolerantIntSpinBox->value();
    if ( value == mQgsTolerantIntSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( field().type() );
    }
  }
  else if ( mIntSpinBox )
  {
    value = mIntSpinBox->value();
    if ( value == mIntSpinBox->minimum() && config( u"AllowNull"_s, true ).toBool() )
    {
      value = QgsVariantUtils::createNullVariant( field().type() );
    }
  }
  else if ( mQgsTolerantDial )
  {
    value = mQgsTolerantDial->variantValue();
  }
  else if ( mQgsTolerantSlider )
  {
    value = mQgsTolerantSlider->variantValue();
  }
  else if ( mQgsDial )
  {
    value = mQgsDial->variantValue();
  }
  else if ( mQgsSlider )
  {
    value = mQgsSlider->variantValue();
  }
  else if ( mDial )
  {
    value = mDial->value();
  }
  else if ( mSlider )
  {
    value = mSlider->value();
  }

  return value;
}

void QgsRangeWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( mQgsTolerantDoubleSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mQgsTolerantDoubleSpinBox->setTolerantValue( mQgsTolerantDoubleSpinBox->minimum() );
    }
    else
    {
      mQgsTolerantDoubleSpinBox->setTolerantValue( value.toDouble() );
    }
  }
  else if ( mDoubleSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mDoubleSpinBox->setValue( mDoubleSpinBox->minimum() );
    }
    else
    {
      mDoubleSpinBox->setValue( value.toDouble() );
    }
  }
  else if ( mQgsTolerantIntSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mQgsTolerantIntSpinBox->setTolerantValue( mQgsTolerantIntSpinBox->minimum() );
    }
    else
    {
      mQgsTolerantIntSpinBox->setTolerantValue( value.toInt() );
    }
  }
  else if ( mIntSpinBox )
  {
    if ( QgsVariantUtils::isNull( value ) && config( u"AllowNull"_s, true ).toBool() )
    {
      mIntSpinBox->setValue( mIntSpinBox->minimum() );
    }
    else
    {
      mIntSpinBox->setValue( value.toInt() );
    }
  }
  else if ( mQgsTolerantDial )
  {
    mQgsTolerantDial->setTolerantValue( value );
  }
  else if ( mQgsTolerantSlider )
  {
    mQgsTolerantSlider->setTolerantValue( value );
  }
  else if ( mQgsDial )
  {
    mQgsDial->setValue( value );
  }
  else if ( mQgsSlider )
  {
    mQgsSlider->setValue( value );
  }
  else if ( mDial )
  {
    mDial->setValue( value.toInt() );
  }
  else if ( mSlider )
  {
    mSlider->setValue( value.toInt() );
  }
}

void QgsRangeWidgetWrapper::setEnabled( bool enabled )
{
  if ( mQgsTolerantDoubleSpinBox )
  {
    mQgsTolerantDoubleSpinBox->setReadOnly( !enabled );
    mQgsTolerantDoubleSpinBox->setFrame( enabled );
  }
  else if ( mDoubleSpinBox )
  {
    mDoubleSpinBox->setReadOnly( !enabled );
    mDoubleSpinBox->setFrame( enabled );
  }
  else if ( mQgsTolerantIntSpinBox )
  {
    mQgsTolerantIntSpinBox->setReadOnly( !enabled );
    mQgsTolerantIntSpinBox->setFrame( enabled );
  }
  else if ( mIntSpinBox )
  {
    mIntSpinBox->setReadOnly( !enabled );
    mIntSpinBox->setFrame( enabled );
  }
  else
  {
    QgsEditorWidgetWrapper::setEnabled( enabled );
  }
}

// Tolerant SpinBox

QgsTolerantSpinBox::QgsTolerantSpinBox( QWidget *parent )
  : QgsSpinBox( parent )
{}

QString QgsTolerantSpinBox::textFromValue( int val ) const
{
  Q_UNUSED( val );
  return QString::number( mTolerantValue );
}

void QgsTolerantSpinBox::stepBy( int steps )
{
  QgsSpinBox::stepBy( steps );
  setTolerantValue( QgsSpinBox::value() );
}

int QgsTolerantSpinBox::value() const
{
  return mTolerantValue;
}

void QgsTolerantSpinBox::setTolerantValue( int val )
{
  // the tolerant value should remain untruncated
  mTolerantValue = val;
  // the value will be truncated
  QgsSpinBox::setValue( val );
}

void QgsTolerantSpinBox::focusOutEvent( QFocusEvent *event )
{
  QgsSpinBox::focusOutEvent( event );
  setTolerantValue( QgsSpinBox::value() );
  // we have to emit valueChanged to save truncating on focus out
  emit valueChanged( QgsSpinBox::value() );
}

void QgsTolerantSpinBox::wheelEvent( QWheelEvent *event )
{
  QgsSpinBox::wheelEvent( event );
  setTolerantValue( QgsSpinBox::value() );
}

// Tolerant DoubleSpinBox

QgsTolerantDoubleSpinBox::QgsTolerantDoubleSpinBox( QWidget *parent )
  : QgsDoubleSpinBox( parent )
{}

QString QgsTolerantDoubleSpinBox::textFromValue( double val ) const
{
  Q_UNUSED( val );
  return QLocale().toString( mTolerantValue, 'f', decimals() );
}

void QgsTolerantDoubleSpinBox::stepBy( int steps )
{
  QgsDoubleSpinBox::stepBy( steps );
  setTolerantValue( QgsDoubleSpinBox::value() );
}

double QgsTolerantDoubleSpinBox::value() const
{
  return mTolerantValue;
}

void QgsTolerantDoubleSpinBox::setTolerantValue( double val )
{
  // the tolerant value should remain untruncated
  mTolerantValue = val;
  // the value will be truncated
  QgsDoubleSpinBox::setValue( val );
}

void QgsTolerantDoubleSpinBox::focusOutEvent( QFocusEvent *event )
{
  QgsDoubleSpinBox::focusOutEvent( event );
  setTolerantValue( QgsDoubleSpinBox::value() );
  // we have to emit valueChanged to save truncating on focus out
  emit valueChanged( QgsDoubleSpinBox::value() );
}

void QgsTolerantDoubleSpinBox::wheelEvent( QWheelEvent *event )
{
  QgsDoubleSpinBox::wheelEvent( event );
  setTolerantValue( QgsDoubleSpinBox::value() );
}

// Tolerant Slider

QgsTolerantSlider::QgsTolerantSlider( QWidget *parent )
  : QgsSlider( parent )
{
  // when the value edited, the tolerant value should be updated as well
  connect( this, &QSlider::valueChanged, this, [this]( const int value ) { mTolerantValue = value; } );
}

QgsTolerantSlider::QgsTolerantSlider( Qt::Orientation orientation, QWidget *parent )
  : QgsSlider( orientation, parent )
{
  // when the value edited, the tolerant value should be updated as well
  connect( this, &QSlider::valueChanged, this, [this]( const int value ) { mTolerantValue = value; } );
}

QVariant QgsTolerantSlider::variantValue() const
{
  return mTolerantValue;
}

void QgsTolerantSlider::setTolerantValue( const QVariant &val )
{
  setValue( val );
  // the tolerant value should be set after the value, to avoid truncating
  mTolerantValue = val;
}

// Tolerant Dial

QgsTolerantDial::QgsTolerantDial( QWidget *parent )
  : QgsDial( parent )
{
  // when the value edited, the tolerant value should be updated as well
  connect( this, &QSlider::valueChanged, this, [this]( const int value ) { mTolerantValue = value; } );
}

QVariant QgsTolerantDial::variantValue() const
{
  return mTolerantValue;
}

void QgsTolerantDial::setTolerantValue( const QVariant &val )
{
  setValue( val );
  // the tolerant value should be set after the value, to avoid truncating
  mTolerantValue = val;
}
