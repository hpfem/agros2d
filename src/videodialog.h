#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include "util.h"
#include "gui.h"
#include "sceneview.h"

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    VideoDialog(SceneView *sceneView, QWidget *parent = 0);
    ~VideoDialog();

    void showDialog();

private:
    int m_timeStep;
    SceneView *m_sceneView;

    QString commandFFmpeg;
    QString outputFile;

    QTabWidget* tabType;

    // file
    QPushButton *btnEncodeFFmpeg;
    QPushButton *btnSaveVideo;
    QPushButton *btnOpenVideo;
    QComboBox *cmbCodec;
    QComboBox *cmbFormat;
    QSpinBox *txtFPS;

    // viewport
    QSpinBox *txtAnimateFrom;
    QSpinBox *txtAnimateTo;
    QSlider *sldAnimate;
    SLineEdit *txtAnimateDelay;
    QLabel *lblAnimateTime;


    void createControls();
    QWidget *createControlsViewport();
    QWidget *createControlsFile();

private slots:
    void doAnimate();
    void doSetTimeStep(int index);
    void doValueFromChanged(int index);
    void doValueToChanged(int index);

    void doCommandFFmpeg();
    void doCreateImages();
    void doEncodeFFmpeg();
    void doSaveVideo();
    void doOpenVideo();
    void doVideoCreated(int result);

    void doClose();
};

#endif // VIDEODIALOG_H
