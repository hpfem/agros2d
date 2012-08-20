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

enum Token_type {OPERATOR = 0, PLUS = 1, MINUS = 2, TIMES = 3, DIVIDE = 4, LANGLE = 5, RANGLE = 6, VARIABLE = 10, CONSTANT = 20, FUNCTION = 30, NUMBER = 40, EXPRESION = 100, EXPRESSION1 = 101, TERM = 102, TERM1 = 103, FACTOR = 104};

class Token
{
public:
    Token(){;}
    Token(Token_type) {this->type = type;}
    Token(Token_type type, QString text);
    Token_type get_type() {return this->type;}
    QString get_text() {return this->text;}
    int nesting_level;

private:
    Token_type type;
    QString text;
};


class LexicalAnalyser
{
public:
    QList<Token> getTokens();
    void print(const QStringList & list);

private:
    void sortByLength(QStringList & list);
    QList<Token> tokens;

public:
    LexicalAnalyser(QString s);
};

class Terminals
{
    QList<Token> list;

public:

    Terminals(Token_type terminal_type, QStringList terminal_list);
    void find(QString s, QList<Token> & symbol_que, int & pos, int & nesting_level);
    void print();
};
#endif // LEX_H
