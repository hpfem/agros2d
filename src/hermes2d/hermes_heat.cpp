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
#include "gui.h"


class WeakFormHeat : public WeakFormAgros
{
public:
    WeakFormHeat()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryHeat *boundary = dynamic_cast<SceneBoundaryHeat *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == PhysicFieldBC_Heat_Flux)
                {
                    // vector flux term
                    double flux = boundary->heatFlux.number + boundary->h.number * boundary->externalTemperature.number;

                    if (fabs(flux) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        flux,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                    if (fabs(boundary->h.number) > EPS_ZERO)
                        add_matrix_form_surf(new WeakFormsH1::SurfaceMatrixForms::DefaultMatrixFormSurf(0, 0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->h.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               material->thermal_conductivity.number,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));
                if (fabs(material->volume_heat.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->volume_heat.number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));


                // transient analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                    if ((fabs(material->density.number) > EPS_ZERO)
                            && (fabs(material->specific_heat.number) > EPS_ZERO))
                        add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                                  QString::number(i).toStdString(),
                                                                                                  material->density.number * material->specific_heat.number / Util::scene()->problemInfo()->timeStep.number,
                                                                                                  HERMES_SYM,
                                                                                                  convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }
    }

    virtual void updateTimeDep(double time)
    {
        for (unsigned i = 0; i < vfvol.size(); i++)
        {
        }
    };
};

/*
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

void callbackHeatWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    wf->add_matrix_form(0, 0, callback(heat_matrix_form));
    if (analysisType == AnalysisType_Transient)
        wf->add_vector_form(0, callback(heat_vector_form), HERMES_ANY, slnArray.at(0));
    else
        wf->add_vector_form(0, callback(heat_vector_form));
    wf->add_matrix_form_surf(0, 0, callback(heat_matrix_form_surf));
    wf->add_vector_form_surf(0, callback(heat_vector_form_surf));
}
*/

// *******************************************************************************************************

void HermesHeat::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
        // TODO
        break;
    case PhysicFieldBC_Heat_Temperature:
        Util::scene()->addBoundary(new SceneBoundaryHeat(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("temperature", "0"))));
        break;
    case PhysicFieldBC_Heat_Flux:
        Util::scene()->addBoundary(new SceneBoundaryHeat(element->attribute("name"), type,
                                                             Value(element->attribute("heat_flux", "0")),
                                                             Value(element->attribute("h", "0")),
                                                             Value(element->attribute("external_temperature", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesHeat::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryHeat *boundary = dynamic_cast<SceneBoundaryHeat *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(boundary->type));

    if (boundary->type == PhysicFieldBC_Heat_Temperature)
    {
        element->setAttribute("temperature", boundary->temperature.text);
    }
    if (boundary->type == PhysicFieldBC_Heat_Flux)
    {
        element->setAttribute("heat_flux", boundary->heatFlux.text);
        element->setAttribute("h", boundary->h.text);
        element->setAttribute("external_temperature", boundary->externalTemperature.text);
    }
}

void HermesHeat::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialHeat(element->attribute("name"),
                                                           Value(element->attribute("volume_heat", "0")),
                                                           Value(element->attribute("thermal_conductivity", "0")),
                                                           Value(element->attribute("density", "0")),
                                                           Value(element->attribute("specific_heat", "0"))));
}

void HermesHeat::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(marker);

    element->setAttribute("thermal_conductivity", material->thermal_conductivity.text);
    element->setAttribute("volume_heat", material->volume_heat.text);
    element->setAttribute("density", material->density.text);
    element->setAttribute("specific_heat", material->specific_heat.text);
}

LocalPointValue *HermesHeat::localPointValue(const Point &point)
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

SceneBoundary *HermesHeat::newBoundary()
{
    return new SceneBoundaryHeat(tr("new boundary"),
                                   PhysicFieldBC_Heat_Temperature,
                                   Value("0"));
}

