/***************************************************************************
  qgs3dmapcanvas.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPCANVAS_H
#define QGS3DMAPCANVAS_H

#include <QWidget>

namespace Qt3DExtras
{
  class Qt3DWindow;
}

class Qgs3DMapSettings;
class Qgs3DMapScene;
class QgsCameraController;
class QgsPointXY;


class Qgs3DMapCanvas : public QWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvas( QWidget *parent = nullptr );
    ~Qgs3DMapCanvas() override;

    //! Configure map scene being displayed. Takes ownership.
    void setMap( Qgs3DMapSettings *map );

    //! Returns access to the 3D scene configuration
    Qgs3DMapSettings *map() { return mMap; }

    //! Returns access to the 3D scene (root 3D entity)
    Qgs3DMapScene *scene() { return mScene; }

    //! Returns access to the view's camera controller. Returns null pointer if the scene has not been initialized yet with setMap()
    QgsCameraController *cameraController();

    //! Resets camera position to the default: looking down at the origin of world coordinates
    void resetView();

    //! Sets camera position to look down at the given point (in map coordinates) in given distance from plane with zero elevation
    void setViewFromTop( const QgsPointXY &center, float distance, float rotation = 0 );

  protected:
    void resizeEvent( QResizeEvent *ev ) override;

  private:
    //! 3D window with all the 3D magic inside
    Qt3DExtras::Qt3DWindow *mWindow3D = nullptr;
    //! Container QWidget that encapsulates mWindow3D so we can use it embedded in ordinary widgets app
    QWidget *mContainer = nullptr;
    //! Description of the 3D scene
    Qgs3DMapSettings *mMap = nullptr;
    //! Root entity of the 3D scene
    Qgs3DMapScene *mScene = nullptr;
};

#endif // QGS3DMAPCANVAS_H
