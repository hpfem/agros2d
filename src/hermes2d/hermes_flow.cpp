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

#include "hermes_flow.h"

#include "scene.h"
#include "gui.h"


struct FlowEdge
{
public:
    PhysicFieldBC type;
    double velocityX;
    double velocityY;
    double pressure;
};

struct FlowLabel
{
    double dynamic_viscosity;
    double density;
};

FlowEdge *flowEdge;
FlowLabel *flowLabel;

template<typename Real, typename Scalar>
Scalar bilinear_form_sym_0_0_1_1(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return flowLabel[e->elem_marker].dynamic_viscosity / flowLabel[e->elem_marker].density * (int_grad_u_grad_v<Real, Scalar>(n, wt, u, v) +
                                                                                              ((analysisType == AnalysisType_Transient) ? int_u_v<Real, Scalar>(n, wt, u, v) / timeStep : 0.0));
}

template<typename Real, typename Scalar>
Scalar bilinear_form_unsym_0_0_1_1(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Func<Scalar>* xvel_prev = ext->fn[0];
    Func<Scalar>* yvel_prev = ext->fn[1];
    return ((analysisType == AnalysisType_Transient) ? int_w_nabla_u_v<Real, Scalar>(n, wt, xvel_prev, yvel_prev, u, v) : 0.0);
}

template<typename Real, typename Scalar>
Scalar linear_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    // this form is used with both velocity components
    Func<Scalar>* vel_prev = ext->fn[0];
    return ((analysisType == AnalysisType_Transient) ? int_u_v<Real, Scalar>(n, wt, vel_prev, v) / timeStep : 0.0);
}

template<typename Real, typename Scalar>
Scalar bilinear_form_unsym_0_2(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return - int_u_dvdx<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar bilinear_form_unsym_1_2(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return - int_u_dvdy<Real, Scalar>(n, wt, u, v);
}

void callbackFlowWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    /*
    wf->add_matrix_form(0, 0, callback(bilinear_form_sym_0_0_1_1), HERMES_SYM);
    if (analysisType == AnalysisType_Transient)
        ; // wf->add_matrix_form(0, 0, callback(bilinear_form_unsym_0_0_1_1), HERMES_NONSYM, HERMES_ANY, slnArray);
    else
        wf->add_matrix_form(0, 0, callback(bilinear_form_unsym_0_0_1_1), HERMES_NONSYM);
    wf->add_matrix_form(1, 1, callback(bilinear_form_sym_0_0_1_1), HERMES_SYM);
    if (analysisType == AnalysisType_Transient)
        ; // wf->add_matrix_form(1, 1, callback(bilinear_form_unsym_0_0_1_1), HERMES_NONSYM, slnArray);
    else
        wf->add_matrix_form(1, 1, callback(bilinear_form_unsym_0_0_1_1), HERMES_NONSYM);
    wf->add_matrix_form(0, 2, callback(bilinear_form_unsym_0_2), HERMES_ANTISYM);
    wf->add_matrix_form(1, 2, callback(bilinear_form_unsym_1_2), HERMES_ANTISYM);
    if (analysisType == AnalysisType_Transient)
    {
        // wf->add_vector_form(0, callback(linear_form), HERMES_ANY, slnArray);
        // wf->add_vector_form(1, callback(linear_form), HERMES_ANY, slnArray);
    }
    else
    {
        wf->add_vector_form(0, callback(linear_form));
        wf->add_vector_form(1, callback(linear_form));
    }
    */

    wf->add_matrix_form(0, 0, callback(bilinear_form_sym_0_0_1_1), HERMES_SYM);
    wf->add_matrix_form(0, 0, callback(bilinear_form_unsym_0_0_1_1), HERMES_NONSYM);
    wf->add_matrix_form(1, 1, callback(bilinear_form_sym_0_0_1_1), HERMES_SYM);
    wf->add_matrix_form(1, 1, callback(bilinear_form_unsym_0_0_1_1), HERMES_NONSYM);
    wf->add_matrix_form(0, 2, callback(bilinear_form_unsym_0_2), HERMES_ANTISYM);
    wf->add_matrix_form(1, 2, callback(bilinear_form_unsym_1_2), HERMES_ANTISYM);
    wf->add_vector_form(0, callback(linear_form));
    wf->add_vector_form(1, callback(linear_form));
}

