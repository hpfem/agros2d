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

class AGROS_API BDF2Table
{
public:
    BDF2Table() : m_n(-100) {}

    // returns true if matrix unchanged
    bool setOrderAndPreviousSteps(int order, QList<double> previousSteps);

    int n() { return m_n;}
    int order() { return m_n;}

    inline double matrixFormCoefficient() {return m_alpha[0];}
    double vectorFormCoefficient(Hermes::Hermes2D::Func<double> **ext, int component, int numComponents, int integrationPoint);
    Hermes::Ord vectorFormCoefficient(Hermes::Hermes2D::Func<Hermes::Ord> **ext, int component, int numComponents, int integrationPoint);

    static void test();

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


/////////////////////////////////////////////////////////////////////////////////////
// TIME DISCRETISATION
/////////////////////////////////////////////////////////////////////////////////////


//template<typename Scalar>
//class CustomMatrixFormVol_time : public MatrixFormVolAgros<Scalar>
//{
//public:
//    CustomMatrixFormVol_time(unsigned int i, unsigned int j, std::string area,
//                             Hermes::Hermes2D::SymFlag sym, Material *materialSource, BDF2Table* table);

//    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
//                         Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
//    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
//                            Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;
//    CustomMatrixFormVol_time<Scalar>* clone();

//private:
//    Hermes::Hermes2D::SymFlag m_sym;
//    BDF2Table* m_table;

//    Material *m_materialSource;
//    mutable Value he_lambda;
//    mutable Value he_p;
//    mutable Value he_vx;
//    mutable Value he_vy;
//    mutable Value he_va;
//    mutable Value he_rho;
//    mutable Value he_cp;
//};


///////////////////////////////////////////////////////////////////////////////////////
//// TIME DISCRETISATION - VECTOR
///////////////////////////////////////////////////////////////////////////////////////

//template<typename Scalar>
//class CustomVectorFormVol_time : public Hermes::Hermes2D::VectorFormVol<Scalar>
//{
//public:
//    CustomVectorFormVol_time(unsigned int i, unsigned int j,
//                              std::string area,
//                              Material *materialSource, BDF2Table* table
//    );

//    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
//                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
//    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
//                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;
//    CustomVectorFormVol_time<Scalar>* clone();
//private:
//    BDF2Table* m_table;

//    Material *m_materialSource;
//    mutable Value he_lambda;
//    mutable Value he_p;
//    mutable Value he_vx;
//    mutable Value he_vy;
//    mutable Value he_va;
//    mutable Value he_rho;
//    mutable Value he_cp;

//    unsigned int j;
//};

///////////////////////////////////////////////////////////////////////////////////////
//// TIME RESIDUAL
///////////////////////////////////////////////////////////////////////////////////////


//template<typename Scalar>
//class CustomVectorFormVol_time_residual : public Hermes::Hermes2D::VectorFormVol<Scalar>
//{
//public:
//    CustomVectorFormVol_time_residual(unsigned int i, unsigned int j,
//                              std::string area,
//                              Material *materialSource, BDF2Table* table);

//    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
//                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
//    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
//                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;
//    CustomVectorFormVol_time_residual<Scalar>* clone();
//private:
//    BDF2Table* m_table;

//    Material *m_materialSource;
//    mutable Value he_lambda;
//    mutable Value he_p;
//    mutable Value he_vx;
//    mutable Value he_vy;
//    mutable Value he_va;
//    mutable Value he_rho;
//    mutable Value he_cp;

//    unsigned int j;
//};
#endif // BDF2_H
