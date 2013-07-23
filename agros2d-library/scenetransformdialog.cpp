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

#include "scenetransformdialog.h"

#include "util/global.h"
#include "gui/valuelineedit.h"

#include "scene.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

SceneTransformDialog::SceneTransformDialog(SceneViewPreprocessor *sceneViewPreprocessor, QWidget *parent)
    : QDialog(parent), m_sceneViewPreprocessor(sceneViewPreprocessor)
{
    setWindowIcon(icon("scene-transform"));
    setWindowTitle(tr("Transform"));

    createControls();

    QSettings settings;
    restoreGeometry(settings.value("SceneTransformDialog/Geometry", saveGeometry()).toByteArray());
}

SceneTransformDialog::~SceneTransformDialog()
{
    QSettings settings;
    settings.setValue("SceneTransformDialog/Geometry", saveGeometry());
}


void SceneTransformDialog::showDialog()
{
    lstTranslateX->setText(Agros2D::problem()->config()->labelX() + ":");
    lstTranslateY->setText(Agros2D::problem()->config()->labelY() + ":");
    lstRotateBasePointX->setText(Agros2D::problem()->config()->labelX() + ":");
    lstRotateBasePointY->setText(Agros2D::problem()->config()->labelY() + ":");
    lstScaleBasePointX->setText(Agros2D::problem()->config()->labelX() + ":");
    lstScaleBasePointY->setText(Agros2D::problem()->config()->labelY() + ":");

    show();
}

void SceneTransformDialog::createControls()
{
    // translate
    txtTranslateX = new ValueLineEdit();
    txtTranslateY = new ValueLineEdit();

    lstTranslateX = new QLabel();
    lstTranslateY = new QLabel();

    QGridLayout *layoutTranslate = new QGridLayout();
    layoutTranslate->addWidget(lstTranslateX, 0, 0);
    layoutTranslate->addWidget(txtTranslateX, 0, 1);
    layoutTranslate->addWidget(lstTranslateY, 1, 0);
    layoutTranslate->addWidget(txtTranslateY, 1, 1);
    layoutTranslate->addWidget(new QLabel(""), 2, 0);

    widTranslate = new QWidget();
    widTranslate->setLayout(layoutTranslate);

    // rotate
    txtRotateBasePointX = new ValueLineEdit();
    txtRotateBasePointY = new ValueLineEdit();
    txtRotateAngle = new ValueLineEdit();

    lstRotateBasePointX = new QLabel();
    lstRotateBasePointY = new QLabel();

    QGridLayout *layoutRotate = new QGridLayout();
    layoutRotate->addWidget(lstRotateBasePointX, 0, 0);
    layoutRotate->addWidget(txtRotateBasePointX, 0, 1);
    layoutRotate->addWidget(lstRotateBasePointY, 1, 0);
    layoutRotate->addWidget(txtRotateBasePointY, 1, 1);
    layoutRotate->addWidget(new QLabel(tr("Angle:")), 2, 0);
    layoutRotate->addWidget(txtRotateAngle, 2, 1);

    widRotate = new QWidget();
    widRotate->setLayout(layoutRotate);

    // scale
    txtScaleBasePointX = new ValueLineEdit();
    txtScaleBasePointY = new ValueLineEdit();
    txtScaleFactor = new ValueLineEdit();

    lstScaleBasePointX = new QLabel();
    lstScaleBasePointY = new QLabel();

    QGridLayout *layoutScale = new QGridLayout();
    layoutScale->addWidget(lstScaleBasePointX, 0, 0);
    layoutScale->addWidget(txtScaleBasePointX, 0, 1);
    layoutScale->addWidget(lstScaleBasePointY, 1, 0);
    layoutScale->addWidget(txtScaleBasePointY, 1, 1);
    layoutScale->addWidget(new QLabel(tr("Scaling factor:")), 2, 0);
    layoutScale->addWidget(txtScaleFactor, 2, 1);

    widScale = new QWidget();
    widScale->setLayout(layoutScale);

    // copy
    chkCopy = new QCheckBox(tr("Copy objects"));
    connect(chkCopy, SIGNAL(toggled(bool)), this, SLOT(doCopyChecked(bool)));
    chkWithMarkers = new QCheckBox(tr("Copy with markers"));
    chkWithMarkers->setChecked(true);
    chkWithMarkers->setEnabled(false);

    // dialog buttons
    QPushButton *btnApply = new QPushButton(tr("Apply"));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(doTransform()));
    btnApply->setDefault(true);

    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtonBox = new QHBoxLayout();
    layoutButtonBox->addStretch();
    layoutButtonBox->addWidget(btnApply);
    layoutButtonBox->addWidget(btnClose);

    // tab widget
    tabWidget = new QTabWidget();
    tabWidget->addTab(widTranslate, icon(""), tr("Translate"));
    tabWidget->addTab(widRotate, icon(""), tr("Rotate"));
    tabWidget->addTab(widScale, icon(""), tr("Scale"));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->addWidget(chkCopy);
    layout->addWidget(chkWithMarkers);
    layout->addStretch();
    layout->addLayout(layoutButtonBox);

    setLayout(layout);
}

void SceneTransformDialog::doCopyChecked(bool checked)
{
    chkWithMarkers->setEnabled(chkCopy->isChecked());
}

void SceneTransformDialog::doClose()
{
    close();
}

void SceneTransformDialog::doTransform()
{
    if (tabWidget->currentWidget() == widTranslate)
    {
        if (!txtTranslateX->evaluate(false)) return;
        if (!txtTranslateY->evaluate(false)) return;
        Agros2D::scene()->transformTranslate(Point(txtTranslateX->number(), txtTranslateY->number()), chkCopy->isChecked(), chkWithMarkers->isChecked());
    }

    if (tabWidget->currentWidget() == widRotate)
    {
        if (!txtRotateBasePointX->evaluate(false)) return;
        if (!txtRotateBasePointY->evaluate(false)) return;
        if (!txtRotateAngle->evaluate(false)) return;
        Agros2D::scene()->transformRotate(Point(txtRotateBasePointX->number(), txtRotateBasePointY->number()), txtRotateAngle->number(), chkCopy->isChecked(), chkWithMarkers->isChecked());
    }

    if (tabWidget->currentWidget() == widScale)
    {
        if (!txtScaleBasePointX->evaluate(false)) return;
        if (!txtScaleBasePointY->evaluate(false)) return;
        if (!txtScaleFactor->evaluate(false)) return;
        Agros2D::scene()->transformScale(Point(txtScaleBasePointX->number(), txtScaleBasePointY->number()), txtScaleFactor->number(), chkCopy->isChecked(), chkWithMarkers->isChecked());
    }

    m_sceneViewPreprocessor->refresh();
}
