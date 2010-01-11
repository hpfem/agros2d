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

#include "hermes_current.h"

#include "scene.h"

#include "localvalueview.h"
#include "surfaceintegralview.h"
#include "volumeintegralview.h"

struct CurrentEdge
{
public:
    PhysicFieldBC type;
    double value;
};

struct CurrentLabel
{
    double conductivity;
};

CurrentEdge *currentEdge;
CurrentLabel *currentLabel;
bool currentPlanar;

int current_bc_types(int marker)
{
    switch (currentEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        return BC_ESSENTIAL;
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        return BC_NATURAL;
    }
}

scalar current_bc_values(int marker, double x, double y)
{
    switch (currentEdge[marker].type)
    {
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        return currentEdge[marker].value;
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        return currentEdge[marker].value;
    }
}

template<typename Real, typename Scalar>
Scalar current_linear_form_surf(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double J = 0.0;

    if (currentEdge[e->marker].type == PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW)
        J = currentEdge[e->marker].value;

    if (currentPlanar)
        return J * int_v<Real, Scalar>(n, wt, v);
    else
        return J * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);

}

template<typename Real, typename Scalar>
Scalar current_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (currentPlanar)
        return currentLabel[e->marker].conductivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return currentLabel[e->marker].conductivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar current_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return 0.0;
    /*
    if (electrostaticPlanar)
        return electrostaticLabel[marker].charge_density / EPS0 * int_v<Real, Scalar>(n, wt, v);
    else
        return electrostaticLabel[marker].charge_density / EPS0 * 2 * M_PI * int_v<Real, Scalar>(n, wt, v); // FIXME int_x_v
    */
}

QList<SolutionArray *> *current_main(SolverDialog *solverDialog)
{
    currentPlanar = (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
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
    space.set_bc_types(current_bc_types);
    space.set_bc_values(current_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs(0);

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(current_bilinear_form));
    wf.add_liform(0, callback(current_linear_form));
    wf.add_liform_surf(0, callback(current_linear_form_surf));

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
    int adaptivitysteps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps;
    for (i = 0; i<(adaptivitysteps); i++)
    {
        space.assign_dofs();

        sys.assemble();
        sys.solve(1, sln);

        // calculate errors and adapt the solution
        if ((adaptivityType != ADAPTIVITYTYPE_NONE))
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

// *******************************************************************************************************

void HermesCurrent::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        Util::scene()->addEdgeMarker(new SceneEdgeCurrentMarker(element->attribute("name"),
                                                                type,
                                                                Value(element->attribute("value", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesCurrent::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeCurrentMarker->type));
    element->setAttribute("value", edgeCurrentMarker->value.text);
}

void HermesCurrent::readLabelMarkerFromDomElement(QDomElement *element)
{    
    Util::scene()-> addLabelMarker(new SceneLabelCurrentMarker(element->attribute("name"),
                                                               Value(element->attribute("conductivity", "0"))));
}

void HermesCurrent::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(marker);

    element->setAttribute("conductivity", labelCurrentMarker->conductivity.text);
}

LocalPointValue *HermesCurrent::localPointValue(Point point)
{
    return new LocalPointValueCurrent(point);
}

QStringList HermesCurrent::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "V" << "Jx" << "Jy" << "J" << "Ex" << "Ey" << "E" << "pj" << "gamma";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesCurrent::surfaceIntegralValue()
{
    return new SurfaceIntegralValueCurrent();
}

QStringList HermesCurrent::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S" << "I";
    return QStringList(headers);
}

VolumeIntegralValue *HermesCurrent::volumeIntegralValue()
{
    return new VolumeIntegralValueCurrent();
}

QStringList HermesCurrent::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "Jx_avg" << "Jy_avg" << "J_avg" << "Ex_avg" << "Ey_avg" << "E_avg" << "Pj";
    return QStringList(headers);
}

SceneEdgeMarker *HermesCurrent::newEdgeMarker()
{
    return new SceneEdgeCurrentMarker("new boundary",
                                      PHYSICFIELDBC_CURRENT_POTENTIAL,
                                      Value("0"));
}

SceneEdgeMarker *HermesCurrent::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeCurrentMarker(name,
                                          physicFieldBCFromStringKey(type),
                                          Value(QString::number(value)));
    }

    return NULL;
}

SceneLabelMarker *HermesCurrent::newLabelMarker()
{
    return new SceneLabelCurrentMarker("new material",
                                       Value("57e6"));
}

