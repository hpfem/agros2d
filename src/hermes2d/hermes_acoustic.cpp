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

#include "hermes_acoustic.h"

#include "scene.h"
#include "gui.h"

const double PRESSURE_MIN_AIR = 20e-6;

struct AcousticEdge
{
    PhysicFieldBC type;
    double value;
    TimeFunction valueTransient;
};

struct AcousticLabel
{
    double density;
    double speed;
};

AcousticEdge *acousticEdge;
AcousticLabel *acousticLabel;

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_surf_imag_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (!(acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_Impedance
          || acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_MatchedBoundary))
        return 0.0;

    double Z = 0.0;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_Impedance)
        Z = acousticEdge[e->edge_marker].value;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_MatchedBoundary)
        Z = acousticLabel[e->elem_marker].density * acousticLabel[e->elem_marker].speed;

    if (fabs(Z) < EPS_ZERO)
        return 0.0;

    if (isPlanar)
        return 2 * M_PI * frequency / Z * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return 2 * M_PI * 2 * M_PI * frequency / Z * int_x_u_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_surf_real_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (!(acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_Impedance
          || acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_MatchedBoundary))
        return 0.0;

    double Z = 0.0;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_Impedance)
        Z = acousticEdge[e->edge_marker].value;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_MatchedBoundary)
        Z = acousticLabel[e->elem_marker].density * acousticLabel[e->elem_marker].speed;

    if (fabs(Z) < EPS_ZERO)
        return 0.0;

    if (isPlanar)
        return - 2 * M_PI * frequency / Z * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return - 2 * M_PI * 2 * M_PI * frequency / Z * int_x_u_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar acoustic_vector_form_surf_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double an = 0.0;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_NormalAcceleration)
        an = acousticEdge[e->edge_marker].value;

    if (isPlanar)
        return an * int_v<Real, Scalar>(n, wt, v);
    else
        return an * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_real_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return 1.0/acousticLabel[e->elem_marker].density * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)
                - sqr(2 * M_PI * frequency) / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return 2 * M_PI * (1.0/acousticLabel[e->elem_marker].density * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e)
                           - sqr(2 * M_PI * frequency) / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                           * int_x_u_v<Real, Scalar>(n, wt, u, v, e));
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_real_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return 0.0;
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_imag_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return 0.0;
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_imag_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return 1.0/acousticLabel[e->elem_marker].density * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)
                - sqr(2 * M_PI * frequency) / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return 2 * M_PI * (1.0/acousticLabel[e->elem_marker].density * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e)
                           - sqr(2 * M_PI * frequency) / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                           * int_x_u_v<Real, Scalar>(n, wt, u, v, e));
}

// ****************************************************************************************************
// time dependent

// - 1/rho * \Delta p + 1/(rho * c^2) * \frac{\partial v}{\partial t} = 0.
// - v                +                 \frac{\partial p}{\partial t} = 0,

