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

#include "hermes_general.h"

#include "scene.h"
#include "gui.h"

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

BCType general_bc_types(int marker)
{
    switch (generalEdge[marker].type)
    {
    case PhysicFieldBC_None:
        return BC_NONE;
    case PhysicFieldBC_General_Value:
        return BC_ESSENTIAL;
    case PhysicFieldBC_General_Derivative:
        return BC_NATURAL;
    }
}

scalar general_bc_values(int marker, double x, double y)
{
    return generalEdge[marker].value;
}

template<typename Real, typename Scalar>
Scalar general_vector_form_surf(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double derivative = 0.0;

    if (generalEdge[e->edge_marker].type == PhysicFieldBC_General_Derivative)
        derivative = generalEdge[e->edge_marker].value;

    if (isPlanar)
        return derivative * int_v<Real, Scalar>(n, wt, v);
    else
        return derivative * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar general_matrix_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    int marker = e->elem_marker;

    if (isPlanar)
        return generalLabel[marker].constant * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return generalLabel[marker].constant * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar general_vector_form(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    int marker = e->elem_marker;

    if (isPlanar)
        return generalLabel[marker].rightside * int_v<Real, Scalar>(n, wt, v);
    else
        return generalLabel[marker].rightside * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

void callbackGeneralWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    wf->add_matrix_form(0, 0, callback(general_matrix_form));
    wf->add_vector_form(0, callback(general_vector_form));
    wf->add_vector_form_surf(0, callback(general_vector_form_surf));
}

// **************************************************************************************************************************

void HermesGeneral::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_General_Value:
    case PhysicFieldBC_General_Derivative:
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

LocalPointValue *HermesGeneral::localPointValue(const Point &point)
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
    return new SceneEdgeGeneralMarker(tr("new boundary"),
                                      PhysicFieldBC_General_Value,
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

SceneEdgeMarker *HermesGeneral::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneEdgeGeneralMarker *marker = dynamic_cast<SceneEdgeGeneralMarker *>(Util::scene()->getEdgeMarker(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                marker->value = Value(QString::number(value));
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

SceneLabelMarker *HermesGeneral::newLabelMarker()
{
    return new SceneLabelGeneralMarker(tr("new material"),
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

SceneLabelMarker *HermesGeneral::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double rightside, constant;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &rightside, &constant))
    {
        if (SceneLabelGeneralMarker *marker = dynamic_cast<SceneLabelGeneralMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->rightside = Value(QString::number(rightside));
            marker->constant = Value(QString::number(constant));
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

ViewScalarFilter *HermesGeneral::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);

    return new ViewScalarFilterGeneral(sln1,
                                       physicFieldVariable,
                                       physicFieldVariableComp);
}

// *******************************************************************************************************************************

QList<SolutionArray *> HermesGeneral::solve(ProgressItemSolve *progressItemSolve)
{
    // edge markers
    BCTypes bcTypes;
    BCValues bcValues;

    generalEdge = new GeneralEdge[Util::scene()->edges.count()+1];
    generalEdge[0].type = PhysicFieldBC_None;
    generalEdge[0].value = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
        {
            generalEdge[i+1].type = PhysicFieldBC_None;
            generalEdge[i+1].value = 0;
        }
        else
        {
            SceneEdgeGeneralMarker *edgeGeneralMarker = dynamic_cast<SceneEdgeGeneralMarker *>(Util::scene()->edges[i]->marker);

            // evaluate script
            if (!edgeGeneralMarker->value.evaluate()) return QList<SolutionArray *>();

            generalEdge[i+1].type = edgeGeneralMarker->type;
            generalEdge[i+1].value = edgeGeneralMarker->value.number;

            switch (edgeGeneralMarker->type)
            {
            case PhysicFieldBC_None:
                bcTypes.add_bc_none(i+1);
                break;
            case PhysicFieldBC_General_Value:
                bcTypes.add_bc_dirichlet(i+1);
                bcValues.add_const(i+1, edgeGeneralMarker->value.number);
                break;
            case PhysicFieldBC_General_Derivative:
                bcTypes.add_bc_newton(i+1);
                break;
            }
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
            if (!labelGeneralMarker->rightside.evaluate()) return QList<SolutionArray *>();
            if (!labelGeneralMarker->constant.evaluate()) return QList<SolutionArray *>();

            generalLabel[i].rightside = labelGeneralMarker->rightside.number;
            generalLabel[i].constant = labelGeneralMarker->constant.number;
        }
    }

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve,
                                                                  Hermes::vector<BCTypes *>(&bcTypes),
                                                                  Hermes::vector<BCValues *>(&bcValues),
                                                                  callbackGeneralWeakForm);

    delete [] generalEdge;
    delete [] generalLabel;

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueGeneral::LocalPointValueGeneral(const Point &point) : LocalPointValue(point)
{
    variable = 0;
    rightside = 0;
    constant = 0;
    gradient = Point();

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (labelMarker)
        {
            // value
            variable = value;

            // gradient
            gradient = derivative * (-1);

            SceneLabelGeneralMarker *marker = dynamic_cast<SceneLabelGeneralMarker *>(labelMarker);

            rightside = marker->rightside.number;
            constant = marker->constant.number;
        }
    }
}

double LocalPointValueGeneral::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        {
            return variable;
        }
        break;
    case PhysicFieldVariable_General_Gradient:
        {
            switch (physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                return gradient.x;
                break;
            case PhysicFieldVariableComp_Y:
                return gradient.y;
                break;
            case PhysicFieldVariableComp_Magnitude:
                return gradient.magnitude();
                break;
            }
        }
        break;
    case PhysicFieldVariable_General_Constant:
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
    calculate();
}

void SurfaceIntegralValueGeneral::calculateVariables(int i)
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
    calculate();
}

