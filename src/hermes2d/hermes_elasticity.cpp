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
    double forceX;
    double forceY;

    // Lame constant
    inline double lambda() { return (young_modulus * poisson_ratio) / ((1.0 + poisson_ratio) * (1.0 - 2.0*poisson_ratio)); }
    inline double mu() { return young_modulus / (2.0*(1.0 + poisson_ratio)); }
};

ElasticityEdge *elasticityEdge;
ElasticityLabel *elasticityLabel;
bool elasticityPlanar;

BCType elasticity_bc_types_x(int marker)
{
    switch (elasticityEdge[marker].typeX)
    {
    case PhysicFieldBC_None:
        return BC_NONE;
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        return BC_ESSENTIAL;
        break;
    case PhysicFieldBC_Elasticity_Free:
        return BC_NATURAL;
        break;
    }
}

BCType elasticity_bc_types_y(int marker)
{
    switch (elasticityEdge[marker].typeY)
    {
    case PhysicFieldBC_None:
        return BC_NONE;
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        return BC_ESSENTIAL;
        break;
    case PhysicFieldBC_Elasticity_Free:
        return BC_NATURAL;
        break;
    }
}

scalar elasticity_bc_values_x(int marker, double x, double y)
{
    switch (elasticityEdge[marker].typeX)
    {
    case PhysicFieldBC_None:
        return 0;
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        return 0;
        break;
    case PhysicFieldBC_Elasticity_Free:
        return elasticityEdge[marker].forceX;
        break;
    }
}

scalar elasticity_bc_values_y(int marker, double x, double y)
{
    switch (elasticityEdge[marker].typeY)
    {
    case PhysicFieldBC_None:
        return 0;
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        return 0;
        break;
    case PhysicFieldBC_Elasticity_Free:
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


void callbackElasticitySpace(Tuple<Space *> space)
{
    space.at(0)->set_bc_types(elasticity_bc_types_x);
    space.at(0)->set_essential_bc_values(elasticity_bc_values_x);

    space.at(1)->set_bc_types(elasticity_bc_types_y);
    space.at(1)->set_essential_bc_values(elasticity_bc_values_y);
}

void callbackElasticityWeakForm(WeakForm *wf, Tuple<Solution *> slnArray)
{
    wf->add_biform(0, 0, callback(elasticity_bilinear_form_0_0), H2D_SYM);
    wf->add_biform(0, 1, callback(elasticity_bilinear_form_0_1), H2D_SYM);
    // wf->add_biform(1, 0, callback(elasticity_bilinear_form_1_0), H2D_SYM);
    wf->add_biform(1, 1, callback(elasticity_bilinear_form_1_1), H2D_SYM);
    wf->add_liform_surf(1, callback(elasticity_linear_form_surf));
}

// *******************************************************************************************************

void HermesElasticity::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC typeX = physicFieldBCFromStringKey(element->attribute("typex"));
    switch (typeX)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Elasticity_Fixed:
    case PhysicFieldBC_Elasticity_Free:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("typex")).toStdString() << endl;
        break;
    }

    PhysicFieldBC typeY = physicFieldBCFromStringKey(element->attribute("typey"));
    switch (typeY)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Elasticity_Fixed:
    case PhysicFieldBC_Elasticity_Free:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("typey")).toStdString() << endl;
        break;
    }

    if ((typeX != PhysicFieldBC_Undefined) && (typeY != PhysicFieldBC_Undefined))
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
                                                                 Value(element->attribute("poisson_ratio")),
                                                                 Value(element->attribute("forcex", "0")),
                                                                 Value(element->attribute("forcey", "0"))));
}

void HermesElasticity::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(marker);

    element->setAttribute("young_modulus", labelElasticityMarker->young_modulus.text);
    element->setAttribute("poisson_ratio", labelElasticityMarker->poisson_ratio.text);
    element->setAttribute("forcex", labelElasticityMarker->forceX.text);
    element->setAttribute("forcey", labelElasticityMarker->forceY.text);
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
    return new SurfaceIntegralValueElasticity();
}

QStringList HermesElasticity::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesElasticity::volumeIntegralValue()
{
    return new VolumeIntegralValueElasticity();
}

QStringList HermesElasticity::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S";
    return QStringList(headers);
}

SceneEdgeMarker *HermesElasticity::newEdgeMarker()
{
    return new SceneEdgeElasticityMarker(tr("new boundary"),
                                         PhysicFieldBC_Elasticity_Free,
                                         PhysicFieldBC_Elasticity_Free,
                                         Value("0"),
                                         Value("0"));
}