scalar acoustic_essential_time_bc_pressure(int edge_marker, double x, double y, double time)
{
    return acousticEdge[edge_marker].valueTransient.value(time);
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_surf_time(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (!(acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_Impedance
          || acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_MatchedBoundary))
        return 0.0;

    double Z = 0.0;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_Impedance)
        Z = acousticEdge[e->edge_marker].value;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_MatchedBoundary)
        Z = acousticLabel[e->elem_marker].density * acousticLabel[e->elem_marker].speed;

    if (fabs(Z) < EPS_ZERO)
        return 0.0;

    if (isPlanar)
        return 1.0 / Z * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return 2 * M_PI / Z * int_x_u_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar acoustic_vector_form_surf_time(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double an = 0.0;

    if (acousticEdge[e->edge_marker].type == PhysicFieldBC_Acoustic_NormalAcceleration)
        an = acousticEdge[e->edge_marker].valueTransient.value(actualTime);

    if (isPlanar)
        return an * int_v<Real, Scalar>(n, wt, v);
    else
        return an * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_time_velocity_velocity(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return int_u_v<Real, Scalar>(n, wt, u, v) / timeStep;
    else
        return 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e) / timeStep;
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_time_velocity_pressure(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return - int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return - 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_time_pressure_velocity(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return 1.0 / acousticLabel[e->elem_marker].density * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return 2 * M_PI * 1.0 / acousticLabel[e->elem_marker].density * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar acoustic_matrix_form_time_pressure_pressure(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return 1.0 / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                * int_u_v<Real, Scalar>(n, wt, u, v) / timeStep;
    else
        return 2 * M_PI * 1.0 / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                * int_x_u_v<Real, Scalar>(n, wt, u, v, e) / timeStep;
}

template<typename Real, typename Scalar>
Scalar acoustic_vector_form_velocity(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return int_u_v<Real, Scalar>(n, wt, ext->fn[0], v) / timeStep;
    else
        return 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, ext->fn[0], v, e) / timeStep;
}

template<typename Real, typename Scalar>
Scalar acoustic_vector_form_pressure(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return 1.0 / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                * int_u_v<Real, Scalar>(n, wt, ext->fn[0], v) / timeStep;
    else
        return 2 * M_PI * 1.0 / (acousticLabel[e->elem_marker].density * sqr(acousticLabel[e->elem_marker].speed))
                * int_x_u_v<Real, Scalar>(n, wt, ext->fn[0], v, e) / timeStep;
}

// *****************************************************************************************


void callbackAcousticWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    if (analysisType == AnalysisType_Transient)
    {
        wf->add_matrix_form(0, 0, callback(acoustic_matrix_form_time_velocity_velocity));
        wf->add_matrix_form(0, 1, callback(acoustic_matrix_form_time_velocity_pressure));
        wf->add_matrix_form(1, 0, callback(acoustic_matrix_form_time_pressure_velocity));
        wf->add_matrix_form(1, 1, callback(acoustic_matrix_form_time_pressure_pressure));

        wf->add_vector_form(0, callback(acoustic_vector_form_velocity), HERMES_ANY, slnArray.at(0));
        wf->add_vector_form(1, callback(acoustic_vector_form_pressure), HERMES_ANY, slnArray.at(1));

        // matched boundary and impedance
        wf->add_matrix_form_surf(1, 0, callback(acoustic_matrix_form_surf_time));

        // pressure normal derivative
        wf->add_vector_form_surf(1, callback(acoustic_vector_form_surf_time));
    }
    else if (analysisType == AnalysisType_Harmonic)
    {
        wf->add_matrix_form(0, 0, callback(acoustic_matrix_form_real_real));
        wf->add_matrix_form(0, 1, callback(acoustic_matrix_form_real_imag));
        wf->add_matrix_form(1, 0, callback(acoustic_matrix_form_imag_real));
        wf->add_matrix_form(1, 1, callback(acoustic_matrix_form_imag_imag));

        // matched boundary and impedance
        wf->add_matrix_form_surf(0, 1, callback(acoustic_matrix_form_surf_imag_real));
        wf->add_matrix_form_surf(1, 0, callback(acoustic_matrix_form_surf_real_imag));

        // pressure normal derivative
        wf->add_vector_form_surf(0, callback(acoustic_vector_form_surf_real));
    }
}

// *******************************************************************************************************

PhysicFieldVariable HermesAcoustic::contourPhysicFieldVariable()
{
    return PhysicFieldVariable_Acoustic_PressureReal;
}

PhysicFieldVariable HermesAcoustic::scalarPhysicFieldVariable()
{
    return PhysicFieldVariable_Acoustic_PressureReal;
}

PhysicFieldVariableComp HermesAcoustic::scalarPhysicFieldVariableComp()
{
    return PhysicFieldVariableComp_Scalar;
}

PhysicFieldVariable HermesAcoustic::vectorPhysicFieldVariable()
{
    return PhysicFieldVariable_Acoustic_LocalAcceleration;
}


void HermesAcoustic::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Acoustic_Pressure:
        Util::scene()->addEdgeMarker(new SceneEdgeAcousticMarker(element->attribute("name"),
                                                                 type,
                                                                 Value(element->attribute("pressure", "0")),
                                                                 TimeFunction(element->attribute("pressure_transient", "0"),
                                                                              0.0, Util::scene()->problemInfo()->timeTotal.number)));
        break;
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        Util::scene()->addEdgeMarker(new SceneEdgeAcousticMarker(element->attribute("name"),
                                                                 type,
                                                                 Value(element->attribute("acceleration", "0")),
                                                                 TimeFunction(element->attribute("acceleration_transient", "0"),
                                                                              0.0, Util::scene()->problemInfo()->timeTotal.number)));
        break;
    case PhysicFieldBC_Acoustic_Impedance:
        Util::scene()->addEdgeMarker(new SceneEdgeAcousticMarker(element->attribute("name"),
                                                                 type,
                                                                 Value(element->attribute("impedance", "0"))));
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
        Util::scene()->addEdgeMarker(new SceneEdgeAcousticMarker(element->attribute("name"),
                                                                 type));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesAcoustic::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeAcousticMarker *edgeAcousticMarker = dynamic_cast<SceneEdgeAcousticMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeAcousticMarker->type));

    switch (edgeAcousticMarker->type)
    {
    case PhysicFieldBC_Acoustic_Pressure:
        element->setAttribute("pressure", edgeAcousticMarker->value_real.text);
        element->setAttribute("pressure_transient", edgeAcousticMarker->value_transient.function());
        break;
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        element->setAttribute("acceleration", edgeAcousticMarker->value_real.text);
        element->setAttribute("acceleration_transient", edgeAcousticMarker->value_transient.function());
        break;
    case PhysicFieldBC_Acoustic_Impedance:
        element->setAttribute("impedance", edgeAcousticMarker->value_real.text);
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }

}