// *******************************************************************************************************

void HermesFlow::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Flow_Velocity:
    case PhysicFieldBC_Flow_Pressure:
    case PhysicFieldBC_Flow_Outlet:
    case PhysicFieldBC_Flow_Wall:
        Util::scene()->addBoundary(new SceneEdgeFlowMarker(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("velocityx", "0")),
                                                             Value(element->attribute("velocityy", "0")),
                                                             Value(element->attribute("pressure", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesFlow::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneEdgeFlowMarker *edgeFlowMarker = dynamic_cast<SceneEdgeFlowMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeFlowMarker->type));
    element->setAttribute("velocityx", edgeFlowMarker->velocityX.text);
    element->setAttribute("velocityy", edgeFlowMarker->velocityY.text);
    element->setAttribute("pressure", edgeFlowMarker->pressure.text);
}

void HermesFlow::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneLabelFlowMarker(element->attribute("name"),
                                                           Value(element->attribute("dynamic_viscosity")),
                                                           Value(element->attribute("density"))));
}

void HermesFlow::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneLabelFlowMarker *labelFlowMarker = dynamic_cast<SceneLabelFlowMarker *>(marker);

    element->setAttribute("dynamic_viscosity", labelFlowMarker->dynamic_viscosity.text);
    element->setAttribute("density", labelFlowMarker->density.text);
}

LocalPointValue *HermesFlow::localPointValue(const Point &point)
{
    return new LocalPointValueFlow(point);
}

QStringList HermesFlow::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "t" << "vx" << "vy" << "v" << "p";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesFlow::surfaceIntegralValue()
{
    return new SurfaceIntegralValueFlow();
}

QStringList HermesFlow::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesFlow::volumeIntegralValue()
{
    return new VolumeIntegralValueFlow();
}

QStringList HermesFlow::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S";
    return QStringList(headers);
}

SceneBoundary *HermesFlow::newBoundary()
{
    return new SceneEdgeFlowMarker(tr("new boundary"),
                                   PhysicFieldBC_Flow_Pressure,
                                   Value("0"),
                                   Value("0"),
                                   Value("0"));
}

SceneBoundary *HermesFlow::newBoundary(PyObject *self, PyObject *args)
{

    double valuex, valuey, press;
    char *name, *type;
    if (PyArg_ParseTuple(args, "sssdd", &name, &type, &valuex, &valuey, &press))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneEdgeFlowMarker(name,
                                       physicFieldBCFromStringKey(type),
                                       Value(QString::number(valuex)),
                                       Value(QString::number(valuey)),
                                       Value(QString::number(press)));
    }

    return Util::scene()->boundaries[0];
}

SceneBoundary *HermesFlow::modifyBoundary(PyObject *self, PyObject *args)
{
    double valuex, valuey, press;
    char *name, *type;
    if (PyArg_ParseTuple(args, "sssdd", &name, &type, &valuex, &valuey, &press))
    {
        if (SceneEdgeFlowMarker *marker = dynamic_cast<SceneEdgeFlowMarker *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                marker->velocityX = Value(QString::number(valuex));
                marker->velocityY = Value(QString::number(valuey));
                marker->pressure = Value(QString::number(press));
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
                return NULL;
            }
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
}

SceneMaterial *HermesFlow::newMaterial()
{
    return new SceneLabelFlowMarker(tr("new material"),
                                    Value("8.9e-4"),
                                    Value("1000"));
}

SceneMaterial *HermesFlow::newMaterial(PyObject *self, PyObject *args)
{
    double dynamic_viscosity, density;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &dynamic_viscosity, &density))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneLabelFlowMarker(name,
                                        Value(QString::number(dynamic_viscosity)),
                                        Value(QString::number(density)));
    }

    return NULL;
}

