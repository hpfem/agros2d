#ifndef SCENEBASIC_H
#define SCENEBASIC_H

#include <QtGui/QDoubleValidator>
#include <QtGui/QHBoxLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QMetaType>
#include <QVariant>
#include <QString>

#include "util.h"
#include "gui.h"
#include "scene.h"
#include "scenemarker.h"

class SLineEdit;

struct Point;

class Scene;
class SceneBasic;
class SceneNode;
class SceneEdge;
class SceneLabel;

class SceneEdgeMarker;
class SceneLabelMarker;

Q_DECLARE_METATYPE(SceneBasic *);

class SceneBasic 
{

public:
    bool isSelected;
    bool isHighlighted;

    SceneBasic();
    virtual ~SceneBasic();

    virtual int showDialog(QWidget *parent) = 0;

    QVariant variant();
};

// *************************************************************************************************************************************

class SceneNode : public SceneBasic 
{
public:
    Point point;

    SceneNode(const Point &point);
    ~SceneNode();

    double distance(const Point &point);

    int showDialog(QWidget *parent);
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
    ~SceneEdge();

    Point center();
    double radius();
    double distance(const Point &point);
    inline int direction() { return (((nodeStart->point.x-nodeEnd->point.x)*nodeStart->point.y - (nodeStart->point.y-nodeEnd->point.y)*nodeStart->point.x) > 0) ? 1 : -1; }

    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabel : public SceneBasic 
{
public:
    SceneLabelMarker *marker;
    Point point;
    double area;

    SceneLabel(const Point &point, SceneLabelMarker *marker, double area);
    ~SceneLabel();

    double distance(const Point &point);

    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class DSceneBasic: public QDialog
{
    Q_OBJECT

public:
    DSceneBasic(QWidget *parent);
    ~DSceneBasic();

private slots:
    void doAccept();
    void doReject();

protected:
    SceneBasic *m_object;

    virtual QLayout *createContent() = 0;
    void createControls();

    virtual void load() = 0;
    virtual void save() = 0;

private:
    QVBoxLayout *layout;
};

// *************************************************************************************************************************************

class DSceneNode : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneNode(SceneNode *node, QWidget *parent);
    ~DSceneNode();

protected:
    QLayout *createContent();

    void load();
    void save();

private:
    SLineEdit *txtPointX;
    SLineEdit *txtPointY;
};

// *************************************************************************************************************************************

class DSceneEdge : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneEdge(SceneEdge *edge, QWidget *parent);
    ~DSceneEdge();

protected:
    QLayout *createContent();

    void load();
    void save();

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
    DSceneLabel(SceneLabel *label, QWidget *parent);
    ~DSceneLabel();

protected:
    QLayout *createContent();

    void load();
    void save();

private:
    SLineEdit *txtPointX;
    SLineEdit *txtPointY;
    QComboBox *cmbMarker;
    SLineEdit *txtArea;

    void fillComboBox();
};

#endif // SCENEBASIC_H
