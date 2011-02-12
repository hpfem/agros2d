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
#include "gui.h"


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
    double alpha;
    double temp;
    double temp_ref;

    // Lame constant
    inline double lambda() { return (young_modulus * poisson_ratio) / ((1.0 + poisson_ratio) * (1.0 - 2.0*poisson_ratio)); }
    inline double mu() { return young_modulus / (2.0*(1.0 + poisson_ratio)); }
};

ElasticityEdge *elasticityEdge;
ElasticityLabel *elasticityLabel;
bool elasticityPlanar;

template<typename Real, typename Scalar>
Scalar elasticity_matrix_form_linear_x_x(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return (elasticityLabel[e->elem_marker].lambda() + 2*elasticityLabel[e->elem_marker].mu()) * int_dudx_dvdx<Real, Scalar>(n, wt, u, v) +
                elasticityLabel[e->elem_marker].mu() * int_dudy_dvdy<Real, Scalar>(n, wt, u, v);
    else
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * (e->x[i] * elasticityLabel[e->elem_marker].lambda() * (u->dx[i] * v->dx[i] +
                                                                                     u->val[i]/e->x[i] * v->dx[i] +
                                                                                     u->dx[i] * v->val[i]/e->x[i] +
                                                                                     1/sqr(e->x[i]) * u->val[i] * v->val[i]) +
                               e->x[i] * elasticityLabel[e->elem_marker].mu() * (2 * u->dx[i] * v->dx[i] +
                                                                                 2 * 1/sqr(e->x[i]) * u->val[i] * v->val[i] +
                                                                                 u->dy[i] * v->dy[i]));
        return result;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_matrix_form_linear_x_y(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return elasticityLabel[e->elem_marker].lambda() * int_dudy_dvdx<Real, Scalar>(n, wt, u, v) +
                elasticityLabel[e->elem_marker].mu() * int_dudx_dvdy<Real, Scalar>(n, wt, u, v);
    else
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * (e->x[i] * elasticityLabel[e->elem_marker].lambda() * (u->dy[i] * v->dx[i] +
                                                                                     u->dy[i] * v->val[i]/e->x[i]) +
                               e->x[i] * elasticityLabel[e->elem_marker].mu() * u->dx[i] * v->dy[i]);
        return result;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_matrix_form_linear_y_x(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return  elasticityLabel[e->elem_marker].mu() * int_dudy_dvdx<Real, Scalar>(n, wt, u, v) +
                elasticityLabel[e->elem_marker].lambda() * int_dudx_dvdy<Real, Scalar>(n, wt, u, v);
    else
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * (e->x[i] * elasticityLabel[e->elem_marker].lambda() * (u->dx[i] * v->dy[i] +
                                                                                     u->val[i]/e->x[i] * v->dy[i]) +
                               e->x[i] * elasticityLabel[e->elem_marker].mu() * u->dy[i] * v->dx[i]);
        return result;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_matrix_form_linear_y_y(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return   elasticityLabel[e->elem_marker].mu() * int_dudx_dvdx<Real, Scalar>(n, wt, u, v) +
                (elasticityLabel[e->elem_marker].lambda() + 2*elasticityLabel[e->elem_marker].mu()) * int_dudy_dvdy<Real, Scalar>(n, wt, u, v);
    else
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * (e->x[i] * elasticityLabel[e->elem_marker].lambda() * (u->dy[i] * v->dy[i]) +
                               e->x[i] * elasticityLabel[e->elem_marker].mu() * (u->dx[i] * v->dx[i] +
                                                                                 2 * u->dy[i] * v->dy[i]));
        return result;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_vector_form_x(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * ((elasticityLabel[e->elem_marker].forceX * v->val[i]) +
                               ((3 * elasticityLabel[e->elem_marker].lambda() + 2 * elasticityLabel[e->elem_marker].mu()) *
                                elasticityLabel[e->elem_marker].alpha * (elasticityLabel[e->elem_marker].temp - elasticityLabel[e->elem_marker].temp_ref) * v->dx[i]));
        return result;
    }
    else
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * (e->x[i] * (elasticityLabel[e->elem_marker].forceX * v->val[i]) +
                               ((3 * elasticityLabel[e->elem_marker].lambda() + 2 * elasticityLabel[e->elem_marker].mu()) *
                                elasticityLabel[e->elem_marker].alpha * (elasticityLabel[e->elem_marker].temp - elasticityLabel[e->elem_marker].temp_ref) * v->dx[i]));
        return result;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_vector_form_y(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * ((elasticityLabel[e->elem_marker].forceY * v->val[i]) +
                               ((3 * elasticityLabel[e->elem_marker].lambda() + 2 * elasticityLabel[e->elem_marker].mu()) *
                                elasticityLabel[e->elem_marker].alpha * (elasticityLabel[e->elem_marker].temp - elasticityLabel[e->elem_marker].temp_ref) * v->dy[i]));
        return result;
    }
    else
    {
        Scalar result = 0;
        for (int i = 0; i < n; i++)
            result += wt[i] * e->x[i] * ((elasticityLabel[e->elem_marker].forceY * v->val[i]) +
                                         ((3 * elasticityLabel[e->elem_marker].lambda() + 2 * elasticityLabel[e->elem_marker].mu()) *
                                          elasticityLabel[e->elem_marker].alpha * (elasticityLabel[e->elem_marker].temp - elasticityLabel[e->elem_marker].temp_ref) * v->dy[i]));
        return result;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_vector_form_x_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return elasticityEdge[e->edge_marker].forceX * int_v<Real, Scalar>(n, wt, v);
    else
        return elasticityEdge[e->edge_marker].forceX * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar elasticity_vector_form_y_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return elasticityEdge[e->edge_marker].forceY * int_v<Real, Scalar>(n, wt, v);
    else
        return elasticityEdge[e->edge_marker].forceY * int_x_v<Real, Scalar>(n, wt, v, e);
}

void callbackElasticityWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    wf->add_matrix_form(0, 0, callback(elasticity_matrix_form_linear_x_x));
    wf->add_matrix_form(0, 1, callback(elasticity_matrix_form_linear_x_y), HERMES_SYM);
    wf->add_matrix_form(1, 1, callback(elasticity_matrix_form_linear_y_y));
    wf->add_vector_form(0, callback(elasticity_vector_form_x));
    wf->add_vector_form(1, callback(elasticity_vector_form_y));
    wf->add_vector_form_surf(0, callback(elasticity_vector_form_x_surf));
    wf->add_vector_form_surf(1, callback(elasticity_vector_form_y_surf));
}

// *******************************************************************************************************

void HermesElasticity::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_Displacement), PhysicFieldVariable_Elasticity_Displacement);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_VonMisesStress), PhysicFieldVariable_Elasticity_VonMisesStress);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StressXX), PhysicFieldVariable_Elasticity_StressXX);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StressYY), PhysicFieldVariable_Elasticity_StressYY);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StressZZ), PhysicFieldVariable_Elasticity_StressZZ);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StressXY), PhysicFieldVariable_Elasticity_StressXY);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StrainXX), PhysicFieldVariable_Elasticity_StrainXX);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StrainYY), PhysicFieldVariable_Elasticity_StrainYY);
    if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StrainZZ), PhysicFieldVariable_Elasticity_StrainZZ);
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_StrainXY), PhysicFieldVariable_Elasticity_StrainXY);
}

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
                                                                   Value(element->attribute("forcey", "0")),
                                                                   Value(element->attribute("displacementx", "0")),
                                                                   Value(element->attribute("displacementx", "0"))));
}

