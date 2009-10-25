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
    QString commandMencoder;
    QString outputFile;

    QPushButton *btnEncodeMencoder;
    QPushButton *btnSaveVideo;
    QLineEdit *txtCommand;
    QComboBox *cmbCodec;
    QComboBox *cmbFormat;
    QSpinBox *txtFPS;

    void createControls();
    QVBoxLayout *createPlayer();

private slots:
    void doCommandMencoder();
    void doCreateImages();
    void doEncodeMencoder();
    void doSaveVideo();
    void doVideoCreated(int result);
    void doClose();
};

#endif // VIDEODIALOG_H
