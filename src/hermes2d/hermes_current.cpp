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

class WeakFormCurrent : public WeakFormAgros
{
public:
    WeakFormCurrent() : WeakFormAgros() { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryCurrent *boundary = dynamic_cast<SceneBoundaryCurrent *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == PhysicFieldBC_Current_InwardCurrentFlow)
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
            SceneMaterialCurrent *material = dynamic_cast<SceneMaterialCurrent *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               material->conductivity.number,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }
    }
};

// ****************************************************************************************************************

void ParserCurrent::setParserVariables(SceneMaterial *material)
{
    SceneMaterialCurrent *marker = dynamic_cast<SceneMaterialCurrent *>(material);

    pgamma = marker->conductivity.number;
}

// ****************************************************************************************************************

LocalPointValueCurrent::LocalPointValueCurrent(const Point &point) : LocalPointValue(point)
{
    parser = new ParserCurrent();
    initParser();

    parser->parser[0]->DefineVar("gamma", &static_cast<ParserCurrent *>(parser)->pgamma);

    calculate();
}

// ****************************************************************************************************************

SurfaceIntegralValueCurrent::SurfaceIntegralValueCurrent() : SurfaceIntegralValue()
{
    parser = new ParserCurrent();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("gamma", &static_cast<ParserCurrent *>(parser)->pgamma);
    }

    calculate();
}

// ****************************************************************************************************************

VolumeIntegralValueCurrent::VolumeIntegralValueCurrent() : VolumeIntegralValue()
{
    parser = new ParserCurrent();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("gamma", &static_cast<ParserCurrent *>(parser)->pgamma);
    }

    sln.push_back(Util::scene()->sceneSolution()->sln(0));

    calculate();
}

// *************************************************************************************************************************************

ViewScalarFilterCurrent::ViewScalarFilterCurrent(Hermes::vector<MeshFunction *> sln,
                                                             std::string expression) :
    ViewScalarFilter(sln)
{
    parser = new ParserCurrent();
    initParser(expression);

    parser->parser[0]->DefineVar("gamma", &static_cast<ParserCurrent *>(parser)->pgamma);
}

// **************************************************************************************************************************

LocalPointValue *ModuleCurrent::local_point_value(const Point &point)
{
    return new LocalPointValueCurrent(point);
}

SurfaceIntegralValue *ModuleCurrent::surface_integral_value()
{
    return new SurfaceIntegralValueCurrent();
}

VolumeIntegralValue *ModuleCurrent::volume_integral_value()
{
    return new VolumeIntegralValueCurrent();
}

ViewScalarFilter *ModuleCurrent::view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                           PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    return new ViewScalarFilterCurrent(sln1, get_expression(physicFieldVariable, physicFieldVariableComp));
}

Hermes::vector<SolutionArray *> ModuleCurrent::solve(ProgressItemSolve *progressItemSolve)
{
    // edge markers
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryCurrent *boundary = dynamic_cast<SceneBoundaryCurrent *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialCurrent *material = dynamic_cast<SceneMaterialCurrent *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->conductivity.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryCurrent *boundary = dynamic_cast<SceneBoundaryCurrent *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->type == PhysicFieldBC_Current_Potential)
                bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value.number));
        }
    }

    WeakFormCurrent wf;

    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

// *******************************************************************************************************
// rewrite

void ModuleCurrent::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Current_Potential:
    case PhysicFieldBC_Current_InwardCurrentFlow:
        Util::scene()->addBoundary(new SceneBoundaryCurrent(element->attribute("name"),
                                                            type,
                                                            Value(element->attribute("value", "0"))));
        break;
    default:
        qCritical() << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type"));
        break;
    }
}

void ModuleCurrent::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryCurrent *boundary = dynamic_cast<SceneBoundaryCurrent *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(boundary->type));
    element->setAttribute("value", boundary->value.text);
}

void ModuleCurrent::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()-> addMaterial(new SceneMaterialCurrent(element->attribute("name"),
                                                         Value(element->attribute("conductivity", "0"))));
}

