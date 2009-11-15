#include "hermes_harmonicmagnetic.h"
#include "scene.h"

struct HarmonicMagneticEdge
{
    PhysicFieldBC type;
    double value;
};

struct HarmonicMagneticLabel
{
    double current_density_real;
    double current_density_imag;
    double permeability;
    double conductivity;
};

HarmonicMagneticEdge *harmonicMagneticEdge;
HarmonicMagneticLabel *harmonicMagneticLabel;
bool harmonicMagneticPlanar;
double frequency;

int harmonicMagnetic_bc_types(int marker)
{
    switch (harmonicMagneticEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT:
        return BC_NATURAL;
        break;
    }
}

scalar harmonicMagnetic_bc_values_real(int marker, double x, double y)
{
    return harmonicMagneticEdge[marker].value;
}

scalar harmonicMagnetic_bc_values_imag(int marker, double x, double y)
{
    return harmonicMagneticEdge[marker].value;
}

template<typename Real, typename Scalar>
Scalar harmonicMagnetic_bilinear_form_real_real(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicMagneticPlanar)
        return 1.0 / (harmonicMagneticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return 1.0 / (harmonicMagneticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v));
}

template<typename Real, typename Scalar>
Scalar harmonicMagnetic_bilinear_form_real_imag(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicMagneticPlanar)
        return - 2 * M_PI * frequency * harmonicMagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return - 2 * M_PI * frequency * harmonicMagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar harmonicMagnetic_bilinear_form_imag_real(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicMagneticPlanar)
        return + 2 * M_PI * frequency * harmonicMagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return + 2 * M_PI * frequency * harmonicMagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar harmonicMagnetic_bilinear_form_imag_imag(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicMagneticPlanar)
        return 1.0 / (harmonicMagneticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return 1.0 / (harmonicMagneticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v));
}

template<typename Real, typename Scalar>
Scalar harmonicMagnetic_linear_form_real(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicMagneticPlanar)
        return harmonicMagneticLabel[e->marker].current_density_real * int_v<Real, Scalar>(n, wt, v);
    else
        return harmonicMagneticLabel[e->marker].current_density_real * int_v<Real, Scalar>(n, wt, v);
}

template<typename Real, typename Scalar>
Scalar harmonicMagnetic_linear_form_imag(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicMagneticPlanar)
        return harmonicMagneticLabel[e->marker].current_density_imag * int_v<Real, Scalar>(n, wt, v);
    else
        return harmonicMagneticLabel[e->marker].current_density_imag * int_v<Real, Scalar>(n, wt, v);
}

