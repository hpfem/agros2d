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

#ifndef DATATABLE_H
#define DATATABLE_H

#include "util.h"
#include "util/enums.h"
#include "spline.h"

class PiecewiseLinear
{
public:
    PiecewiseLinear(Hermes::vector<double> points, Hermes::vector<double> values);
    double value(double x);
    double derivative(double x);

private:
    int leftIndex(double x);

    Hermes::vector<double> m_points;
    Hermes::vector<double> m_values;

    Hermes::vector<double> m_derivatives;
    int m_size;
};

// for testing.. returns average value. Simple "linearization" of the problem
class ConstantTable
{
public:
    ConstantTable(Hermes::vector<double> points, Hermes::vector<double> values);
    double value(double x) const;
    double derivative(double x) const;

private:    
    double m_value;
};

class DataTable
{
public:
    DataTable();
    DataTable(Hermes::vector<double> points, Hermes::vector<double> values);
    DataTable(const DataTable& origin);

    void setValues(Hermes::vector<double> points, Hermes::vector<double> values);
    void setValues(vector<double> points, vector<double> values);
    void setValues(double *keys, double *values, int count);

    void setType(DataTableType type);
    void setSplineFirstDerivatives(bool fd);
    void setExtrapolateConstant(bool ec);

    double value(double x) const;
    double derivative(double x) const;
    inline int size() const { return m_numPoints; }
    inline bool isEmpty() const {return m_isEmpty; }
    DataTableType type() const {return m_type;}
    bool splineFirstDerivatives() const {return m_splineFirstDerivatives; }
    bool extrapolateConstant() const {return m_extrapolateConstant; }

    void clear();

    double minKey() const;
    double maxKey() const;
    double minValue() const;
    double maxValue() const;

    inline Hermes::vector<double> pointsVector() const { return m_points; }
    inline Hermes::vector<double> valuesVector() const { return m_values; }

    void checkTable();

    QString toString() const;
    QString toStringX() const;
    QString toStringY() const;
    QString toStringSetting() const;
    void fromString(const QString &str);

private:
    void inValidate();
    void validate();

    void setImplicit();
    void propertiesFromString(const QString &str);

    Hermes::vector<double> m_points;
    Hermes::vector<double> m_values;

    bool m_valid;

    DataTableType m_type;
    bool m_splineFirstDerivatives;
    bool m_extrapolateConstant;

    QSharedPointer<Hermes::Hermes2D::CubicSpline> m_spline;
    QSharedPointer<PiecewiseLinear> m_linear;
    QSharedPointer<ConstantTable> m_constant;

    // efficiency reasons
    int m_numPoints;
    bool m_isEmpty;
};


#endif // DATATABLE_H
