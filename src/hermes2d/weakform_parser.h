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

#ifndef WEAKFORM_PARSER_H
#define WEAKFORM_PARSER_H

#include "util.h"

class Parser;
class Boundary;
class Material;
class FieldInfo;
class CouplingInfo;

namespace Hermes
{
namespace Hermes2D
{
    template<typename Scalar> class Func;
}
}

namespace Module
{
    struct MaterialTypeVariable;
}

struct ParserFormEssential
{
    ParserFormEssential() : i(0), expressionLinear(""), expressionNewton("") {}
    ParserFormEssential(int i, std::string expr_linear, std::string expr_newton) : i(i), expressionLinear(expr_linear), expressionNewton(expr_newton) {}

    // position
    int i;

    // expression
    std::string expressionLinear;
    std::string expressionNewton;
};

// instead of ParserFormVector and ParserFormMatrix
struct ParserFormExpression
{
    ParserFormExpression() : i(0), j(0), sym(Hermes::Hermes2D::HERMES_NONSYM), expressionLinear(""), expressionNewton("") {}
    ParserFormExpression(int i, int j, string expr_linear, string expr_newton, Hermes::Hermes2D::SymFlag sym = Hermes::Hermes2D::HERMES_NONSYM)
        : i(i), j(j), expressionLinear(expr_linear), expressionNewton(expr_newton), sym(sym) {}

    // position
    int i;
    int j;

    // symmetric flag
    Hermes::Hermes2D::SymFlag sym;

    // expression
    std::string expressionLinear;
    std::string expressionNewton;
};

const int maxSourceFieldComponents = 2;

#endif // WEAKFORM_PARSER_H
