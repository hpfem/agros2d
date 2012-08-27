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

#ifndef POST_VALUES_H
#define POST_VALUES_H

#include "util.h"
#include "hermes2d.h"
#include "module.h"

class FieldInfo;
class CouplingInfo;
class Material;
class SceneLabelContainer;

namespace Module
{
    struct Integral;
    struct MaterialTypeVariable;    
}

struct SceneMaterial;

class PostprocessorValue
{
public:
    PostprocessorValue(FieldInfo *fieldInfo)  : m_fieldInfo(fieldInfo) {}
    ~PostprocessorValue();

    void setMaterialToParsers(Material *material);

protected:
    FieldInfo *m_fieldInfo;

    // parser
    QMap<std::string, double> m_parserVariables;
    QList<mu::Parser *> m_parsers;

    QMap<Module::MaterialTypeVariable *, mu::Parser *> m_parsersNonlinear;

    void setNonlinearParsers();
    void setNonlinearMaterial(Material *material);
};

class PostprocessorIntegralValue : public PostprocessorValue
{
public:
    PostprocessorIntegralValue(FieldInfo *fieldInfo) : PostprocessorValue(fieldInfo) {}

    inline QMap<Module::Integral*, double> values() const { return m_values; }

    void initParser(QList<Module::Integral *> list);

protected:
    // variables
    QMap<Module::Integral*, double> m_values;
};

class LocalForceValue : public PostprocessorValue
{
public:
    LocalForceValue(FieldInfo *fieldInfo);
    ~LocalForceValue();

    Point3 calculate(const Point3 &point, const Point3 &velocity = Point3());

private:
    mu::Parser *addParser(const QString &exp);

    double px;
    double py;
    double pvelx;
    double pvely;
    double pvelz;

    double *pvalue;
    double *pdx;
    double *pdy;
};


#endif // POST_VALUES_H
