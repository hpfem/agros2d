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

#ifndef SCENEEDGE_H
#define SCENEEDGE_H

#include "util.h"
#include "scenebasic.h"
#include "scenemarkerdialog.h"

class SceneEdgeCommandAdd;
class SceneEdgeCommandRemove;

class AGROS_LIBRARY_API SceneEdge : public MarkedSceneBasic<SceneBoundary>
{
public:
    SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, double angle, int segments = 3, bool isCurvilinear = true);

    inline SceneNode *nodeStart() const { return m_nodeStart; }
    inline void setNodeStart(SceneNode *nodeStart) { m_nodeStart = nodeStart; computeCenterAndRadius(); }
    inline SceneNode *nodeEnd() const { return m_nodeEnd; }
    inline void setNodeEnd(SceneNode *nodeEnd) { m_nodeEnd = nodeEnd; computeCenterAndRadius(); }
    inline double angle() const { return m_angle; }
    inline void setAngle(double angle) { m_angle = angle; computeCenterAndRadius(); }
    void swapDirection();

    QList<SceneNode *> lyingNodes() const;

    bool isLyingOnNode(const SceneNode *node) const;
    bool hasLyingNode() const;
    bool isOutsideArea() const;
    bool isError() const;
    bool isCrossed() const; // very slow, use carefully

    inline Point center() const { return m_centerCache; }
    inline double radius() const { return m_radiusCache; }
    inline Point vector() const { return m_vectorCache; }
    double distance(const Point &point) const;    
    double length() const;
    bool isStraight() const { return (fabs(m_angle) < EPS_ZERO); }

    // needed by mesh generator
    inline int segments() const { return m_segments; }
    void setSegments(int segments);
    inline bool isCurvilinear() const { return m_isCurvilinear; }
    void setCurvilinear(bool isCurvilinear);

    SceneEdgeCommandAdd* getAddCommand();
    SceneEdgeCommandRemove* getRemoveCommand();

    int showDialog(QWidget *parent, bool isNew = false);

    static SceneEdge *findClosestEdge(const Point &point);

    void addMarkersFromStrings(QMap<QString, QString> markers);

private:
    SceneNode *m_nodeStart;
    SceneNode *m_nodeEnd;
    double m_angle;
    int m_segments;
    bool m_isCurvilinear;

    // cache
    Point m_centerCache;
    double m_radiusCache;
    Point m_vectorCache;
    void computeCenterAndRadius();

    friend class SceneNode;
};

Q_DECLARE_METATYPE(SceneEdge *)

// *************************************************************************************************************************************

class SceneEdgeContainer : public MarkedSceneBasicContainer<SceneBoundary, SceneEdge>
{
public:
    void removeConnectedToNode(SceneNode* node);

    /// if container contains the same edge, returns it. Otherwise returns NULL
    SceneEdge* get(SceneEdge* edge) const;

    /// returns corresponding edge or NULL
    SceneEdge* get(const Point &pointStart, const Point &pointEnd, double angle, int segments, bool isCurvilinear) const;

    /// returns corresponding edge or NULL
    SceneEdge* get(const Point &pointStart, const Point &pointEnd) const;

    /// returns bounding box, assumes container not empty
    RectPoint boundingBox() const;
    static RectPoint boundingBox(QList<SceneEdge *> edges);
};

// *************************************************************************************************************************************

class SceneEdgeMarker : public QGroupBox
{
    Q_OBJECT

public:
    SceneEdgeMarker(SceneEdge *edge, FieldInfo *fieldInfo, QWidget *parent);

    void load();
    bool save();
    void fillComboBox();

private:
    FieldInfo *m_fieldInfo;
    SceneEdge *m_edge;

    QComboBox *cmbBoundary;
    QPushButton *btnBoundary;

    QCheckBox *chkRefineTowardsEdge;
    QSpinBox *txtRefineTowardsEdge;

private slots:
    void doBoundaryChanged(int index);
    void doBoundaryClicked();
    void doRefineTowardsEdge(int state);    
};

