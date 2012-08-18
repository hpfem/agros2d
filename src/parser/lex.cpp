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


#include <iostream>
#include "lex.h"

void LexicalAnalyser::sortByLength(QStringList &list)
{
    int n = list.count();
    QString temp;

    for(int i = 0; i < n; i++)
    {
        for(int j = 1; j < n; j++)
        {
            if (list[j].count() > list[j-1].count())
            {
                temp = list[j];
                list[j] = list[j-1];
                list[j-1] = temp;
            }
        }
    }
}

void LexicalAnalyser::print(const QStringList &list)
{
    int n = list.count();
    QTextStream qout(stdout);
    for(int i = 0; i < n; i++)
    {
        qout << list[i] << endl;
    }
}

QList<Token> LexicalAnalyser::tokens()
{
    return this->m_tokens;
}

QString get_expression( QList<Token> const &symbol_que, int position = 0)
{
    int n = symbol_que.count();
    int nesting_level = symbol_que[position].nestingLevel;
    QString expression =  "neco";
    for(int i = position; i < n; i++)
    {
        if (nesting_level < symbol_que[i].nestingLevel)
        {
            nesting_level++;
        }

        if (nesting_level > symbol_que[i].nestingLevel)
        {
            nesting_level--;
        }

    }
    return expression;
}


LexicalAnalyser::LexicalAnalyser(QString expression)
{
    QTextStream qout(stdout);
    QStringList variables;
    QStringList operators;
    QStringList functions;
    QList<Terminals>  terminals;

    variables << "x" << "yy" << "y" << "xx";
    sortByLength(variables);
    terminals.append(Terminals(VARIABLE, variables));

    operators << "(" << ")" << "+" << "**" << "-" << "*" << "/" ;
    sortByLength(operators);
    terminals.append(Terminals(OPERATOR, operators));
    functions << "sin" << "cos" << "log" << "log10";
    sortByLength(functions);
    terminals.append(Terminals(FUNCTION, functions));

    int pos = 0;
    Token symbol;

    QRegExp r_exp = QRegExp("[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");

    int loop_counter = 0;
    int n = expression.count();
    int old_pos = 0;
    int nesting_level = 0;
    while ((pos < n) && (loop_counter < n))
    {
        loop_counter++;
        int index = r_exp.indexIn(expression, pos);

        if(index == pos)
        {
            symbol.type = NUMBER;
            symbol.text = r_exp.capturedTexts().takeFirst();
            symbol.nestingLevel = nesting_level;
            pos += symbol.text.count();
            m_tokens.append(symbol);
        }

        foreach(Terminals terminal_symbols, terminals)
        {
            terminal_symbols.find(expression, m_tokens, pos, nesting_level);
        }

        if (old_pos == pos)
        {
            std::cout << "Unexpected symbol:" << "\"z\"" << " on position: " << pos << endl;
            std::cout << expression.at(pos).toAscii() << endl;
            break;
        }
        else
        {
            old_pos = pos;
        }
    }



    /*    for(int i = 0; i < tokens.count();i++)
    {
        qout << tokens[i].text << "  " << tokens[i].terminal_type << "  " << tokens[i].nesting_level << endl;
    } */


    get_expression(m_tokens);
}


Terminals::Terminals(TokenType terminal_type, QStringList terminal_list)
{
    Token symbol;
    int n = terminal_list.count();
    for(int i = 0; i < n; i++)
    {
        symbol.type = terminal_type;
        symbol.text = terminal_list[i];
        this->list.append(symbol);
    }
}

void Terminals::find(QString s, QList<Token> &symbol_que, int &pos, int &nesting_level)
{
    Token symbol;
    int n = this->list.count();
    for (int i = 0; i < n; i++)
    {
        int loc_pos = s.indexOf(list[i].text, pos);
        if (loc_pos == pos) {
            symbol.type = list[i].type;
            symbol.text = list[i].text;

            if (symbol.text == "(")
            {
                symbol.nestingLevel = nesting_level++;
            } else
                if (symbol.text == ")")
                {
                    symbol.nestingLevel = --nesting_level;
                }
                else
                    symbol.nestingLevel = nesting_level;

            pos += list[i].text.count();
            symbol_que.append(symbol);
            break;
        }
    }
}

void Terminals::print()
{
    QTextStream qout(stdout);
    int n =this->list.count();
    for(int i = 0; i < n; i++)
    {
        qout << this->list[i].text << endl;
    }
}
