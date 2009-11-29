#include "hermes_magnetostatic.h"
#include "scene.h"

struct MagnetostaticEdge
{
    PhysicFieldBC type;
    double value;
};

struct MagnetostaticLabel
{
    double current_density;
    double permeability;
    double remanence;
    double remanence_angle;
    double conductivity;
    double velocity_x;
    double velocity_y;
    double velocity_angular;
};

MagnetostaticEdge *magnetostaticEdge;
MagnetostaticLabel *magnetostaticLabel;
bool magnetostaticPlanar;
bool magnetostaticTransient;
double magnetostaticTimeStep;
double magnetostaticTimeTotal;

int magnetostatic_bc_types(int marker)
{
    switch (magnetostaticEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT:
        return BC_NATURAL;
        break;
    }
}

scalar magnetostatic_bc_values(int marker, double x, double y)
{
    return magnetostaticEdge[marker].value;
}

template<typename Real, typename Scalar>
Scalar magnetostatic_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magnetostaticPlanar)
        return 1.0 / (magnetostaticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v) -
                magnetostaticLabel[e->marker].conductivity * int_velocity<Real, Scalar>(n, wt, u, v, e, magnetostaticLabel[e->marker].velocity_x, magnetostaticLabel[e->marker].velocity_y, magnetostaticLabel[e->marker].velocity_angular) +
                ((magnetostaticTransient) ? magnetostaticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v) / magnetostaticTimeStep : 0.0);
    else
        return 1.0 / (magnetostaticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)) -
                magnetostaticLabel[e->marker].conductivity * int_velocity<Real, Scalar>(n, wt, u, v, e, magnetostaticLabel[e->marker].velocity_x, magnetostaticLabel[e->marker].velocity_y, magnetostaticLabel[e->marker].velocity_angular) +
                ((magnetostaticTransient) ? magnetostaticLabel[e->marker].conductivity * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e) / magnetostaticTimeStep : 0.0);
}

template<typename Real, typename Scalar>
Scalar magnetostatic_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magnetostaticPlanar)
        return magnetostaticLabel[e->marker].current_density * int_v<Real, Scalar>(n, wt, v) +
                magnetostaticLabel[e->marker].remanence / (magnetostaticLabel[e->marker].permeability * MU0) * int_magnet<Real, Scalar>(n, wt, v, magnetostaticLabel[e->marker].remanence_angle) +
                ((magnetostaticTransient) ? magnetostaticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, ext->fn[0], v) / magnetostaticTimeStep : 0.0);
    else
        return magnetostaticLabel[e->marker].current_density * int_v<Real, Scalar>(n, wt, v) +
                magnetostaticLabel[e->marker].remanence / (magnetostaticLabel[e->marker].permeability * MU0) * int_magnet<Real, Scalar>(n, wt, v, magnetostaticLabel[e->marker].remanence_angle) +
                ((magnetostaticTransient) ? magnetostaticLabel[e->marker].conductivity * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, ext->fn[0], v, e) / magnetostaticTimeStep : 0.0);
}

