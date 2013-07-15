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

void PythonEngineAgros::materialValues(const QString &function, double from, double to,
                                       QVector<double> *keys, QVector<double> *values, int count)
{
    if (function.isEmpty())
        return;

    // function
    ExpressionResult expressionResult = runExpression(function, false);
    if (!expressionResult.error.isEmpty())
        qDebug() << "Function: " << expressionResult.error;

    // prepare keys
    double step = (to - from) / (count - 1);
    QString keysVector = "[";
    for (int i = 0; i < count; i++)
    {
        double key = from + i * step;
        keys->append(key);

        if (i == 0)
            keysVector += QString("%1").arg(key + EPS_ZERO);
        else if (i == (count - 1))
            keysVector += QString(", %1]").arg(key - EPS_ZERO);
        else
            keysVector += QString(", %1").arg(key);
    }

    // run expression
    runExpression(QString("agros2d_material_values = agros2d_material_eval(%1)").arg(keysVector), false);

    // extract values
    PyObject *result = PyDict_GetItemString(m_dict, "agros2d_material_values");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < count; i++)
            values->append(PyFloat_AsDouble(PyList_GetItem(result, i)));
        Py_XDECREF(result);
    }

    // remove variables
    runExpression("del agros2d_material; del agros2d_material_values", false);

    // error during execution
    if (keys->size() != values->size())
    {
        keys->clear();
        values->clear();
    }
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

    if (Agros2D::problem()->isHarmonic())
        str += QString("problem.frequency = %1\n").
                arg(Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble());

    if (Agros2D::problem()->isTransient())
    {
        str += QString("problem.time_step_method = \"%1\"\n"
                       "problem.time_method_order = %2\n"
                       "problem.time_total = %3\n").
                arg(timeStepMethodToStringKey((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt())).
                arg(Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt()).
                arg(Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble());

        if (((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFTolerance)
        {
            str += QString("problem.time_method_tolerance = %3\n").
                    arg(Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
        }
        else
        {
            str += QString("problem.time_steps = %5\n").
                    arg(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
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
        str += QString("%1.matrix_solver = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(matrixSolverTypeToStringKey(fieldInfo->matrixSolver()));

        if (fieldInfo->matrixSolver() == Hermes::SOLVER_PARALUTION_ITERATIVE)
        {
            str += QString("%1.matrix_iterative_solver_method = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(iterLinearSolverMethodToStringKey((Hermes::Solvers::IterativeParalutionLinearMatrixSolver<double>::ParalutionSolverType) fieldInfo->value(FieldInfo::LinearSolverIterMethod).toInt()));
            str += QString("%1.matrix_iterative_solver_preconditioner = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(iterLinearSolverPreconditionerTypeToStringKey((Hermes::Solvers::ParalutionPrecond<double>::ParalutionPreconditionerType) fieldInfo->value(FieldInfo::LinearSolverIterPreconditioner).toInt()));
            str += QString("%1.matrix_iterative_solver_tolerance = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble());
            str += QString("%1.matrix_iterative_solver_iterations = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());
        }

        if (Agros2D::problem()->isTransient())
        {
            if (fieldInfo->analysisType() == analysisTypeFromStringKey("transient"))
            {
                str += QString("%1.transient_initial_condition = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble());
            }
            else
            {
                str += QString("%1.transient_time_skip = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::TransientTimeSkip).toInt());
            }
        }

        str += QString("%1.number_of_refinements = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());

        str += QString("%1.polynomial_order = %2\n").
                arg(fieldInfo->fieldId()).
                arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());

        str += QString("%1.adaptivity_type = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(adaptivityTypeToStringKey(fieldInfo->adaptivityType()));

        if (fieldInfo->adaptivityType() != AdaptivityType_None)
        {
            str += QString("%1.adaptivity_steps = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());

            str += QString("%1.adaptivity_tolerance = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble());

            str += QString("%1.adaptivity_threshold = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::AdaptivityThreshold).toDouble());

            str += QString("%1.adaptivity_stopping_criterion = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(adaptivityStoppingCriterionTypeToStringKey((AdaptivityStoppingCriterionType) fieldInfo->value(FieldInfo::AdaptivityStoppingCriterion).toInt()));

            str += QString("%1.adaptivity_norm_type = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(adaptivityNormTypeToStringKey((Hermes::Hermes2D::NormType) fieldInfo->value(FieldInfo::AdaptivityProjNormType).toInt()));

            str += QString("%1.adaptivity_anisotropic_refinement= %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->value(FieldInfo::AdaptivityUseAniso).toBool()) ? "True" : "False");

            str += QString("%1.adaptivity_finer_reference= %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->value(FieldInfo::AdaptivityFinerReference).toBool()) ? "True" : "False");

            if (Agros2D::problem()->isTransient())
            {
                str += QString("%1.adaptivity_back_steps = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityTransientBackSteps).toInt());

                str += QString("%1.adaptivity_redone_steps = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::AdaptivityTransientRedoneEach).toInt());
            }
        }

        str += QString("%1.solver = \"%2\"\n").
                arg(fieldInfo->fieldId()).
                arg(linearityTypeToStringKey(fieldInfo->linearityType()));

        if (fieldInfo->linearityType() != LinearityType_Linear)
        {
            str += QString("%1.nonlinear_tolerance = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NonlinearTolerance).toDouble());

            str += QString("%1.nonlinear_steps = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NonlinearSteps).toInt());
        }

        // newton
        if (fieldInfo->linearityType() == LinearityType_Newton)
        {
            str += QString("%1.nonlinear_convergence_measurement = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(nonlinearSolverConvergenceMeasurementToStringKey((Hermes::Hermes2D::NewtonSolverConvergenceMeasurementType) fieldInfo->value(FieldInfo::NonlinearConvergenceMeasurement).toInt()));

            str += QString("%1.newton_damping_type = \"%2\"\n").
                    arg(fieldInfo->fieldId()).
                    arg(dampingTypeToStringKey((DampingType)fieldInfo->value(FieldInfo::NewtonDampingType).toInt()));

            str += QString("%1.newton_damping_factor = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NewtonDampingCoeff).toDouble());

            str += QString("%1.newton_jacobian_reuse = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->value(FieldInfo::NewtonReuseJacobian).toBool()) ? "True" : "False");

            str += QString("%1.newton_jacobian_reuse_ratio = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NewtonSufImprovForJacobianReuse).toDouble());

            str += QString("%1.newton_damping_decrease_ratio = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NewtonSufImprov).toDouble());

            str += QString("%1.newton_jacobian_reuse_steps = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt());

            str += QString("%1.newton_damping_increase_steps = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg(fieldInfo->value(FieldInfo::NewtonStepsToIncreaseDF).toInt());
        }

        // picard
        if (fieldInfo->linearityType() == LinearityType_Picard)
        {
            str += QString("%1.picard_anderson_acceleration = %2\n").
                    arg(fieldInfo->fieldId()).
                    arg((fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool()) ? "True" : "False");

            if (fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool())
            {
                str += QString("%1.picard_anderson_beta = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble());

                str += QString("%1.picard_anderson_last_vectors = %2\n").
                        arg(fieldInfo->fieldId()).
                        arg(fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt());
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

                if (!value.isNumber() && startupScript == StartupScript_Variable)
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
                    if (!value.isNumber())
                        variables += QString("\"%1\" : { \"expression\" : \"%2\", \"x\" : [%3], \"y\" : [%4] }, ").
                                arg(variable.id()).
                                arg(value.text()).
                                arg(value.table().toStringX()).
                                arg(value.table().toStringY());
                    else
                        variables += QString("\"%1\" : { \"value\" : %2, \"x\" : [%3], \"y\" : [%4], \"interpolation\" : \"%5\", \"extrapolation\" : \"%6\", \"derivative_at_endpoints\" : \"%7\" }, ").
                                arg(variable.id()).
                                arg(value.number()).
                                arg(value.table().toStringX()).
                                arg(value.table().toStringY()).
                                arg(dataTableTypeToStringKey(value.table().type())).
                                arg(value.table().extrapolateConstant() == true ? "constant" : "linear").
                                arg(value.table().splineFirstDerivatives() == true ? "first" : "second");
                }
                else if (!value.isNumber() && startupScript == StartupScript_Variable)
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
                        refinements += QString("\"%1\" : %2, ").
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
                    if (fieldInfo->labelPolynomialOrder(label) != fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt())
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

void saveFile(const std::string &file, bool saveWithSolution)
{
    try
    {
        Agros2D::scene()->writeToFile(QString::fromStdString(file), false);

        if (saveWithSolution || silentMode())
            Agros2D::scene()->writeSolutionToFile(QString::fromStdString(file));
    }
    catch (AgrosException &e)
    {
        throw logic_error(e.toString().toStdString());
    }
}

