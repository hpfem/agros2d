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

#ifndef SCENEVIEWMESH_H
#define SCENEVIEWMESH_H

#include "util.h"
#include "sceneview_common2d.h"

class SceneViewMesh : public SceneViewCommon2D
{
    Q_OBJECT

public:
    SceneViewMesh(PostDeal *postDeal, QWidget *parent = 0);
    ~SceneViewMesh();

    QAction *actSceneModeMesh;
    QAction *actExportVTKOrder;
    QAction *actExportVTKMesh;

    virtual QIcon iconView() { return icon("scene-mesh"); }
    virtual QString labelView() { return tr("Mesh and polynomial order view"); }

public slots:
    void setControls();
    virtual void clear();
    void exportVTK(const QString &fileName = QString(), bool exportMeshOnly = false);
    void exportVTKMesh(const QString &fileName = QString());
    void exportVTKOrderView(const QString &fileName = QString());

protected:
    virtual void paintGL();

    void paintGeometry();

    void paintInitialMesh();
    void paintSolutionMesh();
    void paintOrder();
    void paintOrderColorBar();

private:
    QVector<QVector2D> m_arrayInitialMesh;
    QVector<QVector2D> m_arraySolutionMesh;
    QVector<QVector2D> m_arrayOrderMesh;
    QVector<QVector3D> m_arrayOrderMeshColor;

    void createActionsMesh();

private slots:
    virtual void refresh();
};

#endif // SCENEVIEWMESH_H
