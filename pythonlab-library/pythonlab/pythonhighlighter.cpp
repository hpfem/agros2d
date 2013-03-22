/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
**************************************************************************/

#include "pythonhighlighter.h"

#include <QtCore/QSet>
#include <QtCore/QtAlgorithms>
#include <QtCore/QDebug>
#include <QSyntaxHighlighter>

#include <QTextBlock>
#include <QTextBlockUserData>

#include "gui/textedit.h"

const int ParenthesisMatcherPropertyId = QTextFormat::UserProperty;
const int ErrorMarkerPropertyId = QTextFormat::UserProperty + 1;

// ************************************************************************************************************

PythonHighlighter::PythonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywordPatterns;
    keywordPatterns << "and" << "assert" << "break" << "class" << "continue" << "def"
            << "del" << "elif" << "else" << "except" << "exec" << "finally"
            << "for" << "from" << "global" << "if" << "import" << "in"
            << "is" << "lambda" << "not" << "or" << "pass" << "print" << "raise"
            << "return" << "try" << "while" << "yield"
            << "None" << "True"<< "False";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp("\\b" + pattern + "\\b", Qt::CaseInsensitive);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    functionFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    operatorFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("[\\\\|\\<|\\>|\\=|\\!|\\+|\\-|\\*|\\/|\\%]+");
    rule.pattern.setMinimal(true);
    rule.format = operatorFormat;
    highlightingRules.append(rule);

    numberFormat.setForeground(Qt::blue);
    rule.format = numberFormat;
    rule.pattern = QRegExp("\\b[+-]?[0-9]+[lL]?\\b");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b");
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.format = quotationFormat;
    rule.pattern = QRegExp("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("'[^'\\\\]*(\\\\.[^'\\\\]*)*'");
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegExp("\"\"\"");
    commentEndExpression = QRegExp("\"\"\"");
    multiLineCommentFormat.setForeground(Qt::red);
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    int add = 0;
    if (previousBlockState() != 1)
    {
        startIndex = commentStartExpression.indexIn(text);
        add = commentStartExpression.matchedLength();
    }

    while (startIndex >= 0)
    {
        int endIndex = commentEndExpression.indexIn(text, startIndex + add);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            setCurrentBlockState(0);
            commentLength = endIndex - startIndex + add + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }

    // parenthesis
    highlightBlockParenthesis(text, '(', ')');
    // highlightBlockParenthesis(text, '[', ']');
    // highlightBlockParenthesis(text, '{', '}');
}

void PythonHighlighter::highlightBlockParenthesis(const QString &text, char left, char right)
{
    TextBlockData *data = new TextBlockData();

    int leftPos = text.indexOf(left);
    while (leftPos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo();
        info->character = left;
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf(left, leftPos + 1);
    }

    int rightPos = text.indexOf(right);
    while (rightPos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo();
        info->character = right;
        info->position = rightPos;

        data->insert(info);
        rightPos = text.indexOf(right, rightPos + 1);
    }

    setCurrentBlockUserData(data);
}
