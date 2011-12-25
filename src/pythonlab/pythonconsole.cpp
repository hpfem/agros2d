/*
*
*  Copyright (C) 2010 MeVis Medical Solutions AG All Rights Reserved.
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  Further, this software is distributed without any warranty that it is
*  free of the rightful claim of any third person regarding infringement
*  or the like.  Any license provided herein, whether implied or
*  otherwise, applies only to this software file.  Patent licenses, if
*  any, provided herein do not apply to combinations of this program with
*  other software, or any other product whatsoever.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*  Contact information: MeVis Medical Solutions AG, Universitaetsallee 29,
*  28359 Bremen, Germany or:
*
*  http://www.mevis.de
*
*/

#include "pythonconsole.h"
#include "pythonengine.h"

#include <QtGui>

#include <iostream>
#include <Python.h>

PythonScriptingConsole::PythonScriptingConsole(PythonEngine *pythonEngine, QWidget* parent)
    : QTextEdit(parent), pythonEngine(pythonEngine)
{    
    // default char format
    m_defaultTextCharacterFormat = currentCharFormat();

    // _context = context;
    m_historyPosition = 0;
    m_hasError = false;

    m_completer = new QCompleter(this);
    m_completer->setWidget(this);
    QObject::connect(m_completer, SIGNAL(activated(const QString&)),
                     this, SLOT(insertCompletion(const QString&)));

    connect(pythonEngine, SIGNAL(printStdOut(QString)),
            this, SLOT(stdOut(QString)));

    // HACK
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(font.pointSize() - 2);
    setFont(font);

    clear();

    QSettings settings;
    m_history = settings.value("PythonScriptingConsole/RecentFiles").value<QStringList>();
    m_historyPosition = m_history.length();
}

PythonScriptingConsole::~PythonScriptingConsole()
{
    // strip history
    while (m_history.count() > 50)
        m_history.removeFirst();

    QSettings settings;
    settings.setValue("PythonScriptingConsole/RecentFiles", m_history);
}

void PythonScriptingConsole::stdOut(const QString& str)
{
    m_stdOut += str;

    int idx;
    while ((idx = m_stdOut.indexOf('\n')) != -1)
    {
        consoleMessage(m_stdOut.left(idx), Qt::darkGreen);
        // std::cout << m_stdOut.left(idx).toLatin1().data() << std::endl;
        m_stdOut = m_stdOut.mid(idx+1);
    }
}

void PythonScriptingConsole::stdErr(const QString& str)
{
    m_hasError = true;
    m_stdErr += str;

    int idx;
    while ((idx = m_stdErr.indexOf('\n')) != -1)
    {
        consoleMessage(m_stdErr.left(idx), Qt::red);
        // std::cerr << m_stdErr.left(idx).toLatin1().data() << std::endl;
        m_stdErr = m_stdErr.mid(idx+1);
    }
}

void PythonScriptingConsole::flushStdOut()
{
    QApplication::processEvents();

    if (!m_stdOut.isEmpty())
    {
        stdOut("\n");
    }
    if (!m_stdErr.isEmpty())
    {
        stdErr("\n");
    }

    QApplication::processEvents();
}

void PythonScriptingConsole::clear()
{    
    QTextEdit::clear();

    pythonEngine->runPythonScript("v = sys.version + \" on \" + sys.platform; print(v); del v;", "");

    appendCommandPrompt();
}

void PythonScriptingConsole::clearCommandLine()
{
    // Select the text after the last command prompt ...
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(commandPromptPosition(), QTextCursor::KeepAnchor);

    textCursor.insertText("");

    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);
}

void PythonScriptingConsole::executeLine(const QString &str, bool storeOnly)
{
    QString code = str;

    // i don't know where this trailing space is coming from, blast it!
    if (code.endsWith(" ")) {
        code.truncate(code.length()-1);
    }

    if (!code.isEmpty())
    {
        // Update the history
        m_history << code;
        m_historyPosition = m_history.count();
        m_currentMultiLineCode += code + "\n";

        emit historyChanged(code);

        if (!storeOnly)
        {
            executeCode(m_currentMultiLineCode);
            m_currentMultiLineCode = "";
        }
    }
    // Insert a new command prompt
    appendCommandPrompt(storeOnly);
}

