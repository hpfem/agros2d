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
#include "hermes2d.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "ctemplate/template.h"

ResultsView::ResultsView(QWidget *parent): QDockWidget(tr("Results view"), parent)
{
    setMinimumWidth(280);
    setObjectName("ResultsView");

    createActions();

    webView = new QWebView(this);

    // main widget
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(webView);
    layout->setContentsMargins(0, 0, 0, 7);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    setWidget(widget);
}

void ResultsView::createActions()
{

}

void ResultsView::doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor)
{
    m_sceneModePostprocessor = sceneModePostprocessor;
}

void ResultsView::doShowResults()
{
    if (m_sceneModePostprocessor == SceneModePostprocessor_LocalValue)
        showPoint();
    if (m_sceneModePostprocessor == SceneModePostprocessor_SurfaceIntegral)
        showSurfaceIntegral();
    if (m_sceneModePostprocessor == SceneModePostprocessor_VolumeIntegral)
        showVolumeIntegral();
}

void ResultsView::showPoint(const Point &point)
{
    if (!Util::problem()->isSolved())
    {
        showEmpty();
        return;
    }

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string results;
    ctemplate::TemplateDictionary localPointValues("results");

    localPointValues.SetValue("STYLESHEET", style);
    localPointValues.SetValue("LABEL", tr("Local point values").toStdString());

    localPointValues.SetValue("LABELX", Util::problem()->config()->labelX().toLower().toStdString());
    localPointValues.SetValue("LABELY", Util::problem()->config()->labelY().toLower().toStdString());
    localPointValues.SetValue("POINTX", (QString("%1").arg(point.x, 0, 'e', 3)).toStdString());
    localPointValues.SetValue("POINTY", (QString("%1").arg(point.y, 0, 'e', 3)).toStdString());
    localPointValues.SetValue("POINT_UNIT", "m");

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        LocalPointValue value(fieldInfo, point);
        if (value.values.size() > 0)
        {
            ctemplate::TemplateDictionary *field = localPointValues.AddSectionDictionary("FIELD");
            field->SetValue("FIELDNAME", fieldInfo->module()->name);

            for (std::map<Hermes::Module::LocalVariable *, PointValue>::iterator it = value.values.begin();
                 it != value.values.end(); ++it)
            {
                if (it->first->is_scalar)
                {
                    // scalar variable
                    ctemplate::TemplateDictionary *item = field->AddSectionDictionary("ITEM");
                    item->SetValue("NAME", it->first->name);
                    item->SetValue("SHORTNAME", it->first->shortname_html);
                    item->SetValue("VALUE", QString("%1").arg(it->second.scalar, 0, 'e', 3).toStdString());
                    item->SetValue("UNIT", it->first->unit_html);
                }
                else
                {
                    // vector variable
                    ctemplate::TemplateDictionary *itemMagnitude = field->AddSectionDictionary("ITEM");
                    itemMagnitude->SetValue("NAME", it->first->name);
                    itemMagnitude->SetValue("SHORTNAME", it->first->shortname_html);
                    itemMagnitude->SetValue("VALUE", QString("%1").arg(it->second.vector.magnitude(), 0, 'e', 3).toStdString());
                    itemMagnitude->SetValue("UNIT", it->first->unit_html);
                    ctemplate::TemplateDictionary *itemX = field->AddSectionDictionary("ITEM");
                    itemX->SetValue("SHORTNAME", it->first->shortname_html);
                    itemX->SetValue("PART", Util::problem()->config()->labelX().toLower().toStdString());
                    itemX->SetValue("VALUE", QString("%1").arg(it->second.vector.x, 0, 'e', 3).toStdString());
                    itemX->SetValue("UNIT", it->first->unit_html);
                    ctemplate::TemplateDictionary *itemY = field->AddSectionDictionary("ITEM");
                    itemY->SetValue("SHORTNAME", it->first->shortname_html);
                    itemY->SetValue("PART", Util::problem()->config()->labelY().toLower().toStdString());
                    itemY->SetValue("VALUE", QString("%1").arg(it->second.vector.y, 0, 'e', 3).toStdString());
                    itemY->SetValue("UNIT", it->first->unit_html);
                }
            }
        }
    }

    // expand template
    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/local_point_values.tpl", ctemplate::DO_NOT_STRIP, &localPointValues, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::showVolumeIntegral()
{
    if (!Util::problem()->isSolved())
    {
        showEmpty();
        return;
    }

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string results;
    ctemplate::TemplateDictionary volumeIntegrals("results");

    volumeIntegrals.SetValue("STYLESHEET", style);
    volumeIntegrals.SetValue("LABEL", tr("Volume integrals").toStdString());

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        VolumeIntegralValue volumeIntegralValue(fieldInfo);
        if (volumeIntegralValue.values.size() > 0)
        {
            ctemplate::TemplateDictionary *field = volumeIntegrals.AddSectionDictionary("FIELD");
            field->SetValue("FIELDNAME", fieldInfo->module()->name);

            for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegralValue.values.begin();
                 it != volumeIntegralValue.values.end(); ++it)
            {
                ctemplate::TemplateDictionary *item = field->AddSectionDictionary("ITEM");
                item->SetValue("NAME", it->first->name);
                item->SetValue("SHORTNAME", it->first->shortname_html);
                item->SetValue("VALUE", QString("%1").arg(it->second, 0, 'e', 3).toStdString());
                item->SetValue("UNIT", it->first->unit_html);
            }
        }
    }

    // expand template
    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/integrals.tpl", ctemplate::DO_NOT_STRIP, &volumeIntegrals, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::showSurfaceIntegral()
{
    if (!Util::problem()->isSolved())
    {
        showEmpty();
        return;
    }

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string results;
    ctemplate::TemplateDictionary surfaceIntegrals("results");

    surfaceIntegrals.SetValue("STYLESHEET", style);
    surfaceIntegrals.SetValue("LABEL", tr("Surface integrals").toStdString());

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        SurfaceIntegralValue surfaceIntegralValue(fieldInfo);
        if (surfaceIntegralValue.values.size() > 0)
        {
            ctemplate::TemplateDictionary *field = surfaceIntegrals.AddSectionDictionary("FIELD");
            field->SetValue("FIELDNAME", fieldInfo->module()->name);

            for (std::map<Hermes::Module::Integral *, double>::iterator it = surfaceIntegralValue.values.begin();
                 it != surfaceIntegralValue.values.end(); ++it)
            {
                ctemplate::TemplateDictionary *item = field->AddSectionDictionary("ITEM");
                item->SetValue("NAME", it->first->name);
                item->SetValue("SHORTNAME", it->first->shortname_html);
                item->SetValue("VALUE", QString("%1").arg(it->second, 0, 'e', 3).toStdString());
                item->SetValue("UNIT", it->first->unit_html);
            }
        }
    }

    // expand template
    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/integrals.tpl", ctemplate::DO_NOT_STRIP, &surfaceIntegrals, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::showEmpty()
{
    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string results;
    ctemplate::TemplateDictionary empty("empty");

    empty.SetValue("STYLESHEET", style);
    empty.SetValue("LABEL", tr("Problem is not solved.").toStdString());

    // expand template
    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/empty.tpl", ctemplate::DO_NOT_STRIP, &empty, &results);
    webView->setHtml(QString::fromStdString(results));
}

LocalPointValueDialog::LocalPointValueDialog(Point point, QWidget *parent) : QDialog(parent)
{
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
    layoutPoint->addRow(Util::problem()->config()->labelX() + " (m):", txtPointX);
    layoutPoint->addRow(Util::problem()->config()->labelY() + " (m):", txtPointY);

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
    return Point(txtPointX->value().number(), txtPointY->value().number());
}

void LocalPointValueDialog::evaluated(bool isError)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}