SceneBoundary *HermesHeat::newBoundary(PyObject *self, PyObject *args)
{
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Temperature)
            return new SceneBoundaryHeat(name,
                                           physicFieldBCFromStringKey(type),
                                           Value(QString::number(value)));
        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Flux)
            return new SceneBoundaryHeat(name,
                                           physicFieldBCFromStringKey(type),
                                           Value(QString::number(value)),
                                           Value(QString::number(h)),
                                           Value(QString::number(externaltemperature)));
    }

    return NULL;
}

SceneBoundary *HermesHeat::modifyBoundary(PyObject *self, PyObject *args)
{
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        if (SceneBoundaryHeat *marker = dynamic_cast<SceneBoundaryHeat *>(Util::scene()->getBoundary(name)))
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

SceneMaterial *HermesHeat::newMaterial()
{
    return new SceneMaterialHeat(tr("new material"),
                                    Value("0"),
                                    Value("385"),
                                    Value("0"),
                                    Value("0"));
}

SceneMaterial *HermesHeat::newMaterial(PyObject *self, PyObject *args)
{
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialHeat(name,
                                        Value(QString::number(volume_heat)),
                                        Value(QString::number(thermal_conductivity)),
                                        Value(QString::number(density)),
                                        Value(QString::number(specific_heat)));
    }

    return NULL;
}

SceneMaterial *HermesHeat::modifyMaterial(PyObject *self, PyObject *args)
{
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        if (SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(Util::scene()->getMaterial(name)))
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
    addTreeWidgetItemValue(heatNode, tr("Temperature:"), QString("%1").arg(localPointValueHeat->temperature, 0, 'f', 2), tr("K"));

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

    addTreeWidgetItemValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHeat->G.x, 0, 'f', 5), tr("K/m"));
    addTreeWidgetItemValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHeat->G.y, 0, 'f', 5), tr("K/m"));
    addTreeWidgetItemValue(itemTemperatureGradient, "G:", QString("%1").arg(localPointValueHeat->G.magnitude(), 0, 'f', 5), "K/m");
}

void HermesHeat::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueHeat *surfaceIntegralValueHeat = dynamic_cast<SurfaceIntegralValueHeat *>(surfaceIntegralValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat Transfer"));
    heatNode->setExpanded(true);

    addTreeWidgetItemValue(heatNode, tr("Temperature avg.:"), QString("%1").arg(surfaceIntegralValueHeat->averageTemperature, 0, 'e', 3), tr("K"));
    addTreeWidgetItemValue(heatNode, tr("Temperature dif.:"), QString("%1").arg(surfaceIntegralValueHeat->temperatureDifference, 0, 'e', 3), tr("K"));
    addTreeWidgetItemValue(heatNode, tr("Heat flux:"), QString("%1").arg(surfaceIntegralValueHeat->heatFlux, 0, 'e', 3), tr("W"));
}

void HermesHeat::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueHeat *volumeIntegralValueHeat = dynamic_cast<VolumeIntegralValueHeat *>(volumeIntegralValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    addTreeWidgetItemValue(heatNode, tr("Temperature avg.:"), QString("%1").arg(volumeIntegralValueHeat->averageTemperature, 0, 'e', 3), tr("K"));
}

ViewScalarFilter *HermesHeat::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep());
    return new ViewScalarFilterHeat(sln1,
                                    physicFieldVariable,
                                    physicFieldVariableComp);
}

