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

#include "scenebasicselectdialog.h"

#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_common.h"
#include "hermes2d/module.h"

SceneBasicSelectDialog::SceneBasicSelectDialog(SceneViewCommon *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;

    setWindowIcon(icon(""));
    setWindowTitle(tr("Select edge"));

    createControls();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void SceneBasicSelectDialog::createControls()
{
    // edge
    lstEdges = new QListWidget(this);
    for (int i = 1; i < Agros2D::scene()->edges->length(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(lstEdges);
        item->setText(QString::number(i));
        if (Agros2D::scene()->edges->at(i)->isSelected())
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
        lstEdges->addItem(item);
    }

    connect(lstEdges, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(currentItemChanged(QListWidgetItem *, QListWidgetItem *)));

    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstEdges);

    widEdge = new QWidget();
    widEdge->setLayout(layoutSurface);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widEdge, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SceneBasicSelectDialog::doAccept()
{
    Agros2D::scene()->selectNone();
    for (int i = 0; i < lstEdges->count(); i++)
    {
        Agros2D::scene()->edges->at(i)->setSelected(lstEdges->item(i)->checkState() == Qt::Checked);
    }
    m_sceneView->refresh();
    accept();
}

void SceneBasicSelectDialog::doReject()
{
    reject();
}

