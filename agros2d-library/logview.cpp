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
#include "hermes2d/problem.h"

#include "qcustomplot/qcustomplot.h"
#include "ctemplate/template.h"

Log::Log()
{
    qRegisterMetaType<QVector<double>   >("QVector<double>");
}

// *******************************************************************************************************

LogWidget::LogWidget(QWidget *parent) : QWidget(parent),
    m_printCounter(0), m_logInfo(NULL)
{    
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    webView->setMinimumSize(160, 160);

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize() - 1).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_results.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    initWebView();

    memoryLabel = new QLabel("                                                         ");
    memoryLabel->setVisible(false);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(webView, 1);
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

    webView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(webView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
}

LogWidget::~LogWidget()
{
    delete m_logInfo;
}

void LogWidget::initWebView()
{
    if (m_logInfo)
        delete m_logInfo;

    m_logInfo = new ctemplate::TemplateDictionary("info");

    m_logInfo->SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());
    m_logInfo->SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    m_logInfo->SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());

    webView->setHtml("");
}

void LogWidget::showHtml()
{
    ctemplate::TemplateDictionary *local = m_logInfo->MakeCopy("local");

    // remove first items in cache
    while (m_logItems.count() > 150)
        m_logItems.removeFirst();

    local->SetValue("ITEMS", m_logItems.join("").toStdString());

    std::string info;
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/logview.tpl").toStdString(), ctemplate::DO_NOT_STRIP, local, &info);
    delete local;

    webView->setHtml(QString::fromStdString(info));
    webView->page()->mainFrame()->setScrollBarValue(Qt::Vertical, webView->page()->mainFrame()->scrollBarMaximum(Qt::Vertical));

    repaint();
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
    connect(actClear, SIGNAL(triggered()), this, SLOT(initWebView()));
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
    // template
    ctemplate::TemplateDictionary item("heading");

#if QT_VERSION < 0x050000
    item.SetValue("ITEM_HEADING_MESSAGE", Qt::escape(message).toStdString());
#else
    item.SetValue("ITEM_HEADING_MESSAGE", QString(message).toHtmlEscaped().toStdString());
#endif

    std::string info;
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/logview_heading.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &item, &info);
    m_logItems.append(QString::fromStdString(info));

    showHtml();
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
    // template
    ctemplate::TemplateDictionary item("text");

    if (actShowTimestamp->isChecked())
    {
#if QT_VERSION < 0x050000
        item.SetValue("ITEM_TIME", Qt::escape(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ").toStdString());
#else
        item.SetValue("ITEM_TIME", QString(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ").toHtmlEscaped().toStdString());
#endif
    }
    item.SetValue("ITEM_COLOR", color.toStdString());
    item.SetValue("ITEM_MODULE", module.toStdString());
#if QT_VERSION < 0x050000
    item.SetValue("ITEM_MESSAGE", Qt::escape(message).toStdString());
#else
    item.SetValue("ITEM_MESSAGE", QString(message).toHtmlEscaped().toStdString());
#endif

    std::string info;
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/logview_text.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &item, &info);
    m_logItems.append(QString::fromStdString(info));

    showHtml();

    // force run process events
    m_printCounter++;
    if (m_printCounter == 20)
    {
        // reset counter and process events
        m_printCounter = 0;
    }
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

LogDialog::LogDialog(QWidget *parent, const QString &title) : QDialog(parent)
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
    connect(Agros2D::log(), SIGNAL(nonlinearTable(QVector<double>, QVector<double>)), this, SLOT(nonlinearTable(QVector<double>,QVector<double>)));
    connect(Agros2D::log(), SIGNAL(adaptivityTable(QVector<double>, QVector<double>)), this, SLOT(adaptivityTable(QVector<double>, QVector<double>)));

    logWidget = new LogWidget(this);
    logWidget->setMemoryLabelVisible(false);

    m_nonlinearChart = new QCustomPlot(this);
    m_nonlinearChart->setVisible(false);
    m_nonlinearChart->setMinimumWidth(300);
    m_nonlinearChart->xAxis->setTickStep(1.0);
    m_nonlinearChart->xAxis->setAutoTickStep(false);
    m_nonlinearChart->xAxis->setLabel(tr("iteration"));
    m_nonlinearChart->yAxis->setLabel(tr("rel. error (%)"));
    m_nonlinearChart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    m_nonlinearChart->addGraph();

    m_nonlinearChart->graph(0)->setLineStyle(QCPGraph::lsLine);

    if (Agros2D::problem()->determineIsNonlinear())
    {
        m_nonlinearChart->setVisible(true);
    }

    m_adaptivityChart = new QCustomPlot(this);
    m_adaptivityChart->setVisible(false);
    m_adaptivityChart->setMinimumWidth(300);
    m_adaptivityChart->xAxis->setTickStep(1.0);
    m_adaptivityChart->xAxis->setAutoTickStep(false);
    m_adaptivityChart->xAxis->setLabel(tr("iteration"));
    m_adaptivityChart->yAxis->setLabel(tr("error"));
    m_adaptivityChart->addGraph();

    m_adaptivityChart->graph(0)->setLineStyle(QCPGraph::lsLine);

    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(tryClose()));
    btnClose->setEnabled(false);

    btnAbort = new QPushButton(tr("Abort"));
    connect(btnAbort, SIGNAL(clicked()), Agros2D::problem(), SLOT(doAbortSolve()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(close()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(close()));

    QHBoxLayout *layoutStatus = new QHBoxLayout();
    layoutStatus->addStretch();
    layoutStatus->addWidget(btnAbort, 0, Qt::AlignRight);
    layoutStatus->addWidget(btnClose, 0, Qt::AlignRight);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(logWidget, 1);
    layoutHorizontal->addWidget(m_nonlinearChart, 0);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    layout->addLayout(layoutStatus);

    setLayout(layout);
}

void LogDialog::printError(const QString &module, const QString &message)
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);
}

void LogDialog::nonlinearTable(QVector<double> step, QVector<double> error)
{
    m_nonlinearChart->graph(0)->setData(step, error);
    m_nonlinearChart->rescaleAxes();
    m_nonlinearChart->replot();
}

void LogDialog::adaptivityTable(QVector<double> step, QVector<double> error)
{
    m_adaptivityChart->graph(0)->setData(step, error);
    m_adaptivityChart->rescaleAxes();
    m_adaptivityChart->replot();
}

void LogDialog::tryClose()
{
    if (Agros2D::problem()->isSolving())
        Agros2D::log()->printError(tr("Solver"), tr("Problem is being aborted."));
    else
        close();
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

