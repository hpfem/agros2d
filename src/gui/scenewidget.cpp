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

#include "sceneview_common.h"
#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "hermes2d/module.h"

#include "hermes2d/problem.h"

SceneViewWidget::SceneViewWidget(SceneViewCommon *widget, QWidget *parent) : QWidget(parent)
{
    createControls(widget);

    iconLeft(widget->iconView());
    labelLeft(widget->labelView());

    connect(widget, SIGNAL(labelCenter(QString)), this, SLOT(labelCenter(QString)));
    connect(widget, SIGNAL(labelRight(QString)), this, SLOT(labelRight(QString)));
}

SceneViewWidget::SceneViewWidget(QWidget *widget, QWidget *parent) : QWidget(parent)
{
    createControls(widget);

    iconLeft(icon("scene-info"));
    labelLeft(tr("Info"));
}

SceneViewWidget::~SceneViewWidget()
{
}

void SceneViewWidget::createControls(QWidget *widget)
{
    // label
    sceneViewLabelPixmap = new QLabel();
    sceneViewLabelLeft = new QLabel();
    sceneViewLabelLeft->setMinimumWidth(150);
    sceneViewLabelCenter = new QLabel();
    sceneViewLabelCenter->setMinimumWidth(150);
    sceneViewLabelRight = new QLabel();
    sceneViewLabelRight->setMinimumWidth(200);

    QHBoxLayout *sceneViewLabelLayout = new QHBoxLayout();
    sceneViewLabelLayout->addWidget(sceneViewLabelPixmap);
    sceneViewLabelLayout->addWidget(sceneViewLabelLeft);
    sceneViewLabelLayout->addStretch(0.5);
    sceneViewLabelLayout->addWidget(sceneViewLabelCenter);
    sceneViewLabelLayout->addStretch(0.5);
    sceneViewLabelLayout->addWidget(sceneViewLabelRight);

    // view
    QVBoxLayout *sceneViewLayout = new QVBoxLayout();
    sceneViewLayout->addLayout(sceneViewLabelLayout);
    sceneViewLayout->addWidget(widget);
    sceneViewLayout->setStretch(1, 1);

    setLayout(sceneViewLayout);
}

void SceneViewWidget::labelLeft(const QString &left)
{
    sceneViewLabelLeft->setText(left);
}

void SceneViewWidget::labelCenter(const QString &center)
{
    sceneViewLabelCenter->setText(center);
}

void SceneViewWidget::labelRight(const QString &right)
{
    sceneViewLabelRight->setText(right);
}

void SceneViewWidget::iconLeft(const QIcon &left)
{
    QPixmap pixmap = left.pixmap(QSize(16, 16));
    sceneViewLabelPixmap->setPixmap(pixmap);
}
