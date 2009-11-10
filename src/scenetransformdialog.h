#ifndef SCENETRANSFORMDIALOG_H
#define SCENETRANSFORMDIALOG_H

#include "util.h"
#include "gui.h"
#include "scene.h"

class Scene;

class SceneTransformDialog : public QDialog
{
    Q_OBJECT
public:
    SceneTransformDialog(QWidget *parent = 0);
    ~SceneTransformDialog();

private slots:
    void doAccept();
    void doReject();

protected:
    void createControls();

private:
    QTabWidget* tabWidget;

    QWidget *widTranslate;
    QWidget *widRotate;
    QWidget *widScale;

    SLineEditDouble *txtTranslateX;
    SLineEditDouble *txtTranslateY;
    SLineEditDouble *txtRotateBasePointX;
    SLineEditDouble *txtRotateBasePointY;
    SLineEditDouble *txtRotateAngle;
    SLineEditDouble *txtScaleBasePointX;
    SLineEditDouble *txtScaleBasePointY;
    SLineEditDouble *txtScaleFactor;

    QCheckBox *chkCopy;
};

#endif // SCENETRANSFORMDIALOG_H
