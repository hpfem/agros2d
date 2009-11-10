#ifndef SCENEMARKER_H
#define SCENEMARKER_H

#include "util.h"
#include "gui.h"

class SLineEditDouble;
class SLineEditValue;

class SceneEdgeMarker;
class SceneLabelMarker;

Q_DECLARE_METATYPE(SceneEdgeMarker *);
Q_DECLARE_METATYPE(SceneLabelMarker *);

class SceneEdgeMarker
{
public:
    QString name;
    PhysicFieldBC type;

    SceneEdgeMarker(const QString &name, PhysicFieldBC type);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneEdgeMarkerNone : public SceneEdgeMarker
{
public:
    SceneEdgeMarkerNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    int showDialog(QWidget *parent) { return 0; }
};

class EdgeMarkerDialog : public QDialog
{
    Q_OBJECT
public:
    EdgeMarkerDialog(QWidget *parent = 0);
    ~EdgeMarkerDialog();

    inline SceneEdgeMarker *marker() { return (cmbMarker->currentIndex() >= 0) ? cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneEdgeMarker *>() : NULL; }

private:
    QComboBox *cmbMarker;

public slots:
    void doAccept();
};

// *************************************************************************************************************************************

class SceneLabelMarker
{
public:
    QString name;

    SceneLabelMarker(const QString &name);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneLabelMarkerNone : public SceneLabelMarker
{
public:
    SceneLabelMarkerNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    int showDialog(QWidget *parent) { return 0; }
};

class LabelMarkerDialog : public QDialog
{
    Q_OBJECT
public:
    LabelMarkerDialog(QWidget *parent = 0);
    ~LabelMarkerDialog();

    inline SceneLabelMarker *marker() { return (cmbMarker->currentIndex() >= 0) ? cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneLabelMarker *>() : NULL; }

private:
    QComboBox *cmbMarker;

public slots:
    void doAccept();
};

// *************************************************************************************************************************************

class DSceneEdgeMarker: public QDialog
{
    Q_OBJECT

public:
    DSceneEdgeMarker(QWidget *parent);
    ~DSceneEdgeMarker();

protected:
    QLineEdit *txtName;
    SceneEdgeMarker *m_edgeMarker;

    virtual QLayout *createContent() = 0;
    void createDialog();

    virtual void load();
    virtual bool save();
    void setSize();

private slots:
    void doAccept();
    void doReject();

private:
    QVBoxLayout *layout;
};

class DSceneLabelMarker: public QDialog
{
    Q_OBJECT

public:
    DSceneLabelMarker(QWidget *parent);
    ~DSceneLabelMarker();

protected:
    QLineEdit *txtName;
    SceneLabelMarker *m_labelMarker;

    virtual QLayout *createContent() = 0;
    void createDialog();
    void setSize();

    virtual void load();
    virtual bool save();

private slots:
    void doAccept();
    void doReject();

private:
    QVBoxLayout *layout;
};

#endif // SCENEMARKER_H
