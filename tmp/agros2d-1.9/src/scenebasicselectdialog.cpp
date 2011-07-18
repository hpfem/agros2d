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

#include "gui.h"
#include "scene.h"
#include "sceneview.h"

SceneBasicSelectDialog::SceneBasicSelectDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    logMessage("SceneBasicSelectDialog::SceneBasicSelectDialog()");

    m_sceneView = sceneView;

    setWindowIcon(icon(""));
    setWindowTitle(tr("Select edge"));

    createControls();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void SceneBasicSelectDialog::createControls()
{
    logMessage("SceneBasicSelectDialog::createControls()");

    // edge
    lstEdges = new QListWidget(this);
    for (int i = 1; i < Util::scene()->edges.count(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(lstEdges);
        item->setText(QString::number(i)); // Util::scene()->edges[i]->
        if (Util::scene()->edges[i]->isSelected)
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
    logMessage("SceneBasicSelectDialog::doAccept()");

    Util::scene()->selectNone();
    for (int i = 0; i < lstEdges->count(); i++)
    {
        Util::scene()->edges[i]->isSelected = (lstEdges->item(i)->checkState() == Qt::Checked);
    }
    m_sceneView->doInvalidated();
    accept();
}

void SceneBasicSelectDialog::doReject()
{
    logMessage("SceneBasicSelectDialog::doReject()");

    reject();
}

