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

#include "hermes_heat.h"
#include "scene.h"

struct HeatEdge
{
    PhysicFieldBC type;
    double temperature;
    double heatFlux;
    double h;
    double externalTemperature;
};

struct HeatLabel
{
    double thermal_conductivity;
    double volume_heat;
    double density;
    double specific_heat;
};

HeatEdge *heatEdge;
HeatLabel *heatLabel;

template<typename Real, typename Scalar>
Scalar heat_matrix_form_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double h = 0.0;

    if (heatEdge[e->edge_marker].type == PhysicFieldBC_Heat_Flux)
        h = heatEdge[e->edge_marker].h;

    if (isPlanar)
        return h * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return h * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_vector_form_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (heatEdge[e->edge_marker].type == PhysicFieldBC_None)
        return 0.0;

    double q = 0.0;
    double h = 0.0;
    double Text = 0.0;

    if (heatEdge[e->edge_marker].type == PhysicFieldBC_Heat_Flux)
    {
        q = heatEdge[e->edge_marker].heatFlux;
        h = heatEdge[e->edge_marker].h;
        Text = heatEdge[e->edge_marker].externalTemperature;
    }

    if (isPlanar)
        return (q + Text * h) * int_v<Real, Scalar>(n, wt, v);
    else
        return (q + Text * h) * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_matrix_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return heatLabel[e->elem_marker].thermal_conductivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)
        + ((analysisType == AnalysisType_Transient) ? heatLabel[e->elem_marker].density * heatLabel[e->elem_marker].specific_heat * int_u_v<Real, Scalar>(n, wt, u, v) / timeStep : 0.0);
    else
        return heatLabel[e->elem_marker].thermal_conductivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e)
                + ((analysisType == AnalysisType_Transient) ? heatLabel[e->elem_marker].density * heatLabel[e->elem_marker].specific_heat * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e) / timeStep : 0.0);
}

template<typename Real, typename Scalar>
Scalar heat_vector_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (isPlanar)
        return heatLabel[e->elem_marker].volume_heat * int_v<Real, Scalar>(n, wt, v)
        + ((analysisType == AnalysisType_Transient) ? heatLabel[e->elem_marker].density * heatLabel[e->elem_marker].specific_heat * int_u_v<Real, Scalar>(n, wt, ext->fn[0], v) / timeStep : 0.0);
    else
        return heatLabel[e->elem_marker].volume_heat * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e)
        + ((analysisType == AnalysisType_Transient) ? heatLabel[e->elem_marker].density * heatLabel[e->elem_marker].specific_heat * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, ext->fn[0], v, e) / timeStep : 0.0);
}

void callbackHeatWeakForm(WeakForm *wf, Tuple<Solution *> slnArray)
{
    wf->add_matrix_form(0, 0, callback(heat_matrix_form));
    if (analysisType == AnalysisType_Transient)
        wf->add_vector_form(0, callback(heat_vector_form), HERMES_ANY, slnArray.at(0));
    else
        wf->add_vector_form(0, callback(heat_vector_form));
    wf->add_matrix_form_surf(0, 0, callback(heat_matrix_form_surf));
    wf->add_vector_form_surf(0, callback(heat_vector_form_surf));
}

// *******************************************************************************************************

void HermesHeat::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
        // TODO
        break;
    case PhysicFieldBC_Heat_Temperature:
        Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("temperature", "0"))));
        break;
    case PhysicFieldBC_Heat_Flux:
        Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(element->attribute("name"), type,
                                                             Value(element->attribute("heat_flux", "0")),
                                                             Value(element->attribute("h", "0")),
                                                             Value(element->attribute("external_temperature", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesHeat::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeHeatMarker->type));

    if (edgeHeatMarker->type == PhysicFieldBC_Heat_Temperature)
    {
        element->setAttribute("temperature", edgeHeatMarker->temperature.text);
    }
    if (edgeHeatMarker->type == PhysicFieldBC_Heat_Flux)
    {
        element->setAttribute("heat_flux", edgeHeatMarker->heatFlux.text);
        element->setAttribute("h", edgeHeatMarker->h.text);
        element->setAttribute("external_temperature", edgeHeatMarker->externalTemperature.text);
    }
}