void HermesAcoustic::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelAcousticMarker(element->attribute("name"),
                                                               Value(element->attribute("density", "1.25")),
                                                               Value(element->attribute("speed", "343"))));
}

void HermesAcoustic::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelAcousticMarker *labelAcousticMarker = dynamic_cast<SceneLabelAcousticMarker *>(marker);

    element->setAttribute("density", labelAcousticMarker->density.text);
    element->setAttribute("speed", labelAcousticMarker->speed.text);
}

LocalPointValue *HermesAcoustic::localPointValue(const Point &point)
{
    return new LocalPointValueAcoustic(point);
}

QStringList HermesAcoustic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "p_real" << "p_imag" << "p" << "Lp" << "rho" << "c";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesAcoustic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueAcoustic();
}

QStringList HermesAcoustic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesAcoustic::volumeIntegralValue()
{
    return new VolumeIntegralValueAcoustic();
}

QStringList HermesAcoustic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S";
    return QStringList(headers);
}

SceneEdgeMarker *HermesAcoustic::newEdgeMarker()
{
    return new SceneEdgeAcousticMarker(tr("new boundary"),
                                       PhysicFieldBC_Acoustic_Pressure,
                                       Value("0"));
}

SceneEdgeMarker *HermesAcoustic::newEdgeMarker(PyObject *self, PyObject *args)
{
    double pressure;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ss|d", &name, &type, &pressure))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Acoustic_MatchedBoundary)
            return new SceneEdgeAcousticMarker(name,
                                               physicFieldBCFromStringKey(type));
        else
            return new SceneEdgeAcousticMarker(name,
                                               physicFieldBCFromStringKey(type),
                                               Value(QString::number(pressure)));
    }

    return NULL;
}

SceneEdgeMarker *HermesAcoustic::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double pressure;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &pressure))
    {
        if (SceneEdgeAcousticMarker *marker = dynamic_cast<SceneEdgeAcousticMarker *>(Util::scene()->getEdgeMarker(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                if (marker->type == PhysicFieldBC_Acoustic_Pressure)
                {
                    marker->value_real = Value(QString::number(pressure));
                }
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

SceneLabelMarker *HermesAcoustic::newLabelMarker()
{
    return new SceneLabelAcousticMarker(tr("new material"),
                                        Value("1.25"),
                                        Value("343"));
}

SceneLabelMarker *HermesAcoustic::newLabelMarker(PyObject *self, PyObject *args)
{
    double density, speed;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &density, &speed))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelAcousticMarker(name,
                                            Value(QString::number(density)),
                                            Value(QString::number(speed)));
    }

    return NULL;
}

SceneLabelMarker *HermesAcoustic::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double density, speed;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &density, &speed))
    {
        if (SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->density = Value(QString::number(density));
            marker->speed = Value(QString::number(speed));
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

void HermesAcoustic::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    // harmonic
    if (analysisType == AnalysisType_Harmonic)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Pressure), PhysicFieldVariable_Acoustic_Pressure);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureReal), PhysicFieldVariable_Acoustic_PressureReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureImag), PhysicFieldVariable_Acoustic_PressureImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureLevel), PhysicFieldVariable_Acoustic_PressureLevel);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalVelocity), PhysicFieldVariable_Acoustic_LocalVelocity);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalAcceleration), PhysicFieldVariable_Acoustic_LocalAcceleration);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Density), PhysicFieldVariable_Acoustic_Density);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Speed), PhysicFieldVariable_Acoustic_Speed);
    }
    // transient
    if (analysisType == AnalysisType_Transient)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Pressure), PhysicFieldVariable_Acoustic_PressureReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureLevel), PhysicFieldVariable_Acoustic_PressureLevel);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalVelocity), PhysicFieldVariable_Acoustic_LocalVelocity);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalAcceleration), PhysicFieldVariable_Acoustic_LocalAcceleration);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Density), PhysicFieldVariable_Acoustic_Density);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Speed), PhysicFieldVariable_Acoustic_Speed);
    }
}

