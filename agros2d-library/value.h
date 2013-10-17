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

#ifndef VALUE_H
#define VALUE_H

#include "util.h"
#include "util/point.h"
#include "hermes2d/module.h"

#include "datatable.h"

class DataTable;
class FieldInfo;

class AGROS_LIBRARY_API Value
{
public:
    Value(double value = 0.0);
    Value(double value,
          std::vector<double> x, std::vector<double> y, DataTableType type = DataTableType_PiecewiseLinear, bool splineFirstDerivatives = true, bool extrapolateConstant = true);

    Value(const QString &value);
    Value(const QString &value,
          std::vector<double> x, std::vector<double> y, DataTableType type = DataTableType_PiecewiseLinear, bool splineFirstDerivatives = true, bool extrapolateConstant = true);
    Value(const QString &value, const DataTable &table);

    // Copy constructor to be used e.g. for clonning, so that we have one instance of Value for each thread (-> thread safe? )
    //Value(const Value& origin);

    ~Value();

    // expression
    inline double number() const { assert(m_isEvaluated); return m_number; }
    double numberAtPoint(const Point &point, bool evaluate = true);
    double numberAtTime(double time, bool evaluate = true);
    double numberAtTimeAndPoint(double time, const Point &point, bool evaluate = true);

    bool isNumber();
    inline bool isTimeDependent() const { return m_isTimeDependent; }
    inline bool isCoordinateDependent() const { return m_isCoordinateDependent; }

    bool evaluateAtPoint(const Point &point);
    bool evaluateAtTime(double time);
    bool evaluateAtTimeAndPoint(double time, const Point &point);

    // table
    double numberFromTable(double key) const;
    Hermes::Ord numberFromTable(Hermes::Ord ord) const;
    double derivativeFromTable(double key);
    Hermes::Ord derivativeFromTable(Hermes::Ord ord);

    bool hasTable() const;

    void setText(const QString &str);
    inline QString text() const { return m_text; }

    QString toString() const;
    void parseFromString(const QString &str);

    inline DataTable table() const { return m_table; }

protected:

private:
    bool m_isEvaluated;

    // expression
    double m_number;
    QString m_text;
    double m_time;
    Point m_point;
    bool m_isTimeDependent;
    bool m_isCoordinateDependent;

    // table
    DataTable m_table;

    // evaluate
    bool evaluate();
    bool evaluateExpression(const QString &expression);

    friend class ValueLineEdit;
};

#endif // VALUE_H
