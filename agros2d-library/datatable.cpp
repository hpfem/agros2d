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

DataTable::DataTable() : m_valid(false)
{
    setImplicit();
}

DataTable::DataTable(Hermes::vector<double> points, Hermes::vector<double> values)
    : m_valid(false), m_points(points), m_values(values)
{
    setImplicit();
}

void DataTable::clear()
{
    setValues(Hermes::vector<double>(), Hermes::vector<double>());
}

void DataTable::setValues(Hermes::vector<double> points, Hermes::vector<double> values)
{
    inValidate();
    m_points = points;
    m_values = values;
}

void DataTable::setValues(vector<double> points, vector<double> values)
{
    assert(points.size() == values.size());

    inValidate();
    m_points.clear();
    m_values.clear();
    for (int i = 0; i < points.size(); i++)
    {
        m_points.push_back(points[i]);
        m_values.push_back(values[i]);
    }
}

void DataTable::setValues(double *keys, double *values, int count)
{
    inValidate();
    for (int i = 0; i < count; i++)
    {
        m_points.push_back(keys[i]);
        m_values.push_back(values[i]);
    }
}

void DataTable::setType(DataTableType type)
{
    inValidate();
    m_type = type;
}

void DataTable::setImplicit()
{
    m_spline = NULL;
    m_linear = NULL;
    m_type = DataTableType_CubicSpline;
}

double DataTable::value(double x)
{
    if(! m_valid)
        validate();

    if(m_type == DataTableType_PiecewiseLinear)
    {
        return m_linear->value(x);
    }
    else if(m_type == DataTableType_CubicSpline)
    {
        return m_spline->value(x);
    }
    else
        assert(0);
}

double DataTable::derivative(double x)
{
    if(! m_valid)
        validate();

    if(m_type == DataTableType_PiecewiseLinear)
    {
        return m_linear->derivative(x);
    }
    else if(m_type == DataTableType_CubicSpline)
    {
        return m_spline->derivative(x);
    }
    else
        assert(0);
}

void DataTable::inValidate()
{
    m_valid = false;

    if(m_type == DataTableType_PiecewiseLinear)
    {
        delete m_linear;
        m_linear = NULL;
    }
    else if(m_type == DataTableType_CubicSpline)
    {
        delete m_spline;
        m_spline = NULL;
    }
    else
        assert(0);

    assert(m_linear == NULL);
    assert(m_spline == NULL);
}

void DataTable::validate()
{
    assert(m_linear == NULL);
    assert(m_spline == NULL);

    if(m_type == DataTableType_PiecewiseLinear)
    {
        m_linear = new PiecewiseLinear(m_points, m_values);
    }
    else if(m_type == DataTableType_CubicSpline)
    {
        m_spline = new Hermes::Hermes2D::CubicSpline(m_points, m_values, 0.0, 0.0);
        m_spline->calculate_coeffs();
    }
    else
        assert(0);

    m_valid = true;
}

int DataTable::size() const
{
    assert(m_points.size() == m_values.size());

    return m_points.size();
}

double DataTable::minKey() const
{
    double min = 0.0;
    foreach (double point, m_points)
        min = std::min(point, min);

    return min;
}

double DataTable::maxKey() const
{
    double max = 0.0;
    foreach (double point, m_points)
        max = std::max(point, max);

    return max;
}

double DataTable::minValue() const
{
    double min = 0.0;
    foreach (double value, m_values)
        min = std::min(value, min);

    return min;
}

double DataTable::maxValue() const
{
    double max = 0.0;
    foreach (double value, m_values)
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

    for (int i = 0; i < m_points.size(); i++)
    {
        str += QString::number(m_points[i]);
        if (i < m_points.size() - 1)
            str += ",";
    }

    return str;
}

QString DataTable::toStringY() const
{
    QString str;

    for (int i = 0; i < m_values.size(); i++)
    {
        str += QString::number(m_values[i]);
        if (i < m_values.size() - 1)
            str += ",";
    }

    return str;
}

void DataTable::fromString(const std::string &str)
{
    inValidate();

    // clear
    m_points.clear();
    m_values.clear();

    std::string::const_iterator pos = std::find(str.begin(), str.end(), ';');

    std::string str_keys(str.begin(), pos);
    std::string str_values(pos + 1, str.end());

    double number;

    // keys
    std::istringstream i_keys(str_keys);
    while (i_keys >> number)
    {
        m_points.push_back(number);
        if (i_keys.peek() == ',')
            i_keys.ignore();
    }

    // values
    std::istringstream i_values(str_values);
    while (i_values >> number)
    {
        m_values.push_back(number);
        if (i_values.peek() == ',')
            i_values.ignore();
    }

    assert(m_points.size() == m_values.size());
}


PiecewiseLinear::PiecewiseLinear(Hermes::vector<double> points, Hermes::vector<double> values)
    : m_points(points), m_values(values)
{
    assert(m_points.size() == m_values.size());
    m_size = m_points.size();

    for(int i = 0; i < m_size - 1; i++)
    {
        m_derivatives.push_back((m_values[i+1] - m_values[i]) / (m_points[i+1] - m_points[i]));
    }
}

int PiecewiseLinear::leftIndex(double x)
{
  int i_left = 0;
  int i_right = m_size - 1;
  assert(i_right >= 0);

  if(x < m_points[i_left]) return -1;
  if(x > m_points[i_right]) return i_right;

  while (i_left + 1 < i_right)
  {
    int i_mid = (i_left + i_right) / 2;
    if(m_points[i_mid] < x) i_left = i_mid;
    else i_right = i_mid;
  }

  return i_left;
}

double PiecewiseLinear::value(double x)
{
    int leftIdx = leftIndex(x);
    if(leftIdx == -1)
        return m_values[0];
    else if(leftIdx == m_size - 1)
        return m_values[m_size - 1];
    else
        return m_values[leftIdx] + m_derivatives[leftIdx] * (x - m_points[leftIdx]);
}

double PiecewiseLinear::derivative(double x)
{
    int leftIdx = leftIndex(x);
    if(leftIdx == -1)
        return 0;
    else if(leftIdx == m_size - 1)
        return 0;
    else
        return m_derivatives[leftIdx];
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
