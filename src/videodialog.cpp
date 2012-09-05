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

VideoDialog::VideoDialog(SceneViewPostInterface *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;

    setModal(true);
    setWindowIcon(icon("video"));
    setWindowTitle(tr("Video"));

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
    m_sceneView->refresh();

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

    btnAnimate = new QPushButton(tr("Animate"));

    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnAnimate);
    layoutButton->addWidget(btnClose);

    txtDelay = new LineEditDouble(0.1);
    lblStepLabel = new QLabel("");
    lblStep = new QLabel("0.0");

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnAnimate);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel(tr("Delay:")), 0, 0);
    layout->addWidget(txtDelay, 0, 1);
    layout->addWidget(lblStepLabel, 1, 0);
    layout->addWidget(lblStep, 1, 1);
    layout->addWidget(tabType, 2, 0, 1, 2);
    layout->addLayout(layoutButton, 3, 0, 1, 2);

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

    sldTransientAnimate = new QSlider(Qt::Horizontal);
    sldTransientAnimate->setTickPosition(QSlider::TicksBelow);
    connect(sldTransientAnimate, SIGNAL(valueChanged(int)), this, SLOT(doTransientSetStep(int)));


    QGridLayout *layoutControlsViewport = new QGridLayout();
    layoutControlsViewport->addWidget(new QLabel(tr("From:")), 0, 0);
    layoutControlsViewport->addWidget(txtTransientAnimateFrom, 0, 1);
    layoutControlsViewport->addWidget(new QLabel(tr("To:")), 1, 0);
    layoutControlsViewport->addWidget(txtTransientAnimateTo, 1, 1);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addLayout(layoutControlsViewport);
    layoutViewport->addWidget(sldTransientAnimate);

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

    sldAdaptiveAnimate = new QSlider(Qt::Horizontal);
    sldAdaptiveAnimate->setTickPosition(QSlider::TicksBelow);
    connect(sldAdaptiveAnimate, SIGNAL(valueChanged(int)), this, SLOT(doAdaptiveSetStep(int)));

    QGridLayout *layoutControlsViewport = new QGridLayout();
    layoutControlsViewport->addWidget(new QLabel(tr("From:")), 0, 0);
    layoutControlsViewport->addWidget(txtAdaptiveAnimateFrom, 0, 1);
    layoutControlsViewport->addWidget(new QLabel(tr("To:")), 1, 0);
    layoutControlsViewport->addWidget(txtAdaptiveAnimateTo, 1, 1);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addLayout(layoutControlsViewport);
    layoutViewport->addWidget(sldAdaptiveAnimate);

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
        btnAnimate->setText(tr("Animate"));
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
        btnAnimate->setText(tr("Animate"));
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

    m_sceneView->refresh();

    sldTransientAnimate->setValue(index);

    QString time = QString::number(m_timeLevels[index - 1], 'g');
    lblStep->setText(tr("%1 s").arg(time));

    QApplication::processEvents();
}

void VideoDialog::doTransientValueFromChanged(int index)
{
    sldTransientAnimate->setMinimum(txtTransientAnimateFrom->value());
}

void VideoDialog::doTransientValueToChanged(int index)
{
    sldTransientAnimate->setMaximum(txtTransientAnimateTo->value());
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

    m_sceneView->refresh();

    sldAdaptiveAnimate->setValue(index);

    lblStep->setText(tr("%1").arg(index));

    QApplication::processEvents();
}

void VideoDialog::doAdaptiveValueFromChanged(int index)
{
    sldAdaptiveAnimate->setMinimum(txtAdaptiveAnimateFrom->value());
}

void VideoDialog::doAdaptiveValueToChanged(int index)
{
    sldAdaptiveAnimate->setMaximum(txtAdaptiveAnimateTo->value());
}

void VideoDialog::doCreateImages()
{
    btnClose->setEnabled(false);
    progressBar->setMaximum(m_timeLevels.count() - 1);
    progressBar->setValue(0);

    // create directory
    QDir(tempProblemDir()).mkdir("video");
    for (int i = 0; i < m_timeLevels.count(); i++)
    {
        progressBar->setValue(i);
        Util::scene()->setActiveTimeStep(i);
        m_sceneView->saveImageToFile(tempProblemDir() + QString("/video/video_%1.png").arg(QString("0000000" + QString::number(i)).right(8)));
    }

    btnClose->setEnabled(true);
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

void VideoDialog::doClose()
{
    hide();
}
