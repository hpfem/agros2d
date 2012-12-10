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

#include "scenemarker.h"

#include "util/global.h"

#include "hermes2d/marker.h"

#include "scene.h"
#include "scenemarkerdialog.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

template <typename MarkerType>
void MarkerContainer<MarkerType>::add(MarkerType *marker)
{
    data.append(marker);
}

template <typename MarkerType>
void MarkerContainer<MarkerType>::remove(MarkerType *marker)
{
    data.removeOne(marker);
    delete marker;
}

template <typename MarkerType>
void MarkerContainer<MarkerType>::removeFieldMarkers(FieldInfo* fieldInfo)
{
    foreach(MarkerType* item, data)
    {
        if (item->fieldInfo() == fieldInfo)
            remove(item);
    }
}


template <typename MarkerType>
MarkerType* MarkerContainer<MarkerType>::getNone(FieldInfo* field)
{
    if(!noneMarkers.contains(field)){
        noneMarkers[field] = new MarkerType(field, "none");
        noneMarkers[field]->setNone();
    }

    return noneMarkers[field];
}

template <typename MarkerType>
MarkerType *MarkerContainer<MarkerType>::at(int i)
{
    return data.at(i);
}

template <typename MarkerType>
MarkerType* MarkerContainer<MarkerType>::get(const QString &name)
{
    foreach (MarkerType *item, data)
        if (item->name() == name)
            return item;

    return NULL;
}

template <typename MarkerType>
MarkerContainer<MarkerType> MarkerContainer<MarkerType>::filter(const QString &fieldName)
{
    MarkerContainer<MarkerType> items;
    foreach (MarkerType *item, data)
        if (item->fieldInfo() && item->fieldInfo()->fieldId() == fieldName)
            items.add(item);

    return items;
}

template <typename MarkerType>
MarkerContainer<MarkerType> MarkerContainer<MarkerType>::filter(FieldInfo *fieldInfo)
{
    return filter(fieldInfo->fieldId());
}

template <typename MarkerType>
void MarkerContainer<MarkerType>::clear()
{
    foreach (MarkerType* item, data)
        delete item;

    data.clear();
}

template <typename MarkerType>
MarkerType* MarkerContainer<MarkerType>::getSingleOrNull()
{
    if(this->data.size() == 1)
        return data.at(0);
    else
        return NULL;
}

template <typename MarkerType>
void MarkerContainer<MarkerType>::doFieldsChanged()
{
    foreach(FieldInfo* fieldInfo, noneMarkers.keys())
    {
        if (!Agros2D::problem()->fieldInfos().values().contains(fieldInfo)){
            delete noneMarkers[fieldInfo];
            noneMarkers.remove(fieldInfo);
            removeFieldMarkers(fieldInfo);
        }
    }
}

template <typename MarkerType>
bool MarkerContainer<MarkerType>::evaluateAllVariables()
{
    foreach (MarkerType* marker, data)
    {
        if (!marker->evaluateAllVariables())
            return false;
    }

    return true;
}

//template <typename MarkerType>
//void UniqueMarkerContainer<MarkerType>::add(MarkerType *marker)
//{
//    // add check corresponding to the same field ???
//    if (MarkerType *existingMarker = get(QString::fromStdString(marker->getName())))
//        this->data.replace(this->data.indexOf(existingMarker), marker);
//    else
//        this->data.append(marker);
//}


template <typename MarkerType>
QMap<FieldInfo*, MarkerType*> MarkerContainer<MarkerType>::noneMarkers;


template class MarkerContainer<SceneBoundary>;
template class MarkerContainer<SceneMaterial>;
