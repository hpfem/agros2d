#ifndef SCENEFUNCTION_H
#define SCENEFUNCTION_H

#include "util.h"
#include "gui.h"

class SLineEdit;

struct Point;

class Chart;

class SceneFunction;

Q_DECLARE_METATYPE(SceneFunction *);

class SceneFunction
{
public:
    QString name;
    QString function;

    SceneFunction(const QString &name, const QString &function);
    ~SceneFunction();

    int showDialog(QWidget *parent);

    double evaluate(double number) throw (const QString &);

    QString script();
    QVariant variant();
};

// ************************************************************************************************************************

class DSceneFunction: public QDialog
{
    Q_OBJECT

public:
    SceneFunction *m_sceneFunction;

    DSceneFunction(SceneFunction *sceneFunction, QWidget *parent = 0);
    ~DSceneFunction();

private:
    Chart *chart;

    QLineEdit *txtName;
    QLineEdit *txtFunction;
    SLineEditDouble *txtStart;
    SLineEditDouble *txtEnd;

    void createControls();
    void load();
    bool save();

private slots:
    void doAccept();
    void doReject();

    void doSaveImage();
    void doPlot();
};

#endif // SCENEFUNCTION_H