void HermesHeat::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelHeatMarker(element->attribute("name"),
                                                           Value(element->attribute("volume_heat", "0")),
                                                           Value(element->attribute("thermal_conductivity", "0")),
                                                           Value(element->attribute("density", "0")),
                                                           Value(element->attribute("specific_heat", "0"))));
}

void HermesHeat::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(marker);

    element->setAttribute("thermal_conductivity", labelHeatMarker->thermal_conductivity.text);
    element->setAttribute("volume_heat", labelHeatMarker->volume_heat.text);
    element->setAttribute("density", labelHeatMarker->density.text);
    element->setAttribute("specific_heat", labelHeatMarker->specific_heat.text);
}

LocalPointValue *HermesHeat::localPointValue(Point point)
{
    return new LocalPointValueHeat(point);
}

QStringList HermesHeat::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "t" << "T" << "Gx" << "Gy" << "G" << "Fx" << "Fy" << "F" << "lambda";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesHeat::surfaceIntegralValue()
{
    return new SurfaceIntegralValueHeat();
}

QStringList HermesHeat::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S" << "T_avg" << "T_diff" << "F";
    return QStringList(headers);
}

VolumeIntegralValue *HermesHeat::volumeIntegralValue()
{
    return new VolumeIntegralValueHeat();
}

QStringList HermesHeat::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "T_avg" << "Gx_avg" << "Gy_avg" << "G_avg" << "Fx_avg" << "Fy_avg" << "F_avg";
    return QStringList(headers);
}

SceneEdgeMarker *HermesHeat::newEdgeMarker()
{
    return new SceneEdgeHeatMarker(tr("new boundary"),
                                   PhysicFieldBC_Heat_Temperature,
                                   Value("0"));
}

SceneEdgeMarker *HermesHeat::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Temperature)
            return new SceneEdgeHeatMarker(name,
                                           physicFieldBCFromStringKey(type),
                                           Value(QString::number(value)));
        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Flux)
            return new SceneEdgeHeatMarker(name,
                                           physicFieldBCFromStringKey(type),
                                           Value(QString::number(value)),
                                           Value(QString::number(h)),
                                           Value(QString::number(externaltemperature)));
    }

    return NULL;
}

SceneEdgeMarker *HermesHeat::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        if (SceneEdgeHeatMarker *marker = dynamic_cast<SceneEdgeHeatMarker *>(Util::scene()->getEdgeMarker(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
                return NULL;
            }

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Temperature)
                marker->temperature = Value(QString::number(value));

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Flux)
            {
                marker->heatFlux = Value(QString::number(value));
                marker->h = Value(QString::number(h));
                marker->externalTemperature = Value(QString::number(externaltemperature));
                return marker;
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

SceneLabelMarker *HermesHeat::newLabelMarker()
{
    return new SceneLabelHeatMarker(tr("new material"),
                                    Value("0"),
                                    Value("385"),
                                    Value("0"),
                                    Value("0"));
}

SceneLabelMarker *HermesHeat::newLabelMarker(PyObject *self, PyObject *args)
{
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelHeatMarker(name,
                                        Value(QString::number(volume_heat)),
                                        Value(QString::number(thermal_conductivity)),
                                        Value(QString::number(density)),
                                        Value(QString::number(specific_heat)));
    }

    return NULL;
}

