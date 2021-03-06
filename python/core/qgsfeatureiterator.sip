/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeatureiterator.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAbstractFeatureIterator
{
%Docstring
Internal feature iterator to be implemented within data providers
%End

%TypeHeaderCode
#include "qgsfeatureiterator.h"
%End
  public:

    enum CompileStatus
    {
      NoCompilation,
      PartiallyCompiled,
      Compiled,
    };

    QgsAbstractFeatureIterator( const QgsFeatureRequest &request );
%Docstring
base class constructor - stores the iteration parameters
%End

    virtual ~QgsAbstractFeatureIterator();
%Docstring
destructor makes sure that the iterator is closed properly
%End

    virtual bool nextFeature( QgsFeature &f );
%Docstring
fetch next feature, return true on success
%End

    virtual bool rewind() = 0;
%Docstring
reset the iterator to the starting position
%End
    virtual bool close() = 0;
%Docstring
end of iterating: free the resources / lock
%End


    CompileStatus compileStatus() const;
%Docstring
Returns the status of expression compilation for filter expression requests.

.. versionadded:: 2.16
%End

    virtual bool isValid() const;
%Docstring
Returns if this iterator is valid.
An invalid feature iterator is not able to provide a reliable source for data.
If an iterator is invalid, either give up or try to send the request again (preferably
after a timeout to give the system some time to stay responsive).

If you want to check if the iterator successfully completed, better use QgsFeatureIterator.isClosed().

.. versionadded:: 3.0
%End

  protected:

    virtual bool fetchFeature( QgsFeature &f ) = 0;
%Docstring
If you write a feature iterator for your provider, this is the method you
need to implement!!

:param f: The feature to write to

:return:  true if a feature was written to f
%End

    virtual bool nextFeatureFilterExpression( QgsFeature &f );
%Docstring
By default, the iterator will fetch all features and check if the feature
matches the expression.
If you have a more sophisticated metodology (SQL request for the features...)
and you check for the expression in your fetchFeature method, you can just
redirect this call to fetchFeature so the default check will be omitted.

:param f: The feature to write to

:return:  true if a feature was written to f
%End

    virtual bool nextFeatureFilterFids( QgsFeature &f );
%Docstring
By default, the iterator will fetch all features and check if the id
is in the request.
If you have a more sophisticated metodology (SQL request for the features...)
and you are sure, that any feature you return from fetchFeature will match
if the request was FilterFids you can just redirect this call to fetchFeature
so the default check will be omitted.

:param f: The feature to write to

:return:  true if a feature was written to f
%End

    void geometryToDestinationCrs( QgsFeature &feature, const QgsCoordinateTransform &transform ) const;
%Docstring
Transforms ``feature``'s geometry according to the specified coordinate ``transform``.
If ``feature`` has no geometry or ``transform`` is invalid then calling this method
has no effect and will be shortcut.
Iterators should call this method before returning features to ensure that any
QgsFeatureRequest.destinationCrs() set on the request is respected.

.. versionadded:: 3.0
%End


    QgsRectangle filterRectToSourceCrs( const QgsCoordinateTransform &transform ) const;
%Docstring
Returns a rectangle representing the original request's QgsFeatureRequest.filterRect().
If ``transform`` is a valid coordinate transform, the return rectangle will represent
the requested filterRect() transformed to the source's coordinate reference system.
Iterators should call this method and use the returned rectangle for filtering
features to ensure that any QgsFeatureRequest.destinationCrs() set on the request is respected.
Will throw a QgsCsException if the rect cannot be transformed from the destination CRS.

.. versionadded:: 3.0
%End




    void ref();
%Docstring
Add reference
%End
    void deref();
%Docstring
Remove reference, delete if refs == 0
%End



    virtual bool prepareSimplification( const QgsSimplifyMethod &simplifyMethod );
%Docstring
Setup the simplification of geometries to fetch using the specified simplify method
%End


};


template<T>
class QgsAbstractFeatureIteratorFromSource : QgsAbstractFeatureIterator
{
%Docstring
Helper template that cares of two things: 1. automatic deletion of source if owned by iterator, 2. notification of open/closed iterator.

.. note::

   not available in Python bindings (although present in SIP file)
%End

%TypeHeaderCode
#include "qgsfeatureiterator.h"
%End
  public:
    QgsAbstractFeatureIteratorFromSource( T *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsAbstractFeatureIteratorFromSource();

  protected:
    void iteratorClosed();
%Docstring
to be called by from subclass in close()
%End

};


class QgsFeatureIterator
{
%Docstring
Wrapper for iterator of features from vector data provider or vector layer
%End

%TypeHeaderCode
#include "qgsfeatureiterator.h"
%End
  public:

    QgsFeatureIterator *__iter__();
%MethodCode
    sipRes = sipCpp;
%End

    SIP_PYOBJECT __next__();
%MethodCode
    QgsFeature *f = new QgsFeature;
    if ( sipCpp->nextFeature( *f ) )
      sipRes = sipConvertFromType( f, sipType_QgsFeature, Py_None );
    else
    {
      delete f;
      PyErr_SetString( PyExc_StopIteration, "" );
    }
%End

    QgsFeatureIterator();
%Docstring
construct invalid iterator
%End
    QgsFeatureIterator( const QgsFeatureIterator &fi );
%Docstring
copy constructor copies the iterator, increases ref.count
%End
    ~QgsFeatureIterator();
%Docstring
destructor deletes the iterator if it has no more references
%End


    bool nextFeature( QgsFeature &f );
    bool rewind();
    bool close();

    virtual bool isValid() const;
%Docstring
Will return if this iterator is valid.
An invalid iterator was probably introduced by a failed attempt to acquire a connection
or is a default constructed iterator.

.. seealso:: :py:func:`isClosed`

.. versionadded:: 3.0
%End

    bool isClosed() const;
%Docstring
find out whether the iterator is still valid or closed already
%End


    QgsAbstractFeatureIterator::CompileStatus compileStatus() const;
%Docstring
Returns the status of expression compilation for filter expression requests.

.. versionadded:: 2.16
%End


  protected:


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeatureiterator.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
