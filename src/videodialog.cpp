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

#include "videodialog.h"

#include "scene.h"
#include "sceneview_post2d.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

#include "gui/lineeditdouble.h"

VideoDialog::VideoDialog(SceneViewPostInterface *sceneViewInterface, PostHermes *postHermes, QWidget *parent)
    : QDialog(parent), m_sceneViewInterface(sceneViewInterface), m_postHermes(postHermes)
{
    setModal(true);
    setWindowIcon(icon("video"));
    setWindowTitle(tr("Video"));

    // create directory
    QDir(tempProblemDir()).mkdir("video");

    // store timestep
    m_timeStep = Util::scene()->activeTimeStep();
    // store adaptive step
    m_adaptiveStep = Util::scene()->activeAdaptivityStep();

    // time steps
    m_timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());

    // timer create images
    timer = new QTimer(this);

    createControls();
    tabChanged(0);
}

VideoDialog::~VideoDialog()
{
    // restore previous timestep
    Util::scene()->setActiveTimeStep(m_timeStep);
    m_postHermes->refresh();

    delete timer;
}

void VideoDialog::showDialog()
{
    // transient
    int timeSteps = m_timeLevels.count();

    txtTransientAnimateFrom->setMaximum(timeSteps);
    txtTransientAnimateTo->setMaximum(timeSteps);
    txtTransientAnimateTo->setValue(timeSteps);

    doTransientValueFromChanged(txtTransientAnimateFrom->value());
    doTransientValueToChanged(txtTransientAnimateTo->value());

    // adaptivity
    int adaptiveSteps = Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal) + 1;

    txtAdaptiveAnimateFrom->setMaximum(adaptiveSteps);
    txtAdaptiveAnimateTo->setMaximum(adaptiveSteps);
    txtAdaptiveAnimateTo->setValue(adaptiveSteps);

    doAdaptiveValueFromChanged(txtAdaptiveAnimateFrom->value());
    doAdaptiveValueToChanged(txtAdaptiveAnimateTo->value());

    exec();
}

void VideoDialog::createControls()
{
    // tab
    tabTransient = createControlsViewportTimeSteps();
    tabAdaptivity = createControlsViewportAdaptiveSteps();

    tabType = new QTabWidget();
    tabType->addTab(tabAdaptivity, icon(""), tr("Adaptivity"));
    tabType->addTab(tabTransient, icon(""), tr("Transient"));
    connect(tabType, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    btnClose = new QPushButton(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QPushButton *btnVideo = new QPushButton(tr("Show video"));
    btnVideo->setDefault(true);
    connect(btnVideo, SIGNAL(clicked()), this, SLOT(doVideo()));

    btnAnimate = new QPushButton(tr("Run"));

    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnAnimate);
    layoutButton->addWidget(btnVideo);
    layoutButton->addWidget(btnClose);

    txtDelay = new LineEditDouble(0.1);
    lblStepLabel = new QLabel("");
    lblStep = new QLabel("0.0");
    chkSaveImages = new QCheckBox(tr("Save images to disk"));
    chkSaveImages->setChecked(true);

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnAnimate);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel(tr("Delay:")), 0, 0);
    layout->addWidget(txtDelay, 0, 1);
    layout->addWidget(lblStepLabel, 1, 0);
    layout->addWidget(lblStep, 1, 1);
    layout->addWidget(chkSaveImages, 2, 0, 1, 2);
    layout->addWidget(tabType, 3, 0, 1, 2);
    layout->addLayout(layoutButton, 4, 0, 1, 2);

    setLayout(layout);

    setMinimumWidth(300);
}

QWidget *VideoDialog::createControlsViewportTimeSteps()
{
    // adaptivity
    txtTransientAnimateFrom = new QSpinBox();
    txtTransientAnimateFrom->setMinimum(1);
    connect(txtTransientAnimateFrom, SIGNAL(valueChanged(int)), this, SLOT(doTransientValueFromChanged(int)));

    txtTransientAnimateTo = new QSpinBox();
    txtTransientAnimateTo->setMinimum(1);
    connect(txtTransientAnimateTo, SIGNAL(valueChanged(int)), this, SLOT(doTransientValueToChanged(int)));

    sliderTransientAnimate = new QSlider(Qt::Horizontal);
    sliderTransientAnimate->setTickPosition(QSlider::TicksBelow);
    connect(sliderTransientAnimate, SIGNAL(valueChanged(int)), this, SLOT(doTransientSetStep(int)));

    QGridLayout *layoutControlsViewport = new QGridLayout();
    layoutControlsViewport->addWidget(new QLabel(tr("From:")), 0, 0);
    layoutControlsViewport->addWidget(txtTransientAnimateFrom, 0, 1);
    layoutControlsViewport->addWidget(new QLabel(tr("To:")), 1, 0);
    layoutControlsViewport->addWidget(txtTransientAnimateTo, 1, 1);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addLayout(layoutControlsViewport);
    layoutViewport->addWidget(sliderTransientAnimate);

    QWidget *widViewport = new QWidget();
    widViewport->setLayout(layoutViewport);

    return widViewport;
}