QList<SolutionArray *> HermesHeat::solve(ProgressItemSolve *progressItemSolve)
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return QList<SolutionArray *>();
    }

    // edge markers
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryHeat *boundary = dynamic_cast<SceneBoundaryHeat *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->temperature.evaluate()) return QList<SolutionArray *>();
        if (!boundary->heatFlux.evaluate()) return QList<SolutionArray *>();
        if (!boundary->h.evaluate()) return QList<SolutionArray *>();
        if (!boundary->externalTemperature.evaluate()) return QList<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->thermal_conductivity.evaluate()) return QList<SolutionArray *>();
        if (!material->density.evaluate()) return QList<SolutionArray *>();
        if (!material->specific_heat.evaluate()) return QList<SolutionArray *>();
        if (!material->volume_heat.evaluate()) return QList<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryHeat *boundary = dynamic_cast<SceneBoundaryHeat *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->type == PhysicFieldBC_Heat_Temperature)
                bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->temperature.number));
        }
    }

    WeakFormHeat wf;

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;

    /*
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
        if (Util::scene()->boundarys.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            heatEdge[i+1].type = PhysicFieldBC_None;
        }
        else
        {
            SceneEdgeHeatMarker *boundary = dynamic_cast<SceneEdgeHeatMarker *>(Util::scene()->edges[i]->marker);
            heatEdge[i+1].type = boundary->type;
            switch (boundary->type)
            {
            case PhysicFieldBC_None:
            {
                bcTypes.add_bc_none(i+1);
            }
                break;
            case PhysicFieldBC_Heat_Temperature:
            {
                // evaluate script
                if (!boundary->temperature.evaluate()) return QList<SolutionArray *>();

                heatEdge[i+1].temperature = boundary->temperature.number;

                bcTypes.add_bc_dirichlet(i+1);
                bcValues.add_const(i+1, boundary->temperature.number);
            }
                break;
            case PhysicFieldBC_Heat_Flux:
            {
                // evaluate script
                if (!boundary->heatFlux.evaluate()) return QList<SolutionArray *>();
                if (!boundary->h.evaluate()) return QList<SolutionArray *>();
                if (!boundary->externalTemperature.evaluate()) return QList<SolutionArray *>();

                heatEdge[i+1].heatFlux = boundary->heatFlux.number;
                heatEdge[i+1].h = boundary->h.number;
                heatEdge[i+1].externalTemperature = boundary->externalTemperature.number;

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
        if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelHeatMarker *material = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!material->thermal_conductivity.evaluate()) return QList<SolutionArray *>();
            if (!material->density.evaluate()) return QList<SolutionArray *>();
            if (!material->specific_heat.evaluate()) return QList<SolutionArray *>();
            if (!material->volume_heat.evaluate()) return QList<SolutionArray *>();

                heatLabel[i].thermal_conductivity = material->thermal_conductivity.number;
            heatLabel[i].density = material->density.number;
            heatLabel[i].specific_heat = material->specific_heat.number;
            heatLabel[i].volume_heat = material->volume_heat.number;
        }
    }

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve,
                                                                 Hermes::vector<BCTypes *>(&bcTypes),
                                                                 Hermes::vector<BCValues *>(&bcValues),
                                                                 callbackHeatWeakForm);

    delete [] heatEdge;
    delete [] heatLabel;

    return solutionArrayList;
    */
}

void HermesHeat::updateTimeFunctions(WeakFormAgros *wf, double time)
{
    // update markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(Util::scene()->materials[i]);
        material->volume_heat.evaluate(time);
    }

    // update weakform
    wf->updateTimeDep(time);
}

// ****************************************************************************************************************

LocalPointValueHeat::LocalPointValueHeat(const Point &point) : LocalPointValue(point)
{
    thermal_conductivity = 0;
    volume_heat = 0;

    temperature = 0;
    G = Point();
    F = Point();

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (material)
        {
            // temperature
            temperature = value;

            // temperature gradient
            G = derivative * (-1);

            SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(material);

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
        SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(material);

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            averageTemperature += pt[i][2] * tan[i][2] * value[i];
        else
            averageTemperature += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * value[i];

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            temperatureDifference += pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);
        else
            temperatureDifference += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);

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
        SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(material);
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
        SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(material);
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

