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

#include "settings.h"

#include "util/constants.h"
#include "util/global.h"

#include "gui/common.h"
#include "gui/lineeditdouble.h"

#include "hermes2d/problem_config.h"

#include "scene.h"
#include "pythonlab/pythonengine_agros.h"

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    setWindowIcon(icon("options"));
    setObjectName("Settings");

    createActions();
    createControls();

    load();

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(updateControls()));
}

void SettingsWidget::createActions()
{
    actSettings = new QAction(icon("options"), tr("Settings"), this);
    actSettings->setShortcut(tr("Ctrl+8"));
    actSettings->setCheckable(true);
}

void SettingsWidget::load()
{
    // workspace
    chkShowGrid->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowGrid).toBool());

    cmbRulersFont->setCurrentIndex(cmbRulersFont->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_RulersFontFamily).toString()));
    txtRulersFontSizes->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_RulersFontPointSize).toInt());
    cmbPostFont->setCurrentIndex(cmbPostFont->findData(Agros2D::problem()->setting()->value(ProblemSetting::View_PostFontFamily).toString()));
    txtPostFontSizes->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_PostFontPointSize).toInt());

    chkShowAxes->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowAxes).toBool());
    chkShowRulers->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool());

    chkZoomToMouse->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ZoomToMouse).toBool()); 
}

void SettingsWidget::save()
{
    // workspace
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowGrid, chkShowGrid->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowRulers, chkShowRulers->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ZoomToMouse, chkZoomToMouse->isChecked());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_RulersFontFamily, cmbRulersFont->itemData(cmbRulersFont->currentIndex()).toString());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_RulersFontPointSize, txtRulersFontSizes->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PostFontFamily, cmbPostFont->itemData(cmbPostFont->currentIndex()).toString());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_PostFontPointSize, txtPostFontSizes->value());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowAxes, chkShowAxes->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ShowRulers, chkShowRulers->isChecked());
}

void SettingsWidget::createControls()
{
    QWidget *workspace = controlsWorkspace();

    // layout workspace
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(workspace);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    // dialog buttons
    btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnOK);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(widget);
    layoutMain->addLayout(layoutButtons);

    setControls();

    setLayout(layoutMain);
}

QWidget *SettingsWidget::controlsWorkspace()
{
    // workspace
    chkShowGrid = new QCheckBox(tr("Show grid"));
    chkZoomToMouse = new QCheckBox(tr("Zoom to mouse"));
    chkShowRulers = new QCheckBox(tr("Show rulers"));
    chkShowAxes = new QCheckBox(tr("Show axes"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(chkShowGrid, 0, 0);
    layoutGrid->addWidget(chkShowAxes, 1, 0);
    layoutGrid->addWidget(chkShowRulers, 2, 0);
    layoutGrid->addWidget(chkZoomToMouse, 0, 1);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    cmbRulersFont = new QComboBox();
    fillComboBoxFonts(cmbRulersFont);
    txtRulersFontSizes = new QSpinBox();
    txtRulersFontSizes->setMinimum(6);
    txtRulersFontSizes->setMaximum(40);

    cmbPostFont = new QComboBox();
    fillComboBoxFonts(cmbPostFont);
    txtPostFontSizes = new QSpinBox();
    txtPostFontSizes->setMinimum(6);
    txtPostFontSizes->setMaximum(40);

    QGridLayout *layoutFont = new QGridLayout();
    layoutFont->setColumnStretch(1, 1);
    layoutFont->addWidget(new QLabel(tr("Rulers:")), 0, 0);
    layoutFont->addWidget(cmbRulersFont, 0, 1);
    layoutFont->addWidget(txtRulersFontSizes, 0, 2);
    layoutFont->addWidget(new QLabel(tr("Postprocessor:")), 2, 0);
    layoutFont->addWidget(cmbPostFont, 2, 1);
    layoutFont->addWidget(txtPostFontSizes, 2, 2);

    QGroupBox *grpFont = new QGroupBox(tr("Fonts"));
    grpFont->setLayout(layoutFont);

    QPushButton *btnWorkspaceDefault = new QPushButton(tr("Default"));
    connect(btnWorkspaceDefault, SIGNAL(clicked()), this, SLOT(doWorkspaceDefault()));

    QVBoxLayout *layoutWorkspace = new QVBoxLayout();
    layoutWorkspace->addWidget(grpGrid);
    layoutWorkspace->addWidget(grpFont);    
    layoutWorkspace->addStretch();
    layoutWorkspace->addWidget(btnWorkspaceDefault, 0, Qt::AlignLeft);

    QWidget *workspaceWidget = new QWidget();
    workspaceWidget->setLayout(layoutWorkspace);

    return workspaceWidget;
}

void SettingsWidget::setControls()
{

}

void SettingsWidget::updateControls()
{
    load();
}

void SettingsWidget::doApply()
{
    save();

    // time step
    // QApplication::processEvents();

    emit apply();

    activateWindow();
}

void SettingsWidget::doWorkspaceDefault()
{
    chkShowGrid->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowGrid).toBool());

    cmbRulersFont->setCurrentIndex(cmbRulersFont->findData(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_RulersFontFamily).toString()));
    txtRulersFontSizes->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_RulersFontPointSize).toInt());
    cmbPostFont->setCurrentIndex(cmbPostFont->findData(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_PostFontFamily).toString()));
    txtPostFontSizes->setValue(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_PostFontPointSize).toInt());

    chkShowAxes->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowAxes).toBool());
    chkShowRulers->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ShowRulers).toBool());

    chkZoomToMouse->setChecked(Agros2D::problem()->setting()->defaultValue(ProblemSetting::View_ZoomToMouse).toBool());
}

// *******************************************************************************************************

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
    setAutoFillBackground(false);
    setCursor(Qt::PointingHandCursor);
    connect(this, SIGNAL(clicked()), this, SLOT(doClicked()));
}

ColorButton::~ColorButton()
{
}

void ColorButton::setColor(const QColor &color)
{
    m_color = color;
    repaint();
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setPen(m_color);
    painter.setBrush(m_color);
    painter.drawRect(rect());
}

void ColorButton::doClicked()
{
    QColor color = QColorDialog::getColor(m_color);

    if (color.isValid())
    {
        setColor(color);
    }
}
