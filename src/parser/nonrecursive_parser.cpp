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

#include "nonrecursive_parser.h"

#include "nonrecursive_parser.h"
#include "lex.h"

#define ERROR 1

void NonRecursiveParser::parse()
{

}

ParserTable::ParserTable()
{

}

int ParserTable::next(TokenType i, TokenType j)
{
    switch(i)
    {
    case TokenType_EXPRESION:
        switch(j)
        {
        case TokenType_VARIABLE:
        // case TokenType_LANGLE:
        //     stack.append(Token(TokenType_EXPRESSION1));
        //     stack.append(Token(TokenType_TERM));
        //     break;
        default:
            return ERROR;
        }
        break;

    case TokenType_EXPRESSION1:
        switch(j)
        {
        case TokenType_PLUS:
            stack.append(TokenType_EXPRESSION1);
            stack.append(TokenType_TERM);
            stack.append(TokenType_PLUS);
            break;
         default:
            return ERROR;
        }
    case TokenType_TERM:
        break;
    case TokenType_TERM1:
        break;
    case TokenType_FACTOR:
        break;
    default:
        ;
    }

    return 0;
}