QWidget *VideoDialog::createControlsViewportAdaptiveSteps()
{
    // adaptivity
    txtAdaptiveAnimateFrom = new QSpinBox();
    txtAdaptiveAnimateFrom->setMinimum(1);
    connect(txtAdaptiveAnimateFrom, SIGNAL(valueChanged(int)), this, SLOT(doAdaptiveValueFromChanged(int)));

    txtAdaptiveAnimateTo = new QSpinBox();
    txtAdaptiveAnimateTo->setMinimum(1);
    connect(txtAdaptiveAnimateTo, SIGNAL(valueChanged(int)), this, SLOT(doAdaptiveValueToChanged(int)));

    sliderAdaptiveAnimate = new QSlider(Qt::Horizontal);
    sliderAdaptiveAnimate->setTickPosition(QSlider::TicksBelow);
    connect(sliderAdaptiveAnimate, SIGNAL(valueChanged(int)), this, SLOT(doAdaptiveSetStep(int)));

    QGridLayout *layoutControlsViewport = new QGridLayout();
    layoutControlsViewport->addWidget(new QLabel(tr("From:")), 0, 0);
    layoutControlsViewport->addWidget(txtAdaptiveAnimateFrom, 0, 1);
    layoutControlsViewport->addWidget(new QLabel(tr("To:")), 1, 0);
    layoutControlsViewport->addWidget(txtAdaptiveAnimateTo, 1, 1);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addLayout(layoutControlsViewport);
    layoutViewport->addWidget(sliderAdaptiveAnimate);

    QWidget *widViewport = new QWidget();
    widViewport->setLayout(layoutViewport);

    return widViewport;
}

void VideoDialog::doAdaptiveAnimate()
{
    if (timer->isActive())
    {
        btnClose->setEnabled(true);

        timer->stop();
        btnAnimate->setText(tr("Run"));
    }
    else
    {
        btnClose->setEnabled(false);

        btnAnimate->setText(tr("Stop"));
        doAdaptiveSetStep(txtAdaptiveAnimateFrom->value());
        timer->start(txtDelay->value() * 1e3);
    }
}

void VideoDialog::doTransientAnimate()
{
    if (timer->isActive())
    {
        btnClose->setEnabled(true);

        timer->stop();
        btnAnimate->setText(tr("Run"));
    }
    else
    {
        btnClose->setEnabled(false);

        btnAnimate->setText(tr("Stop"));
        doAdaptiveSetStep(txtTransientAnimateFrom->value());
        timer->start(txtDelay->value() * 1e3);
    }
}

void VideoDialog::doTransientAnimateNextStep()
{
    if (Util::scene()->activeTimeStep() + 1 < txtTransientAnimateTo->value())
    {
        doTransientSetStep(Util::scene()->activeTimeStep() + 2);
    }
    else
    {
        doTransientAnimate();
    }
}

void VideoDialog::doTransientSetStep(int index)
{
    Util::scene()->setActiveTimeStep(index - 1);
    m_postHermes->refresh();

    if (chkSaveImages->isChecked())
        m_sceneViewInterface->saveImageToFile(tempProblemDir() + QString("/video/video_%1.png").arg(QString("0000000" + QString::number(index)).right(8)));

    sliderTransientAnimate->setValue(index);

    QString time = QString::number(m_timeLevels[index - 1], 'g');
    lblStep->setText(tr("%1 s").arg(time));

    QApplication::processEvents();
}

void VideoDialog::doTransientValueFromChanged(int index)
{
    sliderTransientAnimate->setMinimum(txtTransientAnimateFrom->value());
}

void VideoDialog::doTransientValueToChanged(int index)
{
    sliderTransientAnimate->setMaximum(txtTransientAnimateTo->value());
}

void VideoDialog::doAdaptiveAnimateNextStep()
{
    if (Util::scene()->activeAdaptivityStep() + 1 < txtAdaptiveAnimateTo->value())
    {
        doAdaptiveSetStep(Util::scene()->activeAdaptivityStep() + 2);
    }
    else
    {
        doAdaptiveAnimate();
    }
}

