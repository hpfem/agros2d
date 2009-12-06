#include "hermes_magnetic.h"
#include "scene.h"

struct MagneticEdge
{
    PhysicFieldBC type;
    double value;
};

struct MagneticLabel
{
    double current_density_real;
    double current_density_imag;
    double permeability;
    double conductivity;
    double remanence;
    double remanence_angle;
    double velocity_x;
    double velocity_y;
    double velocity_angular;
};

MagneticEdge *magneticEdge;
MagneticLabel *magneticLabel;
bool magneticPlanar;
AnalysisType magneticAnalysisType;

double magneticInitialCondition;
double magneticTimeStep;
double magneticTimeTotal;
double magneticFrequency;

int magnetic_bc_types(int marker)
{
    switch (magneticEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT:
        return BC_NATURAL;
        break;
    }
}

scalar magnetic_bc_values_real(int marker, double x, double y)
{
    return magneticEdge[marker].value;
}

scalar magnetic_bc_values_imag(int marker, double x, double y)
{
    return magneticEdge[marker].value;
}

template<typename Real, typename Scalar>
Scalar magnetic_bilinear_form_real_real(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magneticPlanar)
        return 1.0 / (magneticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v) -
                magneticLabel[e->marker].conductivity * int_velocity<Real, Scalar>(n, wt, u, v, e, magneticLabel[e->marker].velocity_x, magneticLabel[e->marker].velocity_y, magneticLabel[e->marker].velocity_angular) +
                ((magneticAnalysisType == ANALYSISTYPE_TRANSIENT) ? magneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v) / magneticTimeStep : 0.0);

    else
        return 1.0 / (magneticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)) -
               magneticLabel[e->marker].conductivity * int_velocity<Real, Scalar>(n, wt, u, v, e, magneticLabel[e->marker].velocity_x, magneticLabel[e->marker].velocity_y, magneticLabel[e->marker].velocity_angular) +
               ((magneticAnalysisType == ANALYSISTYPE_TRANSIENT) ? magneticLabel[e->marker].conductivity * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e) / magneticTimeStep : 0.0);
}

template<typename Real, typename Scalar>
Scalar magnetic_bilinear_form_real_imag(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magneticPlanar)
        return - 2 * M_PI * magneticFrequency * magneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return - 2 * M_PI * magneticFrequency * magneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar magnetic_bilinear_form_imag_real(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magneticPlanar)
        return + 2 * M_PI * magneticFrequency * magneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return + 2 * M_PI * magneticFrequency * magneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar magnetic_bilinear_form_imag_imag(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magneticPlanar)
        return 1.0 / (magneticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v) -
                magneticLabel[e->marker].conductivity * int_velocity<Real, Scalar>(n, wt, u, v, e, magneticLabel[e->marker].velocity_x, magneticLabel[e->marker].velocity_y, magneticLabel[e->marker].velocity_angular);
    else
        return 1.0 / (magneticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v)) -
                magneticLabel[e->marker].conductivity * int_velocity<Real, Scalar>(n, wt, u, v, e, magneticLabel[e->marker].velocity_x, magneticLabel[e->marker].velocity_y, magneticLabel[e->marker].velocity_angular);
}

template<typename Real, typename Scalar>
Scalar magnetic_linear_form_real(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magneticPlanar)
        return magneticLabel[e->marker].current_density_real * int_v<Real, Scalar>(n, wt, v) +
                magneticLabel[e->marker].remanence / (magneticLabel[e->marker].permeability * MU0) * int_magnet<Real, Scalar>(n, wt, v, magneticLabel[e->marker].remanence_angle) +
                ((magneticAnalysisType == ANALYSISTYPE_TRANSIENT) ? magneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, ext->fn[0], v) / magneticTimeStep : 0.0);
    else
        return magneticLabel[e->marker].current_density_real * int_v<Real, Scalar>(n, wt, v) +
                magneticLabel[e->marker].remanence / (magneticLabel[e->marker].permeability * MU0) * int_magnet<Real, Scalar>(n, wt, v, magneticLabel[e->marker].remanence_angle) +
                ((magneticAnalysisType == ANALYSISTYPE_TRANSIENT) ? magneticLabel[e->marker].conductivity * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, ext->fn[0], v, e) / magneticTimeStep : 0.0);
}

template<typename Real, typename Scalar>
Scalar magnetic_linear_form_imag(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (magneticPlanar)
        return magneticLabel[e->marker].current_density_imag * int_v<Real, Scalar>(n, wt, v);
    else
        return magneticLabel[e->marker].current_density_imag * int_v<Real, Scalar>(n, wt, v);
}

