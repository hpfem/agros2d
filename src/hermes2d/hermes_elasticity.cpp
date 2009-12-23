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

#include "hermes_elasticity.h"

#include "scene.h"

#include "localvalueview.h"
#include "surfaceintegralview.h"
#include "volumeintegralview.h"

struct ElasticityEdge
{
public:
    PhysicFieldBC typeX;
    PhysicFieldBC typeY;
    double forceX;
    double forceY;
};

struct ElasticityLabel
{
    double young_modulus;
    double poisson_ratio;

    // Lame constant
    inline double lambda() { return (young_modulus * poisson_ratio) / ((1 + poisson_ratio) * (1 - 2*poisson_ratio)); }
    inline double mu() { return young_modulus / (2*(1 + poisson_ratio)); }
};

ElasticityEdge *elasticityEdge;
ElasticityLabel *elasticityLabel;
bool elasticityPlanar;

int elasticity_bc_types_x(int marker)
{
    switch (elasticityEdge[marker].typeX)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return BC_NATURAL;
        break;
    }
}

int elasticity_bc_types_y(int marker)
{
    switch (elasticityEdge[marker].typeY)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return BC_NATURAL;
        break;
    }
}

scalar elasticity_bc_values_x(int marker, double x, double y)
{
    switch (elasticityEdge[marker].typeX)
    {
    case PHYSICFIELDBC_NONE:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return elasticityEdge[marker].forceX;
        break;
    }
}

