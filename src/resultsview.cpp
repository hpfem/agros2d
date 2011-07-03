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

#include "resultsview.h"
#include "scene.h"
#include "gui.h"
#include "scenemarker.h"
#include "hermes2d.h"

ResultsView::ResultsView(QWidget *parent): QDockWidget(tr("Results view"), parent)
{
    logMessage("ResultsView::ResultsView()");

    setMinimumWidth(280);
    setObjectName("ResultsView");

    createActions();

    txtView = new QTextEdit(this);
    txtView->setReadOnly(true);
    txtView->setMinimumSize(160, 160);

    btnPoint = new QPushButton();
    btnPoint->setText(actPoint->text());
    btnPoint->setIcon(actPoint->icon());
    btnPoint->setMaximumSize(btnPoint->sizeHint());
    connect(btnPoint, SIGNAL(clicked()), this, SLOT(doPoint()));

    // main widget
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtView);
    layout->addWidget(btnPoint);
    layout->setContentsMargins(0, 0, 0, 7);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    setWidget(widget);
}

void ResultsView::createActions()
{
    logMessage("ResultsView::createActions()");

    // point
    actPoint = new QAction(icon("scene-node"), tr("Local point value"), this);
    connect(actPoint, SIGNAL(triggered()), this, SLOT(doPoint()));
}

void ResultsView::doPoint()
{
    logMessage("ResultsView::doPoint()");

    LocalPointValueDialog localPointValueDialog(m_point);
    if (localPointValueDialog.exec() == QDialog::Accepted)
    {
        doShowPoint(localPointValueDialog.point());
    }
}

void ResultsView::doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor)
{
    m_sceneModePostprocessor = sceneModePostprocessor;
    txtView->clear();

    btnPoint->setEnabled(m_sceneModePostprocessor == SceneModePostprocessor_LocalValue);
}

void ResultsView::doShowResults()
{
    btnPoint->setEnabled(m_sceneModePostprocessor == SceneModePostprocessor_LocalValue);

    if (m_sceneModePostprocessor == SceneModePostprocessor_LocalValue)
        doShowPoint();
    if (m_sceneModePostprocessor == SceneModePostprocessor_SurfaceIntegral)
        doShowSurfaceIntegral();
    if (m_sceneModePostprocessor == SceneModePostprocessor_VolumeIntegral)
        doShowVolumeIntegral();
}

void ResultsView::doShowPoint(const Point &point)
{
    logMessage("ResultsView::doShowPoint()");

    // store point
    this->m_point = point;
    doShowPoint();
}

void ResultsView::doShowVolumeIntegral()
{
    logMessage("ResultsView::doShowVolumeIntegral()");

    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    // template
    QString content = readFileContent(datadir() + "/doc/report/dock.html");

    QString html = "<h4>" + tr("Volume integral") + "</h4>"
            "<table width=\"100%\">";

    VolumeIntegralValue *volumeIntegralValue = Util::scene()->problemInfo()->module()->volume_integral_value();
    for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegralValue->values.begin(); it != volumeIntegralValue->values.end(); ++it)
    {
        html += "<tr>"
                "<td><b>" + QString::fromStdString(it->first->name) + "</b></td>"
                "<td><i>" + QString::fromStdString(it->first->shortname) + "</i></td>"
                "<td align=\"right\">" + QString("%1").arg(it->second, 0, 'e', 3) + "</td>"
                "<td>" + unitToHTML(QString::fromStdString(it->first->unit)) + "</td>"
                "</tr>";
    }

    html += "</table>";

    content.replace("[Body]", html);
    txtView->setText(content);
}

void ResultsView::doShowSurfaceIntegral()
{
    logMessage("ResultsView::doShowSurfaceIntegral()");

    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    // template
    QString content = readFileContent(datadir() + "/doc/report/dock.html");

    QString html = "<h4>" + tr("Surface integral") + "</h4>"
            "<table width=\"100%\">";

    SurfaceIntegralValue *surfaceIntegralValue = Util::scene()->problemInfo()->module()->surface_integral_value();
    for (std::map<Hermes::Module::Integral *, double>::iterator it = surfaceIntegralValue->values.begin(); it != surfaceIntegralValue->values.end(); ++it)
    {
        html += "<tr>"
                "<td><b>" + QString::fromStdString(it->first->name) + "</b></td>"
                "<td><i>" + QString::fromStdString(it->first->shortname) + "</i></td>"
                "<td align=\"right\">" + QString("%1").arg(it->second, 0, 'e', 3) + "</td>"
                "<td>" + unitToHTML(QString::fromStdString(it->first->unit)) + "</td>"
                "</tr>";
    }

    html += "</table>";

    content.replace("[Body]", html);
    txtView->setText(content);
}