QList<SolutionArray *> *magnetic_main(SolverThread *solverThread)
{
    int numberOfRefinements = Util::scene()->problemInfo()->numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;

    magneticPlanar = (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);

    magneticAnalysisType = Util::scene()->problemInfo()->analysisType;
    magneticFrequency = (magneticAnalysisType == ANALYSISTYPE_HARMONIC) ? Util::scene()->problemInfo()->frequency : 0.0;
    magneticTimeStep = Util::scene()->problemInfo()->timeStep;
    magneticTimeTotal = Util::scene()->problemInfo()->timeTotal;
    magneticInitialCondition = Util::scene()->problemInfo()->initialCondition;

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    int ndof;

    // load the mesh file
    Mesh mesh;
    mesh.load((tempProblemFileName() + ".mesh").toStdString().c_str());
    for (int i = 0; i < numberOfRefinements; i++)
        mesh.refine_all_elements(0);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    if (magneticAnalysisType == ANALYSISTYPE_HARMONIC)
    {
        // initialize the shapeset and the cache
        H1Shapeset shapeset;
        PrecalcShapeset pssreal(&shapeset);
        PrecalcShapeset pssimag(&shapeset);

        // create the x displacement space
        H1Space spacereal(&mesh, &shapeset);
        spacereal.set_bc_types(magnetic_bc_types);
        spacereal.set_bc_values(magnetic_bc_values_real);
        spacereal.set_uniform_order(polynomialOrder);
        ndof = spacereal.assign_dofs(0);

        // create the y displacement space
        H1Space spaceimag(&mesh, &shapeset);
        spaceimag.set_bc_types(magnetic_bc_types);
        spaceimag.set_bc_values(magnetic_bc_values_imag);
        spaceimag.set_uniform_order(polynomialOrder);

        // initialize the weak formulation
        WeakForm wf(2);
        wf.add_biform(0, 0, callback(magnetic_bilinear_form_real_real));
        wf.add_biform(0, 1, callback(magnetic_bilinear_form_real_imag));
        wf.add_biform(1, 0, callback(magnetic_bilinear_form_imag_real));
        wf.add_biform(1, 1, callback(magnetic_bilinear_form_imag_imag));
        wf.add_liform(0, callback(magnetic_linear_form_real));
        wf.add_liform(1, callback(magnetic_linear_form_imag));

        // initialize the linear solver
        UmfpackSolver umfpack;
        Solution *slnreal = new Solution();
        Solution *slnimag = new Solution();
        Solution rslnreal, rslnimag;

        // initialize the linear system
        LinSystem sys(&wf, &umfpack);
        sys.set_spaces(2, &spacereal, &spaceimag);
        sys.set_pss(2, &pssreal, &pssimag);

        // output
        SolutionArray *solutionArray;
        QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();

        // assemble the stiffness matrix and solve the system
        double error;
        int i;
        int adaptivitysteps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps;
        for (i = 0; i<(adaptivitysteps); i++)
        {
            int ndof = spacereal.assign_dofs(0);
            spaceimag.assign_dofs(ndof);

            sys.assemble();
            sys.solve(2, slnreal, slnimag);

            // calculate errors and adapt the solution
            if (adaptivityType != ADAPTIVITYTYPE_NONE)
            {
                RefSystem rs(&sys);
                rs.assemble();
                rs.solve(2, &rslnreal, &rslnimag);

                H1OrthoHP hp(2, &spacereal, &spaceimag);
                error = hp.calc_error_2(slnreal, slnimag, &rslnreal, &rslnimag) * 100;

                // emit signal
                solverThread->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
                if (solverThread->isCanceled()) return solutionArrayList;

                if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
                if (i != adaptivitysteps-1) hp.adapt(0.3, 0, (int) adaptivityType);
            }
        }

        // real part
        solutionArray = new SolutionArray();
        solutionArray->order = new Orderizer();
        solutionArray->order->process_solution(&spacereal);
        solutionArray->sln = slnreal;
        solutionArray->adaptiveError = error;
        solutionArray->adaptiveSteps = i-1;

        solutionArrayList->append(solutionArray);

        // iamg part
        solutionArray = new SolutionArray();
        solutionArray->order = new Orderizer();
        solutionArray->order->process_solution(&spaceimag);
        solutionArray->sln = slnimag;
        solutionArray->adaptiveError = error;
        solutionArray->adaptiveSteps = i-1;

        solutionArrayList->append(solutionArray);

        return solutionArrayList;
    }

    if (magneticAnalysisType == ANALYSISTYPE_STEADYSTATE || magneticAnalysisType == ANALYSISTYPE_TRANSIENT)
    {
        // initialize the shapeset and the cache
        H1Shapeset shapeset;
        PrecalcShapeset pss(&shapeset);

        // create an H1 space
        H1Space space(&mesh, &shapeset);
        space.set_bc_types(magnetic_bc_types);
        space.set_bc_values(magnetic_bc_values_real);
        space.set_uniform_order(polynomialOrder);
        space.assign_dofs();

        // solution
        QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();

        Solution *sln = new Solution();
        if (magneticAnalysisType == ANALYSISTYPE_TRANSIENT)
        {
            sln->set_const(&mesh, magneticInitialCondition);

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
        wf.add_biform(0, 0, callback(magnetic_bilinear_form_real_real));
        if (magneticAnalysisType == ANALYSISTYPE_TRANSIENT)
            wf.add_liform(0, callback(magnetic_linear_form_real), ANY, 1, sln);
        else
            wf.add_liform(0, callback(magnetic_linear_form_real));

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

            sys.assemble();
            sys.solve(1, sln);

            // calculate errors and adapt the solution
            if (adaptivityType != ADAPTIVITYTYPE_NONE)
            {
                RefSystem rs(&sys);
                rs.assemble();
                rs.solve(1, &rsln);

                H1OrthoHP hp(1, &space);
                error = hp.calc_error(sln, &rsln) * 100;

                // emit signal
                solverThread->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
                if (solverThread->isCanceled())
                {
                    solutionArrayList->clear();
                    return solutionArrayList;
                }

                if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
                if (i != adaptivitysteps-1) hp.adapt(0.3, 0, (int) adaptivityType);
            }
        }

        // timesteps
        int timesteps = (magneticAnalysisType == ANALYSISTYPE_TRANSIENT) ? floor(magneticTimeTotal/magneticTimeStep) : 1;
        for (int n = 0; n<timesteps; n++)
        {
            if (timesteps > 1)
            {
                sys.assemble(true);
                sys.solve(1, sln);
            }
            else if (n > 0)
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
            if (magneticAnalysisType == ANALYSISTYPE_TRANSIENT) solutionArray->time = (n+1)*magneticTimeStep;

            solutionArrayList->append(solutionArray);

            if (magneticAnalysisType == ANALYSISTYPE_TRANSIENT) solverThread->showMessage(QObject::tr("Solver: time step: %1/%2").arg(n+1).arg(timesteps), false);
            if (solverThread->isCanceled())
            {
                solutionArrayList->clear();
                return solutionArrayList;
            }
            solverThread->showProgress((int) (60.0 + 40.0*(n+1)/timesteps));
        }

        return solutionArrayList;
    }
}

// *******************************************************************************************************

int HermesMagnetic::numberOfSolution()
{
    return (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC) ? 2 : 1;
}

PhysicFieldVariable HermesMagnetic::contourPhysicFieldVariable()
{
    return (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC) ?
            PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL : PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL;
}

PhysicFieldVariable HermesMagnetic::scalarPhysicFieldVariable()
{
    return (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC) ?
            PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY : PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL;
}

PhysicFieldVariableComp HermesMagnetic::scalarPhysicFieldVariableComp()
{
    return (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC) ?
            PHYSICFIELDVARIABLECOMP_MAGNITUDE : PHYSICFIELDVARIABLECOMP_MAGNITUDE;
}

PhysicFieldVariable HermesMagnetic::vectorPhysicFieldVariable()
{
    return (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC) ?
            PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL : PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL;
}


void HermesMagnetic::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL:
    case PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT:
        Util::scene()->addEdgeMarker(new SceneEdgeMagneticMarker(element->attribute("name"),
                                                                 type,
                                                                 Value(element->attribute("value", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesMagnetic::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeMagneticMarker->type));
    element->setAttribute("value", edgeMagneticMarker->value.text);
}

void HermesMagnetic::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelMagneticMarker(element->attribute("name"),
                                                               Value(element->attribute("current_density_real", "0")),
                                                               Value(element->attribute("current_density_imag", "0")),
                                                               Value(element->attribute("permeability", "1")),
                                                               Value(element->attribute("conductivity", "0")),
                                                               Value(element->attribute("remanence", "0")),
                                                               Value(element->attribute("remanence_angle", "0")),
                                                               Value(element->attribute("velocity_x", "0")),
                                                               Value(element->attribute("velocity_y", "0")),
                                                               Value(element->attribute("velocity_angular", "0"))));
}

void HermesMagnetic::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(marker);

    element->setAttribute("current_density_real", labelMagneticMarker->current_density_real.text);
    element->setAttribute("current_density_imag", labelMagneticMarker->current_density_imag.text);
    element->setAttribute("permeability", labelMagneticMarker->permeability.text);
    element->setAttribute("conductivity", labelMagneticMarker->conductivity.text);
    element->setAttribute("remanence", labelMagneticMarker->remanence.text);
    element->setAttribute("remanence_angle", labelMagneticMarker->remanence_angle.text);
    element->setAttribute("velocity_x", labelMagneticMarker->velocity_x.text);
    element->setAttribute("velocity_y", labelMagneticMarker->velocity_y.text);
    element->setAttribute("velocity_angular", labelMagneticMarker->velocity_angular.text);}

LocalPointValue *HermesMagnetic::localPointValue(Point point)
{
    return new LocalPointValueMagnetic(point);
}

