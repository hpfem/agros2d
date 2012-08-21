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
    TokenType_OPERATOR = 0,
    TokenType_PLUS = 1,
    TokenType_MINUS = 2,
    TokenType_TIMES = 3,
    TokenType_DIVIDE = 4,
    TokenType_ = 5,
    TokenType_RANGLE = 6,
    TokenType_VARIABLE = 10,
    TokenType_CONSTANT = 20,
    TokenType_FUNCTION = 30,
    TokenType_NUMBER = 40,
    TokenType_EXPRESION = 100,
    TokenType_EXPRESSION1 = 101,
    TokenType_TERM = 102,
    TokenType_TERM1 = 103,
    TokenType_FACTOR = 104
};

class Token
{
public:
    Token() {}
    Token(TokenType type) { this->m_type = type; }
    Token(TokenType m_type, QString m_text);

    inline TokenType type() { return this->m_type; }
    inline QString text() { return this->m_text; }

    int nestingLevel;

private:
    TokenType m_type;
    QString m_text;
};


class LexicalAnalyser
{
public:
    LexicalAnalyser() {}
    void Parse(const QString &expression);
    QList<Token> tokens();
    void print(const QStringList &list);
    QStringList m_variables;

private:
    void sortByLength(QStringList &list);
    QList<Token> m_tokens;
};

class Terminals
{
    QList<Token> m_list;

public:

    Terminals(TokenType terminal_type, QStringList terminal_list);
    void find(const QString &s, QList<Token> &symbol_que, int &pos, int &nesting_level);
    void print();
};
#endif // LEX_H
