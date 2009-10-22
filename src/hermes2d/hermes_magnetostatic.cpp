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
};

MagnetostaticEdge *magnetostaticEdge;
MagnetostaticLabel *magnetostaticLabel;
bool magnetostaticPlanar;

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
        return 1.0 / (magnetostaticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return 1.0 / (magnetostaticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v));
}

template<typename Real, typename Scalar>
Scalar magnetostatic_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magnetostaticPlanar)
        return magnetostaticLabel[e->marker].current_density * int_v<Real, Scalar>(n, wt, v)
                + magnetostaticLabel[e->marker].remanence / (magnetostaticLabel[e->marker].permeability * MU0)
                * (- sin(magnetostaticLabel[e->marker].remanence_angle / 180.0 * M_PI) * int_dvdx<Real, Scalar>(n, wt, v) +
                   + cos(magnetostaticLabel[e->marker].remanence_angle / 180.0 * M_PI) * int_dvdy<Real, Scalar>(n, wt, v));
    else
        return magnetostaticLabel[e->marker].current_density * int_v<Real, Scalar>(n, wt, v)
                + magnetostaticLabel[e->marker].remanence / (magnetostaticLabel[e->marker].permeability * MU0)
                * (- sin(magnetostaticLabel[e->marker].remanence_angle / 180.0 * M_PI) * int_dvdx<Real, Scalar>(n, wt, v) +
                   + cos(magnetostaticLabel[e->marker].remanence_angle / 180.0 * M_PI) * int_dvdy<Real, Scalar>(n, wt, v));
}

QList<SolutionArray *> *magnetostatic_main(SolverThread *solverThread)
{
    magnetostaticPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);
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
    space.set_bc_types(magnetostatic_bc_types);
    space.set_bc_values(magnetostatic_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs();

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(magnetostatic_bilinear_form));
    wf.add_liform(0, callback(magnetostatic_linear_form));

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

    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();
    solutionArrayList->append(solutionArray);

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
                                                                      Value(element->attribute("value"))));
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
                                                                    Value(element->attribute("current_density")),
                                                                    Value(element->attribute("permeability")),
                                                                    Value(element->attribute("remanence")),
                                                                    Value(element->attribute("remanence_angle"))));
}

void HermesMagnetostatic::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(marker);

    element->setAttribute("current_density", labelMagnetostaticMarker->current_density.text);
    element->setAttribute("permeability", labelMagnetostaticMarker->permeability.text);
    element->setAttribute("remanence", labelMagnetostaticMarker->remanence.text);
    element->setAttribute("remanence_angle", labelMagnetostaticMarker->remanence_angle.text);
}

LocalPointValue *HermesMagnetostatic::localPointValue(Point point)
{
    return new LocalPointValueMagnetostatic(point);
}

QStringList HermesMagnetostatic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "A" << "Bx" << "By" << "B" << "Hx" << "Hy" << "H" << "wm" << "mur" << "Br" << "Brangle";
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
    headers << "V" << "S" << "Bx_avg" << "By_avg" << "B_avg" << "Hx_avg" << "Hy_avg" << "H_avg" << "Wm";
    return QStringList(headers);
}