scalar elasticity_bc_values_y(int marker, double x, double y)
{
    switch (elasticityEdge[marker].typeY)
    {
    case PHYSICFIELDBC_NONE:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return elasticityEdge[marker].forceY;
        break;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_0_0(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return (elasticityLabel[e->marker].lambda() + 2*elasticityLabel[e->marker].mu()) * int_dudx_dvdx<Real, Scalar>(n, wt, u, v) +
            elasticityLabel[e->marker].mu() * int_dudy_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_0_1(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return elasticityLabel[e->marker].lambda() * int_dudy_dvdx<Real, Scalar>(n, wt, u, v) +
            elasticityLabel[e->marker].mu() * int_dudx_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_1_0(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return  elasticityLabel[e->marker].mu() * int_dudy_dvdx<Real, Scalar>(n, wt, u, v) +
            elasticityLabel[e->marker].lambda() * int_dudx_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_1_1(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return   elasticityLabel[e->marker].mu() * int_dudx_dvdx<Real, Scalar>(n, wt, u, v) +
            (elasticityLabel[e->marker].lambda() + 2*elasticityLabel[e->marker].mu()) * int_dudy_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_linear_form_surf(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return elasticityEdge[e->marker].forceY * int_v<Real, Scalar>(n, wt, v);
}

QList<SolutionArray *> *elasticity_main(SolverDialog *solverDialog)
{
    elasticityPlanar = (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo()->numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    int adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh xmesh, ymesh;
    xmesh.load((tempProblemFileName() + ".mesh").toStdString().c_str());
    for (int i = 0; i < numberOfRefinements; i++)
        xmesh.refine_all_elements(0);
    ymesh.copy(&xmesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    // initialize the shapeset and the cache
    H1Shapeset shapeset;
    PrecalcShapeset xpss(&shapeset);
    PrecalcShapeset ypss(&shapeset);

    // create the x displacement space
    H1Space xdisp(&xmesh, &shapeset);
    xdisp.set_bc_types(elasticity_bc_types_x);
    xdisp.set_bc_values(elasticity_bc_values_x);
    xdisp.set_uniform_order(polynomialOrder);
    int ndof = xdisp.assign_dofs(0);

    // create the y displacement space
    H1Space ydisp(&ymesh, &shapeset);
    ydisp.set_bc_types(elasticity_bc_types_y);
    ydisp.set_bc_values(elasticity_bc_values_y);
    ydisp.set_uniform_order(polynomialOrder);
    ydisp.assign_dofs();

    // initialize the weak formulation
    WeakForm wf(2);
    wf.add_biform(0, 0, callback(elasticity_bilinear_form_0_0), SYM);
    wf.add_biform(0, 1, callback(elasticity_bilinear_form_0_1), SYM);
    // wf.add_biform(1, 0, callback(elasticity_bilinear_form_1_0), SYM);
    wf.add_biform(1, 1, callback(elasticity_bilinear_form_1_1), SYM);
    wf.add_liform_surf(1, callback(elasticity_linear_form_surf));

    // initialize the linear system and solver
    UmfpackSolver umfpack;
    LinSystem sys(&wf, &umfpack);
    sys.set_spaces(2, &xdisp, &ydisp);
    sys.set_pss(2, &xpss, &ypss);

    // assemble the stiffness matrix and solve the system
    Solution *slnx = new Solution();
    Solution *slny = new Solution();
    sys.assemble();
    sys.solve(2, slnx, slny);

    // output
    xdisp.assign_dofs();
    ydisp.assign_dofs();


    // output
    SolutionArray *solutionArray;
    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();

    // x part
    solutionArray = new SolutionArray();
    solutionArray->order = new Orderizer();
    solutionArray->order->process_solution(&xdisp);
    solutionArray->sln = slnx;
    // solutionArray->adaptiveError = error;
    // solutionArray->adaptiveSteps = i-1;

    solutionArrayList->append(solutionArray);

    // y part
    solutionArray = new SolutionArray();
    solutionArray->order = new Orderizer();
    solutionArray->order->process_solution(&ydisp);
    solutionArray->sln = slny;
    // solutionArray->adaptiveError = error;
    // solutionArray->adaptiveSteps = i-1;

    solutionArrayList->append(solutionArray);

    return solutionArrayList;
}

// *******************************************************************************************************

void HermesElasticity::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC typeX = physicFieldBCFromStringKey(element->attribute("typex"));
    switch (typeX)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_ELASTICITY_FIXED:
    case PHYSICFIELDBC_ELASTICITY_FREE:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("typex")).toStdString() << endl;
        break;
    }

    PhysicFieldBC typeY = physicFieldBCFromStringKey(element->attribute("typey"));
    switch (typeY)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_ELASTICITY_FIXED:
    case PHYSICFIELDBC_ELASTICITY_FREE:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("typey")).toStdString() << endl;
        break;
    }

    if ((typeX != PHYSICFIELDBC_UNDEFINED) && (typeY != PHYSICFIELDBC_UNDEFINED))
        Util::scene()->addEdgeMarker(new SceneEdgeElasticityMarker(element->attribute("name"),
                                                                   typeX, typeY,
                                                                   Value(element->attribute("forcex", "0")),
                                                                   Value(element->attribute("forcey", "0"))));
}

void HermesElasticity::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(marker);

    element->setAttribute("typex", physicFieldBCToStringKey(edgeElasticityMarker->typeX));
    element->setAttribute("typey", physicFieldBCToStringKey(edgeElasticityMarker->typeY));
    element->setAttribute("forcex", edgeElasticityMarker->forceX.text);
    element->setAttribute("forcey", edgeElasticityMarker->forceY.text);
}

void HermesElasticity::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelElasticityMarker(element->attribute("name"),
                                                                 Value(element->attribute("young_modulus")),
                                                                 Value(element->attribute("poisson_ratio"))));
}

void HermesElasticity::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelElasticityMarker *labelHeatMarker = dynamic_cast<SceneLabelElasticityMarker *>(marker);

    element->setAttribute("young_modulus", labelHeatMarker->young_modulus.text);
    element->setAttribute("poisson_ratio", labelHeatMarker->poisson_ratio.text);
}

LocalPointValue *HermesElasticity::localPointValue(Point point)
{
    return new LocalPointValueElasticity(point);
}

QStringList HermesElasticity::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "Von Misses stress";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesElasticity::surfaceIntegralValue()
{
    return NULL; // new SurfaceIntegralValueElectrostatic();
}

QStringList HermesElasticity::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesElasticity::volumeIntegralValue()
{
    return new VolumeIntegralValueElectrostatic();
}

