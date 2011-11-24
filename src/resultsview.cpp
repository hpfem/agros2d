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
#include "scenesolution.h"
#include "hermes2d.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"
#include "hermes2d/module.h"
#include "ctemplate/template.h"

ResultsView::ResultsView(QWidget *parent): QDockWidget(tr("Results view"), parent)
{
    logMessage("ResultsView::ResultsView()");

    setMinimumWidth(280);
    setObjectName("ResultsView");

    createActions();

    webView = new QWebView(this);

    btnPoint = new QPushButton();
    btnPoint->setText(actPoint->text());
    btnPoint->setIcon(actPoint->icon());
    btnPoint->setMaximumSize(btnPoint->sizeHint());
    connect(btnPoint, SIGNAL(clicked()), this, SLOT(doPoint()));

    btnSelectMarker = new QPushButton(tr("Select by marker"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(btnPoint);
    btnLayout->addWidget(btnSelectMarker);

    // main widget
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(webView);
    layout->addLayout(btnLayout);
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

    btnPoint->setEnabled(m_sceneModePostprocessor == SceneModePostprocessor_LocalValue);
    btnSelectMarker->setEnabled(Util::scene()->sceneSolution()->isSolved() && m_sceneModePostprocessor != SceneModePostprocessor_LocalValue);
}

void ResultsView::doShowResults()
{
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

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string results;
    ctemplate::TemplateDictionary volumeIntegrals("results");

    volumeIntegrals.SetValue("STYLESHEET", style);
    volumeIntegrals.SetValue("LABEL", tr("Volume integrals").toStdString());

    VolumeIntegralValue volumeIntegralValue;
    for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegralValue.values.begin();
         it != volumeIntegralValue.values.end(); ++it)
    {
        ctemplate::TemplateDictionary *item = volumeIntegrals.AddSectionDictionary("ITEM");
        item->SetValue("NAME", it->first->name);
        item->SetValue("SHORTNAME", it->first->shortname_html);
        item->SetValue("VALUE", QString("%1").arg(it->second, 0, 'e', 3).toStdString());
        item->SetValue("UNIT", it->first->unit_html);
    }

    // expand template
    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/integrals.tpl", ctemplate::DO_NOT_STRIP, &volumeIntegrals, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::doShowSurfaceIntegral()
{
    logMessage("ResultsView::doShowSurfaceIntegral()");

    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string results;
    ctemplate::TemplateDictionary surfaceIntegrals("results");

    surfaceIntegrals.SetValue("STYLESHEET", style);
    surfaceIntegrals.SetValue("LABEL", tr("Surface integrals").toStdString());

    SurfaceIntegralValue surfaceIntegralValue;
    for (std::map<Hermes::Module::Integral *, double>::iterator it = surfaceIntegralValue.values.begin();
         it != surfaceIntegralValue.values.end(); ++it)
    {
        ctemplate::TemplateDictionary *item = surfaceIntegrals.AddSectionDictionary("ITEM");
        item->SetValue("NAME", it->first->name);
        item->SetValue("SHORTNAME", it->first->shortname_html);
        item->SetValue("VALUE", QString("%1").arg(it->second, 0, 'e', 3).toStdString());
        item->SetValue("UNIT", it->first->unit_html);
    }

    // expand template
    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/integrals.tpl", ctemplate::DO_NOT_STRIP, &surfaceIntegrals, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::doShowPoint()
{
    logMessage("ResultsView::doShowPoint()");

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string results;
    ctemplate::TemplateDictionary localPointValues("results");

    localPointValues.SetValue("STYLESHEET", style);
    localPointValues.SetValue("LABEL", tr("Local point values").toStdString());

    localPointValues.SetValue("LABELX", Util::scene()->problemInfo()->labelX().toLower().toStdString());
    localPointValues.SetValue("LABELY", Util::scene()->problemInfo()->labelY().toLower().toStdString());
    localPointValues.SetValue("POINTX", (QString("%1").arg(m_point.x, 0, 'e', 3)).toStdString());
    localPointValues.SetValue("POINTY", (QString("%1").arg(m_point.y, 0, 'e', 3)).toStdString());
    localPointValues.SetValue("POINT_UNIT", "m");

    LocalPointValue value(m_point);
    for (std::map<Hermes::Module::LocalVariable *, PointValue>::iterator it = value.values.begin();
         it != value.values.end(); ++it)
    {
        if (it->first->is_scalar)
        {
            // scalar variable
            ctemplate::TemplateDictionary *item = localPointValues.AddSectionDictionary("ITEM");
            item->SetValue("NAME", it->first->name);
            item->SetValue("SHORTNAME", it->first->shortname_html);
            item->SetValue("VALUE", QString("%1").arg(it->second.scalar, 0, 'e', 3).toStdString());
            item->SetValue("UNIT", it->first->unit_html);
        }
        else
        {
            // vector variable
            ctemplate::TemplateDictionary *itemMagnitude = localPointValues.AddSectionDictionary("ITEM");
            itemMagnitude->SetValue("NAME", it->first->name);
            itemMagnitude->SetValue("SHORTNAME", it->first->shortname_html);
            itemMagnitude->SetValue("VALUE", QString("%1").arg(it->second.vector.magnitude(), 0, 'e', 3).toStdString());
            itemMagnitude->SetValue("UNIT", it->first->unit_html);
            ctemplate::TemplateDictionary *itemX = localPointValues.AddSectionDictionary("ITEM");
            itemX->SetValue("SHORTNAME", it->first->shortname_html);
            itemX->SetValue("PART", Util::scene()->problemInfo()->labelX().toLower().toStdString());
            itemX->SetValue("VALUE", QString("%1").arg(it->second.vector.x, 0, 'e', 3).toStdString());
            itemX->SetValue("UNIT", it->first->unit_html);
            ctemplate::TemplateDictionary *itemY = localPointValues.AddSectionDictionary("ITEM");
            itemY->SetValue("SHORTNAME", it->first->shortname_html);
            itemY->SetValue("PART", Util::scene()->problemInfo()->labelY().toLower().toStdString());
            itemY->SetValue("VALUE", QString("%1").arg(it->second.vector.y, 0, 'e', 3).toStdString());
            itemY->SetValue("UNIT", it->first->unit_html);
        }
    }

    // expand template
    ctemplate::ExpandTemplate(datadir().toStdString() + "/resources/panels/local_point_values.tpl", ctemplate::DO_NOT_STRIP, &localPointValues, &results);
    webView->setHtml(QString::fromStdString(results));
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

    return Point(txtPointX->value().number(), txtPointY->value().number());
}

void LocalPointValueDialog::evaluated(bool isError)
{
    logMessage("LocalPointValueDialog::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}