QList<SolutionArray *> *harmonicMagnetic_main(SolverThread *solverThread)
{
    frequency = Util::scene()->problemInfo()->frequency;
    harmonicMagneticPlanar = (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo()->numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;

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

    // initialize the shapeset and the cache
    H1Shapeset shapeset;
    PrecalcShapeset pssreal(&shapeset);
    PrecalcShapeset pssimag(&shapeset);

    // create the x displacement space
    H1Space spacereal(&mesh, &shapeset);
    spacereal.set_bc_types(harmonicMagnetic_bc_types);
    spacereal.set_bc_values(harmonicMagnetic_bc_values_real);
    spacereal.set_uniform_order(polynomialOrder);
    ndof = spacereal.assign_dofs(0);

    // create the y displacement space
    H1Space spaceimag(&mesh, &shapeset);
    spaceimag.set_bc_types(harmonicMagnetic_bc_types);
    spaceimag.set_bc_values(harmonicMagnetic_bc_values_imag);
    spaceimag.set_uniform_order(polynomialOrder);

    // initialize the weak formulation
    WeakForm wf(2);
    wf.add_biform(0, 0, callback(harmonicMagnetic_bilinear_form_real_real));
    wf.add_biform(0, 1, callback(harmonicMagnetic_bilinear_form_real_imag));
    wf.add_biform(1, 0, callback(harmonicMagnetic_bilinear_form_imag_real));
    wf.add_biform(1, 1, callback(harmonicMagnetic_bilinear_form_imag_imag));
    wf.add_liform(0, callback(harmonicMagnetic_linear_form_real));
    wf.add_liform(1, callback(harmonicMagnetic_linear_form_imag));

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln1 = new Solution();
    Solution *sln2 = new Solution();
    Solution rsln1, rsln2;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    int adaptivitysteps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps;
    for (i = 0; i<(adaptivitysteps); i++)
    {
        int ndof = spacereal.assign_dofs(0);
        spaceimag.assign_dofs(ndof);

        // initialize the linear system
        LinSystem sys(&wf, &umfpack);
        sys.set_spaces(2, &spacereal, &spaceimag);
        sys.set_pss(2, &pssreal, &pssimag);
        sys.assemble();
        sys.solve(2, sln1, sln2);

        RefSystem rs(&sys);
        rs.assemble();
        rs.solve(2, &rsln1, &rsln2);

        // calculate errors and adapt the solution
        if (adaptivityType != ADAPTIVITYTYPE_NONE)
        {
            H1OrthoHP hp(2, &spacereal, &spaceimag);
            error = hp.calc_error_2(sln1, sln2, &rsln1, &rsln2) * 100;

            // emit signal
            solverThread->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
            if (solverThread->isCanceled()) return NULL;

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            if (i != adaptivitysteps-1) hp.adapt(0.3, 0, (int) adaptivityType);
        }
    }

    // output
    SolutionArray *solutionArray = new SolutionArray();
    solutionArray->order1 = new Orderizer();
    solutionArray->order1->process_solution(&spacereal);
    solutionArray->order2 = new Orderizer();
    solutionArray->order2->process_solution(&spaceimag);
    solutionArray->sln1 = sln1;
    solutionArray->sln2 = sln2;
    solutionArray->adaptiveError = error;
    solutionArray->adaptiveSteps = i-1;

    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();
    solutionArrayList->append(solutionArray);

    return solutionArrayList;
}

// *******************************************************************************************************

void HermesHarmonicMagnetic::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL:
    case PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT:
        Util::scene()->addEdgeMarker(new SceneEdgeHarmonicMagneticMarker(element->attribute("name"),
                                                                         type,
                                                                         Value(element->attribute("value"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesHarmonicMagnetic::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker = dynamic_cast<SceneEdgeHarmonicMagneticMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeHarmonicMagneticMarker->type));
    element->setAttribute("value", edgeHarmonicMagneticMarker->value.text);
}

void HermesHarmonicMagnetic::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelHarmonicMagneticMarker(element->attribute("name"),
                                                                       Value(element->attribute("current_density_real")),
                                                                       Value(element->attribute("current_density_imag")),
                                                                       Value(element->attribute("permeability")),
                                                                       Value(element->attribute("conductivity"))));
}

void HermesHarmonicMagnetic::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(marker);

    element->setAttribute("current_density_real", labelHarmonicMagneticMarker->current_density_real.text);
    element->setAttribute("current_density_imag", labelHarmonicMagneticMarker->current_density_imag.text);
    element->setAttribute("permeability", labelHarmonicMagneticMarker->permeability.text);
    element->setAttribute("conductivity", labelHarmonicMagneticMarker->conductivity.text);
}

LocalPointValue *HermesHarmonicMagnetic::localPointValue(Point point)
{
    return new LocalPointValueHarmonicMagnetic(point);
}

QStringList HermesHarmonicMagnetic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "A_real" << "A_imag" << "A"
            << "B" << "Bx_real" << "By_real" << "B_real" << "Bx_imag" << "By_imag" << "B_imag"
            << "H" << "Hx_real" << "Hy_real" << "H_real" << "Hx_imag" << "Hy_imag" << "H_imag"
            << "Ji_real" << "Ji_imag" << "Ji"
            << "J_real" << "J_imag" << "J"
            << "pj" << "wm" << "mur";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesHarmonicMagnetic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueHarmonicMagnetic();
}

QStringList HermesHarmonicMagnetic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesHarmonicMagnetic::volumeIntegralValue()
{
    return new VolumeIntegralValueHarmonicMagnetic();
}

QStringList HermesHarmonicMagnetic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "Ii_real" << "Ii_imag" << "It_real" << "It_imag" << "Fx_real" << "Fx_imag" << "Fy_real" << "Fy_imag" << "Pj" << "Wm";
    return QStringList(headers);
}

SceneEdgeMarker *HermesHarmonicMagnetic::newEdgeMarker()
{    
    return new SceneEdgeHarmonicMagneticMarker("new boundary",
                                               PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL,
                                               Value("0"));
}

