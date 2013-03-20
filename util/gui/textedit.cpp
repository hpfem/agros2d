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

#include "textedit.h"

#include "util.h"

TextBlockData::TextBlockData()
{

}

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}

void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() && info->position > m_parentheses.at(i)->position)
        ++i;

    m_parentheses.insert(i, info);
}

// ************************************************************************************************************

PlainTextEditParenthesis::PlainTextEditParenthesis(QWidget *parent)
    : QPlainTextEdit(parent)
{
}

void PlainTextEditParenthesis::matchParentheses(char left, char right)
{
    QList<QTextEdit::ExtraSelection> selections;
    setExtraSelections(selections);

    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data)
    {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i)
        {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && info->character == left)
            {
                if (matchLeftParenthesis(left, right, textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
            else if (info->position == curPos - 1 && info->character == right)
            {
                if (matchRightParenthesis(left, right, textCursor().block(), i - 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool PlainTextEditParenthesis::matchLeftParenthesis(char left, char right, QTextBlock currentBlock, int i, int numLeftParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i)
    {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == left)
        {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == right && numLeftParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numLeftParentheses;
        }
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(left, right, currentBlock, 0, numLeftParentheses);

    return false;
}

bool PlainTextEditParenthesis::matchRightParenthesis(char left, char right, QTextBlock currentBlock, int i, int numRightParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i)
    {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == right)
        {
            ++numRightParentheses;
            continue;
        }
        if (info->character == left && numRightParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numRightParentheses;
        }
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
        return matchRightParenthesis(left, right, currentBlock, 0, numRightParentheses);

    return false;
}

void PlainTextEditParenthesis::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setForeground(Qt::red);
    format.setFontWeight(QFont::Bold);
    // format.setBackground(Qt::lightGray);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}
