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

#include "hermes_current.h"

#include "scene.h"
#include "gui.h"

class WeakFormCurrent : public WeakForm
{
public:
    WeakFormCurrent()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(Util::scene()->edges[i]->marker);

            if (edgeCurrentMarker && Util::scene()->edges[i]->marker != Util::scene()->edgeMarkers[0])
            {
                if (edgeCurrentMarker->type == PhysicFieldBC_Current_InwardCurrentFlow)
                    if (fabs(edgeCurrentMarker->value.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        edgeCurrentMarker->value.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[i]->marker);

            if (labelCurrentMarker && Util::scene()->labels[i]->marker != Util::scene()->labelMarkers[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               labelCurrentMarker->conductivity.number,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }
    }
};

// *******************************************************************************************************

void HermesCurrent::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Current_Potential:
    case PhysicFieldBC_Current_InwardCurrentFlow:
        Util::scene()->addEdgeMarker(new SceneEdgeCurrentMarker(element->attribute("name"),
                                                                type,
                                                                Value(element->attribute("value", "0"))));
        break;
    default:
        qCritical() << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type"));
        break;
    }
}

void HermesCurrent::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeCurrentMarker->type));
    element->setAttribute("value", edgeCurrentMarker->value.text);
}

void HermesCurrent::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()-> addLabelMarker(new SceneLabelCurrentMarker(element->attribute("name"),
                                                               Value(element->attribute("conductivity", "0"))));
}

void HermesCurrent::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(marker);

    element->setAttribute("conductivity", labelCurrentMarker->conductivity.text);
}

LocalPointValue *HermesCurrent::localPointValue(const Point &point)
{
    return new LocalPointValueCurrent(point);
}

QStringList HermesCurrent::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "V" << "Jx" << "Jy" << "J" << "Ex" << "Ey" << "E" << "pj" << "gamma";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesCurrent::surfaceIntegralValue()
{
    return new SurfaceIntegralValueCurrent();
}

QStringList HermesCurrent::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S" << "I";
    return QStringList(headers);
}

VolumeIntegralValue *HermesCurrent::volumeIntegralValue()
{
    return new VolumeIntegralValueCurrent();
}

QStringList HermesCurrent::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "Pj";
    return QStringList(headers);
}

SceneEdgeMarker *HermesCurrent::newEdgeMarker()
{
    return new SceneEdgeCurrentMarker(tr("new boundary condition"),
                                      PhysicFieldBC_Current_Potential,
                                      Value("0"));
}

SceneEdgeMarker *HermesCurrent::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeCurrentMarker(name,
                                          physicFieldBCFromStringKey(type),
                                          Value(QString::number(value)));
    }

    return NULL;
}

SceneEdgeMarker *HermesCurrent::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneEdgeCurrentMarker *marker = dynamic_cast<SceneEdgeCurrentMarker *>(Util::scene()->getEdgeMarker(name)))
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
}

SceneLabelMarker *HermesCurrent::newLabelMarker()
{
    return new SceneLabelCurrentMarker(tr("new material"),
                                       Value("57e6"));
}

SceneLabelMarker *HermesCurrent::newLabelMarker(PyObject *self, PyObject *args)
{
    double conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sd", &name, &conductivity))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelCurrentMarker(name,
                                           Value(QString::number(conductivity)));
    }

    return NULL;
}

SceneLabelMarker *HermesCurrent::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sd", &name, &conductivity))
    {
        if (SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->getLabelMarker(name)))
        {
            marker->conductivity = Value(QString::number(conductivity));
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

void HermesCurrent::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueCurrent *localPointValueCurrent = dynamic_cast<LocalPointValueCurrent *>(localPointValue);

    // current field
    QTreeWidgetItem *currentNode = new QTreeWidgetItem(trvWidget);
    currentNode->setText(0, tr("Current field"));
    currentNode->setExpanded(true);

    // Conductivity
    addTreeWidgetItemValue(currentNode, tr("Conductivity:"), QString("%1").arg(localPointValueCurrent->conductivity, 0, 'e', 3), "S/m");

    // Energy density
    addTreeWidgetItemValue(currentNode, tr("Power losses dens.:"), QString("%1").arg(localPointValueCurrent->losses, 0, 'e', 3), tr("W/m3"));

    // Potential
    addTreeWidgetItemValue(currentNode, tr("Potential:"), QString("%1").arg(localPointValueCurrent->potential, 0, 'f', 2), "V");

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(currentNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addTreeWidgetItemValue(itemElectricField, "E" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueCurrent->E.x, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, "E" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueCurrent->E.y, 0, 'e', 3), "V/m");
    addTreeWidgetItemValue(itemElectricField, "E:", QString("%1").arg(localPointValueCurrent->E.magnitude(), 0, 'e', 3), "V/m");

    // Current Density
    QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(currentNode);
    itemCurrentDensity->setText(0, tr("Current density"));
    itemCurrentDensity->setExpanded(true);

    addTreeWidgetItemValue(itemCurrentDensity, "J" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueCurrent->J.x, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, "J" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueCurrent->J.y, 0, 'e', 3), "A/m2");
    addTreeWidgetItemValue(itemCurrentDensity, "J:", QString("%1").arg(localPointValueCurrent->J.magnitude(), 0, 'e', 3), "A/m2");
}