void HermesElasticity::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(marker);

    element->setAttribute("typex", physicFieldBCToStringKey(edgeElasticityMarker->typeX));
    element->setAttribute("typey", physicFieldBCToStringKey(edgeElasticityMarker->typeY));
    element->setAttribute("forcex", edgeElasticityMarker->forceX.text());
    element->setAttribute("forcey", edgeElasticityMarker->forceY.text());
}

void HermesElasticity::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelElasticityMarker(element->attribute("name"),
                                                                 Value(element->attribute("young_modulus")),
                                                                 Value(element->attribute("poisson_ratio")),
                                                                 Value(element->attribute("forcex", "0")),
                                                                 Value(element->attribute("forcey", "0")),
                                                                 Value(element->attribute("alpha", "0")),
                                                                 Value(element->attribute("temp", "0")),
                                                                 Value(element->attribute("temp_ref", "0"))));
}

void HermesElasticity::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(marker);

    element->setAttribute("young_modulus", labelElasticityMarker->young_modulus.text());
    element->setAttribute("poisson_ratio", labelElasticityMarker->poisson_ratio.text());
    element->setAttribute("forcex", labelElasticityMarker->forceX.text());
    element->setAttribute("forcey", labelElasticityMarker->forceY.text());
    element->setAttribute("alpha", labelElasticityMarker->alpha.text());
    element->setAttribute("temp", labelElasticityMarker->temp.text());
    element->setAttribute("temp_ref", labelElasticityMarker->temp_ref.text());
}

LocalPointValue *HermesElasticity::localPointValue(Point point)
{
    return new LocalPointValueElasticity(point);
}

QStringList HermesElasticity::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "mises" << "tresca" << "u" << "v" << "disp"
            << "sxx" << "syy" << "szz" << "sxy"
            << "exx" << "eyy" << "ezz" << "exy";
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
                                         Value("0"),
                                         Value("0"),
                                         Value("0"));
}

