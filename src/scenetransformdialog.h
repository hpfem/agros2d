#ifndef SCENETRANSFORMDIALOG_H
#define SCENETRANSFORMDIALOG_H

#include <QtGui/QTabWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>

#include "util.h"
#include "gui.h"
#include "scene.h"

class Scene;

class SceneTransformDialog : public QDialog
{
    Q_OBJECT
public:
    SceneTransformDialog(Scene *scene, QWidget *parent = 0);
    ~SceneTransformDialog();

private slots:
    void doAccept();
    void doReject();

protected:
    Scene *m_scene;

    void createControls();

private:
    QTabWidget* tabWidget;

    QWidget *widTranslate;
    QWidget *widRotate;
    QWidget *widScale;

    SLineEdit *txtTranslateX;
    SLineEdit *txtTranslateY;
    SLineEdit *txtRotateBasePointX;
    SLineEdit *txtRotateBasePointY;
    SLineEdit *txtRotateAngle;
    SLineEdit *txtScaleBasePointX;
    SLineEdit *txtScaleBasePointY;
    SLineEdit *txtScaleFactor;

    QCheckBox *chkCopy;
};

#endif // SCENETRANSFORMDIALOG_H
