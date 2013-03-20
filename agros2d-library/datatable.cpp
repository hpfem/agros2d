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
    : Hermes::Hermes2D::CubicSpline(Hermes::vector<double>(), Hermes::vector<double>(), 0.0, 0.0)
{
}

DataTable::DataTable(Hermes::vector<double> points, Hermes::vector<double> values)
    : Hermes::Hermes2D::CubicSpline(Hermes::vector<double>(), Hermes::vector<double>(), 0.0, 0.0)
{
    add(points, values);
}

DataTable::DataTable(double key, double value)
    : Hermes::Hermes2D::CubicSpline(Hermes::vector<double>(), Hermes::vector<double>(), 0.0, 0.0)
{
    add(key, value);
}

DataTable::DataTable(double *keys, double *values, int count)
    : Hermes::Hermes2D::CubicSpline(Hermes::vector<double>(), Hermes::vector<double>(), 0.0, 0.0)
{
    add(keys, values, count);
}

void DataTable::clear()
{
    points.clear();
    values.clear();

    this->calculate_coeffs();
}

void DataTable::remove(double key)
{
    Hermes::vector<double>::iterator ip = points.begin();
    Hermes::vector<double>::iterator iv = values.begin();

    while (ip != points.end())
    {
        if (fabs((*ip) - key) < EPS_ZERO)
        {
            ip = points.erase(ip);
            iv = values.erase(iv);

            break;
        }
        ++ip;
        ++iv;
    }
}

void DataTable::add(double key, double value, bool calculate)
{
    Hermes::vector<double>::iterator ip = points.begin();
    Hermes::vector<double>::iterator iv = values.begin();

    // first value
    if (key < (*ip))
    {
        points.insert(ip, key);
        values.insert(iv, value);
    }


    while (ip != points.end())
    {
        // key already exists -> replace value
        if (((*ip) - key) < EPS_ZERO)
        {
            (*iv) = value;
            break;
        }

        ++ip;
        ++iv;
    }

    // last value
    if (key > (*ip))
    {
        points.push_back(key);
        values.push_back(value);
    }

    calculate_coeffs();
}

void DataTable::add(double *keys, double *values, int count)
{
    for (int i = 0; i < count; i++)
    {
        this->points.push_back(keys[i]);
        this->values.push_back(values[i]);
    }

    calculate_coeffs();
}

void DataTable::add(vector<double> points, vector<double> values)
{
    assert(points.size() == values.size());

    for (int i = 0; i < points.size(); i++)
    {
        this->points.push_back(points[i]);
        this->values.push_back(values[i]);
    }

    calculate_coeffs();
}

void DataTable::add(Hermes::vector<double> points, Hermes::vector<double> values)
{
    assert(points.size() == values.size());

    this->points = points;
    this->values = values;

    calculate_coeffs();
}

int DataTable::size() const
{
    assert(points.size() == values.size());

    return points.size();
}

double DataTable::minKey()
{   
    double min = 0.0;
    foreach (double point, points)
        min = std::min(point, min);

    return min;
}

double DataTable::maxKey()
{
    double max = 0.0;
    foreach (double point, points)
        max = std::max(point, max);

    return max;
}

double DataTable::minValue()
{
    double min = 0.0;
    foreach (double value, values)
        min = std::min(value, min);

    return min;
}

double DataTable::maxValue()
{
    double max = 0.0;
    foreach (double value, values)
        max = std::max(value, max);

    return max;
}

QString DataTable::toString() const
{
    return (toStringX() + ";" + toStringY());
}

QString DataTable::toStringX() const
{
    QString str;

    for (int i = 0; i < points.size(); i++)
    {
        str += QString::number(points[i]);
        if (i < points.size() - 1)
            str += ",";
    }

    return str;
}

QString DataTable::toStringY() const
{
    QString str;

    for (int i = 0; i < values.size(); i++)
    {
        str += QString::number(values[i]);
        if (i < values.size() - 1)
            str += ",";
    }

    return str;
}

void DataTable::fromString(const std::string &str)
{
    // clear
    points.clear();
    values.clear();

    std::string::const_iterator pos = std::find(str.begin(), str.end(), ';');

    std::string str_keys(str.begin(), pos);
    std::string str_values(pos + 1, str.end());

    double number;

    // keys
    std::istringstream i_keys(str_keys);
    while (i_keys >> number)
    {
        points.push_back(number);
        if (i_keys.peek() == ',')
            i_keys.ignore();
    }

    // values
    std::istringstream i_values(str_values);
    while (i_values >> number)
    {
        values.push_back(number);
        if (i_values.peek() == ',')
            i_values.ignore();
    }

    assert(points.size() == values.size());

    calculate_coeffs();
}

/*
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
    _assert(fabs(table.minKey() - -10.0) < EPS_ZERO);
    _assert(fabs(table.maxKey() - 40.0) < EPS_ZERO);
    _assert(fabs(table.minValue() - 4.0) < EPS_ZERO);
    _assert(fabs(table.maxValue() - 20.0) < EPS_ZERO);

    table.remove(30); // row exists
    table.remove(31); // row doesn't exist

    // size
    _assert(table.size() == 6);

    // table.print();
}
*/
