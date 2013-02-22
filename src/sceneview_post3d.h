// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef SCENEVIEWPOST3D_H
#define SCENEVIEWPOST3D_H

#include "util.h"
#include "sceneview_common3d.h"

template <typename Scalar> class SceneSolution;
template <typename Scalar> class ViewScalarFilter;

class SceneViewPost3D : public SceneViewCommon3D
{
    Q_OBJECT

public slots:
    virtual void clear();

public:
    SceneViewPost3D(PostHermes *postHermes, QWidget *parent = 0);
    ~SceneViewPost3D();

    QAction *actSceneModePost3D;

    virtual QIcon iconView() { return icon("scene-post3d"); }
    virtual QString labelView() { return tr("Postprocessor 3D"); }

protected:
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    void paintScalarField3D(); // paint scalar field 3d surface
    void paintScalarField3DSolid(); // paint scalar field 3d solid

private:
    // gl lists
    int m_listScalarField3D;
    int m_listScalarField3DSolid;
    int m_listModel;

    void createActionsPost3D();

private slots:
    virtual void refresh();
    virtual void clearGLLists();
};

#endif // SCENEVIEWPOST3D_H
