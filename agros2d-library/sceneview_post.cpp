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

#include "sceneview_post.h"

#include "util/global.h"

#include "sceneview_common2d.h"
#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "logview.h"

#include "hermes2d/plugin_interface.h"
#include "hermes2d/module.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutiontypes.h"
#include "hermes2d/solutionstore.h"

PostHermes::PostHermes() :
    m_activeViewField(NULL), m_activeTimeStep(NOT_FOUND_SO_FAR), m_activeAdaptivityStep(NOT_FOUND_SO_FAR), m_activeSolutionMode(SolutionMode_Undefined), m_isProcessed(false)
{
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));
    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), this, SLOT(clear()));

    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(problemMeshed()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(problemSolved()));
}

PostHermes::~PostHermes()
{
    clear();
}

void PostHermes::processInitialMesh()
{
    if (Agros2D::problem()->isMeshed() && (m_activeViewField) && (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowInitialMeshView).toBool()))
    {
        Agros2D::log()->printMessage(tr("Mesh View"), tr("Initial mesh with %1 elements").arg(m_activeViewField->initialMesh()->get_num_active_elements()));

        // init linearizer for initial mesh
        try
        {
            m_linInitialMeshView.free();
            m_linInitialMeshView.process_solution(MeshFunctionSharedPtr<double>(new Hermes::Hermes2D::ZeroSolution<double>(m_activeViewField->initialMesh())));
        }
        catch (Hermes::Exceptions::Exception& e)
        {
            Agros2D::log()->printError("Mesh View", QObject::tr("Linearizer processing failed: %1").arg(e.what()));
        }
    }
}

void PostHermes::processSolutionMesh()
{
    if ((Agros2D::problem()->isSolved()) && (m_activeViewField) && (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowSolutionMeshView).toBool()))
    {
        // ERROR: FIX component(0)
        Agros2D::log()->printMessage(tr("Mesh View"), tr("Solution mesh with %1 elements").arg(activeMultiSolutionArray().solutions().at(0)->get_mesh()->get_num_active_elements()));

        // init linearizer for solution mesh
        // ERROR: FIX component(0)
        const MeshSharedPtr mesh = activeMultiSolutionArray().solutions().at(0)->get_mesh();

        m_linSolutionMeshView.free();
        m_linSolutionMeshView.process_solution(MeshFunctionSharedPtr<double>(new Hermes::Hermes2D::ZeroSolution<double>(mesh)));
    }
}

void PostHermes::processOrder()
{
    // init linearizer for order view
    if ((Agros2D::problem()->isSolved()) && (m_activeViewField) && (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderView).toBool()))
    {
        Agros2D::log()->printMessage(tr("Mesh View"), tr("Polynomial order"));

        // ERROR: FIX component(0)
        m_orderView.free();
        m_orderView.process_space(activeMultiSolutionArray().spaces().at(0));
    }
}

void PostHermes::processRangeContour()
{
    if (Agros2D::problem()->isSolved() && m_activeViewField && (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowContourView).toBool()))
    {
        bool contains = false;
        foreach (Module::LocalVariable variable, m_activeViewField->viewScalarVariables())
        {
            if (variable.id() == Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString())
            {
                contains = true;
                break;
            }
        }

        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString() == "" || !contains)
        {
            // default values
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_ContourVariable, m_activeViewField->defaultViewScalarVariable().id());
        }

        Agros2D::log()->printMessage(tr("Post View"), tr("Contour view (%1)").arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString()));

        QString variableName = Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString();
        Module::LocalVariable variable = m_activeViewField->localVariable(variableName);

        Hermes::Hermes2D::Filter<double> *slnContourView = NULL;
        if (variable.isScalar())
            slnContourView = viewScalarFilter(m_activeViewField->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString()),
                                              PhysicFieldVariableComp_Scalar);
        else
            slnContourView = viewScalarFilter(m_activeViewField->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString()),
                                              PhysicFieldVariableComp_Magnitude);

        m_linContourView.free();

        // deformed shape
        if (m_activeViewField->hasDeformableShape() && Agros2D::problem()->setting()->value(ProblemSetting::View_DeformContour).toBool())
        {
            Hermes::Hermes2D::MagFilter<double> *filter = new Hermes::Hermes2D::MagFilter<double>(Hermes::vector<MeshFunctionSharedPtr<double> >(activeMultiSolutionArray().solutions().at(0),
                                                                                                                                                 activeMultiSolutionArray().solutions().at(1)));

            RectPoint rect = Agros2D::scene()->boundingBox();
            double dmult = qMax(rect.width(), rect.height()) / filter->get_approx_max_value() / 15.0;

            m_linContourView.set_displacement(activeMultiSolutionArray().solutions().at(0),
                                              activeMultiSolutionArray().solutions().at(1),
                                              dmult);
            delete filter;
        }
        else
        {
            m_linContourView.set_displacement(NULL, NULL);
        }

        // process solution
        m_linContourView.process_solution(slnContourView,
                                          Hermes::Hermes2D::H2D_FN_VAL_0,
                                          paletteQualityToDouble((PaletteQuality) Agros2D::problem()->setting()->value(ProblemSetting::View_LinearizerQuality).toInt()));
    }
}