QList<SolutionArray *> *magnetostatic_main(SolverThread *solverThread)
{
    magnetostaticPlanar = (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
    magnetostaticTransient = (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT);
    magnetostaticTimeStep = Util::scene()->problemInfo()->timeStep;
    magnetostaticTimeTotal = Util::scene()->problemInfo()->timeTotal;
    int numberOfRefinements = Util::scene()->problemInfo()->numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;

    QTime time;

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
    space.set_bc_types(magnetostatic_bc_types);
    space.set_bc_values(magnetostatic_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs();

    // solution
    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();

    Solution *sln = new Solution();
    if (magnetostaticTransient)
    {
        sln->set_const(&mesh, 20);

        // zero time
        /*
        SolutionArray *solutionArray = new SolutionArray();
        solutionArray->order1 = new Orderizer();
        solutionArray->order1->process_solution(&space);
        solutionArray->sln1 = new Solution();
        solutionArray->sln1->copy(sln);
        solutionArray->adaptiveError = 0.0;
        solutionArray->adaptiveSteps = 0;
        solutionArray->time = 0.0;

        solutionArrayList->append(solutionArray);
        */
    }
    Solution rsln;

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(magnetostatic_bilinear_form));
    if (magnetostaticTransient)
        wf.add_liform(0, callback(magnetostatic_linear_form), ANY, 1, sln);
    else
        wf.add_liform(0, callback(magnetostatic_linear_form));

    // initialize the linear solver
    UmfpackSolver umfpack;

    // initialize the linear system
    LinSystem sys(&wf, &umfpack);
    sys.set_spaces(1, &space);
    sys.set_pss(1, &pss);

    // assemble the stiffness matrix and solve the system
    int i;
    double error;

    // adaptivity
    int adaptivitysteps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps;
    for (i = 0; i<adaptivitysteps; i++)
    {
        space.assign_dofs();

        time.start();
        sys.assemble();
        log(QString("sys.assemble(): %1").arg(time.elapsed()));
        time.start();
        sys.solve(1, sln);
        log(QString("sys.solve(): %1").arg(time.elapsed()));

        // calculate errors and adapt the solution
        if (adaptivityType != ADAPTIVITYTYPE_NONE)
        {
            RefSystem rs(&sys);
            time.start();
            rs.assemble();
            log(QString("rs.assemble(): %1").arg(time.elapsed()));
            time.start();
            rs.solve(1, &rsln);
            log(QString("rs.solve(): %1").arg(time.elapsed()));

            H1OrthoHP hp(1, &space);
            error = hp.calc_error(sln, &rsln) * 100;

            // emit signal
            solverThread->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
            if (solverThread->isCanceled()) return NULL;

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            if (i != adaptivitysteps-1) hp.adapt(0.3, 0, (int) adaptivityType);
        }
    }

    // timesteps
    int timesteps = (magnetostaticTransient) ? floor(magnetostaticTimeTotal/magnetostaticTimeStep) : 1;
    for (int n = 0; n<timesteps; n++)
    {
        log("\n");
        if (timesteps > 1)
        {
            sys.assemble(true);
            sys.solve(1, sln);
        }
        else
        {
            space.assign_dofs();
            sys.assemble();
        }

        // output
        SolutionArray *solutionArray = new SolutionArray();
        solutionArray->order = new Orderizer();
        solutionArray->order->process_solution(&space);
        solutionArray->sln = new Solution();
        solutionArray->sln->copy(sln);
        solutionArray->adaptiveError = error;
        solutionArray->adaptiveSteps = i-1;
        if (magnetostaticTransient > 0) solutionArray->time = (n+1)*magnetostaticTimeStep;

        solutionArrayList->append(solutionArray);

        if (magnetostaticTransient > 0) solverThread->showMessage(QObject::tr("Solver: time step: %1/%2").arg(n+1).arg(timesteps), false);
        if (solverThread->isCanceled()) return NULL;
        solverThread->showProgress((int) (60.0 + 40.0*(n+1)/timesteps));
    }

    return solutionArrayList;
}

// *******************************************************************************************************

void HermesMagnetostatic::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL:
    case PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT:
        Util::scene()->addEdgeMarker(new SceneEdgeMagnetostaticMarker(element->attribute("name"),
                                                                      type,
                                                                      Value(element->attribute("value", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesMagnetostatic::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker = dynamic_cast<SceneEdgeMagnetostaticMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeMagnetostaticMarker->type));
    element->setAttribute("value", edgeMagnetostaticMarker->value.text);
}

void HermesMagnetostatic::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelMagnetostaticMarker(element->attribute("name"),
                                                                    Value(element->attribute("current_density", "0")),
                                                                    Value(element->attribute("permeability", "1")),
                                                                    Value(element->attribute("remanence", "0")),
                                                                    Value(element->attribute("remanence_angle", "0")),
                                                                    Value(element->attribute("conductivity", "0")),
                                                                    Value(element->attribute("velocity_x", "0")),
                                                                    Value(element->attribute("velocity_y", "0")),
                                                                    Value(element->attribute("velocity_angular", "0"))));
}

void HermesMagnetostatic::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(marker);

    element->setAttribute("current_density", labelMagnetostaticMarker->current_density.text);
    element->setAttribute("permeability", labelMagnetostaticMarker->permeability.text);
    element->setAttribute("remanence", labelMagnetostaticMarker->remanence.text);
    element->setAttribute("remanence_angle", labelMagnetostaticMarker->remanence_angle.text);
    element->setAttribute("conductivity", labelMagnetostaticMarker->conductivity.text);
    element->setAttribute("velocity_x", labelMagnetostaticMarker->velocity_x.text);
    element->setAttribute("velocity_y", labelMagnetostaticMarker->velocity_y.text);
    element->setAttribute("velocity_angular", labelMagnetostaticMarker->velocity_angular.text);
}