SceneBoundaryHeat::SceneBoundaryHeat(const QString &name, PhysicFieldBC type, Value temperature) : SceneBoundary(name, type)
{
    this->temperature = temperature;
}

SceneBoundaryHeat::SceneBoundaryHeat(const QString &name, PhysicFieldBC type, Value heatFlux, Value h, Value externalTemperature) : SceneBoundary(name, type)
{
    this->heatFlux = heatFlux;
    this->h = h;
    this->externalTemperature = externalTemperature;
}

QString SceneBoundaryHeat::script()
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

QMap<QString, QString> SceneBoundaryHeat::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Heat_Temperature:
        out["Temperature (K)"] = temperature.text;
        break;
    case PhysicFieldBC_Heat_Flux:
        out["Heat flux (W/m2)"] = heatFlux.text;
        out["Heat transfer coef. (Q/m2.K)"] = h.text;
        out["External temperature (K)"] = externalTemperature.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneBoundaryHeat::showDialog(QWidget *parent)
{
    SceneBoundaryHeatDialog *dialog = new SceneBoundaryHeatDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialHeat::SceneMaterialHeat(const QString &name, Value volume_heat, Value thermal_conductivity, Value density, Value specific_heat)
    : SceneMaterial(name)
{
    this->thermal_conductivity = thermal_conductivity;
    this->volume_heat = volume_heat;
    this->density = density;
    this->specific_heat = specific_heat;
}

QString SceneMaterialHeat::script()
{
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState)
        return QString("addmaterial(\"%1\", %2, %3, %4, %5)").
                arg(name).
                arg(volume_heat.text).
                arg(thermal_conductivity.text).
                arg(density.text).
                arg(specific_heat.text);
    else
        return QString("addmaterial(\"%1\", \"%2\", %3, %4, %5)").
                arg(name).
                arg(volume_heat.text).
                arg(thermal_conductivity.text).
                arg(density.text).
                arg(specific_heat.text);
}

QMap<QString, QString> SceneMaterialHeat::data()
{
    QMap<QString, QString> out;
    out["Volume heat (W/m3)"] = volume_heat.text;
    out["Thermal conductivity (W/m.K)"] = thermal_conductivity.text;
    out["Density (kg/m3)"] = density.text;
    out["Specific heat (J/kg.K)"] = specific_heat.text;
    return QMap<QString, QString>(out);
}

int SceneMaterialHeat::showDialog(QWidget *parent)
{
    SceneMaterialHeatDialog *dialog = new SceneMaterialHeatDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryHeatDialog::SceneBoundaryHeatDialog(SceneBoundaryHeat *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryHeatDialog::createContent()
{
    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Heat_Temperature), PhysicFieldBC_Heat_Temperature);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Heat_Flux), PhysicFieldBC_Heat_Flux);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtHeatFlux = new ValueLineEdit(this);
    txtTemperature = new ValueLineEdit(this);
    txtHeatTransferCoefficient = new ValueLineEdit(this);
    txtExternalTemperature = new ValueLineEdit(this);

    connect(txtHeatFlux, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtHeatTransferCoefficient, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtExternalTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());
    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>0</sub> (K)"),
                                  tr("Temperature")), 11, 0);
    layout->addWidget(txtTemperature, 11, 2);
    layout->addWidget(createLabel(tr("<i>f</i><sub>0</sub> (W/m<sup>2</sup>)"),
                                  tr("Heat flux")), 12, 0);
    layout->addWidget(txtHeatFlux, 12, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (W/m<sup>2</sup>·K)").arg(QString::fromUtf8("α")),
                                  tr("Heat transfer coef.")), 13, 0);
    layout->addWidget(txtHeatTransferCoefficient, 13, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>ext</sub> (K)"),
                                  tr("External temperature")), 14, 0);
    layout->addWidget(txtExternalTemperature, 14, 2);

}

void SceneBoundaryHeatDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryHeat *boundary = dynamic_cast<SceneBoundaryHeat *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(boundary->type));
    switch (boundary->type)
    {
    case PhysicFieldBC_Heat_Temperature:
    {
        txtTemperature->setValue(boundary->temperature);
    }
        break;
    case PhysicFieldBC_Heat_Flux:
    {
        txtHeatFlux->setValue(boundary->heatFlux);
        txtHeatTransferCoefficient->setValue(boundary->h);
        txtExternalTemperature->setValue(boundary->externalTemperature);
    }
        break;
    }
}

bool SceneBoundaryHeatDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    SceneBoundaryHeat *boundary = dynamic_cast<SceneBoundaryHeat *>(m_boundary);

    boundary->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();
    switch (boundary->type)
    {
    case PhysicFieldBC_Heat_Temperature:
    {
        if (txtTemperature->evaluate())
            boundary->temperature  = txtTemperature->value();
        else
            return false;
    }
        break;
    case PhysicFieldBC_Heat_Flux:
    {
        if (txtHeatFlux->evaluate())
            boundary->heatFlux  = txtHeatFlux->value();
        else
            return false;
        if (txtHeatTransferCoefficient->evaluate())
            boundary->h  = txtHeatTransferCoefficient->value();
        else
            return false;
        if (txtExternalTemperature->evaluate())
            boundary->externalTemperature  = txtExternalTemperature->value();
        else
            return false;
    }
        break;
    }

    return true;
}

void SceneBoundaryHeatDialog::doTypeChanged(int index)
{
    txtTemperature->setEnabled(false);
    txtHeatFlux->setEnabled(false);
    txtHeatTransferCoefficient->setEnabled(false);
    txtExternalTemperature->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, (PhysicFieldBC) cmbType->itemData(index).toInt());

    // enable controls
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

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialHeatDialog::SceneMaterialHeatDialog(SceneMaterialHeat *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialHeatDialog::createContent()
{
    txtThermalConductivity = new ValueLineEdit(this);
    txtThermalConductivity->setMinimumSharp(0.0);
    txtVolumeHeat = new ValueLineEdit(this, true);
    txtDensity = new ValueLineEdit(this);
    txtDensity->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);
    txtSpecificHeat = new ValueLineEdit(this);
    txtSpecificHeat->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);

    connect(txtThermalConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtSpecificHeat, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i> (W/m·K)").arg(QString::fromUtf8("λ")),
                                  tr("Thermal conductivity")), 10, 0);
    layout->addWidget(txtThermalConductivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>Q</i> (W/m<sup>3</sup>)"),
                                  tr("Volume heat")), 11, 0);
    layout->addWidget(txtVolumeHeat, 11, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (kg/m<sup>3</sup>)").arg(QString::fromUtf8("ρ")),
                                  tr("Mass density")), 12, 0);
    layout->addWidget(txtDensity, 12, 2);
    layout->addWidget(createLabel(tr("<i>c</i><sub>p</sub> (J/kg·K)"),
                                  tr("Specific heat")), 13, 0);
    layout->addWidget(txtSpecificHeat, 13, 2);
}

void SceneMaterialHeatDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(m_material);

    txtThermalConductivity->setValue(material->thermal_conductivity);
    txtVolumeHeat->setValue(material->volume_heat);
    txtDensity->setValue(material->density);
    txtSpecificHeat->setValue(material->specific_heat);
}

bool SceneMaterialHeatDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(m_material);

    if (txtThermalConductivity->evaluate())
        material->thermal_conductivity  = txtThermalConductivity->value();
    else
        return false;

    if (txtVolumeHeat->evaluate())
        material->volume_heat  = txtVolumeHeat->value();
    else
        return false;

    if (txtDensity->evaluate())
        material->density  = txtDensity->value();
    else
        return false;

    if (txtSpecificHeat->evaluate())
        material->specific_heat  = txtSpecificHeat->value();
    else
        return false;

    return true;
}
