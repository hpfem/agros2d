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

#ifndef SCENEBASIC_H
#define SCENEBASIC_H

#include "util.h"

class SLineEditDouble;
class ValueLineEdit;

struct Point;

class SceneBasic;
template <typename MarkerType> class MarkedSceneBasic;
template <typename MarkerType> class UniqueMarkerContainer;
class SceneNode;
class SceneEdge;
class SceneLabel;
class Marker;

class SceneBoundary;
class SceneMaterial;

Q_DECLARE_METATYPE(SceneBasic *)
Q_DECLARE_METATYPE(SceneNode *)
Q_DECLARE_METATYPE(SceneEdge *)
Q_DECLARE_METATYPE(SceneLabel *)

class SceneBasic 
{

public:
    bool isSelected;
    bool isHighlighted;

    SceneBasic();
    void setSelected(bool value = true) { isSelected = value; }
    void setHighlighted(bool value = true) { isHighlighted = value; }

    virtual int showDialog(QWidget *parent, bool isNew = false) = 0;

    QVariant variant();
};

template <typename BasicType>
class SceneBasicContainer
{
public:
    /// items() should be removed step by step from the code.
    /// more methods operating with list data should be defined here
    QList<BasicType*> items() { return data; }

    bool add(BasicType *item);
    bool remove(BasicType *item);
    BasicType *at(int i);
    inline int length() { return data.length(); }
    inline int isEmpty() { return data.isEmpty(); }
    void clear();

    /// selects or unselects all items
    void setSelected(bool value = true);

    /// highlights or unhighlights all items
    void setHighlighted(bool value = true);

    void deleteWithUndo(QString message);

    // saves all members into the node
    //void fillQDomNode(const QDomDocument & document) const;


protected:
    QList<BasicType*> data;

    QString containerName;
};

// *************************************************************************************************************************************



//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO zamyslet se nad porovnavanim markeru
//TODO opravdu chci porovnavat ukazatele?
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


template <typename MarkerType>
class MarkedSceneBasic : public SceneBasic
{
public:
    MarkedSceneBasic() { markers = new UniqueMarkerContainer<MarkerType>; }
    ~MarkedSceneBasic() { delete markers; }

    /// gets marker that corresponds to the given field
    MarkerType* getMarker(QString field);

    /// adds marker. If there exists marker with the same field, is overwritten
    void addMarker(MarkerType* marker);

    /// true if has given marker
    bool hasMarker(MarkerType* marker) {return markers->contains(marker); }

public:
    UniqueMarkerContainer<MarkerType> *markers;
};


template <typename MarkerType, typename MarkedSceneBasicType>
class MarkedSceneBasicContainer : public SceneBasicContainer<MarkedSceneBasicType>
{
public:
    UniqueMarkerContainer<MarkerType> allMarkers();
    void removeMarkerFromAll(MarkerType* marker);
    void addMarkerToAll(MarkerType* marker);

    /// Filters for elements that has given marker
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> haveMarker(MarkerType *marker);

    //TODO unfortunately, those had to be moved here from SceneBasicContainer
    //TODO if they returned SceneBasicContainer, One would have to cast to use methods of this class to return value...
    //TODO it might be possible to do it differently...
    /// Filters for selected
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> selected();

    /// Filters for highlighted
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> highlited();
};


// *************************************************************************************************************************************

class DSceneBasic: public QDialog
{
    Q_OBJECT

public:
    DSceneBasic(QWidget *parent, bool isNew = false);
    ~DSceneBasic();

protected:
    bool isNew;
    SceneBasic *m_object;
    QDialogButtonBox *buttonBox;

    virtual QLayout *createContent() = 0;
    void createControls();

    virtual bool load() = 0;
    virtual bool save() = 0;

protected slots:
    void evaluated(bool isError);

private:    
    QVBoxLayout *layout;

private slots:
    void doAccept();
    void doReject();
};

// *************************************************************************************************************************************


#endif // SCENEBASIC_H