SceneLabelMarker *HermesCurrent::newLabelMarker(PyObject *self, PyObject *args)
{
    double conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sd", &name, &conductivity))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelCurrentMarker(name,
                                           Value(QString::number(conductivity)));
    }

    return NULL;
}

void HermesCurrent::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueCurrent *localPointValueCurrent = dynamic_cast<LocalPointValueCurrent *>(localPointValue);

    // current field
    QTreeWidgetItem *currentNode = new QTreeWidgetItem(trvWidget);
    currentNode->setText(0, tr("Current field"));
    currentNode->setExpanded(true);

    // Conductivity
    addTreeWidgetItemValue(currentNode, tr("Conductivity:"), QString("%1").arg(localPointValueCurrent->conductivity, 0, 'e', 3), "S/m");

    // Energy density
    addTreeWidgetItemValue(currentNode, tr("Power losses dens.:"), QString("%1").arg(localPointValueCurrent->losses, 0, 'e', 3), tr("W/m3"));

    // Potential
    addTreeWidgetItemValue(currentNode, tr("Potential:"), QString("%1").arg(localPointValueCurrent->potential, 0, 'f', 2), "V");

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(currentNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addTreeWidgetItemValue(itemElectricField, "E" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueCurrent->E.x, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, "E" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueCurrent->E.y, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, "E:", QString("%1").arg(localPointValueCurrent->E.magnitude(), 0, 'e', 3), "V/m");

    // Current Density
    QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(currentNode);
    itemCurrentDensity->setText(0, tr("Current density"));
    itemCurrentDensity->setExpanded(true);

    addTreeWidgetItemValue(itemCurrentDensity, "J" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueCurrent->J.x, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, "J" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueCurrent->J.y, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, "J:", QString("%1").arg(localPointValueCurrent->J.magnitude(), 0, 'e', 3), "A/m2");
}

void HermesCurrent::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueCurrent *surfaceIntegralValueCurrent = dynamic_cast<SurfaceIntegralValueCurrent *>(surfaceIntegralValue);

    // current field
    QTreeWidgetItem *currentNode = new QTreeWidgetItem(trvWidget);
    currentNode->setText(0, tr("Current Field"));
    currentNode->setExpanded(true);

    addTreeWidgetItemValue(currentNode, tr("Current:"), QString("%1").arg(surfaceIntegralValueCurrent->currentDensity, 0, 'e', 3), tr("A"));
}

void HermesCurrent::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{

}

QList<SolutionArray *> *HermesCurrent::solve(SolverDialog *solverDialog)
{
    // edge markers
    currentEdge = new CurrentEdge[Util::scene()->edges.count()+1];
    currentEdge[0].type = PHYSICFIELDBC_NONE;
    currentEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            currentEdge[i+1].type = PHYSICFIELDBC_NONE;
            currentEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeCurrentMarker->value.evaluate()) return NULL;

            currentEdge[i+1].type = edgeCurrentMarker->type;
            currentEdge[i+1].value = edgeCurrentMarker->value.number;
        }
    }

    // label markers
    currentLabel = new CurrentLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelCurrentMarker->conductivity.evaluate()) return NULL;

            currentLabel[i].conductivity = labelCurrentMarker->conductivity.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = current_main(solverDialog);

    delete [] currentEdge;
    delete [] currentLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueCurrent::LocalPointValueCurrent(Point &point) : LocalPointValue(point)
{
    conductivity = 0;

    potential = 0;
    J = Point();
    E = Point();
    losses = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (labelMarker != NULL)
        {
            // potential
            potential = value;

            // electric field
            E.x =  derivative.y;
            E.y = -derivative.x;

            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);

            conductivity = marker->conductivity.number;

            // electric displacement
            J = E * marker->conductivity.number;

            // energy density
            losses = J.magnitude() * E.magnitude();
        }
    }
}