SceneMaterial *HermesFlow::modifyMaterial(PyObject *self, PyObject *args)
{
    double dynamic_viscosity, density;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &dynamic_viscosity, &density))
    {
        if (SceneLabelFlowMarker *marker = dynamic_cast<SceneLabelFlowMarker *>(Util::scene()->getMaterial(name)))
        {
            marker->dynamic_viscosity = Value(QString::number(dynamic_viscosity));
            marker->density = Value(QString::number(density));
            return marker;
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Label marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
}

void HermesFlow::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueFlow *localPointValueFlow = dynamic_cast<LocalPointValueFlow *>(localPointValue);

    // flow
    QTreeWidgetItem *flowNode = new QTreeWidgetItem(trvWidget);
    flowNode->setText(0, tr("Flow"));
    flowNode->setExpanded(true);

    // Dynamic viscosity
    addTreeWidgetItemValue(flowNode, tr("Dynamic viscosity:"), QString("%1").arg(localPointValueFlow->dynamic_viscosity, 0, 'e', 3), "Pa.s");

    // Density
    addTreeWidgetItemValue(flowNode, tr("Density:"), QString("%1").arg(localPointValueFlow->density, 0, 'f', 3), "kg/m3");

    // Velocity
    QTreeWidgetItem *itemVelocity = new QTreeWidgetItem(flowNode);
    itemVelocity->setText(0, tr("Velocity"));
    itemVelocity->setExpanded(true);

    addTreeWidgetItemValue(itemVelocity, "v" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueFlow->velocity_x, 0, 'e', 3), "m/s");
    addTreeWidgetItemValue(itemVelocity, "v" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueFlow->velocity_y, 0, 'e', 3), "m/s");
    addTreeWidgetItemValue(itemVelocity, "v:", QString("%1").arg(sqrt(sqr(localPointValueFlow->velocity_x) + sqr(localPointValueFlow->velocity_y)), 0, 'e', 3), "m/s");

    // Pressure
    addTreeWidgetItemValue(flowNode, tr("Pressure:"), QString("%1").arg(localPointValueFlow->pressure, 0, 'e', 3), "Pa");
}

void HermesFlow::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    // SurfaceIntegralValueFlow *surfaceIntegralValueFlow = dynamic_cast<SurfaceIntegralValueFlow *>(surfaceIntegralValue);
}

void HermesFlow::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{

}

ViewScalarFilter *HermesFlow::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *slnX = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    Solution *slnY = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
    Solution *slnPress = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 2);

    return new ViewScalarFilterFlow(Hermes::vector<MeshFunction *>(slnX, slnY, slnPress),
                                    physicFieldVariable,
                                    physicFieldVariableComp);
}

