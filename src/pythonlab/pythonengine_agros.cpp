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

#include "pythonlab/pythonengine_agros.h"

#include "util/constants.h"
#include "util/global.h"

#include "../resources_source/python/agros2d.cpp"

#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "datatable.h"

#include "hermes2d/plugin_interface.h"
#include "hermes2d/module.h"

#include "hermes2d/problem.h"
#include "hermes2d/coupling.h"

#include <Python.h>

// current python engine agros
AGROS_API PythonEngineAgros *currentPythonEngineAgros()
{
    return static_cast<PythonEngineAgros *>(currentPythonEngine());
}

void PythonEngineAgros::addCustomExtensions()
{
    PythonEngine::addCustomExtensions();

    // init agros cython extensions
    initagros2d();
}

void PythonEngineAgros::addCustomFunctions()
{
    addFunctions(readFileContent(datadir() + "/functions_agros2d.py"));
}

void PythonEngineAgros::runPythonHeader()
{
    QString script;

    // global script
    if (!Agros2D::configComputer()->globalScript.isEmpty())
        script += Agros2D::configComputer()->globalScript + "\n";

    // startup script
    if (!Agros2D::problem()->configView()->startupScript.trimmed().isEmpty())
        script += Agros2D::problem()->configView()->startupScript + "\n";

    // run script
    if (!script.trimmed().isEmpty())
        PyRun_String(script.toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

PythonLabAgros::PythonLabAgros(PythonEngine *pythonEngine, QStringList args, QWidget *parent)
    : PythonEditorDialog(pythonEngine, args, parent)
{
    // add create from model
    actCreateFromModel = new QAction(icon("script-create"), tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreatePythonFromModel()));

    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);

    tlbTools->addSeparator();
    tlbTools->addAction(actCreateFromModel);
}

void PythonLabAgros::doCreatePythonFromModel()
{
    txtEditor->setPlainText(createPythonFromModel());
}

