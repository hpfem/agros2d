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

#ifndef MARKER_H
#define MARKER_H

#include "util.h"
#include "value.h"

class FieldInfo;

class AGROS_API Marker
{
public:
    Marker(FieldInfo *m_fieldInfo, QString m_name);
    virtual ~Marker();

    /// value of one individual variable
    Value value(QString id) const;

    /// get all values
    const QHash<QString, Value> values() const;

    void setValue(QString name, Value value) { m_values[name] = value; }

    /// return name
    QString name() {return m_name; }

    /// set name
    void setName(QString paramName) { m_name = paramName; }

    FieldInfo *fieldInfo() { return m_fieldInfo; }

    /// returns QString identification of field
    QString fieldId();

    /// ????
    bool evaluate(const QString &id, double time);

    /// returns true if all OK
    bool evaluateAllVariables();

    int isNone() const { return m_isNone;}
    void setNone() { m_isNone = true; }

private:
    QString m_name;
    bool m_isNone;

protected:
    FieldInfo *m_fieldInfo;

    /// variables - the way to customize boundary "template", given by the type parameter
    QHash<QString, Value> m_values;

private:
    /// we don't want those objects to be copied since we compare pointers
    Marker(const Marker& );
    Marker& operator =(const Marker& );

};


class Boundary : public Marker
{
public:
    Boundary(FieldInfo *m_fieldInfo, QString m_name = "", QString m_type = "",
             QHash<QString, Value> m_values = (QHash<QString, Value>()));

    /// get type
    const QString type() const { return m_type; }

    /// set type
    void setType(const QString &type) { m_type = type; }

private:
    /// type of boundary condition, taken from respective module
    QString m_type;


};


class Material : public Marker
{
public:
    Material(FieldInfo *m_fieldInfo, QString m_name,
             QHash<QString, Value> m_values = (QHash<QString, Value>()));
};


#endif // MARKER_H