SceneEdgeMarker *HermesElasticity::newEdgeMarker(PyObject *self, PyObject *args)
{

    double valuex, valuey, forcex, forcey;
    char *name, *typex, *typey;
    if (PyArg_ParseTuple(args, "sssdd", &name, &typex, &typey, &forcex, &forcey))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeElasticityMarker(name,
                                             physicFieldBCFromStringKey(typex),
                                             physicFieldBCFromStringKey(typey),
                                             Value(QString::number(forcex)),
                                             Value(QString::number(forcey)));
    }

    return Util::scene()->edgeMarkers[0];
}

SceneEdgeMarker *HermesElasticity::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double valuex, valuey;
    char *name, *typex, *typey;
    if (PyArg_ParseTuple(args, "sssdd", &name, &typex, &typey, &valuex, &valuey))
    {
        if (SceneEdgeElasticityMarker *marker = dynamic_cast<SceneEdgeElasticityMarker *>(Util::scene()->getEdgeMarker(name)))
        {
            if (physicFieldBCFromStringKey(typex) && physicFieldBCFromStringKey(typey))
            {
                marker->typeX = physicFieldBCFromStringKey(typex);
                marker->typeY = physicFieldBCFromStringKey(typey);
                marker->forceX = Value(QString::number(valuex));
                marker->forceY = Value(QString::number(valuey));
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' or '%2' is not supported.").arg(typex, typey).toStdString().c_str());
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

SceneLabelMarker *HermesElasticity::newLabelMarker()
{
    return new SceneLabelElasticityMarker(tr("new material"),
                                          Value("2e11"),
                                          Value("0.33"),
                                          Value("0"),
                                          Value("0"));
}

SceneLabelMarker *HermesElasticity::newLabelMarker(PyObject *self, PyObject *args)
{
    double young_modulus, poisson_ratio, forcex, forcey;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &young_modulus, &poisson_ratio, &forcex, &forcey))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelElasticityMarker(name,
                                              Value(QString::number(young_modulus)),
                                              Value(QString::number(poisson_ratio)),
                                              Value(QString::number(forcex)),
                                              Value(QString::number(forcey)));
    }

    return NULL;
}

SceneLabelMarker *HermesElasticity::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double young_modulus, poisson_ratio;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &young_modulus, &poisson_ratio))
    {
        if (SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->young_modulus = Value(QString::number(young_modulus));
            marker->poisson_ratio = Value(QString::number(poisson_ratio));
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

    // Volumetric force
    QTreeWidgetItem *itemVolumetricForce = new QTreeWidgetItem(elasticityNode);
    itemVolumetricForce->setText(0, tr("Volumetric force"));
    itemVolumetricForce->setExpanded(true);

    addTreeWidgetItemValue(itemVolumetricForce, "f" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueElasticity->forceX, 0, 'e', 3), "N/m3");
    addTreeWidgetItemValue(itemVolumetricForce, "f" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElasticity->forceY, 0, 'e', 3), "N/m3");
}

void HermesElasticity::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    // SurfaceIntegralValueElasticity *surfaceIntegralValueElasticity = dynamic_cast<SurfaceIntegralValueElasticity *>(surfaceIntegralValue);
}

void HermesElasticity::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{

}

ViewScalarFilter *HermesElasticity::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

    return new ViewScalarFilterElasticity(sln1,
                                          sln2,
                                          physicFieldVariable,
                                          physicFieldVariableComp);
}

QList<SolutionArray *> *HermesElasticity::solve(ProgressItemSolve *progressItemSolve)
{
    // edge markers
    elasticityEdge = new ElasticityEdge[Util::scene()->edges.count()+1];
    elasticityEdge[0].typeX = PhysicFieldBC_None;
    elasticityEdge[0].typeY = PhysicFieldBC_None;
    elasticityEdge[0].forceX = 0;
    elasticityEdge[0].forceY = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            elasticityEdge[i+1].typeX = PhysicFieldBC_None;
            elasticityEdge[i+1].typeY = PhysicFieldBC_None;
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

            if (!labelElasticityMarker->forceX.evaluate()) return NULL;
            if (!labelElasticityMarker->forceY.evaluate()) return NULL;

            elasticityLabel[i].young_modulus = labelElasticityMarker->young_modulus.number;
            elasticityLabel[i].poisson_ratio = labelElasticityMarker->poisson_ratio.number;
            elasticityLabel[i].forceX = labelElasticityMarker->forceX.number;
            elasticityLabel[i].forceY = labelElasticityMarker->forceY.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = solveSolutioArray(progressItemSolve,
                                                                  callbackElasticitySpace,
                                                                  callbackElasticityWeakForm);

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

            forceX = marker->forceX.number;
            forceY = marker->forceY.number;
        }
    }
}

double LocalPointValueElasticity::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Elasticity_VonMisesStress:
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

void VolumeIntegralValueElasticity::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(0);
    sln2 = Util::scene()->sceneSolution()->sln(1);
}