void HermesAcoustic::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalAcceleration), PhysicFieldVariable_Acoustic_LocalAcceleration);
}

void HermesAcoustic::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueAcoustic *localPointValueAcoustic = dynamic_cast<LocalPointValueAcoustic *>(localPointValue);

    // magnetic
    QTreeWidgetItem *acousticNode = new QTreeWidgetItem(trvWidget);
    acousticNode->setText(0, tr("Acoustic field"));
    acousticNode->setExpanded(true);

    // material
    addTreeWidgetItemValue(acousticNode, tr("Density:"), QString("%1").arg(localPointValueAcoustic->density, 0, 'f', 2), "kg/m3");
    addTreeWidgetItemValue(acousticNode, tr("Speed of sound:"), QString("%1").arg(localPointValueAcoustic->speed, 0, 'f', 2), "m/s");

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        // Pressure
        QTreeWidgetItem *itemPressure = new QTreeWidgetItem(acousticNode);
        itemPressure->setText(0, tr("Acoustic pressure"));
        itemPressure->setExpanded(true);

        addTreeWidgetItemValue(itemPressure, tr("real:"), QString("%1").arg(localPointValueAcoustic->pressure_real, 0, 'e', 3), "Pa");
        addTreeWidgetItemValue(itemPressure, tr("imag:"), QString("%1").arg(localPointValueAcoustic->pressure_imag, 0, 'e', 3), "Pa");
        addTreeWidgetItemValue(itemPressure, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueAcoustic->pressure_real) + sqr(localPointValueAcoustic->pressure_imag)), 0, 'e', 3), "Pa");

        addTreeWidgetItemValue(acousticNode, tr("Sound pressure level:"), QString("%1").arg(localPointValueAcoustic->pressureLevel, 0, 'f', 2), "dB");

        // Local velocity
        /*
        QTreeWidgetItem *itemLocalVelocity = new QTreeWidgetItem(pressureNode);
        itemLocalVelocity->setText(0, tr("Local velocity"));
        itemLocalVelocity->setExpanded(true);

        addTreeWidgetItemValue(itemLocalVelocity, "v" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.x, 0, 'f', 5), tr("m/s"));
        addTreeWidgetItemValue(itemLocalVelocity, "v" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.y, 0, 'f', 5), tr("m/s"));
        addTreeWidgetItemValue(itemLocalVelocity, "v:", QString("%1").arg(localPointValueAcoustic->localAccelaration.magnitude(), 0, 'f', 5), "m/s");

        // Local acceleration
        QTreeWidgetItem *itemLocalAcceleration = new QTreeWidgetItem(pressureNode);
        itemLocalAcceleration->setText(0, tr("Local acceleration"));
        itemLocalAcceleration->setExpanded(true);

        addTreeWidgetItemValue(itemLocalAcceleration, "a" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.x, 0, 'f', 5), tr("m/s2"));
        addTreeWidgetItemValue(itemLocalAcceleration, "a" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.y, 0, 'f', 5), tr("m/s2"));
        addTreeWidgetItemValue(itemLocalAcceleration, "a:", QString("%1").arg(localPointValueAcoustic->localAccelaration.magnitude(), 0, 'f', 5), "m/s2");
        */
    }
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        // Pressure
        addTreeWidgetItemValue(acousticNode, tr("Acoustic pressure:"), QString("%1").arg(localPointValueAcoustic->pressure_real, 0, 'e', 3), "Pa");
        addTreeWidgetItemValue(acousticNode, tr("Sound pressure level:"), QString("%1").arg(localPointValueAcoustic->pressureLevel, 0, 'f', 2), "dB");
    }
}