void HermesCurrent::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueCurrent *surfaceIntegralValueCurrent = dynamic_cast<SurfaceIntegralValueCurrent *>(surfaceIntegralValue);

    // current field
    QTreeWidgetItem *currentNode = new QTreeWidgetItem(trvWidget);
    currentNode->setText(0, tr("Current Field"));
    currentNode->setExpanded(true);

    addTreeWidgetItemValue(currentNode, tr("Current:"), QString("%1").arg(surfaceIntegralValueCurrent->current, 0, 'e', 3), tr("A"));
}

void HermesCurrent::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueCurrent *volumeIntegralValueCurrent = dynamic_cast<VolumeIntegralValueCurrent *>(volumeIntegralValue);

    // harmonic
    QTreeWidgetItem *currentNode = new QTreeWidgetItem(trvWidget);
    currentNode->setText(0, tr("Current field"));
    currentNode->setExpanded(true);

    addTreeWidgetItemValue(currentNode, tr("Power Losses:"), QString("%1").arg(volumeIntegralValueCurrent->powerLosses, 0, 'e', 3), tr("W"));
}

ViewScalarFilter *HermesCurrent::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    return new ViewScalarFilterCurrent(sln1,
                                       physicFieldVariable,
                                       physicFieldVariableComp);
}

QList<SolutionArray *> HermesCurrent::solve(ProgressItemSolve *progressItemSolve)
{
    // edge markers
    for (int i = 1; i<Util::scene()->edgeMarkers.count(); i++)
    {
        SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(Util::scene()->edgeMarkers[i]);

        // evaluate script
        if (!edgeCurrentMarker->value.evaluate()) return QList<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->labelMarkers.count(); i++)
    {
        SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labelMarkers[i]);

        // evaluate script
        if (!labelCurrentMarker->conductivity.evaluate()) return QList<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(Util::scene()->edges[i]->marker);

        if (edgeCurrentMarker)
        {
            if (edgeCurrentMarker->type == PhysicFieldBC_Current_Potential)
                bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), edgeCurrentMarker->value.number));
        }
    }

    WeakFormCurrent wf;

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueCurrent::LocalPointValueCurrent(const Point &point) : LocalPointValue(point)
{
    conductivity = 0;

    potential = 0;
    J = Point();
    E = Point();
    losses = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (labelMarker != NULL)
        {
            // potential
            potential = value;

            // electric field
            E.x = -derivative.x;
            E.y = -derivative.y;

            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);

            conductivity = marker->conductivity.number;

            // electric displacement
            J = E * marker->conductivity.number;

            // energy density
            losses = J.magnitude() * E.magnitude();
        }
    }
}

