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

#include "hermes2d/marker.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

template <typename MarkerType>
void MarkerContainer<MarkerType>::add(MarkerType *marker)
{
    logMessage("MarkerContainer::add()");

    data.append(marker);
}

template <typename MarkerType>
void MarkerContainer<MarkerType>::remove(MarkerType *marker)
{
    logMessage("MarkerContainer::remove()");

    data.removeOne(marker);
}

template <typename MarkerType>
MarkerType *MarkerContainer<MarkerType>::at(int i)
{
    return data.at(i);
}

template <typename MarkerType>
MarkerType* MarkerContainer<MarkerType>::get(const QString &name)
{
    logMessage("MarkerContainer::get()");

    foreach (MarkerType *item, data)
        if (item->getName() == name.toStdString())
            return item;

    return NULL;
}

template <typename MarkerType>
MarkerContainer<MarkerType> MarkerContainer<MarkerType>::filter(const QString &fieldName)
{
    MarkerContainer<MarkerType> items;
    foreach (MarkerType *item, data)
        if (item->field() && item->field()->fieldId() == fieldName)
            items.add(item);

    return items;
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
void UniqueMarkerContainer<MarkerType>::add(MarkerType *marker)
{
    assert(0); //TODO
}

template class MarkerContainer<SceneBoundary>;
template class MarkerContainer<SceneMaterial>;

template class UniqueMarkerContainer<SceneBoundary>;
template class UniqueMarkerContainer<SceneMaterial>;