LocalPointValue *HermesMagnetostatic::localPointValue(Point point)
{
    return new LocalPointValueMagnetostatic(point);
}

QStringList HermesMagnetostatic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "A" << "Bx" << "By" << "B" << "Hx" << "Hy" << "H" << "wm" << "mur" << "Br" << "Brangle" << "vx" << "vy" << "Jext" << "Jvel" << "Jtot" << "pj";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesMagnetostatic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueMagnetostatic();
}

QStringList HermesMagnetostatic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesMagnetostatic::volumeIntegralValue()
{
    return new VolumeIntegralValueMagnetostatic();
}

QStringList HermesMagnetostatic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "Bx_avg" << "By_avg" << "B_avg" << "Hx_avg" << "Hy_avg" << "H_avg" << "Wm" << "Fx" << "Fy" << "Iext" << "Ivel" << "Itot" << "Pj" << "M";
    return QStringList(headers);
}

SceneEdgeMarker *HermesMagnetostatic::newEdgeMarker()
{
    return new SceneEdgeMagnetostaticMarker("new boundary",
                                            PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL,
                                            Value("0"));
}

SceneEdgeMarker *HermesMagnetostatic::newEdgeMarker(PyObject *self, PyObject *args)
{    
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeMagnetostaticMarker(name,
                                                physicFieldBCFromStringKey(type),
                                                Value(QString::number(value)));
    }

    return NULL;
}

SceneLabelMarker *HermesMagnetostatic::newLabelMarker()
{
    return new SceneLabelMagnetostaticMarker("new material",
                                             Value("0"),
                                             Value("1"),
                                             Value("0"),
                                             Value("0"),
                                             Value("0"),
                                             Value("0"),
                                             Value("0"),
                                             Value("0"));
}

SceneLabelMarker *HermesMagnetostatic::newLabelMarker(PyObject *self, PyObject *args)
{
    double current_density, permeability, remanence, remanence_angle, conductivity, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sdddddddd", &name, &current_density, &permeability, &remanence, &remanence_angle, &conductivity, &velocity_x, &velocity_y, &velocity_angular))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelMagnetostaticMarker(name,
                                                 Value(QString::number(current_density)),
                                                 Value(QString::number(permeability)),
                                                 Value(QString::number(remanence)),
                                                 Value(QString::number(remanence_angle)),
                                                 Value(QString::number(conductivity)),
                                                 Value(QString::number(velocity_x)),
                                                 Value(QString::number(velocity_y)),
                                                 Value(QString::number(velocity_angular)));
    }

    return NULL;
}

