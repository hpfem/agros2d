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
#include "util/system_utils.h"

#include "scene.h"
#include "hermes2d/problem.h"

#include "qcustomplot/qcustomplot.h"

Log::Log()
{
}

// *******************************************************************************************************

LogWidget::LogWidget(QWidget *parent) : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setMinimumSize(160, 160);

    createActions();

    // context menu
    mnuInfo = new QMenu(this);
    mnuInfo->addAction(actShowTimestamp);
#ifndef QT_NO_DEBUG_OUTPUT
    mnuInfo->addAction(actShowDebug);
#endif
    mnuInfo->addSeparator();
    mnuInfo->addAction(actCopy);
    mnuInfo->addAction(actClear);

    connect(Agros2D::log(), SIGNAL(messageMsg(QString, QString, bool)), this, SLOT(printMessage(QString, QString, bool)));
    connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString, bool)), this, SLOT(printError(QString, QString, bool)));
    connect(Agros2D::log(), SIGNAL(warningMsg(QString, QString, bool)), this, SLOT(printWarning(QString, QString, bool)));
    connect(Agros2D::log(), SIGNAL(debugMsg(QString, QString, bool)), this, SLOT(printDebug(QString, QString, bool)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
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
    connect(actClear, SIGNAL(triggered()), this, SLOT(clear()));

    actCopy = new QAction(icon(""), tr("Copy"), this);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(copy()));
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

void LogWidget::printMessage(const QString &module, const QString &message, bool escaped)
{
    print(module, message, "black", escaped);
}

void LogWidget::printError(const QString &module, const QString &message, bool escaped)
{
    print(module, message, "red", escaped);
}

void LogWidget::printWarning(const QString &module, const QString &message, bool escaped)
{
    print(module, message, "blue", escaped);
}

void LogWidget::printDebug(const QString &module, const QString &message, bool escaped)
{
#ifndef QT_NO_DEBUG_OUTPUT
    if (actShowDebug->isChecked())
        print(module, message, "gray", escaped);
#endif
}

void LogWidget::print(const QString &module, const QString &message, const QString &color, bool escaped)
{
    setUpdatesEnabled(false);

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);

    QString str;

    // debug - timestamp
    if (actShowTimestamp->isChecked())
    {
        str += "<span style=\"color: gray;\">";
        // str += Qt::escape(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") + ": ");
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        str += Qt::escape(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ");
#else
        str += QString(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ").toHtmlEscaped();
#endif
        str += "</span>";
    }

    // message
    if (!color.isEmpty())
        str += "<span style=\"color: " + color + ";\">";
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    str += "<strong>" + Qt::escape(module) + "</strong>: ";
#else
    str += "<strong>" + QString(module).toHtmlEscaped() + "</strong>: ";
#endif
    if (escaped)
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        str += Qt::escape(message);
#else
        str += QString(message).toHtmlEscaped();
#endif
    else
        str += message;
    if (!color.isEmpty())
        str += "</span>";

    setUpdatesEnabled(true);
    appendHtml(str);

    ensureCursorVisible();
}

void LogWidget::welcomeMessage()
{
    print("Agros2D", tr("version: %1").arg(QApplication::applicationVersion()), "green");

    if (Agros2D::configComputer()->experimentalFeatures)
        Agros2D::log()->printWarning(tr("Agros2D"), tr("Experimental features are allowed. It may lead to application fail. You may disable it in appllication settings."));


}

// *******************************************************************************************************

LogView::LogView(QWidget *parent) : QDockWidget(tr("Applicaton log"), parent)
{
    setObjectName("LogView");

    logWidget = new LogWidget(this);
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

LogDialog::~LogDialog()
{
    QSettings settings;
    settings.setValue("LogDialog/Geometry", saveGeometry());
}

void LogDialog::createControls()
{
    connect(Agros2D::log(), SIGNAL(messageMsg(QString, QString, bool)), this, SLOT(printMessage(QString, QString, bool)));

    logWidget = new LogWidget(this);
    memoryLabel = new QLabel("                                                         ");

    m_chart = new QCustomPlot(this);
    m_chart->setVisible(false);
    m_chart->setMinimumWidth(300);
    m_chart->xAxis->setTickStep(1.0);
    m_chart->xAxis->setAutoTickStep(false);
    m_chart->addGraph();

    m_chart->graph(0)->setLineStyle(QCPGraph::lsLine);

    if (Agros2D::problem()->isNonlinear())
    {
        m_chart->setVisible(true);

        // axes
        m_chart->xAxis->setLabel(tr("iteration"));
        m_chart->yAxis->setLabel(tr("error"));
    }

    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *layoutStatus = new QHBoxLayout();
    layoutStatus->addWidget(memoryLabel, 1, Qt::AlignLeft);
    layoutStatus->addWidget(btnClose, 0, Qt::AlignRight);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(logWidget, 1);
    layoutHorizontal->addWidget(m_chart, 0);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    layout->addLayout(layoutStatus);

    setLayout(layout);
}

void LogDialog::printMessage(const QString &module, const QString &message, bool escaped)
{
    if (Agros2D::problem()->isNonlinear())
    {
        QString strNewton = "residual norm:";
        QString strPicard = "relative error:";

        if (module == tr("Solver") && (message.contains(strNewton) || message.contains(strPicard)))
        {
            bool ok = false;
            double error = 0.0;
            if (message.contains(strNewton))
                error = (message.right(message.length() - (message.indexOf(strNewton) + strNewton.length() + 1))).toDouble(&ok);
            else if (message.contains(strPicard))
                error = (message.right(message.length() - (message.indexOf(strPicard) + strPicard.length() + 1))).toDouble(&ok);

            if (ok)
            {
                m_chartStep.append(m_chartStep.count() + 1);
                m_chartNorm.append(error);

                m_chart->graph(0)->setData(m_chartStep, m_chartNorm);
                m_chart->rescaleAxes();
                m_chart->replot();
            }
        }
        else
        {
            // m_chartStep.clear();
            // m_chartNorm.clear();
        }
    }

    refreshStatus();
}

void LogDialog::refreshStatus()
{
    int memory = getCurrentRSS() / 1024 / 1024;

    memoryLabel->setText(tr("Process Memory: %1 MB").arg(memory));
    logWidget->repaint();
    // memoryLabel->repaint();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

// *******************************************************************************************

LogStdOut::LogStdOut(QWidget *parent) : QObject(parent)
{
    connect(Agros2D::log(), SIGNAL(messageMsg(QString, QString, bool)), this, SLOT(printMessage(QString, QString, bool)));
    connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString, bool)), this, SLOT(printError(QString, QString, bool)));
    connect(Agros2D::log(), SIGNAL(warningMsg(QString, QString, bool)), this, SLOT(printWarning(QString, QString, bool)));
    connect(Agros2D::log(), SIGNAL(debugMsg(QString, QString, bool)), this, SLOT(printDebug(QString, QString, bool)));
}

void LogStdOut::printMessage(const QString &module, const QString &message, bool escaped)
{
    qWarning() << QString("%1: %2").arg(module).arg(message);
}

void LogStdOut::printError(const QString &module, const QString &message, bool escaped)
{
    qCritical() << QString("%1: %2").arg(module).arg(message);
}

void LogStdOut::printWarning(const QString &module, const QString &message, bool escaped)
{
    qWarning() << QString("%1: %2").arg(module).arg(message);
}

void LogStdOut::printDebug(const QString &module, const QString &message, bool escaped)
{
    qDebug() << QString("%1: %2").arg(module).arg(message);
}