SceneLabelMarker *HermesHeat::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        if (SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->volume_heat = Value(QString::number(volume_heat));
            marker->thermal_conductivity = Value(QString::number(thermal_conductivity));
            marker->density = Value(QString::number(density));
            marker->specific_heat = Value(QString::number(specific_heat));
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

void HermesHeat::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueHeat *localPointValueHeat = dynamic_cast<LocalPointValueHeat *>(localPointValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    // Thermal Conductivity
    addTreeWidgetItemValue(heatNode, tr("Thermal conductivity:"), QString("%1").arg(localPointValueHeat->thermal_conductivity, 0, 'f', 2), tr("W/m.K"));

    // Volume Heat
    addTreeWidgetItemValue(heatNode, tr("Volume heat:"), QString("%1").arg(localPointValueHeat->volume_heat, 0, 'e', 3), tr("W/m3"));

    // Temperature
    addTreeWidgetItemValue(heatNode, tr("Temperature:"), QString("%1").arg(localPointValueHeat->temperature, 0, 'f', 2), tr("deg."));

    // Heat Flux
    QTreeWidgetItem *itemHeatFlux = new QTreeWidgetItem(heatNode);
    itemHeatFlux->setText(0, tr("Heat flux"));
    itemHeatFlux->setExpanded(true);

    addTreeWidgetItemValue(itemHeatFlux, "F" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHeat->F.x, 0, 'e', 3), tr("W/m2"));
    addTreeWidgetItemValue(itemHeatFlux, "F" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHeat->F.y, 0, 'e', 3), tr("W/m2"));
    addTreeWidgetItemValue(itemHeatFlux, "F:", QString("%1").arg(localPointValueHeat->F.magnitude(), 0, 'e', 3), "W/m2");

    // Temperature Gradient
    QTreeWidgetItem *itemTemperatureGradient = new QTreeWidgetItem(heatNode);
    itemTemperatureGradient->setText(0, tr("Temperature gradient"));
    itemTemperatureGradient->setExpanded(true);

    addTreeWidgetItemValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHeat->G.x, 0, 'f', 5), tr("deg./m"));
    addTreeWidgetItemValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHeat->G.y, 0, 'f', 5), tr("deg./m"));
    addTreeWidgetItemValue(itemTemperatureGradient, "G:", QString("%1").arg(localPointValueHeat->G.magnitude(), 0, 'f', 5), "deg./m");
}

void HermesHeat::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueHeat *surfaceIntegralValueHeat = dynamic_cast<SurfaceIntegralValueHeat *>(surfaceIntegralValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat Transfer"));
    heatNode->setExpanded(true);

    addTreeWidgetItemValue(heatNode, tr("Temperature avg.:"), QString("%1").arg(surfaceIntegralValueHeat->averageTemperature, 0, 'e', 3), tr("deg."));
    addTreeWidgetItemValue(heatNode, tr("Temperature dif.:"), QString("%1").arg(surfaceIntegralValueHeat->temperatureDifference, 0, 'e', 3), tr("deg."));
    addTreeWidgetItemValue(heatNode, tr("Heat flux:"), QString("%1").arg(surfaceIntegralValueHeat->heatFlux, 0, 'e', 3), tr("W"));
}

void HermesHeat::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueHeat *volumeIntegralValueHeat = dynamic_cast<VolumeIntegralValueHeat *>(volumeIntegralValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    addTreeWidgetItemValue(heatNode, tr("Temperature avg.:"), QString("%1").arg(volumeIntegralValueHeat->averageTemperature, 0, 'e', 3), tr("deg."));
}

ViewScalarFilter *HermesHeat::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep());
    return new ViewScalarFilterHeat(sln1,
                                    physicFieldVariable,
                                    physicFieldVariableComp);
}

