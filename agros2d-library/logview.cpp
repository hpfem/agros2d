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

#include "logview.h"

#include "util/global.h"
#include "util/constants.h"
#include "util/memory_monitor.h"
#include "gui/common.h"

#include "scene.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solver.h"
#include "hermes2d/solutionstore.h"

#include "qcustomplot/qcustomplot.h"

Log::Log()
{
    qRegisterMetaType<QVector<double> >("QVector<double>");
    qRegisterMetaType<SolverAgros::Phase>("SolverAgros::Phase");
}

// *******************************************************************************************************

LogWidget::LogWidget(QWidget *parent) : QWidget(parent),
    m_printCounter(0)
{    
    plainLog = new QPlainTextEdit(this);
    plainLog->setReadOnly(true);
    plainLog->setMaximumBlockCount(500);
    plainLog->setMinimumSize(160, 80);

    memoryLabel = new QLabel("                                                         ");
    memoryLabel->setVisible(false);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(plainLog, 1);
    layoutMain->addWidget(memoryLabel, 0, Qt::AlignLeft);

    setLayout(layoutMain);

    createActions();

    // context menu
    mnuInfo = new QMenu(this);
    mnuInfo->addAction(actShowTimestamp);
#ifndef QT_NO_DEBUG_OUTPUT
    mnuInfo->addAction(actShowDebug);
#endif
    mnuInfo->addSeparator();
    mnuInfo->addAction(actClear);

    connect(Agros2D::log(), SIGNAL(headingMsg(QString)), this, SLOT(printHeading(QString)));
    connect(Agros2D::log(), SIGNAL(messageMsg(QString, QString)), this, SLOT(printMessage(QString, QString)));
    connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(Agros2D::log(), SIGNAL(warningMsg(QString, QString)), this, SLOT(printWarning(QString, QString)));
    connect(Agros2D::log(), SIGNAL(debugMsg(QString, QString)), this, SLOT(printDebug(QString, QString)));

    plainLog->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plainLog, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
}

LogWidget::~LogWidget()
{
}

void LogWidget::contextMenu(const QPoint &pos)
{
    mnuInfo->exec(QCursor::pos());
}

void LogWidget::createActions()
{
    QSettings settings;

    actShowTimestamp = new QAction(icon(""), tr("Show timestamp"), this);
    actShowTimestamp->setCheckable(true);
    actShowTimestamp->setChecked(settings.value("LogWidget/ShowTimestamp", false).toBool());
    connect(actShowTimestamp, SIGNAL(triggered()), this, SLOT(showTimestamp()));

    actShowDebug = new QAction(icon(""), tr("Show debug"), this);
    actShowDebug->setCheckable(true);
    actShowDebug->setChecked(settings.value("LogWidget/ShowDebug", false).toBool());
    connect(actShowDebug, SIGNAL(triggered()), this, SLOT(showDebug()));

    actClear = new QAction(icon(""), tr("Clear"), this);
    connect(actClear, SIGNAL(triggered()), plainLog, SLOT(clear()));
}

void LogWidget::showTimestamp()
{
    QSettings settings;
    settings.setValue("LogWidget/ShowTimestamp", actShowTimestamp->isChecked());
}

void LogWidget::showDebug()
{
    QSettings settings;
    settings.setValue("LogWidget/ShowDebug", actShowDebug->isChecked());
}

void LogWidget::printHeading(const QString &message)
{
    print(tr("Start"), tr("%1").arg(message), "green");
    plainLog->ensureCursorVisible();
}

void LogWidget::printMessage(const QString &module, const QString &message)
{
    print(module, message, "black");
}

void LogWidget::printError(const QString &module, const QString &message)
{
    print(module, message, "red");
}

void LogWidget::printWarning(const QString &module, const QString &message)
{
    print(module, message, "blue");
}

void LogWidget::printDebug(const QString &module, const QString &message)
{
#ifndef QT_NO_DEBUG_OUTPUT
    if (actShowDebug->isChecked())
        print(module, message, "gray");
#endif
}