double LocalPointValueCurrent::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Current_Potential:
        {
            return potential;
        }
        break;
    case PhysicFieldVariable_Current_ElectricField:
        {
            switch (physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                return E.x;
                break;
            case PhysicFieldVariableComp_Y:
                return E.y;
                break;
            case PhysicFieldVariableComp_Magnitude:
                return E.magnitude();
                break;
            }
        }
        break;
    case PhysicFieldVariable_Current_CurrentDensity:
        {
            switch (physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                return J.x;
                break;
            case PhysicFieldVariableComp_Y:
                return J.y;
                break;
            case PhysicFieldVariableComp_Magnitude:
                return J.magnitude();
                break;
            }
        }
        break;
    case PhysicFieldVariable_Current_Losses:
        {
            return losses;
        }
        break;
    case PhysicFieldVariable_Current_Conductivity:
        {
            return conductivity;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueCurrent::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential, 0, 'e', 5) <<
            QString("%1").arg(J.x, 0, 'e', 5) <<
            QString("%1").arg(J.y, 0, 'e', 5) <<
            QString("%1").arg(J.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(E.x, 0, 'e', 5) <<
            QString("%1").arg(E.y, 0, 'e', 5) <<
            QString("%1").arg(E.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(losses, 0, 'e', 5) <<
            QString("%1").arg(conductivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueCurrent::SurfaceIntegralValueCurrent() : SurfaceIntegralValue()
{
    current = 0.0;

    calculate();

    current /= 2.0;
}

void SurfaceIntegralValueCurrent::calculateVariables(int i)
{
    if (boundary)
    {
        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            current -= pt[i][2] * tan[i][2] * marker->conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
        else
            current -= 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
    }
}

QStringList SurfaceIntegralValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5) <<
            QString("%1").arg(current, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueCurrent::VolumeIntegralValueCurrent() : VolumeIntegralValue()
{
    powerLosses = 0.0;

    calculate();
}

void VolumeIntegralValueCurrent::calculateVariables(int i)
{
    result = 0.0;
    SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        h1_integrate_expression(marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    powerLosses += result;
}

void VolumeIntegralValueCurrent::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(0);
    sln2 = NULL;
}

QStringList VolumeIntegralValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(powerLosses, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterCurrent::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Current_Potential:
        {
            node->values[0][0][i] = value1[i];
        }
        break;
    case PhysicFieldVariable_Current_ElectricField:
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
    case PhysicFieldVariable_Current_CurrentDensity:
        {
            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);

            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
                {
                    node->values[0][0][i] = - marker->conductivity.number * dudx1[i];
                }
                break;
            case PhysicFieldVariableComp_Y:
                {
                    node->values[0][0][i] = - marker->conductivity.number * dudy1[i];
                }
                break;
            case PhysicFieldVariableComp_Magnitude:
                {
                    node->values[0][0][i] = marker->conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                }
                break;
            }
        }
        break;
    case PhysicFieldVariable_Current_Losses:
        {
            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
            node->values[0][0][i] = marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i]));
        }
        break;
    case PhysicFieldVariable_Current_Conductivity:
        {
            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
            node->values[0][0][i] = marker->conductivity.number;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterCurrent::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneEdgeCurrentMarker::SceneEdgeCurrentMarker(const QString &name, PhysicFieldBC type, Value value) : SceneEdgeMarker(name, type)
{
    this->value = value;
}

QString SceneEdgeCurrentMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneEdgeCurrentMarker::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Current_Potential:
        out["Potential (V)"] = value.text;
        break;
    case PhysicFieldBC_Current_InwardCurrentFlow:
        out["Inward current flow: (A/m2)"] = value.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneEdgeCurrentMarker::showDialog(QWidget *parent)
{
    DSceneEdgeCurrentMarker *dialog = new DSceneEdgeCurrentMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelCurrentMarker::SceneLabelCurrentMarker(const QString &name, Value conductivity) : SceneLabelMarker(name)
{
    this->conductivity = conductivity;
}

QString SceneLabelCurrentMarker::script()
{
    return QString("addmaterial(\"%1\", %3)").
            arg(name).
            arg(conductivity.text);
}

QMap<QString, QString> SceneLabelCurrentMarker::data()
{
    QMap<QString, QString> out;
    out["Conductivity (S/m)"] = conductivity.number;
    return QMap<QString, QString>(out);
}

int SceneLabelCurrentMarker::showDialog(QWidget *parent)
{
    DSceneLabelCurrentMarker *dialog = new DSceneLabelCurrentMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeCurrentMarker::DSceneEdgeCurrentMarker(SceneEdgeCurrentMarker *edgeCurrentMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeCurrentMarker;

    createDialog();

    load();
    setSize();
}

void DSceneEdgeCurrentMarker::createContent()
{
    lblValueUnit = new QLabel("");

    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Current_Potential), PhysicFieldBC_Current_Potential);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Current_InwardCurrentFlow), PhysicFieldBC_Current_InwardCurrentFlow);
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

void DSceneEdgeCurrentMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeCurrentMarker->type));
    txtValue->setValue(edgeCurrentMarker->value);
}

bool DSceneEdgeCurrentMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(m_edgeMarker);

    edgeCurrentMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        edgeCurrentMarker->value = txtValue->value();
    else
        return false;

    return true;
}

void DSceneEdgeCurrentMarker::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, (PhysicFieldBC) cmbType->itemData(index).toInt());

    // enable controls
    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_Current_Potential:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (V)").arg(QString::fromUtf8("φ")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    case PhysicFieldBC_Current_InwardCurrentFlow:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>J</i><sub>0</sub> (A/m<sup>2</sup>)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

DSceneLabelCurrentMarker::DSceneLabelCurrentMarker(QWidget *parent, SceneLabelCurrentMarker *labelCurrentMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelCurrentMarker;

    createDialog();

    load();
    setSize();
}

void DSceneLabelCurrentMarker::createContent()
{
    txtConductivity = new ValueLineEdit(this);
    txtConductivity->setMinimumSharp(0.0);
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 10, 0);
    layout->addWidget(txtConductivity, 10, 2);
}

void DSceneLabelCurrentMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(m_labelMarker);

    txtConductivity->setValue(labelCurrentMarker->conductivity);
}

bool DSceneLabelCurrentMarker::save()
{
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(m_labelMarker);

    if (txtConductivity->evaluate())
        labelCurrentMarker->conductivity  = txtConductivity->value();
    else
        return false;

    return true;
}