void PostHermes::processRangeScalar()
{
    if ((Agros2D::problem()->isSolved()) && (m_activeViewField)
            && ((Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool())
                || (((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()) == SceneViewPost3DMode_ScalarView3D)))
    {
        bool contains = false;
        foreach (Module::LocalVariable variable, m_activeViewField->viewScalarVariables())
        {
            if (variable.id() == Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString())
            {
                contains = true;
                break;
            }
        }

        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString() == "" || !contains)
        {
            // default values
            Module::LocalVariable variable = m_activeViewField->defaultViewScalarVariable();
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariable, variable.id());
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, variable.isScalar() ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude);
        }

        Agros2D::log()->printMessage(tr("Post View"), tr("Scalar view (%1)").arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString()));

        Hermes::Hermes2D::Filter<double> *slnScalarView = viewScalarFilter(m_activeViewField->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString()),
                                                                           (PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt());

        // QTime time;
        // time.start();
        // qDebug() << "process scalar: start";
        m_linScalarView.free();

        // deformed shape
        if (m_activeViewField->hasDeformableShape() && Agros2D::problem()->setting()->value(ProblemSetting::View_DeformScalar).toBool())
        {
            Hermes::Hermes2D::MagFilter<double> *filter = new Hermes::Hermes2D::MagFilter<double>(Hermes::vector<MeshFunctionSharedPtr<double> >(activeMultiSolutionArray().solutions().at(0),
                                                                                                                                                 activeMultiSolutionArray().solutions().at(1)));

            RectPoint rect = Agros2D::scene()->boundingBox();
            double dmult = qMax(rect.width(), rect.height()) / filter->get_approx_max_value() / 15.0;

            m_linScalarView.set_displacement(activeMultiSolutionArray().solutions().at(0),
                                             activeMultiSolutionArray().solutions().at(1),
                                             dmult);
            delete filter;
        }
        else
        {
            m_linScalarView.set_displacement(NULL, NULL);
        }

        // process solution
        m_linScalarView.process_solution(slnScalarView,
                                         Hermes::Hermes2D::H2D_FN_VAL_0,
                                         paletteQualityToDouble((PaletteQuality) Agros2D::problem()->setting()->value(ProblemSetting::View_LinearizerQuality).toInt()));

        // qDebug() << "process scalar: " << time.elapsed();

        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
        {
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeMin, m_linScalarView.get_min_value());
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_ScalarRangeMax, m_linScalarView.get_max_value());

            // Adjust the min-max if they are very close to each other.
            /*
            double maxAbsValue = std::max(std::abs(Agros2D::problem()->configView()->scalarRangeMax), std::abs(Agros2D::problem()->configView()->scalarRangeMin));
            double distanceBetweenMinMax = Agros2D::problem()->configView()->scalarRangeMax - Agros2D::problem()->configView()->scalarRangeMin;

            // Let us keep this above 1%.
            if(distanceBetweenMinMax / maxAbsValue < 1e-2)
            {
                // And extend the distance symmetrically.
                double middleValue = (Agros2D::problem()->configView()->scalarRangeMin + Agros2D::problem()->configView()->scalarRangeMax) / 2.;
                Agros2D::problem()->configView()->scalarRangeMin = middleValue - 0.5 * 1e-2 * maxAbsValue;
                Agros2D::problem()->configView()->scalarRangeMax = middleValue + 0.5 * 1e-2 * maxAbsValue;
            }
            */
        }
    }
}

