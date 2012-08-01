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
#include "scene.h"
#include "gui.h"

#include "hermes2d/problem.h"

Log::Log()
{
}

// *******************************************************************************************************

LogWidget::LogWidget(QWidget *parent) : QTextEdit(parent)
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

    connect(Util::log(), SIGNAL(messageMsg(QString, QString)), this, SLOT(printMessage(QString, QString)));
    connect(Util::log(), SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(Util::log(), SIGNAL(warningMsg(QString, QString)), this, SLOT(printWarning(QString, QString)));
    connect(Util::log(), SIGNAL(debugMsg(QString, QString)), this, SLOT(printDebug(QString, QString)));

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
    actShowTimestamp->setChecked(settings.value("LogWidget/ShowTimestamp", true).toBool());
    connect(actShowTimestamp, SIGNAL(triggered()), this, SLOT(showTimestamp()));

    actShowDebug = new QAction(icon(""), tr("Show debug"), this);
    actShowDebug->setCheckable(true);
    actShowDebug->setChecked(settings.value("LogWidget/ShowDebug", true).toBool());
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
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);

    append(QString());

    QString str;

    // debug - timestamp
    if (actShowTimestamp->isChecked())
    {
        str += "<span style=\"color: gray;\">";
        // str += Qt::escape(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") + ": ");
        str += Qt::escape(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ");
        str += "</span>";
    }

    // message
    str += "<span style=\"color: " + color + ";\">";
    str += "<strong>" + Qt::escape(module) + "</strong>: ";
    str += Qt::escape(message);
    str += "</span>";

    insertHtml(str);

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void LogWidget::welcomeMessage()
{
    print("Agros2D", tr("version: %1").arg(QApplication::applicationVersion()), "green");
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
    connect(Util::log(), SIGNAL(messageMsg(QString, QString)), this, SLOT(printMessage(QString, QString)));

    logWidget = new LogWidget(this);

    m_chart = new Chart(this, true);
    m_chart->setVisible(false);
    m_chart->setMinimumWidth(300);

    if (Util::problem()->isNonlinear())
    {
        m_chart->setVisible(true);

        // axes
        m_chart->setAxisTitle(QwtPlot::yLeft, tr("error"));
        m_chart->setAxisTitle(QwtPlot::xBottom, tr("iteration"));
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(logWidget, 1);
    layoutHorizontal->addWidget(m_chart, 0);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void LogDialog::printMessage(const QString &module, const QString &message)
{
    if (Util::problem()->isNonlinear())
    {
        QString strNewton = "residual norm:";
        QString strPicard = "rel. error:";

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
                m_chartStep.append(m_chartStep.length() + 1);
                m_chartNorm.append(error);

                m_chart->setData(m_chartStep, m_chartNorm);
            }
        }
        else
        {
            // m_chartStep.clear();
            // m_chartNorm.clear();
        }
    }    
}

// *******************************************************************************************

LogStdOut::LogStdOut(QWidget *parent) : QObject(parent)
{
    connect(Util::log(), SIGNAL(messageMsg(QString, QString)), this, SLOT(printMessage(QString, QString)));
    connect(Util::log(), SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(Util::log(), SIGNAL(warningMsg(QString, QString)), this, SLOT(printWarning(QString, QString)));
    connect(Util::log(), SIGNAL(debugMsg(QString, QString)), this, SLOT(printDebug(QString, QString)));
}

void LogStdOut::printMessage(const QString &module, const QString &message)
{
    qWarning() << QString("%1: %2").arg(module).arg(message);
}

void LogStdOut::printError(const QString &module, const QString &message)
{
    qCritical() << QString("%1: %2").arg(module).arg(message);
}

void LogStdOut::printWarning(const QString &module, const QString &message)
{
    qWarning() << QString("%1: %2").arg(module).arg(message);
}

void LogStdOut::printDebug(const QString &module, const QString &message)
{
    qDebug() << QString("%1: %2").arg(module).arg(message);
}

