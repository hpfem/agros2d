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

#include "value.h"

#include "util/global.h"
#include "logview.h"
#include "pythonlab/pythonengine_agros.h"
#include "hermes2d/problem_config.h"
#include "parser/lex.h"

Value::Value(double value)
    : m_isEvaluated(true), m_isTimeDependent(false), m_isCoordinateDependent(false), m_time(0.0), m_point(Point()), m_table(DataTable())
{
    m_text = QString::number(value);
    m_number = value;
}

Value::Value(double value, std::vector<double> x, std::vector<double> y)
    : m_isEvaluated(true), m_isTimeDependent(false), m_isCoordinateDependent(false), m_time(0.0), m_point(Point()), m_table(DataTable())
{
    assert(x.size() == y.size());

    m_text = QString::number(value);
    m_number = value;
    m_table.add(x, y);
}

Value::Value(const QString &value)
    : m_isEvaluated(false), m_isTimeDependent(false), m_isCoordinateDependent(false), m_time(0.0), m_point(Point()), m_table(DataTable())
{
    parseFromString(value.isEmpty() ? "0" : value);
}

Value::Value(const QString &value, std::vector<double> x, std::vector<double> y)
    : m_isEvaluated(false), m_isTimeDependent(false), m_isCoordinateDependent(false), m_time(0.0), m_point(Point()), m_table(DataTable())
{
    assert(x.size() == y.size());

    parseFromString(value.isEmpty() ? "0" : value);
    m_table.add(x, y);
}

Value::Value(const QString &value, const DataTable &table)
    : m_isEvaluated(false), m_isTimeDependent(false), m_isCoordinateDependent(false), m_time(0.0), m_point(Point()), m_table(table)
{
    parseFromString(value.isEmpty() ? "0" : value);
}

Value::~Value()
{
    m_table.clear();
}

bool Value::hasExpression()
{
    return (QString::number(number()) != text());
}

bool Value::hasTable() const
{
    return (m_table.size() > 0);
}

bool Value::evaluateAtPoint(const Point &point)
{
    m_point = point;
    return evaluate();
}

bool Value::evaluateAtTime(double time)
{
    m_time = time;
    return evaluate();
}

bool Value::evaluateAtTimeAndPoint(double time, const Point &point)
{
    m_time = time;
    m_point = point;
    return evaluate();
}

double Value::numberAtPoint(const Point &point)
{
    // force evaluate
    evaluateAtPoint(point);

    return number();
}

double Value::numberAtTime(double time)
{
    // force evaluate
    evaluateAtTime(time);

    return number();
}

double Value::numberAtTimeAndPoint(double time, const Point &point)
{
    // force evaluate
    evaluateAtTimeAndPoint(time, point);

    return number();
}

double Value::numberFromTable(double key)
{
    if (Agros2D::problem()->isNonlinear() && hasTable())
        return m_table.value(key);
    else
        return number();
}

Hermes::Ord Value::numberFromTable(Hermes::Ord key)
{
    return Hermes::Ord(1);
}

double Value::derivativeFromTable(double key)
{
    if (Agros2D::problem()->isNonlinear() && hasTable())
        return m_table.derivative(key);
    else
        return 0.0;
}

Hermes::Ord Value::derivativeFromTable(Hermes::Ord key)
{
    return Hermes::Ord(1);
}

void Value::setText(const QString &str)
{
    m_isEvaluated = false;
    m_text = str;

    m_isTimeDependent = false;
    m_isCoordinateDependent = false;

    LexicalAnalyser lex;
    lex.setExpression(m_text);
    foreach (Token token, lex.tokens())
    {
        if (token.type() == ParserTokenType_VARIABLE)
        {
            if (token.toString() == "time")
                m_isTimeDependent = true;
            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
            {
                if (token.toString() == "x" || token.toString() == "y")
                    m_isCoordinateDependent = true;
            }
            else
            {
                if (token.toString() == "r" || token.toString() == "z")
                    m_isCoordinateDependent = true;
            }
        }
    }

    evaluate();
}

QString Value::toString() const
{
    if (m_table.size() == 0)
        return m_text;
    else
        return m_text + ";" + m_table.toString();
}

void Value::parseFromString(const QString &str)
{
    if (str.contains(";"))
    {
        // string and table
        QStringList lst = str.split(";");
        this->setText(lst.at(0));

        m_table.fromString((lst.at(1) + ";" + lst.at(2)).toStdString());
    }
    else
    {
        // just string
        this->setText(str);
    }
}

bool Value::evaluate()
{
    return evaluateExpression(m_text);
}

bool Value::evaluateExpression(const QString &expression)
{
    // speed up
    if ((expression == "0") || (expression == "0.0"))
    {
        m_number = 0.0;
        m_isEvaluated = true;
        return true;
    }

    bool signalBlocked = currentPythonEngineAgros()->signalsBlocked();
    currentPythonEngineAgros()->blockSignals(true);

    if (m_isCoordinateDependent && !m_isTimeDependent)
    {
        if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
            currentPythonEngineAgros()->runExpression(QString("x = %1; y = %2").arg(m_point.x).arg(m_point.y), false);
        else
            currentPythonEngineAgros()->runExpression(QString("r = %1; z = %2").arg(m_point.x).arg(m_point.y), false);
    }

    if (m_isTimeDependent && !m_isCoordinateDependent)
    {
        currentPythonEngineAgros()->runExpression(QString("time = %1").arg(m_time), false);
    }

    if (m_isCoordinateDependent && m_isTimeDependent)
    {
        if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
            currentPythonEngineAgros()->runExpression(QString("time = %1; x = %2; y = %3").arg(m_time).arg(m_point.x).arg(m_point.y), false);
        else
            currentPythonEngineAgros()->runExpression(QString("time = %1; r = %2; z = %3").arg(m_time).arg(m_point.x).arg(m_point.y), false);
    }

    // eval expression
    ExpressionResult expressionResult = currentPythonEngineAgros()->runExpression(expression, true);
    if (expressionResult.error.isEmpty())
    {
        m_number = expressionResult.value;
    }
    else
    {
        Agros2D::log()->printDebug(QObject::tr("Problem"), expressionResult.error);
    }

    if (!signalBlocked)
        currentPythonEngineAgros()->blockSignals(false);

    m_isEvaluated = expressionResult.error.isEmpty();
    return m_isEvaluated;
}