QStringList HermesMagnetic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "A_real" << "A_imag" << "A"
            << "B" << "Bx_real" << "By_real" << "B_real" << "Bx_imag" << "By_imag" << "B_imag"
            << "H" << "Hx_real" << "Hy_real" << "H_real" << "Hx_imag" << "Hy_imag" << "H_imag"
            << "Je_real" << "Je_imag" << "Je"
            << "Jit_real" << "Jit_imag" << "Jit"
            << "Jiv_real" << "Jiv_imag" << "Jiv"
            << "J_real" << "J_imag" << "J"
            << "pjt" << "pjv" << "pj" << "wm" << "mur" << "gamma" << "Br" << "Brangle" << "vx" << "vy";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesMagnetic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueMagnetic();
}

QStringList HermesMagnetic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesMagnetic::volumeIntegralValue()
{
    return new VolumeIntegralValueMagnetic();
}

QStringList HermesMagnetic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "Ie_real" << "Ie_imag" << "Ii_real" << "Iv_imag" << "Iv_real" << "Ii_imag" << "I_real" << "I_imag"
            << "Fx_real" << "Fx_imag" << "Fy_real" << "Fy_imag" << "T_real" << "T_imag"
            << "Pjt" << "Pjv" << "Pj" << "Wm";
    return QStringList(headers);
}

SceneEdgeMarker *HermesMagnetic::newEdgeMarker()
{    
    return new SceneEdgeMagneticMarker("new boundary",
                                       PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL,
                                       Value("0"));
}

SceneEdgeMarker *HermesMagnetic::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeMagneticMarker(name,
                                           physicFieldBCFromStringKey(type),
                                           Value(QString::number(value)));
    }

    return NULL;
}

SceneLabelMarker *HermesMagnetic::newLabelMarker()
{
    return new SceneLabelMagneticMarker("new material",
                                        Value("0"),
                                        Value("0"),
                                        Value("1"),
                                        Value("0"),
                                        Value("0"),
                                        Value("0"),
                                        Value("0"),
                                        Value("0"),
                                        Value("0"));
}

SceneLabelMarker *HermesMagnetic::newLabelMarker(PyObject *self, PyObject *args)
{
    double current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity, &remanence, &remanence_angle, &velocity_x, &velocity_y, &velocity_angular))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelMagneticMarker(name,
                                            Value(QString::number(current_density_real)),
                                            Value(QString::number(current_density_imag)),
                                            Value(QString::number(permeability)),
                                            Value(QString::number(conductivity)),
                                            Value(QString::number(remanence)),
                                            Value(QString::number(remanence_angle)),
                                            Value(QString::number(velocity_x)),
                                            Value(QString::number(velocity_y)),
                                            Value(QString::number(velocity_angular)));
    }

    return NULL;
}

void HermesMagnetic::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    // steady state
    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_STEADYSTATE)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL);
        // cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM), PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY);
        // cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL), PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY), PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY), PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE), PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE), PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE);
    }

    // harmonic
    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL), PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL), PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL), PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM), PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL), PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY), PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY), PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE), PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE), PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE);
    }
}

void HermesMagnetic::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    // steady state
    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_STEADYSTATE)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY);
    }

    // harmonic
    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL), PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL), PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG), PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY), PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY);
    }
}


