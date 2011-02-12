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

#include "hermes_electrostatic.h"

#include "scene.h"
#include "gui.h"

struct ElectrostaticEdge
{
    PhysicFieldBC type;
    double value;
};

struct ElectrostaticLabel
{
    double charge_density;
    double permittivity;
};

ElectrostaticEdge *electrostaticEdge;
ElectrostaticLabel *electrostaticLabel;

template<typename Real, typename Scalar>
Scalar electrostatic_vector_form_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double surfaceCharge = 0.0;

    if (electrostaticEdge[e->edge_marker].type == PhysicFieldBC_Electrostatic_SurfaceCharge)
        surfaceCharge = electrostaticEdge[e->edge_marker].value;

    if (isPlanar)
        return surfaceCharge * int_v<Real, Scalar>(n, wt, v);
    else
        return surfaceCharge * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar electrostatic_matrix_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return electrostaticLabel[e->elem_marker].permittivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return electrostaticLabel[e->elem_marker].permittivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar electrostatic_vector_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return electrostaticLabel[e->elem_marker].charge_density / EPS0 * int_v<Real, Scalar>(n, wt, v);
    else
        return electrostaticLabel[e->elem_marker].charge_density / EPS0 * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

void callbackElectrostaticWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    wf->add_matrix_form(0, 0, callback(electrostatic_matrix_form));
    wf->add_vector_form(0, callback(electrostatic_vector_form));
    wf->add_vector_form_surf(0, callback(electrostatic_vector_form_surf));
}

// **************************************************************************************************************************

void HermesElectrostatic::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Electrostatic_Potential:
    case PhysicFieldBC_Electrostatic_SurfaceCharge:
        Util::scene()->addEdgeMarker(new SceneEdgeElectrostaticMarker(element->attribute("name"),
                                                                      type,
                                                                      Value(element->attribute("value", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesElectrostatic::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeElectrostaticMarker->type));
    element->setAttribute("value", edgeElectrostaticMarker->value.text());
}

void HermesElectrostatic::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelElectrostaticMarker(element->attribute("name"),
                                                                    Value(element->attribute("charge_density", "0")),
                                                                    Value(element->attribute("permittivity", "1"))));
}

void HermesElectrostatic::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelElectrostaticMarker *labelElectrostaticMarker = dynamic_cast<SceneLabelElectrostaticMarker *>(marker);

    element->setAttribute("charge_density", labelElectrostaticMarker->charge_density.text());
    element->setAttribute("permittivity", labelElectrostaticMarker->permittivity.text());
}

LocalPointValue *HermesElectrostatic::localPointValue(Point point)
{
    return new LocalPointValueElectrostatic(point);
}

QStringList HermesElectrostatic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "V" << "Ex" << "Ey" << "E" << "Dx" << "Dy" << "D" << "we" << "epsr";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesElectrostatic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueElectrostatic();
}

QStringList HermesElectrostatic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S" << "Q";
    return QStringList(headers);
}

VolumeIntegralValue *HermesElectrostatic::volumeIntegralValue()
{
    return new VolumeIntegralValueElectrostatic();
}

QStringList HermesElectrostatic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "We";
    return QStringList(headers);
}

SceneEdgeMarker *HermesElectrostatic::newEdgeMarker()
{
    return new SceneEdgeElectrostaticMarker(tr("new boundary"),
                                            PhysicFieldBC_Electrostatic_Potential,
                                            Value("0"));
}

SceneEdgeMarker *HermesElectrostatic::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeElectrostaticMarker(name,
                                                physicFieldBCFromStringKey(type),
                                                Value(QString::number(value)));
    }

    return NULL;
}

SceneEdgeMarker *HermesElectrostatic::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneEdgeElectrostaticMarker *marker = dynamic_cast<SceneEdgeElectrostaticMarker *>(Util::scene()->getEdgeMarker(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                marker->value = Value(QString::number(value));
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

SceneLabelMarker *HermesElectrostatic::newLabelMarker()
{
    return new SceneLabelElectrostaticMarker(tr("new material"),
                                             Value("0"),
                                             Value("1"));
}

SceneLabelMarker *HermesElectrostatic::newLabelMarker(PyObject *self, PyObject *args)
{
    double charge_density, permittivity;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &charge_density, &permittivity))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelElectrostaticMarker(name,
                                                 Value(QString::number(charge_density)),
                                                 Value(QString::number(permittivity)));
    }

    return NULL;
}

