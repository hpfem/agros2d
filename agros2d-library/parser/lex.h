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

#include "util.h"

enum ParserTokenType
{
    ParserTokenType_OPERATOR = 0,
    ParserTokenType_VARIABLE = 10,
    ParserTokenType_CONSTANT = 20,
    ParserTokenType_FUNCTION = 30,
    ParserTokenType_NUMBER = 40,
    ParserTokenType_KEYWORD = 200
};

class ParserException : public AgrosException
{
public:
    ParserException(const QString &what, const QString &expr, int pos, const QString &symbol)
        : AgrosException(what), m_expr(expr), m_pos(pos), m_symbol(symbol)
    {        
    }

    inline QString expression() { return m_expr; }
    inline int position() { return m_pos; }
    inline QString symbol() { return m_symbol; }

private:
    QString m_expr;
    int m_pos;
    QString m_symbol;
};

class Terminal
{
public:
    Terminal(ParserTokenType terminalType, QString pattern)
    {
        m_pattern = QRegExp(pattern);
        m_terminalType = terminalType;
    }

    QRegExp m_pattern;
    ParserTokenType m_terminalType;
};

class Token
{
public:
    Token() {}
    Token(ParserTokenType m_type, QString m_text, int nestingLevel = 0, int position = 0);

    inline ParserTokenType type() { return this->m_type; }
    inline QString toString() { return this->m_text; }    
    inline void setText(QString text) { m_text = text; }
    inline int position() { return m_position; }
    inline int nestingLevel() { return m_nestingLevel; }
    inline void setNestingLevel(int nestingLevel) { m_nestingLevel = nestingLevel; }

private:
    ParserTokenType m_type;
    QString m_text;
    int m_nestingLevel;
    int m_position;
};


class AGROS_LIBRARY_API LexicalAnalyser
{
public:

    LexicalAnalyser();

    void setExpression(const QString &expr);

    // return all tokens
    QList<Token> tokens();

    // print tokens
    void printTokens();

    // variables
    inline QStringList variables() { return m_variables; }
    inline void addVariable(const QString &variable) { if (!m_variables.contains(variable)) m_variables.append(variable); }
    inline void addVariables(const QStringList &list) { m_variables.append(list); }
    inline void removeVariable(const QString &variable) { m_variables.removeAll(variable); }
    inline void clearVariables() { m_variables.clear(); }
    QString replaceOperatorByFunction(QString expr);

    QString replaceVariables(QMap<QString, QString> dict, const QString &expr = QString());
    QString latexVariables(QMap<QString, QString> dict, const QString &expr = QString());

private:
    QList<Token> m_tokens;
    QStringList m_variables;
    QList<Terminal> m_patterns;
    void sortByLength(QStringList &list);
    void setPatterns();
};

class Terminals
{
    QList<Token> m_list;

public:

    Terminals(ParserTokenType terminal_type, QStringList terminal_list);
    void find(const QString &s, QList<Token> &symbol_que, int &pos, int &nesting_level);
    void print();
};
#endif // LEX_H
