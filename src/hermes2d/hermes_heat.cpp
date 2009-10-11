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
};


HeatEdge *heatEdge;
HeatLabel *heatLabel;
bool heatPlanar;

int heat_bc_types(int marker)
{
    switch (heatEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        return BC_ESSENTIAL;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        return BC_NATURAL;
    }
}

scalar heat_bc_values(int marker, double x, double y)
{
    switch (heatEdge[marker].type)
    {
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        return heatEdge[marker].temperature;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        return heatEdge[marker].heatFlux;
    }
}

template<typename Real, typename Scalar>
Scalar heat_bilinear_form_surf(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double h = 0.0;

    if (heatEdge[e->marker].type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
        h = heatEdge[e->marker].h;

    if (heatPlanar)
        return h * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return h * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_linear_form_surf(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (heatEdge[e->marker].type == PHYSICFIELDBC_NONE)
        return 0.0;

    double q = 0.0;
    double h = 0.0;
    double Text = 0.0;

    if (heatEdge[e->marker].type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
    {
        q = heatEdge[e->marker].heatFlux;
        h = heatEdge[e->marker].h;
        Text = heatEdge[e->marker].externalTemperature;
    }

    if (heatPlanar)
        return (q + Text * h) * int_v<Real, Scalar>(n, wt, v);
    else
        return (q + Text * h) * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (heatPlanar)
        return heatLabel[e->marker].thermal_conductivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return heatLabel[e->marker].thermal_conductivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (heatPlanar)
        return heatLabel[e->marker].volume_heat * int_v<Real, Scalar>(n, wt, v);
    else
        return heatLabel[e->marker].volume_heat * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

SolutionArray *heat_main(SolverThread *solverThread)
{
    heatPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo().numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo().polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo().adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo().adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo().adaptivityTolerance;

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
    space.set_bc_types(heat_bc_types);
    space.set_bc_values(heat_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs();

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(heat_bilinear_form));
    wf.add_liform(0, callback(heat_linear_form));
    wf.add_biform_surf(0, 0, callback(heat_bilinear_form_surf));
    wf.add_liform_surf(0, callback(heat_linear_form_surf));

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln = new Solution();
    Solution rsln;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    int steps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps;
    for (i = 0; i<(steps); i++)
    {
        space.assign_dofs();

        // initialize the linear system
        LinSystem sys(&wf, &umfpack);
        sys.set_spaces(1, &space);
        sys.set_pss(1, &pss);
        sys.assemble();
        sys.solve(1, sln);

        RefSystem rs(&sys);
        rs.assemble();
        rs.solve(1, &rsln);

        // calculate errors and adapt the solution
        if (adaptivityType != ADAPTIVITYTYPE_NONE)
        {
            H1OrthoHP hp(1, &space);
            error = hp.calc_error(sln, &rsln) * 100;

            // emit signal
            solverThread->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
            if (solverThread->isCanceled()) return NULL;

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            hp.adapt(0.3, 0, (int) adaptivityType);
        }
    }

    // output
    space.assign_dofs();

    SolutionArray *solutionArray = new SolutionArray();
    solutionArray->order1 = new Orderizer();
    solutionArray->order1->process_solution(&space);
    solutionArray->sln1 = sln;
    solutionArray->adaptiveError = error;
    solutionArray->adaptiveSteps = i-1;

    return solutionArray;
}

// *******************************************************************************************************

void HermesHeat::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = PHYSICFIELDBC_UNDEFINED;
    if (element->attribute("type") == physicFieldBCStringKey(PHYSICFIELDBC_NONE))
        type = PHYSICFIELDBC_NONE;
    else if (element->attribute("type") == physicFieldBCStringKey(PHYSICFIELDBC_HEAT_TEMPERATURE))
    {
        type = PHYSICFIELDBC_HEAT_TEMPERATURE;

        Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("temperature"))));
    }
    else if (element->attribute("type") == physicFieldBCStringKey(PHYSICFIELDBC_HEAT_HEAT_FLUX))
    {
        type = PHYSICFIELDBC_HEAT_HEAT_FLUX;

        Util::scene()->addEdgeMarker(new SceneEdgeHeatMarker(element->attribute("name"), type,
                                                             Value(element->attribute("heat_flux")),
                                                             Value(element->attribute("h")),
                                                             Value(element->attribute("external_temperature"))));
    }
    else
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
}

void HermesHeat::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(marker);

    element->setAttribute("type", physicFieldBCStringKey(edgeHeatMarker->type));

    if (edgeHeatMarker->type == PHYSICFIELDBC_HEAT_TEMPERATURE)
    {
        element->setAttribute("temperature", edgeHeatMarker->temperature.text);
    }
    if (edgeHeatMarker->type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
    {
        element->setAttribute("heat_flux", edgeHeatMarker->heatFlux.text);
        element->setAttribute("h", edgeHeatMarker->h.text);
        element->setAttribute("external_temperature", edgeHeatMarker->externalTemperature.text);
    }
}