QStringList HermesElasticity::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S";
    return QStringList(headers);
}

SceneEdgeMarker *HermesElasticity::newEdgeMarker()
{
    return new SceneEdgeElasticityMarker("new boundary",
                                         PHYSICFIELDBC_ELASTICITY_FREE,
                                         PHYSICFIELDBC_ELASTICITY_FREE,
                                         Value("0"),
                                         Value("0"));
}

SceneEdgeMarker *HermesElasticity::newEdgeMarker(PyObject *self, PyObject *args)
{

    double valuex, valuey;
    char *name, *typex, *typey;
    if (PyArg_ParseTuple(args, "sssdd", &name, &typex, &typey, &valuex, &valuey))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeElasticityMarker(name,
                                             physicFieldBCFromStringKey(typex),
                                             physicFieldBCFromStringKey(typey),
                                             Value(QString::number(valuex)),
                                             Value(QString::number(valuey)));
    }

    return Util::scene()->edgeMarkers[0];
}

SceneLabelMarker *HermesElasticity::newLabelMarker()
{
    return new SceneLabelElasticityMarker("new material",
                                          Value("2e11"),
                                          Value("0.33"));
}

SceneLabelMarker *HermesElasticity::newLabelMarker(PyObject *self, PyObject *args)
{
    double young_modulus, poisson_ration;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &young_modulus, &poisson_ration))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelElasticityMarker(name,
                                              Value(QString::number(young_modulus)),
                                              Value(QString::number(poisson_ration)));
    }

    return NULL;
}

void HermesElasticity::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueElasticity *localPointValueElasticity = dynamic_cast<LocalPointValueElasticity *>(localPointValue);

    // elasticity
    QTreeWidgetItem *elasticityNode = new QTreeWidgetItem(trvWidget);
    elasticityNode->setText(0, tr("Elasticity"));
    elasticityNode->setExpanded(true);

    // Young modulus
    addTreeWidgetItemValue(elasticityNode, tr("Young modulus:"), QString("%1").arg(localPointValueElasticity->young_modulus, 0, 'e', 3), "Pa");

    // Poisson ratio
    addTreeWidgetItemValue(elasticityNode, tr("Poisson ratio:"), QString("%1").arg(localPointValueElasticity->poisson_ratio, 0, 'f', 3), "");

    // Von Mises stress
    addTreeWidgetItemValue(elasticityNode, tr("Von Mises stress:"), QString("%1").arg(localPointValueElasticity->von_mises_stress, 0, 'e', 3), "Pa");
}

void HermesElasticity::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    // SurfaceIntegralValueElasticity *surfaceIntegralValueElasticity = dynamic_cast<SurfaceIntegralValueElasticity *>(surfaceIntegralValue);
}

void HermesElasticity::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{

}

QList<SolutionArray *> *HermesElasticity::solve(SolverDialog *solverDialog)
{
    // edge markers
    elasticityEdge = new ElasticityEdge[Util::scene()->edges.count()+1];
    elasticityEdge[0].typeX = PHYSICFIELDBC_NONE;
    elasticityEdge[0].typeY = PHYSICFIELDBC_NONE;
    elasticityEdge[0].forceX = 0;
    elasticityEdge[0].forceY = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            elasticityEdge[i+1].typeX = PHYSICFIELDBC_NONE;
            elasticityEdge[i+1].typeY = PHYSICFIELDBC_NONE;
            elasticityEdge[i+1].forceX = 0;
            elasticityEdge[i+1].forceY = 0;
        }
        else
        {
            SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(Util::scene()->edges[i]->marker);
            elasticityEdge[i+1].typeX = edgeElasticityMarker->typeX;
            elasticityEdge[i+1].typeY = edgeElasticityMarker->typeY;

            if (!edgeElasticityMarker->forceX.evaluate()) return NULL;
            if (!edgeElasticityMarker->forceY.evaluate()) return NULL;

            elasticityEdge[i+1].forceX = edgeElasticityMarker->forceX.number;
            elasticityEdge[i+1].forceY = edgeElasticityMarker->forceY.number;
        }
    }

    // label markers
    elasticityLabel = new ElasticityLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(Util::scene()->labels[i]->marker);

            if (!labelElasticityMarker->young_modulus.evaluate()) return NULL;
            if (!labelElasticityMarker->poisson_ratio.evaluate()) return NULL;

            elasticityLabel[i].young_modulus = labelElasticityMarker->young_modulus.number;
            elasticityLabel[i].poisson_ratio = labelElasticityMarker->poisson_ratio.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = elasticity_main(solverDialog);

    delete [] elasticityEdge;
    delete [] elasticityLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueElasticity::LocalPointValueElasticity(Point &point) : LocalPointValue(point)
{
    von_mises_stress = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (labelMarker)
        {
            // Von Mises stress
            von_mises_stress = value;

            SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

            young_modulus = marker->young_modulus.number;
            poisson_ratio = marker->poisson_ratio.number;
        }
    }
}

