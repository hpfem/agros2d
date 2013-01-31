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

#ifndef GUI_TEXTEDIT_H
#define GUI_TEXTEDIT_H

#include "util.h"

struct ParenthesisInfo
{
    char character;
    int position;
};

class AGROS_API TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData();

    QVector<ParenthesisInfo *> parentheses();
    void insert(ParenthesisInfo *info);

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class AGROS_API PlainTextEditParenthesis : public QPlainTextEdit
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