void LogWidget::print(const QString &module, const QString &message, const QString &color)
{
    QString strTime = "";
    if (actShowTimestamp->isChecked())
    {
#if QT_VERSION < 0x050000
        strTime = Qt::escape(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ");
#else
        strTime = QString(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ").toHtmlEscaped();
#endif
    }
#if QT_VERSION < 0x050000
    QString strMessage = Qt::escape(message);
#else
    QString strMessage = QString(message).toHtmlEscaped();
#endif

    QString html = QString("<div><span style=\"color: gray;\">%1</span><span style=\"color: %2;\"><strong>%3</strong>: %4</span></div>").
            arg(strTime).
            arg(color).
            arg(module).
            arg(strMessage);

    plainLog->appendHtml(html);

    // force run process events
    m_printCounter++;
    if (m_printCounter == 20)
    {
        // reset counter and process events
        m_printCounter = 0;
    }

    // ensure cursor visible
    QTextCursor cursor = plainLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    plainLog->setTextCursor(cursor);
    plainLog->ensureCursorVisible();
    plainLog->repaint();
}

void LogWidget::welcomeMessage()
{
    print("Agros2D", tr("version: %1").arg(QApplication::applicationVersion()), "green");
}

bool LogWidget::isMemoryLabelVisible() const
{
    return memoryLabel->isVisible();
}

void LogWidget::setMemoryLabelVisible(bool visible)
{
    memoryLabel->setVisible(visible);

    if (visible)
        connect(Agros2D::memoryMonitor(), SIGNAL(refreshMemory(int)), this, SLOT(refreshMemory(int)));
    else
        disconnect(Agros2D::memoryMonitor(), SIGNAL(refreshMemory(int)), this, SLOT(refreshMemory(int)));
}

void LogWidget::refreshMemory(int usage)
{
    // show memory usage
    memoryLabel->setText(tr("Physical memory: %1 MB").arg(usage));
    memoryLabel->repaint();
}

// *******************************************************************************************************

LogView::LogView(QWidget *parent) : QDockWidget(tr("Application log"), parent)
{
    setObjectName("LogView");

    logWidget = new LogWidget(this);
    logWidget->setMemoryLabelVisible(true);
    logWidget->welcomeMessage();

    setWidget(logWidget);
}

// *******************************************************************************************************

LogDialog::LogDialog(QWidget *parent, const QString &title) : QDialog(parent),
    m_nonlinearChart(NULL), m_nonlinearErrorGraph(NULL), m_nonlinearProgress(NULL),
    m_adaptivityChart(NULL), m_adaptivityErrorGraph(NULL), m_adaptivityDOFsGraph(NULL), m_adaptivityProgress(NULL),
    m_timeChart(NULL), m_timeTimeStepGraph(NULL), m_timeProgress(NULL),
    m_progress(NULL)
{
    setModal(true);

    setWindowIcon(icon("run"));
    setWindowTitle(title);
    setAttribute(Qt::WA_DeleteOnClose);

    createControls();

    setMinimumSize(550, 250);

    QSettings settings;
    restoreGeometry(settings.value("LogDialog/Geometry", saveGeometry()).toByteArray());
}

void LogDialog::closeEvent(QCloseEvent *e)
{
    if (Agros2D::problem()->isMeshing() || Agros2D::problem()->isSolving())
        e->ignore();
}

void LogDialog::reject()
{
    if (Agros2D::problem()->isMeshing() || Agros2D::problem()->isSolving())
        Agros2D::problem()->doAbortSolve();
    else
        close();
}

LogDialog::~LogDialog()
{
    QSettings settings;
    settings.setValue("LogDialog/Geometry", saveGeometry());
}

void LogDialog::createControls()
{
    connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(Agros2D::log(), SIGNAL(updateNonlinearChart(SolverAgros::Phase, const QVector<double>, const QVector<double>)),
            this, SLOT(updateNonlinearChartInfo(SolverAgros::Phase, const QVector<double>, const QVector<double>)));
    connect(Agros2D::log(), SIGNAL(updateAdaptivityChart(const FieldInfo *, int, int)), this, SLOT(updateAdaptivityChartInfo(const FieldInfo *, int, int)));
    connect(Agros2D::log(), SIGNAL(updateTransientChart(double)), this, SLOT(updateTransientChartInfo(double)));
    connect(Agros2D::log(), SIGNAL(addIconImg(QIcon, QString)), this, SLOT(addIcon(QIcon, QString)));

    m_logWidget = new LogWidget(this);
    m_logWidget->setMemoryLabelVisible(false);

    // set log visibility
    QSettings settings;
    m_logWidget->setVisible(settings.value("LogDialog/ShowLog", false).toBool());

#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif

    QFont fontProgress = font();
    fontProgress.setPointSize(fontSize);

    m_progress = new QListWidget(this);
    m_progress->setCurrentRow(0);
    m_progress->setViewMode(QListView::IconMode);
    m_progress->setResizeMode(QListView::Adjust);
    m_progress->setMovement(QListView::Static);
    m_progress->setResizeMode(QListView::Adjust);
    m_progress->setFlow(QListView::LeftToRight);
    m_progress->setIconSize(QSize(32, 32));
    m_progress->setMinimumHeight(85);
    m_progress->setMaximumHeight(85);
    m_progress->setFont(fontProgress);
    m_progress->setStyleSheet(QString("QListView { background-color: %1; border: 0px; padding: 0px; margin: 0px; }").
                              arg(this->palette().color(QPalette::Background).name()));

    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(tryClose()));
    btnClose->setEnabled(false);

    btnShowHide = new QPushButton(tr("Show/hide log"));
    connect(btnShowHide, SIGNAL(clicked()), this, SLOT(showHide()));

    btnAbort = new QPushButton(tr("Abort"));
    connect(btnAbort, SIGNAL(clicked()), Agros2D::problem(), SLOT(doAbortSolve()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(close()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(close()));

    QHBoxLayout *layoutStatus = new QHBoxLayout();
    layoutStatus->addStretch();
    layoutStatus->addWidget(btnShowHide, 0, Qt::AlignRight);
    layoutStatus->addWidget(btnAbort, 0, Qt::AlignRight);
    layoutStatus->addWidget(btnClose, 0, Qt::AlignRight);

    QHBoxLayout *layoutHorizontal = NULL;
    if (Agros2D::problem()->numAdaptiveFields() > 0 || Agros2D::problem()->determineIsNonlinear() || Agros2D::problem()->isTransient())
    {
        QPen pen;
        pen.setColor(Qt::darkGray);
        pen.setWidth(1.5);

        QPen penError;
        penError.setColor(Qt::darkRed);
        penError.setWidth(2.0);

        QFont fontTitle(font());
        fontTitle.setBold(true);

        QFont fontChart(font());
        fontChart.setPointSize(fontSize);

        layoutHorizontal = new QHBoxLayout();
        if (Agros2D::problem()->isTransient())
        {
            m_timeChart = new QCustomPlot(this);
            m_timeChart->setVisible(Agros2D::problem()->isTransient());
            QCPPlotTitle *timeTitle = new QCPPlotTitle(m_timeChart, tr("Transient problem"));
            timeTitle->setFont(fontTitle);
            m_timeChart->plotLayout()->insertRow(0);
            m_timeChart->plotLayout()->addElement(0, 0, timeTitle);

            m_timeChart->xAxis->setTickLabelFont(fontChart);
            m_timeChart->xAxis->setLabelFont(fontChart);
            // m_timeChart->xAxis->setTickStep(1.0);
            m_timeChart->xAxis->setAutoTickStep(true);
            m_timeChart->xAxis->setLabel(tr("number of steps"));

            m_timeChart->yAxis->setTickLabelFont(fontChart);
            m_timeChart->yAxis->setLabelFont(fontChart);
            m_timeChart->yAxis->setLabel(tr("step length"));

            m_timeTimeStepGraph = m_timeChart->addGraph(m_timeChart->xAxis, m_timeChart->yAxis);
            m_timeTimeStepGraph->setLineStyle(QCPGraph::lsLine);
            m_timeTimeStepGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
            m_timeTimeStepGraph->setPen(pen);
            m_timeTimeStepGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));

            m_timeProgress = new QProgressBar(this);
            m_timeProgress->setMaximum(10000);
            m_timeProgress->setVisible(Agros2D::problem()->isTransient());

            QVBoxLayout *layoutTime = new QVBoxLayout();
            layoutTime->addWidget(m_timeChart, 1);
            layoutTime->addWidget(m_timeProgress);

            layoutHorizontal->addLayout(layoutTime, 1);
        }
        if (Agros2D::problem()->determineIsNonlinear())
        {
            m_nonlinearChart = new QCustomPlot(this);
            m_nonlinearChart->setVisible(Agros2D::problem()->determineIsNonlinear());
            QCPPlotTitle *nonlinearTitle = new QCPPlotTitle(m_nonlinearChart, tr("Nonlinear solver"));
            nonlinearTitle->setFont(fontTitle);
            m_nonlinearChart->plotLayout()->insertRow(0);
            m_nonlinearChart->plotLayout()->addElement(0, 0, nonlinearTitle);
            m_nonlinearChart->setFont(fontChart);

            m_nonlinearChart->xAxis->setTickLabelFont(fontChart);
            m_nonlinearChart->xAxis->setLabelFont(fontChart);
            // m_nonlinearChart->xAxis->setTickStep(1.0);
            m_nonlinearChart->xAxis->setAutoTickStep(true);
            m_nonlinearChart->xAxis->setLabel(tr("number of iterations"));

            m_nonlinearChart->yAxis->setScaleType(QCPAxis::stLogarithmic);
            m_nonlinearChart->yAxis->setTickLabelFont(fontChart);
            m_nonlinearChart->yAxis->setLabelFont(fontChart);
            m_nonlinearChart->yAxis->setLabel(tr("rel. change of sln. (%)"));

            m_nonlinearErrorGraph = m_nonlinearChart->addGraph(m_nonlinearChart->xAxis, m_nonlinearChart->yAxis);
            m_nonlinearErrorGraph->setLineStyle(QCPGraph::lsLine);
            m_nonlinearErrorGraph->setPen(pen);
            m_nonlinearErrorGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));

            m_nonlinearProgress = new QProgressBar(this);
            m_nonlinearProgress->setMaximum(10000);
            m_nonlinearProgress->setVisible(Agros2D::problem()->determineIsNonlinear());

            QVBoxLayout *layoutNonlinear = new QVBoxLayout();
            layoutNonlinear->addWidget(m_nonlinearChart, 1);
            layoutNonlinear->addWidget(m_nonlinearProgress);

            layoutHorizontal->addLayout(layoutNonlinear, 1);
        }
        if (Agros2D::problem()->numAdaptiveFields() > 0)
        {
            m_adaptivityChart = new QCustomPlot(this);
            m_adaptivityChart->setVisible(Agros2D::problem()->numAdaptiveFields() > 0);
            QCPPlotTitle *adaptivityTitle = new QCPPlotTitle(m_adaptivityChart, tr("Adaptivity"));
            adaptivityTitle->setFont(fontTitle);
            m_adaptivityChart->plotLayout()->insertRow(0);
            m_adaptivityChart->plotLayout()->addElement(0, 0, adaptivityTitle);
            m_adaptivityChart->legend->setVisible(true);
            m_adaptivityChart->legend->setFont(fontChart);

            m_adaptivityChart->xAxis->setTickLabelFont(fontChart);
            m_adaptivityChart->xAxis->setLabelFont(fontChart);
            // m_adaptivityChart->xAxis->setTickStep(1.0);
            m_adaptivityChart->xAxis->setAutoTickStep(true);
            m_adaptivityChart->xAxis->setLabel(tr("number of iterations"));

            m_adaptivityChart->yAxis->setScaleType(QCPAxis::stLogarithmic);
            m_adaptivityChart->yAxis->setTickLabelFont(fontChart);
            m_adaptivityChart->yAxis->setLabelFont(fontChart);
            m_adaptivityChart->yAxis->setLabel(tr("error"));
            m_adaptivityChart->yAxis2->setVisible(true);
            m_adaptivityChart->yAxis2->setTickLabelFont(fontChart);
            m_adaptivityChart->yAxis2->setLabelFont(fontChart);
            m_adaptivityChart->yAxis2->setLabel(tr("number of DOFs"));

            m_adaptivityErrorGraph = m_adaptivityChart->addGraph(m_adaptivityChart->xAxis, m_adaptivityChart->yAxis);
            m_adaptivityErrorGraph->setLineStyle(QCPGraph::lsLine);
            m_adaptivityErrorGraph->setPen(pen);
            m_adaptivityErrorGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
            m_adaptivityErrorGraph->setName(tr("error"));
            m_adaptivityDOFsGraph = m_adaptivityChart->addGraph(m_adaptivityChart->xAxis, m_adaptivityChart->yAxis2);
            m_adaptivityDOFsGraph->setLineStyle(QCPGraph::lsLine);
            m_adaptivityDOFsGraph->setPen(pen);
            m_adaptivityDOFsGraph->setBrush(QBrush(QColor(255, 0, 0, 20)));
            m_adaptivityDOFsGraph->setName(tr("DOFs"));

            m_adaptivityProgress = new QProgressBar(this);
            m_adaptivityProgress->setMaximum(10000);
            m_adaptivityProgress->setVisible(Agros2D::problem()->numAdaptiveFields() > 0);

            QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
            layoutAdaptivity->addWidget(m_adaptivityChart, 1);
            layoutAdaptivity->addWidget(m_adaptivityProgress);

            layoutHorizontal->addLayout(layoutAdaptivity, 1);
        }
    }
    else
    {
        m_logWidget->setVisible(true);
    }

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_progress, 0);
    if (Agros2D::problem()->numAdaptiveFields() > 0 || Agros2D::problem()->determineIsNonlinear() || Agros2D::problem()->isTransient())
        layout->addLayout(layoutHorizontal, 4);
    layout->addWidget(m_logWidget, 1);
    layout->addStretch();
    layout->addLayout(layoutStatus);

    setLayout(layout);
}

