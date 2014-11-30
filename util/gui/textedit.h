// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef GUI_TEXTEDIT_H
#define GUI_TEXTEDIT_H

#include "../util.h"

struct ParenthesisInfo
{
    char character;
    int position;
};

class AGROS_UTIL_API TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData();

    QVector<ParenthesisInfo *> parentheses(const char &bracket);
    void insert(const char &bracket, ParenthesisInfo *info);

private:
    QMap<char, QVector<ParenthesisInfo *> > m_parentheses;
};

class AGROS_UTIL_API PlainTextEditParenthesis : public QPlainTextEdit
{
    Q_OBJECT

public:
   PlainTextEditParenthesis(QWidget *parent = 0);

protected slots:    
   void matchParentheses(char left = '(', char right = ')');

protected:
    bool matchLeftParenthesis(char left, char right, QTextBlock currentBlock, int index, int numRightParentheses);
    bool matchRightParenthesis(char left, char right, QTextBlock currentBlock, int index, int numLeftParentheses);
    void createParenthesisSelection(int pos);
};

#endif // GUI_TEXTEDIT_H
