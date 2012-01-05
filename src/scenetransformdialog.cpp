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

#include "gui.h"
#include "scene.h"

SceneTransformDialog::SceneTransformDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("SceneTransformDialog::SceneTransformDialog()");

    setWindowIcon(icon("scene-transform"));
    setWindowTitle(tr("Transform"));

    createControls();

    setMinimumSize(350, 225);
    setMaximumSize(minimumSize());
}

void SceneTransformDialog::createControls()
{
    logMessage("SceneTransformDialog::createControls()");

    // translate
    txtTranslateX = new ValueLineEdit();
    txtTranslateY = new ValueLineEdit();

    QGridLayout *layoutTranslate = new QGridLayout();
    layoutTranslate->addWidget(new QLabel(Util::scene()->problemInfo()->labelX() + ":"), 0, 0);
    layoutTranslate->addWidget(txtTranslateX, 0, 1);
    layoutTranslate->addWidget(new QLabel(Util::scene()->problemInfo()->labelY() + ":"), 1, 0);
    layoutTranslate->addWidget(txtTranslateY, 1, 1);
    layoutTranslate->addWidget(new QLabel(""), 2, 0);

    widTranslate = new QWidget();
    widTranslate->setLayout(layoutTranslate);

    // rotate
    txtRotateBasePointX = new ValueLineEdit();
    txtRotateBasePointY = new ValueLineEdit();
    txtRotateAngle = new ValueLineEdit();

    QGridLayout *layoutRotate = new QGridLayout();
    layoutRotate->addWidget(new QLabel(Util::scene()->problemInfo()->labelX() + ":"), 0, 0);
    layoutRotate->addWidget(txtRotateBasePointX, 0, 1);
    layoutRotate->addWidget(new QLabel(Util::scene()->problemInfo()->labelY() + ":"), 1, 0);
    layoutRotate->addWidget(txtRotateBasePointY, 1, 1);
    layoutRotate->addWidget(new QLabel(tr("Angle:")), 2, 0);
    layoutRotate->addWidget(txtRotateAngle, 2, 1);

    widRotate = new QWidget();
    widRotate->setLayout(layoutRotate);

    // scale
    txtScaleBasePointX = new ValueLineEdit();
    txtScaleBasePointY = new ValueLineEdit();
    txtScaleFactor = new ValueLineEdit();

    QGridLayout *layoutScale = new QGridLayout();
    layoutScale->addWidget(new QLabel(Util::scene()->problemInfo()->labelX() + ":"), 0, 0);
    layoutScale->addWidget(txtScaleBasePointX, 0, 1);
    layoutScale->addWidget(new QLabel(Util::scene()->problemInfo()->labelY() + ":"), 1, 0);
    layoutScale->addWidget(txtScaleBasePointY, 1, 1);
    layoutScale->addWidget(new QLabel(tr("Scaling factor:")), 2, 0);
    layoutScale->addWidget(txtScaleFactor, 2, 1);

    widScale = new QWidget();
    widScale->setLayout(layoutScale);

    // copy
    chkCopy = new QCheckBox(tr("Copy objects"));
/*
    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));
*/

    // my dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(doApply()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    // tab widget
    tabWidget = new QTabWidget();
    tabWidget->addTab(widTranslate, icon(""), tr("Translate"));
    tabWidget->addTab(widRotate, icon(""), tr("Rotate"));
    tabWidget->addTab(widScale, icon(""), tr("Scale"));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->addWidget(chkCopy);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SceneTransformDialog::doAccept()
{
    logMessage("SceneTransformDialog::doAccept()");

    if (tabWidget->currentWidget() == widTranslate)
    {
        if (!txtTranslateX->evaluate(false)) return;
        if (!txtTranslateY->evaluate(false)) return;
        Util::scene()->transformTranslate(Point(txtTranslateX->number(), txtTranslateY->number()), chkCopy->isChecked());
    }

    if (tabWidget->currentWidget() == widRotate)
    {
        if (!txtRotateBasePointX->evaluate(false)) return;
        if (!txtRotateBasePointY->evaluate(false)) return;
        if (!txtRotateAngle->evaluate(false)) return;
        Util::scene()->transformRotate(Point(txtRotateBasePointX->number(), txtRotateBasePointY->number()), txtRotateAngle->number(), chkCopy->isChecked());
    }

    if (tabWidget->currentWidget() == widScale)
    {
        if (!txtScaleBasePointX->evaluate(false)) return;
        if (!txtScaleBasePointY->evaluate(false)) return;
        if (!txtScaleFactor->evaluate(false)) return;
        Util::scene()->transformScale(Point(txtScaleBasePointX->number(), txtScaleBasePointY->number()), txtScaleFactor->number(), chkCopy->isChecked());
    }

    accept();
}

void SceneTransformDialog::doReject()
{
    logMessage("SceneTransformDialog::doReject()");

    reject();
}


void SceneTransformDialog::doApply()
{
    logMessage("SceneTransformDialog::doApply()");

    if (tabWidget->currentWidget() == widTranslate)
    {
        if (!txtTranslateX->evaluate(false)) return;
        if (!txtTranslateY->evaluate(false)) return;
        Util::scene()->transformTranslate(Point(txtTranslateX->number(), txtTranslateY->number()), chkCopy->isChecked());
    }

    exec();

}
