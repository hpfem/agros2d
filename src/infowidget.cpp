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

#include "scene.h"
#include "gui.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "pythonlabagros.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"
#include "ctemplate/template.h"

InfoWidget::InfoWidget(SceneViewPreprocessor *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewGeometry = sceneView;

    // problem information
    webView = new QWebView(this);

    // dialog buttons
    btnAdaptiveError = new QPushButton(tr("Adaptive error"));
    connect(btnAdaptiveError, SIGNAL(clicked()), SLOT(doAdaptiveError()));
    btnDOFs = new QPushButton(tr("Number of DOFs"));
    connect(btnDOFs, SIGNAL(clicked()), SLOT(doAdaptiveDOFs()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnAdaptiveError);
    layoutButtons->addWidget(btnDOFs);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    // layoutMain->setContentsMargins(0, 5, 3, 5);
    layoutMain->addWidget(webView);
    layoutMain->addLayout(layoutButtons);

    setLayout(layoutMain);

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(refresh()));
    connect(currentPythonEngineAgros(), SIGNAL(executedExpression()), this, SLOT(refresh()));

    refresh();
}

InfoWidget::~InfoWidget()
{
}

void InfoWidget::refresh()
{
    btnAdaptiveError->setEnabled(Util::problem()->isSolved());
    btnDOFs->setEnabled(Util::problem()->isSolved());

    QTimer::singleShot(0, this, SLOT(showInfo()));
}