void ResultsView::doShowPoint()
{
    logMessage("ResultsView::doShowPoint()");

    // template
    QString content = readFileContent(datadir() + "/doc/report/dock.html");

    QString html = "<h4>" + tr("Local point values") + "</h4>"
            "<table width=\"100%\">";

    html += "<tr>"
            "<td><b>Point</b></td>"
            "<td><i>" + Util::scene()->problemInfo()->labelX().toLower() + "</i></td>"
            "<td align=\"right\">" + QString("%1").arg(m_point.x, 0, 'e', 3) + "</td>"
            "<td>&nbsp;</td>"
            "</tr>";
    html += "<tr>"
            "<td>&nbsp;</td>"
            "<td><i>" + Util::scene()->problemInfo()->labelY().toLower() + "</i></td>"
            "<td align=\"right\">" + QString("%1").arg(m_point.y, 0, 'e', 3) + "</td>"
            "<td>&nbsp;</td>"
            "</tr>";
    html += "<tr>"
            "<td>&nbsp;</td>"
            "<td>&nbsp;</td>"
            "<td>&nbsp;</td>"
            "<td>&nbsp;</td>"
            "</tr>";

    if (Util::scene()->sceneSolution()->isSolved())
    {
        LocalPointValue *value = Util::scene()->problemInfo()->module()->local_point_value(m_point);
        for (std::map<Hermes::Module::LocalVariable *, PointValue>::iterator it = value->values.begin(); it != value->values.end(); ++it)
        {
            if (it->first->is_scalar)
            {
                // scalar variable
                html += "<tr>"
                        "<td><b>" + QString::fromStdString(it->first->name) + "</b></td>"
                        "<td><i>" + QString::fromStdString(it->first->shortname) + "</i></td>"
                        "<td align=\"right\">" + QString("%1").arg(it->second.scalar, 0, 'e', 3) + "</td>"
                        "<td>" + unitToHTML(QString::fromStdString(it->first->unit)) + "</td>"
                        "</tr>";
            }
            else
            {
                // vector variable
                html += "<tr>"
                        "<td><b>" + QString::fromStdString(it->first->name) + "</b></td>"
                        "<td><i>" + QString::fromStdString(it->first->shortname) + "</i></td>"
                        "<td align=\"right\">" + QString("%1").arg(it->second.vector.magnitude(), 0, 'e', 3) + "</td>"
                        "<td>" + unitToHTML(QString::fromStdString(it->first->unit)) + "</td>"
                        "</tr>";
                html += "<tr>"
                        "<td>&nbsp;</td>"
                        "<td><i>" + QString::fromStdString(it->first->shortname) + "</i><sub>" + Util::scene()->problemInfo()->labelX().toLower() + "</sub></td>"
                        "<td align=\"right\">" + QString("%1").arg(it->second.vector.x, 0, 'e', 3) + "</td>"
                        "<td>" + unitToHTML(QString::fromStdString(it->first->unit)) + "</td>"
                        "</tr>";
                html += "<tr>"
                        "<td>&nbsp;</td>"
                        "<td><i>" + QString::fromStdString(it->first->shortname) + "</i><sub>" + Util::scene()->problemInfo()->labelY().toLower() + "</sub></td>"
                        "<td align=\"right\">" + QString("%1").arg(it->second.vector.y, 0, 'e', 3) + "</td>"
                        "<td>" + unitToHTML(QString::fromStdString(it->first->unit)) + "</td>"
                        "</tr>";
            }
        }
    }

    html += "</table>";

    content.replace("[Body]", html);
    txtView->setText(content);
}

LocalPointValueDialog::LocalPointValueDialog(Point point, QWidget *parent) : QDialog(parent)
{
    logMessage("LocalPointValueDialog::LocalPointValueDialog()");

    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Local point value"));

    setModal(true);

    txtPointX = new ValueLineEdit();
    txtPointX->setNumber(point.x);
    txtPointY = new ValueLineEdit();
    txtPointY->setNumber(point.y);

    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QFormLayout *layoutPoint = new QFormLayout();
    layoutPoint->addRow(Util::scene()->problemInfo()->labelX() + " (m):", txtPointX);
    layoutPoint->addRow(Util::scene()->problemInfo()->labelY() + " (m):", txtPointY);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutPoint);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

Point LocalPointValueDialog::point()
{
    logMessage("LocalPointValueDialog::point()");

    return Point(txtPointX->value().number, txtPointY->value().number);
}

void LocalPointValueDialog::evaluated(bool isError)
{
    logMessage("LocalPointValueDialog::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}
