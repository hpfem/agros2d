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
class SLineEditValue;

struct Point;

class SceneBasic;
class SceneNode;
class SceneEdge;
class SceneLabel;

class SceneEdgeMarker;
class SceneLabelMarker;

Q_DECLARE_METATYPE(SceneBasic *);
Q_DECLARE_METATYPE(SceneNode *);
Q_DECLARE_METATYPE(SceneEdge *);
Q_DECLARE_METATYPE(SceneLabel *);

class SceneBasic 
{

public:
    bool isSelected;
    bool isHighlighted;

    SceneBasic();

    virtual int showDialog(QWidget *parent, bool isNew = false) = 0;

    QVariant variant();
};

// *************************************************************************************************************************************

class SceneNode : public SceneBasic 
{
public:
    Point point;

    SceneNode(const Point &point);

    double distance(const Point &point) const;

    int showDialog(QWidget *parent, bool isNew = false);
};

// *************************************************************************************************************************************

class SceneEdge : public SceneBasic
{
public:
    SceneEdgeMarker *marker;
    SceneNode *nodeStart;
    SceneNode *nodeEnd;
    double angle;
    int refineTowardsEdge;

    SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, SceneEdgeMarker *marker, double angle, int refineTowardsEdge);

    Point center() const;
    double radius() const;
    double distance(const Point &point) const;
    int segments() const; // needed by mesh generator
    double length() const;

    int showDialog(QWidget *parent, bool isNew = false);
};

// *************************************************************************************************************************************

class SceneLabel : public SceneBasic 
{
public:
    SceneLabelMarker *marker;
    Point point;
    double area;
    int polynomialOrder;

    SceneLabel(const Point &point, SceneLabelMarker *marker, double area, int polynomialOrder);

    double distance(const Point &point) const;

    int showDialog(QWidget *parent, bool isNew = false);
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

class DSceneNode : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneNode(SceneNode *node, QWidget *parent, bool isNew = false);
    ~DSceneNode();

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    SLineEditValue *txtPointX;
    SLineEditValue *txtPointY;
    QLabel *lblDistance;
    QLabel *lblAngle;

private slots:
    void doEditingFinished();
};

// *************************************************************************************************************************************

class DSceneEdge : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneEdge(SceneEdge *edge, QWidget *parent, bool isNew);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    QComboBox *cmbNodeStart;
    QComboBox *cmbNodeEnd;
    QComboBox *cmbMarker;
    QPushButton *btnMarker;
    SLineEditValue *txtAngle;
    QLabel *lblLength;
    QCheckBox *chkRefineTowardsEdge;
    QSpinBox *txtRefineTowardsEdge;

    void fillComboBox();

private slots:
    void doMarkerChanged(int index);
    void doMarkerClicked();
    void doNodeChanged();
    void doRefineTowardsEdge(int state);
};

// *************************************************************************************************************************************

class DSceneLabel : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneLabel(SceneLabel *label, QWidget *parent, bool isNew = false);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    SLineEditValue *txtPointX;
    SLineEditValue *txtPointY;
    QComboBox *cmbMarker;
    QPushButton *btnMarker;
    SLineEditValue *txtArea;
    QSpinBox *txtPolynomialOrder;
    QCheckBox *chkArea;
    QCheckBox *chkPolynomialOrder;

    void fillComboBox();

private slots:
    void doMarkerChanged(int index);
    void doMarkerClicked();
    void doArea(int);
    void doPolynomialOrder(int);
};

// undo framework *******************************************************************************************************************

// Node

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

// Label

class SceneLabelCommandAdd : public QUndoCommand
{
public:
    SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
    int m_polynomialOrder;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
    int m_polynomialOrder;
};

class SceneLabelCommandEdit : public QUndoCommand
{
public:
    SceneLabelCommandEdit(const Point &point, const Point &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    Point m_pointNew;
};

// Edge

class SceneEdgeCommandAdd : public QUndoCommand
{
public:
    SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                        double angle, int refineTowardsEdge, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QString m_markerName;
    double m_angle;
    int m_refineTowardsEdge;
};

class SceneEdgeCommandRemove : public QUndoCommand
{
public:
    SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                           double angle, int refineTowardsEdge, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QString m_markerName;
    double m_angle;
    int m_refineTowardsEdge;
};

class SceneEdgeCommandEdit : public QUndoCommand
{
public:
    SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                         double angle, double angleNew, int refineTowardsEdge, int refineTowardsEdgeNew, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    Point m_pointStartNew;
    Point m_pointEndNew;
    double m_angle;
    double m_angleNew;
    int m_refineTowardsEdge;
    int m_refineTowardsEdgeNew;
};

#endif // SCENEBASIC_H
