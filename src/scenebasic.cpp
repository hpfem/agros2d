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

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "gui.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

SceneBasic::SceneBasic()
{
    logMessage("SceneBasic::SceneBasic()");

    isSelected = false;
    isHighlighted = false;
}

QVariant SceneBasic::variant()
{
    logMessage("SceneBasic::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

// *************************************************************************************************************************************


template <typename MarkerType>
MarkerType* MarkedSceneBasic<MarkerType>::getMarker(FieldInfo* field)
{
    assert(markers.contains(field));
    MarkerType* marker = markers[field];
    assert(marker);

    return marker;
}

template <typename MarkerType>
MarkerType* MarkedSceneBasic<MarkerType>::getMarker(QString fieldId)
{
    getMarker(Util::scene()->fieldInfo(fieldId));
}


template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::addMarker(MarkerType* marker)
{
    markers[marker->getFieldInfo()] = marker;
}

template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::putMarkersToList(MarkerContainer<MarkerType>* list)
{
    foreach(MarkerType* marker, markers)
        if(!list->contains(marker))
            list->add(marker);
}

template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::removeMarker(QString field)
{
    removeMarker(Util::scene()->fieldInfo(field));
}


template class MarkedSceneBasic<SceneBoundary>;
template class MarkedSceneBasic<SceneMaterial>;

// *************************************************************************************************************************************

template <typename BasicType>
bool SceneBasicContainer<BasicType>::add(BasicType *item)
{
    //TODO add check
    data.append(item);

    return true;
}

template <typename BasicType>
bool SceneBasicContainer<BasicType>::remove(BasicType *item)
{
    return data.removeOne(item);
}

template <typename BasicType>
BasicType *SceneBasicContainer<BasicType>::at(int i)
{
    return data.at(i);
}

template <typename BasicType>
void SceneBasicContainer<BasicType>::clear()
{
    foreach (BasicType* item, data)
        delete item;

    data.clear();
}

template <typename BasicType>
void SceneBasicContainer<BasicType>::setSelected(bool value)
{
    foreach (BasicType* item, data)
        item->setSelected(value);
}

template <typename BasicType>
void SceneBasicContainer<BasicType>::setHighlighted(bool value)
{
    foreach (BasicType* item, data)
        item->setHighlighted(value);
}

template <typename BasicType>
void SceneBasicContainer<BasicType>::deleteWithUndo(QString message)
{
    foreach (BasicType *node, data)
    {
        Util::scene()->undoStack()->beginMacro(message);
        Util::scene()->undoStack()->push(node->getRemoveCommand());
        this->remove(node);
        Util::scene()->undoStack()->endMacro();
    }
}

//TODO asi to nema cenu delat tady...
//template <typename BasicType>
//void SceneBasicContainer<BasicType>::fillQDomNode(const QDomDocument & document) const
//{
//    QDomNode nodeList = doc.createELement(containerName);
//    int i = 0;
//    foreach (BasicType *node, data)
//    {
//        nodeList.appendChild(node->getQDomElement(document, i));
//        i++;
//    }
//}

template class SceneBasicContainer<SceneNode>;
template class SceneBasicContainer<SceneEdge>;
template class SceneBasicContainer<SceneLabel>;

template <typename MarkerType, typename MarkedSceneBasicType>
MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::selected()
{
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> list;
    foreach (MarkedSceneBasicType* item, this->data)
    {
        if (item->isSelected)
            list.data.push_back(item);
    }

    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::highlited()
{
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> list;
    foreach (MarkedSceneBasicType* item, this->data)
    {
        if (item->isHighlighted)
            list.data.push_back(item);
    }

    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::haveMarker(MarkerType *marker)
{
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> list;
    foreach (MarkedSceneBasicType* item, this->data)
    {
        if (item->hasMarker(marker))
            list.data.push_back(item);
    }

    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::removeFieldMarkers(FieldInfo *fieldInfo)
{
    foreach(MarkedSceneBasicType* item, this->data)
    {
        item->removeMarker(fieldInfo);
    }
}

template <typename MarkerType, typename MarkedSceneBasicType>
MarkerContainer<MarkerType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::allMarkers()
{
    MarkerContainer<MarkerType> list;
    foreach(MarkedSceneBasicType* item, this->data){
        item->putMarkersToList(&list);
    }
    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::removeMarkerFromAll(MarkerType* marker)
{
    foreach(MarkedSceneBasicType* item, this->data){
        item->removeMarker(marker);
    }
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::addMarkerToAll(MarkerType* marker)
{
    foreach(MarkedSceneBasicType* item, this->data){
        item->addMarker(marker);
    }
}

template class MarkedSceneBasicContainer<SceneBoundary, SceneEdge>;
template class MarkedSceneBasicContainer<SceneMaterial, SceneLabel>;


// *************************************************************************************************************************************

DSceneBasic::DSceneBasic(QWidget *parent, bool isNew) : QDialog(parent)
{
    logMessage("DSceneBasic::DSceneBasic()");

    this->isNew = isNew;
    layout = new QVBoxLayout();
}

DSceneBasic::~DSceneBasic()
{
    logMessage("DSceneBasic::~DSceneBasic()");

    delete layout;
}

void DSceneBasic::createControls()
{
    logMessage("DSceneBasic::createControls()");

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addLayout(createContent());
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void DSceneBasic::doAccept()
{
    logMessage("DSceneBasic::doAccept()");

    if (save())
        accept();
}

void DSceneBasic::doReject()
{
    logMessage("DSceneBasic::doReject()");

    reject();
}

void DSceneBasic::evaluated(bool isError)
{
    logMessage("DSceneBasic::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