SceneEdgeMarker *HermesHarmonicMagnetic::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeHarmonicMagneticMarker(name,
                                                   physicFieldBCFromStringKey(type),
                                                   Value(QString::number(value)));
    }

    return NULL;
}

SceneLabelMarker *HermesHarmonicMagnetic::newLabelMarker()
{
    return new SceneLabelHarmonicMagneticMarker("new material",
                                                Value("0"),
                                                Value("0"),
                                                Value("1"),
                                                Value("0"));
}

SceneLabelMarker *HermesHarmonicMagnetic::newLabelMarker(PyObject *self, PyObject *args)
{
    double current_density_real, current_density_imag, permeability, conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelHarmonicMagneticMarker(name,
                                                 Value(QString::number(current_density_real)),
                                                 Value(QString::number(current_density_imag)),
                                                 Value(QString::number(permeability)),
                                                 Value(QString::number(conductivity)));
    }

    return NULL;
}

void HermesHarmonicMagnetic::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueHarmonicMagnetic *localPointValueHarmonicMagnetic = dynamic_cast<LocalPointValueHarmonicMagnetic *>(localPointValue);

    // harmonic magnetic
    QTreeWidgetItem *harmonicMagneticNode = new QTreeWidgetItem(trvWidget);
    harmonicMagneticNode->setText(0, tr("Harmonic magnetic field"));
    harmonicMagneticNode->setExpanded(true);

    // Permittivity
    addTreeWidgetItemValue(harmonicMagneticNode, tr("Permeability:"), QString("%1").arg(localPointValueHarmonicMagnetic->permeability, 0, 'f', 2), "");

    // Current Density
    QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentDensity->setText(0, tr("External current density"));
    itemCurrentDensity->setExpanded(true);

    addTreeWidgetItemValue(itemCurrentDensity, tr("real:"), QString("%1").arg(localPointValueHarmonicMagnetic->current_density_real, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, tr("imag:"), QString("%1").arg(localPointValueHarmonicMagnetic->current_density_imag, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->current_density_real) + sqr(localPointValueHarmonicMagnetic->current_density_imag)), 0, 'e', 3), "A/m2");

    // Average power losses
    addTreeWidgetItemValue(harmonicMagneticNode, tr("Average power losses dens.:"), QString("%1").arg(localPointValueHarmonicMagnetic->pj, 0, 'e', 3), "W/m3");

    // Energy density
    addTreeWidgetItemValue(harmonicMagneticNode, tr("Energy density:"), QString("%1").arg(localPointValueHarmonicMagnetic->wm, 0, 'e', 3), "J/m3");

    // Potential
    QTreeWidgetItem *itemPotential = new QTreeWidgetItem(harmonicMagneticNode);
    itemPotential->setText(0, tr("Vector potential"));
    itemPotential->setExpanded(true);

    addTreeWidgetItemValue(itemPotential, tr("real:"), QString("%1").arg(localPointValueHarmonicMagnetic->potential_real, 0, 'e', 3), "Wb/m");
    addTreeWidgetItemValue(itemPotential, tr("imag:"), QString("%1").arg(localPointValueHarmonicMagnetic->potential_imag, 0, 'e', 3), "Wb/m");
    addTreeWidgetItemValue(itemPotential, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->potential_real) + sqr(localPointValueHarmonicMagnetic->potential_imag)), 0, 'e', 3), "Wb/m");

    // Flux Density
    addTreeWidgetItemValue(harmonicMagneticNode, "Flux density:", QString("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->B_real.x) + sqr(localPointValueHarmonicMagnetic->B_imag.x) + sqr(localPointValueHarmonicMagnetic->B_real.y) + sqr(localPointValueHarmonicMagnetic->B_imag.y)), 0, 'e', 3), "T");

    // Flux Density - real
    QTreeWidgetItem *itemFluxDensityReal = new QTreeWidgetItem(harmonicMagneticNode);
    itemFluxDensityReal->setText(0, tr("Flux density - real"));
    itemFluxDensityReal->setExpanded(false);

    addTreeWidgetItemValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->B_real.x, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->B_real.y, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityReal, "B:", QString("%1").arg(localPointValueHarmonicMagnetic->B_real.magnitude(), 0, 'e', 3), "T");

    // Flux Density - imag
    QTreeWidgetItem *itemFluxDensityImag = new QTreeWidgetItem(harmonicMagneticNode);
    itemFluxDensityImag->setText(0, tr("Flux density - imag"));
    itemFluxDensityImag->setExpanded(false);

    addTreeWidgetItemValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->B_imag.x, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->B_imag.y, 0, 'e', 3), "T");
    addTreeWidgetItemValue(itemFluxDensityImag, "B:", QString("%1").arg(localPointValueHarmonicMagnetic->B_imag.magnitude(), 0, 'e', 3), "T");

    // Magnetic Field
    addTreeWidgetItemValue(harmonicMagneticNode, "Magnetic field:", QString("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->H_real.x) + sqr(localPointValueHarmonicMagnetic->H_imag.x) + sqr(localPointValueHarmonicMagnetic->H_real.y) + sqr(localPointValueHarmonicMagnetic->H_imag.y)), 0, 'e', 3), "A/m");

    // Magnetic Field - real
    QTreeWidgetItem *itemMagneticFieldReal = new QTreeWidgetItem(harmonicMagneticNode);
    itemMagneticFieldReal->setText(0, tr("Magnetic field - real"));
    itemMagneticFieldReal->setExpanded(false);

    addTreeWidgetItemValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->H_real.x, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->H_real.y, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldReal, "H", QString("%1").arg(localPointValueHarmonicMagnetic->H_real.magnitude(), 0, 'e', 3), "A/m");

    // Magnetic Field - imag
    QTreeWidgetItem *itemMagneticFieldImag = new QTreeWidgetItem(harmonicMagneticNode);
    itemMagneticFieldImag->setText(0, tr("Magnetic field - imag"));
    itemMagneticFieldImag->setExpanded(false);

    addTreeWidgetItemValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->H_imag.x, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->H_imag.y, 0, 'e', 3), "A/m");
    addTreeWidgetItemValue(itemMagneticFieldImag, "H", QString("%1").arg(localPointValueHarmonicMagnetic->H_imag.magnitude(), 0, 'e', 3), "A/m");

    // Total current density
    addTreeWidgetItemValue(harmonicMagneticNode, tr("Total current density:"), QString("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->current_density_total_real) + sqr(localPointValueHarmonicMagnetic->current_density_total_imag)), 0, 'e', 3), "A/m2");

    QTreeWidgetItem *itemCurrentDensityTotal = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentDensityTotal->setText(0, tr("Total current density"));
    itemCurrentDensityTotal->setExpanded(false);

    addTreeWidgetItemValue(itemCurrentDensityTotal, tr("real:"), QString("%1").arg(localPointValueHarmonicMagnetic->current_density_total_real, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensityTotal, tr("imag:"), QString("%1").arg(localPointValueHarmonicMagnetic->current_density_total_imag, 0, 'e', 3), "A/m2");

    // Induced current density
    addTreeWidgetItemValue(harmonicMagneticNode, tr("Induced current density:"), QString("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->current_density_induced_real) + sqr(localPointValueHarmonicMagnetic->current_density_induced_imag)), 0, 'e', 3), "A/m2");

    QTreeWidgetItem *itemCurrentDensityInduced = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentDensityInduced->setText(0, tr("Induced current density"));
    itemCurrentDensityInduced->setExpanded(false);

    addTreeWidgetItemValue(itemCurrentDensityInduced, tr("real:"), QString("%1").arg(localPointValueHarmonicMagnetic->current_density_induced_real, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensityInduced, tr("imag:"), QString("%1").arg(localPointValueHarmonicMagnetic->current_density_induced_imag, 0, 'e', 3), "A/m2");

    // Magnetic Field
    addTreeWidgetItemValue(harmonicMagneticNode, "Lorentz force:", QString("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->FL_real.x) + sqr(localPointValueHarmonicMagnetic->FL_imag.x) + sqr(localPointValueHarmonicMagnetic->FL_real.y) + sqr(localPointValueHarmonicMagnetic->FL_imag.y)), 0, 'e', 3), "A/m");

    // Lorentz force - real
    QTreeWidgetItem *itemLorentzForceReal = new QTreeWidgetItem(harmonicMagneticNode);
    itemLorentzForceReal->setText(0, tr("Lorentz force - real"));
    itemLorentzForceReal->setExpanded(false);

    addTreeWidgetItemValue(itemLorentzForceReal, "FL" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->FL_real.x, 0, 'e', 3), "N/m3");
    addTreeWidgetItemValue(itemLorentzForceReal, "FL" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->FL_real.y, 0, 'e', 3), "N/m3");
    addTreeWidgetItemValue(itemLorentzForceReal, "FL", QString("%1").arg(localPointValueHarmonicMagnetic->FL_real.magnitude(), 0, 'e', 3), "N/m3");

    // Lorentz force - imag
    QTreeWidgetItem *itemLorentzForceImag = new QTreeWidgetItem(harmonicMagneticNode);
    itemLorentzForceImag->setText(0, tr("Lorentz force - imag"));
    itemLorentzForceImag->setExpanded(false);

    addTreeWidgetItemValue(itemLorentzForceImag, "FL" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->FL_imag.x, 0, 'e', 3), "N/m3");
    addTreeWidgetItemValue(itemLorentzForceImag, "FL" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueHarmonicMagnetic->FL_imag.y, 0, 'e', 3), "N/m3");
    addTreeWidgetItemValue(itemLorentzForceImag, "FL", QString("%1").arg(localPointValueHarmonicMagnetic->FL_imag.magnitude(), 0, 'e', 3), "N/m3");
}

void HermesHarmonicMagnetic::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    // SurfaceIntegralValueElasticity *surfaceIntegralValueElasticity = dynamic_cast<SurfaceIntegralValueElasticity *>(surfaceIntegralValue);
}

void HermesHarmonicMagnetic::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueHarmonicMagnetic *volumeIntegralValueHarmonicMagnetic = dynamic_cast<VolumeIntegralValueHarmonicMagnetic *>(volumeIntegralValue);

    // harmonic magnetic
    QTreeWidgetItem *harmonicMagneticNode = new QTreeWidgetItem(trvWidget);
    harmonicMagneticNode->setText(0, tr("Harmonic magnetic field"));
    harmonicMagneticNode->setExpanded(true);

    // total current
    QTreeWidgetItem *itemCurrentTotal = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentTotal->setText(0, tr("Total current"));
    itemCurrentTotal->setExpanded(true);

    addTreeWidgetItemValue(itemCurrentTotal, tr("real:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentTotalReal, 0, 'e', 3), "A");
    addTreeWidgetItemValue(itemCurrentTotal, tr("imag:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentTotalImag, 0, 'e', 3), "A");
    addTreeWidgetItemValue(itemCurrentTotal, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueHarmonicMagnetic->currentTotalReal) + sqr(volumeIntegralValueHarmonicMagnetic->currentTotalImag)), 0, 'e', 3), "A");

    // induced current
    QTreeWidgetItem *itemCurrentInduced = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentInduced->setText(0, tr("Induced current"));
    itemCurrentInduced->setExpanded(true);

    addTreeWidgetItemValue(itemCurrentInduced, tr("real:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentInducedReal, 0, 'e', 3), "A");
    addTreeWidgetItemValue(itemCurrentInduced, tr("imag:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentInducedImag, 0, 'e', 3), "A");
    addTreeWidgetItemValue(itemCurrentInduced, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueHarmonicMagnetic->currentInducedReal) + sqr(volumeIntegralValueHarmonicMagnetic->currentInducedImag)), 0, 'e', 3), "A");

    addTreeWidgetItemValue(harmonicMagneticNode, tr("Power losses avg.:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->powerLosses, 0, 'e', 3), tr("W"));
    addTreeWidgetItemValue(harmonicMagneticNode, tr("Energy avg.:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->energy, 0, 'e', 3), tr("J"));

    // force
    QTreeWidgetItem *itemForce = new QTreeWidgetItem(harmonicMagneticNode);
    itemForce->setText(0, tr("Lorentz force avg."));
    itemForce->setExpanded(true);

    addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelX(), tr("%1").arg(-volumeIntegralValueHarmonicMagnetic->forceXReal/2.0, 0, 'e', 3), "N");
    addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelY(), tr("%1").arg(-volumeIntegralValueHarmonicMagnetic->forceYReal/2.0, 0, 'e', 3), "N");
}

QList<SolutionArray *> *HermesHarmonicMagnetic::solve(SolverThread *solverThread)
{
    // edge markers
    harmonicMagneticEdge = new HarmonicMagneticEdge[Util::scene()->edges.count()+1];
    harmonicMagneticEdge[0].type = PHYSICFIELDBC_NONE;
    harmonicMagneticEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            harmonicMagneticEdge[i+1].type = PHYSICFIELDBC_NONE;
            harmonicMagneticEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker = dynamic_cast<SceneEdgeHarmonicMagneticMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeHarmonicMagneticMarker->value.evaluate()) return NULL;

            harmonicMagneticEdge[i+1].type = edgeHarmonicMagneticMarker->type;
            harmonicMagneticEdge[i+1].value = edgeHarmonicMagneticMarker->value.number;
        }
    }

    // label markers
    harmonicMagneticLabel = new HarmonicMagneticLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelHarmonicMagneticMarker->current_density_real.evaluate()) return NULL;
            if (!labelHarmonicMagneticMarker->current_density_imag.evaluate()) return NULL;
            if (!labelHarmonicMagneticMarker->permeability.evaluate()) return NULL;
            if (!labelHarmonicMagneticMarker->conductivity.evaluate()) return NULL;

            harmonicMagneticLabel[i].current_density_real = labelHarmonicMagneticMarker->current_density_real.number;
            harmonicMagneticLabel[i].current_density_imag = labelHarmonicMagneticMarker->current_density_imag.number;
            harmonicMagneticLabel[i].permeability = labelHarmonicMagneticMarker->permeability.number;
            harmonicMagneticLabel[i].conductivity = labelHarmonicMagneticMarker->conductivity.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = harmonicMagnetic_main(solverThread);

    delete [] harmonicMagneticEdge;
    delete [] harmonicMagneticLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueHarmonicMagnetic::LocalPointValueHarmonicMagnetic(Point &point) : LocalPointValue(point)
{
    current_density_real = 0;
    current_density_imag = 0;
    permeability = 0;

    potential_real = 0;
    potential_imag = 0;
    H_real = Point();
    H_imag = Point();
    B_real = Point();
    B_imag = Point();
    wm = 0;

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue valueReal = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
        PointValue valueImag = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln2());
        if (valueReal.marker != NULL)
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

            SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(valueReal.marker);
            permeability = marker->permeability.number;

            // current density
            current_density_real = marker->current_density_real.number;
            current_density_imag = marker->current_density_imag.number;

            // induced current density
            current_density_induced_real =   2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * potential_imag;
            current_density_induced_imag = - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * potential_real;

            // total current density
            current_density_total_real = current_density_real + current_density_induced_real;
            current_density_total_imag = current_density_imag + current_density_induced_imag;

            // electric displacement
            H_real = B_real / (marker->permeability.number * MU0);
            H_imag = B_imag / (marker->permeability.number * MU0);

            // Lorentz force
            FL_real.x = - (current_density_total_real*B_real.y - current_density_total_imag*B_imag.y);
            FL_real.y =   (current_density_total_real*B_real.x - current_density_total_imag*B_imag.x);
            FL_imag.x = - (current_density_total_imag*B_real.y + current_density_total_real*B_imag.y);
            FL_imag.y =   (current_density_total_imag*B_real.x + current_density_total_real*B_imag.x);

            // power losses
            pj = (marker->conductivity.number > 0.0) ?
                 0.5 / marker->conductivity.number * (sqr(current_density_total_real) + sqr(current_density_total_imag)) : 0.0;

            // energy density
            wm = 0.25 * (sqr(B_real.x) + sqr(B_real.y) + sqr(B_imag.x) + sqr(B_imag.y)) / (marker->permeability.number * MU0);
        }
    }
}

double LocalPointValueHarmonicMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL:
        {
            return sqrt(sqr(potential_real) + sqr(potential_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_REAL:
        {
            return potential_real;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_VECTOR_POTENTIAL_IMAG:
        {
            return potential_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY:
        {
            return sqrt(sqr(B_real.x) + sqr(B_imag.x) + sqr(B_real.y) + sqr(B_imag.y));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_REAL:
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
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_FLUX_DENSITY_IMAG:
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
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD:
        {
            return sqrt(sqr(H_real.x) + sqr(H_imag.x) + sqr(H_real.y) + sqr(H_imag.y));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_REAL:
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
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_MAGNETICFIELD_IMAG:
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
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL:
        {
            return sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
        {
            return current_density_total_real;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
        {
            return current_density_total_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED:
        {
            return sqrt(sqr(current_density_induced_real) + sqr(current_density_induced_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_REAL:
        {
            return current_density_induced_real;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_IMAG:
        {
            return current_density_induced_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_POWER_LOSSES:
        {
            return pj;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_ENERGY_DENSITY:
        {
            return wm;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONICMAGNETIC_PERMEABILITY:
        {
            return permeability;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueHarmonicMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueHarmonicMagnetic::variables()
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
            QString("%1").arg(current_density_induced_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_induced_real) + sqr(current_density_induced_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_total_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_total_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag)), 0, 'e', 5) <<
            QString("%1").arg(pj, 0, 'e', 5) <<
            QString("%1").arg(wm, 0, 'e', 5) <<
            QString("%1").arg(permeability, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueHarmonicMagnetic::SurfaceIntegralValueHarmonicMagnetic() : SurfaceIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {

    }
}

QStringList SurfaceIntegralValueHarmonicMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}


// ****************************************************************************************************************

VolumeIntegralValueHarmonicMagnetic::VolumeIntegralValueHarmonicMagnetic() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        currentInducedReal = 0;
        currentInducedImag = 0;
        currentTotalReal = 0;
        currentTotalImag = 0;
        powerLosses = 0;
        energy = 0;
        forceXReal = 0;
        forceXImag = 0;
        forceYReal = 0;
        forceYImag = 0;
        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                currentInducedReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_REAL);
                currentInducedImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_CURRENT_DENSITY_INDUCED_IMAG);
                currentTotalReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_REAL);
                currentTotalImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_CURRENT_DENSITY_TOTAL_IMAG);
                powerLosses += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_POWER_LOSSES);
                energy += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_ENERGY_DENSITY);
                forceXReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_LORENTZ_FORCE_X_REAL);
                forceXImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_LORENTZ_FORCE_X_IMAG);
                forceYReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_LORENTZ_FORCE_Y_REAL);
                forceYImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONICMAGNETIC_LORENTZ_FORCE_Y_IMAG);
            }
        }

        if (volume > 0)
        {
        }
    }
}

QStringList VolumeIntegralValueHarmonicMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(currentInducedReal, 0, 'e', 5) <<
            QString("%1").arg(currentInducedImag, 0, 'e', 5) <<
            QString("%1").arg(currentTotalReal, 0, 'e', 5) <<
            QString("%1").arg(currentTotalImag, 0, 'e', 5) <<
            QString("%1").arg(forceXReal, 0, 'e', 5) <<
            QString("%1").arg(forceXImag, 0, 'e', 5) <<
            QString("%1").arg(forceYReal, 0, 'e', 5) <<
            QString("%1").arg(forceYImag, 0, 'e', 5) <<
            QString("%1").arg(powerLosses, 0, 'e', 5) <<
            QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SceneEdgeHarmonicMagneticMarker::SceneEdgeHarmonicMagneticMarker(const QString &name, PhysicFieldBC type, Value value) : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeHarmonicMagneticMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneEdgeHarmonicMagneticMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL:
        out["Vector potential (Wb/m)"] = value.text;
        break;
    case PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT:
        out["Surface current (A/m2)"] = value.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeHarmonicMagneticMarker::showDialog(QWidget *parent)
{
    DSceneEdgeHarmonicMagneticMarker *dialog = new DSceneEdgeHarmonicMagneticMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelHarmonicMagneticMarker::SceneLabelHarmonicMagneticMarker(const QString &name, Value current_density_real, Value current_density_imag, Value permeability, Value conductivity)
    : SceneLabelMarker(name)
{
    this->permeability = permeability;
    this->conductivity = conductivity;
    this->current_density_real = current_density_real;
    this->current_density_imag = current_density_imag;
}

QString SceneLabelHarmonicMagneticMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4, %5)").
            arg(name).
            arg(current_density_real.text).
            arg(current_density_imag.text).
            arg(permeability.text).
            arg(conductivity.text);
}

