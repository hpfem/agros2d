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
#include "spline.h"

enum DataTableType
{
    DataTableType_CubicSpline,
    DataTableType_PiecewiseLinear
};

class PiecewiseLinear
{
public:
    PiecewiseLinear(Hermes::vector<double> points, Hermes::vector<double> values);
    int leftIndex(double x);
    double value(double x);
    double derivative(double x);

private:
    Hermes::vector<double> m_points;
    Hermes::vector<double> m_values;

    Hermes::vector<double> m_derivatives;
    int m_size;
};

class DataTable
{
public:
    DataTable();
    DataTable(Hermes::vector<double> points, Hermes::vector<double> values);

    void setValues(Hermes::vector<double> points, Hermes::vector<double> values);
    void setValues(vector<double> points, vector<double> values);
    void setValues(double *keys, double *values, int count);
    void setType(DataTableType type);

    double value(double x);
    double derivative(double x);
    int size() const;
    void clear();

    double minKey() const;
    double maxKey() const;
    double minValue() const;
    double maxValue() const;

    inline Hermes::vector<double> pointsVector() { return m_points; }
    inline Hermes::vector<double> valuesVector() { return m_values; }

    QString toString() const;
    QString toStringX() const;
    QString toStringY() const;
    void fromString(const std::string &str);
    inline void fromString(const QString &str) { fromString(str.toStdString()); }

private:
    void inValidate();
    void validate();

    void setImplicit();

    Hermes::vector<double> m_points;
    Hermes::vector<double> m_values;
    bool m_valid;
    DataTableType m_type;

    Hermes::Hermes2D::CubicSpline *m_spline;
    PiecewiseLinear *m_linear;
};


#endif // DATATABLE_H
