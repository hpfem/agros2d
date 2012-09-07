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

double BDF2Table::vectorFormCoefficient(Hermes::Hermes2D::ExtData<double> *ext, int component, int numComponents, int integrationPoint)
{
    double coef = 0;

    for(int ps = 0; ps < n(); ps++)
    {
        coef += (-alpha()[ps + 1]/gamma()[0]) * ext->fn[numComponents * ps + component]->val[integrationPoint];
    }

    return coef / m_actualTimeStep;
}

Hermes::Ord BDF2Table::vectorFormCoefficient(Hermes::Hermes2D::ExtData<Hermes::Ord> *ext, int component, int numComponents, int integrationPoint)
{
    Hermes::Ord coef(0);

    for(int ps = 0; ps < n(); ps++)
    {
        coef += (-alpha()[ps + 1]/gamma()[0]) * ext->fn[numComponents * ps + component]->val[integrationPoint];
    }

    return coef / m_actualTimeStep;
}

void BDF2ATable::recalculate()
{
    if(m_n == 1)
    {
        m_alpha[0] = 1.;
        m_alpha[1] = -1.;
        m_delta = -1./2.;
        m_gamma[0] = 1.;
        m_gamma[1] = 0.;
    }
    else if(m_n == 2)
    {
        m_alpha[0] = (2*th[0] + 1) / (th[0] + 1);
        m_alpha[1] = -th[0] - 1;
        m_alpha[2] = th[0] * th[0] / (th[0] + 1);
        m_delta = -th[0] * (2*th[0] + 1) / (3*th[0] + 2);
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
        m_delta = 3./2.;
        m_gamma[0] = 2./3.;
        m_gamma[1] = 1./3.;
    }
    else if(m_n == 2)
    {
        m_alpha[0] = 1.;
        m_alpha[1] = -1.;
        m_alpha[2] = 0.;
        m_delta = 2 * (th[0]*th[0] + 2*th[0] + 1) / (3*th[0] + 2);
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

    for(int order = 1; order <=2; order++)
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
                double val = tableA.delta() * valA + tableB.delta() * valB;
                vals.push_back(val);

                valA = tableA.testCalcValue(step, valsA, df(actTime), df(actTime - step));
                valsA.push_back(valA);
                valB = tableB.testCalcValue(step, valsB, df(actTime), df(actTime - step));
                valsB.push_back(valB);
            }

            assert(fabs(actTime-1.) < 0.000000001);

            double errorA = valsA.last() - f(1);
            double errorB = valsB.last() - f(1);
            double error = vals.last() - f(1);
            cout << "order " << order << ", step " << step << ", error " << error << "  (" << errorA << ", " << errorB << ")" << endl;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// TIME DISCRETISATION - MATRIX
/////////////////////////////////////////////////////////////////////////////////////


//template <typename Scalar>
//CustomMatrixFormVol_time<Scalar>::CustomMatrixFormVol_time(unsigned int i, unsigned int j, std::string area,
//                                                           Hermes::Hermes2D::SymFlag sym, Material* materialSource, BDF2Table* table)
//    : MatrixFormVolAgros<Scalar>(i, j, area, sym), m_materialSource(materialSource), m_sym(sym), m_table(table)
//{
//    he_lambda = m_materialSource->value("heat_conductivity");
//    he_p = m_materialSource->value("heat_volume_heat");
//    he_vx = m_materialSource->value("heat_velocity_x");
//    he_vy = m_materialSource->value("heat_velocity_y");
//    he_va = m_materialSource->value("heat_velocity_angular");
//    he_rho = m_materialSource->value("heat_density");
//    he_cp = m_materialSource->value("heat_specific_heat");
//}


//template <typename Scalar>
//Scalar CustomMatrixFormVol_time<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
//                                          Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
//{
//    double result = 0;

//    for (int i = 0; i < n; i++)
//    {
//        result += wt[i] * m_table->matrixFormCoefficient() * he_rho.value(0)*he_cp.value(0) * u->val[i] * v->val[i];
//    }
//    result /= Util::problem()->actualTimeStepLength();

//    return result;
//}



//template <typename Scalar>
//Hermes::Ord CustomMatrixFormVol_time<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
//                                             Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
//{
//    Hermes::Ord result(20);
////    for (int i = 0; i < n; i++)
////    {
////       result += wt[i] * (10);
////    }
//    return result;
//}

//template <typename Scalar>
//CustomMatrixFormVol_time<Scalar>* CustomMatrixFormVol_time<Scalar>::clone()
//{
//    return new CustomMatrixFormVol_time(this->i, this->j, this->areas[0], this->m_sym, this->m_materialSource, this->m_table);
//}

///////////////////////////////////////////////////////////////////////////////////////
//// TIME DISCRETISATION - VECTOR
///////////////////////////////////////////////////////////////////////////////////////


//template <typename Scalar>
//CustomVectorFormVol_time<Scalar>::CustomVectorFormVol_time(unsigned int i, unsigned int j,
//                                                 std::string area,
//                                                 Material* materialSource, BDF2Table *table)
//    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), m_materialSource(materialSource), j(j), m_table(table)
//{
// he_lambda = m_materialSource->value("heat_conductivity");
// he_p = m_materialSource->value("heat_volume_heat");
// he_vx = m_materialSource->value("heat_velocity_x");
// he_vy = m_materialSource->value("heat_velocity_y");
// he_va = m_materialSource->value("heat_velocity_angular");
// he_rho = m_materialSource->value("heat_density");
// he_cp = m_materialSource->value("heat_specific_heat");

//}

//template <typename Scalar>
//Scalar CustomVectorFormVol_time<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
//                                          Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
//{
//    double result = 0;

//    for (int i = 0; i < n; i++)
//    {
//        result += wt[i] * m_table->vectorFormCoefficient(ext, i) * he_rho.value(0)*he_cp.value(0) * v->val[i];
//    }
//    result /= Util::problem()->actualTimeStepLength();

//    return result;
//}


//template <typename Scalar>
//Hermes::Ord CustomVectorFormVol_time<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
//                                             Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
//{
//    //variable_definition
//    Hermes::Ord result(20);
////    for (int i = 0; i < n; i++)
////    {
////       result += wt[i] * (1*(u_ext[this->j]->dx[i]*v->dx[i]+u_ext[this->j]->dy[i]*v->dy[i])-1*v->val[i]+1*1*((1-e->y[i]*1)*u_ext[this->j]->dx[i]+(1+e->x[i]*1)*u_ext[this->j]->dy[i])*v->val[i]);
////    }
//    return result;
//}

//template <typename Scalar>
//CustomVectorFormVol_time<Scalar>* CustomVectorFormVol_time<Scalar>::clone()
//{
//    return new CustomVectorFormVol_time(this->i, this->j, this->areas[0],
//                                         this->m_materialSource, m_table);
//}


///////////////////////////////////////////////////////////////////////////////////////
//// TIME RESIDUAL
///////////////////////////////////////////////////////////////////////////////////////


//template <typename Scalar>
//CustomVectorFormVol_time_residual<Scalar>::CustomVectorFormVol_time_residual(unsigned int i, unsigned int j,
//                                                 std::string area,
//                                                 Material* materialSource, BDF2Table* table)
//    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), m_materialSource(materialSource), j(j), m_table(table)
//{
// he_lambda = m_materialSource->value("heat_conductivity");
// he_p = m_materialSource->value("heat_volume_heat");
// he_vx = m_materialSource->value("heat_velocity_x");
// he_vy = m_materialSource->value("heat_velocity_y");
// he_va = m_materialSource->value("heat_velocity_angular");
// he_rho = m_materialSource->value("heat_density");
// he_cp = m_materialSource->value("heat_specific_heat");

//}





//template <typename Scalar>
//Scalar CustomVectorFormVol_time_residual<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
//                                          Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
//{
//    double result = 0;

//    for (int i = 0; i < n; i++)
//    {
//        result += wt[i] * m_table->residualCoefficient() * (he_lambda.value(ext->fn[0]->val[i])*(ext->fn[0]->dx[i]*v->dx[i]+ext->fn[0]->dy[i]*v->dy[i])-he_p.number()*v->val[i]+he_rho.value(ext->fn[0]->val[i])*he_cp.value(ext->fn[0]->val[i])*((he_vx.number()-e->y[i]*he_va.number())*ext->fn[0]->dx[i]+(he_vy.number()+e->x[i]*he_va.number())*ext->fn[0]->dy[i])*v->val[i]);
//    }
//    return result;
//}





//template <typename Scalar>
//Hermes::Ord CustomVectorFormVol_time_residual<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
//                                             Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
//{
//    //variable_definition
//    Hermes::Ord result(20);
////    for (int i = 0; i < n; i++)
////    {
////       result += wt[i] * (1*(u_ext[this->j]->dx[i]*v->dx[i]+u_ext[this->j]->dy[i]*v->dy[i])-1*v->val[i]+1*1*((1-e->y[i]*1)*u_ext[this->j]->dx[i]+(1+e->x[i]*1)*u_ext[this->j]->dy[i])*v->val[i]);
////    }
//    return result;
//}



//template <typename Scalar>
//CustomVectorFormVol_time_residual<Scalar>* CustomVectorFormVol_time_residual<Scalar>::clone()
//{
//    return new CustomVectorFormVol_time_residual(this->i, this->j, this->areas[0],
//                                         this->m_materialSource, this->m_table);
//}


//template class CustomMatrixFormVol_time<double>;
//template class CustomVectorFormVol_time<double>;
//template class CustomVectorFormVol_time_residual<double>;