void HermesHeat::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelHeatMarker(element->attribute("name"),
                                                           Value(element->attribute("volume_heat")),
                                                           Value(element->attribute("thermal_conductivity"))));
}

void HermesHeat::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(marker);

    element->setAttribute("thermal_conductivity", labelHeatMarker->thermal_conductivity.text);
    element->setAttribute("volume_heat", labelHeatMarker->volume_heat.text);
}

LocalPointValue *HermesHeat::localPointValue(Point point)
{
    return new LocalPointValueHeat(point);
}

QStringList HermesHeat::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "T" << "Gx" << "Gy" << "G" << "Fx" << "Fy" << "F" << "lambda";
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
    return new SceneEdgeHeatMarker("new boundary", PHYSICFIELDBC_HEAT_TEMPERATURE, Value("0"));
}

SceneLabelMarker *HermesHeat::newLabelMarker()
{
    return new SceneLabelHeatMarker("new material", Value("0"), Value("385"));
}

void HermesHeat::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueHeat *localPointValueHeat = dynamic_cast<LocalPointValueHeat *>(localPointValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    // Thermal Conductivity
    addTreeWidgetItemValue(heatNode, tr("Thermal conductivity:"), QString("%1").arg(localPointValueHeat->thermal_conductivity, 0, 'f', 2), "W/m.K");

    // Volume Heat
    addTreeWidgetItemValue(heatNode, tr("Volume heat:"), QString("%1").arg(localPointValueHeat->volume_heat, 0, 'e', 3), "W/m3");

    // Temperature
    addTreeWidgetItemValue(heatNode, tr("Temperature:"), QString("%1").arg(localPointValueHeat->temperature, 0, 'f', 2), "deg.");

    // Heat Flux
    QTreeWidgetItem *itemHeatFlux = new QTreeWidgetItem(heatNode);
    itemHeatFlux->setText(0, tr("Heat flux"));
    itemHeatFlux->setExpanded(true);

    addTreeWidgetItemValue(itemHeatFlux, "F" + Util::scene()->problemInfo().labelX().toLower() + ":", QString("%1").arg(localPointValueHeat->F.x, 0, 'e', 3), "W/m2");
    addTreeWidgetItemValue(itemHeatFlux, "F" + Util::scene()->problemInfo().labelY().toLower() + ":", QString("%1").arg(localPointValueHeat->F.y, 0, 'e', 3), "W/m2");
    addTreeWidgetItemValue(itemHeatFlux, "F:", QString("%1").arg(localPointValueHeat->F.magnitude(), 0, 'e', 3), "W/m2");

    // Temperature Gradient
    QTreeWidgetItem *itemTemperatureGradient = new QTreeWidgetItem(heatNode);
    itemTemperatureGradient->setText(0, tr("Temperature gradient"));
    itemTemperatureGradient->setExpanded(true);

    addTreeWidgetItemValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo().labelX().toLower() + ":", QString("%1").arg(localPointValueHeat->G.x, 0, 'f', 5), "K/m");
    addTreeWidgetItemValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo().labelY().toLower() + ":", QString("%1").arg(localPointValueHeat->G.y, 0, 'f', 5), "K/m");
    addTreeWidgetItemValue(itemTemperatureGradient, "G:", QString("%1").arg(localPointValueHeat->G.magnitude(), 0, 'f', 5), "K/m");
}

void HermesHeat::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueHeat *surfaceIntegralValueHeat = dynamic_cast<SurfaceIntegralValueHeat *>(surfaceIntegralValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat Transfer"));
    heatNode->setExpanded(true);

    addTreeWidgetItemValue(heatNode, tr("Temperature avg.:"), tr("%1").arg(surfaceIntegralValueHeat->averageTemperature, 0, 'e', 3), tr("C"));
    addTreeWidgetItemValue(heatNode, tr("Temperature dif.:"), tr("%1").arg(surfaceIntegralValueHeat->temperatureDifference, 0, 'e', 3), tr("C"));
    addTreeWidgetItemValue(heatNode, tr("Heat flux:"), tr("%1").arg(surfaceIntegralValueHeat->heatFlux, 0, 'e', 3), tr("W"));
}