double LocalPointValueElasticity::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        {
            return von_mises_stress;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueHeat::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueElasticity::variables()
{
    QStringList row;
    row << QString("%1").arg(von_mises_stress, 0, 'e', 5);

    return QStringList(row);
}

// *************************************************************************************************************************************

SurfaceIntegralValueElasticity::SurfaceIntegralValueElasticity() : SurfaceIntegralValue()
{
    calculate();
}

void SurfaceIntegralValueElasticity::calculateVariables(int i)
{

}

QStringList SurfaceIntegralValueElasticity::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueElasticity::VolumeIntegralValueElasticity() : VolumeIntegralValue()
{
    calculate();
}

void VolumeIntegralValueElasticity::calculateVariables(int i)
{

}

QStringList VolumeIntegralValueElasticity::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SceneEdgeElasticityMarker::SceneEdgeElasticityMarker(const QString &name, PhysicFieldBC typeX, PhysicFieldBC typeY, Value forceX, Value forceY)
    : SceneEdgeMarker(name, typeX)
{
    this->typeX = typeX;
    this->typeY = typeY;
    this->forceX = forceX;
    this->forceY = forceY;
}

QString SceneEdgeElasticityMarker::script()
{
    return QString("addEdge(\"%1\", \"%2\", \"%3\", %4, %5)").
            arg(name).
            arg(physicFieldBCToStringKey(typeX)).
            arg(physicFieldBCToStringKey(typeY)).
            arg(forceX.text).
            arg(forceY.text);
}

QMap<QString, QString> SceneEdgeElasticityMarker::data()
{
    QMap<QString, QString> out;
    switch (typeX)
    {
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        out["Force X: (N)"] = forceX.number;
        break;
    }
    switch (typeY)
    {
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        out["Force Y: (N)"] = forceY.number;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeElasticityMarker::showDialog(QWidget *parent)
{
    DSceneEdgeElasticityMarker *dialog = new DSceneEdgeElasticityMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelElasticityMarker::SceneLabelElasticityMarker(const QString &name, Value young_modulus, Value poisson_ratio)
    : SceneLabelMarker(name)
{
    this->young_modulus = young_modulus;
    this->poisson_ratio = poisson_ratio;
}

QString SceneLabelElasticityMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(young_modulus.text).
            arg(poisson_ratio.text);
}

QMap<QString, QString> SceneLabelElasticityMarker::data()
{
    QMap<QString, QString> out;
    out["Young modulus (Pa)"] = young_modulus.number;
    out["Poisson ratio (-)"] = poisson_ratio.number;
    return QMap<QString, QString>(out);
}

int SceneLabelElasticityMarker::showDialog(QWidget *parent)
{
    DSceneLabelElasticityMarker *dialog = new DSceneLabelElasticityMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeElasticityMarker::DSceneEdgeElasticityMarker(SceneEdgeElasticityMarker *edgeEdgeElasticityMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeEdgeElasticityMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, cmbTypeX);
    setTabOrder(cmbTypeX, txtForceX);

    load();
    setSize();
}

DSceneEdgeElasticityMarker::~DSceneEdgeElasticityMarker()
{
    delete cmbTypeX;
    delete cmbTypeY;
    delete txtForceX;
    delete txtForceY;
}

QLayout* DSceneEdgeElasticityMarker::createContent()
{
    cmbTypeX = new QComboBox();
    cmbTypeX->addItem("none", PHYSICFIELDBC_NONE);
    cmbTypeX->addItem(physicFieldBCString(PHYSICFIELDBC_ELASTICITY_FREE), PHYSICFIELDBC_ELASTICITY_FREE);
    cmbTypeX->addItem(physicFieldBCString(PHYSICFIELDBC_ELASTICITY_FIXED), PHYSICFIELDBC_ELASTICITY_FIXED);

    cmbTypeY = new QComboBox();
    cmbTypeY->addItem("none", PHYSICFIELDBC_NONE);
    cmbTypeY->addItem(physicFieldBCString(PHYSICFIELDBC_ELASTICITY_FREE), PHYSICFIELDBC_ELASTICITY_FREE);
    cmbTypeY->addItem(physicFieldBCString(PHYSICFIELDBC_ELASTICITY_FIXED), PHYSICFIELDBC_ELASTICITY_FIXED);

    txtForceX = new SLineEditValue(this);
    txtForceY = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("BC Type X:"), cmbTypeX);
    layoutMarker->addRow(tr("BC Type Y:"), cmbTypeY);
    layoutMarker->addRow(tr("Force X:"), txtForceX);
    layoutMarker->addRow(tr("Force Y:"), txtForceY);

    return layoutMarker;
}

void DSceneEdgeElasticityMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(m_edgeMarker);

    cmbTypeX->setCurrentIndex(cmbTypeX->findData(edgeElasticityMarker->typeX));
    cmbTypeY->setCurrentIndex(cmbTypeY->findData(edgeElasticityMarker->typeY));

    txtForceX->setValue(edgeElasticityMarker->forceX);
    txtForceY->setValue(edgeElasticityMarker->forceY);
}

bool DSceneEdgeElasticityMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(m_edgeMarker);

    edgeElasticityMarker->typeX = (PhysicFieldBC) cmbTypeX->itemData(cmbTypeX->currentIndex()).toInt();
    edgeElasticityMarker->typeY = (PhysicFieldBC) cmbTypeY->itemData(cmbTypeY->currentIndex()).toInt();

    if (txtForceX->evaluate())
        edgeElasticityMarker->forceX = txtForceX->value();
    else
        return false;

    if (txtForceY->evaluate())
        edgeElasticityMarker->forceY = txtForceY->value();
    else
        return false;

    return true;
}

