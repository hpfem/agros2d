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
    : m_valid(false), m_points(points), m_values(values), m_numPoints(0), m_isEmpty(true), m_type(DataTableType_PiecewiseLinear)
{
    setImplicit();
    checkTable();
}

DataTable::DataTable(const DataTable &origin)
{
    m_points = origin.m_points;
    m_values = origin.m_values;

    m_valid = false;

    m_type = origin.m_type;
    m_splineFirstDerivatives = origin.m_splineFirstDerivatives;
    m_extrapolateConstant = origin.m_extrapolateConstant;

    m_spline = QSharedPointer<Hermes::Hermes2D::CubicSpline>();
    m_linear = QSharedPointer<PiecewiseLinear>();
    m_constant = QSharedPointer<ConstantTable>();

    m_numPoints = origin.m_numPoints;
    m_isEmpty = origin.m_isEmpty;
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

    checkTable();
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

    checkTable();
}

void DataTable::setValues(double *keys, double *values, int count)
{
    inValidate();
    m_points.clear();
    m_values.clear();
    for (int i = 0; i < count; i++)
    {
        m_points.push_back(keys[i]);
        m_values.push_back(values[i]);
    }

    checkTable();
}

void DataTable::checkTable()
{
    if (m_points.size() > m_values.size())
    {
        clear();
        throw AgrosException(QObject::tr("Size doesn't match (%1 > %2).").arg(m_points.size()).arg(m_values.size()));
    }

    if (m_points.size() < m_values.size())
    {
        clear();
        throw AgrosException(QObject::tr("Size doesn't match (%1 < %2).").arg(m_points.size()).arg(m_values.size()));
    }

    for (int i = 1; i < m_points.size(); i++)
    {
        if (m_points[i] < m_points[i-1])
        {
            clear();
            throw AgrosException(QObject::tr("Points must be in ascending order (%1 < %2).").arg(m_points[i]).arg(m_points[i-1]));
        }
    }

    m_numPoints = m_points.size();
    m_isEmpty = (m_numPoints == 0);
}

void DataTable::setType(DataTableType type)
{
    inValidate();
    m_type = type;
}

void DataTable::setSplineFirstDerivatives(bool fd)
{
    inValidate();
    m_splineFirstDerivatives = fd;
}

void DataTable::setExtrapolateConstant(bool ec)
{
    inValidate();
    m_extrapolateConstant = ec;
}


void DataTable::setImplicit()
{
    m_spline.clear();
    m_linear.clear();
    m_constant.clear();
    m_type = DataTableType_PiecewiseLinear;
    m_splineFirstDerivatives = true;
    m_extrapolateConstant = true;
    m_valid = false;
    m_numPoints = 0;
    m_isEmpty = true;
}

double DataTable::value(double x)
{
    // todo: get rid of this, make value const function !
    if (!m_valid)
    {
#pragma omp critical (validation)
        {
            if (!m_valid)
                validate();
        }
    }

    if (m_type == DataTableType_PiecewiseLinear)
    {
        return m_linear.data()->value(x);
    }
    else if (m_type == DataTableType_CubicSpline)
    {
        return m_spline.data()->value(x);
    }
    else if (m_type == DataTableType_Constant)
    {
        return m_constant.data()->value(x);
    }
    else
        assert(0);
}

double DataTable::derivative(double x)
{    
    // todo: get rid of this, make derivative const function !
    if (!m_valid)
    {
#pragma omp critical (validation)
        {
            if (!m_valid)
                validate();
        }
    }

    if (m_type == DataTableType_PiecewiseLinear)
    {
        return m_linear.data()->derivative(x);
    }
    else if (m_type == DataTableType_CubicSpline)
    {
        return m_spline.data()->derivative(x);
    }
    else if (m_type == DataTableType_Constant)
    {
        return m_constant.data()->derivative(x);
    }
    else
        assert(0);
}

void DataTable::inValidate()
{
    m_valid = false;
    m_numPoints = m_points.size();
    m_isEmpty = (m_numPoints == 0);

    m_linear.clear();
    m_spline.clear();
    m_constant.clear();
}

