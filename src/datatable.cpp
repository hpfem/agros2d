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

#include "datatable.h"

DataTable::DataTable()
{
    m_data = NULL;
}

DataTable::DataTable(double key, double value)
{
    m_data = NULL;
    add(key, value, false);

    init_spline();
}

DataTable::DataTable(double *keys, double *values, int count)
{
    m_data = NULL;
    add(keys, values, count);
}

DataTable::~DataTable()
{
    clear();
}

void DataTable::clear()
{
    DataTableRow *data = m_data;
    while (data)
    {
        DataTableRow *tmp = data;

        // next row
        data = data->next;

        delete tmp;
    }

    m_data = NULL;
}

void DataTable::remove(double key)
{
    DataTableRow *previous = NULL;
    DataTableRow *data = m_data;

    while (data)
    {
        if (fabs(key - data->key) < EPS_ZERO)
        {
            previous->next = data->next;
            delete data;

            return;
        }

        // previous row
        previous = data;

        // next row
        data = data->next;
    }
}

void DataTable::add(double key, double value, bool init)
{
    DataTableRow *data = m_data;

    // first value
    if ((m_data && key < m_data->key) || (!m_data))
    {
        DataTableRow *tmp = new DataTableRow();

        tmp->key = key;
        tmp->value = value;
        tmp->next = data;
        m_data = tmp;

        return;
    }

    while (data)
    {
        // key already exists -> replace value
        if (fabs(key - data->key) < EPS_ZERO)
        {
            data->key = key;
            data->value = value;

            break;
        }

        // key inside
        if (data->next && key > data->key && key < data->next->key)
        {
            DataTableRow *tmp = new DataTableRow();

            tmp->key = key;
            tmp->value = value;
            tmp->next = data->next;
            data->next = tmp;

            break;
        }

        // last value
        if (!data->next)
        {
            DataTableRow *tmp = new DataTableRow();

            tmp->key = key;
            tmp->value = value;
            tmp->next = NULL;
            data->next = tmp;

            break;
        }

        // next value
        data = data->next;
    }

    if (init)
        init_spline();
}

void DataTable::add(double *keys, double *values, int count)
{
    for (int i = 0; i<count; i++)
    {
        add(keys[i], values[i], false);
    }

    init_spline();
}

void DataTable::get(double *keys, double *values, double *derivatives)
{
    DataTableRow *data = m_data;
    int i = 0;
    while (data)
    {
        DataTableRow *tmp = data;

        keys[i] = tmp->key;
        values[i] = tmp->value;
        derivatives[i] = derivative(tmp->key);

        // next row
        data = data->next;
        i++;
    }
}

DataTable *DataTable::copy() const
{
    DataTable *table = new DataTable();

    DataTableRow *data = m_data;
    while (data)
    {
        DataTableRow *tmp = data;

        table->add(tmp->key, tmp->value, false);

        // next row
        data = data->next;
    }

    table->init_spline();
    return table;
}

void DataTable::init_spline()
{
    int length = size();

    if (length <= 1)
        return;

    // prepare data
    double *x = new double[length];
    double *y = new double[length];

    DataTableRow *data = m_data;
    int i = 0;
    while (data)
    {
        DataTableRow *tmp = data;

        x[i] = tmp->key;
        y[i] = tmp->value;

        // next row
        data = data->next;
        i++;
    }

    //std::cout << "init spline" << std::endl;

    // alglib
    // delete old spline
    alglib::real_1d_array xi;
    alglib::real_1d_array yi;
    xi.setcontent(length, x);
    yi.setcontent(length, y);

    alglib::ae_int_t natural_bound_type = 2;
    // alglib::spline1dbuildcubic(xi, yi, length, natural_bound_type, 0.0, natural_bound_type, 0.0, m_splineAlglib);
    alglib::spline1dbuildakima(xi, yi, m_splineAlglib);
    // alglib::spline1dbuildlinear(xi, yi, m_splineAlglib);
    // alglib::spline1dbuildcatmullrom(xi, yi, m_splineAlglib);

    // cleanup
    delete [] x;
    delete [] y;
}

int DataTable::size()
{
    int size = 0;

    DataTableRow *data = m_data;
    while (data)
    {
        size++;

        // next row
        data = data->next;
    }

    return size;
}

double DataTable::min_key()
{
    if (m_data)
        return m_data->key;
    else
        return 0.0;
}

double DataTable::max_key()
{
    double max = 0.0;

    DataTableRow *data = m_data;
    while (data)
    {
        max = data->key;

        // next row
        data = data->next;
    }

    return max;
}

double DataTable::min_value()
{
    if (m_data)
        return m_data->value;
    else
        return 0.0;
}

double DataTable::max_value()
{
    double max = 0.0;

    DataTableRow *data = m_data;
    while (data)
    {
        max = data->value;

        // next row
        data = data->next;
    }

    return max;
}

double DataTable::value(double key)
{
    DataTableRow *data = m_data;

    // just one row
    if (!data->next)
        return data->value;

    // key < first value
    if (key <= data->key)
        return data->value;

    while (data)
    {
        // key > last value
        if (!data->next && key >= data->key)
            return data->value;

        // key
        if (fabs(key - data->key) < EPS_ZERO)
            return data->value;

        // linear interpolation between two keys
        if (data->next && key > data->key && key < data->next->key)
        {
            // std::cout << data->value + (key - data->key) / (data->next->key - data->key) * (data->next->value - data->value) << std::endl;
            // std::cout << key << std::endl;
            return data->value + (key - data->key) / (data->next->key - data->key) * (data->next->value - data->value);
        }

        // next row
        data = data->next;
    }
}