void PostHermes::processRangeVector()
{
    if ((Agros2D::problem()->isSolved()) && (m_activeViewField) && (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowVectorView).toBool()))
    {
        bool contains = false;
        foreach (Module::LocalVariable variable, m_activeViewField->viewVectorVariables())
        {
            if (variable.id() == Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString())
            {
                contains = true;
                break;
            }
        }

        if (Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString() == "" || !contains)
        {
            // default values
            Agros2D::problem()->setting()->setValue(ProblemSetting::View_VectorVariable, m_activeViewField->defaultViewVectorVariable().id());
        }

        Agros2D::log()->printMessage(tr("Post View"), tr("Vector view (%1)").arg(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString()));

        Hermes::Hermes2D::Filter<double> *slnVectorXView = viewScalarFilter(m_activeViewField->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString()),
                                                                            PhysicFieldVariableComp_X);

        Hermes::Hermes2D::Filter<double> *slnVectorYView = viewScalarFilter(m_activeViewField->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString()),
                                                                            PhysicFieldVariableComp_Y);

        m_vecVectorView.free();

        // deformed shape
        if (m_activeViewField->hasDeformableShape() && Agros2D::problem()->setting()->value(ProblemSetting::View_DeformVector).toBool())
        {
            Hermes::Hermes2D::MagFilter<double> *filter = new Hermes::Hermes2D::MagFilter<double>(Hermes::vector<MeshFunctionSharedPtr<double> >(activeMultiSolutionArray().solutions().at(0),
                                                                                                                                                 activeMultiSolutionArray().solutions().at(1)));

            RectPoint rect = Agros2D::scene()->boundingBox();
            double dmult = qMax(rect.width(), rect.height()) / filter->get_approx_max_value() / 15.0;

            m_vecVectorView.set_displacement(activeMultiSolutionArray().solutions().at(0),
                                             activeMultiSolutionArray().solutions().at(1),
                                             dmult);
            delete filter;
        }
        else
        {
            m_vecVectorView.set_displacement(NULL, NULL);
        }

        // process solution
        m_vecVectorView.process_solution(slnVectorXView, slnVectorYView,
                                         Hermes::Hermes2D::H2D_FN_VAL_0, Hermes::Hermes2D::H2D_FN_VAL_0,
                                         Hermes::Hermes2D::Views::HERMES_EPS_LOW);
    }
}

void PostHermes::refresh()
{
    m_isProcessed = false;

    m_linInitialMeshView.free();
    m_linSolutionMeshView.free();
    m_orderView.free();

    m_linContourView.free();
    m_linScalarView.free();
    m_vecVectorView.free();

    if (Agros2D::problem()->isMeshed())
        processMeshed();

    if (Agros2D::problem()->isSolved())
        processSolved();

    m_isProcessed = true;
    emit processed();
}

void PostHermes::clear()
{
    m_isProcessed = false;

    m_linInitialMeshView.free();
    m_linSolutionMeshView.free();
    m_orderView.free();

    m_linContourView.free();
    m_linScalarView.free();
    m_vecVectorView.free();

    m_activeViewField = NULL;
    m_activeTimeStep = NOT_FOUND_SO_FAR;
    m_activeAdaptivityStep = NOT_FOUND_SO_FAR;
    m_activeSolutionMode = SolutionMode_Undefined;
}

void PostHermes::problemMeshed()
{
    if (!m_activeViewField)
    {
        setActiveViewField(Agros2D::problem()->fieldInfos().begin().value());
    }
    //    if (m_activeTimeStep == NOT_FOUND_SO_FAR)
    //    {
    //        setActiveTimeStep(0);
    //    }
    //    if (m_activeAdaptivityStep == NOT_FOUND_SO_FAR)
    //    {
    //        setActiveAdaptivityStep(0);
    //        setActiveAdaptivitySolutionType(SolutionMode_Normal);
    //    }
}

void PostHermes::problemSolved()
{
    if (!m_activeViewField)
    {
        setActiveViewField(Agros2D::problem()->fieldInfos().begin().value());
    }

    // time step
    int lastTimeStep = Agros2D::solutionStore()->lastTimeStep(m_activeViewField, SolutionMode_Normal);

    if (m_activeTimeStep == NOT_FOUND_SO_FAR || activeTimeStep() > lastTimeStep)
    {
        setActiveTimeStep(lastTimeStep);
    }

    // adaptive step
    int lastAdaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(m_activeViewField, SolutionMode_Normal, m_activeTimeStep);

    setActiveAdaptivityStep(lastAdaptivityStep);
    setActiveAdaptivitySolutionType(SolutionMode_Normal);
}

void PostHermes::processMeshed()
{
    processInitialMesh();
}