class SceneEdgeDialog : public SceneBasicDialog
{
    Q_OBJECT

public:
    SceneEdgeDialog(SceneEdge *edge, QWidget *parent, bool m_isNew);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    QLabel *lblEquation;
    ValueLineEdit *txtAngle;
    QSpinBox *txtSegments;
    QCheckBox *chkIsCurvilinear;
    QLabel *lblLength;

    QComboBox *cmbNodeStart;
    QComboBox *cmbNodeEnd;

    QList<SceneEdgeMarker *> m_edgeMarkers;

    void fillComboBox();

private slots:
    void nodeChanged();
    void swap();
    void angleChanged();
};

class SceneEdgeSelectDialog : public QDialog
{
    Q_OBJECT

public:
    SceneEdgeSelectDialog(MarkedSceneBasicContainer<SceneBoundary, SceneEdge> edges, QWidget *parent);

protected:
    void load();
    bool save();

private:
    MarkedSceneBasicContainer<SceneBoundary, SceneEdge> m_edges;
    QMap<FieldInfo*, QComboBox *> cmbBoundaries;

    void fillComboBox();

private slots:
    void doAccept();
    void doReject();
};

// undo framework *******************************************************************************************************************

class SceneEdgeCommandAdd : public QUndoCommand
{
public:
    SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QMap<QString, QString> &markers,
                        double angle, int segments, bool isCurvilinear, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QMap<QString, QString> m_markers;
    double m_angle;
    int m_segments;
    bool m_isCurvilinear;
};

class SceneEdgeCommandAddOrRemoveMulti : public QUndoCommand
{
public:
    SceneEdgeCommandAddOrRemoveMulti(QList<Point> pointStarts, QList<Point> pointEnds,
                        QList<double> angles, QList<int> segments, QList<bool> isCurvilinear, QList<QMap<QString, QString> > markers, QUndoCommand *parent = 0);
    void add();
    void remove();

private:
    QList<Point> m_pointStarts;
    QList<Point> m_pointEnds;
    QList<double> m_angles;
    QList<int> m_segments;
    QList<bool> m_isCurvilinear;
    QList<QMap<QString, QString> > m_markers;
};

class SceneEdgeCommandRemoveMulti : public SceneEdgeCommandAddOrRemoveMulti
{
public:
    SceneEdgeCommandRemoveMulti(QList<Point> pointStarts, QList<Point> pointEnds,
                                QList<double> angles, QList<int> segments, QList<bool> isCurvilinear, QList<QMap<QString, QString> > markers, QUndoCommand *parent = 0)
        : SceneEdgeCommandAddOrRemoveMulti(pointStarts, pointEnds, angles, segments, isCurvilinear, markers, parent) {}

    void undo() { add(); }
    void redo() { remove(); }
};

class SceneEdgeCommandAddMulti : public SceneEdgeCommandAddOrRemoveMulti
{
public:
    SceneEdgeCommandAddMulti(QList<Point> pointStarts, QList<Point> pointEnds,
                                QList<double> angles, QList<int> segments, QList<bool> isCurvilinear, QList<QMap<QString, QString> > markers, QUndoCommand *parent = 0)
        : SceneEdgeCommandAddOrRemoveMulti(pointStarts, pointEnds, angles, segments, isCurvilinear, markers, parent) {}

    void undo() { remove(); }
    void redo() { add(); }
};

class SceneEdgeCommandRemove : public QUndoCommand
{
public:
    SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QMap<QString, QString> &markers,
                           double angle, int segments, bool isCurvilinear, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QMap<QString, QString> m_markers;
    double m_angle;
    int m_segments;
    bool m_isCurvilinear;
};

class SceneEdgeCommandEdit : public QUndoCommand
{
public:
    SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                         double angle, double angleNew, int segments, int segmentsNew, bool isCurvilinear, bool isCurvilinearNew, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    Point m_pointStartNew;
    Point m_pointEndNew;
    double m_angle;
    double m_angleNew;
    int m_segments;
    int m_segmentsNew;
    bool m_isCurvilinear;
    bool m_isCurvilinearNew;
};


#endif // SCENEEDGE_H