void DataTable::validate()
{
    // object may have been validated by other thread. In that case, leave.
    if(m_valid)
        return;

    assert(m_linear.isNull());
    assert(m_spline.isNull());
    assert(m_constant.isNull());
    assert(!m_valid);

    assert(m_points.size() == m_values.size());
    m_numPoints = 0;
    m_isEmpty = true;

    if (m_type == DataTableType_PiecewiseLinear)
    {
        m_linear = QSharedPointer<PiecewiseLinear>(new PiecewiseLinear(m_points, m_values));
    }
    else if (m_type == DataTableType_CubicSpline)
    {
        // first create object and calculate coeffs, than assign to m_spline
        // otherwise probably some other thread interfered and caused application fail
        // this is strange, though, since no other thread should be able to access thanks to mutex check
        try
        {
            QSharedPointer<Hermes::Hermes2D::CubicSpline> spline
                    = QSharedPointer<Hermes::Hermes2D::CubicSpline>(new Hermes::Hermes2D::CubicSpline(m_points,
                                                                                                      m_values,
                                                                                                      0.0, 0.0,
                                                                                                      m_splineFirstDerivatives, m_splineFirstDerivatives,
                                                                                                      !m_extrapolateConstant, !m_extrapolateConstant));
            spline.data()->calculate_coeffs();
            m_spline = spline;
        }
        catch (Hermes::Exceptions::Exception e)
        {
            m_valid = false;
            qDebug() << "Exception thrown from Hermes::Hermes2D::CubicSpline has been ignored. DataTable not validated!";
            return;
        }

    }
    else if(m_type == DataTableType_Constant)
    {
        m_constant = QSharedPointer<ConstantTable>(new ConstantTable(m_points, m_values));
    }
    else
        assert(0);

    m_numPoints = m_points.size();
    m_isEmpty = (m_numPoints == 0);
    m_valid = true;
}

double DataTable::minKey() const
{
    double min = numeric_limits<double>::max();
    foreach (double point, m_points)
        min = std::min(point, min);

    return min;
}

double DataTable::maxKey() const
{
    double max = - numeric_limits<double>::max();
    foreach (double point, m_points)
        max = std::max(point, max);

    return max;
}

double DataTable::minValue() const
{
    double min = numeric_limits<double>::max();
    foreach (double value, m_values)
        min = std::min(value, min);

    return min;
}

double DataTable::maxValue() const
{
    double max = - numeric_limits<double>::max();
    foreach (double value, m_values)
        max = std::max(value, max);

    return max;
}

QString DataTable::toString() const
{
    return (toStringSetting() + "$" + toStringX() + ";" + toStringY());
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

QString DataTable::toStringSetting() const
{
    QString str;
    str += dataTableTypeToStringKey(m_type);
    str += ",";
    str += QString::number(int(m_splineFirstDerivatives));
    str += ",";
    str += QString::number(int(m_extrapolateConstant));
    // todo: add more settings here, separated by comas

    return str;
}

void DataTable::propertiesFromString(const QString &str)
{
    QStringList lst = str.split(",");
    if(lst.size() >= 1)
        m_type = dataTableTypeFromStringKey(lst.at(0));

    if(lst.size() >= 2)
        m_splineFirstDerivatives = lst.at(1).toInt();

    if(lst.size() >= 3)
        m_extrapolateConstant = lst.at(2).toInt();

    // todo: read more settings here
}

void DataTable::fromString(const QString &str)
{
    setImplicit();
    inValidate();

    // clear
    m_points.clear();
    m_values.clear();

    QString rest(str);
    if(str.contains("$"))
    {
        QStringList list = str.split("$");
        propertiesFromString(list.at(0));
        rest = list.at(1);
    }

    QStringList lst = rest.split(";");

    QStringList lstPts = lst.at(0).split(",");
    foreach (QString numStr, lstPts)
        m_points.push_back(numStr.toDouble());

    QStringList lstVal = lst.at(1).split(",");
    foreach (QString numStr, lstVal)
        m_values.push_back(numStr.toDouble());

    checkTable();
}


ConstantTable::ConstantTable(Hermes::vector<double> points, Hermes::vector<double> values)
{
    double sum = 0;
    int size = values.size();
    for(int i = 0; i < size; i++)
        sum += values[i];

    m_value = sum / size;
}

double ConstantTable::value(double x) const
{
    return m_value;
}

double ConstantTable::derivative(double x) const
{
    return 0;
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
    // slower implementation
    //
    // Hermes::vector<double>::const_iterator it = std::lower_bound(m_points.begin(), m_points.end(), x);
    // if (it == m_points.end())
    //     return m_size - 1;
    // else if (it == m_points.begin())
    //     return 0;
    // else
    //     return (it - m_points.begin()) - 1;

    int i_left = 0;
    int i_right = m_size - 1;
    assert(i_right >= 0);

    while (i_left + 1 < i_right)
    {
        int i_mid = (i_left + i_right) / 2;
        if (m_points[i_mid] < x)
            i_left = i_mid;
        else
            i_right = i_mid;
    }
    return i_left;
}

double PiecewiseLinear::value(double x)
{
    if (x < m_points.front())
    {
        return m_values.front();
    }
    else if (x > m_points.back())
    {
        return m_values[m_size - 1];
    }
    else
    {
        int leftIdx = leftIndex(x);
        return m_values[leftIdx] + m_derivatives[leftIdx] * (x - m_points[leftIdx]);
    }
}

double PiecewiseLinear::derivative(double x)
{
    if (x < m_points.front())
    {
        return 0.0;
    }
    else if (x > m_points.back())
    {
        return 0.0;
    }
    else
    {
        int leftIdx = leftIndex(x);
        return m_derivatives[leftIdx];
    }
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