double LocalPointValueCurrent::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        {
            return potential;
        }
        break;
    case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
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
    case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return J.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return J.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return J.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
        {
            return losses;
        }
        break;
    case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
        {
            return conductivity;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueCurrent::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential, 0, 'e', 5) <<
            QString("%1").arg(J.x, 0, 'e', 5) <<
            QString("%1").arg(J.y, 0, 'e', 5) <<
            QString("%1").arg(J.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(E.x, 0, 'e', 5) <<
            QString("%1").arg(E.y, 0, 'e', 5) <<
            QString("%1").arg(E.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(losses, 0, 'e', 5) <<
            QString("%1").arg(conductivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueCurrent::SurfaceIntegralValueCurrent() : SurfaceIntegralValue()
{
    currentDensity = 0.0;

    calculate();

    currentDensity /= 2.0;
}

void SurfaceIntegralValueCurrent::calculateVariables(int i)
{
    if (boundary)
    {
        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[e->marker]->marker);
        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
            currentDensity += pt[i][2] * tan[i][2] * marker->conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
        else
            currentDensity += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->conductivity.number * (- tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
    }
}

QStringList SurfaceIntegralValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5) <<
            QString("%1").arg(currentDensity, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueCurrent::VolumeIntegralValueCurrent() : VolumeIntegralValue()
{
    losses = 0.0;

    calculate();
}

void VolumeIntegralValueCurrent::calculateVariables(int i)
{
    result = 0.0;
    SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[e->marker]->marker);
    if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
    {
        h1_integrate_expression(marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    losses += result;
}

QStringList VolumeIntegralValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(losses, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SceneEdgeCurrentMarker::SceneEdgeCurrentMarker(const QString &name, PhysicFieldBC type, Value value) : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeCurrentMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneEdgeCurrentMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        out["Potential (V)"] = value.text;
        break;
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        out["Inward current flow: (A/m2)"] = value.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeCurrentMarker::showDialog(QWidget *parent)
{
    DSceneEdgeCurrentMarker *dialog = new DSceneEdgeCurrentMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelCurrentMarker::SceneLabelCurrentMarker(const QString &name, Value conductivity) : SceneLabelMarker(name)
{
    this->conductivity = conductivity;
}

QString SceneLabelCurrentMarker::script()
{
    return QString("addmaterial(\"%1\", %3)").
            arg(name).
            arg(conductivity.text);
}

QMap<QString, QString> SceneLabelCurrentMarker::data()
{
    QMap<QString, QString> out;
    out["Conductivity (S/m)"] = conductivity.number;
    return QMap<QString, QString>(out);
}

int SceneLabelCurrentMarker::showDialog(QWidget *parent)
{
    DSceneLabelCurrentMarker *dialog = new DSceneLabelCurrentMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeCurrentMarker::DSceneEdgeCurrentMarker(SceneEdgeCurrentMarker *edgeCurrentMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeCurrentMarker;

    createDialog();

    load();
    setSize();
}

DSceneEdgeCurrentMarker::~DSceneEdgeCurrentMarker()
{
    delete cmbType;
    delete txtValue;
}

void DSceneEdgeCurrentMarker::createContent()
{
    cmbType = new QComboBox(this);
    cmbType->addItem("none", PHYSICFIELDBC_NONE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_CURRENT_POTENTIAL), PHYSICFIELDBC_CURRENT_POTENTIAL);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW), PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW);

    txtValue = new SLineEditValue(this);

    layout->addWidget(new QLabel(tr("BC type:")), 1, 0);
    layout->addWidget(cmbType, 1, 1);
    layout->addWidget(new QLabel(tr("Value:")), 2, 0);
    layout->addWidget(txtValue, 2, 1);
}

void DSceneEdgeCurrentMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeCurrentMarker->type));
    txtValue->setValue(edgeCurrentMarker->value);
}

bool DSceneEdgeCurrentMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(m_edgeMarker);

    edgeCurrentMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeCurrentMarker->value = txtValue->value();
    else
        return false;

    return true;
}

// *************************************************************************************************************************************

DSceneLabelCurrentMarker::DSceneLabelCurrentMarker(QWidget *parent, SceneLabelCurrentMarker *labelCurrentMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelCurrentMarker;

    createDialog();

    load();
    setSize();
}

DSceneLabelCurrentMarker::~DSceneLabelCurrentMarker()
{
    delete txtConductivity;
}

void DSceneLabelCurrentMarker::createContent()
{
    txtConductivity = new SLineEditValue(this);

    layout->addWidget(new QLabel(tr("Conductivity (S/m):")), 1, 0);
    layout->addWidget(txtConductivity, 1, 1);
}

void DSceneLabelCurrentMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(m_labelMarker);

    txtConductivity->setValue(labelCurrentMarker->conductivity);
}

bool DSceneLabelCurrentMarker::save()
{
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(m_labelMarker);

    if (txtConductivity->evaluate())
        labelCurrentMarker->conductivity  = txtConductivity->value();
    else
        return false;

    return true;
}
