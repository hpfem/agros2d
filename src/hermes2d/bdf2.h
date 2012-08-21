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

class BDF2Table
{
public:
    BDF2Table() : m_calculated(false) {}
    void setOrder(int m_order);
    void setPreviousSteps(QList<double> previousSteps);

    QVector<double> alpha() { assert(m_calculated); return m_alpha; }

    // coefficient of linear combination that combines solutions of A and B method to produce final solution
    // has different meaning than in diploma thesis - here we ADD solutions (instead of substract), multiplied by corresponding delta
    double delta() { assert(m_calculated); return m_delta; }

    QVector<double> gamma() { assert(m_calculated); return m_gamma; }

    double testCalcValue(double step, QList<double> values, double fVal, double fPrevVal);
    static void test();

protected:
    virtual void recalculate() = 0;

    int m_order;
    QList<double> th;
    QVector<double> m_alpha;
    double m_delta;
    QVector<double> m_gamma;
    bool m_calculated;
};

class BDF2ATable : public BDF2Table
{
    virtual void recalculate();
};

class BDF2BTable : public BDF2Table
{
    virtual void recalculate();
};

#endif // BDF2_H
