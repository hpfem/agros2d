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


#include "bdf2.h"
#include "scene.h"
#include "problem.h"
#include "marker.h"

void BDF2Table::setOrder(int order)
{
    this->m_n = order;
    m_calculated = false;
}

void BDF2Table::setPreviousSteps(QList<double> previousSteps)
{
    int numSteps = previousSteps.length();
    assert(numSteps >= m_n);

    m_actualTimeStep = previousSteps[numSteps-1];
    if(m_n >= 2)
        th[0] = previousSteps[numSteps-1] / previousSteps[numSteps - 2];
    if(m_n >= 3)
        th[1] = previousSteps[numSteps-2] / previousSteps[numSteps - 3];

    recalculate();

    m_calculated = true;
}

double BDF2Table::vectorFormCoefficient(Hermes::Hermes2D::Func<double> **ext, int component, int numComponents, int integrationPoint)
{
    double coef = 0;

    for(int ps = 0; ps < n(); ps++)
    {
        coef += (-alpha()[ps + 1]/gamma()[0]) * ext[numComponents * ps + component]->val[integrationPoint];
    }

    return coef / m_actualTimeStep;
}

Hermes::Ord BDF2Table::vectorFormCoefficient(Hermes::Hermes2D::Func<Hermes::Ord> **ext, int component, int numComponents, int integrationPoint)
{
    Hermes::Ord coef(0);

    for(int ps = 0; ps < n(); ps++)
    {
        coef += (-alpha()[ps + 1]/gamma()[0]) * ext[numComponents * ps + component]->val[integrationPoint];
    }

    return coef / m_actualTimeStep;
}

class Monomial
{
public:
    Monomial(double t0, double t1) : m_t0(t0), m_t1(t1) {}
    double operator()(int exp1, int exp2) { return pow(m_t0, exp1) * pow(m_t1, exp2); }
private:
    double m_t0, m_t1;
};

void BDF2ATable::recalculate()
{
    if(m_n == 1)
    {
        m_alpha[0] = 1.;
        m_alpha[1] = -1.;
        m_delta = -3./2.;
        m_gamma[0] = 1.;
        m_gamma[1] = 0.;
    }
    else if(m_n == 2)
    {
        m_alpha[0] = (2*th[0] + 1) / (th[0] + 1);
        m_alpha[1] = -th[0] - 1;
        m_alpha[2] = th[0] * th[0] / (th[0] + 1);
        m_delta = -2 * (th[0]*th[0] + 2*th[0] + 1) / (3*th[0] + 2);
        m_gamma[0] = 1.;
        m_gamma[1] = 0.;
    }
    else if(m_n == 3)
    {
        double t0 = th[0];
        double t1 = th[1];
        m_alpha[0] = (4*t0*t1 + 3*t0*t0*t1 + t1 + 1 + 2*t0) / (t0 + 2*t0*t1 + 1 + t1 + t0*t0*t1);
        m_alpha[1] = -(t0 + 2*t0*t1 + 1 + t1 + t0*t0*t1) / (1+t1);
        m_alpha[2] = (t1 + t0*t1 + 1) * t0*t0 / (1+t0);
        m_alpha[3] =  -(1+t0) * t0*t0 * t1*t1*t1 / (t0*t1*t1 + t0*t1 + 2*t1 + 1 + t1*t1);

        Monomial m(t0, t1);

        m_delta = -(3*m(4,2) + 10*m(3,2) + 13*m(2,2) + 8*m(1,2) + 2*m(0,2) + 5*m(3,1) + 13*m(2,1) + 12*m(1,1) + 4*m(0,1) + 2*m(2,0) + 4*m(1,0) + 2)/
                (4*m(2,1) + 9*m(1,1) + 3*m(1,0) + 4*m(2,2) + 6*m(1,2) + 2 + 4*m(0,1) + 2*m(0,2));

        m_gamma[0] = 1.;
        m_gamma[1] = 0.;
    }
    else
        assert(0);
}