void HermesMagnetostatic::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueMagnetostatic *localPointValueMagnetostatic = dynamic_cast<LocalPointValueMagnetostatic *>(localPointValue);

    // magnetostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic field"));
    magnetostaticNode->setExpanded(true);

    // Permeability
    addTreeWidgetItemValue(magnetostaticNode, tr("Permeability:"), QString("%1").arg(localPointValueMagnetostatic->permeability, 0, 'f', 2), "");

    // Conductivity
    addTreeWidgetItemValue(magnetostaticNode, tr("Conductivity:"), QString("%1").arg(localPointValueMagnetostatic->conductivity, 0, 'g', 3), "");

    // Current Density
    addTreeWidgetItemValue(magnetostaticNode, tr("Ext. current dens.:"), QString("%1").arg(localPointValueMagnetostatic->current_density, 0, 'e', 3), "A/m2");

    // Remanence
    addTreeWidgetItemValue(magnetostaticNode, tr("Rem. flux dens.:"), QString("%1").arg(localPointValueMagnetostatic->remanence, 0, 'e', 3), "T");
    addTreeWidgetItemValue(magnetostaticNode, tr("Direction of rem.:"), QString("%1").arg(localPointValueMagnetostatic->remanence_angle, 0, 'f', 2), "deg.");

    // Velocity
    QTreeWidgetItem *itemVelocity = new QTreeWidgetItem(magnetostaticNode);
    itemVelocity->setText(0, tr("Velocity"));
    itemVelocity->setExpanded(true);

    addTreeWidgetItemValue(itemVelocity, tr("Velocity ") + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->velocity.x, 0, 'e', 3), "m/s");
    addTreeWidgetItemValue(itemVelocity, tr("Velocity ") + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->velocity.y, 0, 'e', 3), "m/s");

    // Energy density
    addTreeWidgetItemValue(magnetostaticNode, tr("Energy density:"), QString("%1").arg(localPointValueMagnetostatic->wm, 0, 'e', 3), "J/m3");

    // Potential
    addTreeWidgetItemValue(magnetostaticNode, tr("Vector potential:"), QString("%1").arg(localPointValueMagnetostatic->potential, 0, 'e', 3), "Wb/m");

    // Flux Density
    QTreeWidgetItem *itemFluxDensity = new QTreeWidgetItem(magnetostaticNode);
    itemFluxDensity->setText(0, tr("Flux density"));
    itemFluxDensity->setExpanded(true);

    addTreeWidgetItemValue(itemFluxDensity, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->B.x, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensity, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->B.y, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensity, "B:", QString("%1").arg(localPointValueMagnetostatic->B.magnitude(), 0, 'e', 3), "T");

    // Magnetic Field
    QTreeWidgetItem *itemMagneticField = new QTreeWidgetItem(magnetostaticNode);
    itemMagneticField->setText(0, tr("Magnetic field"));
    itemMagneticField->setExpanded(true);

    addTreeWidgetItemValue(itemMagneticField, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->H.x, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticField, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->H.y, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticField, "H", QString("%1").arg(localPointValueMagnetostatic->H.magnitude(), 0, 'e', 3), "A/m");

    // Induced current density
    QTreeWidgetItem *itemInducedCurrentDensity = new QTreeWidgetItem(magnetostaticNode);
    itemInducedCurrentDensity->setText(0, tr("Induced current density"));
    itemInducedCurrentDensity->setExpanded(true);

    addTreeWidgetItemValue(itemInducedCurrentDensity, tr("Vel. current dens.:"), QString("%1").arg(localPointValueMagnetostatic->current_density_velocity, 0, 'e', 3), "m/s");
    addTreeWidgetItemValue(itemInducedCurrentDensity, tr("Total current dens.:"), QString("%1").arg(localPointValueMagnetostatic->current_density + localPointValueMagnetostatic->current_density_velocity, 0, 'e', 3), "m/s");

    // losses
    addTreeWidgetItemValue(magnetostaticNode, tr("Power losses:"), QString("%1").arg(localPointValueMagnetostatic->pj, 0, 'e', 3), "W/m3");
}

void HermesMagnetostatic::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueMagnetostatic *surfaceIntegralValueMagnetostatic = dynamic_cast<SurfaceIntegralValueMagnetostatic *>(surfaceIntegralValue);

    // magnetostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic Field"));
    magnetostaticNode->setExpanded(true);
}

