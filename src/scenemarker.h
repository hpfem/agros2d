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

#ifndef SCENEMAR_H
#define SCENEMAR_H

#include "util.h"

class SceneBoundary;
class SceneMaterial;
class Marker;
class FieldInfo;

template <typename MarkerType>
class MarkerContainer
{
public:
    MarkerContainer() : data(QList<MarkerType* >()) {}
    ~MarkerContainer();

    /// items() should be removed step by step from the code.
    /// more methods operating with list data should be defined here
    QList<MarkerType*> items() { return data; }

    /// add marker.
    virtual void add(MarkerType *marker);

    /// remove marker
    void remove(MarkerType *marker);

    /// remove all markers of this field
    void removeFieldMarkers(FieldInfo* fieldInfo);

    /// get marker at position i
    MarkerType *at(int i);

    /// get marker by name
    MarkerType *get(const QString &name);

    static MarkerType *getNone(FieldInfo* field);

    /// filter field
    MarkerContainer<MarkerType> filter(const QString &fieldName);
    MarkerContainer<MarkerType> filter(FieldInfo* fieldInfo);

    /// length of the array
    inline int length() { return data.length(); }

    /// if contains exactly one element, return it. Otherwise return NULL
    MarkerType* getSingleOrNull();

    /// checks if array is empty
    inline int isEmpty() { return data.isEmpty(); }

    /// bool if contains given marker (pointer)
    bool contains(MarkerType* marker) {return data.contains(marker);}

    /// clear and delete data
    void clear();

    /// create or delete markers for new or removed field
    void doFieldsChanged();

    bool evaluateAllVariables();

protected:
    QList<MarkerType* > data;

    static QMap<FieldInfo*, MarkerType*> noneMarkers;
};

//template <typename MarkerType>
//class UniqueMarkerContainer : public MarkerContainer<MarkerType>
//{
//public:
//    /// If container allready contains marker corresponding to the same field, it is repladded
//    virtual void add(MarkerType *marker);
//};

class SceneBoundaryContainer : public MarkerContainer<SceneBoundary>
{

};

class SceneMaterialContainer : public MarkerContainer<SceneMaterial>
{

};

#endif // SCENEMARKER_H
