/*
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

#ifndef _PYTHONCONSOLE_H
#define _PYTHONCONSOLE_H

#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif

class QCompleter;
class PythonEngine;

class PythonScriptingConsole : public QTextEdit
{
    Q_OBJECT
public:
    PythonScriptingConsole(PythonEngine *pythonEngine, QWidget* parent = NULL);
    ~PythonScriptingConsole();

    // static history for all consoles
    static QStringList history;
    static int historyPosition;

signals:
    void historyChanged(const QString &code);

public slots:
    // execute current line
    void executeLine(bool storeOnly);
    void executeLine(const QString &str, bool storeOnly);

    // derived key press event
    void keyPressEvent(QKeyEvent *e);

    // output from console
    void consoleMessage(const QString &message, const QColor& color = Qt::black);

    // clear command line
    void clearCommandLine();

    // clear the console
    void welcomeMessage();

    // overridden to control which characters a user may delete
    virtual void cut();

    // clear output
    void stdClear();
    // output redirection
    void stdOut(const QString& str);
    // error redirection
    void stdErr(const QString& str);
    // output html
    void stdHtml(const QString& str);
    // output image
    void stdImage(const QString &fileName);

    void insertCompletion(const QString& completion);

    // Appends a newline and command prompt at the end of the document.
    void appendCommandPrompt(bool storeOnly = false);

    void connectStdOut(const QString &currentPath = "");
    void disconnectStdOut();

protected:
    // handle the pressing of tab
    void handleTabCompletion(bool autoComplete = false);

    // Returns the position of the command prompt
    int commandPromptPosition();

    // Returns if deletion is allowed at the current cursor
    // (with and without selected text)
    bool verifySelectionBeforeDeletion();

    // change the history according to _historyPos
    void changeHistory();

private:
    PythonEngine *pythonEngine;

    void executeCode(const QString& code);

    QString m_clickedAnchor;
    QString m_storageKey;
    int m_commandPromptLength;

    QString m_currentMultiLineCode;

    QTextCharFormat m_defaultTextCharacterFormat;
    QCompleter* completer;

    QString m_currentPath;
};

class PythonScriptingConsoleView : public QDockWidget
{
    Q_OBJECT
public:
    PythonScriptingConsoleView(PythonEngine *pythonEngine, QWidget *parent = 0);
    ~PythonScriptingConsoleView();

    inline PythonScriptingConsole *console() { return m_console; }

protected:
    void focusInEvent(QFocusEvent *event);

private:
    PythonScriptingConsole *m_console;

private slots:
    void doVisibilityChanged(bool);

};

class PythonScriptingHistoryView : public QDockWidget
{
    Q_OBJECT
public:
    PythonScriptingHistoryView(PythonScriptingConsole *console, QWidget *parent = 0);

private slots:
    void historyChanged(const QString &code);
    void executeCommand(QTreeWidgetItem *item, int role);

private:
    QTreeWidget *trvHistory;

    PythonScriptingConsole *console;
};

#endif _PYTHONCONSOLE