QList<SolutionArray *> HermesFlow::solve(ProgressItemSolve *progressItemSolve)
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return QList<SolutionArray *>();
    }

    // edge markers
    BCTypes bcTypesX, bcTypesY, bcTypesP;
    BCValues bcValuesX, bcValuesY, bcValuesP;

    flowEdge = new FlowEdge[Util::scene()->edges.count()+1];
    flowEdge[0].type = PhysicFieldBC_None;
    flowEdge[0].velocityX = 0;
    flowEdge[0].velocityY = 0;
    flowEdge[0].pressure = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->boundaries.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            flowEdge[i+1].type = PhysicFieldBC_None;
            flowEdge[i+1].velocityX = 0;
            flowEdge[i+1].velocityY = 0;
            flowEdge[i+1].pressure = 0;
        }
        else
        {
            SceneEdgeFlowMarker *edgeFlowMarker = dynamic_cast<SceneEdgeFlowMarker *>(Util::scene()->edges[i]->marker);
            flowEdge[i+1].type = edgeFlowMarker->type;

            if (!edgeFlowMarker->velocityX.evaluate()) return QList<SolutionArray *>();
            if (!edgeFlowMarker->velocityY.evaluate()) return QList<SolutionArray *>();
            if (!edgeFlowMarker->pressure.evaluate()) return QList<SolutionArray *>();

            flowEdge[i+1].velocityX = edgeFlowMarker->velocityX.number;
            flowEdge[i+1].velocityY = edgeFlowMarker->velocityY.number;
            flowEdge[i+1].pressure = edgeFlowMarker->pressure.number;

            switch (edgeFlowMarker->type)
            {
            case PhysicFieldBC_None:
                bcTypesX.add_bc_none(i+1);
                bcTypesY.add_bc_none(i+1);
                bcTypesP.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Flow_Wall:
                bcTypesX.add_bc_dirichlet(i+1);
                bcValuesX.add_const(i+1, 0.0);
                bcTypesY.add_bc_dirichlet(i+1);
                bcValuesY.add_const(i+1, 0.0);
                // bcTypesP.add_bc_neumann(i+1);
                bcTypesP.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Flow_Velocity:
                bcTypesX.add_bc_dirichlet(i+1);
                bcValuesX.add_const(i+1, edgeFlowMarker->velocityX.number);
                bcTypesY.add_bc_dirichlet(i+1);
                bcValuesY.add_const(i+1, edgeFlowMarker->velocityY.number);
                bcTypesP.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Flow_Outlet:
                bcTypesX.add_bc_none(i+1);
                bcTypesY.add_bc_none(i+1);
                bcTypesP.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Flow_Pressure:
                bcTypesX.add_bc_none(i+1);
                bcTypesY.add_bc_none(i+1);
                bcTypesP.add_bc_dirichlet(i+1);
                bcValuesP.add_const(i+1, edgeFlowMarker->pressure.number);
                break;
            }
        }
    }

    // label markers
    flowLabel = new FlowLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelFlowMarker *labelFlowMarker = dynamic_cast<SceneLabelFlowMarker *>(Util::scene()->labels[i]->marker);

            if (!labelFlowMarker->dynamic_viscosity.evaluate()) return QList<SolutionArray *>();
            if (!labelFlowMarker->density.evaluate()) return QList<SolutionArray *>();

            flowLabel[i].dynamic_viscosity = labelFlowMarker->dynamic_viscosity.number;
            flowLabel[i].density = labelFlowMarker->density.number;
        }
    }

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve,
                                                                 Hermes::vector<BCTypes *>(&bcTypesX, &bcTypesY, &bcTypesP),
                                                                 Hermes::vector<BCValues *>(&bcValuesX, &bcValuesY, &bcValuesP),
                                                                 callbackFlowWeakForm);

    delete [] flowEdge;
    delete [] flowLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueFlow::LocalPointValueFlow(const Point &point) : LocalPointValue(point)
{
    velocity_x = 0;
    velocity_y = 0;
    pressure = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (material)
        {
            Solution *slnX = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
            Solution *slnY = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
            Solution *slnPress = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 2);
            PointValue valueX = pointValue(slnX, point);
            PointValue valueY = pointValue(slnY, point);
            PointValue valuePress = pointValue(slnPress, point);

            velocity_x = valueX.value;
            velocity_y = valueY.value;
            pressure = valuePress.value;

            SceneLabelFlowMarker *marker = dynamic_cast<SceneLabelFlowMarker *>(material);

            dynamic_viscosity = marker->dynamic_viscosity.number;
            density = marker->density.number;
        }
    }
}