SceneEdgeMarker *HermesElasticity::newEdgeMarker(PyObject *self, PyObject *args)
{

    double forcex, forcey, displacementx, displacementy;
    char *name, *typex, *typey;
    if (PyArg_ParseTuple(args, "sssdddd", &name, &typex, &typey, &forcex, &forcey, &displacementx, &displacementy))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeElasticityMarker(name,
                                             physicFieldBCFromStringKey(typex),
                                             physicFieldBCFromStringKey(typey),
                                             Value(QString::number(forcex)),
                                             Value(QString::number(forcey)),
                                             Value(QString::number(displacementx)),
                                             Value(QString::number(displacementy)));
    }

    return Util::scene()->edgeMarkers[0];
}

SceneEdgeMarker *HermesElasticity::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double forcex, forcey, displacementx, displacementy;
    char *name, *typex, *typey;
    if (PyArg_ParseTuple(args, "sssdddd", &name, &typex, &typey, &forcex, &forcey, &displacementx, &displacementy))
    {
        if (SceneEdgeElasticityMarker *marker = dynamic_cast<SceneEdgeElasticityMarker *>(Util::scene()->getEdgeMarker(name)))
        {
            if (physicFieldBCFromStringKey(typex) && physicFieldBCFromStringKey(typey))
            {
                marker->typeX = physicFieldBCFromStringKey(typex);
                marker->typeY = physicFieldBCFromStringKey(typey);
                marker->forceX = Value(QString::number(forcex));
                marker->forceY = Value(QString::number(forcey));
                marker->displacementX = Value(QString::number(displacementx));
                marker->displacementY = Value(QString::number(displacementy));
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
                                          Value("0"),
                                          Value("1.2e-5"),
                                          Value("0"),
                                          Value("0"));
}

SceneLabelMarker *HermesElasticity::newLabelMarker(PyObject *self, PyObject *args)
{
    double young_modulus, poisson_ratio, forcex, forcey, alpha, temp, temp_ref;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddd", &name, &young_modulus, &poisson_ratio, &forcex, &forcey, &alpha, &temp, &temp_ref))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelElasticityMarker(name,
                                              Value(QString::number(young_modulus)),
                                              Value(QString::number(poisson_ratio)),
                                              Value(QString::number(forcex)),
                                              Value(QString::number(forcey)),
                                              Value(QString::number(alpha)),
                                              Value(QString::number(temp)),
                                              Value(QString::number(temp_ref)));
    }

    return NULL;
}

SceneLabelMarker *HermesElasticity::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double young_modulus, poisson_ratio, forcex, forcey, alpha, temp, temp_ref;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddd", &name, &young_modulus, &poisson_ratio, &forcex, &forcey, &alpha, &temp, &temp_ref))
    {
        if (SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->young_modulus = Value(QString::number(young_modulus));
            marker->poisson_ratio = Value(QString::number(poisson_ratio));
            marker->forceX = Value(QString::number(forcex));
            marker->forceY = Value(QString::number(forcey));
            marker->alpha = Value(QString::number(alpha));
            marker->temp = Value(QString::number(temp));
            marker->temp_ref = Value(QString::number(temp_ref));
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

    // Volumetric force
    QTreeWidgetItem *itemVolumetricForce = new QTreeWidgetItem(elasticityNode);
    itemVolumetricForce->setText(0, tr("Volumetric force"));
    itemVolumetricForce->setExpanded(true);

    addTreeWidgetItemValue(itemVolumetricForce, "f" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueElasticity->forceX, 0, 'e', 3), "N/m3");
    addTreeWidgetItemValue(itemVolumetricForce, "f" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElasticity->forceY, 0, 'e', 3), "N/m3");

    // Thermal expansion
    QTreeWidgetItem *itemThermalExpansion = new QTreeWidgetItem(elasticityNode);
    itemThermalExpansion->setText(0, tr("Thermal expansion"));
    itemThermalExpansion->setExpanded(true);

    addTreeWidgetItemValue(itemThermalExpansion, "Thermal exp. coef.:", QString("%1").arg(localPointValueElasticity->alpha, 0, 'e', 3), "1/deg");
    addTreeWidgetItemValue(itemThermalExpansion, "Temperature:", QString("%1").arg(localPointValueElasticity->temp, 0, 'e', 3), "deg");
    addTreeWidgetItemValue(itemThermalExpansion, "Ref. temperature:", QString("%1").arg(localPointValueElasticity->temp_ref, 0, 'e', 3), "deg");

    // Von Mises stress
    addTreeWidgetItemValue(elasticityNode, tr("Von Mises stress:"), QString("%1").arg(localPointValueElasticity->von_mises_stress, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(elasticityNode, tr("Tresca stress:"), QString("%1").arg(localPointValueElasticity->tresca_stress, 0, 'e', 3), "Pa");

    // Displacement
    QTreeWidgetItem *itemDisplacement = new QTreeWidgetItem(elasticityNode);
    itemDisplacement->setText(0, tr("Displacement"));
    itemDisplacement->setExpanded(true);

    addTreeWidgetItemValue(itemDisplacement, "D" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueElasticity->d.x, 0, 'e', 3), "m");
    addTreeWidgetItemValue(itemDisplacement, "D" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElasticity->d.y, 0, 'e', 3), "m");
    addTreeWidgetItemValue(itemDisplacement, "D:", QString("%1").arg(localPointValueElasticity->d.magnitude(), 0, 'e', 3), "m");

    // Stresses
    QTreeWidgetItem *itemStresses = new QTreeWidgetItem(elasticityNode);
    itemStresses->setText(0, tr("Stresses"));
    itemStresses->setExpanded(true);

    addTreeWidgetItemValue(itemStresses, "s" + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueElasticity->stress_x, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(itemStresses, "s" + Util::scene()->problemInfo()->labelY().toLower() + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElasticity->stress_y, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(itemStresses, "s" + Util::scene()->problemInfo()->labelZ().toLower() + Util::scene()->problemInfo()->labelZ().toLower() + ":", QString("%1").arg(localPointValueElasticity->stress_z, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(itemStresses, "s" + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElasticity->stress_xy, 0, 'e', 3), "Pa");

    // Strains
    QTreeWidgetItem *itemStrains = new QTreeWidgetItem(elasticityNode);
    itemStrains->setText(0, tr("Strains"));
    itemStrains->setExpanded(true);

    addTreeWidgetItemValue(itemStrains, "e" + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueElasticity->strain_x, 0, 'e', 3), "");
    addTreeWidgetItemValue(itemStrains, "e" + Util::scene()->problemInfo()->labelY().toLower() + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElasticity->strain_y, 0, 'e', 3), "");
    if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
        addTreeWidgetItemValue(itemStrains, "e" + Util::scene()->problemInfo()->labelZ().toLower() + Util::scene()->problemInfo()->labelZ().toLower() + ":", QString("%1").arg(localPointValueElasticity->strain_z, 0, 'e', 3), "");
    addTreeWidgetItemValue(itemStrains, "e" + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueElasticity->strain_xy, 0, 'e', 3), "");
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

    return new ViewScalarFilterElasticity(Hermes::vector<MeshFunction *>(sln1, sln2),
                                          physicFieldVariable,
                                          physicFieldVariableComp);
}

