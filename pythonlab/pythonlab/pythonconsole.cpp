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
#include "pythoneditor.h"
#include "pythoncompleter.h"
#include "pythonbrowser.h"

#include <QtGui>

#include <iostream>
#include <Python.h>

// static history for all consoles
QStringList PythonScriptingConsole::history;
int PythonScriptingConsole::historyPosition;

PythonScriptingConsole::PythonScriptingConsole(PythonEngine *pythonEngine, QWidget* parent)
    : QTextEdit(parent), pythonEngine(pythonEngine)
{    
    // default char format
    m_defaultTextCharacterFormat = currentCharFormat();

    // _context = context;
    PythonScriptingConsole::historyPosition = 0;

    completer = createCompleter();
    completer->setWidget(this);
    QObject::connect(completer, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));

    setFont(FONT);

    welcomeMessage();

    QSettings settings;
    PythonScriptingConsole::history = settings.value("PythonScriptingConsole/History").value<QStringList>();
    PythonScriptingConsole::historyPosition = PythonScriptingConsole::history.length();
}

PythonScriptingConsole::~PythonScriptingConsole()
{
    // strip history
    while (PythonScriptingConsole::history.count() > 50)
        PythonScriptingConsole::history.removeFirst();

    QSettings settings;
    settings.setValue("PythonScriptingConsole/History", PythonScriptingConsole::history);
}

void PythonScriptingConsole::stdClear()
{
    QTextEdit::clear();
}

void PythonScriptingConsole::stdOut(const QString& str)
{
    QStringList strList = str.trimmed().split("\n");
    for (int i = 0; i < strList.count(); i++)
        consoleMessage(strList[i], Qt::darkGreen);
}

void PythonScriptingConsole::stdHtml(const QString& str)
{
    append(QString());
    insertHtml(str);
}

void PythonScriptingConsole::stdErr(const QString& str)
{
    QStringList strList = str.split("\n");
    for (int i = 0; i < strList.count(); i++)
        consoleMessage(strList[i], Qt::red);
}

void PythonScriptingConsole::stdImage(const QString &fileName)
{
    QString fn = fileName;
    if (!QFile::exists(fileName))
        fn = m_currentPath + QDir::separator() + fileName;

    if (QFile::exists(fn))
    {
        append(QString());

        // QUrl uri(QString("file://%1").arg(fn));
        QUrl uri(QString("%1").arg(fn));
        QImage image = QImageReader(fn).read();

        QTextDocument *textDocument = document();
        textDocument->addResource(QTextDocument::ImageResource, uri, QVariant (image));
        QTextCursor cursor = textCursor();
        QTextImageFormat imageFormat;
        imageFormat.setWidth(image.width());
        imageFormat.setHeight(image.height());
        imageFormat.setName(uri.toString());
        cursor.insertImage(imageFormat);

        // appendCommandPrompt();
    }
}