void HermesMagnetostatic::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueMagnetostatic *volumeIntegralValueMagnetostatic = dynamic_cast<VolumeIntegralValueMagnetostatic *>(volumeIntegralValue);

    // magnetostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic field"));
    magnetostaticNode->setExpanded(true);

    addTreeWidgetItemValue(magnetostaticNode, tr("Hx avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageMagneticFieldX, 0, 'e', 3), tr("A/m"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Hy avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageMagneticFieldY, 0, 'e', 3), tr("A/m"));
    addTreeWidgetItemValue(magnetostaticNode, tr("H: avg."), tr("%1").arg(volumeIntegralValueMagnetostatic->averageMagneticField, 0, 'e', 3), tr("A/m"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Bx avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageFluxDensityX, 0, 'e', 3), tr("T"));
    addTreeWidgetItemValue(magnetostaticNode, tr("By avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageFluxDensityY, 0, 'e', 3), tr("T"));
    addTreeWidgetItemValue(magnetostaticNode, tr("B avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageFluxDensity, 0, 'e', 3), tr("T"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Energy:"), tr("%1").arg(volumeIntegralValueMagnetostatic->energy, 0, 'e', 3), tr("J"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Fx:"), tr("%1").arg(volumeIntegralValueMagnetostatic->forceX, 0, 'e', 3), tr("N"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Fy:"), tr("%1").arg(volumeIntegralValueMagnetostatic->forceY, 0, 'e', 3), tr("N"));
    addTreeWidgetItemValue(magnetostaticNode, tr("External current:"), tr("%1").arg(volumeIntegralValueMagnetostatic->currentExternal, 0, 'e', 3), tr("A"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Velocity current:"), tr("%1").arg(volumeIntegralValueMagnetostatic->currentVelocity, 0, 'e', 3), tr("A"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Total current:"), tr("%1").arg(volumeIntegralValueMagnetostatic->currentTotal, 0, 'e', 3), tr("A"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Losses:"), tr("%1").arg(volumeIntegralValueMagnetostatic->losses, 0, 'e', 3), tr("A/m2"));
    addTreeWidgetItemValue(magnetostaticNode, tr("Torque:"), tr("%1").arg(volumeIntegralValueMagnetostatic->torque, 0, 'e', 3), tr("Nm"));
}

QList<SolutionArray *> *HermesMagnetostatic::solve(SolverThread *solverThread)
{
    // edge markers
    magnetostaticEdge = new MagnetostaticEdge[Util::scene()->edges.count()+1];
    magnetostaticEdge[0].type = PHYSICFIELDBC_NONE;
    magnetostaticEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            magnetostaticEdge[i+1].type = PHYSICFIELDBC_NONE;
            magnetostaticEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker = dynamic_cast<SceneEdgeMagnetostaticMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeMagnetostaticMarker->value.evaluate()) return NULL;

            magnetostaticEdge[i+1].type = edgeMagnetostaticMarker->type;
            magnetostaticEdge[i+1].value = edgeMagnetostaticMarker->value.number;
        }
    }

    // label markers
    magnetostaticLabel = new MagnetostaticLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelMagnetostaticMarker->current_density.evaluate()) return NULL;
            if (!labelMagnetostaticMarker->permeability.evaluate()) return NULL;
            if (!labelMagnetostaticMarker->remanence.evaluate()) return NULL;
            if (!labelMagnetostaticMarker->remanence_angle.evaluate()) return NULL;
            if (!labelMagnetostaticMarker->conductivity.evaluate()) return NULL;
            if (!labelMagnetostaticMarker->velocity_x.evaluate()) return NULL;
            if (!labelMagnetostaticMarker->velocity_y.evaluate()) return NULL;
            if (!labelMagnetostaticMarker->velocity_angular.evaluate()) return NULL;

            magnetostaticLabel[i].current_density = labelMagnetostaticMarker->current_density.number;
            magnetostaticLabel[i].permeability = labelMagnetostaticMarker->permeability.number;
            magnetostaticLabel[i].remanence = labelMagnetostaticMarker->remanence.number;
            magnetostaticLabel[i].remanence_angle = labelMagnetostaticMarker->remanence_angle.number;
            magnetostaticLabel[i].conductivity = labelMagnetostaticMarker->conductivity.number;
            magnetostaticLabel[i].velocity_x = labelMagnetostaticMarker->velocity_x.number;
            magnetostaticLabel[i].velocity_y = labelMagnetostaticMarker->velocity_y.number;
            magnetostaticLabel[i].velocity_angular = labelMagnetostaticMarker->velocity_angular.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = magnetostatic_main(solverThread);

    delete [] magnetostaticEdge;
    delete [] magnetostaticLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueMagnetostatic::LocalPointValueMagnetostatic(Point &point) : LocalPointValue(point)
{
    current_density = 0;
    permeability = 0;
    remanence = 0;
    remanence_angle = 0;
    conductivity = 0;

    velocity = Point();
    current_density_velocity = 0;
    pj = 0;

    potential = 0;
    H = Point();
    B = Point();
    wm = 0;

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
        if (value.marker != NULL)
        {
            // potential
            potential = value.value;

            // magnetic field
            Point der;
            der = value.derivative;

            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
            {
                B.x =  der.y;
                B.y = -der.x;
            }
            else
            {
                B.x = -der.y;
                B.y =  der.x + ((point.x > 0.0) ? potential/point.x : 0.0);
            }

            SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(value.marker);

            current_density = marker->current_density.number;
            permeability = marker->permeability.number;
            remanence = marker->remanence.number;
            remanence_angle = marker->remanence_angle.number;
            conductivity = marker->conductivity.number;

            velocity = Point(marker->velocity_x.number - marker->velocity_angular.number * point.y, marker->velocity_y.number + marker->velocity_angular.number * point.x);

            // current density
            current_density_velocity = - conductivity * (velocity.x * der.x + velocity.y * der.y);

            // pj
            pj = (conductivity > 0 ) ? sqr(current_density + current_density_velocity) / conductivity : 0.0;

            // electric displacement
            H = B / (marker->permeability.number * MU0);

            // energy density
            wm = 0.5 * H.magnitude() * B.magnitude();
        }
    }
}

double LocalPointValueMagnetostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL:
        {
            return potential;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return B.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return B.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return B.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return H.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return H.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return H.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY:
        {
            return wm;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY:
        {
            return permeability;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE:
        {
            return remanence;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE_ANGLE:
        {
            return remanence_angle;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_CONDUCTIVITY:
        {
            return conductivity;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VELOCITY:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return velocity.x * point.y;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return velocity.y * point.x;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return sqrt(sqr(velocity.x * point.y) + sqr(velocity.y * point.x));
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY:
        {
            return current_density;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY_VELOCITY:
        {
            return current_density_velocity;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY_TOTAL:
        {
            return current_density + current_density_velocity;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_POWER_LOSSES:
        {
            return (conductivity > 0) ? sqr(current_density + current_density_velocity) / conductivity : 0.0;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueMagnetostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueMagnetostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential, 0, 'e', 5) <<
            QString("%1").arg(B.x, 0, 'e', 5) <<
            QString("%1").arg(B.y, 0, 'e', 5) <<
            QString("%1").arg(B.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(H.x, 0, 'e', 5) <<
            QString("%1").arg(H.y, 0, 'e', 5) <<
            QString("%1").arg(H.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(wm, 0, 'e', 5) <<
            QString("%1").arg(permeability, 0, 'f', 3) <<
            QString("%1").arg(remanence, 0, 'e', 5) <<
            QString("%1").arg(remanence_angle, 0, 'e', 5) <<
            QString("%1").arg(velocity.x, 0, 'e', 5) <<
            QString("%1").arg(velocity.y, 0, 'e', 5) <<
            QString("%1").arg(current_density, 0, 'e', 5) <<
            QString("%1").arg(current_density_velocity, 0, 'e', 5) <<
            QString("%1").arg(current_density + current_density_velocity, 0, 'e', 5) <<
            QString("%1").arg(pj, 0, 'e', 5);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueMagnetostatic::SurfaceIntegralValueMagnetostatic() : SurfaceIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {

    }
}

QStringList SurfaceIntegralValueMagnetostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueMagnetostatic::VolumeIntegralValueMagnetostatic() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        averageMagneticFieldX = 0;
        averageMagneticFieldY = 0;
        averageMagneticField = 0;
        averageFluxDensityX = 0;
        averageFluxDensityY = 0;
        averageFluxDensity = 0;
        energy = 0;
        forceX = 0;
        forceY = 0;
        currentExternal = 0;
        currentVelocity = 0;
        currentTotal = 0;
        losses = 0;
        torque = 0;

        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                averageMagneticFieldX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_X);
                averageMagneticFieldY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_Y);
                averageMagneticField += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD);
                averageFluxDensityX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_X);
                averageFluxDensityY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_Y);
                averageFluxDensity += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY);
                energy += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_ENERGY_DENSITY);
                forceX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FORCE_X);
                forceY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FORCE_Y);
                currentExternal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_CURRENT_DENSITY);
                currentVelocity += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_CURRENT_DENSITY_VELOCITY);
                losses += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_POWER_LOSSES);
                torque += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_TORQUE);
            }
        }

        currentTotal = currentExternal + currentVelocity;
        if (volume > 0)
        {
            averageMagneticFieldX /= volume;
            averageMagneticFieldY /= volume;
            averageMagneticField /= volume;
            averageFluxDensityX /= volume;
            averageFluxDensityY /= volume;
            averageFluxDensity /= volume;
        }
    }
}

QStringList VolumeIntegralValueMagnetostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(averageMagneticFieldX, 0, 'e', 5) <<
            QString("%1").arg(averageMagneticFieldY, 0, 'e', 5) <<
            QString("%1").arg(averageMagneticField, 0, 'e', 5) <<
            QString("%1").arg(averageFluxDensityX, 0, 'e', 5) <<
            QString("%1").arg(averageFluxDensityY, 0, 'e', 5) <<
            QString("%1").arg(averageFluxDensity, 0, 'e', 5) <<
            QString("%1").arg(energy, 0, 'e', 5) <<
            QString("%1").arg(forceX, 0, 'e', 5) <<
            QString("%1").arg(forceY, 0, 'e', 5) <<
            QString("%1").arg(currentExternal, 0, 'e', 5) <<
            QString("%1").arg(currentVelocity, 0, 'e', 5) <<
            QString("%1").arg(currentTotal, 0, 'e', 5) <<
            QString("%1").arg(losses, 0, 'e', 5) <<
            QString("%1").arg(torque, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SceneEdgeMagnetostaticMarker::SceneEdgeMagnetostaticMarker(const QString &name, PhysicFieldBC type, Value value) : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeMagnetostaticMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneEdgeMagnetostaticMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL:
        out["Vector potential (Wb/m)"] = value.text;
        break;
    case PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT:
        out["Surface current (A/m2)"] = value.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeMagnetostaticMarker::showDialog(QWidget *parent)
{
    DSceneEdgeMagnetostaticMarker *dialog = new DSceneEdgeMagnetostaticMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelMagnetostaticMarker::SceneLabelMagnetostaticMarker(const QString &name, Value current_density, Value permeability, Value remanence, Value remanence_angle,
                                                            Value conductivity, Value velocity_x, Value velocity_y, Value velocity_angular)
    : SceneLabelMarker(name)
{
    this->permeability = permeability;
    this->current_density = current_density;
    this->remanence = remanence;
    this->remanence_angle = remanence_angle;
    this->conductivity = conductivity;
    this->velocity_x = velocity_x;
    this->velocity_y = velocity_y;
    this->velocity_angular = velocity_angular;
}

QString SceneLabelMagnetostaticMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4, %5, %6, %7, %8, %9)").
            arg(name).
            arg(current_density.text).
            arg(permeability.text).
            arg(remanence.text).
            arg(remanence_angle.text).
            arg(conductivity.text).
            arg(velocity_x.text).
            arg(velocity_y.text).
            arg(velocity_angular.text);
}

