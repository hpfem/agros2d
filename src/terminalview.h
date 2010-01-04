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

#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include "util.h"
#include "scripteditordialog.h"

class TerminalLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    TerminalLineEdit(QWidget *parent = 0);
    inline QStringListModel *model() { return m_model; }

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    int m_index;
    QStringListModel *m_model;
};

class Terminal : public QWidget
{
    Q_OBJECT
public:
    Terminal(QWidget *parent = 0);
    ~Terminal();

public slots:
    void doWriteResult(ScriptResult result);
    void doPrintStdout(const QString &message, QColor color = Qt::blue);

private:
    TerminalLineEdit *txtCommand;
    QTextEdit *txtOutput;
    QPushButton *btnExecute;
    QPushButton *btnClear;

protected:
    void focusInEvent(QFocusEvent *event);

private slots:
    void doExecute();
    void doCommandTextChanged(const QString &str);
};

// ****************************************************************************************************

class TerminalView : public QDockWidget
{
    Q_OBJECT
public:
    TerminalView(QWidget *parent = 0);

    inline Terminal *terminal() { return m_terminal; }

protected:
    void focusInEvent(QFocusEvent *event);

private:
    Terminal *m_terminal;
};

#endif // TERMINALVIEW_H
