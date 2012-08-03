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

class LineEditDouble;

class SceneViewPostInterface;

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    VideoDialog(SceneViewPostInterface *sceneView, QWidget *parent = 0);
    ~VideoDialog();

    void showDialog();

private:
    SceneViewPostInterface *m_sceneView;

    int m_timeStep;
    QList<double> m_timeLevels;

    int m_adaptiveStep;

    QTabWidget *tabType;
    QWidget *tabTransient;
    QWidget *tabAdaptivity;

    QTimer *timer;

    // file
    QPushButton *btnClose;
    QPushButton *btnAnimate;
    QProgressBar *progressBar;

    LineEditDouble *txtDelay;
    QLabel *lblStep;
    QLabel *lblStepLabel;

    // adaptivity
    QSpinBox *txtAdaptiveAnimateFrom;
    QSpinBox *txtAdaptiveAnimateTo;
    QSlider *sldAdaptiveAnimate;

    // transient
    QSpinBox *txtTransientAnimateFrom;
    QSpinBox *txtTransientAnimateTo;
    QSlider *sldTransientAnimate;

    void createControls();
    QWidget *createControlsViewportAdaptiveSteps();
    QWidget *createControlsViewportTimeSteps();

private slots:
    void doAdaptiveAnimate();
    void doAdaptiveAnimateNextStep();
    void doAdaptiveSetStep(int index);
    void doAdaptiveValueFromChanged(int index);
    void doAdaptiveValueToChanged(int index);

    void doTransientAnimate();
    void doTransientAnimateNextStep();
    void doTransientSetStep(int index);
    void doTransientValueFromChanged(int index);
    void doTransientValueToChanged(int index);

    void doCreateImages();

    void tabChanged(int index);

    void doClose();
};

#endif // VIDEODIALOG_H
