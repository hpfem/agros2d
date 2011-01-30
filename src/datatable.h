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
#include "weakform/forms.h"

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

    void add(double key, double value);
    void add(double *keys, double *values, int count);
    void get(double *keys, double *values, double *derivatives);
    DataTable *copy() const;

    inline DataTableRow *data() { return m_data; }

    int size();

    double min_key();
    double max_key();
    double min_value();
    double max_value();

    double value(double key);
    Ord value(Ord key);

    double derivative(double key);
    Ord derivative(Ord key);

    void print();
    void save(const char *filename, double start, double end, int count);

private:
    DataTableRow *m_data;
};

#endif // DATATABLE_H