void InfoWidget::showInfo()
{
    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize() - 1).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/problem_style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string info;
    ctemplate::TemplateDictionary problemInfo("info");

    problemInfo.SetValue("STYLESHEET", style);
    problemInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    problemInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
    problemInfo.SetValue("NAME", Util::problem()->config()->name().toStdString());

    problemInfo.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
    problemInfo.SetValue("COORDINATE_TYPE", coordinateTypeString(Util::problem()->config()->coordinateType()).toStdString());

    problemInfo.SetValue("MESH_TYPE_LABEL", tr("Mesh type:").toStdString());
    problemInfo.SetValue("MESH_TYPE", meshTypeString(Util::problem()->config()->meshType()).toStdString());

    if (Util::problem()->isHarmonic())
        problemInfo.ShowSection("HARMONIC");
    problemInfo.SetValue("FREQUENCY_LABEL", tr("Frequency:").toStdString());
    problemInfo.SetValue("FREQUENCY", QString::number(Util::problem()->config()->frequency()).toStdString() + " Hz");
    if (Util::problem()->isTransient())
        problemInfo.ShowSection("TRANSIENT");
    problemInfo.SetValue("TIME_STEP_LABEL", tr("Time step:").toStdString());
    problemInfo.SetValue("TIME_STEP", QString::number(Util::problem()->config()->timeStep().number()).toStdString() + " s");
    problemInfo.SetValue("TIME_TOTAL_LABEL", tr("Total time:").toStdString());
    problemInfo.SetValue("TIME_TOTAL", QString::number(Util::problem()->config()->timeTotal().number()).toStdString() + " s");

    problemInfo.SetValue("GEOMETRY_LABEL", tr("Geometry").toStdString());
    problemInfo.SetValue("GEOMETRY_NODES_LABEL", tr("Nodes:").toStdString());
    problemInfo.SetValue("GEOMETRY_NODES", QString::number(Util::scene()->nodes->count()).toStdString());
    problemInfo.SetValue("GEOMETRY_EDGES_LABEL", tr("Edges:").toStdString());
    problemInfo.SetValue("GEOMETRY_EDGES", QString::number(Util::scene()->edges->count()).toStdString());
    problemInfo.SetValue("GEOMETRY_LABELS_LABEL", tr("Labels:").toStdString());
    problemInfo.SetValue("GEOMETRY_LABELS", QString::number(Util::scene()->labels->count()).toStdString());
    problemInfo.SetValue("GEOMETRY_MATERIALS_LABEL", tr("Materials:").toStdString());
    problemInfo.SetValue("GEOMETRY_MATERIALS", QString::number(Util::scene()->materials->items().count()).toStdString());
    problemInfo.SetValue("GEOMETRY_BOUNDARIES_LABEL", tr("Boundaries:").toStdString());
    problemInfo.SetValue("GEOMETRY_BOUNDARIES", QString::number(Util::scene()->boundaries->items().count()).toStdString());
    problemInfo.SetValue("GEOMETRY_SVG", generateGeometry().toStdString());

    if (Util::problem()->fieldInfos().count() > 0)
    {
        problemInfo.SetValue("PHYSICAL_FIELD_MAIN_LABEL", tr("Physical fields").toStdString());

        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            ctemplate::TemplateDictionary *field = problemInfo.AddSectionDictionary("FIELD_SECTION");

            field->SetValue("PHYSICAL_FIELD_LABEL", fieldInfo->name().toStdString());

            field->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
            field->SetValue("ANALYSIS_TYPE", analysisTypeString(fieldInfo->analysisType()).toStdString());

            if (fieldInfo->analysisType() == AnalysisType_Transient)
            {
                field->SetValue("INITIAL_CONDITION_LABEL", tr("Initial condition:").toStdString());
                field->SetValue("INITIAL_CONDITION", QString::number(fieldInfo->initialCondition().number()).toStdString());
                field->ShowSection("INITIAL_CONDITION_SECTION");
            }

            field->SetValue("REFINEMENS_NUMBER_LABEL", tr("Number of refinements:").toStdString());
            field->SetValue("REFINEMENS_NUMBER", QString::number(fieldInfo->numberOfRefinements()).toStdString());
            field->SetValue("POLYNOMIAL_ORDER_LABEL", tr("Polynomial order:").toStdString());
            field->SetValue("POLYNOMIAL_ORDER", QString::number(fieldInfo->polynomialOrder()).toStdString());

            field->SetValue("ADAPTIVITY_TYPE_LABEL", tr("Adaptivity:").toStdString());
            field->SetValue("ADAPTIVITY_TYPE", adaptivityTypeString(fieldInfo->adaptivityType()).toStdString());

            if (fieldInfo->adaptivityType() != AdaptivityType_None)
            {
                field->SetValue("ADAPTIVITY_STEPS_LABEL", tr("Steps:").toStdString());
                field->SetValue("ADAPTIVITY_STEPS", QString::number(fieldInfo->adaptivitySteps()).toStdString());
                field->SetValue("ADAPTIVITY_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
                field->SetValue("ADAPTIVITY_TOLERANCE", QString::number(fieldInfo->adaptivityTolerance()).toStdString() + "%");
                field->ShowSection("ADAPTIVITY_PARAMETERS_SECTION");
            }

            field->SetValue("LINEARITY_TYPE_LABEL", tr("Solver:").toStdString());
            field->SetValue("LINEARITY_TYPE", linearityTypeString(fieldInfo->linearityType()).toStdString());

            if (fieldInfo->linearityType() != LinearityType_Linear)
            {
                field->SetValue("NONLINEAR_STEPS_LABEL", tr("Steps:").toStdString());
                field->SetValue("NONLINEAR_STEPS", QString::number(fieldInfo->nonlinearSteps()).toStdString());
                field->SetValue("NONLINEAR_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
                field->SetValue("NONLINEAR_TOLERANCE", QString::number(fieldInfo->nonlinearTolerance()).toStdString() + "%");
                field->ShowSection("SOLVER_PARAMETERS_SECTION");
            }

            int solutionMeshNodes = 0;
            int solutionMeshElements = 0;
            int DOFs = 0;
            int error = 0;

            if (Util::problem()->isSolved())
            {
                int timeStep = Util::solutionStore()->lastTimeStep(fieldInfo, SolutionMode_Normal);
                int adaptiveStep = Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal);
                MultiSolutionArray<double> msa = Util::solutionStore()->multiSolution(FieldSolutionID(fieldInfo, timeStep, adaptiveStep, SolutionMode_Normal));

                solutionMeshNodes = msa.solutions().at(0)->get_mesh()->get_num_nodes();
                solutionMeshElements = msa.solutions().at(0)->get_mesh()->get_num_active_elements();
                DOFs = Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(desmartize(msa.spaces())));
                error = msa.adaptiveError();
            }

            if (Util::problem()->isMeshed())
            {
                field->SetValue("MESH_LABEL", tr("Mesh parameters").toStdString());
                field->SetValue("INITIAL_MESH_LABEL", tr("Initial mesh:").toStdString());
                field->SetValue("INITIAL_MESH_NODES", tr("%1 nodes").arg(Util::problem()->meshInitial(fieldInfo)->get_num_nodes()).toStdString());
                field->SetValue("INITIAL_MESH_ELEMENTS", tr("%1 elements").arg(Util::problem()->meshInitial(fieldInfo)->get_num_active_elements()).toStdString());

                if (Util::problem()->isSolved() && (fieldInfo->adaptivityType() != AdaptivityType_None))
                {
                    field->SetValue("SOLUTION_MESH_LABEL", tr("Solution mesh:").toStdString());
                    field->SetValue("SOLUTION_MESH_NODES", tr("%1 nodes").arg(solutionMeshNodes).toStdString());
                    field->SetValue("SOLUTION_MESH_ELEMENTS", tr("%1 elements").arg(solutionMeshElements).toStdString());
                    field->ShowSection("MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION");
                }

                if (Util::problem()->isSolved())
                {
                    field->SetValue("DOFS_LABEL", tr("Number of DOFs:").toStdString());
                    field->SetValue("DOFS", tr("%1").arg(DOFs).toStdString());
                    field->SetValue("ERROR_LABEL", tr("Error:").toStdString());
                    field->SetValue("ERROR", tr("%1 %").arg(error).toStdString());
                    field->ShowSection("MESH_SOLUTION_DOFS_PARAMETERS_SECTION");
                }

                field->ShowSection("MESH_PARAMETERS_SECTION");
            }

            //        if (Util::problem()->isMeshed())
            //        {
            //            if (Util::problem()->isSolved())
            //            {
            //                if (fieldInfo->adaptivityType != AdaptivityType_None)
            //                {
            //                    problem.SetValue("ADAPTIVITY_LABEL", tr("Adaptivity").toStdString());
            //                    problem.SetValue("ADAPTIVITY_ERROR_LABEL", tr("Error:").toStdString());
            //                    problem.SetValue("ADAPTIVITY_ERROR", QString::number(Util::problem()->adaptiveError(), 'f', 3).toStdString());

            //                    problem.SetValue("SOLUTION_MESH_LABEL", tr("Solution mesh").toStdString());
            //                    problem.SetValue("SOLUTION_MESH_NODES_LABEL", tr("Nodes:").toStdString());
            //                    problem.SetValue("SOLUTION_MESH_NODES", QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_nodes()).toStdString());
            //                    problem.SetValue("SOLUTION_MESH_ELEMENTS_LABEL", tr("Elements:").toStdString());
            //                    problem.SetValue("SOLUTION_MESH_ELEMENTS", QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_active_elements()).toStdString());

            //                    problem.ShowSection("ADAPTIVITY_SECTION");
            //                }
            //                problem.ShowSection("SOLUTION_PARAMETERS_SECTION");
            //            }
            //        }
        }

        problemInfo.ShowSection("FIELD");
    }

    if (Util::problem()->couplingInfos().count() > 0)
    {
        problemInfo.SetValue("COUPLING_MAIN_LABEL", tr("Coupled fields").toStdString());

        foreach (CouplingInfo *couplingInfo, Util::problem()->couplingInfos())
        {
            ctemplate::TemplateDictionary *couplingSection = problemInfo.AddSectionDictionary("COUPLING_SECTION");

            couplingSection->SetValue("COUPLING_LABEL", couplingInfo->coupling()->name().toStdString());

            couplingSection->SetValue("COUPLING_SOURCE_LABEL", tr("Source:").toStdString());
            couplingSection->SetValue("COUPLING_SOURCE", couplingInfo->sourceField()->name().toStdString());
            couplingSection->SetValue("COUPLING_TARGET_LABEL", tr("Target:").toStdString());
            couplingSection->SetValue("COUPLING_TARGET", couplingInfo->targetField()->name().toStdString());
            couplingSection->SetValue("COUPLING_TYPE_LABEL", tr("Coupling type:").toStdString());
            couplingSection->SetValue("COUPLING_TYPE", tr("%1").arg(couplingTypeString(couplingInfo->couplingType())).toStdString());
        }
        problemInfo.ShowSection("COUPLING");
    }

    if (Util::problem()->isSolved())
    {
        problemInfo.SetValue("SOLUTION_LABEL", tr("Solution").toStdString());
        problemInfo.SetValue("SOLUTION_ELAPSED_TIME_LABEL", tr("Total elapsed time:").toStdString());
        problemInfo.SetValue("SOLUTION_ELAPSED_TIME", tr("%1 s").arg(Util::problem()->timeElapsed().toString("mm:ss.zzz")).toStdString());
        problemInfo.SetValue("NUM_THREADS_LABEL", tr("Number of threads:").toStdString());
        problemInfo.SetValue("NUM_THREADS", tr("%1").arg(Hermes::Hermes2D::Hermes2DApi.getParamValue(Hermes::Hermes2D::numThreads)).toStdString());
        problemInfo.ShowSection("SOLUTION_PARAMETERS_SECTION");
    }

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/problem.tpl", ctemplate::DO_NOT_STRIP, &problemInfo, &info);
    webView->setHtml(QString::fromStdString(info));

    // setFocus();
}

