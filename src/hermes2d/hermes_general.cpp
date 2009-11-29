#include "hermes_general.h"
#include "scene.h"

struct GeneralEdge
{
public:
    PhysicFieldBC type;
    double value;
};

struct GeneralLabel
{
    double rightside;
    double constant;
};

GeneralEdge *generalEdge;
GeneralLabel *generalLabel;
bool generalPlanar;

int general_bc_types(int marker)
{
    switch (generalEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_GENERAL_VALUE:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_GENERAL_DERIVATIVE:
        return BC_NATURAL;
        break;
    }
}

scalar general_bc_values(int marker, double x, double y)
{
    return generalEdge[marker].value;
}

template<typename Real, typename Scalar>
Scalar general_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    int marker = e->marker;

    if (generalPlanar)
        return generalLabel[marker].constant * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return generalLabel[marker].constant * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar general_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    int marker = e->marker;

    if (generalPlanar)
        return generalLabel[marker].rightside * int_v<Real, Scalar>(n, wt, v);
    else
        return generalLabel[marker].rightside * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

QList<SolutionArray *> *general_main(SolverThread *solverThread)
{
    generalPlanar = (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo()->numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;

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
    space.set_bc_types(general_bc_types);
    space.set_bc_values(general_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs(0);

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(general_bilinear_form));
    wf.add_liform(0, callback(general_linear_form));

    Solution *sln = new Solution();
    Solution rsln;

    // initialize the linear solver
    UmfpackSolver umfpack;

    // initialize the linear system
    LinSystem sys(&wf, &umfpack);
    sys.set_spaces(1, &space);
    sys.set_pss(1, &pss);

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    int adaptivitysteps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps + 1;
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
            solverThread->showMessage(QObject::tr("Solver: relative error is %1 %").arg(error, 0, 'f', 5), false);
            if (solverThread->isCanceled()) return NULL;

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            if (i != adaptivitysteps-1) hp.adapt(0.3, 0, (int) adaptivityType);
        }
    }

    // output
    SolutionArray *solutionArray = new SolutionArray();
    solutionArray->order = new Orderizer();
    solutionArray->order->process_solution(&space);
    solutionArray->sln = sln;
    solutionArray->adaptiveError = error;
    solutionArray->adaptiveSteps = i-1;

    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();
    solutionArrayList->append(solutionArray);
    
    return solutionArrayList;
}

// **************************************************************************************************************************

void HermesGeneral::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PHYSICFIELDBC_NONE:
    case PHYSICFIELDBC_GENERAL_VALUE:
    case PHYSICFIELDBC_GENERAL_DERIVATIVE:
        Util::scene()->addEdgeMarker(new SceneEdgeGeneralMarker(element->attribute("name"),
                                                                type,
                                                                Value(element->attribute("value", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesGeneral::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeGeneralMarker *edgeGeneralMarker = dynamic_cast<SceneEdgeGeneralMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeGeneralMarker->type));
    element->setAttribute("value", edgeGeneralMarker->value.text);
}

void HermesGeneral::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelGeneralMarker(element->attribute("name"),
                                                              Value(element->attribute("rightside", "0")),
                                                              Value(element->attribute("constant", "0"))));
}

void HermesGeneral::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelGeneralMarker *labelGeneralMarker = dynamic_cast<SceneLabelGeneralMarker *>(marker);

    element->setAttribute("rightside", labelGeneralMarker->rightside.text);
    element->setAttribute("constant", labelGeneralMarker->constant.text);
}

LocalPointValue *HermesGeneral::localPointValue(Point point)
{
    return new LocalPointValueGeneral(point);
}

QStringList HermesGeneral::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "V" << "Gx" << "Gy" << "G" << "constant";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesGeneral::surfaceIntegralValue()
{
    return new SurfaceIntegralValueGeneral();
}

QStringList HermesGeneral::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S";
    return QStringList(headers);
}

VolumeIntegralValue *HermesGeneral::volumeIntegralValue()
{
    return new VolumeIntegralValueGeneral();
}

QStringList HermesGeneral::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S";
    return QStringList(headers);
}

SceneEdgeMarker *HermesGeneral::newEdgeMarker()
{
    return new SceneEdgeGeneralMarker("new boundary",
                                      PHYSICFIELDBC_GENERAL_VALUE,
                                      Value("0"));
}

SceneEdgeMarker *HermesGeneral::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeGeneralMarker(name,
                                          physicFieldBCFromStringKey(type),
                                          Value(QString::number(value)));
    }

    return NULL;
}

SceneLabelMarker *HermesGeneral::newLabelMarker()
{
    return new SceneLabelGeneralMarker("new material",
                                       Value("0"),
                                       Value("1"));
}

