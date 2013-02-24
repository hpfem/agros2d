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

class PostHermes;
class LineEditDouble;
class SceneViewPostInterface;
class FieldInfo;

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    VideoDialog(SceneViewPostInterface *sceneViewInterface, PostHermes *postHermes, QWidget *parent = 0);
    ~VideoDialog();

    void showDialog();

private:
    SceneViewPostInterface *m_sceneViewInterface;
    PostHermes *m_postHermes;

    int m_timeSteps;
    int m_timeStepStore;
    QList<double> m_timeLevels;

    int m_adaptiveStepStore;
    int m_adaptiveSteps;
    bool m_showRulersStore;
    bool m_showGridStore;
    bool m_showAxesStore;

    QTabWidget *tabType;
    QWidget *tabTransient;
    QWidget *tabAdaptivity;

    QTimer *timer;

    // file
    QPushButton *btnClose;
    QPushButton *btnAnimate;

    QCheckBox *chkSaveImages;
    QCheckBox *chkFigureShowGrid;
    QCheckBox *chkFigureShowRulers;
    QCheckBox *chkFigureShowAxes;

    // adaptivity
    QLabel *lblAdaptiveStep;
    QSlider *sliderAdaptiveAnimate;

    // transient
    QLabel *lblTransientStep;
    QLabel *lblTransientTime;
    QSlider *sliderTransientAnimate;

    void createControls();
    QWidget *createControlsViewportAdaptiveSteps();
    QWidget *createControlsViewportTimeSteps();

private slots:
    void adaptiveAnimate();
    void adaptiveAnimateNextStep();
    void adaptiveSetStep(int index);

    void transientAnimate();
    void transientAnimateNextStep();
    void setTransientStep(int transientStep);

    void tabChanged(int index);

    void doClose();
    void doVideo();
};

// *********************************************************************************

class ImageSequenceDialog : public QDialog
{
    Q_OBJECT

public:
    ImageSequenceDialog(QWidget *parent = 0);
    ~ImageSequenceDialog();

    bool showDialog();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    QStringList m_images;
    QPixmap m_currentImage;

    QTimer *timer;
    QLabel *lblImage;
    QLabel *lblStep;

    QPushButton *btnClose;
    QPushButton *btnAnimate;
    QSlider *sliderAnimateSequence;
    QComboBox *cmbSpeed;

private slots:
    void updateImage();
    void animate();
    void animateNextStep();
    void animateSequence(int index);

    void doClose();
};

#endif // VIDEODIALOG_H
