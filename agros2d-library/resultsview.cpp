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

#include "util/constants.h"
#include "util/global.h"

#include "gui/common.h"
#include "gui/valuelineedit.h"

#include "scene.h"
#include "hermes2d.h"
#include "hermes2d/plugin_interface.h"
#include "hermes2d/module.h"
#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "sceneview_post.h"

#include <ctemplate/template.h>

class CustomVolumetricIntegralCalculator : public Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>
{
public:
    CustomVolumetricIntegralCalculator(Hermes::Hermes2D::MeshFunctionSharedPtr<double> source_function, int number_of_integrals)
        : Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>(source_function, number_of_integrals)
    {
    }

    CustomVolumetricIntegralCalculator(Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > source_functions, int number_of_integrals)
        : Hermes::Hermes2D::PostProcessing::VolumetricIntegralCalculator<double>(source_functions, number_of_integrals)
    {
    }

    virtual void integral(int n, double* wt, Hermes::Hermes2D::Func<double> **fns, Hermes::Hermes2D::Geom<double> *e, double* result)
    {
        for (int i = 0; i < n; i++)
        {
            result[0] += wt[i]; // * fns[0]->val[i];
            result[1] += wt[i] * 2 * M_PI * e->x[i];
        }
    }

    virtual void order(Hermes::Hermes2D::Func<Hermes::Ord> **fns, Hermes::Ord* result) {
        result[0] = Hermes::Ord(21);
    }
};


ResultsView::ResultsView(PostHermes *postHermes, QWidget *parent)
    : QDockWidget(tr("Results view"), parent), m_postHermes(postHermes)
{
    setObjectName("ResultsView");

    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());

    createActions();

    connect(postHermes, SIGNAL(processed()), this, SLOT(doShowResults()));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize() - 1).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_results.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

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
    QAction *copyAction = webView->pageAction(QWebPage::Copy);
    copyAction->setShortcut(QKeySequence::Copy);
    webView->addAction(copyAction);
}

void ResultsView::doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor)
{
    m_sceneModePostprocessor = sceneModePostprocessor;

    doShowResults();
}

void ResultsView::doShowResults()
{
    if (m_sceneModePostprocessor == SceneModePostprocessor_Empty)
        showEmpty();
    if (m_sceneModePostprocessor == SceneModePostprocessor_LocalValue)
        showPoint();
    if (m_sceneModePostprocessor == SceneModePostprocessor_SurfaceIntegral)
        showSurfaceIntegral();
    if (m_sceneModePostprocessor == SceneModePostprocessor_VolumeIntegral)
        showVolumeIntegral();
}

void ResultsView::showPoint(const Point &point)
{
    m_point = point;

    showPoint();
}

