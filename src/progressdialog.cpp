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

#include "progressdialog.h"

#include "gui.h"
#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "sceneview_common.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenesolution.h"
#include "logview.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

ProgressItem::ProgressItem()
{
    logMessage("ProgressItem::ProgressItem()");

    m_name = "";

    connect(this, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
}

void ProgressItem::init()
{
    m_steps = 0;
    m_isError = false;
    m_isCanceled = false;
}

void ProgressItem::showMessage(const QString &msg, bool isError, int position)
{
    logMessage("ProgressItem::()");

    m_isError = isError;
}

// *********************************************************************************************

ProgressItemSolve::ProgressItemSolve() : ProgressItem()
{
    logMessage("ProgressItemSolve::ProgressItemSolve()");

    m_name = tr("Solver");
}

void ProgressItemSolve::setSteps()
{
    m_steps = 1;
    //        if (Util::scene()->problemInfo()->analysisType() == AnalysisType_Transient)
    //        {
    //            assert(0); //TODO

    //            Util::scene()->problemInfo()->timeTotal.evaluate();
    //            Util::scene()->problemInfo()->timeStep.evaluate();

    //            m_steps += floor(Util::scene()->problemInfo()->timeTotal.number() / Util::scene()->problemInfo()->timeStep.number());
    //        }
}

bool ProgressItemSolve::run(bool quiet)
{
    logMessage("ProgressItemSolve::()");

    if (quiet) blockSignals(true);
    solve();
    if (quiet) blockSignals(false);

    return !m_isError;
}

void ProgressItemSolve::solve()
{
    logMessage("ProgressItemSolve::solve()");

    m_adaptivityError.clear();
    m_adaptivityDOF.clear();
    m_nonlinearError.clear();

    if (!QFile::exists(tempProblemFileName() + ".xml"))
        return;

    // benchmark
    QTime time;
    time.start();

    Util::problem()->createStructure();
    Util::problem()->solve(SolverMode_MeshAndSolve);

    //        emit message(tr("Problem analysis: %1 (%2, %3)").
    //                     arg(QString::fromStdString(Util::scene()->problemInfo()->module()->name)).
    //                     arg(problemTypeString(Util::scene()->problemInfo()->problemType)).
    //                     arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)), false, 1);

    //        emit message(tr("Solver was started: %1 ").arg(matrixSolverTypeString(Util::scene()->problemInfo()->matrixSolver)), false, 1);

    //        Hermes::vector<SolutionArray<double> *> solutionArrayList = Util::scene()->problemInfo()->module()->solve(this);  //TODO PK <double>

    //        if (solutionArrayList.size() > 0)
    //        {
    //            emit message(tr("Problem was solved"), false, 2);
    //            Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
    //        }
    //        else
    //        {
    //            emit message(tr("Problem was not solved"), true, 0);
    //            Util::scene()->sceneSolution()->setTimeElapsed(0);
    //        }

    //        Util::scene()->sceneSolution()->setSolutionArrayList(solutionArrayList);
}

// *********************************************************************************************

ProgressItemSolveAdaptiveStep::ProgressItemSolveAdaptiveStep() : ProgressItemSolve()
{
    logMessage("ProgressItemSolve::ProgressItemSolveAdaptive()");

    m_name = tr("Adapt. solver");
}

void ProgressItemSolveAdaptiveStep::solve()
{
    assert(0); //TODO
    //    logMessage("ProgressItemSolveAdaptive::solve()");

    //    // benchmark
    //    QTime time;
    //    time.start();

    //    emit message(tr("Problem analysis: %1 (%2, %3)").
    //                 arg(QString::fromStdString(Util::scene()->problemInfo()->module()->name)).
    //                 arg(problemTypeString(Util::scene()->problemInfo()->problemType)).
    //                 arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)), false, 1);

    //    emit message(tr("Solver was started: %1 ").arg(matrixSolverTypeString(Util::scene()->problemInfo()->matrixSolver)), false, 1);

    //    Hermes::vector<SolutionArray<double> *> solutionArrayList = Util::scene()->problemInfo()->module()->solveAdaptiveStep(this);  //TODO PK <double>

    //    if (solutionArrayList.size() > 0)
    //    {
    //        emit message(tr("Problem was solved"), false, 2);
    //        Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
    //    }
    //    else
    //    {
    //        emit message(tr("Problem was not solved"), true, 0);
    //        Util::scene()->sceneSolution()->setTimeElapsed(0);
    //    }

    //    Util::scene()->sceneSolution()->setSolutionArrayList(solutionArrayList);
}

// ***********************************************************************************************

ProgressDialog::ProgressDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ProgressDialog::ProgressDialog()");

    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    setWindowIcon(icon("run"));
    setWindowTitle(tr("Progress..."));

    createControls();

    setMinimumSize(550, 360);
    setMaximumSize(minimumSize());

    QSettings settings;
    restoreGeometry(settings.value("ProgressDialog/Geometry", saveGeometry()).toByteArray());
}