double LocalPointValueFlow::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Flow_Velocity:
        return sqrt(sqr(velocity_x) + sqr(velocity_y));
    case PhysicFieldVariable_Flow_VelocityX:
        return velocity_x;
    case PhysicFieldVariable_Flow_VelocityY:
        return velocity_y;
    case PhysicFieldVariable_Flow_Pressure:
        return pressure;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueHeat::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueFlow::variables()
{
    QStringList row;
    row << QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(Util::scene()->sceneSolution()->time(), 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(velocity_x) + sqr(velocity_y)), 0, 'e', 5) <<
           QString("%1").arg(velocity_x, 0, 'e', 5) <<
           QString("%1").arg(velocity_y, 0, 'e', 5) <<
           QString("%1").arg(pressure, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SurfaceIntegralValueFlow::SurfaceIntegralValueFlow() : SurfaceIntegralValue()
{
    calculate();
}

void SurfaceIntegralValueFlow::calculateVariables(int i)
{

}

QStringList SurfaceIntegralValueFlow::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
           QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueFlow::VolumeIntegralValueFlow() : VolumeIntegralValue()
{
    calculate();
}

void VolumeIntegralValueFlow::calculateVariables(int i)
{

}

void VolumeIntegralValueFlow::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(0);
    sln2 = Util::scene()->sceneSolution()->sln(1);
}

QStringList VolumeIntegralValueFlow::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
           QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterFlow::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Flow_Velocity:
    {
        SceneLabelFlowMarker *marker = dynamic_cast<SceneLabelFlowMarker *>(material);

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = value1[i];
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = value2[i];
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
            }
                break;
            }
        }
    }
        break;
    case PhysicFieldVariable_Flow_Pressure:
    {
        SceneLabelFlowMarker *marker = dynamic_cast<SceneLabelFlowMarker *>(material);

        node->values[0][0][i] = value3[i];
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterFlow::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneEdgeFlowMarker::SceneEdgeFlowMarker(const QString &name, PhysicFieldBC type, Value velocityX, Value velocityY, Value pressure)
    : SceneBoundary(name, type)
{
    this->velocityX = velocityX;
    this->velocityY = velocityY;
    this->pressure = pressure;
}

QString SceneEdgeFlowMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3, %4, %5)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(velocityX.text).
            arg(velocityY.text).
            arg(pressure.text);
}

QMap<QString, QString> SceneEdgeFlowMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Flow_Velocity:
        out["Velocity: (m/s)"] = sqrt(sqr(velocityX.number) + sqr(velocityY.number));
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeFlowMarker::showDialog(QWidget *parent)
{
    DSceneEdgeFlowMarker *dialog = new DSceneEdgeFlowMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelFlowMarker::SceneLabelFlowMarker(const QString &name, Value dynamic_viscosity, Value density)
    : SceneMaterial(name)
{
    this->dynamic_viscosity = dynamic_viscosity;
    this->density = density;
}

QString SceneLabelFlowMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(dynamic_viscosity.text).
            arg(density.text);
}

QMap<QString, QString> SceneLabelFlowMarker::data()
{
    QMap<QString, QString> out;
    out["Dynamic_viscosity (Pa.s)"] = dynamic_viscosity.number;
    out["Density (kg/m3)"] = density.number;
    return QMap<QString, QString>(out);
}

int SceneLabelFlowMarker::showDialog(QWidget *parent)
{
    DSceneLabelFlowMarker *dialog = new DSceneLabelFlowMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeFlowMarker::DSceneEdgeFlowMarker(SceneEdgeFlowMarker *edgeEdgeFlowMarker, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = edgeEdgeFlowMarker;

    createDialog();

    load();
    setSize();
}

void DSceneEdgeFlowMarker::createContent()
{
    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Flow_Wall), PhysicFieldBC_Flow_Wall);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Flow_Outlet), PhysicFieldBC_Flow_Outlet);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Flow_Velocity), PhysicFieldBC_Flow_Velocity);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Flow_Pressure), PhysicFieldBC_Flow_Pressure);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtVelocityX = new ValueLineEdit(this);
    txtVelocityY = new ValueLineEdit(this);
    txtPressure = new ValueLineEdit(this);

    layout->addWidget(new QLabel(tr("BC Type:")), 1, 0);
    layout->addWidget(cmbType, 1, 1);
    layout->addWidget(new QLabel(tr("Velocity X (m/s):")), 2, 0);
    layout->addWidget(txtVelocityX, 2, 1);
    layout->addWidget(new QLabel(tr("Velocity Y (m/s):")), 3, 0);
    layout->addWidget(txtVelocityY, 3, 1);
    layout->addWidget(new QLabel(tr("Pressure (Pa):")), 4, 0);
    layout->addWidget(txtPressure, 4, 1);
}

