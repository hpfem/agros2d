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

#ifndef SCENENODE_H
#define SCENENODE_H

#include "util.h"
#include "scenebasic.h"

class SceneNodeCommandRemove;
class QDomElement;

class SceneNode : public SceneBasic
{
public:
    SceneNode(const Point &m_point);

    inline Point point() const { return m_point; }
    void setPoint(const Point &point);

    // geometry editor
    bool isConnected() const { return connectedEdges().length() > 0; }
    bool isEndNode() const {return connectedEdges().length() == 1; }
    QList<SceneEdge *> connectedEdges() const;
    bool isLyingOnEdges() const { return lyingEdges().length() > 0; }
    QList<SceneEdge *> lyingEdges() const;
    bool isOutsideArea() const;
    bool isError();
    double distance(const Point &m_point) const;

    int showDialog(QWidget *parent, bool isNew = false);

    SceneNodeCommandRemove* getRemoveCommand();

    static SceneNode *findClosestNode(const Point &point);

private:
    Point m_point;
};

Q_DECLARE_METATYPE(SceneNode *)

class SceneNodeContainer : public SceneBasicContainer<SceneNode>
{
public:
    /// if container contains object with the same coordinates as node, returns it. Otherwise returns NULL
    SceneNode* get(SceneNode* node) const;

    /// returns node with given coordinates or NULL
    SceneNode* get(const Point& point) const;

    SceneNode* findClosest(const Point& point) const;

    virtual bool remove(SceneNode *item);

    /// returns bounding box, assumes container not empty
    RectPoint boundingBox() const;

    //TODO should be in SceneBasicContainer, but I would have to cast the result....
    SceneNodeContainer selected();
    SceneNodeContainer highlighted();
};


// *************************************************************************************************************************************

class DSceneNode : public SceneBasicDialog
{
    Q_OBJECT

public:
    DSceneNode(SceneNode *node, QWidget *parent, bool m_isNew = false);
    ~DSceneNode();

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;
    QLabel *lblDistance;
    QLabel *lblAngle;

private slots:
    void doEditingFinished();
};


// undo framework *******************************************************************************************************************

class SceneNodeCommandAdd : public QUndoCommand
{
public:
    SceneNodeCommandAdd(const Point &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
};

class SceneNodeCommandRemove : public QUndoCommand
{
public:
    SceneNodeCommandRemove(const Point &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
};

class SceneNodeCommandEdit : public QUndoCommand
{
public:
    SceneNodeCommandEdit(const Point &point, const Point &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    Point m_pointNew;
};

class SceneNodeCommandMoveMulti : public QUndoCommand
{
public:
    SceneNodeCommandMoveMulti(QList<Point> points, QList<Point> pointsNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    QList<Point> m_points;
    QList<Point> m_pointsNew;
};



#endif // SCENENODE_H
