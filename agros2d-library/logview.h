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
namespace ctemplate
{
    class TemplateDictionary;
}

class AGROS_LIBRARY_API Log: public QObject
{
    Q_OBJECT
public:
    Log();

    inline void printHeading(const QString &message) { emit headingMsg(message); }
    inline void printMessage(const QString &module, const QString &message) { emit messageMsg(module, message); }
    inline void printError(const QString &module, const QString &message) { emit errorMsg(module, message); }
    inline void printWarning(const QString &module, const QString &message) { emit warningMsg(module, message); }
    inline void printDebug(const QString &module, const QString &message) { emit debugMsg(module, message); }

    inline void setNonlinearTable(QVector<double> step, QVector<double> error) { emit nonlinearTable(step, error); }
    inline void setAdaptivityTable(QVector<double> step, QVector<double> error) { emit nonlinearTable(step, error); }

signals:
    void headingMsg(const QString &message);
    void messageMsg(const QString &module, const QString &message);
    void errorMsg(const QString &module, const QString &message);
    void warningMsg(const QString &module, const QString &message);
    void debugMsg(const QString &module, const QString &message);

    void nonlinearTable(QVector<double> step, QVector<double> error);
    void adaptivityTable(QVector<double> step, QVector<double> error);
};

class AGROS_LIBRARY_API LogWidget : public QWidget
{
    Q_OBJECT
public:
    LogWidget(QWidget *parent = 0);
    ~LogWidget();

    void welcomeMessage();

    bool isMemoryLabelVisible() const;
    void setMemoryLabelVisible(bool visible = true);

private slots:
    void contextMenu(const QPoint &pos);

    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);
    void printHeading(const QString &message);

    void showTimestamp();
    void showDebug();

protected:
    ctemplate::TemplateDictionary *logInfo;

    void print(const QString &module, const QString &message,
               const QString &color = "");

private:
    QMenu *mnuInfo;

    QWebView *webView;
    QString m_cascadeStyleSheet;

    QAction *actShowTimestamp;
    QAction *actShowDebug;
    QAction *actClear;

    QLabel *memoryLabel;
    int m_printCounter;

    void createActions();

private slots:
    void refreshMemory(int usage);
    void initWebView();
};

class LogView : public QDockWidget
{
    Q_OBJECT
public:
    LogView(QWidget *parent = 0);

private:
    LogWidget *logWidget;
};

class AGROS_LIBRARY_API LogDialog : public QDialog
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

    QPushButton *btnClose;
    QPushButton *btnAbort;

    QCustomPlot *m_nonlinearChart;
    QCustomPlot *m_adaptivityChart;

    void createControls();

private slots:    
    void printError(const QString &module, const QString &message);

    void nonlinearTable(QVector<double> step, QVector<double> error);
    void adaptivityTable(QVector<double> step, QVector<double> error);

    void tryClose();
};

class AGROS_LIBRARY_API LogStdOut : public QObject
{
    Q_OBJECT
public:
    LogStdOut(QWidget *parent = 0);

private slots:
    void printHeading(const QString &message);
    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);
};

#endif // TOOLTIPVIEW_H
