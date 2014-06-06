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

#include "optilab_data.h"

#include "assert.h"

OutputVariable::OutputVariable()
    : m_number(0.0), m_name(""), m_x(QVector<double>()), m_y(QVector<double>())
{
}

OutputVariable::OutputVariable::OutputVariable(const QString &valueName, const QString &valueString)
    : m_number(0.0), m_name(valueName), m_x(QVector<double>()), m_y(QVector<double>())
{
    bool isNum;

    valueString.toDouble(&isNum);
    if (isNum)
    {
        m_number = valueString.toDouble();
    }
    else
    {
        m_number = -1;
        /*
        QString valueTMP = valueString;

        QStringList data = valueTMP.replace("[[", "").replace("]]", "").split("], [");
        QStringList dataX = data[0].split(",");
        QStringList dataY = data[1].split(",");

        m_x.clear();
        m_y.clear();
        for (int i = 0; i < dataX.size(); i++)
        {
            m_x.append(dataX[i].toDouble());
            m_y.append(dataY[i].toDouble());
        }
        */
    }
}

int OutputVariable::size() const
{
    assert(m_x.size() == m_y.size());

    return m_x.size();
}

// *****************************************************************************

void OutputVariablesAnalysis::append(int index, const QList<OutputVariable> &variables)
{
    m_variables[index] = variables;
    m_cacheUpToDate = false;
}

void OutputVariablesAnalysis::clear()
{
    foreach (QList<OutputVariable> varList, m_variables.values())
        varList.clear();

    m_variables.clear();
    m_cacheUpToDate = false;
}

QStringList OutputVariablesAnalysis::names(bool onlyNumbers) const
{
    QStringList nms;

    // TODO: do it better
    // get variable names from first variable
    if (m_variables.size() > 0)
    {
        for (int j = 0; j < m_variables.values().at(0).size(); j++)
        {
            const OutputVariable *variable = &m_variables.values().at(0).at(j);
            if ((onlyNumbers && variable->isNumber()) || !onlyNumbers)
                nms.append(variable->name());
        }
    }

    return nms;
}

void OutputVariablesAnalysis::updateCache()
{
    m_variablesCache.clear();
    if(variables().size() == 0)
        return;

    QList<OutputVariable> variableList = variables().values().at(0);
    foreach (OutputVariable variable, variableList)
        m_variablesCache.insert(variable.name(), QVector<double>());

    foreach(QList<OutputVariable> variableList, variables())
    {
        foreach (OutputVariable variable, variableList)
        {
            m_variablesCache[variable.name()].append(variable.number());
        }
    }

    m_cacheUpToDate = true;
}

QVector<double> OutputVariablesAnalysis::values(const QString &name)
{
    if(!m_cacheUpToDate)
        updateCache();

    return m_variablesCache[name];
}


double OutputVariablesAnalysis::value(int index, const QString &name) const
{
    assert(m_variables.contains(index));

    for (int j = 0; j < m_variables[index].size(); j++)
    {
        const OutputVariable *variable = &m_variables[index].at(j);

        if (variable->name() == name)
            return variable->number();
    }

    assert(0);
}

// the following implementation of value would have different meaning in the case that keys in
// variables are not 0, 1, 2, ... max
// but now, on the other hand, values(name)[index] != value(name, index)
// this is probably not good!

//double OutputVariablesAnalysis::value(int index, const QString &name)
//{
//    if(!m_cacheUpToDate)
//        updateCache();

//    assert(m_variables.contains(index));

//    return m_variablesCache[name].at(index);
//}