void PostHermes::processSolved()
{
    if (activeMultiSolutionArray().spaces().empty())
        return;
    if (activeMultiSolutionArray().solutions().empty())
        return;

    // temporary use 3/4 of max threads
    int threads = omp_get_max_threads() * 3/4;
    if (threads == 0)
        threads = 1;

    Hermes::Hermes2D::Hermes2DApi.set_integral_param_value(Hermes::Hermes2D::numThreads, threads);

    processSolutionMesh();
    processOrder();

    processRangeContour();
    processRangeScalar();
    processRangeVector();

    // restore settings
    Hermes::Hermes2D::Hermes2DApi.set_integral_param_value(Hermes::Hermes2D::numThreads, Agros2D::configComputer()->numberOfThreads);
}

Hermes::Hermes2D::Filter<double> *PostHermes::viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                                               PhysicFieldVariableComp physicFieldVariableComp)
{
    // update time functions
    /*
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        m_fieldInfo->module()->update_time_functions(Agros2D::problem()->time());
    */

    Hermes::vector<MeshFunctionSharedPtr<double> > slns;
    for (int k = 0; k < activeViewField()->numberOfSolutions(); k++)
        slns.push_back(activeMultiSolutionArray().solutions().at(k));

    // qDebug() << "viewScalarFilter: " << activeViewField()->fieldId() << activeTimeStep() << activeAdaptivityStep() << activeAdaptivitySolutionType();

    return activeViewField()->plugin()->filter(activeViewField(),
                                               activeTimeStep(),
                                               activeAdaptivityStep(),
                                               activeAdaptivitySolutionType(),
                                               slns,
                                               physicFieldVariable.id(),
                                               physicFieldVariableComp);
}


void PostHermes::setActiveViewField(FieldInfo* fieldInfo)
{
    // previous active field
    FieldInfo* previousActiveViewField = m_activeViewField;

    // set new field
    m_activeViewField = fieldInfo;

    // check for different field
    if (previousActiveViewField != fieldInfo)
    {
        setActiveTimeStep(NOT_FOUND_SO_FAR);
        setActiveAdaptivityStep(NOT_FOUND_SO_FAR);
        setActiveAdaptivitySolutionType(SolutionMode_Normal);
    }
}

void PostHermes::setActiveTimeStep(int ts)
{
    m_activeTimeStep = ts;
}

void PostHermes::setActiveAdaptivityStep(int as)
{
    m_activeAdaptivityStep = as;
}

MultiArray<double> PostHermes::activeMultiSolutionArray()
{
    FieldSolutionID fsid(activeViewField(), activeTimeStep(), activeAdaptivityStep(), activeAdaptivitySolutionType());
    return Agros2D::solutionStore()->multiArray(fsid);
}

// ************************************************************************************************

SceneViewPostInterface::SceneViewPostInterface(PostHermes *postHermes, QWidget *parent)
    : SceneViewCommon(parent),
      m_postHermes(postHermes),
      m_textureScalar(0)
{
}

void SceneViewPostInterface::initializeGL()
{
    clearGLLists();

    SceneViewCommon::initializeGL();
}

const QVector3D SceneViewPostInterface::paletteColor2(const int pos) const
{
    int n = (int) (pos / (PALETTEENTRIES / Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteSteps).toInt()))
            * (PALETTEENTRIES / Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteSteps).toInt());

    if (n < 0)
        n = 0;
    else if (n > PALETTEENTRIES - 1)
        n = PALETTEENTRIES - 1;

    const double *colors = NULL;
    switch ((PaletteType) Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteType).toInt())
    {
    case Palette_Agros2D:
        colors = paletteDataAgros2D[n];
    }

    assert(colors);
    return QVector3D(colors[0], colors[1], colors[2]);
}

const double* SceneViewPostInterface::paletteColor(double x) const
{
    switch ((PaletteType) Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteType).toInt())
    {
    case Palette_Agros2D:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataAgros2D[n];
    }
        break;
    case Palette_Jet:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataJet[n];
    }
        break;
    case Palette_Copper:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataCopper[n];
    }
        break;
    case Palette_Hot:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataHot[n];
    }
        break;
    case Palette_Cool:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataCool[n];
    }
        break;
    case Palette_Bone:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataBone[n];
    }
        break;
    case Palette_Pink:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataPink[n];
    }
        break;
    case Palette_Spring:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataSpring[n];
    }
        break;
    case Palette_Summer:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataSummer[n];
    }
        break;
    case Palette_Autumn:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataAutumn[n];
    }
        break;
    case Palette_Winter:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataWinter[n];
    }
        break;
    case Palette_HSV:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataHSV[n];
    }
        break;
    case Palette_BWAsc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = x;
        return color;
    }
        break;
    case Palette_BWDesc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = 1.0 - x;
        return color;
    }
        break;
    default:
        qWarning() << QString("Undefined: %1.").arg(((PaletteType) Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteType).toInt()));
        return NULL;
    }
}

