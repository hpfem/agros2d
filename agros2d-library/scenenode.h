// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SCENENODE_H
#define SCENENODE_H

#include "util.h"
#include "value.h"
#include "scenebasic.h"

class SceneNodeCommandRemove;
class QDomElement;

class SceneNode : public SceneBasic
{
public:
    SceneNode(const Point &point);
    SceneNode(const PointValue &pointValue);

    inline Point point() const { return m_point.point(); }
    inline PointValue pointValue() const { return m_point; }
    void setPointValue(const PointValue &point);

    // geometry editor
    bool isConnected() const;
    bool isEndNode() const;
    QList<SceneEdge *> connectedEdges() const;
    int numberOfConnectedEdges() const;
    bool hasLyingEdges() const;
    QList<SceneEdge *> lyingEdges() const;
    bool isOutsideArea() const;
    bool isError();
    double distance(const Point &point) const;

    int showDialog(QWidget *parent, bool isNew = false);

    SceneNodeCommandRemove* getRemoveCommand();

    static SceneNode *findClosestNode(const Point &point);

private:
    PointValue m_point;
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

class SceneNodeDialog : public SceneBasicDialog
{
    Q_OBJECT

public:
    SceneNodeDialog(SceneNode *node, QWidget *parent, bool m_isNew = false);
    ~SceneNodeDialog();

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
    SceneNodeCommandAdd(const PointValue &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
};

class SceneNodeCommandRemove : public QUndoCommand
{
public:
    SceneNodeCommandRemove(const PointValue &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
};

class SceneNodeCommandEdit : public QUndoCommand
{
public:
    SceneNodeCommandEdit(const PointValue &point, const PointValue &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
    PointValue m_pointNew;
};

class SceneNodeCommandMoveMulti : public QUndoCommand
{
public:
    SceneNodeCommandMoveMulti(QList<PointValue> points, QList<PointValue> pointsNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    static void moveAll(QList<PointValue> moveFrom, QList<PointValue> moveTo);

    QList<PointValue> m_points;
    QList<PointValue> m_pointsNew;
};

class SceneNodeCommandAddMulti : public QUndoCommand
{
public:
    SceneNodeCommandAddMulti(QList<PointValue> points,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    QList<PointValue> m_points;
};

class SceneNodeCommandRemoveMulti : public QUndoCommand
{
public:
    SceneNodeCommandRemoveMulti(QList<PointValue> points,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // nodes
    QList<PointValue> m_nodePoints;
    // edges
    QList<Point> m_edgePointStart;
    QList<Point> m_edgePointEnd;
    QList<QMap<QString, QString> > m_edgeMarkers;
    QList<Value> m_edgeAngle;
};


#endif // SCENENODE_H
