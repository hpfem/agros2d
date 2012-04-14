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

#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include "util.h"

class SLineEditDouble;

class SceneViewPost2D;

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    VideoDialog(SceneViewPost2D *sceneView, QWidget *parent = 0);
    ~VideoDialog();

    void showDialog();

private:
    int m_timeStep;
    SceneViewPost2D *m_sceneViewPost2D;
    QList<double> m_timeLevels;

    QString commandFFmpeg;
    QString outputFile;

    QTabWidget *tabType;

    QTimer *timerAnimate;
    QTimer *timerFile;

    // file
    QPushButton *btnClose;
    QPushButton *btnEncodeFFmpeg;
    QPushButton *btnSaveVideo;
    QPushButton *btnOpenVideo;
    QPushButton *btnAnimate;
    QComboBox *cmbCodec;
    QComboBox *cmbFormat;
    QSpinBox *txtFPS;
    QProgressBar *progressBar;

    // viewport
    QSpinBox *txtAnimateFrom;
    QSpinBox *txtAnimateTo;
    QSlider *sldAnimate;
    SLineEditDouble *txtAnimateDelay;
    QLabel *lblAnimateTime;

    void createControls();
    QWidget *createControlsViewport();
    QWidget *createControlsFile();

private slots:
    void doAnimate();
    void doAnimateNextStep();
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
