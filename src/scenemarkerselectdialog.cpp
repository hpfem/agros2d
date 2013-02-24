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

#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "sceneview_post2d.h"

SceneMarkerSelectDialog::SceneMarkerSelectDialog(SceneViewPost2D *sceneView, SceneModePostprocessor mode, QWidget *parent) : QDialog(parent)
{
    m_sceneViewPost2D = sceneView;

    setWindowIcon(icon(""));
    setWindowTitle(tr("Select by marker"));

    createControls();

    if (mode == SceneModePostprocessor_SurfaceIntegral)
        tabWidget->setCurrentWidget(widSurface);
    else if (mode == SceneModePostprocessor_VolumeIntegral)
        tabWidget->setCurrentWidget(widVolume);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void SceneMarkerSelectDialog::createControls()
{
    // surface
    lstSurface = new QListWidget(this);
    foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(m_sceneViewPost2D->postHermes()->activeViewField()).items())
    {
        QListWidgetItem *item = new QListWidgetItem(lstSurface);
        item->setText(boundary->name());
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, boundary->variant());
        lstSurface->addItem(item);
    }

    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstSurface);

    widSurface = new QWidget();
    widSurface->setLayout(layoutSurface);

    // volume
    lstVolume = new QListWidget(this);
    foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(m_sceneViewPost2D->postHermes()->activeViewField()).items())
    {
        QListWidgetItem *item = new QListWidgetItem(lstVolume);
        item->setText(material->name());
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, material->variant());
        lstVolume->addItem(item);
    }

    QGridLayout *layoutVolume = new QGridLayout();
    layoutVolume->addWidget(lstVolume);

    widVolume = new QWidget();
    widVolume->setLayout(layoutVolume);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    // tab widget
    tabWidget = new QTabWidget(this);
    tabWidget->addTab(widSurface, icon(""), tr("Surface"));
    tabWidget->addTab(widVolume, icon(""), tr("Volume"));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabWidget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SceneMarkerSelectDialog::doAccept()
{
    if (tabWidget->currentWidget() == widSurface)
    {
        Agros2D::scene()->selectNone();
        m_sceneViewPost2D->actPostprocessorModeSurfaceIntegral->trigger();
        for (int i = 0; i < lstSurface->count(); i++)
        {
            if (lstSurface->item(i)->checkState() == Qt::Checked)
            {
                foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
                {
                    if (edge->marker(m_sceneViewPost2D->postHermes()->activeViewField()) ==
                            lstSurface->item(i)->data(Qt::UserRole).value<SceneBoundary *>())
                        edge->setSelected(true);
                }
            }
        }
        m_sceneViewPost2D->postHermes()->refresh();
    }

    if (tabWidget->currentWidget() == widVolume)
    {
        Agros2D::scene()->selectNone();
        m_sceneViewPost2D->actPostprocessorModeVolumeIntegral->trigger();
        for (int i = 0; i < lstVolume->count(); i++)
        {
            if (lstVolume->item(i)->checkState() == Qt::Checked)
            {
                foreach (SceneLabel *label, Agros2D::scene()->labels->items())
                {
                    if (label->marker(m_sceneViewPost2D->postHermes()->activeViewField()) ==
                            lstVolume->item(i)->data(Qt::UserRole).value<SceneMaterial *>())
                        label->setSelected(true);
                }
            }
        }
        m_sceneViewPost2D->postHermes()->refresh();
    }

    accept();
}

void SceneMarkerSelectDialog::doReject()
{
    reject();
}