SceneLabelMarker *HermesElectrostatic::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double charge_density, permittivity;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &charge_density, &permittivity))
    {
        if (SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->charge_density = Value(QString::number(charge_density));
            marker->permittivity = Value(QString::number(permittivity));
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

void HermesElectrostatic::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueElectrostatic *localPointValueElectrostatic = dynamic_cast<LocalPointValueElectrostatic *>(localPointValue);

    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic field"));
    electrostaticNode->setExpanded(true);

    // Permittivity
    addTreeWidgetItemValue(electrostaticNode, tr("Permittivity:"), QString("%1").arg(localPointValueElectrostatic->permittivity, 0, 'f', 2), tr(""));

    // Charge Density
    addTreeWidgetItemValue(electrostaticNode, tr("Charge density:"), QString("%1").arg(localPointValueElectrostatic->charge_density, 0, 'e', 3), "C/m3");

    // Energy density
    addTreeWidgetItemValue(electrostaticNode, tr("Energy density:"), QString("%1").arg(localPointValueElectrostatic->we, 0, 'e', 3), "J/m3");

    // Potential
    addTreeWidgetItemValue(electrostaticNode, tr("Potential:"), QString("%1").arg(localPointValueElectrostatic->potential, 0, 'e', 3), "V");

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(electrostaticNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addTreeWidgetItemValue(itemElectricField, "E" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueElectrostatic->E.x, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, "E" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElectrostatic->E.y, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, "E:", QString("%1").arg(localPointValueElectrostatic->E.magnitude(), 0, 'e', 3), "V/m");

    // Electric Displacement
    QTreeWidgetItem *itemElectricDisplacement = new QTreeWidgetItem(electrostaticNode);
    itemElectricDisplacement->setText(0, tr("Electric displacement"));
    itemElectricDisplacement->setExpanded(true);

    addTreeWidgetItemValue(itemElectricDisplacement, "D" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueElectrostatic->D.x, 0, 'e', 3), "C/m2");
    addTreeWidgetItemValue(itemElectricDisplacement, "D" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElectrostatic->D.y, 0, 'e', 3), "C/m2");
    addTreeWidgetItemValue(itemElectricDisplacement, "D:", QString("%1").arg(localPointValueElectrostatic->D.magnitude(), 0, 'e', 3), "C/m2");
}

void HermesElectrostatic::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueElectrostatic *surfaceIntegralValueElectrostatic = dynamic_cast<SurfaceIntegralValueElectrostatic *>(surfaceIntegralValue);

    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic Field"));
    electrostaticNode->setExpanded(true);

    addTreeWidgetItemValue(electrostaticNode, tr("Charge:"), QString("%1").arg(surfaceIntegralValueElectrostatic->surfaceCharge, 0, 'e', 3), tr("C"));
}

void HermesElectrostatic::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueElectrostatic *volumeIntegralValueElectrostatic = dynamic_cast<VolumeIntegralValueElectrostatic *>(volumeIntegralValue);

    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic field"));
    electrostaticNode->setExpanded(true);

    addTreeWidgetItemValue(electrostaticNode, tr("Energy:"), QString("%1").arg(volumeIntegralValueElectrostatic->energy, 0, 'e', 3), tr("J"));
}

ViewScalarFilter *HermesElectrostatic::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    return new ViewScalarFilterElectrostatic(sln1,
                                             physicFieldVariable,
                                             physicFieldVariableComp);
}

// *******************************************************************************************************************************

QList<SolutionArray *> HermesElectrostatic::solve(ProgressItemSolve *progressItemSolve)
{
    // edge markers
    BCTypes bcTypes;
    BCValues bcValues;

    electrostaticEdge = new ElectrostaticEdge[Util::scene()->edges.count()+1];
    electrostaticEdge[0].type = PhysicFieldBC_None;
    electrostaticEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            electrostaticEdge[i+1].type = PhysicFieldBC_None;
            electrostaticEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeElectrostaticMarker->value.evaluate()) return QList<SolutionArray *>();

            electrostaticEdge[i+1].type = edgeElectrostaticMarker->type;
            electrostaticEdge[i+1].value = edgeElectrostaticMarker->value.number();

            switch (edgeElectrostaticMarker->type)
            {
            case PhysicFieldBC_None:
                bcTypes.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Electrostatic_Potential:
                bcTypes.add_bc_dirichlet(i+1);
                bcValues.add_const(i+1, edgeElectrostaticMarker->value.number());
                break;
            case PhysicFieldBC_Electrostatic_SurfaceCharge:
                bcTypes.add_bc_neumann(i+1);
                break;
            }
        }
    }

    // label markers
    electrostaticLabel = new ElectrostaticLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelElectrostaticMarker *labelElectrostaticMarker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelElectrostaticMarker->charge_density.evaluate()) return QList<SolutionArray *>();
            if (!labelElectrostaticMarker->permittivity.evaluate()) return QList<SolutionArray *>();

            electrostaticLabel[i].charge_density = labelElectrostaticMarker->charge_density.number();
            electrostaticLabel[i].permittivity = labelElectrostaticMarker->permittivity.number();
        }
    }

    SolutionAgros solutionAgros(progressItemSolve);

    QList<SolutionArray *> solutionArrayList = solutionAgros.solveSolutioArray(Hermes::vector<BCTypes *>(&bcTypes),
                                                                               Hermes::vector<BCValues *>(&bcValues),
                                                                               callbackElectrostaticWeakForm);

    delete [] electrostaticEdge;
    delete [] electrostaticLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueElectrostatic::LocalPointValueElectrostatic(Point &point) : LocalPointValue(point)
{
    charge_density = 0;
    permittivity = 0;

    potential = 0;
    E = Point();
    D = Point();
    we = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (labelMarker)
        {
            // potential
            potential = value;

            // electric field
            E = derivative * (-1);

            SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);

            charge_density = marker->charge_density.number();
            permittivity = marker->permittivity.number();

            // electric displacement
            D = E * (marker->permittivity.number() * EPS0);

            // energy density
            we = 0.5 * E.magnitude() * D.magnitude();
        }
    }
}

double LocalPointValueElectrostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Electrostatic_Potential:
    {
        return potential;
    }
        break;
    case PhysicFieldVariable_Electrostatic_ElectricField:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return E.x;
            break;
        case PhysicFieldVariableComp_Y:
            return E.y;
            break;
        case PhysicFieldVariableComp_Magnitude:
            return E.magnitude();
            break;
        }
    }
        break;
    case PhysicFieldVariable_Electrostatic_Displacement:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return D.x;
            break;
        case PhysicFieldVariableComp_Y:
            return D.y;
            break;
        case PhysicFieldVariableComp_Magnitude:
            return D.magnitude();
            break;
        }
    }
        break;
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
    {
        return we;
    }
        break;
    case PhysicFieldVariable_Electrostatic_Permittivity:
    {
        return permittivity;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueElectrostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueElectrostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(potential, 0, 'e', 5) <<
           QString("%1").arg(E.x, 0, 'e', 5) <<
           QString("%1").arg(E.y, 0, 'e', 5) <<
           QString("%1").arg(E.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(D.x, 0, 'e', 5) <<
           QString("%1").arg(D.y, 0, 'e', 5) <<
           QString("%1").arg(D.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(we, 0, 'e', 5) <<
           QString("%1").arg(permittivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueElectrostatic::SurfaceIntegralValueElectrostatic() : SurfaceIntegralValue()
{
    surfaceCharge = 0.0;

    calculate();

    surfaceCharge /= 2.0;
}

void SurfaceIntegralValueElectrostatic::calculateVariables(int i)
{
    if (boundary)
    {
        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[e->marker]->marker);
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            surfaceCharge += pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number() * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
        else
            surfaceCharge += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number() * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
    }
}

QStringList SurfaceIntegralValueElectrostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
           QString("%1").arg(surface, 0, 'e', 5) <<
           QString("%1").arg(surfaceCharge, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueElectrostatic::VolumeIntegralValueElectrostatic() : VolumeIntegralValue()
{
    energy = 0;

    calculate();
}

void VolumeIntegralValueElectrostatic::calculateVariables(int i)
{
    result = 0.0;
    SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[e->marker]->marker);
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        h1_integrate_expression(0.5 * EPS0 * marker->permittivity.number() * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * 0.5 * EPS0 * marker->permittivity.number() * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    energy += result;
}

void VolumeIntegralValueElectrostatic::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(0);
    sln2 = NULL;
}

QStringList VolumeIntegralValueElectrostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
           QString("%1").arg(crossSection, 0, 'e', 5) <<
           QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterElectrostatic::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Electrostatic_Potential:
    {
        node->values[0][0][i] = value1[i];
    }
        break;
    case PhysicFieldVariable_Electrostatic_ElectricField:
    {
        switch (m_physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
        {
            node->values[0][0][i] = - dudx1[i];
        }
            break;
        case PhysicFieldVariableComp_Y:
        {
            node->values[0][0][i] = - dudy1[i];
        }
            break;
        case PhysicFieldVariableComp_Magnitude:
        {
            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
        }
            break;
        }
    }
        break;
    case PhysicFieldVariable_Electrostatic_Displacement:
    {
        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);

        switch (m_physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
        {
            node->values[0][0][i] = - EPS0 * marker->permittivity.number() * dudx1[i];
        }
            break;
        case PhysicFieldVariableComp_Y:
        {
            node->values[0][0][i] = - EPS0 * marker->permittivity.number() * dudy1[i];
        }
            break;
        case PhysicFieldVariableComp_Magnitude:
        {
            node->values[0][0][i] = EPS0 * marker->permittivity.number() * sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
        }
            break;
        }
    }
        break;
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
    {
        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
        node->values[0][0][i] = 0.5 * EPS0 * marker->permittivity.number() * (sqr(dudx1[i]) + sqr(dudy1[i]));
    }
        break;
    case PhysicFieldVariable_Electrostatic_Permittivity:
    {
        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
        node->values[0][0][i] = marker->permittivity.number();
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterElectrostatic::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneEdgeElectrostaticMarker::SceneEdgeElectrostaticMarker(const QString &name, PhysicFieldBC type, Value value)
    : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeElectrostaticMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text());
}

QMap<QString, QString> SceneEdgeElectrostaticMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Electrostatic_Potential:
        out["Potential (V)"] = value.text();
        break;
    case PhysicFieldBC_Electrostatic_SurfaceCharge:
        out["Surface charge density (C/m3)"] = value.text();
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeElectrostaticMarker::showDialog(QWidget *parent)
{
    DSceneEdgeElectrostaticMarker *dialog = new DSceneEdgeElectrostaticMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelElectrostaticMarker::SceneLabelElectrostaticMarker(const QString &name, Value charge_density, Value permittivity)
    : SceneLabelMarker(name)
{
    this->charge_density = charge_density;
    this->permittivity = permittivity;
}

QString SceneLabelElectrostaticMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(charge_density.text()).
            arg(permittivity.text());
}

QMap<QString, QString> SceneLabelElectrostaticMarker::data()
{
    QMap<QString, QString> out;
    out["Charge density (C/m3)"] = charge_density.text();
    out["Permittivity (-)"] = permittivity.text();
    return QMap<QString, QString>(out);
}

int SceneLabelElectrostaticMarker::showDialog(QWidget *parent)
{
    DSceneLabelElectrostaticMarker *dialog = new DSceneLabelElectrostaticMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeElectrostaticMarker::DSceneEdgeElectrostaticMarker(SceneEdgeElectrostaticMarker *edgeElectrostaticMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeElectrostaticMarker;

    createDialog();

    load();
    setSize();
}

DSceneEdgeElectrostaticMarker::~DSceneEdgeElectrostaticMarker()
{
    delete cmbType;
    delete txtValue;
}

void DSceneEdgeElectrostaticMarker::createContent()
{
    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Electrostatic_Potential), PhysicFieldBC_Electrostatic_Potential);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Electrostatic_SurfaceCharge), PhysicFieldBC_Electrostatic_SurfaceCharge);

    txtValue = new SLineEditValue(this);
    connect(txtValue, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(new QLabel(tr("BC type:")), 1, 0);
    layout->addWidget(cmbType, 1, 1);
    layout->addWidget(new QLabel(tr("Value:")), 2, 0);
    layout->addWidget(txtValue, 2, 1);
}

void DSceneEdgeElectrostaticMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeElectrostaticMarker->type));
    txtValue->setValue(edgeElectrostaticMarker->value);
}

bool DSceneEdgeElectrostaticMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;

    SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(m_edgeMarker);

    edgeElectrostaticMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeElectrostaticMarker->value  = txtValue->value();
    else
        return false;

    return true;
}

// *************************************************************************************************************************************

DSceneLabelElectrostaticMarker::DSceneLabelElectrostaticMarker(QWidget *parent, SceneLabelElectrostaticMarker *labelElectrostaticMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelElectrostaticMarker;

    createDialog();

    load();
    setSize();
}

DSceneLabelElectrostaticMarker::~DSceneLabelElectrostaticMarker()
{
    delete txtPermittivity;
    delete txtChargeDensity;
}

void DSceneLabelElectrostaticMarker::createContent()
{
    txtPermittivity = new SLineEditValue(this);
    txtPermittivity->setMinimumSharp(0.0);
    txtChargeDensity = new SLineEditValue(this);
    connect(txtPermittivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtChargeDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(new QLabel(tr("Permittivity (-):")), 1, 0);
    layout->addWidget(txtPermittivity, 1, 1);
    layout->addWidget(new QLabel(tr("Charge density (C/m3):")), 2, 0);
    layout->addWidget(txtChargeDensity, 2, 1);
}

void DSceneLabelElectrostaticMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelElectrostaticMarker *labelElectrostaticMarker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_labelMarker);

    txtPermittivity->setValue(labelElectrostaticMarker->permittivity);
    txtChargeDensity->setValue(labelElectrostaticMarker->charge_density);
}

bool DSceneLabelElectrostaticMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelElectrostaticMarker *labelElectrostaticMarker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_labelMarker);

    if (txtPermittivity->evaluate())
        labelElectrostaticMarker->permittivity = txtPermittivity->value();
    else
        return false;

    if (txtChargeDensity->evaluate())
        labelElectrostaticMarker->charge_density = txtChargeDensity->value();
    else
        return false;

    return true;
}