QStringList VolumeIntegralValueElasticity::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterElasticity::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Elasticity_VonMisesStress:
        {
            SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

            // stress tensor
            double tz = marker->lambda() * (dudx1[i] + dudy2[i]);
            double tx = tz + 2*marker->mu() * dudx1[i];
            double ty = tz + 2*marker->mu() * dudy2[i];
            if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
                tz += 2*marker->mu() * value1[i] / x[i];
            double txy = marker->mu() * (dudy1[i] + dudx2[i]);

            // Von Mises stress
            node->values[0][0][i] = 1.0/sqrt(2.0) * sqrt(sqr(tx - ty) + sqr(ty - tz) + sqr(tz - tx) + 6*sqr(txy));
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterElasticity::calculateVariable()" << endl;
        throw;
        break;
    }
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
    case PhysicFieldBC_Elasticity_Fixed:
        out["Force X: (N)"] = forceX.number;
        break;
    }
    switch (typeY)
    {
    case PhysicFieldBC_Elasticity_Fixed:
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

SceneLabelElasticityMarker::SceneLabelElasticityMarker(const QString &name, Value young_modulus, Value poisson_ratio, Value forceX, Value forceY)
    : SceneLabelMarker(name)
{
    this->young_modulus = young_modulus;
    this->poisson_ratio = poisson_ratio;
    this->forceX = forceX;
    this->forceY = forceY;
}

QString SceneLabelElasticityMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(young_modulus.text).
            arg(poisson_ratio.text).
            arg(forceX.text).
            arg(forceY.text);
}

QMap<QString, QString> SceneLabelElasticityMarker::data()
{
    QMap<QString, QString> out;
    out["Young modulus (Pa)"] = young_modulus.number;
    out["Poisson ratio (-)"] = poisson_ratio.number;
    out["Volumetric force X (N/m3)"] = forceX.number;
    out["Volumetric force Y (N/m3)"] = forceX.number;
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

    load();
    setSize();
}

void DSceneEdgeElasticityMarker::createContent()
{
    cmbTypeX = new QComboBox(this);
    cmbTypeX->addItem("none", PhysicFieldBC_None);
    cmbTypeX->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Free), PhysicFieldBC_Elasticity_Free);
    cmbTypeX->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Fixed), PhysicFieldBC_Elasticity_Fixed);

    cmbTypeY = new QComboBox(this);
    cmbTypeY->addItem("none", PhysicFieldBC_None);
    cmbTypeY->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Free), PhysicFieldBC_Elasticity_Free);
    cmbTypeY->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Fixed), PhysicFieldBC_Elasticity_Fixed);

    txtForceX = new SLineEditValue(this);
    txtForceY = new SLineEditValue(this);

    layout->addWidget(new QLabel(tr("BC Type X:")), 1, 0);
    layout->addWidget(cmbTypeX, 1, 1);
    layout->addWidget(new QLabel(tr("BC Type Y:")), 2, 0);
    layout->addWidget(cmbTypeY, 2, 1);
    layout->addWidget(new QLabel(tr("Force X (N):")), 3, 0);
    layout->addWidget(txtForceX, 3, 1);
    layout->addWidget(new QLabel(tr("Force Y (N):")), 4, 0);
    layout->addWidget(txtForceY, 4, 1);
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

    load();
    setSize();
}

void DSceneLabelElasticityMarker::createContent()
{
    txtYoungModulus = new SLineEditValue(this);
    txtPoissonNumber = new SLineEditValue(this);
    txtForceX = new SLineEditValue(this);
    txtForceY = new SLineEditValue(this);

    layout->addWidget(new QLabel(tr("Young modulus (Pa):")), 1, 0);
    layout->addWidget(txtYoungModulus, 1, 1);
    layout->addWidget(new QLabel(tr("Poisson number (-):")), 2, 0);
    layout->addWidget(txtPoissonNumber, 2, 1);
    layout->addWidget(new QLabel(tr("Volumetric force X (N):")), 3, 0);
    layout->addWidget(txtForceX, 3, 1);
    layout->addWidget(new QLabel(tr("Volumetric force Y (N):")), 4, 0);
    layout->addWidget(txtForceY, 4, 1);
}

void DSceneLabelElasticityMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(m_labelMarker);

    txtYoungModulus->setValue(labelElasticityMarker->young_modulus);
    txtPoissonNumber->setValue(labelElasticityMarker->poisson_ratio);

    txtForceX->setValue(labelElasticityMarker->forceX);
    txtForceY->setValue(labelElasticityMarker->forceY);
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

    if (txtForceX->evaluate())
         labelElasticityMarker->forceX = txtForceX->value();
    else
         return false;

    if (txtForceY->evaluate())
         labelElasticityMarker->forceY = txtForceY->value();
    else
         return false;

    return true;
}
