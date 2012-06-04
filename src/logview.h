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

class Log: public QObject
{
    Q_OBJECT
public:
    Log();

    inline void printMessage(const QString &module, const QString &message) { emit messageMsg(module, message); }
    inline void printError(const QString &module, const QString &message) { emit errorMsg(module, message); }
    inline void printWarning(const QString &module, const QString &message) { emit warningMsg(module, message); }
    inline void printDebug(const QString &module, const QString &message) { emit debugMsg(module, message); }

signals:
    void messageMsg(const QString &module, const QString &message);
    void errorMsg(const QString &module, const QString &message);
    void warningMsg(const QString &module, const QString &message);
    void debugMsg(const QString &module, const QString &message);
};

class LogWidget : public QTextEdit
{
    Q_OBJECT
public:
    LogWidget(QWidget *parent = 0);

    void welcomeMessage();

private slots:
    void contextMenu(const QPoint &pos);

    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);


    void showTimestamp();
    void showDebug();

protected:
     void print(const QString &module, const QString &message, const QString &color = false);

private:
     QMenu *mnuInfo;

     QAction *actShowTimestamp;
     QAction *actShowDebug;
     QAction *actClear;

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

class LogDialog : public QDialog
{
    Q_OBJECT
public:
    LogDialog(QWidget *parent = 0, const QString &title = tr("Progress..."));
    ~LogDialog();

private:
     LogWidget *logWidget;

     QwtPlotCurve *m_curve;
     Chart *m_chartNewton;
     QList<double> m_chartStep;
     QList<double> m_chartNorm;

     void createControls();

private slots:
     void printDebug(const QString &module, const QString &message);
};

class LogStdOut : public QObject
{
    Q_OBJECT
public:
    LogStdOut(QWidget *parent = 0);

private slots:
    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);
};

#endif // TOOLTIPVIEW_H