SceneEdgeMarker *HermesMagnetostatic::newEdgeMarker()
{
    return new SceneEdgeMagnetostaticMarker("new boundary", PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL, Value("0"));
}
/*
SceneEdgeMarker *HermesMagnetostatic::newEdgeMarker(const QString &name, PhysicFieldBC physicFieldBC[], Value *value[])
{
    return new SceneEdgeMagnetostaticMarker(name, physicFieldBC[0], *value[0]);
}
*/
SceneLabelMarker *HermesMagnetostatic::newLabelMarker()
{
    return new SceneLabelMagnetostaticMarker("new material", Value("0"), Value("1"), Value("0"), Value("0"));
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

    // Current Density
    addTreeWidgetItemValue(magnetostaticNode, tr("Current density:"), QString("%1").arg(localPointValueMagnetostatic->current_density, 0, 'e', 3), "A/m2");

    // Remanence
    addTreeWidgetItemValue(magnetostaticNode, tr("Rem. flux dens.:"), QString("%1").arg(localPointValueMagnetostatic->remanence, 0, 'e', 3), "T");
    addTreeWidgetItemValue(magnetostaticNode, tr("Direction of rem.:"), QString("%1").arg(localPointValueMagnetostatic->remanence_angle, 0, 'f', 2), "deg.");

    // Energy density
    addTreeWidgetItemValue(magnetostaticNode, tr("Energy density:"), QString("%1").arg(localPointValueMagnetostatic->wm, 0, 'e', 3), "J/m3");

    // Potential
    addTreeWidgetItemValue(magnetostaticNode, tr("Vector potential:"), QString("%1").arg(localPointValueMagnetostatic->potential, 0, 'e', 3), "Wb/m");

    // Flux Density
    QTreeWidgetItem *itemFluxDensity = new QTreeWidgetItem(magnetostaticNode);
    itemFluxDensity->setText(0, tr("Flux density"));
    itemFluxDensity->setExpanded(true);

    addTreeWidgetItemValue(itemFluxDensity, "B" + Util::scene()->problemInfo().labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->B.x, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensity, "B" + Util::scene()->problemInfo().labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->B.y, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensity, "B:", QString("%1").arg(localPointValueMagnetostatic->B.magnitude(), 0, 'e', 3), "T");

    // Magnetic Field
    QTreeWidgetItem *itemMagneticField = new QTreeWidgetItem(magnetostaticNode);
    itemMagneticField->setText(0, tr("Magnetic field"));
    itemMagneticField->setExpanded(true);

    addTreeWidgetItemValue(itemMagneticField, "H" + Util::scene()->problemInfo().labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->H.x, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticField, "H" + Util::scene()->problemInfo().labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetostatic->H.y, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticField, "H", QString("%1").arg(localPointValueMagnetostatic->H.magnitude(), 0, 'e', 3), "A/m");
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
            if (!edgeMagnetostaticMarker->value.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;

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
            if (!labelMagnetostaticMarker->current_density.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;
            if (!labelMagnetostaticMarker->permeability.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;
            if (!labelMagnetostaticMarker->remanence.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;
            if (!labelMagnetostaticMarker->remanence_angle.evaluate(Util::scene()->problemInfo().scriptStartup)) return NULL;

            magnetostaticLabel[i].current_density = labelMagnetostaticMarker->current_density.number;
            magnetostaticLabel[i].permeability = labelMagnetostaticMarker->permeability.number;
            magnetostaticLabel[i].remanence = labelMagnetostaticMarker->remanence.number;
            magnetostaticLabel[i].remanence_angle = labelMagnetostaticMarker->remanence_angle.number;
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

            if (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
            QString("%1").arg(remanence_angle, 0, 'e', 5);

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
            }
        }

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
            QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SceneEdgeMagnetostaticMarker::SceneEdgeMagnetostaticMarker(const QString &name, PhysicFieldBC type, Value value) : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeMagnetostaticMarker::script()
{
    return QString("addBoundary(\"%1\", \"%2\", %3);").
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

SceneLabelMagnetostaticMarker::SceneLabelMagnetostaticMarker(const QString &name, Value current_density, Value permeability, Value remanence, Value remanence_angle)
    : SceneLabelMarker(name)
{
    this->permeability = permeability;
    this->current_density = current_density;
    this->remanence = remanence;
    this->remanence_angle = remanence_angle;
}

QString SceneLabelMagnetostaticMarker::script()
{
    return QString("addMaterial(\"%1\", %2, %3, %4, %5);").
            arg(name).
            arg(current_density.text).
            arg(permeability.text).
            arg(remanence.text).
            arg(remanence_angle.text);
}

QMap<QString, QString> SceneLabelMagnetostaticMarker::data()
{
    QMap<QString, QString> out;
    out["Current density (A/m2)"] = current_density.text;
    out["Permeability (-)"] = permeability.text;
    out["Remanence (T)"] = remanence.text;
    out["Remanence angle (-)"] = remanence_angle.text;
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
    txtValue->setText(edgeMagnetostaticMarker->value.text);
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
    setTabOrder(txtCurrentDensity, txtRemanence);
    setTabOrder(txtRemanence, txtRemanenceAngle);

    load();
    setSize();
}

DSceneLabelMagnetostaticMarker::~DSceneLabelMagnetostaticMarker()
{
    delete txtPermeability;
    delete txtCurrentDensity;
    delete txtRemanence;
    delete txtRemanenceAngle;
}

QLayout* DSceneLabelMagnetostaticMarker::createContent()
{
    txtPermeability = new SLineEditValue(this);
    txtCurrentDensity = new SLineEditValue(this);
    txtRemanence = new SLineEditValue(this);
    txtRemanenceAngle = new SLineEditValue(this);

    /*
    QHBoxLayout *layoutRemanence = new QHBoxLayout();
    layoutRemanence->addWidget(txtRemanence);
    layoutRemanence->addWidget(new QLabel(tr("Angle (deg.):")));
    layoutRemanence->addWidget(txtRemanenceAngle);
    */

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("Permeability (-):"), txtPermeability);
    layoutMarker->addRow(tr("Current density (A/m2):"), txtCurrentDensity);
    // layoutMarker->addRow(tr("Remanence (T):"), layoutRemanence);
    layoutMarker->addRow(tr("Rem. flux dens. (T):"), txtRemanence);
    layoutMarker->addRow(tr("Direction of rem. (deg.):"), txtRemanenceAngle);

    return layoutMarker;
}

void DSceneLabelMagnetostaticMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_labelMarker);

    txtPermeability->setText(labelMagnetostaticMarker->permeability.text);
    txtCurrentDensity->setText(labelMagnetostaticMarker->current_density.text);
    txtRemanence->setText(labelMagnetostaticMarker->remanence.text);
    txtRemanenceAngle->setText(labelMagnetostaticMarker->remanence_angle.text);
}

bool DSceneLabelMagnetostaticMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_labelMarker);

    if (txtPermeability->evaluate())
        labelMagnetostaticMarker->permeability  = txtPermeability->value();
    else
        return false;

    if (txtCurrentDensity->evaluate())
        labelMagnetostaticMarker->current_density  = txtCurrentDensity->value();
    else
        return false;

    if (txtRemanence->evaluate())
        labelMagnetostaticMarker->remanence  = txtRemanence->value();
    else
        return false;

    if (txtRemanenceAngle->evaluate())
        labelMagnetostaticMarker->remanence_angle  = txtRemanenceAngle->value();
    else
        return false;

    return true;
}