QList<SolutionArray *> *HermesHeat::solve(ProgressItemSolve *progressItemSolve)
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return NULL;
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return NULL;
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return NULL;
    }

    // edge markers
    BCTypes bcTypes;
    BCValues bcValues;

    heatEdge = new HeatEdge[Util::scene()->edges.count()+1];
    heatEdge[0].type = PhysicFieldBC_None;
    heatEdge[0].temperature = 0;
    heatEdge[0].heatFlux = 0;
    heatEdge[0].h = 0;
    heatEdge[0].externalTemperature = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            heatEdge[i+1].type = PhysicFieldBC_None;
        }
        else
        {
            SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(Util::scene()->edges[i]->marker);
            heatEdge[i+1].type = edgeHeatMarker->type;
            switch (edgeHeatMarker->type)
            {
            case PhysicFieldBC_None:
                {
                    bcTypes.add_bc_none(i+1);
                }
                break;
            case PhysicFieldBC_Heat_Temperature:
                {
                    // evaluate script
                    if (!edgeHeatMarker->temperature.evaluate()) return NULL;

                    heatEdge[i+1].temperature = edgeHeatMarker->temperature.number;

                    bcTypes.add_bc_dirichlet(i+1);
                    bcValues.add_const(i+1, edgeHeatMarker->temperature.number);
                }
                break;
            case PhysicFieldBC_Heat_Flux:
                {
                    // evaluate script
                    if (!edgeHeatMarker->heatFlux.evaluate()) return NULL;
                    if (!edgeHeatMarker->h.evaluate()) return NULL;
                    if (!edgeHeatMarker->externalTemperature.evaluate()) return NULL;

                    heatEdge[i+1].heatFlux = edgeHeatMarker->heatFlux.number;
                    heatEdge[i+1].h = edgeHeatMarker->h.number;
                    heatEdge[i+1].externalTemperature = edgeHeatMarker->externalTemperature.number;

                    bcTypes.add_bc_newton(i+1);
                }
                break;
            }
        }
    }

    // label markers
    heatLabel = new HeatLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelHeatMarker->thermal_conductivity.evaluate()) return NULL;
            if (!labelHeatMarker->volume_heat.evaluate()) return NULL;
            if (!labelHeatMarker->density.evaluate()) return NULL;
            if (!labelHeatMarker->specific_heat.evaluate()) return NULL;

            heatLabel[i].thermal_conductivity = labelHeatMarker->thermal_conductivity.number;
            heatLabel[i].volume_heat = labelHeatMarker->volume_heat.number;
            heatLabel[i].density = labelHeatMarker->density.number;
            heatLabel[i].specific_heat = labelHeatMarker->specific_heat.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = solveSolutioArray(progressItemSolve,
                                                                  Tuple<BCTypes *>(&bcTypes),
                                                                  Tuple<BCValues *>(&bcValues),
                                                                  callbackHeatWeakForm);

    delete [] heatEdge;
    delete [] heatLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueHeat::LocalPointValueHeat(Point &point) : LocalPointValue(point)
{
    thermal_conductivity = 0;
    volume_heat = 0;

    temperature = 0;
    G = Point();
    F = Point();

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (labelMarker)
        {
            // temperature
            temperature = value;

            // temperature gradient
            G = derivative * (-1);

            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);

            thermal_conductivity = marker->thermal_conductivity.number;
            volume_heat = marker->volume_heat.number;

            // heat flux
            F = G * marker->thermal_conductivity.number;
        }
    }
}

double LocalPointValueHeat::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Heat_Temperature:
        {
            return temperature;
        }
        break;
    case PhysicFieldVariable_Heat_TemperatureGradient:
        {
            switch (physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                return G.x;
                break;
            case PhysicFieldVariableComp_Y:
                return G.y;
                break;
            case PhysicFieldVariableComp_Magnitude:
                return G.magnitude();
                break;
            }
        }
        break;
    case PhysicFieldVariable_Heat_Flux:
        {
            switch (physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                return F.x;
                break;
            case PhysicFieldVariableComp_Y:
                return F.y;
                break;
            case PhysicFieldVariableComp_Magnitude:
                return F.magnitude();
                break;
            }
        }
        break;
    case PhysicFieldVariable_Heat_Conductivity:
        {
            return thermal_conductivity;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueHeat::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueHeat::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(Util::scene()->sceneSolution()->time(), 0, 'e', 5) <<
            QString("%1").arg(temperature, 0, 'e', 5) <<
            QString("%1").arg(G.x, 0, 'e', 5) <<
            QString("%1").arg(G.y, 0, 'e', 5) <<
            QString("%1").arg(G.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(F.x, 0, 'e', 5) <<
            QString("%1").arg(F.y, 0, 'e', 5) <<
            QString("%1").arg(F.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(thermal_conductivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueHeat::SurfaceIntegralValueHeat() : SurfaceIntegralValue()
{
    averageTemperature = 0.0;
    temperatureDifference = 0.0;
    heatFlux = 0.0;

    calculate();

    if (length > 0.0)
    {
        averageTemperature /= length;
    }
    temperatureDifference /= 2.0;
    averageTemperature /= 2.0;
    heatFlux /= 2.0;
}

void SurfaceIntegralValueHeat::calculateVariables(int i)
{
    if (boundary)
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            averageTemperature += pt[i][2] * tan[i][2] * value[i];
        else
            averageTemperature += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * value[i];

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            temperatureDifference += pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);
        else
            temperatureDifference += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);

        SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[e->marker]->marker);
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            heatFlux -= pt[i][2] * tan[i][2] * marker->thermal_conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
        else
            heatFlux -= 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->thermal_conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
    }
}

