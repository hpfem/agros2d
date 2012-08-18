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

#ifndef LEX_H
#define LEX_H

#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QStringList>

enum TokenType
{
    OPERATOR = 100,
    VARIABLE = 101,
    CONSTANT = 102,
    FUNCTION = 103,
    NUMBER = 104
};

enum NonTerminals
{
    EXPRESION = 0,
    EXPRESSION1 = 1,
    TERM = 2,
    TERM1 = 3,
    FACTOR = 4
};

struct Token
{
    TokenType type;
    QString text;
    int nestingLevel;
};

class LexicalAnalyser
{
public:
    QList<Token> tokens();
    void print(const QStringList & list);

private:
    void sortByLength(QStringList & list);
    QList<Token> m_tokens;

public:
    LexicalAnalyser(QString s);
};

class Terminals
{
public:
    Terminals(TokenType terminal_type, QStringList terminal_list);

    void find(QString s, QList<Token> &symbol_que, int &pos, int &nesting_level);
    void print();

private:
    QList<Token> list;
};

#endif // LEX_H