template <class T>
void deformShapeTemplate(T linVert, int count)
{
    double min = numeric_limits<double>::max();
    double max = -numeric_limits<double>::max();
    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = Util::scene()->sceneSolution()->sln(0)->get_pt_value(x, y);
        double dy = Util::scene()->sceneSolution()->sln(1)->get_pt_value(x, y);

        double dm = sqrt(sqr(dx) + sqr(dy));

        if (dm < min) min = dm;
        if (dm > max) max = dm;
    }

    RectPoint rect = Util::scene()->boundingBox();
    double k = qMax(rect.width(), rect.height()) / qMax(min, max) / 15.0;

    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = Util::scene()->sceneSolution()->sln(0)->get_pt_value(x, y);
        double dy = Util::scene()->sceneSolution()->sln(1)->get_pt_value(x, y);

        linVert[i][0] += k*dx;
        linVert[i][1] += k*dy;
    }
}

void HermesElasticity::deformShape(double3* linVert, int count)
{
    deformShapeTemplate<double3 *>(linVert, count);
}

void HermesElasticity::deformShape(double4* linVert, int count)
{
    deformShapeTemplate<double4 *>(linVert, count);
}

QList<SolutionArray *> HermesElasticity::solve(ProgressItemSolve *progressItemSolve)
{
    // edge markers
    BCTypes bcTypesX, bcTypesY;
    BCValues bcValuesX, bcValuesY;

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

            if (!edgeElasticityMarker->forceX.evaluate()) return QList<SolutionArray *>();
            if (!edgeElasticityMarker->forceY.evaluate()) return QList<SolutionArray *>();

            elasticityEdge[i+1].forceX = edgeElasticityMarker->forceX.number();
            elasticityEdge[i+1].forceY = edgeElasticityMarker->forceY.number();

            switch (edgeElasticityMarker->typeX)
            {
            case PhysicFieldBC_None:
                bcTypesX.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Elasticity_Fixed:
                bcTypesX.add_bc_dirichlet(i+1);
                bcValuesX.add_const(i+1, edgeElasticityMarker->displacementX.number());
                break;
            case PhysicFieldBC_Elasticity_Free:
                bcTypesX.add_bc_neumann(i+1);
                break;
            }

            switch (edgeElasticityMarker->typeY)
            {
            case PhysicFieldBC_None:
                bcTypesY.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Elasticity_Fixed:
                bcTypesY.add_bc_dirichlet(i+1);
                bcValuesY.add_const(i+1, edgeElasticityMarker->displacementY.number());
                break;
            case PhysicFieldBC_Elasticity_Free:
                bcTypesY.add_bc_neumann(i+1);
                break;
            }
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

            if (!labelElasticityMarker->young_modulus.evaluate()) return QList<SolutionArray *>();
            if (!labelElasticityMarker->poisson_ratio.evaluate()) return QList<SolutionArray *>();

            if (!labelElasticityMarker->forceX.evaluate()) return QList<SolutionArray *>();
            if (!labelElasticityMarker->forceY.evaluate()) return QList<SolutionArray *>();

            if (!labelElasticityMarker->alpha.evaluate()) return QList<SolutionArray *>();
            if (!labelElasticityMarker->temp.evaluate()) return QList<SolutionArray *>();
            if (!labelElasticityMarker->temp_ref.evaluate()) return QList<SolutionArray *>();

            elasticityLabel[i].young_modulus = labelElasticityMarker->young_modulus.number();
            elasticityLabel[i].poisson_ratio = labelElasticityMarker->poisson_ratio.number();
            elasticityLabel[i].forceX = labelElasticityMarker->forceX.number();
            elasticityLabel[i].forceY = labelElasticityMarker->forceY.number();
            elasticityLabel[i].alpha = labelElasticityMarker->alpha.number();
            elasticityLabel[i].temp = labelElasticityMarker->temp.number();
            elasticityLabel[i].temp_ref = labelElasticityMarker->temp_ref.number();
        }
    }

    SolutionAgros solutionAgros(progressItemSolve);

    QList<SolutionArray *> solutionArrayList = solutionAgros.solveSolutioArray(Hermes::vector<BCTypes *>(&bcTypesX, &bcTypesY),
                                                                               Hermes::vector<BCValues *>(&bcValuesX, &bcValuesY),
                                                                               callbackElasticityWeakForm);

    delete [] elasticityEdge;
    delete [] elasticityLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueElasticity::LocalPointValueElasticity(Point &point) : LocalPointValue(point)
{
    young_modulus = 0.0;
    poisson_ratio = 0.0;
    von_mises_stress = 0.0;
    tresca_stress = 0.0;
    forceX = 0.0;
    forceY = 0.0;
    alpha = 0.0;
    temp = 0.0;
    temp_ref = 0.0;
    d = Point();
    strain_x = 0.0;
    strain_y = 0.0;
    strain_z = 0.0;
    strain_xy = 0.0;
    stress_x = 0.0;
    stress_y = 0.0;
    stress_z = 0.0;
    stress_xy = 0.0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (labelMarker)
        {
            SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

            young_modulus = marker->young_modulus.number();
            poisson_ratio = marker->poisson_ratio.number();

            forceX = marker->forceX.number();
            forceY = marker->forceY.number();

            alpha = marker->alpha.number();
            temp = marker->temp.number();
            temp_ref = marker->temp_ref.number();

            Solution *sln_x = Util::scene()->sceneSolution()->sln(0);
            Solution *sln_y = Util::scene()->sceneSolution()->sln(1);

            PointValue value_x = pointValue(sln_x, point);
            PointValue value_y = pointValue(sln_y, point);

            d.x = value_x.value;
            d.y = value_y.value;

            strain_x = value_x.derivative.x;
            strain_y = value_y.derivative.y;
            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
                strain_z = 0.0;
            else
                strain_z = (point.x > 0) ? value_x.value / point.x : 0.0;

            strain_xy = 0.5 * (value_x.derivative.y + value_y.derivative.x);

            // elastic constant
            double D = young_modulus / ((1 + poisson_ratio) * (1 - 2 * poisson_ratio));
            double eps_th = (1 + poisson_ratio) * alpha * (temp - temp_ref);

            stress_x = D * ((1 - poisson_ratio) * (strain_x - eps_th) + poisson_ratio * (strain_y - eps_th) + poisson_ratio * (strain_z));
            stress_y = D * (poisson_ratio * (strain_x - eps_th) + (1 - poisson_ratio) * (strain_y - eps_th) + poisson_ratio * (strain_z));
            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
                stress_z = D * (poisson_ratio * (strain_x - eps_th) + poisson_ratio * (strain_y - eps_th) + (1 - poisson_ratio) * (strain_z)) - young_modulus/1e6;
            else
                stress_z = D * (poisson_ratio * (strain_x - eps_th) + poisson_ratio * (strain_y - eps_th) + (1 - poisson_ratio) * (strain_z));

            stress_xy = D * (1 - 2 * poisson_ratio) / 2 * (value_x.derivative.y + value_y.derivative.x);

            von_mises_stress = sqrt(0.5 * (sqr(stress_x - stress_y) + sqr(stress_y - stress_z) + sqr(stress_z - stress_x) + 6*sqr(stress_xy)));

            tresca_stress = stress_x - stress_z;
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
    case PhysicFieldVariable_Elasticity_StrainXX:
    {
        return strain_x;
    }
        break;
    case PhysicFieldVariable_Elasticity_StrainYY:
    {
        return strain_y;
    }
        break;
    case PhysicFieldVariable_Elasticity_StrainZZ:
    {
        return strain_z;
    }
        break;
    case PhysicFieldVariable_Elasticity_StrainXY:
    {
        return strain_xy;
    }
        break;
    case PhysicFieldVariable_Elasticity_StressXX:
    {
        return stress_x;
    }
        break;
    case PhysicFieldVariable_Elasticity_StressYY:
    {
        return stress_y;
    }
        break;
    case PhysicFieldVariable_Elasticity_StressZZ:
    {
        return stress_z;
    }
        break;
    case PhysicFieldVariable_Elasticity_StressXY:
    {
        return stress_xy;
    }
        break;
    case PhysicFieldVariable_Elasticity_Displacement:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return d.x;
            break;
        case PhysicFieldVariableComp_Y:
            return d.y;
            break;
        case PhysicFieldVariableComp_Magnitude:
            return d.magnitude();
            break;
        }
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
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(von_mises_stress, 0, 'e', 5) <<
           QString("%1").arg(tresca_stress, 0, 'e', 5) <<
           QString("%1").arg(d.x, 0, 'e', 5) <<
           QString("%1").arg(d.y, 0, 'e', 5) <<
           QString("%1").arg(d.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(stress_x, 0, 'e', 5) <<
           QString("%1").arg(stress_y, 0, 'e', 5) <<
           QString("%1").arg(stress_z, 0, 'e', 5) <<
           QString("%1").arg(stress_xy, 0, 'e', 5) <<
           QString("%1").arg(strain_x, 0, 'e', 5) <<
           QString("%1").arg(strain_y, 0, 'e', 5) <<
           QString("%1").arg(strain_z, 0, 'e', 5) <<
           QString("%1").arg(strain_xy, 0, 'e', 5);

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
    case PhysicFieldVariable_Elasticity_StrainXX:
    {
        node->values[0][0][i] = dudx1[i];
    }
        break;
    case PhysicFieldVariable_Elasticity_StrainYY:
    {
        node->values[0][0][i] = dudy2[i];
    }
        break;
    case PhysicFieldVariable_Elasticity_StrainZZ:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            node->values[0][0][i] = 0.0;
        else
            node->values[0][0][i] = value1[i] / x[i];
    }
        break;
    case PhysicFieldVariable_Elasticity_StrainXY:
    {
        node->values[0][0][i] = 0.5 * (dudy1[i] + dudx2[i]);
    }
        break;
    case PhysicFieldVariable_Elasticity_StressXX:
    {
        SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

        // elastic constant
        double D = marker->young_modulus.number() / ((1 + marker->poisson_ratio.number()) * (1 - 2 * marker->poisson_ratio.number()));
        double eps_th = (1 + marker->poisson_ratio.number()) * marker->alpha.number() * (marker->temp.number() - marker->temp_ref.number());

        double strain_x = dudx1[i];
        double strain_y = dudy2[i];
        double strain_z = 0.0;
        if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
            strain_z = value1[i] / x[i];

        node->values[0][0][i] = D * ((1 - marker->poisson_ratio.number()) * (strain_x - eps_th) + marker->poisson_ratio.number() * (strain_y - eps_th) + marker->poisson_ratio.number() * (strain_z));

    }
        break;
    case PhysicFieldVariable_Elasticity_StressYY:
    {
        SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

        // elastic constant
        double D = marker->young_modulus.number() / ((1 + marker->poisson_ratio.number()) * (1 - 2 * marker->poisson_ratio.number()));
        double eps_th = (1 + marker->poisson_ratio.number()) * marker->alpha.number() * (marker->temp.number() - marker->temp_ref.number());

        double strain_x = dudx1[i];
        double strain_y = dudy2[i];
        double strain_z = 0.0;
        if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
            strain_z = value1[i] / x[i];

        node->values[0][0][i] = (marker->poisson_ratio.number() * (strain_x - eps_th) + (1 - marker->poisson_ratio.number()) * (strain_y - eps_th) + marker->poisson_ratio.number() * (strain_z));
    }
        break;
    case PhysicFieldVariable_Elasticity_StressZZ:
    {
        SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

        // elastic constant
        double D = marker->young_modulus.number() / ((1 + marker->poisson_ratio.number()) * (1 - 2 * marker->poisson_ratio.number()));
        double eps_th = (1 + marker->poisson_ratio.number()) * marker->alpha.number() * (marker->temp.number() - marker->temp_ref.number());

        double strain_x = dudx1[i];
        double strain_y = dudy2[i];
        double strain_z = 0.0;
        if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
            strain_z = value1[i] / x[i];

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            node->values[0][0][i] = D * (marker->poisson_ratio.number() * (strain_x - eps_th) + marker->poisson_ratio.number() * (strain_y - eps_th) + (1 - marker->poisson_ratio.number()) * (strain_z)) - marker->young_modulus.number()/1e6;
        else
            node->values[0][0][i] = D * (marker->poisson_ratio.number() * (strain_x - eps_th) + marker->poisson_ratio.number() * (strain_y - eps_th) + (1 - marker->poisson_ratio.number()) * (strain_z));
    }
        break;
    case PhysicFieldVariable_Elasticity_StressXY:
    {
        SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

        // elastic constant
        double D = marker->young_modulus.number() / ((1 + marker->poisson_ratio.number()) * (1 - 2 * marker->poisson_ratio.number()));

        node->values[0][0][i] = D * (1 - 2 * marker->poisson_ratio.number()) / 2 * (dudy1[i] + dudx2[i]);
    }
        break;
    case PhysicFieldVariable_Elasticity_Displacement:
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
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterElasticity::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneEdgeElasticityMarker::SceneEdgeElasticityMarker(const QString &name, PhysicFieldBC typeX,
                                                     PhysicFieldBC typeY, Value forceX, Value forceY,
                                                     Value displacementX, Value displacementY)
    : SceneEdgeMarker(name, typeX)
{
    this->typeX = typeX;
    this->typeY = typeY;
    this->forceX = forceX;
    this->forceY = forceY;
    this->displacementX = displacementX;
    this->displacementY = displacementY;
}

QString SceneEdgeElasticityMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", \"%3\", %4, %5, %6, %7)").
            arg(name).
            arg(physicFieldBCToStringKey(typeX)).
            arg(physicFieldBCToStringKey(typeY)).
            arg(forceX.text()).
            arg(forceY.text()).
            arg(displacementX.text()).
            arg(displacementY.text());
}