void HermesMagnetic::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueMagnetic *localPointValueMagnetic = dynamic_cast<LocalPointValueMagnetic *>(localPointValue);

    // magnetic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Magnetic field"));
    magneticNode->setExpanded(true);

    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
    {
        // Permittivity
        addTreeWidgetItemValue(magneticNode, tr("Permeability:"), QString("%1").arg(localPointValueMagnetic->permeability, 0, 'f', 2), "");

        // Potential
        QTreeWidgetItem *itemPotential = new QTreeWidgetItem(magneticNode);
        itemPotential->setText(0, tr("Vector potential"));
        itemPotential->setExpanded(true);

        addTreeWidgetItemValue(itemPotential, tr("real:"), QString("%1").arg(localPointValueMagnetic->potential_real, 0, 'e', 3), "Wb/m");
        addTreeWidgetItemValue(itemPotential, tr("imag:"), QString("%1").arg(localPointValueMagnetic->potential_imag, 0, 'e', 3), "Wb/m");
        addTreeWidgetItemValue(itemPotential, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueMagnetic->potential_real) + sqr(localPointValueMagnetic->potential_imag)), 0, 'e', 3), "Wb/m");

        // Flux Density
        addTreeWidgetItemValue(magneticNode, "Flux density:", QString("%1").arg(sqrt(sqr(localPointValueMagnetic->B_real.x) + sqr(localPointValueMagnetic->B_imag.x) + sqr(localPointValueMagnetic->B_real.y) + sqr(localPointValueMagnetic->B_imag.y)), 0, 'e', 3), "T");

        // Flux Density - real
        QTreeWidgetItem *itemFluxDensityReal = new QTreeWidgetItem(magneticNode);
        itemFluxDensityReal->setText(0, tr("Flux density - real"));
        itemFluxDensityReal->setExpanded(true);

        addTreeWidgetItemValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->B_real.x, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->B_real.y, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensityReal, "B:", QString("%1").arg(localPointValueMagnetic->B_real.magnitude(), 0, 'e', 3), "T");

        // Flux Density - imag
        QTreeWidgetItem *itemFluxDensityImag = new QTreeWidgetItem(magneticNode);
        itemFluxDensityImag->setText(0, tr("Flux density - imag"));
        itemFluxDensityImag->setExpanded(true);

        addTreeWidgetItemValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->B_imag.x, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->B_imag.y, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensityImag, "B:", QString("%1").arg(localPointValueMagnetic->B_imag.magnitude(), 0, 'e', 3), "T");

        // Magnetic Field
        addTreeWidgetItemValue(magneticNode, "Magnetic field:", QString("%1").arg(sqrt(sqr(localPointValueMagnetic->H_real.x) + sqr(localPointValueMagnetic->H_imag.x) + sqr(localPointValueMagnetic->H_real.y) + sqr(localPointValueMagnetic->H_imag.y)), 0, 'e', 3), "A/m");

        // Magnetic Field - real
        QTreeWidgetItem *itemMagneticFieldReal = new QTreeWidgetItem(magneticNode);
        itemMagneticFieldReal->setText(0, tr("Magnetic field - real"));
        itemMagneticFieldReal->setExpanded(true);

        addTreeWidgetItemValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->H_real.x, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->H_real.y, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticFieldReal, "H", QString("%1").arg(localPointValueMagnetic->H_real.magnitude(), 0, 'e', 3), "A/m");

        // Magnetic Field - imag
        QTreeWidgetItem *itemMagneticFieldImag = new QTreeWidgetItem(magneticNode);
        itemMagneticFieldImag->setText(0, tr("Magnetic field - imag"));
        itemMagneticFieldImag->setExpanded(true);

        addTreeWidgetItemValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->H_imag.x, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->H_imag.y, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticFieldImag, "H", QString("%1").arg(localPointValueMagnetic->H_imag.magnitude(), 0, 'e', 3), "A/m");

        // Current Density
        QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(magneticNode);
        itemCurrentDensity->setText(0, tr("Currrent dens. - external"));
        itemCurrentDensity->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentDensity, tr("real:"), QString("%1").arg(localPointValueMagnetic->current_density_real, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensity, tr("imag:"), QString("%1").arg(localPointValueMagnetic->current_density_imag, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensity, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueMagnetic->current_density_real) + sqr(localPointValueMagnetic->current_density_imag)), 0, 'e', 3), "A/m2");

        // Transform induced current density
        QTreeWidgetItem *itemCurrentDensityInducedTransform = new QTreeWidgetItem(magneticNode);
        itemCurrentDensityInducedTransform->setText(0, tr("Current density - avg. transform"));
        itemCurrentDensityInducedTransform->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentDensityInducedTransform, tr("real:"), QString("%1").arg(localPointValueMagnetic->current_density_induced_transform_real, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensityInducedTransform, tr("imag:"), QString("%1").arg(localPointValueMagnetic->current_density_induced_transform_imag, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensityInducedTransform, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueMagnetic->current_density_induced_transform_real) + sqr(localPointValueMagnetic->current_density_induced_transform_imag)), 0, 'e', 3), "A/m2");

        // Velocity induced current density
        QTreeWidgetItem *itemCurrentDensityInducedVelocity = new QTreeWidgetItem(magneticNode);
        itemCurrentDensityInducedVelocity->setText(0, tr("Current density - velocity"));
        itemCurrentDensityInducedVelocity->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentDensityInducedVelocity, tr("real:"), QString("%1").arg(localPointValueMagnetic->current_density_induced_velocity_real, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensityInducedVelocity, tr("imag:"), QString("%1").arg(localPointValueMagnetic->current_density_induced_velocity_imag, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensityInducedVelocity, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueMagnetic->current_density_induced_velocity_real) + sqr(localPointValueMagnetic->current_density_induced_velocity_imag)), 0, 'e', 3), "A/m2");

        // Total current density
        QTreeWidgetItem *itemCurrentDensityTotal = new QTreeWidgetItem(magneticNode);
        itemCurrentDensityTotal->setText(0, tr("Current density - total"));
        itemCurrentDensityTotal->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentDensityTotal, tr("real:"), QString("%1").arg(localPointValueMagnetic->current_density_total_real, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensityTotal, tr("imag:"), QString("%1").arg(localPointValueMagnetic->current_density_total_imag, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemCurrentDensityTotal, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueMagnetic->current_density_total_real) + sqr(localPointValueMagnetic->current_density_total_imag)), 0, 'e', 3), "A/m2");

        // Magnetic Field
        addTreeWidgetItemValue(magneticNode, "Lorentz force:", QString("%1").arg(sqrt(sqr(localPointValueMagnetic->FL_real.x) + sqr(localPointValueMagnetic->FL_imag.x) + sqr(localPointValueMagnetic->FL_real.y) + sqr(localPointValueMagnetic->FL_imag.y)), 0, 'e', 3), "A/m");

        // Lorentz force - real
        QTreeWidgetItem *itemLorentzForceReal = new QTreeWidgetItem(magneticNode);
        itemLorentzForceReal->setText(0, tr("Lorentz force - real"));
        itemLorentzForceReal->setExpanded(true);

        addTreeWidgetItemValue(itemLorentzForceReal, "FL" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->FL_real.x, 0, 'e', 3), "N/m3");
        addTreeWidgetItemValue(itemLorentzForceReal, "FL" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->FL_real.y, 0, 'e', 3), "N/m3");
        addTreeWidgetItemValue(itemLorentzForceReal, "FL", QString("%1").arg(localPointValueMagnetic->FL_real.magnitude(), 0, 'e', 3), "N/m3");

        // Lorentz force - imag
        QTreeWidgetItem *itemLorentzForceImag = new QTreeWidgetItem(magneticNode);
        itemLorentzForceImag->setText(0, tr("Lorentz force - imag"));
        itemLorentzForceImag->setExpanded(true);

        addTreeWidgetItemValue(itemLorentzForceImag, "FL" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->FL_imag.x, 0, 'e', 3), "N/m3");
        addTreeWidgetItemValue(itemLorentzForceImag, "FL" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->FL_imag.y, 0, 'e', 3), "N/m3");
        addTreeWidgetItemValue(itemLorentzForceImag, "FL", QString("%1").arg(localPointValueMagnetic->FL_imag.magnitude(), 0, 'e', 3), "N/m3");

        // Power losses
        addTreeWidgetItemValue(magneticNode, tr("Losses dens. - avg. trans.:"), QString("%1").arg(localPointValueMagnetic->pj_transform, 0, 'e', 3), "W/m3");
        addTreeWidgetItemValue(magneticNode, tr("Losses dens. - velocity:"), QString("%1").arg(localPointValueMagnetic->pj_velocity, 0, 'e', 3), "W/m3");
        addTreeWidgetItemValue(magneticNode, tr("Losses dens. - total:"), QString("%1").arg(localPointValueMagnetic->pj_total, 0, 'e', 3), "W/m3");

        // Energy density
        addTreeWidgetItemValue(magneticNode, tr("Energy density:"), QString("%1").arg(localPointValueMagnetic->wm, 0, 'e', 3), "J/m3");
    }
    else
    {
        // Permeability
        addTreeWidgetItemValue(magneticNode, tr("Permeability:"), QString("%1").arg(localPointValueMagnetic->permeability, 0, 'f', 2), "");

        // Conductivity
        addTreeWidgetItemValue(magneticNode, tr("Conductivity:"), QString("%1").arg(localPointValueMagnetic->conductivity, 0, 'g', 3), "");

        // Remanence
        addTreeWidgetItemValue(magneticNode, tr("Rem. flux dens.:"), QString("%1").arg(localPointValueMagnetic->remanence, 0, 'e', 3), "T");
        addTreeWidgetItemValue(magneticNode, tr("Direction of rem.:"), QString("%1").arg(localPointValueMagnetic->remanence_angle, 0, 'f', 2), "deg.");

        // Velocity
        QTreeWidgetItem *itemVelocity = new QTreeWidgetItem(magneticNode);
        itemVelocity->setText(0, tr("Velocity"));
        itemVelocity->setExpanded(true);

        addTreeWidgetItemValue(itemVelocity, Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->velocity.x, 0, 'e', 3), "m/s");
        addTreeWidgetItemValue(itemVelocity, Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->velocity.y, 0, 'e', 3), "m/s");

        // Potential
        addTreeWidgetItemValue(magneticNode, tr("Vector potential:"), QString("%1").arg(localPointValueMagnetic->potential_real, 0, 'e', 3), "Wb/m");

        // Flux Density
        QTreeWidgetItem *itemFluxDensity = new QTreeWidgetItem(magneticNode);
        itemFluxDensity->setText(0, tr("Flux density"));
        itemFluxDensity->setExpanded(true);

        addTreeWidgetItemValue(itemFluxDensity, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->B_real.x, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensity, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->B_real.y, 0, 'e', 3), "T");
        addTreeWidgetItemValue(itemFluxDensity, "B:", QString("%1").arg(localPointValueMagnetic->B_real.magnitude(), 0, 'e', 3), "T");

        // Magnetic Field
        QTreeWidgetItem *itemMagneticField = new QTreeWidgetItem(magneticNode);
        itemMagneticField->setText(0, tr("Magnetic field"));
        itemMagneticField->setExpanded(true);

        addTreeWidgetItemValue(itemMagneticField, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueMagnetic->H_real.x, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticField, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueMagnetic->H_real.y, 0, 'e', 3), "A/m");
        addTreeWidgetItemValue(itemMagneticField, "H", QString("%1").arg(localPointValueMagnetic->H_real.magnitude(), 0, 'e', 3), "A/m");

        // Current density
        QTreeWidgetItem *itemInducedCurrentDensity = new QTreeWidgetItem(magneticNode);
        itemInducedCurrentDensity->setText(0, tr("Current density"));
        itemInducedCurrentDensity->setExpanded(true);

        addTreeWidgetItemValue(itemInducedCurrentDensity, tr("external:"), QString("%1").arg(localPointValueMagnetic->current_density_real, 0, 'e', 3), "m/s");
        addTreeWidgetItemValue(itemInducedCurrentDensity, tr("velocity:"), QString("%1").arg(localPointValueMagnetic->current_density_induced_velocity_real, 0, 'e', 3), "m/s");
        addTreeWidgetItemValue(itemInducedCurrentDensity, tr("total:"), QString("%1").arg(localPointValueMagnetic->current_density_total_real, 0, 'e', 3), "m/s");

        // Power losses
        addTreeWidgetItemValue(magneticNode, tr("Power losses:"), QString("%1").arg(localPointValueMagnetic->pj_velocity, 0, 'e', 3), "W/m3");

        // Energy density
        addTreeWidgetItemValue(magneticNode, tr("Energy density:"), QString("%1").arg(localPointValueMagnetic->wm, 0, 'e', 3), "J/m3");
    }
}