void ResultsView::showPoint()
{
    if (!(Agros2D::problem()->isSolved() && m_postHermes->isProcessed()))
    {
        showNotSolved();
        return;
    }

    // template
    std::string results;
    ctemplate::TemplateDictionary localPointValues("results");

    localPointValues.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    localPointValues.SetValue("LABEL", tr("Local point values").toStdString());

    localPointValues.SetValue("LABELX", QString("<i>%1</i>").arg(Agros2D::problem()->config()->labelX().toLower()).toStdString());
    localPointValues.SetValue("LABELY", QString("<i>%1</i>").arg(Agros2D::problem()->config()->labelY().toLower()).toStdString());
    localPointValues.SetValue("POINTX", (QString("%1").arg(m_point.x, 0, 'e', 3)).toStdString());
    localPointValues.SetValue("POINTY", (QString("%1").arg(m_point.y, 0, 'e', 3)).toStdString());
    localPointValues.SetValue("POINT_UNIT", "m");

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        LocalValue *value = fieldInfo->plugin()->localValue(fieldInfo,
                                                            m_postHermes->activeTimeStep(),
                                                            m_postHermes->activeAdaptivityStep(),
                                                            m_postHermes->activeAdaptivitySolutionType(),
                                                            m_point);
        QMap<QString, PointValue> values = value->values();
        delete value;

        if (values.size() > 0)
        {
            ctemplate::TemplateDictionary *field = localPointValues.AddSectionDictionary("FIELD");
            field->SetValue("FIELDNAME", fieldInfo->name().toStdString());

            foreach (Module::LocalVariable variable, fieldInfo->localPointVariables())
            {
                if (variable.isScalar())
                {
                    // scalar variable
                    ctemplate::TemplateDictionary *item = field->AddSectionDictionary("ITEM");
                    item->SetValue("NAME", variable.name().toStdString());
                    item->SetValue("SHORTNAME", variable.shortnameHtml().toStdString());
                    item->SetValue("VALUE", QString("%1").arg(values[variable.id()].scalar, 0, 'e', 3).toStdString());
                    item->SetValue("UNIT", variable.unitHtml().toStdString());
                }
                else
                {
                    // vector variable
                    ctemplate::TemplateDictionary *itemMagnitude = field->AddSectionDictionary("ITEM");
                    itemMagnitude->SetValue("NAME", variable.name().toStdString());
                    itemMagnitude->SetValue("SHORTNAME", variable.shortnameHtml().toStdString());
                    itemMagnitude->SetValue("VALUE", QString("%1").arg(values[variable.id()].vector.magnitude(), 0, 'e', 3).toStdString());
                    itemMagnitude->SetValue("UNIT", variable.unitHtml().toStdString());
                    ctemplate::TemplateDictionary *itemX = field->AddSectionDictionary("ITEM");
                    itemX->SetValue("SHORTNAME", variable.shortnameHtml().toStdString());
                    itemX->SetValue("PART", Agros2D::problem()->config()->labelX().toLower().toStdString());
                    itemX->SetValue("VALUE", QString("%1").arg(values[variable.id()].vector.x, 0, 'e', 3).toStdString());
                    itemX->SetValue("UNIT", variable.unitHtml().toStdString());
                    ctemplate::TemplateDictionary *itemY = field->AddSectionDictionary("ITEM");
                    itemY->SetValue("SHORTNAME", variable.shortnameHtml().toStdString());
                    itemY->SetValue("PART", Agros2D::problem()->config()->labelY().toLower().toStdString());
                    itemY->SetValue("VALUE", QString("%1").arg(values[variable.id()].vector.y, 0, 'e', 3).toStdString());
                    itemY->SetValue("UNIT", variable.unitHtml().toStdString());
                }
            }
        }
    }

    // expand template
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/local_point_values.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &localPointValues, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::showVolumeIntegral()
{
    if (!Agros2D::problem()->isSolved())
    {
        showNotSolved();
        return;
    }

    // template
    std::string results;
    ctemplate::TemplateDictionary volumeIntegrals("results");

    volumeIntegrals.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    volumeIntegrals.SetValue("LABEL", tr("Volume integrals").toStdString());

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        // TODO: dummy calc - remove
        CustomVolumetricIntegralCalculator calc(Hermes::Hermes2D::MeshFunctionSharedPtr<double>(), 0);

        IntegralValue *integral = fieldInfo->plugin()->volumeIntegral(fieldInfo,
                                                                      m_postHermes->activeTimeStep(),
                                                                      m_postHermes->activeAdaptivityStep(),
                                                                      m_postHermes->activeAdaptivitySolutionType());
        QMap<QString, double> values = integral->values();
        if (values.size() > 0)
        {
            ctemplate::TemplateDictionary *field = volumeIntegrals.AddSectionDictionary("FIELD");
            field->SetValue("FIELDNAME", fieldInfo->name().toStdString());

            foreach (Module::Integral integral, fieldInfo->volumeIntegrals())
            {
                ctemplate::TemplateDictionary *item = field->AddSectionDictionary("ITEM");
                item->SetValue("NAME", integral.name().toStdString());
                item->SetValue("SHORTNAME", integral.shortnameHtml().toStdString());
                item->SetValue("VALUE", QString("%1").arg(values[integral.id()], 0, 'e', 3).toStdString());
                item->SetValue("UNIT", integral.unitHtml().toStdString());
            }
        }
        delete integral;
    }

    // expand template
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/integrals.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &volumeIntegrals, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::showSurfaceIntegral()
{
    if (!Agros2D::problem()->isSolved())
    {
        showNotSolved();
        return;
    }

    // template
    std::string results;
    ctemplate::TemplateDictionary surfaceIntegrals("results");

    surfaceIntegrals.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    surfaceIntegrals.SetValue("LABEL", tr("Surface integrals").toStdString());

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        IntegralValue *integral = fieldInfo->plugin()->surfaceIntegral(fieldInfo,
                                                                       m_postHermes->activeTimeStep(),
                                                                       m_postHermes->activeAdaptivityStep(),
                                                                       m_postHermes->activeAdaptivitySolutionType());
        QMap<QString, double> values = integral->values();
        {
            ctemplate::TemplateDictionary *field = surfaceIntegrals.AddSectionDictionary("FIELD");
            field->SetValue("FIELDNAME", fieldInfo->name().toStdString());

            foreach (Module::Integral integral, fieldInfo->surfaceIntegrals())
            {
                ctemplate::TemplateDictionary *item = field->AddSectionDictionary("ITEM");
                item->SetValue("NAME", integral.name().toStdString());
                item->SetValue("SHORTNAME", integral.shortnameHtml().toStdString());
                item->SetValue("VALUE", QString("%1").arg(values[integral.id()], 0, 'e', 3).toStdString());
                item->SetValue("UNIT", integral.unitHtml().toStdString());
            }
        }
        delete integral;
    }

    // expand template
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/integrals.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &surfaceIntegrals, &results);
    webView->setHtml(QString::fromStdString(results));
}

void ResultsView::showEmpty()
{
    webView->setHtml("");
}

void ResultsView::showNotSolved()
{
    // template
    std::string results;
    ctemplate::TemplateDictionary empty("empty");

    empty.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    empty.SetValue("LABEL", tr("Problem is not solved.").toStdString());

    // expand template
    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/empty.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &empty, &results);
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
    layoutPoint->addRow(Agros2D::problem()->config()->labelX() + " (m):", txtPointX);
    layoutPoint->addRow(Agros2D::problem()->config()->labelY() + " (m):", txtPointY);

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