QMap<QString, QString> SceneLabelMagnetostaticMarker::data()
{
    QMap<QString, QString> out;
    out["Current density (A/m2)"] = current_density.text;
    out["Permeability (-)"] = permeability.text;
    out["Remanence (T)"] = remanence.text;
    out["Remanence angle (-)"] = remanence_angle.text;
    out["Conductivity (S/m)"] = conductivity.text;
    out["Velocity x (m/s)"] = conductivity.text;
    out["Velocity y (m/s)"] = conductivity.text;
    out["Angular velocity (m/s)"] = velocity_angular.text;
    return QMap<QString, QString>(out);
}

int SceneLabelMagnetostaticMarker::showDialog(QWidget *parent)
{
    DSceneLabelMagnetostaticMarker *dialog = new DSceneLabelMagnetostaticMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeMagnetostaticMarker::DSceneEdgeMagnetostaticMarker(SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeMagnetostaticMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtValue);

    load();
    setSize();
}

DSceneEdgeMagnetostaticMarker::~DSceneEdgeMagnetostaticMarker()
{
    delete cmbType;
    delete txtValue;
}

QLayout* DSceneEdgeMagnetostaticMarker::createContent()
{
    cmbType = new QComboBox();
    cmbType->addItem("none", PHYSICFIELDBC_NONE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL), PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT), PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT);

    txtValue = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("BC type:"), cmbType);
    layoutMarker->addRow(tr("Value:"), txtValue);

    return layoutMarker;
}

void DSceneEdgeMagnetostaticMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker = dynamic_cast<SceneEdgeMagnetostaticMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeMagnetostaticMarker->type));
    txtValue->setValue(edgeMagnetostaticMarker->value);
}

bool DSceneEdgeMagnetostaticMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker = dynamic_cast<SceneEdgeMagnetostaticMarker *>(m_edgeMarker);

    edgeMagnetostaticMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeMagnetostaticMarker->value  = txtValue->value();
    else
        return false;

    return true;
}

// *************************************************************************************************************************************

DSceneLabelMagnetostaticMarker::DSceneLabelMagnetostaticMarker(QWidget *parent, SceneLabelMagnetostaticMarker *labelMagnetostaticMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelMagnetostaticMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, txtPermeability);
    setTabOrder(txtPermeability, txtCurrentDensity);
    setTabOrder(txtCurrentDensity, txtConductivity);
    setTabOrder(txtConductivity, txtRemanence);
    setTabOrder(txtRemanence, txtRemanenceAngle);
    setTabOrder(txtRemanenceAngle, txtVelocityX);
    setTabOrder(txtVelocityX, txtVelocityY);
    setTabOrder(txtVelocityY, txtVelocityAngular);

    load();
    setSize();
}