const double* SceneViewPostInterface::paletteColorOrder(int n) const
{
    switch ((PaletteOrderType) Agros2D::problem()->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt())
    {
    case PaletteOrder_Hermes:
        return paletteOrderHermes[n];
    case PaletteOrder_Jet:
        return paletteOrderJet[n];
    case PaletteOrder_Copper:
        return paletteOrderCopper[n];
    case PaletteOrder_Hot:
        return paletteOrderHot[n];
    case PaletteOrder_Cool:
        return paletteOrderCool[n];
    case PaletteOrder_Bone:
        return paletteOrderBone[n];
    case PaletteOrder_Pink:
        return paletteOrderPink[n];
    case PaletteOrder_Spring:
        return paletteOrderSpring[n];
    case PaletteOrder_Summer:
        return paletteOrderSummer[n];
    case PaletteOrder_Autumn:
        return paletteOrderAutumn[n];
    case PaletteOrder_Winter:
        return paletteOrderWinter[n];
    case PaletteOrder_HSV:
        return paletteOrderHSV[n];
    case PaletteOrder_BWAsc:
        return paletteOrderBWAsc[n];
    case PaletteOrder_BWDesc:
        return paletteOrderBWDesc[n];
    default:
        qWarning() << QString("Undefined: %1.").arg(Agros2D::problem()->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt());
        return NULL;
    }
}

void SceneViewPostInterface::paletteCreate()
{
    int paletteSteps = Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteFilter).toBool()
            ? 100 : Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteSteps).toInt();

    unsigned char palette[256][3];
    for (int i = 0; i < paletteSteps; i++)
    {
        const double* color = paletteColor((double) i / paletteSteps);
        palette[i][0] = (unsigned char) (color[0] * 255);
        palette[i][1] = (unsigned char) (color[1] * 255);
        palette[i][2] = (unsigned char) (color[2] * 255);
    }
    for (int i = paletteSteps; i < 256; i++)
        memcpy(palette[i], palette[paletteSteps-1], 3);

    makeCurrent();
    if (glIsTexture(m_textureScalar))
        glDeleteTextures(1, &m_textureScalar);
    glGenTextures(1, &m_textureScalar);

    int palFilter = Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteFilter).toBool() ? GL_LINEAR : GL_NEAREST;
    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, palFilter);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, palFilter);
    glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

    // adjust palette
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteFilter).toBool())
    {
        m_texScale = (double) (paletteSteps-1) / 256.0;
        m_texShift = 0.5 / 256.0;
    }
    else
    {
        m_texScale = (double) paletteSteps / 256.0;
        m_texShift = 0.0;
    }
}

void SceneViewPostInterface::paintScalarFieldColorBar(double min, double max)
{
    if (!Agros2D::problem()->isSolved() || !Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarColorBar).toBool()) return;

    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0)
            * (QString::number(-1.0, '+e', Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt()).length() + 1);
    int textHeight = 2 * (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // height() - 20.0
    Point scaleBorder = Point(10.0, (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // palette
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBegin(GL_QUADS);
    if (fabs(min - max) > EPS_ZERO)
        glTexCoord1d(m_texScale + m_texShift);
    else
        glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_1D);

    // ticks
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = 0.0;
        if (!Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeLog).toBool())
            value = min + (double) (i-1) / (numTicks-1) * (max - min);
        else
            value = min + (double) pow((double) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt(),
                                       ((i-1) / (numTicks-1)))/Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt() * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        printPostAt(scaleLeft + 33.0 + ((value >= 0.0) ? (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) : 0.0),
                    scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                    QString::number(value, '+e', Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt()));
    }

    // variable
    Module::LocalVariable localVariable = postHermes()->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString());
    QString str = QString("%1 (%2)").
            arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.shortname()).
            arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.unit());

    printPostAt(scaleLeft + scaleSize.x / 2.0 - (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) * str.count() / 2.0,
                scaleBorder.y + scaleSize.y - 20.0,
                str);
}