// create script from model
QString createPythonFromModel()
{
    QString str;

    // import modules
    str += "import agros2d\n\n";

    // startup script
    if (!Agros2D::problem()->configView()->startupScript.trimmed().isEmpty())
    {
        str += "# startup script\n";
        str += Agros2D::problem()->configView()->startupScript;
        str += "\n\n";
    }

    // model
    str += "# problem\n";
    str += QString("problem = agros2d.problem(clear = True)\n");
    if (!Agros2D::problem()->config()->name().isEmpty())
        str += QString("problem.name = \"%1\"\n").arg(Agros2D::problem()->config()->name());
    str += QString("problem.coordinate_type = \"%1\"\n").arg(coordinateTypeToStringKey(Agros2D::problem()->config()->coordinateType()));
    str += QString("problem.mesh_type = \"%1\"\n").arg(meshTypeToStringKey(Agros2D::problem()->config()->meshType()));
    str += QString("problem.matrix_solver = \"%1\"\n").arg(matrixSolverTypeToStringKey(Agros2D::problem()->config()->matrixSolver()));

    if (Agros2D::problem()->isHarmonic())
        str += QString("problem.frequency = %1\n").
                arg(Agros2D::problem()->config()->frequency());

    if (Agros2D::problem()->isTransient())
    {
        str += QString("problem.time_step_method = \"%1\"\n"
                       "problem.time_method_order = %2\n"
                       "problem.time_total = %3\n").
                arg(timeStepMethodToStringKey(Agros2D::problem()->config()->timeStepMethod())).
                arg(Agros2D::problem()->config()->timeOrder()).
                arg(Agros2D::problem()->config()->timeTotal().toString());

        if (Agros2D::problem()->config()->timeStepMethod() == TimeStepMethod_BDFTolerance)
        {
            str += QString("problem.time_method_tolerance = %3\n").
                    arg(Agros2D::problem()->config()->timeMethodTolerance().toString());
        }
        else
        {
            str += QString("problem.time_steps = %5\n").
                    arg(Agros2D::problem()->config()->numConstantTimeSteps());
        }
    }

    // fields
    str += "\n# fields\n";
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        str += QString("# %1\n").arg(fieldInfo->fieldId());

        // str += QString("%1 = agros2d.field(field_id = \"%2\")\n").
        str += QString("%1 = agros2d.field(\"%2\")\n").
                arg(fieldInfo->fieldId()).
                arg(fieldInfo->fieldId());
        str += QString("%1.analysis_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(analysisTypeToStringKey(fieldInfo->analysisType()));

        if (Agros2D::problem()->isTransient())
        {
            if (fieldInfo->analysisType() == analysisTypeFromStringKey("transient"))
            {
                str += QString("%1.initial_condition = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->initialCondition().number());
            }
            else
            {
                str += QString("%1.time_skip = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->timeSkip().number());
            }
        }

        if (fieldInfo->numberOfRefinements() > 0)
            str += QString("%1.number_of_refinements = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->numberOfRefinements());

        if (fieldInfo->polynomialOrder() > 0)
            str += QString("%1.polynomial_order = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->polynomialOrder());

        str += QString("%1.adaptivity_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(adaptivityTypeToStringKey(fieldInfo->adaptivityType()));

        if (fieldInfo->adaptivityType() != AdaptivityType_None)
        {
            str += QString("%1.adaptivity_steps = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->adaptivitySteps());

            str += QString("%1.adaptivity_tolerance = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->adaptivityTolerance());

            if (Agros2D::problem()->isTransient())
            {
                str += QString("%1.adaptivity_back_steps = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->adaptivityBackSteps());

                str += QString("%1.adaptivity_redone = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->adaptivityRedoneEach());
            }
        }

        str += QString("%1.linearity_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(linearityTypeToStringKey(fieldInfo->linearityType()));

        if (fieldInfo->linearityType() != LinearityType_Linear)
        {
            str += QString("%1.nonlinear_tolerance = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->nonlinearTolerance());

            str += QString("%1.nonlinear_steps = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->nonlinearSteps());
        }

        // newton
        if (fieldInfo->linearityType() == LinearityType_Newton)
        {
            str += QString("%1.automatic_damping = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->newtonAutomaticDamping()) ? "True" : "False");

            if (fieldInfo->newtonAutomaticDamping())
            {
                str += QString("%1.damping_number_to_increase = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->newtonDampingNumberToIncrease());
            }
            else
            {
                str += QString("%1.damping_coeff = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->newtonDampingCoeff());
            }
        }

        // picard
        if (fieldInfo->linearityType() == LinearityType_Picard)
        {
            str += QString("%1.anderson_acceleration = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->picardAndersonAcceleration()) ? "True" : "False");

            if (fieldInfo->picardAndersonAcceleration())
            {
                str += QString("%1.anderson_beta = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->picardAndersonBeta());

                str += QString("%1.anderson_last_vectors = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->picardAndersonNumberOfLastVectors());
            }
        }

        str += "\n";

        str += "\n# boundaries\n";
        foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(fieldInfo).items())
        {
            const QHash<QString, Value> values = boundary->values();

            QString variables = "{";

            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());
            foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
            {
                Value value = values[variable.id()];

                if (value.hasExpression())
                {
                    variables += QString("\"%1\" : { \"expression\" : \"%2\" }, ").
                            arg(variable.id()).
                            arg(value.text());
                }
                else
                {
                    variables += QString("\"%1\" : %2, ").
                            arg(variable.id()).
                            arg(value.toString());
                }
            }
            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_boundary(\"%2\", \"%3\", %4)\n").
                    arg(fieldInfo->fieldId()).
                    arg(boundary->name()).
                    arg(boundary->type()).
                    arg(variables);
        }

        str += "\n";

        str += "\n# materials\n";
        foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(fieldInfo).items())
        {
            const QHash<QString, Value> values = material->values();

            QString variables = "{";
            foreach (Module::MaterialTypeVariable variable, material->fieldInfo()->materialTypeVariables())
            {
                Value value = values[variable.id()];

                if (value.hasTable())
                {
                    if (value.hasExpression())
                        variables += QString("\"%1\" : { \"expression\" : \"%2\", \"x\" : [%3], \"y\" : [%4] }, ").
                                arg(variable.id()).
                                arg(value.text()).
                                arg(value.table().toStringX()).
                                arg(value.table().toStringY());
                    else
                        variables += QString("\"%1\" : { \"value\" : %2, \"x\" : [%3], \"y\" : [%4] }, ").
                                arg(variable.id()).
                                arg(value.number()).
                                arg(value.table().toStringX()).
                                arg(value.table().toStringY());
                }
                else if (value.hasExpression())
                {
                    variables += QString("\"%1\" : { \"expression\" : \"%2\" }, ").
                            arg(variable.id()).
                            arg(value.text());
                }
                else
                {
                    variables += QString("\"%1\" : %2, ").
                            arg(variable.id()).
                            arg(value.toString());
                }
            }
            variables = (variables.endsWith(", ") ? variables.left(variables.length() - 2) : variables) + "}";

            str += QString("%1.add_material(\"%2\", %3)\n").
                    arg(fieldInfo->fieldId()).
                    arg(material->name()).
                    arg(variables);
        }

        str += "\n";
    }

    // geometry
    str += "# geometry\n";
    str += "geometry = agros2d.geometry\n";

    // edges
    if (Agros2D::scene()->edges->count() > 0)
    {
        //str += "\n# edges\n";
        foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        {
            str += QString("geometry.add_edge(%1, %2, %3, %4").
                    arg(edge->nodeStart()->point().x).
                    arg(edge->nodeStart()->point().y).
                    arg(edge->nodeEnd()->point().x).
                    arg(edge->nodeEnd()->point().y);

            if (edge->angle() > 0.0)
                str += ", angle = " + QString::number(edge->angle());

            // refinement
            if (Agros2D::problem()->fieldInfos().count() > 0)
            {
                int refinementCount = 0;
                QString refinements = ", refinements = {";
                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    if (fieldInfo->edgeRefinement(edge) > 0)
                    {
                        refinements += QString("\"%1\" : %2, ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->edgeRefinement(edge));

                        refinementCount++;
                    }
                }
                refinements = (refinements.endsWith(", ") ? refinements.left(refinements.length() - 2) : refinements) + "}";

                if (refinementCount > 0)
                    str += refinements;
            }

            // boundaries
            if (Agros2D::problem()->fieldInfos().count() > 0)
            {
                int boundariesCount = 0;
                QString boundaries = ", boundaries = {";
                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    SceneBoundary *marker = edge->marker(fieldInfo);

                    if (marker != Agros2D::scene()->boundaries->getNone(fieldInfo))
                    {
                        boundaries += QString("\"%1\" : \"%2\", ").
                                arg(fieldInfo->fieldId()).
                                arg(marker->name());

                        boundariesCount++;
                    }
                }
                boundaries = (boundaries.endsWith(", ") ? boundaries.left(boundaries.length() - 2) : boundaries) + "}";
                if (boundariesCount > 0)
                    str += boundaries;
            }

            str += ")\n";
        }
        str += "\n";
    }

    // labels
    if (Agros2D::scene()->labels->count() > 0)
    {
        //str += "# labels\n";
        foreach (SceneLabel *label, Agros2D::scene()->labels->items())
        {
            str += QString("geometry.add_label(%1, %2").
                    arg(label->point().x).
                    arg(label->point().y);

            if (label->area() > 0.0)
                str += ", area = " + QString::number(label->area());

            // refinements
            if (Agros2D::problem()->fieldInfos().count() > 0)
            {
                int refinementsCount = 0;
                QString refinements = ", refinements = {";
                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    if (fieldInfo->labelRefinement(label) > 0)
                    {
                        refinements += QString("\"%1\" : \"%2\", ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->labelRefinement(label));

                        refinementsCount++;
                    }
                }
                refinements = (refinements.endsWith(", ") ? refinements.left(refinements.length() - 2) : refinements) + "}";
                if (refinementsCount > 0)
                    str += refinements;
            }

            // orders
            if (Agros2D::problem()->fieldInfos().count() > 0)
            {
                int ordersCount = 0;
                QString orders = ", orders = {";
                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    if (fieldInfo->labelPolynomialOrder(label) != fieldInfo->polynomialOrder())
                    {
                        orders += QString("\"%1\" : %2, ").
                                arg(fieldInfo->fieldId()).
                                arg(fieldInfo->labelPolynomialOrder(label));

                        ordersCount++;
                    }
                }
                orders = (orders.endsWith(", ") ? orders.left(orders.length() - 2) : orders) + "}";
                if (ordersCount > 0)
                    str += orders;
            }

            // materials
            if (Agros2D::problem()->fieldInfos().count() > 0)
            {
                QString materials = ", materials = {";
                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    SceneMaterial *marker = label->marker(fieldInfo);

                    materials += QString("\"%1\" : \"%2\", ").
                            arg(fieldInfo->fieldId()).
                            arg(marker->name());
                }
                materials = (materials.endsWith(", ") ? materials.left(materials.length() - 2) : materials) + "}";
                str += materials;
            }

            str += ")\n";
        }
    }
    str += "geometry.zoom_best_fit()";

    return str;
}

// ************************************************************************************

void pyOpenDocument(char *str)
{
    ErrorResult result = Agros2D::scene()->readFromFile(QString(str));
    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}

void pySaveDocument(char *str)
{
    ErrorResult result = Agros2D::scene()->writeToFile(QString(str));
    if (result.isError())
        throw invalid_argument(result.message().toStdString());
}

void pyCloseDocument()
{
    Agros2D::scene()->clear();
    // sceneView()->doDefaultValues();
    Agros2D::scene()->invalidate();

    currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();

    currentPythonEngineAgros()->sceneViewPreprocessor()->doZoomBestFit();
    currentPythonEngineAgros()->sceneViewMesh()->doZoomBestFit();
    currentPythonEngineAgros()->sceneViewPost2D()->doZoomBestFit();
    currentPythonEngineAgros()->sceneViewPost3D()->doZoomBestFit();
}

// ************************************************************************************

// solutionfilename()
char *pythonSolutionFileName()
{
    if (Agros2D::problem()->isSolved())
    {
        char *fileName = const_cast<char*>(QString(tempProblemFileName() + ".sln").toStdString().c_str());
        //Agros2D::scene()->sceneSolution()->sln()->save(fileName);
        return fileName;
    }
    else
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());
}
