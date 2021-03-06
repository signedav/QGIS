/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeaturelistview.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFeatureListView : QListView
{
%Docstring
Shows a list of features and renders a edit button next to each feature.

Accepts a display expression to define the way, features are rendered.
Uses a QgsFeatureListModel as source model.
%End

%TypeHeaderCode
#include "qgsfeaturelistview.h"
%End
  public:

    explicit QgsFeatureListView( QWidget *parent /TransferThis/ = 0 );
%Docstring
Creates a feature list view

:param parent:   owner
%End

    QgsVectorLayerCache *layerCache();
%Docstring
Returns the layer cache

:return: the layer cache used as backend
%End

    virtual void setModel( QgsFeatureListModel *featureListModel );
%Docstring
Set the QgsFeatureListModel which is used to retrieve information

:param featureListModel:  The model to use
%End

    QgsFeatureListModel *featureListModel();
%Docstring
Get the featureListModel used by this view

:return: The model in use
%End

    bool setDisplayExpression( const QString &displayExpression );
%Docstring
The display expression is an expression used to render the fields into a single string
which is displaied.

:param displayExpression:  The expression used to render the feature

.. seealso:: :py:class:`QgsExpression`
%End

    const QString displayExpression() const;
%Docstring
Returns the expression which is currently used to render the features.

:return: A string containing the currend display expression

.. seealso:: :py:class:`QgsExpression`
%End

    QString parserErrorString();
%Docstring
Returns a detailed message about errors while parsing a QgsExpression.

:return: A message containing information about the parser error.
%End

    QgsFeatureIds currentEditSelection();
%Docstring
Get the currentEditSelection

:return: A list of edited feature ids
%End

    void setCurrentFeatureEdited( bool state );
%Docstring
Sets if the currently shown form has received any edit events so far.

:param state: The state
%End

    void setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager /Transfer/ );
%Docstring
 setFeatureSelectionManager

:param featureSelectionManager: We will take ownership
%End

  protected:
    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );

    virtual void mouseReleaseEvent( QMouseEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );

    virtual void contextMenuEvent( QContextMenuEvent *event );


  signals:

    void currentEditSelectionChanged( QgsFeature &feat );
%Docstring
Is emitted, whenever the current edit selection has been changed.

:param feat: the feature, which will be edited.
%End

    void displayExpressionChanged( const QString &expression );
%Docstring
Is emitted, whenever the display expression is successfully changed

:param expression: The expression that was applied
%End


  public slots:

    void setEditSelection( const QgsFeatureIds &fids );
%Docstring
Set the feature(s) to be edited

:param fids:  A list of features to be edited
%End

    void setEditSelection( const QModelIndex &index, QItemSelectionModel::SelectionFlags command );
%Docstring
Set the feature(s) to be edited

:param index: The selection to set
:param command: selection update mode
%End

    virtual void selectAll();

%Docstring
Select all currently visible features
%End

    void repaintRequested( const QModelIndexList &indexes );
    void repaintRequested();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfeaturelistview.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