void PythonScriptingConsole::executeLine(bool storeOnly)
{
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);

    // Select the text from the command prompt until the end of the block
    // and get the selected text.
    textCursor.setPosition(commandPromptPosition());
    textCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString code = textCursor.selectedText();

    executeLine(code, storeOnly);
}

void PythonScriptingConsole::executeCode(const QString& code)
{
    // put visible cursor to the end of the line
    QTextCursor cursor = QTextEdit::textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);

    int cursorPosition = this->textCursor().position();

    // evaluate the code
    m_stdOut = "";
    m_stdErr = "";

    ExpressionResult result = pythonEngine->runPythonExpression(code, false);

    if (!result.error.isEmpty())
        stdErr(result.error);

    flushStdOut();

    // bool messageInserted = (this->textCursor().position() != cursorPosition);
}

void PythonScriptingConsole::appendCommandPrompt(bool storeOnly)
{
    if (storeOnly)
    {
        append("<b><span style=\"color: #0000A0;\">...></span></b> ");
        m_commandPromptLength = 5;
    }
    else
    {
        append("<b><span style=\"color: #0000A0;\">>>></span></b> ");
        m_commandPromptLength = 4;
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

int PythonScriptingConsole::commandPromptPosition() {

    QTextCursor textCursor(this->textCursor());
    textCursor.movePosition(QTextCursor::End);

    return textCursor.block().position() + m_commandPromptLength;
}

void PythonScriptingConsole::insertCompletion(const QString& completion)
{
    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    if (tc.selectedText() == ".")
    {
        tc.insertText(QString(".") + completion);
    }
    else
    {
        tc = textCursor();
        tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        tc.insertText(completion);
        setTextCursor(tc);
    }
}

void PythonScriptingConsole::handleTabCompletion()
{
    QTextCursor textCursor   = this->textCursor();
    int pos = textCursor.position();
    textCursor.setPosition(commandPromptPosition());
    textCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    int startPos = textCursor.selectionStart();

    int offset = pos-startPos;
    QString text = textCursor.selectedText();

    QString textToComplete;
    int cur = offset;
    while (cur--)
    {
        QChar c = text.at(cur);
        if (c.isLetterOrNumber() || c == '.' || c == '_')
        {
            textToComplete.prepend(c);
        }
        else
        {
            break;
        }
    }

    QString lookup;
    QString compareText = textToComplete;
    int dot = compareText.lastIndexOf('.');
    if (dot!=-1)
    {
        lookup = compareText.mid(0, dot);
        compareText = compareText.mid(dot+1, offset);
    }
    if (!lookup.isEmpty() || !compareText.isEmpty())
    {
        compareText = compareText.toLower();
        QStringList found;
        QStringList l = QStringList(); //TODO PythonQt::self()->introspection(_context, lookup, PythonQt::Anything);
        foreach (QString n, l)
        {
            if (n.toLower().startsWith(compareText))
            {
                found << n;
            }
        }

        if (!found.isEmpty())
        {
            m_completer->setCompletionPrefix(compareText);
            m_completer->setCompletionMode(QCompleter::PopupCompletion);
            m_completer->setModel(new QStringListModel(found, m_completer));
            m_completer->setCaseSensitivity(Qt::CaseInsensitive);
            QTextCursor c = this->textCursor();
            c.movePosition(QTextCursor::StartOfWord);
            QRect cr = cursorRect(c);
            cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
                        + m_completer->popup()->verticalScrollBar()->sizeHint().width());
            cr.translate(0,8);
            m_completer->complete(cr);
        }
        else
        {
            m_completer->popup()->hide();
        }
    }
    else
    {
        m_completer->popup()->hide();
    }
}

void PythonScriptingConsole::keyPressEvent(QKeyEvent* event) {

    if (m_completer && m_completer->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (event->key())
        {
        case Qt::Key_Return:
            if (!m_completer->popup()->currentIndex().isValid())
            {
                insertCompletion(m_completer->currentCompletion());
                m_completer->popup()->hide();
                event->accept();
            }
            event->ignore();
            return;
            break;
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:

            event->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    bool eventHandled = false;
    QTextCursor textCursor = this->textCursor();

    int key = event->key();
    switch (key)
    {
    case Qt::Key_Left:

        // Moving the cursor left is limited to the position
        // of the command prompt.

        if (textCursor.position() <= commandPromptPosition())
        {
            QApplication::beep();
            eventHandled = true;
        }
        break;

    case Qt::Key_Up:

        // Display the previous command in the history
        if (m_historyPosition > 0)
        {
            m_historyPosition--;
            changeHistory();
        }

        eventHandled = true;
        break;

    case Qt::Key_Down:

        // clean input
        if (m_historyPosition+1 == m_history.count())
        {
            clearCommandLine();
            m_historyPosition = m_history.count();
        }

        // Display the next command in the history
        if (m_historyPosition+1 < m_history.count())
        {
            m_historyPosition++;
            changeHistory();
        }

        eventHandled = true;
        break;

    case Qt::Key_Return:

        executeLine(event->modifiers() & Qt::ShiftModifier);
        eventHandled = true;
        break;

    case Qt::Key_Backspace:

        if (textCursor.hasSelection())
        {
            cut();
            eventHandled = true;
        }
        else
        {
            // Intercept backspace key event to check if
            // deleting a character is allowed. It is not
            // allowed, if the user wants to delete the
            // command prompt.

            if (textCursor.position() <= commandPromptPosition())
            {
                QApplication::beep();
                eventHandled = true;
            }
        }
        break;

    case Qt::Key_Delete:

        if (textCursor.hasSelection())
        {
            cut();
            eventHandled = true;
        }
        else
        {
            int commandPromptPosition = this->commandPromptPosition();
            if (textCursor.position() < commandPromptPosition)
            {
                textCursor.movePosition(QTextCursor::End);
                setTextCursor(textCursor);
            }
        }

        break;

    case Qt::Key_Home:

    {
        QTextCursor textCursor = this->textCursor();
        textCursor.setPosition(commandPromptPosition(),
                               (event->modifiers() & Qt::ShiftModifier) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);

        setTextCursor(textCursor);
        eventHandled = true;
    }
        break;

    default:
        if (key >= Qt::Key_Space && key <= Qt::Key_division)
        {
            if (textCursor.hasSelection() && !verifySelectionBeforeDeletion())
            {
                // The selection must not be deleted.
                eventHandled = true;
            }
            else
            {
                // The key is an input character, check if the cursor is
                // behind the last command prompt, else inserting the
                // character is not allowed.

                int commandPromptPosition = this->commandPromptPosition();
                if (textCursor.position() < commandPromptPosition)
                {
                    textCursor.setPosition(commandPromptPosition);
                    setTextCursor(textCursor);
                }
            }
        }
    }

    if (eventHandled)
    {
        m_completer->popup()->hide();
        event->accept();
    }
    else
    {
        QTextEdit::keyPressEvent(event);
        QString text = event->text();
        if (!text.isEmpty())
        {
            handleTabCompletion();
        }
        else
        {
            m_completer->popup()->hide();
        }
        eventHandled = true;
    }
}

void PythonScriptingConsole::cut()
{
    bool deletionAllowed = verifySelectionBeforeDeletion();
    if (deletionAllowed)
    {
        QTextEdit::cut();
    }
}

bool PythonScriptingConsole::verifySelectionBeforeDeletion()
{
    bool deletionAllowed = true;

    QTextCursor textCursor = this->textCursor();

    int commandPromptPosition = this->commandPromptPosition();
    int selectionStart = textCursor.selectionStart();
    int selectionEnd = textCursor.selectionEnd();

    if (textCursor.hasSelection())
    {

        // Selected text may only be deleted after the last command prompt.
        // If the selection is partly after the command prompt set the selection
        // to the part and deletion is allowed. If the selection occurs before the
        // last command prompt, then deletion is not allowed.

        if (selectionStart < commandPromptPosition ||
                selectionEnd < commandPromptPosition)
        {

            // Assure selectionEnd is bigger than selection start
            if (selectionStart > selectionEnd)
            {
                int tmp         = selectionEnd;
                selectionEnd    = selectionStart;
                selectionStart  = tmp;
            }

            if (selectionEnd < commandPromptPosition)
            {

                // Selection is completely before command prompt,
                // so deletion is not allowed.
                QApplication::beep();
                deletionAllowed = false;

            }
            else
            {

                // The selectionEnd is after the command prompt, so set
                // the selection start to the commandPromptPosition.
                selectionStart = commandPromptPosition;
                textCursor.setPosition(selectionStart);
                textCursor.setPosition(selectionStart, QTextCursor::KeepAnchor);
                setTextCursor(textCursor);
            }
        }

    }
    else
    { // if (hasSelectedText())

        // When there is no selected text, deletion is not allowed before the
        // command prompt.
        if (textCursor.position() < commandPromptPosition)
        {
            QApplication::beep();
            deletionAllowed = false;
        }
    }

    return deletionAllowed;
}

void PythonScriptingConsole::changeHistory()
{
    // Select the text after the last command prompt ...
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(commandPromptPosition(), QTextCursor::KeepAnchor);

    // ... and replace it with the history text.
    textCursor.insertText(m_history.value(m_historyPosition));

    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);
}

void PythonScriptingConsole::consoleMessage(const QString &message, const QColor& color)
{
    append(QString());

    QString str;

    str = "<span style=\"color: " + color.name() + ";\">";
    str += Qt::escape(message);
    str += "</span>";

    insertHtml(str);

    // Reset all font modifications done by the html string
    setCurrentCharFormat(m_defaultTextCharacterFormat);
}

// ***********************************************************************************************

PythonScriptingConsoleView::PythonScriptingConsoleView(PythonEngine *pythonEngine, QWidget *parent)
    : QDockWidget(tr("Console"), parent)
{
    setMinimumWidth(280);
    setObjectName("ConsoleView");

    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(doVisibilityChanged(bool)));

    m_console = new PythonScriptingConsole(pythonEngine, this);

    setWidget(m_console);
}

PythonScriptingConsoleView::~PythonScriptingConsoleView()
{
    delete m_console;
}

void PythonScriptingConsoleView::focusInEvent(QFocusEvent *event)
{
    m_console->setFocus(event->reason());
}

void PythonScriptingConsoleView::doVisibilityChanged(bool)
{
    if (isVisible())
        m_console->setFocus();
}

// ***********************************************************************************************

PythonScriptingHistoryView::PythonScriptingHistoryView(PythonScriptingConsole *console, QWidget *parent)
    : QDockWidget(tr("History"), parent), console(console)
{
    setMinimumWidth(280);
    setObjectName("ConsoleHistoryView");

    connect(console, SIGNAL(historyChanged(QString)), this, SLOT(historyChanged(QString)));

    trvHistory = new QTreeWidget(this);
    trvHistory->setHeaderHidden(true);
    trvHistory->setIndentation(2);

    if (console->history().count() > 0)
        historyChanged(console->history().last());

    connect(trvHistory, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(executeCommand(QTreeWidgetItem *, int)));

    setWidget(trvHistory);
}

void PythonScriptingHistoryView::historyChanged(const QString &code)
{
    trvHistory->clear();


    if (console->history().count() > 0)
    {
        foreach (QString historyItem, console->history())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(trvHistory);
            item->setText(0, historyItem);
            item->setIcon(0, icon("history-command"));

            // ensure visible
            trvHistory->scrollToItem(item);
        }
    }
}

void PythonScriptingHistoryView::executeCommand(QTreeWidgetItem *item, int role)
{
    if (item)
    {
        QString command = item->text(0);

        console->clearCommandLine();
        console->insertPlainText(command);
        console->executeLine(command, false);
    }
}
