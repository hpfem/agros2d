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

#ifndef OPTILABDATA_H
#define OPTILABDATA_H

#include "util.h"

class OutputVariable
{
public:
    OutputVariable();
    OutputVariable(const QString &valueName, const QString &valueString);

    inline QString name() const { return m_name; }

    inline bool isNumber() const { return m_x.size() == 0; }
    inline double number() const { return m_number; }

    inline QVector<double> x() const { return m_x; }
    inline QVector<double> y() const { return m_y; }
    int size() const;

private:
    // name
    QString m_name;

    // number
    double m_number;

    // table
    QVector<double> m_x;
    QVector<double> m_y;
};

class OutputVariablesAnalysis
{
public:
    inline QMap<int, QList<OutputVariable> > variables() const { return m_variables; }

    void append(int index, const QList<OutputVariable> &variables);
    inline int size() const { return m_variables.size(); }
    void clear();

    QStringList names(bool onlyNumbers = false) const;
    QVector<double> values(const QString &name) const;
    double value(int index, const QString &name) const;

private:
    QMap<int, QList<OutputVariable> > m_variables;
};

#endif // OPTILABDATA_H
