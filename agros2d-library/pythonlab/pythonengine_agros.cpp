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
#include "../resources_source/python/agros2d.cpp"
#include "logview.h"
#include "hermes2d/plugin_interface.h"
#include "hermes2d/module.h"
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
    if (!Agros2D::problem()->setting()->value(ProblemSetting::Problem_StartupScript).toString().trimmed().isEmpty())
        script += Agros2D::problem()->setting()->value(ProblemSetting::Problem_StartupScript).toString() + "\n";

    // run script
    if (!script.trimmed().isEmpty())
        PyRun_String(script.toLatin1().data(), Py_file_input, m_dict, m_dict);
}

PythonLabAgros::PythonLabAgros(PythonEngine *pythonEngine, QStringList args, QWidget *parent)
    : PythonEditorDialog(pythonEngine, args, parent)
{
    QSettings settings;

    // add create from model
    actCreateFromModel = new QAction(icon("script-create"), tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreatePythonFromModel()));

    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);

    tlbTools->addSeparator();
    tlbTools->addAction(actCreateFromModel);

    // startup script options
    actStartupScriptVariables = new QAction(tr("Variables"), this);
    actStartupScriptVariables->setCheckable(true);
    actStartupScriptVariables->setChecked(settings.value("PythonEditorDialog/StartupScriptVariables", true).toBool());

    actStartupScriptValues = new QAction(tr("Values"), this);
    actStartupScriptValues->setCheckable(true);
    actStartupScriptValues->setChecked(settings.value("PythonEditorDialog/StartupScriptValues", false).toBool());

    // console output
    actConsoleOutput = new QAction(tr("Console output"), this);
    actConsoleOutput->setCheckable(true);
    actConsoleOutput->setChecked(settings.value("PythonEditorDialog/ConsoleOutput", true).toBool());

    QActionGroup *actStartupScriptGroup = new QActionGroup(this);
    actStartupScriptGroup->addAction(actStartupScriptVariables);
    actStartupScriptGroup->addAction(actStartupScriptValues);

    QMenu *mnuStartupScript = new QMenu(tr("Startup script"), this);
    mnuStartupScript->addAction(actStartupScriptVariables);
    mnuStartupScript->addAction(actStartupScriptValues);

    mnuOptions->addAction(actConsoleOutput);
    mnuOptions->addSeparator();
    mnuOptions->addMenu(mnuStartupScript);
}

PythonLabAgros::~PythonLabAgros()
{
    QSettings settings;
    settings.setValue("PythonEditorDialog/StartupScriptVariables", actStartupScriptVariables->isChecked());
    settings.setValue("PythonEditorDialog/StartupScriptValues", actStartupScriptValues->isChecked());
    settings.setValue("PythonEditorDialog/ConsoleOutput", actConsoleOutput->isChecked());
}

void PythonLabAgros::doCreatePythonFromModel()
{
    StartupScript_Type type = actStartupScriptVariables->isChecked() ? StartupScript_Variable : StartupScript_Value;
    txtEditor->setPlainText(createPythonFromModel(type));
}

void PythonLabAgros::scriptPrepare()
{
    if (actConsoleOutput->isChecked())
    {
        connect(Agros2D::log(), SIGNAL(messageMsg(QString, QString, bool)), this, SLOT(printMessage(QString, QString, bool)));
        connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString, bool)), this, SLOT(printError(QString, QString, bool)));
        connect(Agros2D::log(), SIGNAL(warningMsg(QString, QString, bool)), this, SLOT(printWarning(QString, QString, bool)));
        connect(Agros2D::log(), SIGNAL(debugMsg(QString, QString, bool)), this, SLOT(printDebug(QString, QString, bool)));
    }
}

void PythonLabAgros::scriptFinish()
{
    if (actConsoleOutput->isChecked())
    {
        disconnect(Agros2D::log(), SIGNAL(messageMsg(QString, QString, bool)), this, SLOT(printMessage(QString, QString, bool)));
        disconnect(Agros2D::log(), SIGNAL(errorMsg(QString, QString, bool)), this, SLOT(printError(QString, QString, bool)));
        disconnect(Agros2D::log(), SIGNAL(warningMsg(QString, QString, bool)), this, SLOT(printWarning(QString, QString, bool)));
        disconnect(Agros2D::log(), SIGNAL(debugMsg(QString, QString, bool)), this, SLOT(printDebug(QString, QString, bool)));
    }
}

void PythonLabAgros::printMessage(const QString &module, const QString &message, bool escaped)
{
    consoleView->console()->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::gray);
}

void PythonLabAgros::printError(const QString &module, const QString &message, bool escaped)
{
    consoleView->console()->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::red);
}

void PythonLabAgros::printWarning(const QString &module, const QString &message, bool escaped)
{
    consoleView->console()->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::green);
}

void PythonLabAgros::printDebug(const QString &module, const QString &message, bool escaped)
{
#ifndef QT_NO_DEBUG_OUTPUT
    consoleView->console()->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::lightGray);
#endif
}

// *****************************************************************************

// create script from model
QString createPythonFromModel(StartupScript_Type startupScript)
{
    QString str;

    // import modules
    str += "import agros2d as a2d\n\n";

    // startup script
    if (!Agros2D::problem()->setting()->value(ProblemSetting::Problem_StartupScript).toString().trimmed().isEmpty())
    {
        str += "# startup script\n";
        str += Agros2D::problem()->setting()->value(ProblemSetting::Problem_StartupScript).toString();
        str += "\n\n";
    }

    // model
    str += "# problem\n";
    str += QString("problem = a2d.problem(clear = True)\n");
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
                arg(Agros2D::problem()->config()->timeTotal());

        if (Agros2D::problem()->config()->timeStepMethod() == TimeStepMethod_BDFTolerance)
        {
            str += QString("problem.time_method_tolerance = %3\n").
                    arg(Agros2D::problem()->config()->timeMethodTolerance());
        }
        else
        {
            str += QString("problem.time_steps = %5\n").
                    arg(Agros2D::problem()->config()->timeNumConstantTimeSteps());
        }
    }

    // fields
    str += "\n# fields\n";
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        str += QString("# %1\n").arg(fieldInfo->fieldId());

        // str += QString("%1 = a2d.field(field_id = \"%2\")\n").
        str += QString("%1 = a2d.field(\"%2\")\n").
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
                        arg(fieldInfo->initialCondition());
            }
            else
            {
                str += QString("%1.time_skip = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->timeSkip());
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
                str += QString("%1.automatic_damping_coeff = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->newtonAutomaticDampingCoeff());
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

                if (value.hasExpression() && startupScript == StartupScript_Variable)
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
                else if (value.hasExpression() && startupScript == StartupScript_Variable)
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
    str += "geometry = a2d.geometry\n";

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
    str += "a2d.view.zoom_best_fit()";

    return str;
}

// ************************************************************************************

void openFile(const std::string &file)
{
    try
    {
        Agros2D::scene()->readFromFile(QString::fromStdString(file));
    }
    catch (AgrosException &e)
    {
        throw logic_error(e.toString().toStdString());
    }
}

void saveFile(const std::string &file)
{
    try
    {
        Agros2D::scene()->writeToFile(QString::fromStdString(file), false);
    }
    catch (AgrosException &e)
    {
        throw logic_error(e.toString().toStdString());
    }
}