void HermesHeat::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueHeat *volumeIntegralValueHeat = dynamic_cast<VolumeIntegralValueHeat *>(volumeIntegralValue);

    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    addTreeWidgetItemValue(heatNode, tr("Temperature:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperature, 0, 'e', 3), tr("deg."));
    addTreeWidgetItemValue(heatNode, tr("Gx avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperatureGradientX, 0, 'e', 3), tr("K/m"));
    addTreeWidgetItemValue(heatNode, tr("Gy avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperatureGradientY, 0, 'e', 3), tr("K/m"));
    addTreeWidgetItemValue(heatNode, tr("G avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperatureGradient, 0, 'e', 3), tr("K/m"));
    addTreeWidgetItemValue(heatNode, tr("Fx avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageHeatFluxX, 0, 'e', 3), tr("W/m2"));
    addTreeWidgetItemValue(heatNode, tr("Fy avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageHeatFluxY, 0, 'e', 3), tr("W/m2"));
    addTreeWidgetItemValue(heatNode, tr("F avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageHeatFlux, 0, 'e', 3), tr("W/m2"));
}

SolutionArray *HermesHeat::solve(SolverThread *solverThread)
{
    // edge markers
    heatEdge = new HeatEdge[Util::scene()->edges.count()+1];
    heatEdge[0].type = PHYSICFIELDBC_NONE;
    heatEdge[0].temperature = 0;
    heatEdge[0].heatFlux = 0;
    heatEdge[0].h = 0;
    heatEdge[0].externalTemperature = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            heatEdge[i+1].type = PHYSICFIELDBC_NONE;
        }
        else
        {
            SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(Util::scene()->edges[i]->marker);
            heatEdge[i+1].type = edgeHeatMarker->type;
            switch (edgeHeatMarker->type)
            {
            case PHYSICFIELDBC_HEAT_TEMPERATURE:
                {
                    // evaluate script
                    if (!edgeHeatMarker->temperature.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;

                    heatEdge[i+1].temperature = edgeHeatMarker->temperature.number;
                }
                break;
            case PHYSICFIELDBC_HEAT_HEAT_FLUX:
                {
                    // evaluate script
                    if (!edgeHeatMarker->heatFlux.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;
                    if (!edgeHeatMarker->h.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;
                    if (!edgeHeatMarker->externalTemperature.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;

                    heatEdge[i+1].heatFlux = edgeHeatMarker->heatFlux.number;
                    heatEdge[i+1].h = edgeHeatMarker->h.number;
                    heatEdge[i+1].externalTemperature = edgeHeatMarker->externalTemperature.number;
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
            if (!labelHeatMarker->thermal_conductivity.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;
            if (!labelHeatMarker->volume_heat.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;

            heatLabel[i].thermal_conductivity = labelHeatMarker->thermal_conductivity.number;
            heatLabel[i].volume_heat = labelHeatMarker->volume_heat.number;
        }
    }

    SolutionArray *solutionArray = heat_main(solverThread);

    delete [] heatEdge;
    delete [] heatLabel;

    return solutionArray;
}

// ****************************************************************************************************************

LocalPointValueHeat::LocalPointValueHeat(Point &point) : LocalPointValue(point)
{
    thermal_conductivity = 0;
    volume_heat = 0;

    temperature = 0;
    G = Point();
    F = Point();

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
        if (value.marker != NULL)
        {
            // temperature
            temperature = value.value;

            // temperature gradient
            G = value.derivative * (-1);

            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(value.marker);

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
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        {
            return temperature;
        }
        break;
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return G.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return G.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return G.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return F.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return F.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return F.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
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
    averageTemperature = 0;
    temperatureDifference = 0;
    heatFlux = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        for (int i = 0; i<Util::scene()->edges.length(); i++)
        {
            if (Util::scene()->edges[i]->isSelected)
            {
                averageTemperature += Util::scene()->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE);
                temperatureDifference += Util::scene()->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE_DIFFERENCE);
                heatFlux += Util::scene()->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_HEAT_FLUX);
            }
        }

        if (length > 0)
        {
            averageTemperature /= length;
        }
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
    if (Util::scene()->sceneSolution()->isSolved())
    {
        averageTemperature = 0;
        averageTemperatureGradientX = 0;
        averageTemperatureGradientY = 0;
        averageTemperatureGradient = 0;
        averageHeatFluxX = 0;
        averageHeatFluxY = 0;
        averageHeatFlux = 0;
        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                averageTemperature += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE);
                averageTemperatureGradientX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_X);
                averageTemperatureGradientY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_Y);
                averageTemperatureGradient += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT);
                averageHeatFluxX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_X);
                averageHeatFluxY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_Y);
                averageHeatFlux += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX);
            }
        }

        if (volume > 0)
        {
            averageTemperature /= volume;
            averageTemperatureGradientX /= volume;
            averageTemperatureGradientY /= volume;
            averageTemperatureGradient /= volume;
            averageHeatFluxX /= volume;
            averageHeatFluxY /= volume;
            averageHeatFlux /= volume;
        }
    }
}

QStringList VolumeIntegralValueHeat::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(averageTemperature, 0, 'e', 5) <<
            QString("%1").arg(averageTemperatureGradientX, 0, 'e', 5) <<
            QString("%1").arg(averageTemperatureGradientY, 0, 'e', 5) <<
            QString("%1").arg(averageTemperatureGradient, 0, 'e', 5) <<
            QString("%1").arg(averageHeatFluxX, 0, 'e', 5) <<
            QString("%1").arg(averageHeatFluxY, 0, 'e', 5) <<
            QString("%1").arg(averageHeatFlux, 0, 'e', 5);
    return QStringList(row);
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
    if (type == PHYSICFIELDBC_HEAT_TEMPERATURE)
    {
        return QString("addBoundary(\"%1\", \"%2\", %3);").
                arg(name).
                arg(physicFieldBCStringKey(type)).
                arg(temperature.text);
    }
    if (type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
    {
        return QString("addBoundary(\"%1\", \"%2\", %3, %4, %5)").
                arg(name).
                arg(physicFieldBCStringKey(type)).
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
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        out["Temperature (deg.)"] = temperature.text;
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
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

SceneLabelHeatMarker::SceneLabelHeatMarker(const QString &name, Value volume_heat, Value thermal_conductivity)
    : SceneLabelMarker(name)
{
    this->thermal_conductivity = thermal_conductivity;
    this->volume_heat = volume_heat;
}

QString SceneLabelHeatMarker::script()
{
    return QString("addMaterial(\"%1\", %2, %3);").
            arg(name).
            arg(volume_heat.text).
            arg(thermal_conductivity.text);
}

QMap<QString, QString> SceneLabelHeatMarker::data()
{
    QMap<QString, QString> out;
    out["Volume heat (W/m3)"] = volume_heat.text;
    out["Thermal conductivity (W/m.K)"] = thermal_conductivity.text;
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

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtTemperature);
    setTabOrder(txtTemperature, txtHeatFlux);
    setTabOrder(txtHeatFlux, txtHeatTransferCoefficient);
    setTabOrder(txtHeatTransferCoefficient, txtExternalTemperature);

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

QLayout* DSceneEdgeHeatMarker::createContent()
{
    cmbType = new QComboBox();
    cmbType->addItem("none", PHYSICFIELDBC_NONE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_HEAT_TEMPERATURE), PHYSICFIELDBC_HEAT_TEMPERATURE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_HEAT_HEAT_FLUX), PHYSICFIELDBC_HEAT_HEAT_FLUX);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtHeatFlux = new SLineEditValue(this);
    txtTemperature = new SLineEditValue(this);
    txtHeatTransferCoefficient = new SLineEditValue(this);
    txtExternalTemperature = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("BC Type:"), cmbType);
    layoutMarker->addRow(tr("Temperature:"), txtTemperature);
    layoutMarker->addRow(tr("Heat flux:"), txtHeatFlux);
    layoutMarker->addRow(tr("Heat transfer coef.:"), txtHeatTransferCoefficient);
    layoutMarker->addRow(tr("External temperature:"), txtExternalTemperature);

    return layoutMarker;
}

void DSceneEdgeHeatMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeHeatMarker->type));
    switch (edgeHeatMarker->type)
    {
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        {
            txtTemperature->setText(edgeHeatMarker->temperature.text);
        }
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        {
            txtHeatFlux->setText(edgeHeatMarker->heatFlux.text);
            txtHeatTransferCoefficient->setText(edgeHeatMarker->h.text);
            txtExternalTemperature->setText(edgeHeatMarker->externalTemperature.text);
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
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        {
            if (txtTemperature->evaluate())
                edgeHeatMarker->temperature  = txtTemperature->value();
            else
                return false;
        }
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
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
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        {
            txtTemperature->setEnabled(true);
        }
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
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

    // tab order
    setTabOrder(txtName, txtThermalConductivity);
    setTabOrder(txtThermalConductivity, txtVolumeHeat);

    load();
    setSize();
}

DSceneLabelHeatMarker::~DSceneLabelHeatMarker()
{
    delete txtThermalConductivity;
    delete txtVolumeHeat;
}

QLayout* DSceneLabelHeatMarker::createContent()
{
    txtThermalConductivity = new SLineEditValue(this);
    txtVolumeHeat = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("Thermal conductivity (W/m.K):"), txtThermalConductivity);
    layoutMarker->addRow(tr("Volume heat (J/m3):"), txtVolumeHeat);

    return layoutMarker;
}

void DSceneLabelHeatMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(m_labelMarker);

    txtThermalConductivity->setText(labelHeatMarker->thermal_conductivity.text);
    txtVolumeHeat->setText(labelHeatMarker->volume_heat.text);
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

    return true;
}
