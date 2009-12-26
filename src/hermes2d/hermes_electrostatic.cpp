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

struct ElectrostaticEdge
{
public:
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
bool electrostaticPlanar;

int electrostatic_bc_types(int marker)
{
    switch (electrostaticEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
        return BC_NATURAL;
        break;
    }
}

scalar electrostatic_bc_values(int marker, double x, double y)
{
    return electrostaticEdge[marker].value;
}

template<typename Real, typename Scalar>
Scalar electrostatic_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (electrostaticPlanar)
        return electrostaticLabel[e->marker].permittivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return electrostaticLabel[e->marker].permittivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar electrostatic_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (electrostaticPlanar)
        return electrostaticLabel[e->marker].charge_density / EPS0 * int_v<Real, Scalar>(n, wt, v);
    else
        return electrostaticLabel[e->marker].charge_density / EPS0 * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

QList<SolutionArray *> *electrostatic_main(SolverDialog *solverDialog)
{
    electrostaticPlanar = (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo()->numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh mesh;
    mesh.load((tempProblemFileName() + ".mesh").toStdString().c_str());
    for (int i = 0; i < numberOfRefinements; i++)
        mesh.refine_all_elements(0);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    // initialize the shapeset and the cache
    H1Shapeset shapeset;
    PrecalcShapeset pss(&shapeset);

    // create an H1 space
    H1Space space(&mesh, &shapeset);
    space.set_bc_types(electrostatic_bc_types);
    space.set_bc_values(electrostatic_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs(0);

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(electrostatic_bilinear_form));
    wf.add_liform(0, callback(electrostatic_linear_form));

    Solution *sln = new Solution();
    Solution rsln;

    // initialize the linear solver
    UmfpackSolver umfpack;

    // initialize the linear system
    LinSystem sys(&wf, &umfpack);
    sys.set_spaces(1, &space);
    sys.set_pss(1, &pss);

    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    int adaptivitysteps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps + 1;
    for (i = 0; i<adaptivitysteps; i++)
    {
        space.assign_dofs();

        sys.assemble();
        sys.solve(1, sln);



        // calculate errors and adapt the solution
        if (adaptivityType != ADAPTIVITYTYPE_NONE)
        {
            RefSystem rs(&sys);
            rs.assemble();
            rs.solve(1, &rsln);

            H1OrthoHP hp(1, &space);
            error = hp.calc_error(sln, &rsln) * 100;

            // emit signal
            solverDialog->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
            if (solverDialog->isCanceled())
            {
                solutionArrayList->clear();
                return solutionArrayList;
            }

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            if (i != adaptivitysteps-1) hp.adapt(0.3, 0, (int) adaptivityType);
        }
    }

    // output
    SolutionArray *solutionArray = new SolutionArray();
    solutionArray->order = new Orderizer();
    solutionArray->order->process_solution(&space);
    solutionArray->sln = sln;
    solutionArray->adaptiveError = error;
    solutionArray->adaptiveSteps = i-1;   

    solutionArrayList->append(solutionArray);

    return solutionArrayList;
}

// **************************************************************************************************************************

void HermesElectrostatic::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
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
    element->setAttribute("value", edgeElectrostaticMarker->value.text);
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

    element->setAttribute("charge_density", labelElectrostaticMarker->charge_density.text);
    element->setAttribute("permittivity", labelElectrostaticMarker->permittivity.text);
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
    return new SceneEdgeElectrostaticMarker("new boundary",
                                            PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL,
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

SceneLabelMarker *HermesElectrostatic::newLabelMarker()
{
    return new SceneLabelElectrostaticMarker("new material",
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

QList<SolutionArray *> *HermesElectrostatic::solve(SolverDialog *solverDialog)
{
    // edge markers
    electrostaticEdge = new ElectrostaticEdge[Util::scene()->edges.count()+1];
    electrostaticEdge[0].type = PHYSICFIELDBC_NONE;
    electrostaticEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            electrostaticEdge[i+1].type = PHYSICFIELDBC_NONE;
            electrostaticEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeElectrostaticMarker->value.evaluate()) return NULL;

            electrostaticEdge[i+1].type = edgeElectrostaticMarker->type;
            electrostaticEdge[i+1].value = edgeElectrostaticMarker->value.number;
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
            if (!labelElectrostaticMarker->charge_density.evaluate()) return NULL;
            if (!labelElectrostaticMarker->permittivity.evaluate()) return NULL;

            electrostaticLabel[i].charge_density = labelElectrostaticMarker->charge_density.number;
            electrostaticLabel[i].permittivity = labelElectrostaticMarker->permittivity.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = electrostatic_main(solverDialog);

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

            charge_density = marker->charge_density.number;
            permittivity = marker->permittivity.number;

            // electric displacement
            D = E * (marker->permittivity.number * EPS0);

            // energy density
            we = 0.5 * E.magnitude() * D.magnitude();
        }
    }
}

double LocalPointValueElectrostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        {
            return potential;
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return E.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return E.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return E.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return D.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return D.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return D.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        {
            return we;
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
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
        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
            surfaceCharge += pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
        else
            surfaceCharge += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
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
    if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
    {
        h1_integrate_expression(0.5 * EPS0 * marker->permittivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * 0.5 * EPS0 * marker->permittivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    energy += result;
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
            arg(value.text);
}

QMap<QString, QString> SceneEdgeElectrostaticMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
        out["Potential (V)"] = value.text;
        break;
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
        out["Surface charge density (C/m3)"] = value.text;
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
            arg(charge_density.text).
            arg(permittivity.text);
}

QMap<QString, QString> SceneLabelElectrostaticMarker::data()
{
    QMap<QString, QString> out;
    out["Charge density (C/m3)"] = charge_density.text;
    out["Permittivity (-)"] = permittivity.text;
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

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtValue);

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
    cmbType = new QComboBox();
    cmbType->addItem("none", PHYSICFIELDBC_NONE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL), PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE), PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE);

    txtValue = new SLineEditValue(this);

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

    // tab order
    setTabOrder(txtName, txtPermittivity);
    setTabOrder(txtPermittivity, txtChargeDensity);

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
    txtChargeDensity = new SLineEditValue(this);

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