void LogDialog::printError(const QString &module, const QString &message)
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);

    m_logWidget->setVisible(true);
}

void LogDialog::updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions)
{
    if (!m_nonlinearErrorGraph)
        return;

    m_nonlinearErrorGraph->setData(steps, relativeChangeOfSolutions);
    m_nonlinearChart->rescaleAxes();
    m_nonlinearChart->replot();

    // progress bar
    if (phase == SolverAgros::Phase_Finished)
    {
        m_nonlinearProgress->setValue(10000);
    }
    else
    {
        double valueRelativeChange = pow(10000.0, ((relativeChangeOfSolutions.first() - relativeChangeOfSolutions.last()) / relativeChangeOfSolutions.first()));
        m_nonlinearProgress->setValue(valueRelativeChange);
    }
}

void LogDialog::updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{   
    if (!m_adaptivityErrorGraph)
        return;

    QVector<double> adaptiveSteps;
    QVector<double> adaptiveDOFs;
    QVector<double> adaptiveError;

    for (int i = 0; i < adaptivityStep; i++)
    {
        SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(fieldInfo, timeStep, i, SolutionMode_Normal));

        adaptiveSteps.append(i + 1);
        adaptiveDOFs.append(runTime.DOFs());
        adaptiveError.append(runTime.adaptivityError());
    }

    m_adaptivityErrorGraph->setData(adaptiveSteps, adaptiveError);
    m_adaptivityDOFsGraph->setData(adaptiveSteps, adaptiveDOFs);
    m_adaptivityChart->rescaleAxes();
    m_adaptivityChart->replot();

    // progress bar
    double valueSteps = 10000.0 * (adaptivityStep / fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());
    double valueTol = pow(10000.0, (adaptiveError.first() - adaptiveError.last()) / adaptiveError.first());
    m_adaptivityProgress->setValue(qMax(valueSteps, valueTol));
}