void ModuleCurrent::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialCurrent *material = dynamic_cast<SceneMaterialCurrent *>(marker);

    element->setAttribute("conductivity", material->conductivity.text);
}

SceneBoundary *ModuleCurrent::newBoundary()
{
    return new SceneBoundaryCurrent(tr("new boundary condition"),
                                    PhysicFieldBC_Current_Potential,
                                    Value("0"));
}

SceneBoundary *ModuleCurrent::newBoundary(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryCurrent(name,
                                        physicFieldBCFromStringKey(type),
                                        Value(QString::number(value)));
    }

    return NULL;
}

SceneBoundary *ModuleCurrent::modifyBoundary(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneBoundaryCurrent *marker = dynamic_cast<SceneBoundaryCurrent *>(Util::scene()->getBoundary(name)))
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

SceneMaterial *ModuleCurrent::newMaterial()
{
    return new SceneMaterialCurrent(tr("new material"),
                                    Value("57e6"));
}

SceneMaterial *ModuleCurrent::newMaterial(PyObject *self, PyObject *args)
{
    double conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sd", &name, &conductivity))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialCurrent(name,
                                        Value(QString::number(conductivity)));
    }

    return NULL;
}

SceneMaterial *ModuleCurrent::modifyMaterial(PyObject *self, PyObject *args)
{
    double conductivity;
    char *name;
    if (PyArg_ParseTuple(args, "sd", &name, &conductivity))
    {
        if (SceneMaterialCurrent *marker = dynamic_cast<SceneMaterialCurrent *>(Util::scene()->getMaterial(name)))
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

// ****************************************************************************************************************

SceneBoundaryCurrent::SceneBoundaryCurrent(const QString &name, PhysicFieldBC type, Value value) : SceneBoundary(name, type)
{
    this->value = value;
}

QString SceneBoundaryCurrent::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneBoundaryCurrent::data()
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

int SceneBoundaryCurrent::showDialog(QWidget *parent)
{
    SceneBoundaryCurrentDialog *dialog = new SceneBoundaryCurrentDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialCurrent::SceneMaterialCurrent(const QString &name, Value conductivity) : SceneMaterial(name)
{
    this->conductivity = conductivity;
}

QString SceneMaterialCurrent::script()
{
    return QString("addmaterial(\"%1\", %3)").
            arg(name).
            arg(conductivity.text);
}

QMap<QString, QString> SceneMaterialCurrent::data()
{
    QMap<QString, QString> out;
    out["Conductivity (S/m)"] = conductivity.number;
    return QMap<QString, QString>(out);
}

int SceneMaterialCurrent::showDialog(QWidget *parent)
{
    SceneMaterialCurrentDialog *dialog = new SceneMaterialCurrentDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryCurrentDialog::SceneBoundaryCurrentDialog(SceneBoundaryCurrent *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryCurrentDialog::createContent()
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

void SceneBoundaryCurrentDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryCurrent *boundary = dynamic_cast<SceneBoundaryCurrent *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(boundary->type));
    txtValue->setValue(boundary->value);
}

bool SceneBoundaryCurrentDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    SceneBoundaryCurrent *boundary = dynamic_cast<SceneBoundaryCurrent *>(m_boundary);

    boundary->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        boundary->value = txtValue->value();
    else
        return false;

    return true;
}

void SceneBoundaryCurrentDialog::doTypeChanged(int index)
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

SceneMaterialCurrentDialog::SceneMaterialCurrentDialog(SceneMaterialCurrent *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialCurrentDialog::createContent()
{
    txtConductivity = new ValueLineEdit(this);
    txtConductivity->setMinimumSharp(0.0);
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 10, 0);
    layout->addWidget(txtConductivity, 10, 2);
}

void SceneMaterialCurrentDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialCurrent *material = dynamic_cast<SceneMaterialCurrent *>(m_material);

    txtConductivity->setValue(material->conductivity);
}

bool SceneMaterialCurrentDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialCurrent *material = dynamic_cast<SceneMaterialCurrent *>(m_material);

    if (txtConductivity->evaluate())
        material->conductivity  = txtConductivity->value();
    else
        return false;

    return true;
}