void VolumeIntegralValueGeneral::calculateVariables(int i)
{
}

void VolumeIntegralValueGeneral::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(0);
    sln2 = NULL;
}

QStringList VolumeIntegralValueGeneral::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterGeneral::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        {
            node->values[0][0][i] = value1[i];
        }
        break;
    case PhysicFieldVariable_General_Gradient:
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                {
                    node->values[0][0][i] = -dudx1[i];
                }
                break;
            case PhysicFieldVariableComp_Y:
                {
                    node->values[0][0][i] = -dudy1[i];
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
    case PhysicFieldVariable_General_Constant:
        {
            SceneLabelGeneralMarker *marker = dynamic_cast<SceneLabelGeneralMarker *>(labelMarker);
            node->values[0][0][i] = marker->constant.number;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterGeneral::calculateVariable()" << endl;
        throw;
        break;
    }
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
    case PhysicFieldBC_General_Value:
        out["Value"] = value.text;
        break;
    case PhysicFieldBC_General_Derivative:
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

void DSceneEdgeGeneralMarker::createContent()
{
    cmbType = new QComboBox();
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_General_Value), PhysicFieldBC_General_Value);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_General_Derivative), PhysicFieldBC_General_Derivative);

    txtValue = new ValueLineEdit(this);
    connect(txtValue, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(new QLabel(tr("BC type:")), 1, 0);
    layout->addWidget(cmbType, 1, 1);
    layout->addWidget(new QLabel(tr("Value:")), 2, 0);
    layout->addWidget(txtValue, 2, 1);
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

void DSceneLabelGeneralMarker::createContent()
{
    txtConstant = new ValueLineEdit(this);
    txtConstant->setMinimumSharp(0.0);
    txtRightSide = new ValueLineEdit(this);

    connect(txtConstant, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRightSide, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(new QLabel(tr("Constant:")), 1, 0);
    layout->addWidget(txtConstant, 1, 1);
    layout->addWidget(new QLabel(tr("Rightside:")), 2, 0);
    layout->addWidget(txtRightSide, 2, 1);
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