// *************************************************************************************************************************************

DSceneLabelElasticityMarker::DSceneLabelElasticityMarker(QWidget *parent, SceneLabelElasticityMarker *labelElasticityMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelElasticityMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, txtYoungModulus);
    setTabOrder(txtYoungModulus, txtPoissonNumber);

    load();
    setSize();
}

DSceneLabelElasticityMarker::~DSceneLabelElasticityMarker()
{
    delete txtYoungModulus;
    delete txtPoissonNumber;
}

QLayout* DSceneLabelElasticityMarker::createContent()
{
    txtYoungModulus = new SLineEditValue();
    txtPoissonNumber = new SLineEditValue();

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("Young modulus (Pa):"), txtYoungModulus);
    layoutMarker->addRow(tr("Poisson number (-):"), txtPoissonNumber);

    return layoutMarker;
}

void DSceneLabelElasticityMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(m_labelMarker);

    txtYoungModulus->setValue(labelElasticityMarker->young_modulus);
    txtPoissonNumber->setValue(labelElasticityMarker->poisson_ratio);
}

bool DSceneLabelElasticityMarker::save()
{
    if (!DSceneLabelMarker::save()) return false;

    SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(m_labelMarker);

    if (txtYoungModulus->evaluate())
        labelElasticityMarker->young_modulus = txtYoungModulus->value();
    else
        return false;

    if (txtPoissonNumber->evaluate())
        labelElasticityMarker->poisson_ratio = txtPoissonNumber->value();
    else
        return false;

    return true;
}