void PythonScriptingConsole::welcomeMessage()
{    
    QTextEdit::clear();

    connectStdOut();
    pythonEngine->runScript("import sys; v = sys.version + \" on \" + sys.platform; print(v); del v;", "");
    disconnectStdOut();
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
        PythonScriptingConsole::history << code;
        PythonScriptingConsole::historyPosition = PythonScriptingConsole::history.count();
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

    connectStdOut();
    ExpressionResult result = pythonEngine->runExpression(code, false);
    disconnectStdOut();

    if (!result.error.isEmpty())
    {
        stdErr(result.error);

        QSettings settings;
        if (settings.value("PythonEditorWidget/PrintStacktrace", true).toBool())
        {
            stdErr("\nStacktrace:");
            stdErr(result.traceback);
        }
    }

    QApplication::processEvents();

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

void PythonScriptingConsole::connectStdOut(const QString &currentPath)
{
    if (QDir(currentPath).exists())
        m_currentPath = currentPath;
    else
        m_currentPath = "";

    connect(pythonEngine, SIGNAL(pythonClear()), this, SLOT(stdClear()));
    connect(pythonEngine, SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(pythonEngine, SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));
    connect(pythonEngine, SIGNAL(pythonShowImage(QString)), this, SLOT(stdImage(QString)));
}

void PythonScriptingConsole::disconnectStdOut()
{
    disconnect(pythonEngine, SIGNAL(pythonClear()), this, SLOT(stdClear()));
    disconnect(pythonEngine, SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    disconnect(pythonEngine, SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));
    disconnect(pythonEngine, SIGNAL(pythonShowImage(QString)), this, SLOT(stdImage(QString)));
}

int PythonScriptingConsole::commandPromptPosition() {

    QTextCursor textCursor(this->textCursor());
    textCursor.movePosition(QTextCursor::End);

    return textCursor.block().position() + m_commandPromptLength;
}

void PythonScriptingConsole::insertCompletion(const QString& completion)
{
    QString str = completion.left(completion.indexOf("(") - 1);

    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    // if (tc.selectedText() == ".")
    // {
    //     tc.insertText(QString(".") + str);
    // }
    // else
    {
        tc = textCursor();
        tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        tc.insertText(str);
        setTextCursor(tc);
    }
}

void PythonScriptingConsole::handleTabCompletion(bool autoComplete)
{
    QTextCursor c = textCursor();
    int pos = c.position();
    c.setPosition(commandPromptPosition());
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QString search = c.selectedText();

    // code completion
    QStringList found = pythonEngine->codeCompletion(search.toLower(), search.length());
    // qDebug() << found.length();

    // add variables
    QList<PythonVariable> list = pythonEngine->variableList();
    if (!search.contains("."))
    {
        foreach (PythonVariable variable, list)
        {
            if (isPythonVariable(variable.type))
                found.append(QString("%1 (global, variable)").arg(variable.name));
            else if (variable.type == "function")
                found.append(QString("%1 (global, function)").arg(variable.name));
            else if (variable.type == "module")
                found.append(QString("%1 (global, module)").arg(variable.name));
            else
                found.append(QString("%1").arg(variable.name));
        }
    }

    found.sort();

    if (!found.isEmpty())
    {
        QString str = search.trimmed();

        for (int i = 33; i <= 126; i++)
        {
            // skip numbers and alphabet and dot
            if ((i >= 48 && i <= 57) || (i >= 65 && i <= 90) || (i >= 97 && i <= 122) || (i == 46))
                continue;

            QChar c(i);
            // qDebug() << c << ", " << str.lastIndexOf(c) << ", " << str.length();

            if (str.lastIndexOf(c) != -1)
            {
                str = str.right(str.length() - str.lastIndexOf(c) - 1);
                break;
            }
        }

        if (str.contains(".") && str.right(1) == ".")
            str = "";
        else
            str = str.right(str.length() - str.lastIndexOf(".") - 1);

        // qDebug() << str.trimmed();

        completer->setCompletionPrefix(str.trimmed());
        completer->setModel(new QStringListModel(found, completer));
        if (autoComplete && completer->completionCount() == 1)
        {
            // autocomplete
            insertCompletion(completer->currentCompletion());
            completer->popup()->hide();
        }
        else
        {
            // show completer
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::StartOfWord);
            QRect cr = cursorRect(c);
            cr.setWidth(completer->popup()->sizeHintForColumn(0)
                        + completer->popup()->verticalScrollBar()->sizeHint().width() + 30);
            cr.translate(0, 4);
            completer->complete(cr);
        }
    }
    else
    {
        completer->popup()->hide();
    }
}

