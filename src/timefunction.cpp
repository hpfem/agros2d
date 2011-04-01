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

#include "timefunction.h"
#include "scripteditordialog.h"

// TimeFunction timeFunction("sin(2*pi*50*time)", 0, 0.02);

TimeFunction::TimeFunction(const QString &function, double time_min, double time_max, double N)
{
    m_error = "";
    setFunction(function, time_min, time_max, N);
}

TimeFunction::~TimeFunction()
{
    m_times.clear();
    m_values.clear();
}

void TimeFunction::setFunction(const QString &function, double time_min, double time_max, double N)
{
    m_error = "";
    m_function = function;
    m_time_min = time_min;
    m_time_max = time_max;
    m_N = N;

    fillValues();
}

void TimeFunction::showError()
{
    if (!m_error.isEmpty())
        QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Time function error"), m_error);
}

bool TimeFunction::fillValues(bool quiet)
{
    m_times.clear();
    m_values.clear();

    if (m_function.isEmpty())
        m_function = "0.0";

    // times
    double dt = (m_time_max - m_time_min) / (m_N + 1);
    for (int i = 0; i < m_N; i++)
        m_times.append(i * dt);

    // values
    QString error = fillTimeFunction(m_function, m_time_min, m_time_max, m_N, &m_values);

    if (error.isEmpty())
    {
        m_error = "";
        return true;
    }
    else
    {
        m_error = error;
        if (!quiet)
            showError();
        return false;
    }
}

double TimeFunction::value(double time) const
{   
    if (m_times.length() == 0)
        return 0.0;

    // first value
    if (time <= m_times.at(0))
        return m_values.at(0);

    // last value
    if (time >= m_times.last())
        return m_values.last();

    // linear approach
    for (int i = 0; i < m_times.length() - 1; i++)
        if ((m_times.at(i) >= time) && (time <= m_times.at(i+1)))
            return m_values.at(i) + ((time - m_times.at(i)) / (m_times.at(i+1) - m_times.at(i))) * (m_values.at(i+1) - m_values.at(i));

    return m_values.last();
}
