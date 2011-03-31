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

#ifndef TIMEFUNCTION_H
#define TIMEFUNCTION_H

#include "util.h"

class TimeFunction
{
public:
    Point point;

    TimeFunction(const QString &function, double time_min, double time_max, double N = 100);
    ~TimeFunction();

    void setFunction(const QString &function, double time_min, double time_max, double N = 100);

    double value(double time) const;
private:
    double m_time_min;
    double m_time_max;
    int m_N;

    QString m_function;

    QList<double> m_times;
    QList<double> m_values;

    void fillValues();
};


#endif // TIMEFUNCTION_H
