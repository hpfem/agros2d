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

class FieldInfo;

class Marker
{
public:
    Marker(FieldInfo *fieldInfo, QString name);
    virtual ~Marker();

    /// value of one individual variable
    Value getValue(QString id);

    /// get all values
    const map<QString, Value> getValues() const;

    void setValue(QString name, Value value) { values[name] = value; }

    /// return name
    QString getName() {return name; }

    /// set name
    void setName(QString paramName) { name = paramName; }

    FieldInfo *getFieldInfo() { return fieldInfo; }

    /// returns QString identification of field
    QString fieldId();

    /// ????
    void evaluate(QString id, double time);

    /// returns true if all OK
    bool evaluateAllVariables();

    int isNone() const { return m_isNone;}
    void setNone() {m_isNone = true;}

private:
    QString name;
    bool m_isNone;

protected:
    FieldInfo *fieldInfo;

    /// variables - the way to customize boundary "template", given by the type parameter
    std::map<QString, Value> values;

private:
    /// we don't want those objects to be copied since we compare pointers
    Marker(const Marker& );
    Marker& operator =(const Marker& );

};


class Boundary : public Marker
{
public:
    Boundary(FieldInfo *fieldInfo, QString name = "", QString type = "",
             std::map<QString, Value> values = (std::map<QString, Value>()));

    /// get type
    const QString getType() const {return type;}

    /// set type
    void setType(QString p_type) { type = p_type; }

private:
    /// type of boundary condition, taken from respective module
    QString type;


};


class Material : public Marker
{
public:
    Material(FieldInfo *fieldInfo, QString name,
             std::map<QString, Value> values = (std::map<QString, Value>()));
};


#endif // MARKER_H