ProgressDialog::~ProgressDialog()
{
    logMessage("ProgressDialog::~ProgressDialog()");

    if (Util::config()->enabledProgressLog)
        saveProgressLog();
}

void ProgressDialog::clear()
{
    logMessage("ProgressDialog::clear()");

    // progress items
    m_progressItem.clear();
    m_currentProgressItem = NULL;

    // convergence charts
    curveError->setData(0);
    curveErrorMax->setData(0);
    curveDOF->setData(0);
    curveErrorDOF->setData(0);
    curveErrorDOFMax->setData(0);

    m_showViewProgress = true;
}

void ProgressDialog::createControls()
{
    logMessage("ProgressDialog::createControls()");

    controlsProgress = createControlsProgress();
    controlsConvergenceErrorChart = createControlsConvergenceErrorChart();
    controlsConvergenceDOFChart = createControlsConvergenceDOFChart();
    controlsConvergenceErrorDOFChart = createControlsConvergenceErrorDOFChart();
    controlsNonlinear = createControlsNonlinear();

    tabType = new QTabWidget();
    tabType->addTab(controlsProgress, icon(""), tr("Progress"));
    tabType->addTab(controlsNonlinear, icon(""), tr("Nonlin. error"));
    tabType->addTab(controlsConvergenceErrorChart, icon(""), tr("Adapt. error"));
    tabType->addTab(controlsConvergenceDOFChart, icon(""), tr("Adapt. DOFs"));
    tabType->addTab(controlsConvergenceErrorDOFChart, icon(""), tr("Adapt. conv."));
    connect(tabType, SIGNAL(currentChanged(int)), this, SLOT(resetControls(int)));

    btnCancel = new QPushButton(tr("Cance&l"));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(this, SIGNAL(finished(int)), this, SLOT(cancel()));

    btnClose = new QPushButton(tr("&Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    btnSaveImage = new QPushButton(tr("&Save image"));
    btnSaveImage->setDisabled(true);;
    connect(btnSaveImage, SIGNAL(clicked()), this, SLOT(saveImage()));

    btnSaveData = new QPushButton(tr("Save data"));
    btnSaveData->setDisabled(true);;
    connect(btnSaveData, SIGNAL(clicked()), this, SLOT(saveData()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(btnClose, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnCancel, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnSaveImage, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnSaveData, QDialogButtonBox::RejectRole);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ProgressDialog::createControlsProgress()
{
    logMessage("ProgressDialog::createControlsProgress()");

    progressBar = new QProgressBar(this);
    lblMessage = new QLabel("", this);
    lstMessage = new QTextEdit(this);
    lstMessage->setReadOnly(true);

    QVBoxLayout *layoutProgress = new QVBoxLayout();
    layoutProgress->addWidget(lblMessage);
    layoutProgress->addWidget(progressBar);
    layoutProgress->addWidget(lstMessage);

    QWidget *widProgress = new QWidget();
    widProgress->setLayout(layoutProgress);

    return widProgress;
}

QWidget *ProgressDialog::createControlsConvergenceErrorChart()
{
    logMessage("ProgressDialog::createControlsConvergenceErrorChart()");

    chartError = new Chart(this);

    // curves
    curveError = new QwtPlotCurve();
    curveError->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveError->setPen(QPen(Qt::blue));
    curveError->setCurveAttribute(QwtPlotCurve::Inverted);
    curveError->setYAxis(QwtPlot::yLeft);
    curveError->setTitle(tr("current error"));
    curveError->attach(chartError);

    // curves
    curveErrorMax = new QwtPlotCurve();
    curveErrorMax->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveErrorMax->setPen(QPen(Qt::red));
    curveErrorMax->setCurveAttribute(QwtPlotCurve::Inverted);
    curveErrorMax->setYAxis(QwtPlot::yLeft);
    curveErrorMax->setTitle(tr("max. error"));
    curveErrorMax->attach(chartError);

    // labels
    QwtText textErrorLeft(tr("Error (%)"));
    textErrorLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartError->setAxisTitle(QwtPlot::yLeft, textErrorLeft);

    QwtText textErrorBottom(tr("Steps (-)"));
    textErrorBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartError->setAxisTitle(QwtPlot::xBottom, textErrorBottom);

    // legend
    /*
    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    chart->insertLegend(legend, QwtPlot::BottomLegend);
    */

    QVBoxLayout *layoutConvergenceChart = new QVBoxLayout();
    layoutConvergenceChart->addWidget(chartError);

    QWidget *widConvergenceChart = new QWidget();
    widConvergenceChart->setLayout(layoutConvergenceChart);

    return widConvergenceChart;
}

QWidget *ProgressDialog::createControlsConvergenceDOFChart()
{
    logMessage("ProgressDialog::createControlsConvergenceDOFChart()");

    chartDOF = new Chart(this);

    // curves
    curveDOF = new QwtPlotCurve();
    curveDOF->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveDOF->setPen(QPen(Qt::blue));
    curveDOF->setCurveAttribute(QwtPlotCurve::Inverted);
    curveDOF->setYAxis(QwtPlot::yLeft);
    curveDOF->setTitle(tr("current error"));
    curveDOF->attach(chartDOF);

    // labels
    QwtText textDOFLeft(tr("DOFs (-)"));
    textDOFLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartDOF->setAxisTitle(QwtPlot::yLeft, textDOFLeft);

    QwtText textDOFBottom(tr("Steps (-)"));
    textDOFBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartDOF->setAxisTitle(QwtPlot::xBottom, textDOFBottom);

    QVBoxLayout *layoutConvergenceChart = new QVBoxLayout();
    layoutConvergenceChart->addWidget(chartDOF);

    QWidget *widConvergenceChart = new QWidget();
    widConvergenceChart->setLayout(layoutConvergenceChart);

    return widConvergenceChart;
}

QWidget *ProgressDialog::createControlsConvergenceErrorDOFChart()
{
    logMessage("ProgressDialog::createControlsConvergenceErrorDOFChart()");

    chartErrorDOF = new Chart(this);
    //    chartErrorDOF->setAxisScaleEngine(0, new QwtLog10ScaleEngine);
    //    chartErrorDOF->setAxisScaleEngine(2, new QwtLog10ScaleEngine);

    // curves
    curveErrorDOF = new QwtPlotCurve();
    curveErrorDOF->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveErrorDOF->setPen(QPen(Qt::blue));
    curveErrorDOF->setCurveAttribute(QwtPlotCurve::Inverted);
    curveErrorDOF->setYAxis(QwtPlot::yLeft);
    curveErrorDOF->setTitle(tr("conv. chart"));
    curveErrorDOF->attach(chartErrorDOF);

    // curves
    curveErrorDOFMax = new QwtPlotCurve();
    curveErrorDOFMax->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveErrorDOFMax->setPen(QPen(Qt::red));
    curveErrorDOFMax->setCurveAttribute(QwtPlotCurve::Inverted);
    curveErrorDOFMax->setYAxis(QwtPlot::yLeft);
    curveErrorDOFMax->setTitle(tr("max. error"));
    curveErrorDOFMax->attach(chartErrorDOF);

    // labels
    QwtText textErrorDOFLeft(tr("Error (%)"));
    textErrorDOFLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartErrorDOF->setAxisTitle(QwtPlot::yLeft, textErrorDOFLeft);

    QwtText textErrorDOFBottom(tr("DOFs (-)"));
    textErrorDOFBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartErrorDOF->setAxisTitle(QwtPlot::xBottom, textErrorDOFBottom);

    // legend
    /*
    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    chart->insertLegend(legend, QwtPlot::BottomLegend);
    */

    QVBoxLayout *layoutConvergenceChart = new QVBoxLayout();
    layoutConvergenceChart->addWidget(chartErrorDOF);

    QWidget *widConvergenceChart = new QWidget();
    widConvergenceChart->setLayout(layoutConvergenceChart);

    return widConvergenceChart;
}

QWidget *ProgressDialog::createControlsNonlinear()
{
    logMessage("ProgressDialog::createControlsNonlinear()");

    chartNonlinear = new Chart(this);

    // curves
    curveNonlinear = new QwtPlotCurve();
    curveNonlinear->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveNonlinear->setPen(QPen(Qt::blue));
    curveNonlinear->setCurveAttribute(QwtPlotCurve::Inverted);
    curveNonlinear->setYAxis(QwtPlot::yLeft);
    curveNonlinear->setTitle(tr("nonlinearity"));
    curveNonlinear->attach(chartNonlinear);

    // curves
    curveNonlinearMax = new QwtPlotCurve();
    curveNonlinearMax->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveNonlinearMax->setPen(QPen(Qt::red));
    curveNonlinearMax->setCurveAttribute(QwtPlotCurve::Inverted);
    curveNonlinearMax->setYAxis(QwtPlot::yLeft);
    curveNonlinearMax->setTitle(tr("max. error"));
    curveNonlinearMax->attach(chartNonlinear);

    // labels
    QwtText textErrorLeft(tr("Error (%)"));
    textErrorLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartNonlinear->setAxisTitle(QwtPlot::yLeft, textErrorLeft);

    QwtText textErrorBottom(tr("Steps (-)"));
    textErrorBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartNonlinear->setAxisTitle(QwtPlot::xBottom, textErrorBottom);

    // legend
    /*
QwtLegend *legend = new QwtLegend();
legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
chart->insertLegend(legend, QwtPlot::BottomLegend);
*/

    QVBoxLayout *layoutNonlinearity = new QVBoxLayout();
    layoutNonlinearity->addWidget(chartNonlinear);

    QWidget *widNonlinearityChart = new QWidget();
    widNonlinearityChart->setLayout(layoutNonlinearity);

    return widNonlinearityChart;
}

void ProgressDialog::resetControls(int currentTab)
{
    btnSaveImage->setEnabled(false);
    btnSaveData->setEnabled(false);

    if (Util::problem()->isSolved())
    {
        btnSaveImage->setDisabled(tabType->currentIndex() == 0);
        btnSaveData->setDisabled(tabType->currentIndex() == 0);
    }
}

int ProgressDialog::progressSteps()
{
    logMessage("ProgressDialog::progressSteps()");

    int steps = 0;
    for (int i = 0; i < m_progressItem.count(); i++)
        steps += m_progressItem.at(i)->steps();

    return steps;
}

int ProgressDialog::currentProgressStep()
{
    logMessage("ProgressDialog::currentProgressStep()");

    int steps = 0;
    for (int i = 0; i < m_progressItem.count(); i++)
    {
        if (m_progressItem.at(i) == m_currentProgressItem)
            return steps;

        steps += m_progressItem.at(i)->steps();
    }

    return -1;
}

void ProgressDialog::appendProgressItem(ProgressItem *progressItem)
{
    logMessage("ProgressDialog::appendProgressItem()");

    progressItem->init();
    progressItem->setSteps();
    progressItem->disconnect();

    m_progressItem.append(progressItem);
    connect(progressItem, SIGNAL(changed()), this, SLOT(itemChanged()));
    connect(progressItem, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
    connect(this, SIGNAL(cancelProgressItem()), progressItem, SLOT(cancelProgressItem()));
}

bool ProgressDialog::run(bool showViewProgress)
{
    logMessage("ProgressDialog::run()");

    // current widget
    tabType->setCurrentWidget(controlsProgress);

    //TODO
    //    if (Util::scene()->problemInfo()->adaptivityType == AdaptivityType_None)
    //    {
    //        controlsConvergenceErrorChart->setEnabled(false);
    //        controlsConvergenceDOFChart->setEnabled(false);
    //        controlsConvergenceErrorDOFChart->setEnabled(false);
    //    }

    m_showViewProgress = showViewProgress;
    QTimer::singleShot(0, this, SLOT(start()));

    return exec();
}

void ProgressDialog::start()
{
    logMessage("ProgressDialog::start()");

    lstMessage->clear();

    progressBar->setRange(0, progressSteps());
    progressBar->setValue(0);

    for (int i = 0; i < m_progressItem.count(); i++)
    {
        m_currentProgressItem = m_progressItem.at(i);

        if (!m_currentProgressItem->run())
        {
            // error
            finished();
            clear();
            return;
        }
    }

    //TODO
    // successfull run
    //        if ((Util::config()->showConvergenceChart
    //             && Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None
    //             && curveError->dataSize() > 0)
    //                || (Util::config()->showNonlinearChart && curveNonlinear->dataSize() > 0))
    {
        btnCancel->setEnabled(false);
        btnSaveImage->setEnabled(false);
        btnSaveData->setEnabled(false);
    }
    //        else
    //        {
    //            clear();
    //            close();
    //        }
}

void ProgressDialog::showMessage(const QString &msg, bool isError, int position)
{
    logMessage("ProgressDialog::showMessage()");

    btnCancel->setEnabled(!isError);

    if (isError)
    {
        lstMessage->setTextColor(QColor(Qt::red));
        finished();
    }
    else
    {
        lstMessage->setTextColor(QColor(Qt::black));
    }

    QString message = QString("%1: %2\n").
            arg(m_currentProgressItem->name()).
            arg(msg);

    lstMessage->insertPlainText(message);

    lstMessage->ensureCursorVisible();
    lblMessage->setText(message);

    if (position > 0)
    {
        progressBar->setValue(currentProgressStep() + position);
        Indicator::setProgress((double) position / progressSteps());
    }

    // update
    if (position % 3 == 0) QApplication::processEvents();
    lstMessage->update();
}

void ProgressDialog::itemChanged()
{
    assert(0); //TODO
    //    logMessage("ProgressDialog::itemChanged()");

    //    if (m_progressItem.count() == 0)
    //        return;

    //    ProgressItemSolve *itemSolve = dynamic_cast<ProgressItemSolve *>(m_currentProgressItem);
    //    if (itemSolve)
    //    {
    //        // adaptivity error
    //        int adaptivityCount = itemSolve->adaptivityError().count();
    //        if (adaptivityCount > 0)
    //        {
    //            double *xval = new double[adaptivityCount];
    //            double *yvalError = new double[adaptivityCount];
    //            double *yvalDOF = new double[adaptivityCount];

    //            double minDOF = numeric_limits<double>::max();
    //            double maxDOF = numeric_limits<double>::min();

    //            for (int i = 0; i<adaptivityCount; i++)
    //            {
    //                xval[i] = i+1;
    //                yvalError[i] = itemSolve->adaptivityError().at(i);
    //                yvalDOF[i] = itemSolve->adaptivityDOF().at(i);

    //                minDOF = min(minDOF, yvalDOF[i]);
    //                maxDOF = max(maxDOF, yvalDOF[i]);
    //            }

    //            // max error
    //            double *xvalErrorMax = new double[2];
    //            double *yvalErrorMax = new double[2];
    //            xvalErrorMax[0] = 1;
    //            xvalErrorMax[1] = adaptivityCount;
    //            yvalErrorMax[0] = Util::scene()->problemInfo()->adaptivityTolerance;
    //            yvalErrorMax[1] = Util::scene()->problemInfo()->adaptivityTolerance;

    //            // plot error
    //            bool doReplotError = chartError->autoReplot();
    //            chartError->setAutoReplot(false);

    //            curveError->setData(xval, yvalError, adaptivityCount);
    //            curveErrorMax->setData(xvalErrorMax, yvalErrorMax, 2);

    //            chartError->setAutoReplot(doReplotError);
    //            chartError->replot();

    //            // plot dof
    //            bool doReplotDOF = chartDOF->autoReplot();
    //            chartDOF->setAutoReplot(false);

    //            curveDOF->setData(xval, yvalDOF, adaptivityCount);

    //            chartDOF->setAutoReplot(doReplotDOF);
    //            chartDOF->replot();

    //            // max error
    //            double *xvalErrorDOFMax = new double[2];
    //            double *yvalErrorDOFMax = new double[2];
    //            xvalErrorDOFMax[0] = minDOF;
    //            xvalErrorDOFMax[1] = maxDOF;
    //            yvalErrorDOFMax[0] = Util::scene()->problemInfo()->adaptivityTolerance;
    //            yvalErrorDOFMax[1] = Util::scene()->problemInfo()->adaptivityTolerance;

    //            // plot conv. chart
    //            bool doReplotErrorDOF = chartErrorDOF->autoReplot();
    //            chartErrorDOF->setAutoReplot(false);

    //            curveErrorDOF->setData(yvalDOF, yvalError, adaptivityCount);
    //            curveErrorDOFMax->setData(xvalErrorDOFMax, yvalErrorDOFMax, 2);

    //            chartErrorDOF->setAutoReplot(doReplotErrorDOF);
    //            chartErrorDOF->replot();

    //            // save data
    //            QFile fileErr(tempProblemDir() + "/adaptivity_error.csv");
    //            QTextStream outErr(&fileErr);
    //            if (fileErr.open(QIODevice::WriteOnly | QIODevice::Text))
    //            {
    //                for (unsigned i = 0; i < curveError->data().size(); i++)
    //                    outErr << curveError->data().x(i) << ";" << curveError->data().y(i) << endl;
    //            }
    //            fileErr.close();

    //            QFile fileDOF(tempProblemDir() + "/adaptivity_dof.csv");
    //            QTextStream outDOF(&fileDOF);
    //            if (fileDOF.open(QIODevice::WriteOnly | QIODevice::Text))
    //            {
    //                for (unsigned i = 0; i < curveDOF->data().size(); i++)
    //                    outDOF << curveDOF->data().x(i) << ";" << curveDOF->data().y(i) << endl;
    //            }
    //            fileDOF.close();

    //            QFile fileErrDOF(tempProblemDir() + "/adaptivity_conv.csv");
    //            QTextStream outErrDOF(&fileErrDOF);
    //            if (fileErrDOF.open(QIODevice::WriteOnly | QIODevice::Text))
    //            {
    //                for (unsigned i = 0; i < curveErrorDOF->data().size(); i++)
    //                    outErrDOF << curveErrorDOF->data().x(i) << ";" << curveErrorDOF->data().y(i) << endl;
    //            }
    //            fileErrDOF.close();

    //            // save images
    //            QFile::remove(tempProblemDir() + "/adaptivity_error.png");
    //            QFile::remove(tempProblemDir() + "/adaptivity_dof.png");
    //            QFile::remove(tempProblemDir() + "/adaptivity_conv.png");

    //            chartError->saveImage(tempProblemDir() + "/adaptivity_error.png");
    //            chartDOF->saveImage(tempProblemDir() + "/adaptivity_dof.png");
    //            chartErrorDOF->saveImage(tempProblemDir() + "/adaptivity_conv.png");

    //            delete[] xval;
    //            delete[] yvalError;
    //            delete[] xvalErrorMax;
    //            delete[] yvalErrorMax;
    //            delete[] yvalDOF;
    //            delete[] xvalErrorDOFMax;
    //            delete[] yvalErrorDOFMax;
    //        }

    //        // nonlinearity
    //        int nonlinearErrorCount = itemSolve->nonlinearError().count();
    //        if (nonlinearErrorCount > 0)
    //        {
    //            double *xval = new double[nonlinearErrorCount];
    //            double *yvalError = new double[nonlinearErrorCount];

    //            for (int i = 0; i<nonlinearErrorCount; i++)
    //            {
    //                xval[i] = i+1;
    //                yvalError[i] = itemSolve->nonlinearError().at(i);
    //            }

    //            // max error
    //            double *xvalErrorMax = new double[2];
    //            double *yvalErrorMax = new double[2];
    //            xvalErrorMax[0] = 1;
    //            xvalErrorMax[1] = nonlinearErrorCount;
    //            yvalErrorMax[0] = Util::scene()->problemInfo()->nonlinearTolerance;
    //            yvalErrorMax[1] = Util::scene()->problemInfo()->nonlinearTolerance;

    //            // plot error
    //            bool doReplotError = chartNonlinear->autoReplot();
    //            chartNonlinear->setAutoReplot(false);

    //            curveNonlinear->setData(xval, yvalError, nonlinearErrorCount);
    //            curveNonlinearMax->setData(xvalErrorMax, yvalErrorMax, 2);

    //            chartNonlinear->setAutoReplot(doReplotError);
    //            chartNonlinear->replot();

    //            // save data
    //            QFile fileErr(tempProblemDir() + "/nonlinear_error.csv");
    //            QTextStream outErr(&fileErr);
    //            if (fileErr.open(QIODevice::WriteOnly | QIODevice::Text))
    //            {
    //                for (int i = 0; i < curveNonlinear->data().size(); i++)
    //                    outErr << curveNonlinear->data().x(i) << ";" << curveNonlinear->data().y(i) << endl;
    //            }
    //            fileErr.close();

    //            // save image
    //            chartNonlinear->saveImage(tempProblemDir() + "/nonlinear_error.png");

    //            delete[] xval;
    //            delete[] yvalError;
    //            delete[] xvalErrorMax;
    //            delete[] yvalErrorMax;
    //        }
    //    }
}

void ProgressDialog::finished()
{
    logMessage("ProgressDialog::finished()");

    btnClose->setEnabled(true);
    btnCancel->setEnabled(false);
}

void ProgressDialog::cancel()
{
    logMessage("ProgressDialog::cancel()");

    emit cancelProgressItem();
    finished();
    QApplication::processEvents();
}

void ProgressDialog::close()
{
    logMessage("ProgressDialog::close()");

    QSettings settings;
    settings.setValue("ProgressDialog/Geometry", saveGeometry());

    // save progress messages
    if (Util::config()->enabledProgressLog)
        saveProgressLog();

    cancel();
    accept();
}

void ProgressDialog::saveProgressLog()
{
    logMessage("ProgressDialog::saveProgressLog()");

    QFile file(tempProblemDir() + "/messages.log");
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream messages(&file);
        messages << QDateTime(QDateTime::currentDateTime()).toString("dd.MM.yyyy hh:mm") + ",\x20" << Util::scene()->problemInfo()->name + "\n" << lstMessage->toPlainText() + "\n";
    }
}

void ProgressDialog::saveImage()
{
    logMessage("ProgressDialog::saveImage()");

    if (tabType->currentWidget() == controlsConvergenceErrorChart)
    {
        chartError->saveImage();
    }
    else if (tabType->currentWidget() == controlsConvergenceDOFChart)
    {
        chartDOF->saveImage();
    }
    else if (tabType->currentWidget() == controlsConvergenceErrorDOFChart)
    {
        chartErrorDOF->saveImage();
    }
    else if (tabType->currentWidget() == controlsNonlinear)
    {
        chartNonlinear->saveImage();
    }
}

void ProgressDialog::saveData()
{
    logMessage("ProgressDialog::saveData()");

    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), dir, tr("CSV files (*.csv)"), &selectedFilter);
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);

        // open file for write
        if (fileInfo.suffix().isEmpty())
            fileName = fileName + ".csv";

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            cerr << "Could not create " + fileName.toStdString() + " file." << endl;
            return;
        }
        QTextStream out(&file);

        if (tabType->currentWidget() == controlsConvergenceErrorChart)
        {
            for (unsigned i = 0; i < curveError->data().size(); i++)
                out << curveError->data().x(i) << ";" << curveError->data().y(i) << endl;
        }
        else if (tabType->currentWidget() == controlsConvergenceDOFChart)
        {
            for (unsigned i = 0; i < curveDOF->data().size(); i++)
                out << curveDOF->data().x(i) << ";" << curveDOF->data().y(i) << endl;
        }
        else if (tabType->currentWidget() == controlsConvergenceErrorDOFChart)
        {
            for (unsigned i = 0; i < curveErrorDOF->data().size(); i++)
                out << curveErrorDOF->data().x(i) << ";" << curveErrorDOF->data().y(i) << endl;
        }
        else if (tabType->currentWidget() == controlsNonlinear)
        {
            for (unsigned i = 0; i < curveNonlinear->data().size(); i++)
                out << curveNonlinear->data().x(i) << ";" << curveNonlinear->data().y(i) << endl;
        }

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDataDir", fileInfo.absolutePath());

        file.close();
    }

}