void HermesAcoustic::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueAcoustic *surfaceIntegralValueAcoustic = dynamic_cast<SurfaceIntegralValueAcoustic *>(surfaceIntegralValue);

    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Acoustic field"));
    magneticNode->setExpanded(true);
}

void HermesAcoustic::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueAcoustic *volumeIntegralValueAcoustic = dynamic_cast<VolumeIntegralValueAcoustic *>(volumeIntegralValue);

    // harmonic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Acoustic field"));
    magneticNode->setExpanded(true);
}

ViewScalarFilter *HermesAcoustic::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    Solution *sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
    return new ViewScalarFilterAcoustic(Hermes::vector<MeshFunction *>(sln1, sln2),
                                        physicFieldVariable,
                                        physicFieldVariableComp);
}

QList<SolutionArray *> HermesAcoustic::solve(ProgressItemSolve *progressItemSolve)
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return QList<SolutionArray *>();

        // check functions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneEdgeAcousticMarker *edgeAcousticMarker = dynamic_cast<SceneEdgeAcousticMarker *>(Util::scene()->edges[i]->marker);

            TimeFunction timeFunction(edgeAcousticMarker->value_transient.function(), 0.0, Util::scene()->problemInfo()->timeTotal.number, 2);
            if (!timeFunction.isValid())
                return QList<SolutionArray *>();
        }
    }

    // edge markers
    BCTypes bcTypesReal, bcTypesImag;
    BCValues bcValuesReal(&actualTime), bcValuesImag;

    acousticEdge = new AcousticEdge[Util::scene()->edges.count()+1];
    acousticEdge[0].type = PhysicFieldBC_None;
    acousticEdge[0].value = 0.0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            acousticEdge[i+1].type = PhysicFieldBC_None;
            acousticEdge[i+1].value = 0.0;
        }
        else
        {
            SceneEdgeAcousticMarker *edgeAcousticMarker = dynamic_cast<SceneEdgeAcousticMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeAcousticMarker->value_real.evaluate()) return QList<SolutionArray *>();

            acousticEdge[i+1].type = edgeAcousticMarker->type;
            acousticEdge[i+1].value = edgeAcousticMarker->value_real.number;
            acousticEdge[i+1].valueTransient = TimeFunction(edgeAcousticMarker->value_transient.function(),
                                                            0.0, Util::scene()->problemInfo()->timeTotal.number);

            switch (edgeAcousticMarker->type)
            {
            case PhysicFieldBC_None:
                bcTypesReal.add_bc_none(i+1);
                bcTypesImag.add_bc_none(i+1);
                break;
            case PhysicFieldBC_Acoustic_Pressure:
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    bcTypesReal.add_bc_dirichlet(i+1);
                    bcTypesImag.add_bc_dirichlet(i+1);
                    bcValuesReal.add_const(i+1, edgeAcousticMarker->value_real.number);
                    bcValuesImag.add_const(i+1, 0.0);
                }
                else
                {
                    bcTypesReal.add_bc_dirichlet(i+1);
                    if (edgeAcousticMarker->value_transient.isValid())
                        bcValuesReal.add_timedep_function(i+1, acoustic_essential_time_bc_pressure);
                    else
                        bcValuesReal.add_const(i+1, edgeAcousticMarker->value_real.number);
                    // bcTypesImag.add_bc_dirichlet(i+1);
                    // bcTypesImag.add_bc_none(i+1);
                    // bcValuesImag.add_const(i+1, 0.0);
                }
                break;
            case PhysicFieldBC_Acoustic_NormalAcceleration:
                bcTypesReal.add_bc_neumann(i+1);
                bcTypesImag.add_bc_neumann(i+1);
                break;
            case PhysicFieldBC_Acoustic_MatchedBoundary:
                bcTypesReal.add_bc_neumann(i+1);
                bcTypesImag.add_bc_neumann(i+1);
                break;
            }
        }
    }

    // label markers
    acousticLabel = new AcousticLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelAcousticMarker *labelAcousticMarker = dynamic_cast<SceneLabelAcousticMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelAcousticMarker->density.evaluate()) return QList<SolutionArray *>();
            if (!labelAcousticMarker->speed.evaluate()) return QList<SolutionArray *>();

            acousticLabel[i].density = labelAcousticMarker->density.number;
            acousticLabel[i].speed = labelAcousticMarker->speed.number;
        }
    }

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve,
                                                                 Hermes::vector<BCTypes *>(&bcTypesReal, &bcTypesImag),
                                                                 Hermes::vector<BCValues *>(&bcValuesReal, &bcValuesImag),
                                                                 callbackAcousticWeakForm);

    delete [] acousticEdge;
    delete [] acousticLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueAcoustic::LocalPointValueAcoustic(const Point &point) : LocalPointValue(point)
{
    density = 0;
    speed = 0;
    pressure_real = 0;
    pressure_imag = 0;
    pressureLevel = 0;
    localAccelaration = Point();
    localVelocity = Point();

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // value real
        PointValue valueReal = PointValue(value, derivative, labelMarker);

        SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(valueReal.marker);
        // solution
        if (marker != NULL)
        {            
            density = marker->density.number;
            speed = marker->speed.number;

            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            {
                Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

                // value imag
                PointValue valueImag = pointValue(sln2, point);
                double frequency = Util::scene()->problemInfo()->frequency;

                Point derReal = valueReal.derivative;
                Point derImag = valueImag.derivative;

                pressure_real = valueReal.value;
                pressure_imag = valueImag.value;

                localAccelaration.x = - derReal.x / density;
                localAccelaration.y = - derReal.y / density;

                localVelocity.x = localAccelaration.x / (2 * M_PI * frequency);
                localVelocity.y = localAccelaration.y / (2 * M_PI * frequency);

                pressureLevel = (sqrt(sqr(valueReal.value) + sqr(valueImag.value)) > PRESSURE_MIN_AIR) ?
                        20.0 * log10(sqrt(sqr(valueReal.value) + sqr(valueImag.value)) / sqrt(2.0) / PRESSURE_MIN_AIR) : 0.0;
            }
            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            {
                Point derReal = valueReal.derivative;

                pressure_real = valueReal.value;

                pressureLevel = (valueReal.value > PRESSURE_MIN_AIR) ? 20.0 * log10(valueReal.value / PRESSURE_MIN_AIR) : 0.0;
            }
        }
    }
}

