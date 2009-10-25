#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include "util.h"
#include "sceneview.h"

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    VideoDialog(SceneView *sceneView, QWidget *parent = 0);
    ~VideoDialog();

    void showDialog();

private:
    SceneView *m_sceneView;
    QString commandFFmpeg;
    QString outputFile;

    QPushButton *btnEncodeFFmpeg;
    QPushButton *btnSaveVideo;
    QPushButton *btnOpenVideo;
    QLineEdit *txtCommand;
    QComboBox *cmbCodec;
    QComboBox *cmbFormat;
    QSpinBox *txtFPS;

    void createControls();
    QVBoxLayout *createPlayer();

private slots:
    void doCommandFFmpeg();
    void doCreateImages();
    void doEncodeFFmpeg();
    void doSaveVideo();
    void doOpenVideo();
    void doVideoCreated(int result);
    void doClose();
};

#endif // VIDEODIALOG_H