QStringList SurfaceIntegralValueHeat::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5) <<
            QString("%1").arg(averageTemperature, 0, 'e', 5) <<
            QString("%1").arg(temperatureDifference, 0, 'e', 5) <<
            QString("%1").arg(heatFlux, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueHeat::VolumeIntegralValueHeat() : VolumeIntegralValue()
{
    averageTemperature = 0;

    calculate();

    if (volume > 0)
    {
        averageTemperature /= volume;
    }
}

void VolumeIntegralValueHeat::calculateVariables(int i)
{
    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        h1_integrate_expression(value1[i]);
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * value1[i]);
    }
    averageTemperature += result;
}

void VolumeIntegralValueHeat::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep());
    sln2 = NULL;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() - 1);
}

QStringList VolumeIntegralValueHeat::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(averageTemperature, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterHeat::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Heat_Temperature:
        {
            node->values[0][0][i] = value1[i];
        }
        break;
    case PhysicFieldVariable_Heat_TemperatureGradient:
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
    case PhysicFieldVariable_Heat_Flux:
        {
            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                {
                    node->values[0][0][i] = - marker->thermal_conductivity.number * dudx1[i];
                }
                break;
            case PhysicFieldVariableComp_Y:
                {
                    node->values[0][0][i] = - marker->thermal_conductivity.number * dudy1[i];
                }
                break;
            case PhysicFieldVariableComp_Magnitude:
                {
                    node->values[0][0][i] =  marker->thermal_conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                }
                break;
            }
        }
        break;
    case PhysicFieldVariable_Heat_Conductivity:
        {
            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);
            node->values[0][0][i] = marker->thermal_conductivity.number;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterHeat::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneEdgeHeatMarker::SceneEdgeHeatMarker(const QString &name, PhysicFieldBC type, Value temperature) : SceneEdgeMarker(name, type)
{
    this->temperature = temperature;
}

SceneEdgeHeatMarker::SceneEdgeHeatMarker(const QString &name, PhysicFieldBC type, Value heatFlux, Value h, Value externalTemperature) : SceneEdgeMarker(name, type)
{
    this->heatFlux = heatFlux;
    this->h = h;
    this->externalTemperature = externalTemperature;
}

QString SceneEdgeHeatMarker::script()
{
    if (type == PhysicFieldBC_Heat_Temperature)
    {
        return QString("addboundary(\"%1\", \"%2\", %3)").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(temperature.text);
    }
    if (type == PhysicFieldBC_Heat_Flux)
    {
        return QString("addboundary(\"%1\", \"%2\", %3, %4, %5)").
                arg(name).
                arg(physicFieldBCToStringKey(type)).
                arg(heatFlux.text).
                arg(h.text).
                arg(externalTemperature.text);
    }
}