QString InfoWidget::generateGeometry()
{
    RectPoint boundingBox = Util::scene()->boundingBox();

    double size = 200;
    double stroke_width = max(boundingBox.width(), boundingBox.height()) / size / 2.0;

    // svg
    QString str;
    str += QString("<svg width=\"%1px\" height=\"%2px\" viewBox=\"%3 %4 %5 %6\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n").
            arg(size).
            arg(size).
            arg(boundingBox.start.x).
            arg(0).
            arg(boundingBox.width()).
            arg(boundingBox.height());

    str += QString("<g stroke=\"black\" stroke-width=\"%1\" fill=\"none\">\n").arg(stroke_width);

    foreach (SceneEdge *edge, Util::scene()->edges->items())
    {
        if (edge->angle() > 0.0)
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            int segments = edge->angle() / 5.0;
            if (segments < 2) segments = 2;
            double theta = edge->angle() / double(segments - 1);

            for (int i = 0; i < segments-1; i++)
            {
                double arc1 = (startAngle + i*theta)/180.0*M_PI;
                double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                double x1 = radius * cos(arc1);
                double y1 = radius * sin(arc1);
                double x2 = radius * cos(arc2);
                double y2 = radius * sin(arc2);

                str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                        arg(center.x + x1).
                        arg(boundingBox.end.y - (center.y + y1)).
                        arg(center.x + x2).
                        arg(boundingBox.end.y - (center.y + y2));
            }
        }
        else
        {
            str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                    arg(edge->nodeStart()->point().x).
                    arg(boundingBox.end.y - edge->nodeStart()->point().y).
                    arg(edge->nodeEnd()->point().x).
                    arg(boundingBox.end.y - edge->nodeEnd()->point().y);
        }
    }
    str += "</g>\n";
    str += "</svg>\n";

    return str;
}