Ord DataTable::value(Ord key)
{
    return 1.0;
}

double DataTable::derivative(double key)
{
    DataTableRow *previous = NULL;
    DataTableRow *data = m_data;

    // just one row
    if (!data->next)
        return 0.0;

    // key < first value
    if (key <= data->key)
        return 0.0; // (data->next->value - data->value) / (data->next->key - data->key);

    while (data)
    {
        // key > last value
        if (!data->next && key >= data->key)
            return 0.0; // (data->value - previous->value) / (data->key - previous->key);

        // key
        if (data->next && fabs(key - data->key) < EPS_ZERO)
            return (previous->value - data->next->value) / (previous->key - data->next->key);

        // between two keys
        if (data->next && key >= data->key && key < data->next->key)
            return (data->next->value - data->value) / (data->next->key - data->key);

        // previous row
        previous = data;

        // next row
        data = data->next;
    }
}

Ord DataTable::derivative(Ord key)
{
    return 1.0;
}

double DataTable::value_spline(double key)
{
    if (m_data && m_data->next)
        return alglib::spline1dcalc(m_splineAlglib, key);
    else
        return 0.0;
}

Ord DataTable::value_spline(Ord key)
{
    return 1.0;
}

double DataTable::derivative_spline(double key)
{
    if (m_data && m_data->next)
    {
        double s, ds, d2s;
        alglib::spline1ddiff(m_splineAlglib, key, s, ds, d2s);
        return ds;
    }
    else
        return 0.0;
}

Ord DataTable::derivative_spline(Ord key)
{
    return 1.0;
}

std::string DataTable::to_string()
{
    std::string str_key;
    std::string str_value;

    DataTableRow *data = m_data;
    while (data)
    {
        std::ostringstream o_key;
        std::ostringstream o_value;

        o_key << data->key;
        str_key += o_key.str();
        o_value << data->value;
        str_value += o_value.str();

        // next row
        data = data->next;

        // separator
        if (data)
        {
            str_key += ",";
            str_value += ",";
        }
    }

    return (str_key + ";" + str_value);
}

void DataTable::from_string(const std::string &str)
{
    std::string::const_iterator pos = std::find(str.begin(), str.end(), ';');

    std::string str_keys(str.begin(), pos);
    std::string str_values(pos + 1, str.end());

    double number;

    // keys
    std::istringstream i_keys(str_keys);
    std::vector<double> keys_double;
    while (i_keys >> number)
    {
        keys_double.push_back(number);
        if (i_keys.peek() == ',')
            i_keys.ignore();
    }

    // values
    std::istringstream i_values(str_values);
    std::vector<double> values_double;
    while (i_values >> number)
    {
        values_double.push_back(number);
        if (i_values.peek() == ',')
            i_values.ignore();
    }

    // add to the array
    for (int i = 0; i < keys_double.size(); i++)
        add(keys_double[i], values_double[i], false);

    init_spline();
}

void DataTable::print()
{
    DataTableRow *data = m_data;
    while (data)
    {
        printf("%.14g\t%.14g", data->key, data->value);

        // next row
        data = data->next;
    }
}

void DataTable::save(const char *filename, double start, double end, int count)
{
    FILE* f = fopen(filename, "w");
    if (f == NULL) printf("Error writing to %s.", filename);

    for (double val = start; val <= end; val += (end - start) / (count-1))
    {
        fprintf(f, "%.14g\t%.14g\t%.14g\n", val, value(val), derivative(val));
    }

    fclose(f);
}



























void _assert(bool a)
{
    if (!a) throw std::runtime_error("Assertion failed.");
}

void test()
{
    DataTable table;

    table.add(0, 0);
    table.add(10, 20);
    table.add(20, 20);
    table.add(30, 40);
    table.add(40, 10);
    table.add(40, 20);
    table.add(-10, 4);
    table.add(35, 10);

    // table.print();

    // size
    _assert(table.size() == 7);

    // value
    _assert(fabs(table.value(-20) - 4.0) < EPS_ZERO);
    _assert(fabs(table.value(12) - 20.0) < EPS_ZERO);
    _assert(fabs(table.value(32) - 28.0) < EPS_ZERO);
    _assert(fabs(table.value(30) - 40.0) < EPS_ZERO);
    _assert(fabs(table.value(50) - 20.0) < EPS_ZERO);

    _assert(fabs(table.derivative(-20) - -0.4) < EPS_ZERO);
    _assert(fabs(table.derivative(12) - 0.0) < EPS_ZERO);
    _assert(fabs(table.derivative(32) - -6.0) < EPS_ZERO);
    _assert(fabs(table.derivative(30) - -2.0/3.0) < EPS_ZERO);
    _assert(fabs(table.derivative(50) - 2.0) < EPS_ZERO);

    // min and max
    _assert(fabs(table.min_key() - -10.0) < EPS_ZERO);
    _assert(fabs(table.max_key() - 40.0) < EPS_ZERO);
    _assert(fabs(table.min_value() - 4.0) < EPS_ZERO);
    _assert(fabs(table.max_value() - 20.0) < EPS_ZERO);

    table.remove(30); // row exists
    table.remove(31); // row doesn't exist

    // size
    _assert(table.size() == 6);

    // table.print();
}