QMap<QString, QString> SceneEdgeElasticityMarker::data()
{
    QMap<QString, QString> out;
    switch (typeX)
    {
    case PhysicFieldBC_Elasticity_Free:
        out["Force X: (N)"] = forceX.number();
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        out["Displacement X: (N)"] = displacementX.number();
        break;
    }

    switch (typeY)
    {
    case PhysicFieldBC_Elasticity_Free:
        out["Force Y: (N)"] = forceY.number();
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        out["Displacement Y: (N)"] = displacementY.number();
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

SceneLabelElasticityMarker::SceneLabelElasticityMarker(const QString &name, Value young_modulus, Value poisson_ratio, Value forceX, Value forceY,
                                                       Value alpha, Value temp, Value temp_ref)
    : SceneLabelMarker(name)
{
    this->young_modulus = young_modulus;
    this->poisson_ratio = poisson_ratio;
    this->forceX = forceX;
    this->forceY = forceY;
    this->alpha = alpha;
    this->temp = temp;
    this->temp_ref = temp_ref;
}

QString SceneLabelElasticityMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4, %5, %6, %7, %8)").
            arg(name).
            arg(young_modulus.text()).
            arg(poisson_ratio.text()).
            arg(forceX.text()).
            arg(forceY.text()).
            arg(alpha.text()).
            arg(temp.text()).
            arg(temp_ref.text());
}

