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

#include "infowidget.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "pythonlabagros.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "ctemplate/template.h"

InfoWidget::InfoWidget(SceneViewPreprocessor *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewGeometry = sceneView;

    setMinimumWidth(160);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    // setWindowFlags(Qt::Popup | Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);

    createActions();

    // problem information
    webView = new QWebView(this);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0, 5, 3, 5);
    layoutMain->addWidget(webView);

    setLayout(layoutMain);

    refresh();
}

/*
int w = 450;
int h = height() - 80;

InfoWidget *popup = new InfoWidget(sceneViewPreprocessor, this);
popup->resize(w, h);
popup->move(pos().x() + 75, pos().y() + 70);
popup->setAttribute(Qt::WA_DeleteOnClose);

popup->show();
*/

InfoWidget::~InfoWidget()
{
}

void InfoWidget::createActions()
{
    actInfo = new QAction(icon("scene-info"), tr("Info"), this);
    actInfo->setShortcut(QKeySequence("Alt+I"));
    actInfo->setCheckable(true);
}

void InfoWidget::refresh()
{
    QTimer::singleShot(0, this, SLOT(showInfo()));
}

void InfoWidget::showInfo()
{
    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize()).toStdString()));

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

    // template
    std::string info;
    ctemplate::TemplateDictionary problem("info");

    problem.SetValue("STYLESHEET", style);
    problem.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

    problem.SetValue("NAME_LABEL", tr("Name:").toStdString());
    problem.SetValue("NAME", Util::problem()->config()->name().toStdString());

    problem.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
    problem.SetValue("COORDINATE_TYPE", coordinateTypeString(Util::problem()->config()->coordinateType()).toStdString());

    problem.SetValue("MESH_TYPE_LABEL", tr("Mesh type:").toStdString());
    problem.SetValue("MESH_TYPE", meshTypeString(Util::problem()->config()->meshType()).toStdString());

    problem.SetValue("FREQUENCY_LABEL", tr("Frequency:").toStdString());
    problem.SetValue("FREQUENCY", QString::number(Util::problem()->config()->frequency()).toStdString() + " Hz");
    problem.SetValue("TIME_STEP_LABEL", tr("Time step:").toStdString());
    problem.SetValue("TIME_STEP", QString::number(Util::problem()->config()->timeStep().number()).toStdString() + " s");
    problem.SetValue("TIME_TOTAL_LABEL", tr("Total time:").toStdString());
    problem.SetValue("TIME_TOTAL", QString::number(Util::problem()->config()->timeTotal().number()).toStdString() + " s");

    if (Util::problem()->isMeshed())
    {
        problem.SetValue("SOLUTION_INFORMATION_LABEL", tr("Mesh and solution informations").toStdString());

        //TODO
        //        problem.SetValue("INITIAL_MESH_LABEL", tr("Initial mesh").toStdString());
        //        problem.SetValue("INITIAL_MESH_NODES_LABEL", tr("Nodes:").toStdString());
        //        problem.SetValue("INITIAL_MESH_NODES", QString::number(Util::problem()->meshInitial()->get_num_nodes()).toStdString());
        //        problem.SetValue("INITIAL_MESH_ELEMENTS_LABEL", tr("Elements:").toStdString());
        //        problem.SetValue("INITIAL_MESH_ELEMENTS", QString::number(Util::problem()->meshInitial()->get_num_active_elements()).toStdString());


        //        if (Util::problem()->isSolved())
        //        {
        //            if (Util::scene()->sceneSolution()->space() && (Util::scene()->sceneSolution()->space()->get_num_dofs() > 0))
        //            {
        //                QTime time = milisecondsToTime(Util::problem()->timeElapsed());
        //                problem.SetValue("ELAPSED_TIME_LABEL", tr("Elapsed time:").toStdString());
        //                problem.SetValue("ELAPSED_TIME", time.toString("mm:ss.zzz").toStdString());

        //                problem.SetValue("DOFS_LABEL", tr("DOFs:").toStdString());
        //                problem.SetValue("DOFS", QString::number(Util::scene()->sceneSolution()->space()->get_num_dofs()).toStdString());
        //            }
        //            problem.ShowSection("SOLUTION_PARAMETERS_SECTION");
        //        }
        problem.ShowSection("SOLUTION_SECTION");
    }

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        ctemplate::TemplateDictionary *field = problem.AddSectionDictionary("FIELD");

        field->SetValue("PHYSICAL_FIELD", fieldInfo->module()->name);

        field->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
        field->SetValue("ANALYSIS_TYPE", analysisTypeString(fieldInfo->analysisType()).toStdString());

        field->SetValue("WEAK_FORMS_TYPE_LABEL", tr("Weak forms:").toStdString());
        field->SetValue("WEAK_FORMS_TYPE", weakFormsTypeString(fieldInfo->weakFormsType).toStdString());

        if (fieldInfo->analysisType() == AnalysisType_Transient)
        {
            field->SetValue("INITIAL_CONDITION_LABEL", tr("Initial condition:").toStdString());
            field->SetValue("INITIAL_CONDITION", QString::number(fieldInfo->initialCondition.number()).toStdString());
            field->ShowSection("INITIAL_CONDITION_SECTION");
        }

        field->SetValue("REFINEMENS_NUMBER_LABEL", tr("Number of refinements:").toStdString());
        field->SetValue("REFINEMENS_NUMBER", QString::number(fieldInfo->numberOfRefinements).toStdString());
        field->SetValue("POLYNOMIAL_ORDER_LABEL", tr("Polynomial order:").toStdString());
        field->SetValue("POLYNOMIAL_ORDER", QString::number(fieldInfo->polynomialOrder).toStdString());

        field->SetValue("ADAPTIVITY_TYPE_LABEL", tr("Adaptivity:").toStdString());
        field->SetValue("ADAPTIVITY_TYPE", adaptivityTypeString(fieldInfo->adaptivityType).toStdString());

        if (fieldInfo->adaptivityType != AdaptivityType_None)
        {
            field->SetValue("ADAPTIVITY_STEPS_LABEL", tr("Steps:").toStdString());
            field->SetValue("ADAPTIVITY_STEPS", QString::number(fieldInfo->adaptivitySteps).toStdString());
            field->SetValue("ADAPTIVITY_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
            field->SetValue("ADAPTIVITY_TOLERANCE", QString::number(fieldInfo->adaptivityTolerance).toStdString());
            field->ShowSection("ADAPTIVITY_PARAMETERS_SECTION");
        }

        field->SetValue("LINEARITY_TYPE_LABEL", tr("Solution type:").toStdString());
        field->SetValue("LINEARITY_TYPE", linearityTypeString(fieldInfo->linearityType).toStdString());

        if (fieldInfo->linearityType != LinearityType_Linear)
        {
            field->SetValue("NONLINEAR_STEPS_LABEL", tr("Steps:").toStdString());
            field->SetValue("NONLINEAR_STEPS", QString::number(fieldInfo->nonlinearSteps).toStdString());
            field->SetValue("NONLINEAR_TOLERANCE_LABEL", tr("Tolerance:").toStdString());
            field->SetValue("NONLINEAR_TOLERANCE", QString::number(fieldInfo->nonlinearTolerance).toStdString());
            field->ShowSection("SOLVER_PARAMETERS_SECTION");
        }

        //        if (Util::problem()->isMeshed())
        //        {
        //            if (Util::problem()->isSolved())
        //            {
        //                if (fieldInfo->adaptivityType != AdaptivityType_None)
        //                {
        //                    problem.SetValue("ADAPTIVITY_LABEL", tr("Adaptivity").toStdString());
        //                    problem.SetValue("ADAPTIVITY_ERROR_LABEL", tr("Error:").toStdString());
        //                    problem.SetValue("ADAPTIVITY_ERROR", QString::number(Util::problem()->adaptiveError(), 'f', 3).toStdString());

        //                    problem.SetValue("SOLUTION_MESH_LABEL", tr("Solution mesh").toStdString());
        //                    problem.SetValue("SOLUTION_MESH_NODES_LABEL", tr("Nodes:").toStdString());
        //                    problem.SetValue("SOLUTION_MESH_NODES", QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_nodes()).toStdString());
        //                    problem.SetValue("SOLUTION_MESH_ELEMENTS_LABEL", tr("Elements:").toStdString());
        //                    problem.SetValue("SOLUTION_MESH_ELEMENTS", QString::number(Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_active_elements()).toStdString());

        //                    problem.ShowSection("ADAPTIVITY_SECTION");
        //                }
        //                problem.ShowSection("SOLUTION_PARAMETERS_SECTION");
        //            }
        //        }
    }

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/problem.tpl", ctemplate::DO_NOT_STRIP, &problem, &info);
    webView->setHtml(QString::fromStdString(info));

    setFocus();
}
