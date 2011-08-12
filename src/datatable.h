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
#include "forms.h"
#include "spline.h"

// data table row
struct DataTableRow
{
    DataTableRow() : key(0.0), value(0.0), next(NULL) {}

    double key;
    double value;
    DataTableRow *next;
};

class DataTable
{
public:
    DataTable();
    DataTable(double key, double value);
    DataTable(double *keys, double *values, int count);
    ~DataTable();

    void clear();
    void remove(double key);

    void add(double key, double value, bool init = true);
    void add(double *keys, double *values, int count);
    void get(double *keys, double *values, double *derivatives);
    DataTable *copy() const;

    void init_spline();

    inline DataTableRow *data() { return m_data; }

    int size();

    double min_key();
    double max_key();
    double min_value();
    double max_value();

    double value(double key);
    Hermes::Ord value(Hermes::Ord key);

    double derivative(double key);
    Hermes::Ord derivative(Hermes::Ord key);

    double value_spline(double key);
    Hermes::Ord value_spline(Hermes::Ord key);

    double derivative_spline(double key);
    Hermes::Ord derivative_spline(Hermes::Ord key);

    std::string to_string();
    void from_string(const std::string &str);

    void print();
    void save(const char *filename, double start, double end, int count);

private:
    DataTableRow *m_data;

    Hermes::Hermes2D::CubicSpline *m_spline;
};

#endif // DATATABLE_H