QMap<QString, QString> SceneLabelElasticityMarker::data()
{
    QMap<QString, QString> out;
    out["Young modulus (Pa)"] = young_modulus.number();
    out["Poisson ratio (-)"] = poisson_ratio.number();
    out["Volumetric force X (N/m3)"] = forceX.number();
    out["Volumetric force Y (N/m3)"] = forceX.number();
    out["Thermal exp. coef. (1/deg)"] = alpha.number();
    out["Volumetric force Y (deg)"] = temp.number();
    out["Volumetric force Y (deg)"] = temp_ref.number();
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
    cmbTypeX->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Free), PhysicFieldBC_Elasticity_Free);
    cmbTypeX->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Fixed), PhysicFieldBC_Elasticity_Fixed);
    connect(cmbTypeX, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeXChanged(int)));

    cmbTypeY = new QComboBox(this);
    cmbTypeY->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Free), PhysicFieldBC_Elasticity_Free);
    cmbTypeY->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Fixed), PhysicFieldBC_Elasticity_Fixed);
    connect(cmbTypeY, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeYChanged(int)));

    txtForceX = new SLineEditValue(this);
    txtForceY = new SLineEditValue(this);
    txtDisplacementX = new SLineEditValue(this);
    txtDisplacementY = new SLineEditValue(this);

    doTypeXChanged(cmbTypeX->currentIndex());
    doTypeYChanged(cmbTypeY->currentIndex());

    // X
    QFormLayout *layoutX = new QFormLayout();
    layoutX->addRow(tr("BC Type:"), cmbTypeX);
    layoutX->addRow(tr("Force (N):"), txtForceX);
    layoutX->addRow(tr("Displacement (m):"), txtDisplacementX);

    QGroupBox *grpX = new QGroupBox(tr("Direction %1").arg(Util::scene()->problemInfo()->labelX()), this);
    grpX->setLayout(layoutX);

    // X
    QFormLayout *layoutY = new QFormLayout();
    layoutY->addRow(tr("BC Type:"), cmbTypeY);
    layoutY->addRow(tr("Force (N):"), txtForceY);
    layoutY->addRow(tr("Displacement (m):"), txtDisplacementY);

    QGroupBox *grpY = new QGroupBox(tr("Direction %1").arg(Util::scene()->problemInfo()->labelY()), this);
    grpY->setLayout(layoutY);

    layout->addWidget(grpX, 1, 0, 1, 2);
    layout->addWidget(grpY, 2, 0, 1, 2);
}

void DSceneEdgeElasticityMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(m_edgeMarker);

    cmbTypeX->setCurrentIndex(cmbTypeX->findData(edgeElasticityMarker->typeX));
    cmbTypeY->setCurrentIndex(cmbTypeY->findData(edgeElasticityMarker->typeY));

    txtForceX->setValue(edgeElasticityMarker->forceX);
    txtForceY->setValue(edgeElasticityMarker->forceY);

    txtDisplacementX->setValue(edgeElasticityMarker->displacementX);
    txtDisplacementY->setValue(edgeElasticityMarker->displacementY);
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

    if (txtDisplacementX->evaluate())
        edgeElasticityMarker->displacementX = txtDisplacementX->value();
    else
        return false;

    if (txtDisplacementY->evaluate())
        edgeElasticityMarker->displacementY = txtDisplacementY->value();
    else
        return false;

    return true;
}

void DSceneEdgeElasticityMarker::doTypeXChanged(int index)
{
    txtForceX->setEnabled(false);
    txtDisplacementX->setEnabled(false);

    switch ((PhysicFieldBC) cmbTypeX->itemData(index).toInt())
    {
    case PhysicFieldBC_Elasticity_Fixed:
    {
        txtDisplacementX->setEnabled(true);
    }
        break;
    case PhysicFieldBC_Elasticity_Free:
    {
        txtForceX->setEnabled(true);
    }
        break;
    }
}