double LocalPointValueAcoustic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Acoustic_Pressure:
        return sqrt(sqr(pressure_real) + sqr(pressure_imag));
    case PhysicFieldVariable_Acoustic_PressureReal:
        return pressure_real;
    case PhysicFieldVariable_Acoustic_PressureImag:
        return pressure_imag;
    case PhysicFieldVariable_Acoustic_LocalVelocity:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return localVelocity.x;
        case PhysicFieldVariableComp_Y:
            return localVelocity.y;
        case PhysicFieldVariableComp_Magnitude:
            return localVelocity.magnitude();
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_LocalAcceleration:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return localAccelaration.x;
        case PhysicFieldVariableComp_Y:
            return localAccelaration.y;
        case PhysicFieldVariableComp_Magnitude:
            return localAccelaration.magnitude();
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureLevel:
        return pressureLevel;
    case PhysicFieldVariable_Acoustic_Density:
        return density;
    case PhysicFieldVariable_Acoustic_Speed:
        return speed;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueAcoustic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueAcoustic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(pressure_real, 0, 'e', 5) <<
           QString("%1").arg(pressure_imag, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(pressure_real) + sqr(pressure_imag)), 0, 'e', 5) <<
           QString("%1").arg(pressureLevel, 0, 'e', 5) <<
           QString("%1").arg(density, 0, 'f', 2) <<
           QString("%1").arg(speed, 0, 'f', 2);
    // QString("%1").arg(localAccelaration.x, 0, 'e', 5) <<
    // QString("%1").arg(localAccelaration.y, 0, 'e', 5);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueAcoustic::SurfaceIntegralValueAcoustic() : SurfaceIntegralValue()
{
    calculate();
}

void SurfaceIntegralValueAcoustic::calculateVariables(int i)
{
    SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(Util::scene()->labels[e->marker]->marker);
}

QStringList SurfaceIntegralValueAcoustic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
           QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}


// ****************************************************************************************************************

VolumeIntegralValueAcoustic::VolumeIntegralValueAcoustic() : VolumeIntegralValue()
{
    calculate();
}

