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
    Marker(FieldInfo *fieldInfo, std::string name);
    virtual ~Marker();

    /// value of one individual variable
    const Value getValue(std::string id);

    /// get all values
    const map<string, Value> getValues() const;

    void setValue(string name, Value value) { values[name] = value; }

    /// return name
    std::string getName() {return name; }

    /// set name
    void setName(string paramName) { name = paramName; }

    FieldInfo *getFieldInfo() { return fieldInfo; }

    /// returns QString identification of field
    QString fieldId();

    /// ????
    void evaluate(std::string id, double time);

    /// returns true if all OK
    bool evaluateAllVariables();

    int isNone() const { return m_isNone;}
    void setNone() {m_isNone = true;}

private:
    std::string name;
    bool m_isNone;

protected:
    FieldInfo *fieldInfo;

    /// variables - the way to customize boundary "template", given by the type parameter
    std::map<std::string, Value> values;

private:
    /// we don't want those objects to be copied since we compare pointers
    Marker(const Marker& );
    Marker& operator =(const Marker& );

};


class Boundary : public Marker
{
public:
    Boundary(FieldInfo *fieldInfo, std::string name = "", std::string type = "",
             std::map<std::string, Value> values = (std::map<std::string, Value>()));

    /// get type
    const std::string getType() const {return type;}

    /// set type
    void setType(string p_type) { type = p_type; }

private:
    /// type of boundary condition, taken from respective module
    std::string type;


};


class Material : public Marker
{
public:
    Material(FieldInfo *fieldInfo, std::string name,
             std::map<std::string, Value> values = (std::map<std::string, Value>()));
};


#endif // MARKER_H