void LogDialog::updateTransientChartInfo(double actualTime)
{
    if (!m_timeTimeStepGraph)
        return;

    QVector<double> timeSteps;
    QVector<double> timeLengths = Agros2D::problem()->timeStepLengths().toVector();
    double maximum = 0.0;
    for (int i = 0; i < timeLengths.size(); i++)
    {
        timeSteps.append(i + 1);
        if (timeLengths[i] > maximum)
            maximum = timeLengths[i];
    }

    m_timeTimeStepGraph->setData(timeSteps, timeLengths);
    m_timeChart->yAxis->setRangeLower(0.0);
    m_timeChart->yAxis->setRangeUpper(maximum);
    m_timeTimeStepGraph->rescaleKeyAxis();
    m_timeChart->replot();

    // progress bar
    m_timeProgress->setValue((10000.0 * actualTime / Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble()));
}

void LogDialog::addIcon(const QIcon &icn, const QString &label)
{
    static QString previousLabel;

    if (previousLabel != label)
    {
        QListWidgetItem *item = new QListWidgetItem(icn, label, m_progress);
        item->setTextAlignment(Qt::AlignHCenter);

        m_progress->addItem(item);
        // m_progress->setCurrentItem(item);
        m_progress->repaint();
    }

    previousLabel = label;
}

