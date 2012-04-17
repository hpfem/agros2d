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
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "pythonlabagros.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"
#include "ctemplate/template.h"

InfoWidget::InfoWidget(SceneViewPreprocessor *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewGeometry = sceneView;

    setMinimumWidth(160);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    // setWindowFlags(Qt::Popup | Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);

    createActions();

    // problem information
    webView = new QWebView(this);

    // dialog buttons
    btnAdaptiveError = new QPushButton(tr("Adaptive error"));
    connect(btnAdaptiveError, SIGNAL(clicked()), SLOT(doAdaptiveError()));
    btnDOFs = new QPushButton(tr("DOFs"));
    connect(btnDOFs, SIGNAL(clicked()), SLOT(doAdaptiveDOFs()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnAdaptiveError);
    layoutButtons->addWidget(btnDOFs);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0, 5, 3, 5);
    layoutMain->addWidget(webView);
    layoutMain->addLayout(layoutButtons);

    setLayout(layoutMain);

    refresh();
}

/*
int w = 450;
int h = height() - 80;

InfoWidget *popup = new InfoWidget(sceneViewPreprocessor, this);
popup->resize(w, h);
popup->move(pos().x() + 75, pos().y() + 70);
popup->setAttribute(Qt::WA_DeleteOnClose);

popup->show();
*/

InfoWidget::~InfoWidget()
{
}

void InfoWidget::createActions()
{
    actInfo = new QAction(icon("scene-info"), tr("Info"), this);
    actInfo->setShortcut(QKeySequence("Alt+I"));
    actInfo->setCheckable(true);
}

void InfoWidget::refresh()
{
    btnAdaptiveError->setEnabled(Util::problem()->isSolved());

    QTimer::singleShot(0, this, SLOT(showInfo()));
}

void InfoWidget::showInfo()
{
    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

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

    problemInfo.SetValue("FREQUENCY_LABEL", tr("Frequency:").toStdString());
    problemInfo.SetValue("FREQUENCY", QString::number(Util::problem()->config()->frequency()).toStdString() + " Hz");
    problemInfo.SetValue("TIME_STEP_LABEL", tr("Time step:").toStdString());
    problemInfo.SetValue("TIME_STEP", QString::number(Util::problem()->config()->timeStep().number()).toStdString() + " s");
    problemInfo.SetValue("TIME_TOTAL_LABEL", tr("Total time:").toStdString());
    problemInfo.SetValue("TIME_TOTAL", QString::number(Util::problem()->config()->timeTotal().number()).toStdString() + " s");


    //        if (Util::problem()->isSolved())
    //        {
    //            if (Util::scene()->sceneSolution()->space() && (Util::scene()->sceneSolution()->space()->get_num_dofs() > 0))
    //            {
    //                QTime time = milisecondsToTime(Util::problem()->timeElapsed());
    //                problem.SetValue("ELAPSED_TIME_LABEL", tr("Elapsed time:").toStdString());
    //                problem.SetValue("ELAPSED_TIME", time.toString("mm:ss.zzz").toStdString());

    //                problem.SetValue("DOFS_LABEL", tr("DOFs:").toStdString());
    //                problem.SetValue("DOFS", QString::number(Util::scene()->sceneSolution()->space()->get_num_dofs()).toStdString());
    //            }
    //            problem.ShowSection("SOLUTION_PARAMETERS_SECTION");
    //        }
    // problemInfo.ShowSection("SOLUTION_SECTION");

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        ctemplate::TemplateDictionary *field = problemInfo.AddSectionDictionary("FIELD");

        field->SetValue("PHYSICAL_FIELD", fieldInfo->module()->name);

        field->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
        field->SetValue("ANALYSIS_TYPE", analysisTypeString(fieldInfo->analysisType()).toStdString());

        field->SetValue("WEAK_FORMS_TYPE_LABEL", tr("Weak forms:").toStdString());
        field->SetValue("WEAK_FORMS_TYPE", weakFormsTypeString(fieldInfo->weakFormsType()).toStdString());

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

        field->SetValue("LINEARITY_TYPE_LABEL", tr("Solution type:").toStdString());
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
            int timeStep = Util::solutionStore()->timeLevels(fieldInfo).count() - 1;
            int adaptiveStep = Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionType_Normal);
            MultiSolutionArray<double> msa = Util::solutionStore()->multiSolution(FieldSolutionID(fieldInfo, timeStep, adaptiveStep, SolutionType_Normal));

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
                field->SetValue("DOFS_LABEL", tr("DOFs:").toStdString());
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

    if (Util::problem()->isSolved())
    {
        problemInfo.SetValue("SOLUTION_LABEL", tr("Solution").toStdString());
        problemInfo.SetValue("SOLUTION_ELAPSED_TIME_LABEL", tr("Total elapsed time:").toStdString());
        problemInfo.SetValue("SOLUTION_ELAPSED_TIME", tr("%1 s").arg(Util::problem()->timeElapsed().toString("mm:ss.zzz")).toStdString());
        problemInfo.ShowSection("SOLUTION_PARAMETERS_SECTION");
    }

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/problem.tpl", ctemplate::DO_NOT_STRIP, &problemInfo, &info);
    webView->setHtml(QString::fromStdString(info));

    setFocus();
}