void VolumeIntegralValueAcoustic::calculateVariables(int i)
{
    SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(Util::scene()->labels[e->marker]->marker);
}

void VolumeIntegralValueAcoustic::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
}

QStringList VolumeIntegralValueAcoustic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
           QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterAcoustic::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Acoustic_Pressure:
    {
        node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureReal:
    {
        node->values[0][0][i] = value1[i];
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureImag:
    {
        node->values[0][0][i] = value2[i];
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureLevel:
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            node->values[0][0][i] = (sqrt(sqr(value1[i]) + sqr(value2[i])) > PRESSURE_MIN_AIR) ?
                    20.0 * log10(sqrt(sqr(value1[i]) + sqr(value2[i])) / sqrt(2.0) / PRESSURE_MIN_AIR) : 0.0;
        else if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            node->values[0][0][i] = (value1[i] > PRESSURE_MIN_AIR) ?
                    20.0 * log10(value1[i] / PRESSURE_MIN_AIR) : 0.0;
    }
        break;
    case PhysicFieldVariable_Acoustic_LocalVelocity:
    {
        SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(labelMarker);

        switch (m_physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
        {
            node->values[0][0][i] = - dudx1[i] / marker->density.number / (2 * M_PI * frequency);
        }
            break;
        case PhysicFieldVariableComp_Y:
        {
            node->values[0][0][i] = - dudy1[i] / marker->density.number / (2 * M_PI * frequency);
        }
            break;
        case PhysicFieldVariableComp_Magnitude:
        {
            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i])) / marker->density.number / (2 * M_PI * frequency);;
        }
            break;
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_LocalAcceleration:
    {
        SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(labelMarker);

        switch (m_physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
        {
            node->values[0][0][i] = - dudx1[i] / marker->density.number;
        }
            break;
        case PhysicFieldVariableComp_Y:
        {
            node->values[0][0][i] = - dudy1[i] / marker->density.number;
        }
            break;
        case PhysicFieldVariableComp_Magnitude:
        {
            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i])) / marker->density.number;
        }
            break;
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_Density:
    {
        SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(labelMarker);
        node->values[0][0][i] = marker->density.number;
    }
        break;
    case PhysicFieldVariable_Acoustic_Speed:
    {
        SceneLabelAcousticMarker *marker = dynamic_cast<SceneLabelAcousticMarker *>(labelMarker);
        node->values[0][0][i] = marker->speed.number;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterAcoustic::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneEdgeAcousticMarker::SceneEdgeAcousticMarker(const QString &name, PhysicFieldBC type, Value value_real, TimeFunction value_transient)
 : SceneEdgeMarker(name, type)
{
    this->value_real = value_real;
    this->value_transient = value_transient;
}

SceneEdgeAcousticMarker::SceneEdgeAcousticMarker(const QString &name, PhysicFieldBC type, Value value_real) : SceneEdgeMarker(name, type)
{
    this->value_real = value_real;
}

SceneEdgeAcousticMarker::SceneEdgeAcousticMarker(const QString &name, PhysicFieldBC type) : SceneEdgeMarker(name, type)
{
}

QString SceneEdgeAcousticMarker::script()
{
    if (value_transient.isValid())
        return QString("addboundary(\"%1\", \"%2\", %3, \"%4\")").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(value_real.text).
                arg(value_transient.function());
    else
        return QString("addboundary(\"%1\", \"%2\", %3)").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(value_real.text);
}

QMap<QString, QString> SceneEdgeAcousticMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Acoustic_Pressure:
        out["Pressure (Pa)"] = value_real.text;
        break;
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        out["Normal acceleration (m/s2)"] = value_real.text;
        break;
    case PhysicFieldBC_Acoustic_Impedance:
        out["Input impedance (Pa.s/m)"] = value_real.text;
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeAcousticMarker::showDialog(QWidget *parent)
{
    DSceneEdgeAcousticMarker *dialog = new DSceneEdgeAcousticMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelAcousticMarker::SceneLabelAcousticMarker(const QString &name, Value density, Value speed)
    : SceneLabelMarker(name)
{
    this->density = density;
    this->speed = speed;
}

QString SceneLabelAcousticMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(density.text).
            arg(speed.text);
}

QMap<QString, QString> SceneLabelAcousticMarker::data()
{
    QMap<QString, QString> out;
    out["Density (-)"] = density.text;
    out["Speed (-)"] = speed.text;
    return QMap<QString, QString>(out);
}

int SceneLabelAcousticMarker::showDialog(QWidget *parent)
{
    DSceneLabelAcousticMarker *dialog = new DSceneLabelAcousticMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeAcousticMarker::DSceneEdgeAcousticMarker(SceneEdgeAcousticMarker *edgeAcousticMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeAcousticMarker;

    createDialog();

    load();
    setSize();
}

void DSceneEdgeAcousticMarker::createContent()
{
    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_Pressure), PhysicFieldBC_Acoustic_Pressure);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_NormalAcceleration), PhysicFieldBC_Acoustic_NormalAcceleration);
    // FIX impedance boundary for transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_MatchedBoundary), PhysicFieldBC_Acoustic_MatchedBoundary);
        cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_Impedance), PhysicFieldBC_Acoustic_Impedance);
    }
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValue = new SLineEditValue(this);
    txtValueTransient = new QLineEdit("0", this);

    connect(txtValue, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    layout->addWidget(new QLabel(tr("BC type:")), 1, 0);
    layout->addWidget(cmbType, 1, 1, 1, 2);
    layout->addWidget(new QLabel(tr("Value:")), 2, 0);
    layout->addWidget(txtValue, 2, 1);
    layout->addWidget(new QLabel(tr("Transient value:")), 3, 0);
    layout->addWidget(txtValueTransient, 3, 1);
}

void DSceneEdgeAcousticMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeAcousticMarker *edgeAcousticMarker = dynamic_cast<SceneEdgeAcousticMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeAcousticMarker->type));
    txtValue->setValue(edgeAcousticMarker->value_real);

    // transient
    txtValueTransient->setText(edgeAcousticMarker->value_transient.function());
}

bool DSceneEdgeAcousticMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeAcousticMarker *edgeAcousticMarker = dynamic_cast<SceneEdgeAcousticMarker *>(m_edgeMarker);

    edgeAcousticMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeAcousticMarker->value_real = txtValue->value();
    else
        return false;

    // transient
    TimeFunction valueTransient(txtValueTransient->text(), 0.0, Util::scene()->problemInfo()->timeTotal.number);
    if (valueTransient.isValid())
        edgeAcousticMarker->value_transient = valueTransient;
    else
    {
        valueTransient.showError();
        return false;
    }

    return true;
}

void DSceneEdgeAcousticMarker::doTypeChanged(int index)
{
    txtValue->setEnabled(false);
    txtValueTransient->setEnabled(false);

    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_Acoustic_Pressure:
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            txtValue->setEnabled(true);
        else
            txtValueTransient->setEnabled(true);
        break;
    case PhysicFieldBC_Acoustic_Impedance:
    {
        txtValue->setEnabled(true);
    }
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
    {
    }
        break;
    }
}

// *************************************************************************************************************************************

DSceneLabelAcousticMarker::DSceneLabelAcousticMarker(QWidget *parent, SceneLabelAcousticMarker *labelAcousticMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelAcousticMarker;

    createDialog();

    load();
    setSize();
}

void DSceneLabelAcousticMarker::createContent()
{
    txtDensity = new SLineEditValue(this);
    txtSpeed = new SLineEditValue(this);

    connect(txtDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtSpeed, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(new QLabel(tr("Density (kg/m3):")), 1, 0);
    layout->addWidget(txtDensity, 1, 1);
    layout->addWidget(new QLabel(tr("Speed of sound (m/s):")), 2, 0);
    layout->addWidget(txtSpeed, 2, 1);
}

void DSceneLabelAcousticMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelAcousticMarker *labelAcousticMarker = dynamic_cast<SceneLabelAcousticMarker *>(m_labelMarker);

    txtDensity->setValue(labelAcousticMarker->density);
    txtSpeed->setValue(labelAcousticMarker->speed);
}

bool DSceneLabelAcousticMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelAcousticMarker *labelAcousticMarker = dynamic_cast<SceneLabelAcousticMarker *>(m_labelMarker);

    if (txtDensity->evaluate())
        labelAcousticMarker->density  = txtDensity->value();
    else
        return false;

    if (txtSpeed->evaluate())
        labelAcousticMarker->speed  = txtSpeed->value();
    else
        return false;

    return true;
}
