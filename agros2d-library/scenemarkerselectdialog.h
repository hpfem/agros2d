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

#ifndef SCENEMARKERSELECTDIALOG_H
#define SCENEMARKERSELECTDIALOG_H

#include "util.h"
#include "util/enums.h"

class Scene;
class SceneViewPost2D;
class FieldInfo;

class SceneMarkerSelectDialog : public QDialog
{
    Q_OBJECT
public:
    SceneMarkerSelectDialog(SceneViewPost2D *sceneView, SceneModePostprocessor mode, QWidget *parent = 0);

private slots:
    void doAccept();
    void doReject();

protected:
    void createControls();

private:
    SceneViewPost2D *m_sceneViewPost2D;

    QTabWidget* tabWidget;

    QWidget *widSurface;
    QWidget *widVolume;

    QListWidget *lstSurface;
    QListWidget *lstVolume;
};

#endif // SCENEMARKERSELECTDIALOG_H
