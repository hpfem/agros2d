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

#ifndef {{CLASS}}_EXTFUNCTION_H
#define {{CLASS}}_EXTFUNCTION_H

#include "util.h"
#include "solver/plugin_interface.h"
#include "solver/marker.h"
#include "{{ID}}_interface.h"

/*
{{#EXT_FUNCTIONS_PART}}
{{#EXT_FUNCTION}}
class {{EXT_FUNCTION_NAME}} : public AgrosExtFunction
{
public:
    virtual std::vector<double> value(dealii::FEValues<2> &fe_values, int n_q_points, SceneLabel *label) const;

protected:
    QList<QWeakPointer<Value> > {{QUANTITY_SHORTNAME}};
};
{{/EXT_FUNCTION}}
{{/EXT_FUNCTIONS_PART}}
*/


// PART OF EXT FUNCTIONS (with the same analysis, linearity and coordinate type
// **** {{PART_NAME}} *****
/*
{{#EXT_FUNCTION}}
class {{EXT_FUNCTION_NAME}} : public AgrosExtFunction
{
public:
    virtual void value(int n, Func<double>** ext, Func<double> **u_ext, Func<double> *result, Geom<double> *geometry) const;
    {{EXT_FUNCTION_NAME}}(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros);
    Function<double>* clone() const
    {
        return new {{EXT_FUNCTION_NAME}}(this->m_fieldInfo, this->m_wfAgros);
    }
protected:
    QList<QWeakPointer<Value> > {{QUANTITY_SHORTNAME}};
};
{{/EXT_FUNCTION}}
*/
// ***********************************************************************************************************************************
// ext functions with more quantities and expression (merge with previous!)

/*
{{#VALUE_FUNCTION_SOURCE}}
class {{VALUE_FUNCTION_FULL_NAME}} : public AgrosExtFunction
{
public:
    {{VALUE_FUNCTION_FULL_NAME}}(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros);
    virtual double getValue(int hermesMarker, double h) const;
    virtual void value(int n, Func<double>** ext, Func<double> **u_ext, Func<double> *result, Geom<double> *geometry) const;
    Function<double>* clone() const
    {
        return new {{VALUE_FUNCTION_FULL_NAME}}(this->m_fieldInfo, this->m_wfAgros);
    }
private:
{{#PARAMETERS_LINEAR}}    QList<QWeakPointer<Value> > {{PARAMETER_NAME}}_pointers;
{{/PARAMETERS_LINEAR}}
{{#PARAMETERS_NONLINEAR}}    QList<QWeakPointer<Value> > {{PARAMETER_NAME}}_pointers;
{{/PARAMETERS_NONLINEAR}}
};
{{/VALUE_FUNCTION_SOURCE}}
*/
// ***********************************************************************************************************************************
// Special functions
/*
{{#SPECIAL_FUNCTION_SOURCE}}
class {{SPECIAL_EXT_FUNCTION_FULL_NAME}} : public AgrosSpecialExtFunction
{
public:
    {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros);
    ~{{SPECIAL_EXT_FUNCTION_FULL_NAME}}();
    virtual double calculateValue(int hermesMarker, double h) const;
    virtual void value(int n, Func<double>** ext, Func<double> **u_ext, Func<double> *result, Geom<double> *geometry) const;
    private:
{{#PARAMETERS}}    QList<QWeakPointer<Value> > {{PARAMETER_NAME}}_pointers;
{{/PARAMETERS}}
};
{{/SPECIAL_FUNCTION_SOURCE}}
*/

#endif // {{CLASS}}_EXTFUNCTION_H