void HermesMagnetic::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    // SurfaceIntegralValueElasticity *surfaceIntegralValueElasticity = dynamic_cast<SurfaceIntegralValueElasticity *>(surfaceIntegralValue);
}

void HermesMagnetic::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueMagnetic *volumeIntegralValueMagnetic = dynamic_cast<VolumeIntegralValueMagnetic *>(volumeIntegralValue);

    // harmonic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Magnetic field"));
    magneticNode->setExpanded(true);

    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
    {
        // external current
        QTreeWidgetItem *itemCurrentInducedExternal = new QTreeWidgetItem(magneticNode);
        itemCurrentInducedExternal->setText(0, tr("External current"));
        itemCurrentInducedExternal->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentInducedExternal, tr("real:"), tr("%1").arg(volumeIntegralValueMagnetic->currentReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedExternal, tr("imag:"), tr("%1").arg(volumeIntegralValueMagnetic->currentImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedExternal, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentReal) + sqr(volumeIntegralValueMagnetic->currentImag)), 0, 'e', 3), "A");

        // transform induced current
        QTreeWidgetItem *itemCurrentInducedTransform = new QTreeWidgetItem(magneticNode);
        itemCurrentInducedTransform->setText(0, tr("Transform induced current"));
        itemCurrentInducedTransform->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentInducedTransform, tr("real:"), tr("%1").arg(volumeIntegralValueMagnetic->currentInducedTransformReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedTransform, tr("imag:"), tr("%1").arg(volumeIntegralValueMagnetic->currentInducedTransformImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedTransform, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentInducedTransformReal) + sqr(volumeIntegralValueMagnetic->currentInducedTransformImag)), 0, 'e', 3), "A");

        // velocity induced current
        QTreeWidgetItem *itemCurrentInducedVelocity = new QTreeWidgetItem(magneticNode);
        itemCurrentInducedVelocity->setText(0, tr("Velocity induced current"));
        itemCurrentInducedVelocity->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentInducedVelocity, tr("real:"), tr("%1").arg(volumeIntegralValueMagnetic->currentInducedVelocityReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedVelocity, tr("imag:"), tr("%1").arg(volumeIntegralValueMagnetic->currentInducedVelocityImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedVelocity, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentInducedVelocityReal) + sqr(volumeIntegralValueMagnetic->currentInducedVelocityImag)), 0, 'e', 3), "A");

        // total current
        QTreeWidgetItem *itemCurrentTotal = new QTreeWidgetItem(magneticNode);
        itemCurrentTotal->setText(0, tr("Total current"));
        itemCurrentTotal->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentTotal, tr("real:"), tr("%1").arg(volumeIntegralValueMagnetic->currentTotalReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentTotal, tr("imag:"), tr("%1").arg(volumeIntegralValueMagnetic->currentTotalImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentTotal, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentTotalReal) + sqr(volumeIntegralValueMagnetic->currentTotalImag)), 0, 'e', 3), "A");

        // Power losses
        QTreeWidgetItem *itemPowerLosses = new QTreeWidgetItem(magneticNode);
        itemPowerLosses->setText(0, tr("Power losses"));
        itemPowerLosses->setExpanded(true);

        addTreeWidgetItemValue(itemPowerLosses, tr("transform avg.:"), tr("%1").arg(volumeIntegralValueMagnetic->powerLossesTransform, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemPowerLosses, tr("velocity:"), tr("%1").arg(volumeIntegralValueMagnetic->powerLossesVelocity, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemPowerLosses, tr("total:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->powerLosses)), 0, 'e', 3), "A");

        addTreeWidgetItemValue(magneticNode, tr("Energy avg.:"), tr("%1").arg(volumeIntegralValueMagnetic->energy, 0, 'e', 3), tr("J"));

        // force
        QTreeWidgetItem *itemForce = new QTreeWidgetItem(magneticNode);
        itemForce->setText(0, tr("Lorentz force avg."));
        itemForce->setExpanded(true);

        addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelX(), tr("%1").arg(-volumeIntegralValueMagnetic->forceXReal/2.0, 0, 'e', 3), "N");
        addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelY(), tr("%1").arg(-volumeIntegralValueMagnetic->forceYReal/2.0, 0, 'e', 3), "N");

        addTreeWidgetItemValue(magneticNode, tr("Torque - real:"), tr("%1").arg(volumeIntegralValueMagnetic->torqueReal, 0, 'e', 3), tr("Nm"));
        addTreeWidgetItemValue(magneticNode, tr("Torque - imag:"), tr("%1").arg(volumeIntegralValueMagnetic->torqueImag, 0, 'e', 3), tr("Nm"));
    }
    else
    {
        addTreeWidgetItemValue(magneticNode, tr("External current:"), tr("%1").arg(volumeIntegralValueMagnetic->currentReal, 0, 'e', 3), tr("A"));
        addTreeWidgetItemValue(magneticNode, tr("Velocity current:"), tr("%1").arg(volumeIntegralValueMagnetic->currentInducedVelocityReal, 0, 'e', 3), tr("A"));
        addTreeWidgetItemValue(magneticNode, tr("Total current:"), tr("%1").arg(volumeIntegralValueMagnetic->currentTotalReal, 0, 'e', 3), tr("A"));
        addTreeWidgetItemValue(magneticNode, tr("Losses:"), tr("%1").arg(volumeIntegralValueMagnetic->powerLossesVelocity, 0, 'e', 3), tr("W"));
        addTreeWidgetItemValue(magneticNode, tr("Energy:"), tr("%1").arg(volumeIntegralValueMagnetic->energy, 0, 'e', 3), tr("J"));
        addTreeWidgetItemValue(magneticNode, tr("Fx:"), tr("%1").arg(volumeIntegralValueMagnetic->forceXReal, 0, 'e', 3), tr("N"));
        addTreeWidgetItemValue(magneticNode, tr("Fy:"), tr("%1").arg(volumeIntegralValueMagnetic->forceYReal, 0, 'e', 3), tr("N"));
        addTreeWidgetItemValue(magneticNode, tr("Torque:"), tr("%1").arg(volumeIntegralValueMagnetic->torqueReal, 0, 'e', 3), tr("Nm"));
    }
}