void DSceneEdgeElasticityMarker::doTypeYChanged(int index)
{
    txtForceY->setEnabled(false);
    txtDisplacementY->setEnabled(false);

    switch ((PhysicFieldBC) cmbTypeY->itemData(index).toInt())
    {
    case PhysicFieldBC_Elasticity_Fixed:
    {
        txtDisplacementY->setEnabled(true);
    }
        break;
    case PhysicFieldBC_Elasticity_Free:
    {
        txtForceY->setEnabled(true);
    }
        break;
    }
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
    txtAlpha = new SLineEditValue(this);
    txtTemp = new SLineEditValue(this);
    txtTempRef = new SLineEditValue(this);

    // forces
    QFormLayout *layoutForces = new QFormLayout();
    layoutForces->addRow(tr("Force X (N):"), txtForceX);
    layoutForces->addRow(tr("Force Y (N):"), txtForceY);

    QGroupBox *grpForces = new QGroupBox(tr("Volumetric forces"), this);
    grpForces->setLayout(layoutForces);

    // thermal expansion
    QFormLayout *layoutThermalExpansion = new QFormLayout();
    layoutThermalExpansion->addRow(tr("Thermal exp. coef. (1/deg):"), txtAlpha);
    layoutThermalExpansion->addRow(tr("Temperature (deg):"), txtTemp);
    layoutThermalExpansion->addRow(tr("Ref. temperature (deg):"), txtTempRef);

    QGroupBox *grpThermalExpansion = new QGroupBox(tr("Thermal expansion"), this);
    grpThermalExpansion->setLayout(layoutThermalExpansion);

    layout->addWidget(new QLabel(tr("Young modulus (Pa):")), 1, 0);
    layout->addWidget(txtYoungModulus, 1, 1);
    layout->addWidget(new QLabel(tr("Poisson number (-):")), 2, 0);
    layout->addWidget(txtPoissonNumber, 2, 1);
    layout->addWidget(grpForces, 3, 0, 1, 2);
    layout->addWidget(grpThermalExpansion, 4, 0, 1, 2);
}

void DSceneLabelElasticityMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(m_labelMarker);

    txtYoungModulus->setValue(labelElasticityMarker->young_modulus);
    txtPoissonNumber->setValue(labelElasticityMarker->poisson_ratio);

    txtForceX->setValue(labelElasticityMarker->forceX);
    txtForceY->setValue(labelElasticityMarker->forceY);

    txtAlpha->setValue(labelElasticityMarker->alpha);
    txtTemp->setValue(labelElasticityMarker->temp);
    txtTempRef->setValue(labelElasticityMarker->temp_ref);
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

    if (txtAlpha->evaluate())
        labelElasticityMarker->alpha = txtAlpha->value();
    else
        return false;

    if (txtTemp->evaluate())
        labelElasticityMarker->temp = txtTemp->value();
    else
        return false;

    if (txtTempRef->evaluate())
        labelElasticityMarker->temp_ref = txtTempRef->value();
    else
        return false;


    return true;
}