DSceneLabelMagnetostaticMarker::~DSceneLabelMagnetostaticMarker()
{
    delete txtPermeability;
    delete txtCurrentDensity;
    delete txtRemanence;
    delete txtRemanenceAngle;
    delete txtConductivity;
    delete txtVelocityX;
    delete txtVelocityY;
    delete txtVelocityAngular;
}

QLayout* DSceneLabelMagnetostaticMarker::createContent()
{
    txtPermeability = new SLineEditValue(this);
    txtCurrentDensity = new SLineEditValue(this);
    txtRemanence = new SLineEditValue(this);
    txtRemanenceAngle = new SLineEditValue(this);
    txtConductivity = new SLineEditValue(this);
    txtVelocityX = new SLineEditValue(this);
    txtVelocityX->setEnabled(Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
    txtVelocityY = new SLineEditValue(this);
    txtVelocityAngular = new SLineEditValue(this);
    txtVelocityAngular->setEnabled(Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);

    // remanence
    QFormLayout *layoutRemanence = new QFormLayout();
    layoutRemanence->addRow(tr("Rem. flux dens. (T):"), txtRemanence);
    layoutRemanence->addRow(tr("Direction of rem. (deg.):"), txtRemanenceAngle);

    QGroupBox *grpRemanence = new QGroupBox(tr("Permanent magnet"), this);
    grpRemanence->setLayout(layoutRemanence);

    // velocity
    QFormLayout *layoutVelocity = new QFormLayout();
    layoutVelocity->addRow(tr("Velocity %1 (m/s):").arg(Util::scene()->problemInfo()->labelX().toLower()), txtVelocityX);
    layoutVelocity->addRow(tr("Velocity %1 (m/s):").arg(Util::scene()->problemInfo()->labelY().toLower()), txtVelocityY);
    layoutVelocity->addRow(tr("Velocity angular (rad/s):"), txtVelocityAngular);

    QGroupBox *grpVelocity = new QGroupBox(tr("Velocity"), this);
    grpVelocity->setLayout(layoutVelocity);

    QFormLayout *layoutMarkerVariables = new QFormLayout();
    layoutMarkerVariables->addRow(tr("Permeability (-):"), txtPermeability);
    layoutMarkerVariables->addRow(tr("Current density (A/m2):"), txtCurrentDensity);
    layoutMarkerVariables->addRow(tr("Conductivity (S/m):"), txtConductivity);

    QVBoxLayout *layoutMarker = new QVBoxLayout();
    layoutMarker->addLayout(layoutMarkerVariables);
    layoutMarker->addWidget(grpRemanence);
    layoutMarker->addWidget(grpVelocity);

    return layoutMarker;
}

void DSceneLabelMagnetostaticMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_labelMarker);

    txtPermeability->setValue(labelMagnetostaticMarker->permeability);
    txtCurrentDensity->setValue(labelMagnetostaticMarker->current_density);
    txtRemanence->setValue(labelMagnetostaticMarker->remanence);
    txtRemanenceAngle->setValue(labelMagnetostaticMarker->remanence_angle);
    txtConductivity->setValue(labelMagnetostaticMarker->conductivity);
    txtVelocityX->setValue(labelMagnetostaticMarker->velocity_x);
    txtVelocityY->setValue(labelMagnetostaticMarker->velocity_y);
    txtVelocityAngular->setValue(labelMagnetostaticMarker->velocity_angular);
}

bool DSceneLabelMagnetostaticMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_labelMarker);

    if (txtPermeability->evaluate())
        labelMagnetostaticMarker->permeability = txtPermeability->value();
    else
        return false;

    if (txtCurrentDensity->evaluate())
        labelMagnetostaticMarker->current_density = txtCurrentDensity->value();
    else
        return false;

    if (txtRemanence->evaluate())
        labelMagnetostaticMarker->remanence = txtRemanence->value();
    else
        return false;

    if (txtRemanenceAngle->evaluate())
        labelMagnetostaticMarker->remanence_angle = txtRemanenceAngle->value();
    else
        return false;

    if (txtConductivity->evaluate())
        labelMagnetostaticMarker->conductivity = txtConductivity->value();
    else
        return false;

    if (txtVelocityX->evaluate())
        labelMagnetostaticMarker->velocity_x = txtVelocityX->value();
    else
        return false;

    if (txtVelocityY->evaluate())
        labelMagnetostaticMarker->velocity_y = txtVelocityY->value();
    else
        return false;

    if (txtConductivity->evaluate())
        labelMagnetostaticMarker->velocity_angular = txtVelocityAngular->value();
    else
        return false;

    return true;
}