QList<SolutionArray *> *HermesMagnetic::solve(SolverThread *solverThread)
{
    // edge markers
    magneticEdge = new MagneticEdge[Util::scene()->edges.count()+1];
    magneticEdge[0].type = PHYSICFIELDBC_NONE;
    magneticEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            magneticEdge[i+1].type = PHYSICFIELDBC_NONE;
            magneticEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeMagneticMarker->value.evaluate()) return NULL;

            magneticEdge[i+1].type = edgeMagneticMarker->type;
            magneticEdge[i+1].value = edgeMagneticMarker->value.number;
        }
    }

    // label markers
    magneticLabel = new MagneticLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelMagneticMarker->current_density_real.evaluate()) return NULL;
            if (!labelMagneticMarker->current_density_imag.evaluate()) return NULL;
            if (!labelMagneticMarker->permeability.evaluate()) return NULL;
            if (!labelMagneticMarker->conductivity.evaluate()) return NULL;
            if (!labelMagneticMarker->remanence.evaluate()) return NULL;
            if (!labelMagneticMarker->remanence_angle.evaluate()) return NULL;
            if (!labelMagneticMarker->velocity_x.evaluate()) return NULL;
            if (!labelMagneticMarker->velocity_y.evaluate()) return NULL;
            if (!labelMagneticMarker->velocity_angular.evaluate()) return NULL;

            magneticLabel[i].current_density_real = labelMagneticMarker->current_density_real.number;
            magneticLabel[i].current_density_imag = labelMagneticMarker->current_density_imag.number;
            magneticLabel[i].permeability = labelMagneticMarker->permeability.number;
            magneticLabel[i].conductivity = labelMagneticMarker->conductivity.number;
            magneticLabel[i].remanence = labelMagneticMarker->remanence.number;
            magneticLabel[i].remanence_angle = labelMagneticMarker->remanence_angle.number;
            magneticLabel[i].velocity_x = labelMagneticMarker->velocity_x.number;
            magneticLabel[i].velocity_y = labelMagneticMarker->velocity_y.number;
            magneticLabel[i].velocity_angular = labelMagneticMarker->velocity_angular.number;        }
    }

    QList<SolutionArray *> *solutionArrayList = magnetic_main(solverThread);

    delete [] magneticEdge;
    delete [] magneticLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueMagnetic::LocalPointValueMagnetic(Point &point) : LocalPointValue(point)
{
    permeability = 0;
    conductivity = 0;
    remanence = 0;
    remanence_angle = 0;

    potential_real = 0;
    potential_imag = 0;

    current_density_real = 0;
    current_density_imag = 0;
    current_density_induced_transform_real = 0;
    current_density_induced_transform_imag = 0;
    current_density_induced_velocity_real = 0;
    current_density_induced_velocity_imag = 0;
    current_density_total_real = 0;
    current_density_total_imag = 0;

    H_real = Point();
    H_imag = Point();
    B_real = Point();
    B_imag = Point();
    FL_real = Point();
    FL_imag = Point();

    pj_transform = 0;
    pj_velocity = 0;
    pj_total = 0;
    wm = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        double frequency;

        // value real
        PointValue valueReal = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());

        // value imag
        PointValue valueImag;
        if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
        {
            valueImag = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln2());
            frequency = Util::scene()->problemInfo()->frequency;
        }
        else
        {
            valueImag = PointValue();
            frequency = 0.0;
        }

        // solution
        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(valueReal.marker);
        if (marker != NULL)
        {
            // potential
            potential_real = valueReal.value;
            potential_imag = valueImag.value;

            // magnetic field
            Point derReal;
            derReal = valueReal.derivative;

            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
            {
                B_real.x =  derReal.y;
                B_real.y = -derReal.x;
            }
            else
            {
                B_real.x = -derReal.y;
                B_real.y =  derReal.x + ((point.x > 0.0) ? potential_real/point.x : 0.0);
            }

            Point derImag;
            derImag = valueImag.derivative;

            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
            {
                B_imag.x =  derImag.y;
                B_imag.y = -derImag.x;
            }
            else
            {
                B_imag.x = -derImag.y;
                B_imag.y =  derImag.x + ((point.x > 0.0) ? potential_imag/point.x : 0.0);
            }

            permeability = marker->permeability.number;
            conductivity = marker->conductivity.number;
            remanence = marker->remanence.number;
            remanence_angle = marker->remanence_angle.number;
            velocity = Point(marker->velocity_x.number - marker->velocity_angular.number * point.y, marker->velocity_y.number + marker->velocity_angular.number * point.x);

            // current density
            current_density_real = marker->current_density_real.number;
            current_density_imag = marker->current_density_imag.number;

            // induced current density transform
            current_density_induced_transform_real =   2 * M_PI * frequency * marker->conductivity.number * potential_imag;
            current_density_induced_transform_imag = - 2 * M_PI * frequency * marker->conductivity.number * potential_real;

            // induced current density velocity
            current_density_induced_velocity_real = - conductivity * (velocity.x * derReal.x + velocity.y * derReal.y);
            current_density_induced_velocity_imag = - conductivity * (velocity.x * derImag.x + velocity.y * derImag.y);

            // total current density
            current_density_total_real = current_density_real + current_density_induced_transform_real + current_density_induced_velocity_real;
            current_density_total_imag = current_density_imag + current_density_induced_transform_imag + current_density_induced_velocity_imag;

            // electric displacement
            H_real = B_real / (marker->permeability.number * MU0);
            H_imag = B_imag / (marker->permeability.number * MU0);

            // Lorentz force
            FL_real.x = - (current_density_total_real*B_real.y - current_density_total_imag*B_imag.y);
            FL_real.y =   (current_density_total_real*B_real.x - current_density_total_imag*B_imag.x);
            FL_imag.x = - (current_density_total_imag*B_real.y + current_density_total_real*B_imag.y);
            FL_imag.y =   (current_density_total_imag*B_real.x + current_density_total_real*B_imag.x);

            // power losses
            pj_transform = (marker->conductivity.number > 0.0) ?
                      0.5 / marker->conductivity.number * (sqr(current_density_induced_transform_real) + sqr(current_density_induced_transform_imag)) : 0.0;
            pj_velocity = (marker->conductivity.number > 0.0) ?
                      0.5 / marker->conductivity.number * (sqr(current_density_induced_velocity_real) + sqr(current_density_induced_velocity_imag)) : 0.0;

            pj_total = pj_transform + pj_velocity;

            // energy density
            wm = 0.25 * (sqr(B_real.x) + sqr(B_real.y) + sqr(B_imag.x) + sqr(B_imag.y)) / (marker->permeability.number * MU0);
        }
    }
}

double LocalPointValueMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL:
        {
            return sqrt(sqr(potential_real) + sqr(potential_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL:
        {
            return potential_real;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG:
        {
            return potential_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY:
        {
            return sqrt(sqr(B_real.x) + sqr(B_imag.x) + sqr(B_real.y) + sqr(B_imag.y));
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return B_real.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return B_real.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return B_real.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return B_imag.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return B_imag.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return B_imag.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD:
        {
            return sqrt(sqr(H_real.x) + sqr(H_imag.x) + sqr(H_real.y) + sqr(H_imag.y));
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return H_real.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return H_real.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return H_real.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return H_imag.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return H_imag.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return H_imag.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL:
        {
            return current_density_real;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG:
        {
            return current_density_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY:
        {
            return sqrt(sqr(current_density_real) + sqr(current_density_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL:
        {
            return current_density_induced_transform_real;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG:
        {
            return current_density_induced_transform_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM:
        {
            return sqrt(sqr(current_density_induced_transform_real) + sqr(current_density_induced_transform_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
        {
            return current_density_induced_velocity_real;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
        {
            return current_density_induced_velocity_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY:
        {
            return sqrt(sqr(current_density_induced_velocity_real) + sqr(current_density_induced_velocity_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
        {
            return current_density_total_real;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
        {
            return current_density_total_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL:
        {
            return sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM:
        {
            return pj_transform;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY:
        {
            return pj_velocity;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL:
        {
            return pj_total;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY:
        {
            return wm;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY:
        {
            return permeability;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY:
        {
            return conductivity;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return velocity.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return velocity.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return sqrt(sqr(velocity.x) + sqr(velocity.y));
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE:
        {
            return remanence;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE:
        {
            return remanence_angle;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential_real, 0, 'e', 5) <<
            QString("%1").arg(potential_imag, 0, 'e', 5) <<
            QString("%1").arg(sqr(potential_real) + sqr(potential_imag), 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(B_real.x) + sqr(B_imag.x) + sqr(B_real.y) + sqr(B_imag.y)), 0, 'e', 5) <<
            QString("%1").arg(B_real.x, 0, 'e', 5) <<
            QString("%1").arg(B_real.y, 0, 'e', 5) <<
            QString("%1").arg(B_real.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(B_imag.x, 0, 'e', 5) <<
            QString("%1").arg(B_imag.y, 0, 'e', 5) <<
            QString("%1").arg(B_imag.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(H_real.x) + sqr(H_imag.x) + sqr(H_real.y) + sqr(H_imag.y)), 0, 'e', 5) <<
            QString("%1").arg(H_real.x, 0, 'e', 5) <<
            QString("%1").arg(H_real.y, 0, 'e', 5) <<
            QString("%1").arg(H_real.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(H_imag.x, 0, 'e', 5) <<
            QString("%1").arg(H_imag.y, 0, 'e', 5) <<
            QString("%1").arg(H_imag.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(current_density_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_real) + sqr(current_density_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_transform_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_transform_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_induced_transform_real) + sqr(current_density_induced_transform_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_velocity_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_velocity_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_induced_velocity_real) + sqr(current_density_induced_velocity_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_total_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_total_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag)), 0, 'e', 5) <<
            QString("%1").arg(pj_transform, 0, 'e', 5) <<
            QString("%1").arg(pj_velocity, 0, 'e', 5) <<
            QString("%1").arg(pj_total, 0, 'e', 5) <<
            QString("%1").arg(wm, 0, 'e', 5) <<
            QString("%1").arg(permeability, 0, 'f', 3) <<
            QString("%1").arg(conductivity, 0, 'e', 5) <<
            QString("%1").arg(remanence, 0, 'e', 5) <<
            QString("%1").arg(remanence_angle, 0, 'e', 5) <<
            QString("%1").arg(velocity.x, 0, 'e', 5) <<
            QString("%1").arg(velocity.y, 0, 'e', 5);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueMagnetic::SurfaceIntegralValueMagnetic() : SurfaceIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {

    }
}

QStringList SurfaceIntegralValueMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}


// ****************************************************************************************************************

VolumeIntegralValueMagnetic::VolumeIntegralValueMagnetic() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        currentReal = 0;
        currentImag = 0;
        currentInducedTransformReal = 0;
        currentInducedTransformImag = 0;
        currentInducedVelocityReal = 0;
        currentInducedVelocityImag = 0;
        currentTotalReal = 0;
        currentTotalImag = 0;
        powerLossesTransform = 0;
        powerLossesVelocity = 0;
        energy = 0;
        forceXReal = 0;
        forceXImag = 0;
        forceYReal = 0;
        forceYImag = 0;
        torqueReal = 0;
        torqueImag = 0;

        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_STEADYSTATE)
                {
                    currentReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_REAL);
                    // currentInducedTransformReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL);
                    currentInducedVelocityReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL);
                    // powerLossesTransform += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_TRANSFORM);
                    powerLossesVelocity += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_VELOCITY);
                    energy += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_ENERGY_DENSITY);
                    forceXReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_X_REAL);
                    forceYReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_Y_REAL);
                    torqueReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_TORQUE_REAL);
                }

                if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                {
                    currentReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_REAL);
                    currentImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_IMAG);
                    currentInducedTransformReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL);
                    currentInducedTransformImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG);
                    currentInducedVelocityReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL);
                    currentInducedVelocityImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG);
                    powerLossesTransform += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_TRANSFORM);
                    powerLossesVelocity += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_VELOCITY);
                    energy += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_ENERGY_DENSITY);
                    forceXReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_X_REAL);
                    forceXImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_X_IMAG);
                    forceYReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_Y_REAL);
                    forceYImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_Y_IMAG);
                    torqueReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_TORQUE_REAL);
                    torqueImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_TORQUE_IMAG);
                }
            }
        }

        currentTotalReal = currentReal + currentInducedTransformReal + currentInducedVelocityReal;
        currentTotalImag = currentImag + currentInducedTransformImag + currentInducedVelocityImag;
        powerLosses = powerLossesTransform + powerLossesVelocity;
        if (volume > 0)
        {
        }
    }
}

QStringList VolumeIntegralValueMagnetic::variables()
{ 
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(currentReal, 0, 'e', 5) <<
            QString("%1").arg(currentImag, 0, 'e', 5) <<
            QString("%1").arg(currentInducedTransformReal, 0, 'e', 5) <<
            QString("%1").arg(currentInducedTransformImag, 0, 'e', 5) <<
            QString("%1").arg(currentInducedVelocityReal, 0, 'e', 5) <<
            QString("%1").arg(currentInducedVelocityImag, 0, 'e', 5) <<
            QString("%1").arg(currentTotalReal, 0, 'e', 5) <<
            QString("%1").arg(currentTotalImag, 0, 'e', 5) <<
            QString("%1").arg(forceXReal, 0, 'e', 5) <<
            QString("%1").arg(forceXImag, 0, 'e', 5) <<
            QString("%1").arg(forceYReal, 0, 'e', 5) <<
            QString("%1").arg(forceYImag, 0, 'e', 5) <<
            QString("%1").arg(torqueReal, 0, 'e', 5) <<
            QString("%1").arg(torqueImag, 0, 'e', 5) <<
            QString("%1").arg(powerLossesTransform, 0, 'e', 5) <<
            QString("%1").arg(powerLossesVelocity, 0, 'e', 5) <<
            QString("%1").arg(powerLossesTransform, 0, 'e', 5) <<
            QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SceneEdgeMagneticMarker::SceneEdgeMagneticMarker(const QString &name, PhysicFieldBC type, Value value) : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeMagneticMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneEdgeMagneticMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL:
        out["Vector potential (Wb/m)"] = value.text;
        break;
    case PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT:
        out["Surface current (A/m2)"] = value.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeMagneticMarker::showDialog(QWidget *parent)
{
    DSceneEdgeMagneticMarker *dialog = new DSceneEdgeMagneticMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelMagneticMarker::SceneLabelMagneticMarker(const QString &name, Value current_density_real, Value current_density_imag, Value permeability, Value conductivity, Value remanence, Value remanence_angle, Value velocity_x, Value velocity_y, Value velocity_angular)
    : SceneLabelMarker(name)
{
    this->permeability = permeability;
    this->conductivity = conductivity;
    this->current_density_real = current_density_real;
    this->current_density_imag = current_density_imag;
    this->remanence = remanence;
    this->remanence_angle = remanence_angle;
    this->velocity_x = velocity_x;
    this->velocity_y = velocity_y;
    this->velocity_angular = velocity_angular;
}

QString SceneLabelMagneticMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4, %5, %6, %7, %8, %9, %10)").
            arg(name).
            arg(current_density_real.text).
            arg(current_density_imag.text).
            arg(permeability.text).
            arg(conductivity.text).
            arg(remanence.text).
            arg(remanence_angle.text).
            arg(velocity_x.text).
            arg(velocity_y.text).
            arg(velocity_angular.text);
}

QMap<QString, QString> SceneLabelMagneticMarker::data()
{
    QMap<QString, QString> out;
    out["Current density - real (A/m2)"] = current_density_real.text;
    out["Current density - imag (A/m2)"] = current_density_imag.text;
    out["Permeability (-)"] = permeability.text;
    out["Conductivity (S/m)"] = conductivity.text;
    out["Remanence (T)"] = remanence.text;
    out["Remanence angle (-)"] = remanence_angle.text;
    out["Velocity x (m/s)"] = conductivity.text;
    out["Velocity y (m/s)"] = conductivity.text;
    out["Angular velocity (m/s)"] = velocity_angular.text;
    return QMap<QString, QString>(out);
}

int SceneLabelMagneticMarker::showDialog(QWidget *parent)
{
    DSceneLabelMagneticMarker *dialog = new DSceneLabelMagneticMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeMagneticMarker::DSceneEdgeMagneticMarker(SceneEdgeMagneticMarker *edgeMagneticMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeMagneticMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtValue);

    load();
    setSize();
}

DSceneEdgeMagneticMarker::~DSceneEdgeMagneticMarker()
{
    delete cmbType;
    delete txtValue;
}

QLayout* DSceneEdgeMagneticMarker::createContent()
{
    cmbType = new QComboBox();
    cmbType->addItem("none", PHYSICFIELDBC_NONE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL), PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT), PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT);

    txtValue = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("BC type:"), cmbType);
    layoutMarker->addRow(tr("Value:"), txtValue);

    return layoutMarker;
}

void DSceneEdgeMagneticMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeMagneticMarker->type));
    txtValue->setValue(edgeMagneticMarker->value);
}

bool DSceneEdgeMagneticMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(m_edgeMarker);

    edgeMagneticMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeMagneticMarker->value  = txtValue->value();
    else
        return false;

    return true;
}

// *************************************************************************************************************************************

DSceneLabelMagneticMarker::DSceneLabelMagneticMarker(QWidget *parent, SceneLabelMagneticMarker *labelMagneticMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelMagneticMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, txtPermeability);
    setTabOrder(txtPermeability, txtConductivity);
    setTabOrder(txtConductivity, txtCurrentDensityReal);
    setTabOrder(txtCurrentDensityReal, txtCurrentDensityImag);

    load();
    setSize();
}

DSceneLabelMagneticMarker::~DSceneLabelMagneticMarker()
{
    delete txtPermeability;
    delete txtConductivity;
    delete txtCurrentDensityReal;
    delete txtCurrentDensityImag;
}

QLayout* DSceneLabelMagneticMarker::createContent()
{
    txtPermeability = new SLineEditValue(this);
    txtConductivity = new SLineEditValue(this);
    txtCurrentDensityReal = new SLineEditValue(this);
    txtCurrentDensityImag = new SLineEditValue(this);
    txtCurrentDensityImag->setEnabled(Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC);
    txtRemanence = new SLineEditValue(this);
    txtRemanenceAngle = new SLineEditValue(this);
    txtVelocityX = new SLineEditValue(this);
    txtVelocityX->setEnabled(Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
    txtVelocityY = new SLineEditValue(this);
    txtVelocityAngular = new SLineEditValue(this);
    txtVelocityAngular->setEnabled(Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtCurrentDensityReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtCurrentDensityImag);

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
    layoutMarkerVariables->addRow(tr("Conductivity (S/m):"), txtConductivity);
    layoutMarkerVariables->addRow(tr("Current density (A/m2):"), layoutCurrentDensity);

    QVBoxLayout *layoutMarker = new QVBoxLayout();
    layoutMarker->addLayout(layoutMarkerVariables);
    layoutMarker->addWidget(grpRemanence);
    layoutMarker->addWidget(grpVelocity);

    return layoutMarker;
}

void DSceneLabelMagneticMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(m_labelMarker);

    txtPermeability->setValue(labelMagneticMarker->permeability);
    txtConductivity->setValue(labelMagneticMarker->conductivity);
    txtCurrentDensityReal->setValue(labelMagneticMarker->current_density_real);
    txtCurrentDensityImag->setValue(labelMagneticMarker->current_density_imag);
    txtRemanence->setValue(labelMagneticMarker->remanence);
    txtRemanenceAngle->setValue(labelMagneticMarker->remanence_angle);
    txtVelocityX->setValue(labelMagneticMarker->velocity_x);
    txtVelocityY->setValue(labelMagneticMarker->velocity_y);
    txtVelocityAngular->setValue(labelMagneticMarker->velocity_angular);
}

bool DSceneLabelMagneticMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(m_labelMarker);

    if (txtPermeability->evaluate())
        labelMagneticMarker->permeability  = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        labelMagneticMarker->conductivity  = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        labelMagneticMarker->current_density_real  = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        labelMagneticMarker->current_density_imag  = txtCurrentDensityImag->value();
    else
        return false;

    if (txtRemanence->evaluate())
        labelMagneticMarker->remanence = txtRemanence->value();
    else
        return false;

    if (txtRemanenceAngle->evaluate())
        labelMagneticMarker->remanence_angle = txtRemanenceAngle->value();
    else
        return false;

    if (txtVelocityX->evaluate())
        labelMagneticMarker->velocity_x = txtVelocityX->value();
    else
        return false;

    if (txtVelocityY->evaluate())
        labelMagneticMarker->velocity_y = txtVelocityY->value();
    else
        return false;

    if (txtVelocityAngular->evaluate())
        labelMagneticMarker->velocity_angular = txtVelocityAngular->value();
    else
        return false;

    return true;
}