void PythonScriptingConsole::keyPressEvent(QKeyEvent* event)
{
    if (completer && completer->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (event->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Backtab:

            event->ignore();
            return; // let the completer do default behavior
        case Qt::Key_Tab:
        {
            handleTabCompletion(true);
        }
        default:
            break;
        }
    }

    bool eventHandled = false;

    switch (event->key())
    {
    case Qt::Key_Left:
    {
        // Moving the cursor left is limited to the position
        // of the command prompt.
        if (textCursor().position() <= commandPromptPosition())
        {
            QApplication::beep();
            eventHandled = true;
        }
    }
        break;

    case Qt::Key_Up:
    {
        // Display the previous command in the history
        if (PythonScriptingConsole::historyPosition > 0)
        {
            PythonScriptingConsole::historyPosition--;
            changeHistory();
        }

        eventHandled = true;
    }
        break;

    case Qt::Key_Down:
    {
        // clean input
        if (PythonScriptingConsole::historyPosition+1 == PythonScriptingConsole::history.count())
        {
            clearCommandLine();
            PythonScriptingConsole::historyPosition = PythonScriptingConsole::history.count();
        }

        // Display the next command in the history
        if (PythonScriptingConsole::historyPosition+1 < PythonScriptingConsole::history.count())
        {
            PythonScriptingConsole::historyPosition++;
            changeHistory();
        }

        eventHandled = true;
    }
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
        //TODO - enable store
        // executeLine(event->modifiers() & Qt::ShiftModifier);
        executeLine(false);
        eventHandled = true;
    }
        break;

    case Qt::Key_Backspace:
    {
        if (textCursor().hasSelection())
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

            if (textCursor().position() <= commandPromptPosition())
            {
                QApplication::beep();
                eventHandled = true;
            }
        }
    }
        break;

    case Qt::Key_Delete:
    {
        if (textCursor().hasSelection())
        {
            cut();
            eventHandled = true;
        }
        else
        {
            int commandPromptPosition = this->commandPromptPosition();
            if (textCursor().position() < commandPromptPosition)
            {
                QTextCursor c = textCursor();
                c.movePosition(QTextCursor::End);
                setTextCursor(c);
            }
        }
    }
        break;

    case Qt::Key_Home:
    {
        QTextCursor c = textCursor();
        c.setPosition(commandPromptPosition(),
                      (event->modifiers() & Qt::ShiftModifier) ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);

        setTextCursor(c);
        eventHandled = true;
    }
        break;

    case Qt::Key_Tab:
    {
        handleTabCompletion(true);
        event->accept();
        return;
    }
        break;

    case Qt::Key_Period:
    {
        QTextEdit::keyPressEvent(event);
        handleTabCompletion();
        event->accept();
        return;
    }
        break;

    default:
        if (event->key() >= Qt::Key_Space && event->key() <= Qt::Key_division)
        {
            if (textCursor().hasSelection() && !verifySelectionBeforeDeletion())
            {
                // The selection must not be deleted.
                // eventHandled = true;
            }
            else
            {
                // The key is an input character, check if the cursor is
                // behind the last command prompt, else inserting the
                // character is not allowed.

                int commandPromptPosition = this->commandPromptPosition();
                if (textCursor().position() < commandPromptPosition)
                {
                    QTextCursor c = textCursor();
                    c.setPosition(commandPromptPosition);
                    setTextCursor(c);
                }
            }
        }
    }

    if (eventHandled)
    {
        completer->popup()->hide();
        event->accept();
    }
    else
    {
        QTextEdit::keyPressEvent(event);

        if ((event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Space)
                || completer->popup()->isVisible())
        {
            handleTabCompletion();
        }
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

    QTextCursor c = textCursor();

    int commandPromptPosition = this->commandPromptPosition();
    int selectionStart = c.selectionStart();
    int selectionEnd = c.selectionEnd();

    if (c.hasSelection())
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
                c.setPosition(selectionStart);
                c.setPosition(selectionStart, QTextCursor::KeepAnchor);
                setTextCursor(c);
            }
        }

    }
    else
    { // if (hasSelectedText())

        // When there is no selected text, deletion is not allowed before the
        // command prompt.
        if (c.position() < commandPromptPosition)
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
    textCursor.insertText(PythonScriptingConsole::history.value(PythonScriptingConsole::historyPosition));

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

    // repaint widget
    repaint();

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

    if (PythonScriptingConsole::history.count() > 0)
        historyChanged(PythonScriptingConsole::history.last());

    connect(trvHistory, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(executeCommand(QTreeWidgetItem *, int)));

    setWidget(trvHistory);
}

void PythonScriptingHistoryView::historyChanged(const QString &code)
{
    trvHistory->clear();

    if (PythonScriptingConsole::history.count() > 0)
    {
        foreach (QString historyItem, PythonScriptingConsole::history)
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