QMap<QString, QString> SceneEdgeHeatMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Heat_Temperature:
        out["Temperature (deg.)"] = temperature.text;
        break;
    case PhysicFieldBC_Heat_Flux:
        out["Heat flux (W/m2)"] = heatFlux.text;
        out["Heat transfer coef. (Q/m2.K)"] = h.text;
        out["External temperature (deg.)"] = externalTemperature.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeHeatMarker::showDialog(QWidget *parent)
{
    DSceneEdgeHeatMarker *dialog = new DSceneEdgeHeatMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelHeatMarker::SceneLabelHeatMarker(const QString &name, Value volume_heat, Value thermal_conductivity, Value density, Value specific_heat)
    : SceneLabelMarker(name)
{
    this->thermal_conductivity = thermal_conductivity;
    this->volume_heat = volume_heat;
    this->density = density;
    this->specific_heat = specific_heat;
}

QString SceneLabelHeatMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4, %5)").
            arg(name).
            arg(volume_heat.text).
            arg(thermal_conductivity.text).
            arg(density.text).
            arg(specific_heat.text);
}

QMap<QString, QString> SceneLabelHeatMarker::data()
{
    QMap<QString, QString> out;
    out["Volume heat (W/m3)"] = volume_heat.text;
    out["Thermal conductivity (W/m.K)"] = thermal_conductivity.text;
    out["Density (kg/m3)"] = density.text;
    out["Specific heat (J/kg.K)"] = specific_heat.text;
    return QMap<QString, QString>(out);
}

int SceneLabelHeatMarker::showDialog(QWidget *parent)
{
    DSceneLabelHeatMarker *dialog = new DSceneLabelHeatMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeHeatMarker::DSceneEdgeHeatMarker(SceneEdgeHeatMarker *edgeEdgeHeatMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeEdgeHeatMarker;

    createDialog();

    load();
    setSize();
}

DSceneEdgeHeatMarker::~DSceneEdgeHeatMarker()
{
    delete cmbType;
    delete txtTemperature;
    delete txtHeatFlux;
    delete txtHeatTransferCoefficient;
    delete txtExternalTemperature;
}

void DSceneEdgeHeatMarker::createContent()
{
    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Heat_Temperature), PhysicFieldBC_Heat_Temperature);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Heat_Flux), PhysicFieldBC_Heat_Flux);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtHeatFlux = new SLineEditValue(this);
    txtTemperature = new SLineEditValue(this);
    txtHeatTransferCoefficient = new SLineEditValue(this);
    txtExternalTemperature = new SLineEditValue(this);

    connect(txtHeatFlux, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtHeatTransferCoefficient, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtExternalTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    layout->addWidget(new QLabel(tr("BC type:")), 1, 0);
    layout->addWidget(cmbType, 1, 1);
    layout->addWidget(new QLabel(tr("Temperature (deg.):")), 2, 0);
    layout->addWidget(txtTemperature, 2, 1);
    layout->addWidget(new QLabel(tr("Heat flux (W/m2):")), 3, 0);
    layout->addWidget(txtHeatFlux, 3, 1);
    layout->addWidget(new QLabel(tr("Heat transfer coef. (W/m2.K):")), 4, 0);
    layout->addWidget(txtHeatTransferCoefficient, 4, 1);
    layout->addWidget(new QLabel(tr("External temperature (deg.):")), 5, 0);
    layout->addWidget(txtExternalTemperature, 5, 1);
}

void DSceneEdgeHeatMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeHeatMarker->type));
    switch (edgeHeatMarker->type)
    {
    case PhysicFieldBC_Heat_Temperature:
        {
            txtTemperature->setValue(edgeHeatMarker->temperature);
        }
        break;
    case PhysicFieldBC_Heat_Flux:
        {
            txtHeatFlux->setValue(edgeHeatMarker->heatFlux);
            txtHeatTransferCoefficient->setValue(edgeHeatMarker->h);
            txtExternalTemperature->setValue(edgeHeatMarker->externalTemperature);
        }
        break;
    }
}

