// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

#include <QGLWidget>

#include "util.h"

class Scene;
class SceneViewCommon;
class SceneViewInterface;

class SceneViewCommon;

class SceneViewWidget : public QWidget
{
    Q_OBJECT
public:
   SceneViewWidget(SceneViewCommon *widget, QWidget *parent = 0);
   SceneViewWidget(QWidget *widget, QWidget *parent = 0);
   ~SceneViewWidget();

public slots:
   void labelLeft(const QString &left);
   void labelCenter(const QString &center);
   void labelRight(const QString &right);
   void iconLeft(const QIcon &left);

private:
   void createControls(QWidget *widget);

   QLabel *sceneViewLabelPixmap;
   QLabel *sceneViewLabelLeft;
   QLabel *sceneViewLabelCenter;
   QLabel *sceneViewLabelRight;
};

#endif // SCENEWIDGET_H