void BDF2BTable::recalculate()
{
    if(m_n == 1)
    {
        m_alpha[0] = 1.;
        m_alpha[1] = -1.;
        m_delta = -1./2.;
        m_gamma[0] = 2./3.;
        m_gamma[1] = 1./3.;
    }
    else if(m_n == 2)
    {
        m_alpha[0] = 1.;
        m_alpha[1] = -1.;
        m_alpha[2] = 0.;
        m_delta = -th[0] * (2*th[0] + 1) / (3*th[0] + 2);
        m_gamma[0] = 1./2.;
        m_gamma[1] = 1./2.;
    }
    else if(m_n == 3)
    {
        double t0 = th[0];
        double t1 = th[1];
        m_alpha[0] = 0.5 * (4*t0*t1 + 3*t0*t0*t1 + 2*t1 + 2 + 2*t0) / (t0 + 2*t0*t1 + 1 + t1 + t0*t0*t1);
        m_alpha[1] = -0.5 * (2 + 2*t1 + t0*t0*t1) / (1 + t1);
        m_alpha[2] = 0.5 * t0*t0*t0 * t1 / (1+t0);
        m_alpha[3] = -0.5 * t0*t0*t0 * t1*t1*t1 / (t0*t1*t1 + t0*t1 + 2*t1 + 1 + t1*t1);

        Monomial m(t0, t1);

        m_delta = -t0 * (3*m(3,2) + 10*m(2,2) + 9*m(1,2) + 2*m(0,1) + 5*m(2,1) + 9*m(1,1) + 3*t1 + 2*t0 + 1)/
                (4*m(2,1) + 9*m(1,1) + 3*m(1,0) + 4*m(2,2) + 6*m(1,2) + 2 + 4*m(0,1) + 2*m(0,2));

        m_gamma[0] = 1./2.;
        m_gamma[1] = 1./2.;
    }
    else
        assert(0);
}


const double a = 500;

double f(double x)
{
    return (exp(a*x) - 1) / (exp(a) - 1);
}

double df(double x)
{
    return a * exp(a*x) / (exp(a) - 1);
}

double BDF2Table::testCalcValue(double step, QList<double> values, double fVal, double fPrevVal)
{
    double result = step * (m_gamma[0] * fVal + m_gamma[1] * fPrevVal);

    for(int i = 1; i <= m_n; i++)
    {
        result -= m_alpha[i] * values[values.size() - i];
    }

    result /= m_alpha[0];

    return result;
}

void BDF2Table::test()
{
    BDF2ATable tableA;
    BDF2BTable tableB;

    QList<double> constantSteps;
    for(int i = 0; i < 5; i++)
        constantSteps.append(1.);

    int numStepsArray[] = {100, 1000, 10000, 100000};

    for(int order = 1; order <=3; order++)
    {
        for(int i = 0; i < 4; i++)
        {
            int numSteps = numStepsArray[i];
            double step = 1./double(numSteps);
            QList<double> vals, valsA, valsB;
            vals.push_back(f(0));
            valsA.push_back(f(0));
            valsB.push_back(f(0));
            double actTime = 0;
            int realOrder;
            for(int s = 0; s < numSteps; s++)
            {
                if(s == 0)
                    realOrder = 1;

                if((s == 1) && (order >= 2))
                    realOrder = 2;

                if((s == 2) && (order >= 3))
                    realOrder = 3;

                if(s <= 2)
                {
                    tableA.setOrder(realOrder);
                    tableB.setOrder(realOrder);
                    tableA.setPreviousSteps(constantSteps);
                    tableB.setPreviousSteps(constantSteps);
                }
                actTime += step;
                double valA = tableA.testCalcValue(step, vals, df(actTime), df(actTime - step));
                double valB = tableB.testCalcValue(step, vals, df(actTime), df(actTime - step));
                double val = tableB.delta() * valA - tableA.delta() * valB;
                vals.push_back(val);

                valA = tableA.testCalcValue(step, valsA, df(actTime), df(actTime - step));
                valsA.push_back(valA);
                valB = tableB.testCalcValue(step, valsB, df(actTime), df(actTime - step));
                valsB.push_back(valB);
            }

           // assert(fabs(actTime-1.) < 0.000000001);

            double errorA = valsA.last() - f(1);
            double errorB = valsB.last() - f(1);
            double error = vals.last() - f(1);
            cout << "order " << order << ", step " << step << ", error " << error << "  (" << errorA << ", " << errorB << ")" << endl;
        }
    }
}