void DSceneEdgeFlowMarker::load()
{
    SceneBoundaryDialog::load();

    SceneEdgeFlowMarker *edgeFlowMarker = dynamic_cast<SceneEdgeFlowMarker *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(edgeFlowMarker->type));

    txtVelocityX->setValue(edgeFlowMarker->velocityX);
    txtVelocityY->setValue(edgeFlowMarker->velocityY);
    txtPressure->setValue(edgeFlowMarker->pressure);
}

bool DSceneEdgeFlowMarker::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    SceneEdgeFlowMarker *edgeFlowMarker = dynamic_cast<SceneEdgeFlowMarker *>(m_boundary);

    edgeFlowMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtVelocityX->evaluate())
        edgeFlowMarker->velocityX = txtVelocityX->value();
    else
        return false;

    if (txtVelocityY->evaluate())
        edgeFlowMarker->velocityY = txtVelocityY->value();
    else
        return false;

    if (txtPressure->evaluate())
        edgeFlowMarker->pressure = txtPressure->value();
    else
        return false;

    return true;
}

void DSceneEdgeFlowMarker::doTypeChanged(int index)
{
    txtVelocityX->setEnabled(false);
    txtVelocityY->setEnabled(false);
    txtPressure->setEnabled(false);

    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_Flow_Velocity:
    {
        txtVelocityX->setEnabled(true);
        txtVelocityY->setEnabled(true);
    }
        break;
    case PhysicFieldBC_Flow_Pressure:
    {
        txtPressure->setEnabled(true);
    }
        break;
    }
}

// *************************************************************************************************************************************

DSceneLabelFlowMarker::DSceneLabelFlowMarker(QWidget *parent, SceneLabelFlowMarker *labelFlowMarker) : SceneMaterialDialog(parent)
{
    m_material = labelFlowMarker;

    createDialog();

    load();
    setSize();
}

void DSceneLabelFlowMarker::createContent()
{
    txtDynamicViscosity = new ValueLineEdit(this);
    txtDensity = new ValueLineEdit(this);

    layout->addWidget(new QLabel(tr("Dynamic viscosity")), 10, 0);
    layout->addWidget(new QLabel(tr("<i>%1</i> (Pa.s)").arg(QString::fromUtf8("μ"))), 10, 1);
    layout->addWidget(txtDynamicViscosity, 10, 2);
    layout->addWidget(new QLabel(tr("Mass density")), 11, 0);
    layout->addWidget(new QLabel(tr("<i>%1</i> (kg/m<sup>3</sup>)").arg(QString::fromUtf8("ρ"))), 11, 1);
    layout->addWidget(txtDensity, 11, 2);
}

void DSceneLabelFlowMarker::load()
{
    SceneMaterialDialog::load();

    SceneLabelFlowMarker *labelFlowMarker = dynamic_cast<SceneLabelFlowMarker *>(m_material);

    txtDynamicViscosity->setValue(labelFlowMarker->dynamic_viscosity);
    txtDensity->setValue(labelFlowMarker->density);
}

bool DSceneLabelFlowMarker::save()
{
    if (!SceneMaterialDialog::save()) return false;

    SceneLabelFlowMarker *labelFlowMarker = dynamic_cast<SceneLabelFlowMarker *>(m_material);

    if (txtDynamicViscosity->evaluate())
        labelFlowMarker->dynamic_viscosity = txtDynamicViscosity->value();
    else
        return false;

    if (txtDensity->evaluate())
        labelFlowMarker->density = txtDensity->value();
    else
        return false;

    return true;
}
