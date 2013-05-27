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

bool BDF2Table::setOrderAndPreviousSteps(int order, QList<double> previousSteps)
{
    bool matrixUnchanged = true;
    if(this->m_n != order)
        matrixUnchanged = false;

    this->m_n = order;

    int numSteps = previousSteps.length();
    assert(numSteps >= m_n);

    int numToCheckConst = min(order, numSteps);
    for(int iCheck = 0; iCheck < numToCheckConst; iCheck++)
        if(m_actualTimeStep != previousSteps[numSteps - 1 - iCheck])
            matrixUnchanged = false;

    m_actualTimeStep = previousSteps[numSteps-1];
    if(m_n >= 2)
        th[0] = previousSteps[numSteps-1] / previousSteps[numSteps - 2];
    if(m_n >= 3)
        th[1] = previousSteps[numSteps-2] / previousSteps[numSteps - 3];

    recalculate();

    return matrixUnchanged;
}

double BDF2Table::vectorFormCoefficient(Hermes::Hermes2D::Func<double> **ext, int component, int numComponents, int integrationPoint)
{
    double coef = 0;

    for(int ps = 0; ps < n(); ps++)
    {
        coef += (-alpha()[ps + 1]) * ext[numComponents * ps + component]->val[integrationPoint];
    }

    return coef;
}

Hermes::Ord BDF2Table::vectorFormCoefficient(Hermes::Hermes2D::Func<Hermes::Ord> **ext, int component, int numComponents, int integrationPoint)
{
    Hermes::Ord coef(0);

    for(int ps = 0; ps < n(); ps++)
    {
        coef += (-alpha()[ps + 1]) * ext[numComponents * ps + component]->val[integrationPoint];
    }

    return coef;
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
        m_alpha[0] = 1. / m_actualTimeStep;
        m_alpha[1] = -1. / m_actualTimeStep;
    }
    else if(m_n == 2)
    {
        m_alpha[0] = ((2*th[0] + 1) / (th[0] + 1)) / m_actualTimeStep;
        m_alpha[1] = (-th[0] - 1) / m_actualTimeStep ;
        m_alpha[2] = (th[0] * th[0] / (th[0] + 1)) / m_actualTimeStep;
    }
    else if(m_n == 3)
    {
        double t0 = th[0];
        double t1 = th[1];
        m_alpha[0] = ((4*t0*t1 + 3*t0*t0*t1 + t1 + 1 + 2*t0) / (t0 + 2*t0*t1 + 1 + t1 + t0*t0*t1)) / m_actualTimeStep;
        m_alpha[1] = (-(t0 + 2*t0*t1 + 1 + t1 + t0*t0*t1) / (1+t1)) / m_actualTimeStep;
        m_alpha[2] = ((t1 + t0*t1 + 1) * t0*t0 / (1+t0)) / m_actualTimeStep;
        m_alpha[3] = ( -(1+t0) * t0*t0 * t1*t1*t1 / (t0*t1*t1 + t0*t1 + 2*t1 + 1 + t1*t1)) / m_actualTimeStep;

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

double BDF2Table::testCalcValue(double step, QList<double> values, double fVal)
{
    double result = fVal;

    for(int i = 1; i <= m_n; i++)
    {
        result -= m_alpha[i] * values[values.size() - i];
    }

    result /= m_alpha[0];

    return result;
}

void BDF2Table::test(bool varyLength)
{
    BDF2ATable tableA;

    double results[3][4];

    int numStepsArray[] = {100, 1000, 10000, 100000};

    for(int order = 1; order <=3; order++)
    {
        for(int numStepsIdx = 0; numStepsIdx < 4; numStepsIdx++)
        {
            int numSteps = numStepsArray[numStepsIdx];
            double constantStepLen = 1./double(numSteps);

            if(varyLength)
                numSteps = 3*numSteps/2;

            QList<double> previousSteps;

            QList<double>  valsA;
            valsA.push_back(f(0));
            double actTime = 0;
            int realOrder;
            for(int s = 0; s < numSteps; s++)
            {
                double actualStepLen = constantStepLen;
                if(varyLength && (s % 3))
                    actualStepLen = constantStepLen/2.;

                previousSteps.push_back(actualStepLen);

                if(s == 0)
                    realOrder = 1;

                if((s == 1) && (order >= 2))
                    realOrder = 2;

                if((s == 2) && (order >= 3))
                    realOrder = 3;

                if(s <= 2)
                {
                    tableA.setOrderAndPreviousSteps(realOrder, previousSteps);
                }
                actTime += actualStepLen;

                double valA = tableA.testCalcValue(actualStepLen, valsA, df(actTime));
                valsA.push_back(valA);
            }

            cout << "actTime " << actTime << ", step " << numSteps << endl;
            assert(fabs(actTime-1.) < 0.000000001);

            double errorA = valsA.last() - f(1);
            cout << "order " << order << ", step " << 1./double(numSteps) << (varyLength ? " approx(alternate)" : " exact") << ", error " << errorA << endl;
            results[order-1][numStepsIdx] = errorA;
        }
    }
    cout << "errors = [";
    for(int ord = 0; ord < 3; ord++)
    {
        cout << "[";
        for(int st = 0; st < 4; st++)
        {
            cout << results[ord][st];
            if(st < 3)
                cout << ",";
        }
        cout << "]";
        if(ord < 2)
            cout << ",";
    }
    cout << "]"<< endl << endl;
}
