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

class DataTable : public Hermes::Hermes2D::CubicSpline
{
public:
    DataTable();
    DataTable(Hermes::vector<double> points, Hermes::vector<double> values);
    DataTable(double key, double value);
    DataTable(double *keys, double *values, int count);

    void clear();
    void remove(double key);

    void add(double key, double value, bool calculate);
    void add(double *keys, double *values, int count);
    void add(Hermes::vector<double> points, Hermes::vector<double> values);
    void add(vector<double> points, vector<double> values);

    int size();

    double minKey();
    double maxKey();
    double minValue();
    double maxValue();

    inline Hermes::vector<double> pointsVector() { return points; }
    inline Hermes::vector<double> valuesVector() { return values; }

    QString toString() const;
    QString toStringX() const;
    QString toStringY() const;
    void fromString(const std::string &str);
    inline void fromString(const QString &str) { fromString(str.toStdString()); }

    // void save(const char *filename, double start, double end, int count);

private:
};

#endif // DATATABLE_H