SceneLabelMarker *HermesGeneral::newLabelMarker(PyObject *self, PyObject *args)
{
    double rightside, constant;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &rightside, &constant))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelGeneralMarker(name,
                                           Value(QString::number(rightside)),
                                           Value(QString::number(constant)));
    }

    return NULL;
}

void HermesGeneral::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueGeneral *localPointValueGeneral = dynamic_cast<LocalPointValueGeneral *>(localPointValue);

    // general
    QTreeWidgetItem *generalNode = new QTreeWidgetItem(trvWidget);
    generalNode->setText(0, tr("General field"));
    generalNode->setExpanded(true);

    // Constant
    addTreeWidgetItemValue(generalNode, tr("Constant:"), QString("%1").arg(localPointValueGeneral->constant, 0, 'f', 2), tr(""));

    // Right side
    addTreeWidgetItemValue(generalNode, tr("Right side:"), QString("%1").arg(localPointValueGeneral->rightside, 0, 'e', 3), "");

    // Value
    addTreeWidgetItemValue(generalNode, tr("Value:"), QString("%1").arg(localPointValueGeneral->variable, 0, 'e', 3), "");

    // Gradient
    QTreeWidgetItem *itemGradient = new QTreeWidgetItem(generalNode);
    itemGradient->setText(0, tr("Gradient"));
    itemGradient->setExpanded(true);

    addTreeWidgetItemValue(itemGradient, "G" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueGeneral->gradient.x, 0, 'e', 3), "");
    addTreeWidgetItemValue(itemGradient, "G" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueGeneral->gradient.y, 0, 'e', 3), "");
    addTreeWidgetItemValue(itemGradient, "G:", QString("%1").arg(localPointValueGeneral->gradient.magnitude(), 0, 'e', 3), "");
}

void HermesGeneral::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueGeneral *surfaceIntegralValueGeneral = dynamic_cast<SurfaceIntegralValueGeneral *>(surfaceIntegralValue);    
}

void HermesGeneral::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueGeneral *volumeIntegralValueGeneral = dynamic_cast<VolumeIntegralValueGeneral *>(volumeIntegralValue);
}

QList<SolutionArray *> *HermesGeneral::solve(SolverThread *solverThread)
{
    // edge markers
    generalEdge = new GeneralEdge[Util::scene()->edges.count()+1];
    generalEdge[0].type = PHYSICFIELDBC_NONE;
    generalEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            generalEdge[i+1].type = PHYSICFIELDBC_NONE;
            generalEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeGeneralMarker *edgeGeneralMarker = dynamic_cast<SceneEdgeGeneralMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeGeneralMarker->value.evaluate()) return NULL;

            generalEdge[i+1].type = edgeGeneralMarker->type;
            generalEdge[i+1].value = edgeGeneralMarker->value.number;
        }
    }

    // label markers
    generalLabel = new GeneralLabel[Util::scene()->labels.count()];
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
        {
        }
        else
        {
            SceneLabelGeneralMarker *labelGeneralMarker = dynamic_cast<SceneLabelGeneralMarker *>(Util::scene()->labels[i]->marker);

            // evaluate script
            if (!labelGeneralMarker->rightside.evaluate()) return NULL;
            if (!labelGeneralMarker->constant.evaluate()) return NULL;

            generalLabel[i].rightside = labelGeneralMarker->rightside.number;
            generalLabel[i].constant = labelGeneralMarker->constant.number;
        }
    }

    QList<SolutionArray *> *solutionArrayList = general_main(solverThread);

    delete [] generalEdge;
    delete [] generalLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueGeneral::LocalPointValueGeneral(Point &point) : LocalPointValue(point)
{
    variable = 0;
    rightside = 0;
    constant = 0;
    gradient = Point();

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());

        if (value.marker != NULL)
        {
            // value
            variable = value.value;

            // gradient
            gradient = value.derivative * (-1);

            SceneLabelGeneralMarker *marker = dynamic_cast<SceneLabelGeneralMarker *>(value.marker);

            rightside = marker->rightside.number;
            constant = marker->constant.number;
        }
    }
}