void VideoDialog::doAdaptiveSetStep(int index)
{
    Util::scene()->setActiveAdaptivityStep(index - 1);
    m_postHermes->refresh();

    if (chkSaveImages->isChecked())
        m_sceneViewInterface->saveImageToFile(tempProblemDir() + QString("/video/video_%1.png").arg(QString("0000000" + QString::number(index)).right(8)));

    sliderAdaptiveAnimate->setValue(index);

    lblStep->setText(tr("%1").arg(index));

    QApplication::processEvents();
}

void VideoDialog::doAdaptiveValueFromChanged(int index)
{
    sliderAdaptiveAnimate->setMinimum(txtAdaptiveAnimateFrom->value());
}

void VideoDialog::doAdaptiveValueToChanged(int index)
{
    sliderAdaptiveAnimate->setMaximum(txtAdaptiveAnimateTo->value());
}

void VideoDialog::tabChanged(int index)
{
    timer->disconnect(this);
    btnAnimate->disconnect(this);

    if (tabType->currentWidget() == tabTransient)
    {
        lblStepLabel->setText(tr("Time:"));

        connect(btnAnimate, SIGNAL(clicked()), this, SLOT(doTransientAnimate()));
        connect(timer, SIGNAL(timeout()), this, SLOT(doTransientAnimateNextStep()));
    }
    else if (tabType->currentWidget() == tabAdaptivity)
    {
        lblStepLabel->setText(tr("Step:"));

        connect(btnAnimate, SIGNAL(clicked()), this, SLOT(doAdaptiveAnimate()));
        connect(timer, SIGNAL(timeout()), this, SLOT(doAdaptiveAnimateNextStep()));
    }
}

void VideoDialog::doVideo()
{
    ImageSequenceDialog video;
    video.exec();
}

void VideoDialog::doClose()
{
    hide();
}

// *********************************************************************************

ImageSequenceDialog::ImageSequenceDialog(QWidget *parent)
    : QDialog(parent)
{
    // read images
    QStringList filters;
    filters << "*.png";

    QDir dir(tempProblemDir() + QString("/video"));
    dir.setNameFilters(filters);
    images = dir.entryList();

    sliderAnimateSequence = new QSlider(Qt::Horizontal);
    sliderAnimateSequence->setTickPosition(QSlider::TicksBelow);
    sliderAnimateSequence->setMaximum(images.count() - 1);
    connect(sliderAnimateSequence, SIGNAL(valueChanged(int)), this, SLOT(doAnimateSequence(int)));

    btnAnimate = new QPushButton(tr("Run"));

    btnClose = new QPushButton(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutControls = new QHBoxLayout();
    layoutControls->addWidget(sliderAnimateSequence);
    layoutControls->addWidget(btnAnimate);
    layoutControls->addWidget(btnClose);

    lblImage = new QLabel();

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addWidget(lblImage);
    layoutViewport->addLayout(layoutControls);

    setLayout(layoutViewport);

    // timer create images
    timer = new QTimer(this);
    connect(btnAnimate, SIGNAL(clicked()), this, SLOT(doAnimate()));
    connect(timer, SIGNAL(timeout()), this, SLOT(doAnimateNextStep()));

    if (images.count() > 0)
        doAnimateSequence(0);
}

bool ImageSequenceDialog::showDialog()
{
    return exec();
}

void ImageSequenceDialog::doAnimate()
{
    if (timer->isActive())
    {
        btnClose->setEnabled(true);

        timer->stop();
        btnAnimate->setText(tr("Run"));
    }
    else
    {
        sliderAnimateSequence->setValue(0);

        btnClose->setEnabled(false);

        btnAnimate->setText(tr("Stop"));
        timer->start(5e2);
    }
}

void ImageSequenceDialog::doAnimateNextStep()
{
    if (sliderAnimateSequence->value() < images.count() - 1)
    {
        sliderAnimateSequence->setValue(sliderAnimateSequence->value() + 1);
        doAnimateSequence(sliderAnimateSequence->value());
    }
    else
    {
        doAnimate();
    }
}

void ImageSequenceDialog::doAnimateSequence(int index)
{
    if (images.count() == 0)
        return;

    QString fileName = tempProblemDir() + QString("/video/") + images.at(index);
    if (QFile::exists(fileName))
    {
        QPixmap image(fileName);
        lblImage->setPixmap(image);
    }
}

void ImageSequenceDialog::doClose()
{
    hide();
}