void InfoWidget::doAdaptiveError()
{
    // FIXME: fieldInfo
    FieldInfo *fieldInfo = Util::scene()->activeViewField();

    int timeStep = Util::solutionStore()->timeLevels(fieldInfo).count() - 1;
    int adaptiveSteps = Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal) + 1;

    double *xval = new double[adaptiveSteps];
    double *yval = new double[adaptiveSteps];

    for (int i = 0; i < adaptiveSteps; i++)
    {
        MultiSolutionArray<double> msa = Util::solutionStore()->multiSolution(FieldSolutionID(fieldInfo, timeStep, i, SolutionMode_Normal));

        xval[i] = i+1;
        yval[i] = msa.adaptiveError();
    }

    double xerrorval[2] = { 1, adaptiveSteps };
    double yerrorval[2] = { fieldInfo->adaptivityTolerance(), fieldInfo->adaptivityTolerance() };

    ChartInfoWidget *chartDialog = new ChartInfoWidget(this);
    chartDialog->setXLabel(tr("Error (%)"));
    chartDialog->setYLabel(tr("DOFs (-)"));
    chartDialog->setDataCurve(xval, yval, adaptiveSteps);
    chartDialog->setDataCurve(xerrorval, yerrorval, 2, Qt::red);

    chartDialog->exec();

    delete [] xval;
    delete [] yval;
}

