#ifndef SCENEBASIC_H
#define SCENEBASIC_H

#include "util.h"
#include "gui.h"
#include "scenemarker.h"

class SLineEdit;

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

    double distance(const Point &point);

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

    SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, SceneEdgeMarker *marker, double angle);

    Point center();
    double radius();
    double distance(const Point &point);
    inline int direction() { return (((nodeStart->point.x-nodeEnd->point.x)*nodeStart->point.y - (nodeStart->point.y-nodeEnd->point.y)*nodeStart->point.x) > 0) ? 1 : -1; }

    int showDialog(QWidget *parent, bool isNew = false);
};

// *************************************************************************************************************************************

class SceneLabel : public SceneBasic 
{
public:
    SceneLabelMarker *marker;
    Point point;
    double area;

    SceneLabel(const Point &point, SceneLabelMarker *marker, double area);

    double distance(const Point &point);

    int showDialog(QWidget *parent, bool isNew = false);
};

// *************************************************************************************************************************************

class DSceneBasic: public QDialog
{
    Q_OBJECT

public:
    DSceneBasic(QWidget *parent, bool isNew = false);
    ~DSceneBasic();

private slots:
    void doAccept();
    void doReject();

protected:
    bool isNew;
    SceneBasic *m_object;

    virtual QLayout *createContent() = 0;
    void createControls();

    virtual bool load() = 0;
    virtual bool save() = 0;

private:    
    QVBoxLayout *layout;
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
    SLineEdit *txtPointX;
    SLineEdit *txtPointY;
};

// *************************************************************************************************************************************

class DSceneEdge : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneEdge(SceneEdge *edge, QWidget *parent, bool isNew);
    ~DSceneEdge();

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    QComboBox *cmbNodeStart;
    QComboBox *cmbNodeEnd;
    QComboBox *cmbMarker;
    SLineEdit *txtAngle;

    void fillComboBox();
};

// *************************************************************************************************************************************

class DSceneLabel : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneLabel(SceneLabel *label, QWidget *parent, bool isNew = false);
    ~DSceneLabel();

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    SLineEdit *txtPointX;
    SLineEdit *txtPointY;
    QComboBox *cmbMarker;
    SLineEdit *txtArea;

    void fillComboBox();
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
    SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
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
    SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName, double angle, QUndoCommand *parent = 0);
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
    SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName, double angle, QUndoCommand *parent = 0);
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
    SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew, double angle, double angleNew, QUndoCommand *parent = 0);
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

#endif // SCENEBASIC_H
