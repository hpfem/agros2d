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

#include "terminalview.h"

#include "scene.h"
#include "pythonlabagros.h"

TerminalLineEdit::TerminalLineEdit(QWidget *parent) : QLineEdit(parent)
{
    logMessage("TerminalLineEdit::TerminalLineEdit()");

    m_model = dynamic_cast<QStringListModel *>(Util::completer()->model());
    m_index = -1;

    connect(this, SIGNAL(returnPressed()), this, SLOT(doClear()));
}

void TerminalLineEdit::keyPressEvent(QKeyEvent *event)
{
    logMessage("TerminalLineEdit::keyPressEvent()");

    if (event->key() == Qt::Key_Up)
    {
        m_index++;
        if (m_index > (m_model->stringList().count()-1))
            m_index = (m_model->stringList().count()-1);
    }
    if (event->key() == Qt::Key_Down)
    {
        m_index--;
        if (m_index < 0)
            m_index = -1;
    }
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
    {
        if (m_index >= 0 && m_index < m_model->stringList().count())
        {
            setText(m_model->stringList().at(m_index));
        }
        else
        {
            m_index = -1;
            setText("");
        }
    }

    QLineEdit::keyPressEvent(event);
}

void TerminalLineEdit::doClear()
{
    logMessage("TerminalLineEdit::doClear()");

    m_index = -1;
}

// **********************************************************************************************************************

Terminal::Terminal(QWidget *parent) : QWidget(parent)
{
    logMessage("Terminal::Terminal()");

    setWindowTitle(tr("Command dialog"));
    setWindowIcon(icon("run"));

    txtCommand = new TerminalLineEdit(this);
    txtCommand->setCompleter(Util::completer());
    txtCommand->setMinimumWidth(300);
    connect(txtCommand, SIGNAL(returnPressed()), this, SLOT(doExecute()));
    connect(txtCommand, SIGNAL(textChanged(QString)), this, SLOT(doCommandTextChanged(QString)));

    txtOutput = new QTextEdit(this);
    txtOutput->setLineWrapMode (QTextEdit::NoWrap);
#ifndef Q_WS_MAC
    txtOutput->setFont(QFont("Monospaced", 9));
#endif
    txtOutput->setReadOnly(true);    

    btnExecute = new QPushButton(this);
    btnExecute->setText(tr("Execute"));
    btnExecute->setEnabled(false);
    connect(btnExecute, SIGNAL(clicked()), this, SLOT(doExecute()));

    btnClear = new QPushButton(this);
    btnClear->setText(tr("Clear"));
    connect(btnClear, SIGNAL(clicked()), txtOutput, SLOT(clear()));

    // layout
    QHBoxLayout *layoutCommand = new QHBoxLayout();
    layoutCommand->addWidget(new QLabel(tr("Enter command:")));
    layoutCommand->addWidget(txtCommand, 1);
    layoutCommand->addWidget(btnExecute);
    layoutCommand->addWidget(btnClear);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtOutput);
    layout->addLayout(layoutCommand);
    txtCommand->setFocus();

    setTabOrder(txtCommand, btnExecute);
    setTabOrder(btnExecute, btnClear);
    setTabOrder(btnClear, txtCommand);

    setLayout(layout);
}

Terminal::~Terminal()
{
    logMessage("Terminal::~Terminal()");

    delete txtCommand;
    delete txtOutput;
}

void Terminal::focusInEvent(QFocusEvent *event)
{
    logMessage("Terminal::focusInEvent()");

    txtCommand->setFocus(event->reason());
    QWidget::focusInEvent(event);
}

void Terminal::doExecute()
{
    logMessage("Terminal::doExecute()");

    if (!txtCommand->text().isEmpty())
    {
        QSettings settings;
        QStringList list = settings.value("CommandDialog/RecentCommands").value<QStringList>();

        list.insert(0, txtCommand->text());

        // remove last item (over 50), empty strings and duplicates
        list.removeAll("");
        // list.removeDuplicates();
        while (list.count() > 50)
            list.removeAt(list.count()-1);

        txtCommand->model()->setStringList(list);
        settings.setValue("CommandDialog/RecentCommands", list);

        // command
        doPrintStdout(">>> " + txtCommand->text() + "\n", Qt::black);

        // execute command
        doWriteResult(runPythonScript(txtCommand->text()));
    }
    txtCommand->clear();
    txtCommand->setFocus();
}

void Terminal::doCommandTextChanged(const QString &str)
{
    logMessage("Terminal::doCommandTextChanged()");

    btnExecute->setEnabled(!str.isEmpty());
}

void Terminal::doWriteResult(ScriptResult result)
{
    logMessage("Terminal::doWriteResult()");

    QColor color = Qt::blue;
    if (result.isError)
        color = (Qt::red);

    doPrintStdout(result.text.trimmed() + "\n", color);
}

void Terminal::doPrintStdout(const QString &message, QColor color)
{
    logMessage("Terminal::doPrintStdout()");

    if (!message.trimmed().isEmpty())
    {
        // format
        QTextCharFormat format;
        format.setForeground(color);

        // cursor
        QTextCursor cursor = txtOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.beginEditBlock();
        cursor.insertText(message, format);
        cursor.endEditBlock();

        // output
        txtOutput->setTextCursor(cursor);
        txtOutput->ensureCursorVisible();
        QApplication::processEvents();
    }
}

// ****************************************************************************************************

TerminalView::TerminalView(QWidget *parent) : QDockWidget(tr("Terminal"), parent)
{
    logMessage("TerminalView::TerminalView()");

    setMinimumWidth(280);
    setObjectName("TerminalView");

    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(doVisibilityChanged(bool)));

    m_terminal = new Terminal(this);

    setWidget(m_terminal);
}

void TerminalView::focusInEvent(QFocusEvent *event)
{
    logMessage("TerminalView::focusInEvent()");

    m_terminal->setFocus(event->reason());
}

void TerminalView::doVisibilityChanged(bool)
{
    logMessage("TerminalView::doVisibilityChanged()");

    if (isVisible())
        m_terminal->setFocus();
}