QMap<QString, QString> SceneLabelHarmonicMagneticMarker::data()
{
    QMap<QString, QString> out;
    out["Current density - real (A/m2)"] = current_density_real.text;
    out["Current density - imag (A/m2)"] = current_density_imag.text;
    out["Permeability (-)"] = permeability.text;
    out["Conductivity (S/m)"] = conductivity.text;
    return QMap<QString, QString>(out);
}

int SceneLabelHarmonicMagneticMarker::showDialog(QWidget *parent)
{
    DSceneLabelHarmonicMagneticMarker *dialog = new DSceneLabelHarmonicMagneticMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeHarmonicMagneticMarker::DSceneEdgeHarmonicMagneticMarker(SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeHarmonicMagneticMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtValue);

    load();
    setSize();
}

DSceneEdgeHarmonicMagneticMarker::~DSceneEdgeHarmonicMagneticMarker()
{
    delete cmbType;
    delete txtValue;
}

QLayout* DSceneEdgeHarmonicMagneticMarker::createContent()
{
    cmbType = new QComboBox();
    cmbType->addItem("none", PHYSICFIELDBC_NONE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL), PHYSICFIELDBC_HARMONICMAGNETIC_VECTOR_POTENTIAL);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT), PHYSICFIELDBC_HARMONICMAGNETIC_SURFACE_CURRENT);

    txtValue = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("BC type:"), cmbType);
    layoutMarker->addRow(tr("Value:"), txtValue);

    return layoutMarker;
}

void DSceneEdgeHarmonicMagneticMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker = dynamic_cast<SceneEdgeHarmonicMagneticMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeHarmonicMagneticMarker->type));
    txtValue->setValue(edgeHarmonicMagneticMarker->value);
}

bool DSceneEdgeHarmonicMagneticMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker = dynamic_cast<SceneEdgeHarmonicMagneticMarker *>(m_edgeMarker);

    edgeHarmonicMagneticMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeHarmonicMagneticMarker->value  = txtValue->value();
    else
        return false;

    return true;
}

// *************************************************************************************************************************************

DSceneLabelHarmonicMagneticMarker::DSceneLabelHarmonicMagneticMarker(QWidget *parent, SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelHarmonicMagneticMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, txtPermeability);
    setTabOrder(txtPermeability, txtConductivity);
    setTabOrder(txtConductivity, txtCurrentDensityReal);
    setTabOrder(txtCurrentDensityReal, txtCurrentDensityImag);

    load();
    setSize();
}

DSceneLabelHarmonicMagneticMarker::~DSceneLabelHarmonicMagneticMarker()
{
    delete txtPermeability;
    delete txtConductivity;
    delete txtCurrentDensityReal;
    delete txtCurrentDensityImag;
}

QLayout* DSceneLabelHarmonicMagneticMarker::createContent()
{
    txtPermeability = new SLineEditValue(this);
    txtConductivity = new SLineEditValue(this);
    txtCurrentDensityReal = new SLineEditValue(this);
    txtCurrentDensityImag = new SLineEditValue(this);

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtCurrentDensityReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtCurrentDensityImag);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("Permeability (-):"), txtPermeability);
    layoutMarker->addRow(tr("Conductivity (-):"), txtConductivity);
    layoutMarker->addRow(tr("Current density (A/m2):"), layoutCurrentDensity);

    return layoutMarker;
}

void DSceneLabelHarmonicMagneticMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_labelMarker);

    txtPermeability->setValue(labelHarmonicMagneticMarker->permeability);
    txtConductivity->setValue(labelHarmonicMagneticMarker->conductivity);
    txtCurrentDensityReal->setValue(labelHarmonicMagneticMarker->current_density_real);
    txtCurrentDensityImag->setValue(labelHarmonicMagneticMarker->current_density_imag);
}

bool DSceneLabelHarmonicMagneticMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_labelMarker);

    if (txtPermeability->evaluate())
        labelHarmonicMagneticMarker->permeability  = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        labelHarmonicMagneticMarker->conductivity  = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        labelHarmonicMagneticMarker->current_density_real  = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        labelHarmonicMagneticMarker->current_density_imag  = txtCurrentDensityImag->value();
    else
        return false;

    return true;
}