double LocalPointValueGeneral::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_GENERAL_VARIABLE:
        {
            return variable;
        }
        break;
    case PHYSICFIELDVARIABLE_GENERAL_GRADIENT:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return gradient.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return gradient.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return gradient.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_GENERAL_CONSTANT:
        {
            return constant;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueGeneral::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueGeneral::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(variable, 0, 'e', 5) <<
            QString("%1").arg(gradient.x, 0, 'e', 5) <<
            QString("%1").arg(gradient.y, 0, 'e', 5) <<
            QString("%1").arg(gradient.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(constant, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueGeneral::SurfaceIntegralValueGeneral() : SurfaceIntegralValue()
{

}

QStringList SurfaceIntegralValueGeneral::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueGeneral::VolumeIntegralValueGeneral() : VolumeIntegralValue()
{
}

QStringList VolumeIntegralValueGeneral::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

SceneEdgeGeneralMarker::SceneEdgeGeneralMarker(const QString &name, PhysicFieldBC type, Value value)
    : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeGeneralMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneEdgeGeneralMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PHYSICFIELDBC_GENERAL_VALUE:
        out["Value"] = value.text;
        break;
    case PHYSICFIELDBC_GENERAL_DERIVATIVE:
        out["Derivative"] = value.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeGeneralMarker::showDialog(QWidget *parent)
{
    DSceneEdgeGeneralMarker *dialog = new DSceneEdgeGeneralMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelGeneralMarker::SceneLabelGeneralMarker(const QString &name, Value rightside, Value constant)
    : SceneLabelMarker(name)
{
    this->rightside = rightside;
    this->constant = constant;
}

QString SceneLabelGeneralMarker::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(rightside.text).
            arg(constant.text);
}

QMap<QString, QString> SceneLabelGeneralMarker::data()
{
    QMap<QString, QString> out;
    out["Rightside"] = rightside.text;
    out["Constant"] = constant.text;
    return QMap<QString, QString>(out);
}

int SceneLabelGeneralMarker::showDialog(QWidget *parent)
{
    DSceneLabelGeneralMarker *dialog = new DSceneLabelGeneralMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeGeneralMarker::DSceneEdgeGeneralMarker(SceneEdgeGeneralMarker *edgeGeneralMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeGeneralMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtValue);

    load();
    setSize();
}

DSceneEdgeGeneralMarker::~DSceneEdgeGeneralMarker()
{
    delete cmbType;
    delete txtValue;
}

QLayout* DSceneEdgeGeneralMarker::createContent()
{
    cmbType = new QComboBox();
    cmbType->addItem("none", PHYSICFIELDBC_NONE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_GENERAL_VALUE), PHYSICFIELDBC_GENERAL_VALUE);
    cmbType->addItem(physicFieldBCString(PHYSICFIELDBC_GENERAL_DERIVATIVE), PHYSICFIELDBC_GENERAL_DERIVATIVE);

    txtValue = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("BC type:"), cmbType);
    layoutMarker->addRow(tr("Value:"), txtValue);

    return layoutMarker;
}

void DSceneEdgeGeneralMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeGeneralMarker *edgeGeneralMarker = dynamic_cast<SceneEdgeGeneralMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeGeneralMarker->type));
    txtValue->setValue(edgeGeneralMarker->value);
}

bool DSceneEdgeGeneralMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;

    SceneEdgeGeneralMarker *edgeGeneralMarker = dynamic_cast<SceneEdgeGeneralMarker *>(m_edgeMarker);

    edgeGeneralMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeGeneralMarker->value  = txtValue->value();
    else
        return false;

    return true;
}

// *************************************************************************************************************************************

DSceneLabelGeneralMarker::DSceneLabelGeneralMarker(QWidget *parent, SceneLabelGeneralMarker *labelGeneralMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelGeneralMarker;

    createDialog();

    // tab order
    setTabOrder(txtName, txtConstant);
    setTabOrder(txtConstant, txtRightSide);

    load();
    setSize();
}

DSceneLabelGeneralMarker::~DSceneLabelGeneralMarker()
{
    delete txtConstant;
    delete txtRightSide;
}

QLayout* DSceneLabelGeneralMarker::createContent()
{
    txtConstant = new SLineEditValue(this);
    txtRightSide = new SLineEditValue(this);

    QFormLayout *layoutMarker = new QFormLayout();
    layoutMarker->addRow(tr("Constant:"), txtConstant);
    layoutMarker->addRow(tr("Rightside:"), txtRightSide);

    return layoutMarker;
}

void DSceneLabelGeneralMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelGeneralMarker *labelGeneralMarker = dynamic_cast<SceneLabelGeneralMarker *>(m_labelMarker);

    txtConstant->setValue(labelGeneralMarker->constant);
    txtRightSide->setValue(labelGeneralMarker->rightside);
}

bool DSceneLabelGeneralMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelGeneralMarker *labelGeneralMarker = dynamic_cast<SceneLabelGeneralMarker *>(m_labelMarker);

    if (txtConstant->evaluate())
        labelGeneralMarker->constant = txtConstant->value();
    else
        return false;

    if (txtRightSide->evaluate())
        labelGeneralMarker->rightside = txtRightSide->value();
    else
        return false;

    return true;
}
