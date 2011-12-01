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

#include "scenemarkerselectdialog.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "sceneview.h"

SceneMarkerSelectDialog::SceneMarkerSelectDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    logMessage("SceneMarkerSelectDialog::SceneMarkerSelectDialog()");

    m_sceneView = sceneView;

    setWindowIcon(icon(""));
    setWindowTitle(tr("Select by marker"));

    createControls();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void SceneMarkerSelectDialog::createControls()
{
    assert(0); //TODO
//    logMessage("SceneMarkerSelectDialog::createControls()");

//    // surface
//    lstSurface = new QListWidget(this);
//    for (int i = 1; i < Util::scene()->boundaries.count(); i++)
//    {
//        QListWidgetItem *item = new QListWidgetItem(lstSurface);
//        item->setText(QString::fromStdString(Util::scene()->boundaries[i]->name));
//        item->setCheckState(Qt::Unchecked);
//        lstSurface->addItem(item);
//    }

//    QGridLayout *layoutSurface = new QGridLayout();
//    layoutSurface->addWidget(lstSurface);

//    widSurface = new QWidget();
//    widSurface->setLayout(layoutSurface);

//    // volume
//    lstVolume = new QListWidget(this);
//    for (int i = 1; i < Util::scene()->materials.count(); i++)
//    {
//        QListWidgetItem *item = new QListWidgetItem(lstVolume);
//        item->setText(QString::fromStdString(Util::scene()->materials[i]->name));
//        item->setCheckState(Qt::Unchecked);
//        lstVolume->addItem(item);
//    }

//    QGridLayout *layoutVolume = new QGridLayout();
//    layoutVolume->addWidget(lstVolume);

//    widVolume = new QWidget();
//    widVolume->setLayout(layoutVolume);

//    // dialog buttons
//    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
//    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

//    // tab widget
//    tabWidget = new QTabWidget(this);
//    tabWidget->addTab(widSurface, icon(""), tr("Surface"));
//    tabWidget->addTab(widVolume, icon(""), tr("Volume"));

//    QVBoxLayout *layout = new QVBoxLayout();
//    layout->addWidget(tabWidget, 1);
//    layout->addStretch();
//    layout->addWidget(buttonBox);

//    setLayout(layout);
}

void SceneMarkerSelectDialog::doAccept()
{
    logMessage("SceneMarkerSelectDialog::doAccept()");

    if (tabWidget->currentWidget() == widSurface)
    {
        Util::scene()->selectNone();
        m_sceneView->actSceneModePostprocessor->trigger();
        m_sceneView->actPostprocessorModeSurfaceIntegral->trigger();
        for (int i = 0; i < lstSurface->count(); i++)
        {
            if (lstSurface->item(i)->checkState() == Qt::Checked)
            {
                for (int j = 0; j < Util::scene()->edges->length(); j++)
                {
                    if (Util::scene()->edges->at(j)->marker == Util::scene()->boundaries->items().at(i+1))
                        Util::scene()->edges->at(j)->isSelected = true;
                }
            }
        }
        m_sceneView->doInvalidated();
    }

    if (tabWidget->currentWidget() == widVolume)
    {
        Util::scene()->selectNone();
        m_sceneView->actSceneModePostprocessor->trigger();
        m_sceneView->actPostprocessorModeVolumeIntegral->trigger();
        for (int i = 0; i < lstVolume->count(); i++)
        {
            if (lstVolume->item(i)->checkState() == Qt::Checked)
            {
                for (int j = 0; j < Util::scene()->labels->length(); j++)
                {
                    if (Util::scene()->labels->at(j)->marker == Util::scene()->materials->items().at(i+1))
                        Util::scene()->labels->at(j)->isSelected = true;
                }
            }
        }
        m_sceneView->doInvalidated();
    }

    accept();
}

void SceneMarkerSelectDialog::doReject()
{
    logMessage("SceneMarkerSelectDialog::doReject()");

    reject();
}
