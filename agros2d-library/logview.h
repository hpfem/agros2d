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

#ifndef TOOLTIPVIEW_H
#define TOOLTIPVIEW_H

#include "util.h"

class QCustomPlot;

class AGROS_API Log: public QObject
{
    Q_OBJECT
public:
    Log();

    inline void printMessage(const QString &module, const QString &message, bool escaped = true) { emit messageMsg(module, message, escaped); }
    inline void printError(const QString &module, const QString &message, bool escaped = true) { emit errorMsg(module, message, escaped); }
    inline void printWarning(const QString &module, const QString &message, bool escaped = true) { emit warningMsg(module, message, escaped); }
    inline void printDebug(const QString &module, const QString &message, bool escaped = true) { emit debugMsg(module, message, escaped); }

    inline void setNonlinearTable(QVector<double> step, QVector<double> error) { emit nonlinearTable(step, error); }
    inline void setAdaptivityTable(QVector<double> step, QVector<double> error) { emit nonlinearTable(step, error); }

signals:
    void messageMsg(const QString &module, const QString &message, bool escaped);
    void errorMsg(const QString &module, const QString &message, bool escaped);
    void warningMsg(const QString &module, const QString &message, bool escaped);
    void debugMsg(const QString &module, const QString &message, bool escaped);

    void nonlinearTable(QVector<double> step, QVector<double> error);
    void adaptivityTable(QVector<double> step, QVector<double> error);
};

class AGROS_API LogWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    LogWidget(QWidget *parent = 0);

    void welcomeMessage();

private slots:
    void contextMenu(const QPoint &pos);

    void printMessage(const QString &module, const QString &message, bool escaped = true);
    void printError(const QString &module, const QString &message, bool escaped = true);
    void printWarning(const QString &module, const QString &message, bool escaped = true);
    void printDebug(const QString &module, const QString &message, bool escaped = true);

    void showTimestamp();
    void showDebug();

protected:
    void print(const QString &module, const QString &message,
               const QString &color = "", bool escaped = true);

private:
     QMenu *mnuInfo;

     QAction *actShowTimestamp;
     QAction *actShowDebug;
     QAction *actClear;
     QAction *actCopy;

     void createActions();
};

class LogView : public QDockWidget
{
    Q_OBJECT
public:
    LogView(QWidget *parent = 0);

private:
     LogWidget *logWidget;
};

class AGROS_API LogDialog : public QDialog
{
    Q_OBJECT
public:
    LogDialog(QWidget *parent = 0, const QString &title = tr("Progress..."));
    ~LogDialog();

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void reject();

private:
     LogWidget *logWidget;

     QLabel *memoryLabel;

     QCustomPlot *m_nonlinearChart;
     QCustomPlot *m_adaptivityChart;

     void createControls();

private slots:
     void printMessage(const QString &module, const QString &message, bool escaped = true);
     void refreshStatus();

     void nonlinearTable(QVector<double> step, QVector<double> error);
     void adaptivityTable(QVector<double> step, QVector<double> error);
};

class AGROS_API LogStdOut : public QObject
{
    Q_OBJECT
public:
    LogStdOut(QWidget *parent = 0);

private slots:
    void printMessage(const QString &module, const QString &message, bool escaped = true);
    void printError(const QString &module, const QString &message, bool escaped = true);
    void printWarning(const QString &module, const QString &message, bool escaped = true);
    void printDebug(const QString &module, const QString &message, bool escaped = true);
};

#endif // TOOLTIPVIEW_H
