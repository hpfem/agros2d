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

#include "marker.h"
#include "module.h"
#include "scene.h"
#include "util.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

Marker::Marker(const FieldInfo *fieldInfo, QString name)
    : m_fieldInfo(fieldInfo), m_name(name), m_isNone(false)
{
}

Marker::~Marker()
{
    m_values.clear();
}

const QSharedPointer<Value> Marker::value(const QString &id) const
{
    assert(! id.isEmpty());
    if(!m_values.contains(id))
    {
        qDebug() << " marker does not contain " << id;
    }
    assert(m_values.contains(id));

    return m_values[id];
}

const Value* Marker::valueNakedPtr(const QString &id) const
{
    return value(id).data();
}

const QMap<QString, QSharedPointer<Value> > Marker::values() const
{
    return m_values;
}

void Marker::setValue(const QString& name, Value value)
{
    QMutex mutex;
    mutex.lock();
    m_values[name] = QSharedPointer<Value>(new Value(value));
    mutex.unlock();
}


bool Marker::evaluate(const QString &id, double time)
{
    return m_values[id]->evaluateAtTime(time);
}

bool Marker::evaluateAllVariables()
{
    foreach (QString key, m_values.keys())
        if (!evaluate(key, 0.0))
            return false;

    return true;
}

QString Marker::fieldId()
{
    return m_fieldInfo->fieldId();
}

Boundary::Boundary(const FieldInfo *fieldInfo, QString name, QString type,
                   QMap<QString, Value> values) : Marker(fieldInfo, name)
{
    // type
    setType(type);

    // set values
    foreach(QString id, values.keys())
        setValue(id, values[id]);

    if (!isNone() && !m_type.isEmpty())
    {
        foreach(Module::BoundaryType boundaryType, fieldInfo->boundaryTypes())
        {
            foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
            {
                if(!this->values().contains(variable.id()))
                {
                    // default for GUI
                    Module::DialogRow row = fieldInfo->boundaryUI().dialogRow(variable.id());
                    setValue(variable.id(), Value(QString::number(row.defaultValue())));
                }
            }
        }
    }
}

Material::Material(const FieldInfo *fieldInfo, QString name,
                   QMap<QString, Value> values) : Marker(fieldInfo, name)
{
    // set values
    foreach(QString id, values.keys())
        setValue(id, values[id]);

    // set values
    if (name != "none")
    {
        QList<Module::MaterialTypeVariable> materialTypeVariables = fieldInfo->materialTypeVariables();
        foreach (Module::MaterialTypeVariable variable, materialTypeVariables)
        {
            if(!this->values().contains(variable.id()))
            {
                // default for GUI
                Module::DialogRow row = fieldInfo->materialUI().dialogRow(variable.id());
                setValue(variable.id(), Value(QString::number(row.defaultValue())));
            }
        }

        foreach (QString id, m_fieldInfo->allMaterialQuantities())
        {
            if(!this->values().contains(id))
            {
                setValue(id, Value(QString::number(0)));
            }
        }
    }
}
