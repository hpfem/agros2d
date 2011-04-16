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

class WeakFormGenerals : public WeakFormAgros
{
public:
    WeakFormGenerals() : WeakFormAgros() { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryGeneral *boundary = dynamic_cast<SceneBoundaryGeneral *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == PhysicFieldBC_General_Derivative)
                    if (fabs(boundary->value.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->value.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialGeneral *material = dynamic_cast<SceneMaterialGeneral *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               material->constant.number * EPS0,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));
                if (fabs(material->rightside.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->rightside.number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }

    }
};

// **************************************************************************************************************************

void HermesGeneral::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_General_Value:
    case PhysicFieldBC_General_Derivative:
        Util::scene()->addBoundary(new SceneBoundaryGeneral(element->attribute("name"),
                                                                type,
                                                                Value(element->attribute("value", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesGeneral::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryGeneral *boundary = dynamic_cast<SceneBoundaryGeneral *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(boundary->type));
    element->setAttribute("value", boundary->value.text);
}

void HermesGeneral::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialGeneral(element->attribute("name"),
                                                              Value(element->attribute("rightside", "0")),
                                                              Value(element->attribute("constant", "0"))));
}

void HermesGeneral::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialGeneral *material = dynamic_cast<SceneMaterialGeneral *>(marker);

    element->setAttribute("rightside", material->rightside.text);
    element->setAttribute("constant", material->constant.text);
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

SceneBoundary *HermesGeneral::newBoundary()
{
    return new SceneBoundaryGeneral(tr("new boundary"),
                                      PhysicFieldBC_General_Value,
                                      Value("0"));
}

SceneBoundary *HermesGeneral::newBoundary(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryGeneral(name,
                                          physicFieldBCFromStringKey(type),
                                          Value(QString::number(value)));
    }

    return NULL;
}

SceneBoundary *HermesGeneral::modifyBoundary(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneBoundaryGeneral *marker = dynamic_cast<SceneBoundaryGeneral *>(Util::scene()->getBoundary(name)))
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

SceneMaterial *HermesGeneral::newMaterial()
{
    return new SceneMaterialGeneral(tr("new material"),
                                       Value("0"),
                                       Value("1"));
}

SceneMaterial *HermesGeneral::newMaterial(PyObject *self, PyObject *args)
{
    double rightside, constant;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &rightside, &constant))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialGeneral(name,
                                           Value(QString::number(rightside)),
                                           Value(QString::number(constant)));
    }

    return NULL;
}

SceneMaterial *HermesGeneral::modifyMaterial(PyObject *self, PyObject *args)
{
    double rightside, constant;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &rightside, &constant))
    {
        if (SceneMaterialGeneral *marker = dynamic_cast<SceneMaterialGeneral *>(Util::scene()->getMaterial(name)))
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
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryGeneral *boundary = dynamic_cast<SceneBoundaryGeneral *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value.evaluate()) return QList<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialGeneral *material = dynamic_cast<SceneMaterialGeneral *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->rightside.evaluate()) return QList<SolutionArray *>();
        if (!material->constant.evaluate()) return QList<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryGeneral *boundary = dynamic_cast<SceneBoundaryGeneral *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->type == PhysicFieldBC_General_Value)
                bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value.number));
        }
    }

    WeakFormGenerals wf;

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

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
        if (material)
        {
            // value
            variable = value;

            // gradient
            gradient = derivative * (-1);

            SceneMaterialGeneral *marker = dynamic_cast<SceneMaterialGeneral *>(material);

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
        SceneMaterialGeneral *marker = dynamic_cast<SceneMaterialGeneral *>(material);
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

SceneBoundaryGeneral::SceneBoundaryGeneral(const QString &name, PhysicFieldBC type, Value value)
    : SceneBoundary(name, type)
{
    this->value = value;
}

QString SceneBoundaryGeneral::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneBoundaryGeneral::data()
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

int SceneBoundaryGeneral::showDialog(QWidget *parent)
{
    SceneBoundaryGeneralDialog *dialog = new SceneBoundaryGeneralDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialGeneral::SceneMaterialGeneral(const QString &name, Value rightside, Value constant)
    : SceneMaterial(name)
{
    this->rightside = rightside;
    this->constant = constant;
}

QString SceneMaterialGeneral::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(rightside.text).
            arg(constant.text);
}

QMap<QString, QString> SceneMaterialGeneral::data()
{
    QMap<QString, QString> out;
    out["Rightside"] = rightside.text;
    out["Constant"] = constant.text;
    return QMap<QString, QString>(out);
}

int SceneMaterialGeneral::showDialog(QWidget *parent)
{
    SceneMaterialGeneralDialog *dialog = new SceneMaterialGeneralDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryGeneralDialog::SceneBoundaryGeneralDialog(SceneBoundaryGeneral *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    // tab order
    setTabOrder(txtName, cmbType);
    setTabOrder(cmbType, txtValue);

    load();
    setSize();
}

void SceneBoundaryGeneralDialog::createContent()
{
    lblValueUnit = new QLabel("");

    cmbType = new QComboBox();
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_General_Value), PhysicFieldBC_General_Value);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_General_Derivative), PhysicFieldBC_General_Derivative);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValue = new ValueLineEdit(this);
    connect(txtValue, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnit, 11, 0);
    layout->addWidget(txtValue, 11, 2);
}

void SceneBoundaryGeneralDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryGeneral *boundary = dynamic_cast<SceneBoundaryGeneral *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(boundary->type));
    txtValue->setValue(boundary->value);
}

bool SceneBoundaryGeneralDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;

    SceneBoundaryGeneral *boundary = dynamic_cast<SceneBoundaryGeneral *>(m_boundary);

    boundary->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        boundary->value  = txtValue->value();
    else
        return false;

    return true;
}

void SceneBoundaryGeneralDialog::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, (PhysicFieldBC) cmbType->itemData(index).toInt());

    // enable controls
    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_General_Value:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>u</i><sub>0</sub> (-)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    case PhysicFieldBC_General_Derivative:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>g</i><sub>0</sub> (-)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialGeneralDialog::SceneMaterialGeneralDialog(SceneMaterialGeneral *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    // tab order
    setTabOrder(txtName, txtConstant);
    setTabOrder(txtConstant, txtRightSide);

    load();
    setSize();
}

void SceneMaterialGeneralDialog::createContent()
{
    txtConstant = new ValueLineEdit(this);
    txtConstant->setMinimumSharp(0.0);
    txtRightSide = new ValueLineEdit(this);

    connect(txtConstant, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRightSide, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>c</i> (-)"),
                                  tr("Constant")), 10, 0);
    layout->addWidget(txtConstant, 10, 2);
    layout->addWidget(createLabel(tr("<i>r</i> (-)"),
                                  tr("Rightside")), 11, 0);
    layout->addWidget(txtRightSide, 11, 2);
}

void SceneMaterialGeneralDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialGeneral *material = dynamic_cast<SceneMaterialGeneral *>(m_material);

    txtConstant->setValue(material->constant);
    txtRightSide->setValue(material->rightside);
}

bool SceneMaterialGeneralDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialGeneral *material = dynamic_cast<SceneMaterialGeneral *>(m_material);

    if (txtConstant->evaluate())
        material->constant = txtConstant->value();
    else
        return false;

    if (txtRightSide->evaluate())
        material->rightside = txtRightSide->value();
    else
        return false;

    return true;
}