bool DSceneEdgeHeatMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(m_edgeMarker);

    edgeHeatMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();
    switch (edgeHeatMarker->type)
    {
    case PhysicFieldBC_Heat_Temperature:
        {
            if (txtTemperature->evaluate())
                edgeHeatMarker->temperature  = txtTemperature->value();
            else
                return false;
        }
        break;
    case PhysicFieldBC_Heat_Flux:
        {
            if (txtHeatFlux->evaluate())
                edgeHeatMarker->heatFlux  = txtHeatFlux->value();
            else
                return false;
            if (txtHeatTransferCoefficient->evaluate())
                edgeHeatMarker->h  = txtHeatTransferCoefficient->value();
            else
                return false;
            if (txtExternalTemperature->evaluate())
                edgeHeatMarker->externalTemperature  = txtExternalTemperature->value();
            else
                return false;
        }
        break;
    }

    return true;
}

void DSceneEdgeHeatMarker::doTypeChanged(int index)
{
    txtTemperature->setEnabled(false);
    txtHeatFlux->setEnabled(false);
    txtHeatTransferCoefficient->setEnabled(false);
    txtExternalTemperature->setEnabled(false);

    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_Heat_Temperature:
        {
            txtTemperature->setEnabled(true);
        }
        break;
    case PhysicFieldBC_Heat_Flux:
        {
            txtHeatFlux->setEnabled(true);
            txtHeatTransferCoefficient->setEnabled(true);
            txtExternalTemperature->setEnabled(true);
        }
        break;
    }
}

// *************************************************************************************************************************************

DSceneLabelHeatMarker::DSceneLabelHeatMarker(QWidget *parent, SceneLabelHeatMarker *labelHeatMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelHeatMarker;

    createDialog();

    load();
    setSize();
}

DSceneLabelHeatMarker::~DSceneLabelHeatMarker()
{
    delete txtThermalConductivity;
    delete txtVolumeHeat;
    delete txtDensity;
    delete txtSpecificHeat;
}

void DSceneLabelHeatMarker::createContent()
{
    txtThermalConductivity = new SLineEditValue(this);
    txtThermalConductivity->setMinimumSharp(0.0);
    txtVolumeHeat = new SLineEditValue(this);
    txtDensity = new SLineEditValue(this);
    txtDensity->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);
    txtSpecificHeat = new SLineEditValue(this);
    txtSpecificHeat->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);

    connect(txtThermalConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtVolumeHeat, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtSpecificHeat, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(new QLabel(tr("Thermal conductivity (W/m.K):")), 1, 0);
    layout->addWidget(txtThermalConductivity, 1, 1);
    layout->addWidget(new QLabel(tr("Volume heat (J/m3):")), 2, 0);
    layout->addWidget(txtVolumeHeat, 2, 1);
    layout->addWidget(new QLabel(tr("Density (kg/m3):")), 3, 0);
    layout->addWidget(txtDensity, 3, 1);
    layout->addWidget(new QLabel(tr("Specific heat (J/kg.K):")), 4, 0);
    layout->addWidget(txtSpecificHeat, 4, 1);
}

void DSceneLabelHeatMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(m_labelMarker);

    txtThermalConductivity->setValue(labelHeatMarker->thermal_conductivity);
    txtVolumeHeat->setValue(labelHeatMarker->volume_heat);
    txtDensity->setValue(labelHeatMarker->density);
    txtSpecificHeat->setValue(labelHeatMarker->specific_heat);
}

bool DSceneLabelHeatMarker::save()
{
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(m_labelMarker);

    if (txtThermalConductivity->evaluate())
        labelHeatMarker->thermal_conductivity  = txtThermalConductivity->value();
    else
        return false;

    if (txtVolumeHeat->evaluate())
        labelHeatMarker->volume_heat  = txtVolumeHeat->value();
    else
        return false;

    if (txtDensity->evaluate())
        labelHeatMarker->density  = txtDensity->value();
    else
        return false;

    if (txtSpecificHeat->evaluate())
        labelHeatMarker->specific_heat  = txtSpecificHeat->value();
    else
        return false;

    return true;
}
