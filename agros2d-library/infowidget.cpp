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

#include "infowidget.h"

#include "util/constants.h"
#include "util/global.h"

#include "gui/common.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "pythonlab/pythonengine_agros.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "ctemplate/template.h"

#include "hermes2d.h"

InfoWidget::InfoWidget(SceneViewPreprocessor *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewGeometry = sceneView;

    // problem information
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(networkAccessManager());
    webView->setMinimumSize(200, 200);

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(2, 2, 2, 2);
    layoutMain->addWidget(webView);

    setLayout(layoutMain);

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(refresh()));

    connect(Agros2D::problem(), SIGNAL(timeStepChanged()), this, SLOT(refresh()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(refresh()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(refresh()));
    connect(Agros2D::problem(), SIGNAL(couplingsChanged()), this, SLOT(refresh()));

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(refresh()));
    connect(currentPythonEngineAgros(), SIGNAL(executedExpression()), this, SLOT(refresh()));

    refresh();
}

InfoWidget::~InfoWidget()
{
    QFile::remove(tempProblemDir() + "/info.html");
}

void InfoWidget::refresh()
{
    // QTimer::singleShot(0, this, SLOT(showInfo()));
    showInfo();
}

void InfoWidget::showInfo()
{
    // template
    std::string info;
    ctemplate::TemplateDictionary problemInfo("info");

    problemInfo.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

    problemInfo.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    problemInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());

    problemInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    problemInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
    problemInfo.SetValue("NAME", QFileInfo(Agros2D::problem()->config()->fileName()).baseName().toStdString());

    problemInfo.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
    problemInfo.SetValue("COORDINATE_TYPE", coordinateTypeString(Agros2D::problem()->config()->coordinateType()).toStdString());

    problemInfo.SetValue("MESH_TYPE_LABEL", tr("Mesh type:").toStdString());
    problemInfo.SetValue("MESH_TYPE", meshTypeString(Agros2D::problem()->config()->meshType()).toStdString());

    if (Agros2D::problem()->isHarmonic())
        problemInfo.ShowSection("HARMONIC");

    problemInfo.SetValue("HARMONIC_LABEL", tr("Harmonic analysis").toStdString());
    problemInfo.SetValue("HARMONIC_FREQUENCY_LABEL", tr("Frequency:").toStdString());
    problemInfo.SetValue("HARMONIC_FREQUENCY", QString::number(Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble()).toStdString() + " Hz");

    if (Agros2D::problem()->isTransient())
    {
        problemInfo.ShowSection("TRANSIENT");
        if (Agros2D::problem()->config()->isTransientAdaptive())
        {
            problemInfo.ShowSection("TRANSIENT_ADAPTIVE");

            if (Agros2D::problem()->isSolved())
            {
                QString dataTimeSteps = "[";
                QList<double> lengths = Agros2D::problem()->timeStepLengths();
                double time = 0;
                for (int i = 0; i < lengths.size(); i++)
                {
                    dataTimeSteps += QString("[%1, %2], ").arg(time).arg(lengths.at(i));
                    time += lengths.at(i);
                }
                dataTimeSteps += "]";

                // chart time step vs. steps
                QString timeSteps = QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_time_step_length\"), [ { data: %1, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: true } } ], { grid: { hoverable : true }, xaxes: [ { axisLabel: 'Time (s)' } ], yaxes: [ { axisLabel: 'Time step (s)' } ] });});</script>").
                        arg(dataTimeSteps);

                problemInfo.SetValue("TIME_STEPS_CHART", timeSteps.toStdString());
            }
        }
    }
    problemInfo.SetValue("TRANSIENT_LABEL", tr("Transient analysis").toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_METHOD_LABEL", tr("Method:").toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_METHOD", timeStepMethodString((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()).toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_ORDER_LABEL", tr("Order:").toStdString());
    problemInfo.SetValue("TRANSIENT_STEP_ORDER", QString::number(Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt()).toStdString());
    problemInfo.SetValue("TRANSIENT_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
    problemInfo.SetValue("TRANSIENT_TOLERANCE", QString::number(Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble()).toStdString());
    problemInfo.SetValue("TRANSIENT_CONSTANT_STEP_LABEL", tr("Constant time step:").toStdString());
    problemInfo.SetValue("TRANSIENT_CONSTANT_STEP", QString::number(Agros2D::problem()->config()->constantTimeStepLength()).toStdString() + " s");
    problemInfo.SetValue("TRANSIENT_CONSTANT_NUM_STEPS_LABEL", tr("Number of const. time steps:").toStdString());
    problemInfo.SetValue("TRANSIENT_CONSTANT_NUM_STEPS", QString::number(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).toStdString());
    problemInfo.SetValue("TRANSIENT_TOTAL_LABEL", tr("Total time:").toStdString());
    problemInfo.SetValue("TRANSIENT_TOTAL", QString::number(Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble()).toStdString() + " s");

    problemInfo.SetValue("GEOMETRY_LABEL", tr("Geometry").toStdString());
    problemInfo.SetValue("GEOMETRY_NODES_LABEL", tr("Nodes:").toStdString());
    problemInfo.SetValue("GEOMETRY_NODES", QString::number(Agros2D::scene()->nodes->count()).toStdString());
    problemInfo.SetValue("GEOMETRY_EDGES_LABEL", tr("Edges:").toStdString());
    problemInfo.SetValue("GEOMETRY_EDGES", QString::number(Agros2D::scene()->edges->count()).toStdString());
    problemInfo.SetValue("GEOMETRY_LABELS_LABEL", tr("Labels:").toStdString());
    problemInfo.SetValue("GEOMETRY_LABELS", QString::number(Agros2D::scene()->labels->count()).toStdString());
    problemInfo.SetValue("GEOMETRY_MATERIALS_LABEL", tr("Materials:").toStdString());
    problemInfo.SetValue("GEOMETRY_MATERIALS", QString::number(Agros2D::scene()->materials->items().count() - 1).toStdString());
    problemInfo.SetValue("GEOMETRY_BOUNDARIES_LABEL", tr("Boundaries:").toStdString());
    problemInfo.SetValue("GEOMETRY_BOUNDARIES", QString::number(Agros2D::scene()->boundaries->items().count() - 1).toStdString());
    problemInfo.SetValue("GEOMETRY_SVG", generateSvgGeometry(Agros2D::scene()->edges->items()).toStdString());

    if (Agros2D::problem()->fieldInfos().count() > 0)
    {
        problemInfo.SetValue("PHYSICAL_FIELD_MAIN_LABEL", tr("Physical fields").toStdString());

        foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
        {
            ctemplate::TemplateDictionary *field = problemInfo.AddSectionDictionary("FIELD_SECTION");

            field->SetValue("PHYSICAL_FIELD_LABEL", fieldInfo->name().toStdString());
            field->SetValue("PHYSICAL_FIELD_ID", fieldInfo->fieldId().toStdString());

            field->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
            field->SetValue("ANALYSIS_TYPE", analysisTypeString(fieldInfo->analysisType()).toStdString());

            if (fieldInfo->analysisType() == AnalysisType_Transient)
            {
                field->SetValue("INITIAL_CONDITION_LABEL", tr("Initial condition:").toStdString());
                field->SetValue("INITIAL_CONDITION", QString::number(fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble()).toStdString());
                field->ShowSection("INITIAL_CONDITION_SECTION");
            }

            field->SetValue("REFINEMENS_NUMBER_LABEL", tr("Number of refinements:").toStdString());
            field->SetValue("REFINEMENS_NUMBER", QString::number(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt()).toStdString());
            field->SetValue("POLYNOMIAL_ORDER_LABEL", tr("Polynomial order:").toStdString());
            field->SetValue("POLYNOMIAL_ORDER", QString::number(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()).toStdString());

            field->SetValue("ADAPTIVITY_TYPE_LABEL", tr("Adaptivity:").toStdString());
            field->SetValue("ADAPTIVITY_TYPE", adaptivityTypeString(fieldInfo->adaptivityType()).toStdString());

            if (fieldInfo->adaptivityType() != AdaptivityType_None)
            {
                field->SetValue("ADAPTIVITY_STEPS_LABEL", tr("Steps:").toStdString());
                field->SetValue("ADAPTIVITY_STEPS", QString::number(fieldInfo->value(FieldInfo::AdaptivitySteps).toInt()).toStdString());
                field->SetValue("ADAPTIVITY_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
                field->SetValue("ADAPTIVITY_TOLERANCE", QString::number(fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble()).toStdString() + " %");

                field->ShowSection("ADAPTIVITY_PARAMETERS_SECTION");
            }

            field->SetValue("LINEARITY_TYPE_LABEL", tr("Solver:").toStdString());
            field->SetValue("LINEARITY_TYPE", linearityTypeString(fieldInfo->linearityType()).toStdString());

            if (fieldInfo->linearityType() != LinearityType_Linear)
            {
                field->SetValue("NONLINEAR_STEPS_LABEL", tr("Steps:").toStdString());
                field->SetValue("NONLINEAR_STEPS", QString::number(fieldInfo->value(FieldInfo::NonlinearSteps).toInt()).toStdString());
                field->SetValue("NONLINEAR_CONVERGENCE_MEASUREMENT_LABEL", tr("Convergence:").toStdString());
                field->SetValue("NONLINEAR_CONVERGENCE_MEASUREMENT", nonlinearSolverConvergenceMeasurementString((Hermes::Hermes2D::NewtonSolverConvergenceMeasurementType) fieldInfo->value(FieldInfo::NonlinearConvergenceMeasurement).toInt()).toStdString());
                field->SetValue("NONLINEAR_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
                field->SetValue("NONLINEAR_TOLERANCE", QString::number(fieldInfo->value(FieldInfo::NonlinearTolerance).toDouble()).toStdString());

                field->SetValue("NONLINEAR_NEWTON_SUFFICIENT_IMPROVEMENT_FACTOR_JACOBIAN_LABEL", tr("Sufficient improvement factor Jacobian:").toStdString());
                //field->SetValue("NONLINEAR_NEWTON_SUFFICIENT_IMPROVEMENT_FACTOR_JACOBIAN", QString::number(fieldInfo->value(FieldInfo::NewtonSufficientImprovementFactorJacobian).toDouble()).toStdString());
                field->SetValue("NONLINEAR_NEWTON_MAXIMUM_STEPS_WITH_REUSED_JACOBIAN_LABEL", tr("Maximum steps with reused Jacobian:").toStdString());
                //field->SetValue("NONLINEAR_NEWTON_MAXIMUM_STEPS_WITH_REUSED_JACOBIAN", QString::number(fieldInfo->value(FieldInfo::NewtonMaximumStepsWithReusedJacobian).toInt()).toStdString());

                // todo: damping coeff in info

                //                if (fieldInfo->value(FieldInfo::NewtonAutomaticDamping).toBool())
                //                {
                //                    field->SetValue("NONLINEAR_AUTOMATIC_DAMPING_LABEL", tr("Automatic damping:").toStdString());
                //                    field->SetValue("NONLINEAR_AUTOMATIC_DAMPING_FACTOR_LABEL", tr("Maximum damping factor:").toStdString());
                //                    //field->SetValue("NONLINEAR_AUTOMATIC_DAMPING_FACTOR", QString::number(fieldInfo->value(FieldInfo::NewtonAutomaticDampingCoeff).toDouble()).toStdString());
                //                    field->SetValue("NONLINEAR_AUTOMATIC_DAMPING_INCREASE_LABEL", tr("Steps to increase:").toStdString());
                //                    //field->SetValue("NONLINEAR_AUTOMATIC_DAMPING_INCREASE", QString::number(fieldInfo->value(FieldInfo::NewtonDampingNumberToIncrease).toInt()).toStdString());

                //                    field->ShowSection("SOLVER_NONLINEAR_NEWTON_AUTOMATIC_DAMPING_SECTION");
                //                }
                //                else
                //                {
                //                    field->SetValue("NONLINEAR_NONAUTOMATIC_DAMPING_FACTOR_LABEL", tr("Maximum damping factor:").toStdString());
                //                    field->SetValue("NONLINEAR_NONAUTOMATIC_DAMPING_FACTOR", QString::number(fieldInfo->value(FieldInfo::NewtonDampingCoeff).toDouble()).toStdString());

                //                    field->ShowSection("SOLVER_NONLINEAR_NEWTON_NONAUTOMATIC_DAMPING_SECTION");
                //                }



                field->ShowSection("SOLVER_PARAMETERS_SECTION");
            }

            QString matrixSolver = matrixSolverTypeString(fieldInfo->matrixSolver());
            if ((fieldInfo->matrixSolver() == Hermes::SOLVER_PARALUTION))
                matrixSolver += tr(" (%1, %2) - iterative").
                        arg(iterLinearSolverMethodString((Hermes::Solvers::ParalutionLinearMatrixSolver<double>::ParalutionSolverType) fieldInfo->value(FieldInfo::LinearSolverIterMethod).toInt())).
                        arg(iterLinearSolverPreconditionerTypeString((Hermes::Solvers::ParalutionPrecond<double>::ParalutionPreconditionerType) fieldInfo->value(FieldInfo::LinearSolverIterPreconditioner).toInt()));
            else
                matrixSolver += tr(" - direct");

            field->SetValue("LINEARSOLVER_TYPE_LABEL", tr("Linear solver:").toStdString());
            field->SetValue("LINEARSOLVER_TYPE", matrixSolver.toStdString());

            int solutionMeshNodes = 0;
            int solutionMeshElements = 0;
            int DOFs = 0;
            int error = 0;

            if (Agros2D::problem()->isSolved())
            {
                int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo, SolutionMode_Normal);
                int adaptiveStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal);
                MultiArray<double> msa = Agros2D::solutionStore()->multiArray(FieldSolutionID(fieldInfo, timeStep, adaptiveStep, SolutionMode_Normal));

                solutionMeshNodes = msa.solutions().at(0)->get_mesh()->get_num_vertex_nodes();
                solutionMeshElements = msa.solutions().at(0)->get_mesh()->get_num_active_elements();
                DOFs = Hermes::Hermes2D::Space<double>::get_num_dofs(msa.spaces());
            }

            if (Agros2D::problem()->isMeshed())
            {
                field->SetValue("MESH_LABEL", tr("Mesh parameters").toStdString());
                field->SetValue("INITIAL_MESH_LABEL", tr("Initial mesh:").toStdString());
                field->SetValue("INITIAL_MESH_NODES", tr("%1 nodes").arg(fieldInfo->initialMesh()->get_num_vertex_nodes()).toStdString());
                field->SetValue("INITIAL_MESH_ELEMENTS", tr("%1 elements").arg(fieldInfo->initialMesh()->get_num_active_elements()).toStdString());

                if (Agros2D::problem()->isSolved() && (fieldInfo->linearityType() == LinearityType_Newton))
                {
                    field->ShowSection("MESH_SOLUTION_NEWTON_SECTION");

                    int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo, SolutionMode_Normal);
                    int adaptiveStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal);

                    SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(fieldInfo, timeStep, adaptiveStep, SolutionMode_Normal));
                    QString newtonResiduals = "[";
                    for (int i = 0; i < runTime.newtonResidual().size(); i++)
                        newtonResiduals += QString("[%1, %2], ").arg(i+1).arg(runTime.newtonResidual().at(i));
                    newtonResiduals += "]";

                    // chart newton residual vs. steps
                    QString commandResidual = QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_newton_residuals_%1\"), [ { data: %2, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: true } } ], { grid: { hoverable : true }, xaxes: [ { axisLabel: 'steps (-)' } ], yaxes: [ { axisLabel: 'Newton residual (-)' } ] });});</script>").
                            arg(fieldInfo->fieldId()).
                            arg(newtonResiduals);

                    field->SetValue("NEWTON_RESIDUAL_CHART", commandResidual.toStdString());
                }

                if (Agros2D::problem()->isSolved() && (fieldInfo->adaptivityType() != AdaptivityType_None))
                {
                    field->SetValue("SOLUTION_MESH_LABEL", tr("Solution mesh:").toStdString());
                    field->SetValue("SOLUTION_MESH_NODES", tr("%1 nodes").arg(solutionMeshNodes).toStdString());
                    field->SetValue("SOLUTION_MESH_ELEMENTS", tr("%1 elements").arg(solutionMeshElements).toStdString());
                    field->ShowSection("MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION");

                    int timeStep = Agros2D::solutionStore()->timeLevels(fieldInfo).count() - 1;
                    int adaptiveSteps = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal);
                    error = Agros2D::solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(fieldInfo, timeStep, adaptiveSteps, SolutionMode_Normal)).adaptivityError();

                    QString dataDOFs = "[";
                    QString dataError = "[";
                    for (int i = 0; i <= adaptiveSteps; i++)
                    {
                        SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(fieldInfo, timeStep, i, SolutionMode_Normal));

                        // qDebug() << structure.adaptivity_error;

                        dataDOFs += QString("[%1, %2], ").arg(i+1).arg(runTime.DOFs());
                        dataError += QString("[%1, %2], ").arg(i+1).arg(runTime.adaptivityError());
                    }
                    dataDOFs += "]";
                    dataError += "]";

                    // error
                    QString prescribedError = QString("[[1, %1], [%2, %3]]").
                            arg(fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble()).
                            arg(adaptiveSteps + 1).
                            arg(fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble());

                    // chart error vs. steps
                    QString commandError = QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_error_steps_%1\"), [ { data: %2, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: true } }, { data: %3, color: \"rgb(240, 0, 0)\" } ], { grid: { hoverable : true }, xaxes: [ { axisLabel: 'steps (-)' } ], yaxes: [ { axisLabel: 'Rel. error (%)' } ] });});</script>").
                            arg(fieldInfo->fieldId()).
                            arg(dataError).
                            arg(prescribedError);

                    // chart DOFs vs. steps
                    QString commandDOFs = QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_dofs_steps_%1\"), [ { data: %2, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: true } } ], { grid: { hoverable : true }, xaxes: [ { axisLabel: 'steps (-)' } ], yaxes: [ { axisLabel: 'Num. of DOFs (-)' } ] });});</script>").
                            arg(fieldInfo->fieldId()).
                            arg(dataDOFs);

                    field->SetValue("ERROR_STEPS_CHART", commandError.toStdString());
                    field->SetValue("DOFS_STEPS_CHART", commandDOFs.toStdString());
                }

                if (Agros2D::problem()->isSolved())
                {
                    field->SetValue("DOFS_LABEL", tr("Number of DOFs:").toStdString());
                    field->SetValue("DOFS", QString("%1").arg(DOFs).toStdString());
                    field->SetValue("ERROR_LABEL", tr("Error:").toStdString());
                    field->SetValue("ERROR", QString("%1 %").arg(error).toStdString());
                    field->ShowSection("MESH_SOLUTION_DOFS_PARAMETERS_SECTION");
                }

                field->ShowSection("MESH_PARAMETERS_SECTION");
            }
        }

        problemInfo.ShowSection("FIELD");
    }

    if (Agros2D::problem()->couplingInfos().count() > 0)
    {
        problemInfo.SetValue("COUPLING_MAIN_LABEL", tr("Coupled fields").toStdString());

        foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos())
        {
            ctemplate::TemplateDictionary *couplingSection = problemInfo.AddSectionDictionary("COUPLING_SECTION");

            couplingSection->SetValue("COUPLING_LABEL", couplingInfo->name().toStdString());

            couplingSection->SetValue("COUPLING_SOURCE_LABEL", tr("Source:").toStdString());
            couplingSection->SetValue("COUPLING_SOURCE", couplingInfo->sourceField()->name().toStdString());
            couplingSection->SetValue("COUPLING_TARGET_LABEL", tr("Target:").toStdString());
            couplingSection->SetValue("COUPLING_TARGET", couplingInfo->targetField()->name().toStdString());
            couplingSection->SetValue("COUPLING_TYPE_LABEL", tr("Coupling type:").toStdString());
            couplingSection->SetValue("COUPLING_TYPE", QString("%1").arg(couplingTypeString(couplingInfo->couplingType())).toStdString());
        }
        problemInfo.ShowSection("COUPLING");
    }

    if (Agros2D::problem()->isSolved())
    {
        problemInfo.SetValue("SOLUTION_LABEL", tr("Solution").toStdString());
        problemInfo.SetValue("SOLUTION_ELAPSED_TIME_LABEL", tr("Total elapsed time:").toStdString());
        problemInfo.SetValue("SOLUTION_ELAPSED_TIME", tr("%1 s").arg(Agros2D::problem()->timeElapsed().toString("mm:ss.zzz")).toStdString());
        // problemInfo.SetValue("NUM_THREADS_LABEL", tr("Number of threads:").toStdString());
        // problemInfo.SetValue("NUM_THREADS", QString("%1").arg(Hermes::Hermes2D::Hermes2DApi.get_integral_param_value(Hermes::numThreads)).toStdString());
        problemInfo.ShowSection("SOLUTION_PARAMETERS_SECTION");
    }

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/problem.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &problemInfo, &info);

    // setHtml(...) doesn't work
    // webView->setHtml(QString::fromStdString(info));

    // load(...) works
    writeStringContent(tempProblemDir() + "/info.html", QString::fromStdString(info));
    webView->load(QUrl::fromLocalFile(tempProblemDir() + "/info.html"));
}