QDialog *InfoWidget::createChart(const QString &xlabel, const QString &ylabel,
                                 double *x1, double *y1, int size1,
                                 double *x2 = NULL, double *y2 = NULL, int size2 = 0)
{
    Chart *chartError = new Chart(this);

    // curves
    QwtPlotCurve *curve1 = new QwtPlotCurve();
    curve1->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve1->setPen(QPen(Qt::blue));
    curve1->setCurveAttribute(QwtPlotCurve::Inverted);
    curve1->setYAxis(QwtPlot::yLeft);
    curve1->attach(chartError);
    curve1->setData(x1, y1, size1);

    // curves
    if (size2 > 0)
    {
        QwtPlotCurve *curve2 = new QwtPlotCurve();
        curve2->setRenderHint(QwtPlotItem::RenderAntialiased);
        curve2->setPen(QPen(Qt::red));
        curve2->setCurveAttribute(QwtPlotCurve::Inverted);
        curve2->setYAxis(QwtPlot::yLeft);
        curve2->attach(chartError);
        curve2->setData(x2, y2, size2);
    }

    // labels
    QwtText textErrorLeft(ylabel);
    textErrorLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartError->setAxisTitle(QwtPlot::yLeft, textErrorLeft);

    QwtText textErrorBottom(xlabel);
    textErrorBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartError->setAxisTitle(QwtPlot::xBottom, textErrorBottom);

    // legend
    /*
    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    chart->insertLegend(legend, QwtPlot::BottomLegend);
    */

    QDialog *chartDialog = new QDialog(this);
    chartDialog->setAttribute(Qt::WA_DeleteOnClose);

    // dialog buttons
    QPushButton *btnSaveImage = new QPushButton(tr("Save image"));
    // connect(btnSaveImage, SIGNAL(clicked()), chartDialog, SLOT(doSaveImage()));
    QPushButton *btnSaveData = new QPushButton(tr("Save data"));
    // connect(btnSaveData, SIGNAL(clicked()), chartDialog, SLOT(doSaveData()));
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), chartDialog, SLOT(close()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnSaveImage);
    layoutButtons->addWidget(btnSaveData);
    layoutButtons->addWidget(btnClose);

    QVBoxLayout *layoutChart = new QVBoxLayout();
    layoutChart->addWidget(chartError);
    layoutChart->addLayout(layoutButtons);

    chartDialog->setLayout(layoutChart);

    return chartDialog;
}

void InfoWidget::doAdaptiveError()
{
    // FIXME: fieldInfo
    FieldInfo *fieldInfo = Util::scene()->activeViewField();

    int timeStep = Util::solutionStore()->timeLevels(fieldInfo).count() - 1;
    int adaptiveSteps = Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionType_Normal) + 1;

    double *xval = new double[adaptiveSteps];
    double *yval = new double[adaptiveSteps];

    for (int i = 0; i < adaptiveSteps; i++)
    {
        MultiSolutionArray<double> msa = Util::solutionStore()->multiSolution(FieldSolutionID(fieldInfo, timeStep, i, SolutionType_Normal));

        xval[i] = i+1;
        yval[i] = msa.adaptiveError();
    }

    double xerrorval[2] = { 1, adaptiveSteps };
    double yerrorval[2] = { fieldInfo->adaptivityTolerance(), fieldInfo->adaptivityTolerance() };

    QDialog *chartDialog = createChart(tr("Steps (-)"), tr("Error (%)"),
                                       xval, yval, adaptiveSteps,
                                       xerrorval, yerrorval, 2);
    chartDialog->exec();

    delete [] xval;
    delete [] yval;
}

void InfoWidget::doAdaptiveDOFs()
{
    // FIXME: fieldInfo
    FieldInfo *fieldInfo = Util::scene()->activeViewField();

    int timeStep = Util::solutionStore()->timeLevels(fieldInfo).count() - 1;
    int adaptiveSteps = Util::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionType_Normal) + 1;

    double *xval = new double[adaptiveSteps];
    double *yval = new double[adaptiveSteps];

    for (int i = 0; i < adaptiveSteps; i++)
    {
        MultiSolutionArray<double> msa = Util::solutionStore()->multiSolution(FieldSolutionID(fieldInfo, timeStep, i, SolutionType_Normal));

        xval[i] = i+1;
        yval[i] = Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(desmartize(msa.spaces())));
    }

    QDialog *chartDialog = createChart(tr("Steps (-)"), tr("DOFs (-)"), xval, yval, adaptiveSteps);
    chartDialog->exec();

    delete [] xval;
    delete [] yval;
}
