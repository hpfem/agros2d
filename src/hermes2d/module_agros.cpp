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

#include "module_agros.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "field.h"
#include "problem.h"

void Module::ModuleAgros::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, viewScalarVariables());
}

void Module::ModuleAgros::fillComboBoxContourVariable(QComboBox *cmbFieldVariable)
{
    foreach (Module::LocalVariable *variable, viewScalarVariables())
        cmbFieldVariable->addItem(variable->name(),
                                  variable->id());
}

void Module::ModuleAgros::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, viewVectorVariables());
}

void Module::ModuleAgros::fillComboBox(QComboBox *cmbFieldVariable, QList<Module::LocalVariable *> list)
{
    foreach (Module::LocalVariable *variable, list)
        cmbFieldVariable->addItem(variable->name(),
                                  variable->id());
}

void Module::ModuleAgros::fillComboBoxBoundaryCondition(QComboBox *cmbFieldVariable)
{
    foreach (Module::BoundaryType *boundary, boundaryTypes())
        cmbFieldVariable->addItem(boundary->name(),
                                  boundary->id());
}

void Module::ModuleAgros::fillComboBoxMaterialProperties(QComboBox *cmbFieldVariable)
{
    foreach (Module::MaterialTypeVariable *material, materialTypeVariables())
        cmbFieldVariable->addItem(material->id(),
                                  material->id());
}

SceneBoundary *Module::ModuleAgros::newBoundary()
{
    FieldInfo *field = Agros2D::problem()->fieldInfo(this->m_fieldid);

    // TODO: (Franta) default of other boundary types
    SceneBoundary *boundary = new SceneBoundary(field, tr("new boundary"),
                                 field->module()->boundaryTypeDefault()->id());

    return boundary;
}

SceneMaterial *Module::ModuleAgros::newMaterial()
{
    FieldInfo *field = Agros2D::problem()->fieldInfo(this->m_fieldid);

    SceneMaterial *material = new SceneMaterial(field, tr("new material"));

    return material;
}

//SceneBoundaryContainer Module::ModuleAgros::boundaries()
//{
//    //TODO - how to cast???
//    MarkerContainer<SceneBoundary> boundaries = Agros2D::scene()->boundaries->filter(QString::fromStdString(fieldid));

//    SceneBoundaryContainer boundaryContainer;
//    foreach (SceneBoundary *boundary, boundaries.items())
//        boundaryContainer.add(boundary);

//    return boundaryContainer;
//}

//SceneMaterialContainer Module::ModuleAgros::materials()
//{
//    //TODO - how to cast???
//    MarkerContainer<SceneMaterial> materials = Agros2D::scene()->materials->filter(QString::fromStdString(fieldid));

//    SceneMaterialContainer materialContainer;
//    foreach (SceneMaterial *material, materials.items())
//        materialContainer.add(material);

//    return materialContainer;
//}

template <class T>
void deformShapeTemplate(T linVert, int count)
{
    MultiSolutionArray<double> msa = Agros2D::scene()->activeMultiSolutionArray();

    double min =  numeric_limits<double>::max();
    double max = -numeric_limits<double>::max();
    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = msa.component(0).sln.data()->get_pt_value(x, y)->val[0];
        double dy = msa.component(1).sln.data()->get_pt_value(x, y)->val[0];

        double dm = sqrt(Hermes::sqr(dx) + Hermes::sqr(dy));

        if (dm < min) min = dm;
        if (dm > max) max = dm;
    }

    RectPoint rect = Agros2D::scene()->boundingBox();
    double k = qMax(rect.width(), rect.height()) / qMax(min, max) / 15.0;

    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = msa.component(0).sln.data()->get_pt_value(x, y)->val[0];
        double dy = msa.component(1).sln.data()->get_pt_value(x, y)->val[0];

        linVert[i][0] += k*dx;
        linVert[i][1] += k*dy;
    }
}

void Module::ModuleAgros::deformShape(double3* linVert, int count)
{
    if (Agros2D::problem()->fieldInfo(m_fieldid)->module()->hasDeformableShape())
        deformShapeTemplate<double3 *>(linVert, count);
}

void Module::ModuleAgros::deformShape(double4* linVert, int count)
{
    if (Agros2D::problem()->fieldInfo(m_fieldid)->module()->hasDeformableShape())
        deformShapeTemplate<double4 *>(linVert, count);
}

void Module::ModuleAgros::updateTimeFunctions(double time)
{
    // update materials
    foreach (SceneMaterial *material, Agros2D::scene()->materials->items())
        if (material->fieldInfo())
            foreach (Module::MaterialTypeVariable *variable, material->fieldInfo()->module()->materialTypeVariables())
                if (variable->isTimeDep() && material->fieldInfo()->analysisType() == AnalysisType_Transient)
                    material->evaluate(variable->id(), time);

    // update boundaries
    foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->items())
        if (boundary->fieldInfo())
            foreach (Module::BoundaryType *boundaryType, boundary->fieldInfo()->module()->boundaryTypes())
                foreach (Module::BoundaryTypeVariable *variable, boundaryType->variables())
                    if (variable->isTimeDep() && boundary->fieldInfo()->analysisType() == AnalysisType_Transient)
                        boundary->evaluate(variable->id(), time);
}
