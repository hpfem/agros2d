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

#ifndef BDF2_H
#define BDF2_H

#include "util.h"
#include "plugin_interface.h"

class Material;

// todo: zrychlit, odebrat asserty, uchovavat rovnou alpha/gamma, atd

class AGROS_LIBRARY_API BDF2Table
{
public:
    BDF2Table() : m_n(-100) {}

    // returns true if matrix unchanged
    bool setOrderAndPreviousSteps(int order, QList<double> previousSteps);

    int n() { return m_n;}
    int order() { return m_n;}

    inline double matrixFormCoefficient() {return m_alpha[0];}
    double vectorFormCoefficient(Hermes::Hermes2D::Func<double> **ext, int component, int numComponents, int offsetPreviousTimeExt, int integrationPoint);
    Hermes::Ord vectorFormCoefficient(Hermes::Hermes2D::Func<Hermes::Ord> **ext, int component, int numComponents, int offsetPreviousTimeExt, int integrationPoint);

    static void test(bool varyLength = false);

protected:
    inline double* alpha() {return m_alpha; }

    double testCalcValue(double step, QList<double> values, double fVal);

    virtual void recalculate() = 0;

    int m_n;
    double th[10];
    double m_actualTimeStep;
    double m_alpha[10];
};

class BDF2ATable : public BDF2Table
{
    virtual void recalculate();
};

#endif // BDF2_H