void InfoWidget::doAdaptiveDOFs()
{
    // FIXME: fieldInfo
    FieldInfo *fieldInfo = Util::scene()->activeViewField();

    int timeStep = Util::solutionStore()->timeLevels(fieldInfo).count() - 1;
    int adaptiveSteps = Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal) + 1;

    double *xval = new double[adaptiveSteps];
    double *yval = new double[adaptiveSteps];

    for (int i = 0; i < adaptiveSteps; i++)
    {
        MultiSolutionArray<double> msa = Util::solutionStore()->multiSolution(FieldSolutionID(fieldInfo, timeStep, i, SolutionMode_Normal));

        xval[i] = i+1;
        yval[i] = Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(desmartize(msa.spaces())));
    }

    ChartInfoWidget *chartDialog = new ChartInfoWidget(this);
    chartDialog->setXLabel(tr("Steps (-)"));
    chartDialog->setYLabel(tr("DOFs (-)"));
    chartDialog->setDataCurve(xval, yval, adaptiveSteps);

    chartDialog->exec();

    delete [] xval;
    delete [] yval;
}

ChartInfoWidget::ChartInfoWidget(QWidget *parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    createControls();
}

void ChartInfoWidget::createControls()
{
    chart = new Chart(this, true);

    // dialog buttons
    QPushButton *btnSaveImage = new QPushButton(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), this, SLOT(saveImage()));
    // TODO: add export data
    // QPushButton *btnSaveData = new QPushButton(tr("Save data"));
    // connect(btnSaveData, SIGNAL(clicked()), this, SLOT(saveData()));
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnSaveImage);
    // layoutButtons->addWidget(btnSaveData);
    layoutButtons->addWidget(btnClose);

    QVBoxLayout *layoutChart = new QVBoxLayout();
    layoutChart->addWidget(chart);
    layoutChart->addLayout(layoutButtons);

    setLayout(layoutChart);
}

void ChartInfoWidget::setDataCurve(double *x, double *y, int size, QColor color)
{
    // curves
    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setPen(QPen(color));
    curve->setCurveAttribute(QwtPlotCurve::Inverted);
    curve->setYAxis(QwtPlot::yLeft);
    curve->attach(chart);
    curve->setSamples(x, y, size);
}

void ChartInfoWidget::setXLabel(const QString &xlabel)
{
    chart->setAxisTitle(QwtPlot::xBottom, xlabel);
}

void ChartInfoWidget::setYLabel(const QString &ylabel)
{
    chart->setAxisTitle(QwtPlot::yLeft, ylabel);
}

void ChartInfoWidget::saveImage()
{
    chart->saveImage();
}

