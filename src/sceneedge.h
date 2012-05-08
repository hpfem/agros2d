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

class SceneEdgeCommandRemove;

class SceneEdge : public MarkedSceneBasic<SceneBoundary>
{
public:
    SceneNode *nodeStart;
    SceneNode *nodeEnd;
    double angle;

    SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, double angle);

    bool isCrossed() const { return crossedEdges.length() > 0; }
    QList<SceneEdge *> crossedEdges;
    bool isLyingNode() const { return lyingNodes.length() > 0; }
    QList<SceneNode *> lyingNodes;
    bool isOutsideArea() const;
    bool isError() const;

    Point center() const;
    double radius() const;
    double distance(const Point &point) const;
    int segments() const; // needed by mesh generator
    double length() const;
    bool isStraight() const { return (fabs(angle) < EPS_ZERO); }

    SceneEdgeCommandRemove* getRemoveCommand();

    int showDialog(QWidget *parent, bool isNew = false);
};

// *************************************************************************************************************************************

class SceneEdgeContainer : public MarkedSceneBasicContainer<SceneBoundary, SceneEdge>
{
public:
    void removeConnectedToNode(SceneNode* node);

    /// if container contains the same edge, returns it. Otherwise returns NULL
    SceneEdge* get(SceneEdge* edge) const;

    /// returns corresponding edge or NULL
    SceneEdge* get(const Point &pointStart, const Point &pointEnd, double angle) const;

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
    SceneEdgeDialog(SceneEdge *edge, QWidget *parent, bool isNew);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    QLabel *lblEquation;
    ValueLineEdit *txtAngle;
    QLabel *lblLength;

    QList<SceneEdgeMarker *> m_edgeMarkers;

    void fillComboBox();

private slots:
    void doAngleChanged();
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
    SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                        double angle, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QString m_markerName;
    double m_angle;
};

class SceneEdgeCommandRemove : public QUndoCommand
{
public:
    SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                           double angle, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QString m_markerName;
    double m_angle;
};

class SceneEdgeCommandEdit : public QUndoCommand
{
public:
    SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                         double angle, double angleNew, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    Point m_pointStartNew;
    Point m_pointEndNew;
    double m_angle;
    double m_angleNew;
};


#endif // SCENEEDGE_H