void LogDialog::tryClose()
{
    if (Agros2D::problem()->isSolving())
        Agros2D::log()->printError(tr("Solver"), tr("Problem is being aborted."));
    else
        close();
}

void LogDialog::showHide()
{
    m_logWidget->setVisible(!m_logWidget->isVisible());
    repaint();

    QSettings settings;
    settings.setValue("LogDialog/ShowLog", m_logWidget->isVisible());
}

// *******************************************************************************************

LogStdOut::LogStdOut(QWidget *parent) : QObject(parent)
{
    connect(Agros2D::log(), SIGNAL(headingMsg(QString)), this, SLOT(printHeading(QString)));
    connect(Agros2D::log(), SIGNAL(messageMsg(QString, QString)), this, SLOT(printMessage(QString, QString)));
    connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(Agros2D::log(), SIGNAL(warningMsg(QString, QString)), this, SLOT(printWarning(QString, QString)));
    connect(Agros2D::log(), SIGNAL(debugMsg(QString, QString)), this, SLOT(printDebug(QString, QString)));
}

void LogStdOut::printHeading(const QString &message)
{
    Hermes::Mixins::Loggable::Static::warn(QString("%1").arg(message).toLatin1());
}

void LogStdOut::printMessage(const QString &module, const QString &message)
{
    Hermes::Mixins::Loggable::Static::warn(QString("%1: %2").arg(module).arg(message).toLatin1());
}

void LogStdOut::printError(const QString &module, const QString &message)
{
    Hermes::Mixins::Loggable::Static::error(QString("%1: %2").arg(module).arg(message).toLatin1());
}

void LogStdOut::printWarning(const QString &module, const QString &message)
{
    Hermes::Mixins::Loggable::Static::info(QString("%1: %2").arg(module).arg(message).toLatin1());
}

void LogStdOut::printDebug(const QString &module, const QString &message)
{
    Hermes::Mixins::Loggable::Static::info(QString("%1: %2").arg(module).arg(message).toLatin1());
}

