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

#include "hermes_electrostatic.h"

#include "scene.h"
#include "gui.h"

class WeakFormElectrostatics : public WeakFormAgros
{
public:
    WeakFormElectrostatics() : WeakFormAgros() { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryElectrostatic *boundaryHeat = dynamic_cast<SceneBoundaryElectrostatic *>(Util::scene()->edges[i]->boundary);

            if (boundaryHeat && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundaryHeat->type == PhysicFieldBC_Electrostatic_SurfaceCharge)
                    if (fabs(boundaryHeat->value.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundaryHeat->value.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialElectrostatic *materialHeat = dynamic_cast<SceneMaterialElectrostatic *>(Util::scene()->labels[i]->material);

            if (materialHeat && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               materialHeat->permittivity.number * EPS0,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));
                if (fabs(materialHeat->charge_density.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   materialHeat->charge_density.number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }
    }
};

// **************************************************************************************************************************

void HermesElectrostatic::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Electrostatic_Potential:
    case PhysicFieldBC_Electrostatic_SurfaceCharge:
        Util::scene()->addBoundary(new SceneBoundaryElectrostatic(element->attribute("name"),
                                                                  type,
                                                                  Value(element->attribute("value", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesElectrostatic::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryElectrostatic *material = dynamic_cast<SceneBoundaryElectrostatic *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(material->type));
    element->setAttribute("value", material->value.text);
}

void HermesElectrostatic::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialElectrostatic(element->attribute("name"),
                                                              Value(element->attribute("charge_density", "0")),
                                                              Value(element->attribute("permittivity", "1"))));
}

void HermesElectrostatic::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialElectrostatic *material = dynamic_cast<SceneMaterialElectrostatic *>(marker);

    element->setAttribute("charge_density", material->charge_density.text);
    element->setAttribute("permittivity", material->permittivity.text);
}

LocalPointValue *HermesElectrostatic::localPointValue(const Point &point)
{
    return new LocalPointValueElectrostatic(point);
}

QStringList HermesElectrostatic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "V" << "Ex" << "Ey" << "E" << "Dx" << "Dy" << "D" << "we" << "epsr";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesElectrostatic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueElectrostatic();
}

QStringList HermesElectrostatic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S" << "Q";
    return QStringList(headers);
}

VolumeIntegralValue *HermesElectrostatic::volumeIntegralValue()
{
    return new VolumeIntegralValueElectrostatic();
}

QStringList HermesElectrostatic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "We";
    return QStringList(headers);
}

SceneBoundary *HermesElectrostatic::newBoundary()
{
    return new SceneBoundaryElectrostatic(tr("new boundary"),
                                          PhysicFieldBC_Electrostatic_Potential,
                                          Value("0"));
}

SceneBoundary *HermesElectrostatic::newBoundary(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryElectrostatic(name,
                                              physicFieldBCFromStringKey(type),
                                              Value(QString::number(value)));
    }

    return NULL;
}

SceneBoundary *HermesElectrostatic::modifyBoundary(PyObject *self, PyObject *args)
{
    double value;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &value))
    {
        if (SceneBoundaryElectrostatic *marker = dynamic_cast<SceneBoundaryElectrostatic *>(Util::scene()->getBoundary(name)))
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

SceneMaterial *HermesElectrostatic::newMaterial()
{
    return new SceneMaterialElectrostatic(tr("new material"),
                                          Value("0"),
                                          Value("1"));
}

SceneMaterial *HermesElectrostatic::newMaterial(PyObject *self, PyObject *args)
{
    double charge_density, permittivity;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &charge_density, &permittivity))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialElectrostatic(name,
                                              Value(QString::number(charge_density)),
                                              Value(QString::number(permittivity)));
    }

    return NULL;
}

SceneMaterial *HermesElectrostatic::modifyMaterial(PyObject *self, PyObject *args)
{
    double charge_density, permittivity;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &charge_density, &permittivity))
    {
        if (SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(Util::scene()->getMaterial(name)))
        {
            marker->charge_density = Value(QString::number(charge_density));
            marker->permittivity = Value(QString::number(permittivity));
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

ViewScalarFilter *HermesElectrostatic::viewScalarFilter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                        PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    return new ViewScalarFilterElectrostatic(sln1,
                                             Hermes::Module::Module::get_expression(physicFieldVariable,
                                                                                    physicFieldVariableComp));
}

// *******************************************************************************************************************************

QList<SolutionArray *> HermesElectrostatic::solve(ProgressItemSolve *progressItemSolve)
{
    // boundaries
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryElectrostatic *boundary = dynamic_cast<SceneBoundaryElectrostatic *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value.evaluate()) return QList<SolutionArray *>();
    }

    // materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialElectrostatic *material = dynamic_cast<SceneMaterialElectrostatic *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->charge_density.evaluate()) return QList<SolutionArray *>();
        if (!material->permittivity.evaluate()) return QList<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryElectrostatic *boundary = dynamic_cast<SceneBoundaryElectrostatic *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->type == PhysicFieldBC_Electrostatic_Potential)
                bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value.number));
        }
    }

    WeakFormElectrostatics wf;

    // QTime time;
    // time.start();
    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);
    // qDebug() << "solveSolutioArray: " << time.elapsed();

    return solutionArrayList;
}

// ****************************************************************************************************************

LocalPointValueElectrostatic::LocalPointValueElectrostatic(const Point &point) : LocalPointValue(point)
{      
    calculate();
}

void LocalPointValueElectrostatic::prepareParser(SceneMaterial *material, mu::Parser *parser)
{
    SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(material);

    parser->DefineVar("epsr", &marker->permittivity.number);
    parser->DefineVar("rho", &marker->charge_density.number);
}

double LocalPointValueElectrostatic::variableValue(PhysicFieldVariableDeprecated physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Electrostatic_Potential:
    {
        return potential;
    }
        break;
    case PhysicFieldVariable_Electrostatic_ElectricField:
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
    case PhysicFieldVariable_Electrostatic_Displacement:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return D.x;
            break;
        case PhysicFieldVariableComp_Y:
            return D.y;
            break;
        case PhysicFieldVariableComp_Magnitude:
            return D.magnitude();
            break;
        }
    }
        break;
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
    {
        return we;
    }
        break;
    case PhysicFieldVariable_Electrostatic_Permittivity:
    {
        return permittivity;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueElectrostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueElectrostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(potential, 0, 'e', 5) <<
           QString("%1").arg(E.x, 0, 'e', 5) <<
           QString("%1").arg(E.y, 0, 'e', 5) <<
           QString("%1").arg(E.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(D.x, 0, 'e', 5) <<
           QString("%1").arg(D.y, 0, 'e', 5) <<
           QString("%1").arg(D.magnitude(), 0, 'e', 5) <<
           QString("%1").arg(we, 0, 'e', 5) <<
           QString("%1").arg(permittivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueElectrostatic::SurfaceIntegralValueElectrostatic() : SurfaceIntegralValue()
{
    calculate();
}

void SurfaceIntegralValueElectrostatic::initSolutions()
{
    // sln1 = Util::scene()->sceneSolution()->sln(0);
    // sln2 = NULL;

    for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("epsr", &pepsr);
        ((mu::Parser *) *it)->DefineVar("rho", &prho);
    }
}

void SurfaceIntegralValueElectrostatic::prepareParser(SceneMaterial *material)
{
    SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(material);

    pepsr = marker->permittivity.number;
    prho = marker->charge_density.number;
}

/*
if (boundary)
{
    SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(material);

    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        surfaceCharge += pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx1[i] - tan[i][0] * dudy1[i]);
    else
        surfaceCharge += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx1[i] - tan[i][0] * dudy1[i]);
}
*/

QStringList SurfaceIntegralValueElectrostatic::variables()
{
    /*
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
           QString("%1").arg(surface, 0, 'e', 5) <<
           QString("%1").arg(surfaceCharge, 0, 'e', 5);
    return QStringList(row);
    */
}

// ****************************************************************************************************************

VolumeIntegralValueElectrostatic::VolumeIntegralValueElectrostatic() : VolumeIntegralValue()
{
    calculate();
}

/*
void VolumeIntegralValueElectrostatic::calculateVariables(int i)
{
    SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(material);

    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        h1_integrate_expression(0.5 * EPS0 * marker->permittivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * 0.5 * EPS0 * marker->permittivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
    }
    energy += result;
}
*/

void VolumeIntegralValueElectrostatic::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(0);
    sln2 = NULL;

    for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("epsr", &pepsr);
        ((mu::Parser *) *it)->DefineVar("rho", &prho);
    }
}

void VolumeIntegralValueElectrostatic::prepareParser(SceneMaterial *material)
{
    SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(material);

    pepsr = marker->permittivity.number;
    prho = marker->charge_density.number;
}

QStringList VolumeIntegralValueElectrostatic::variables()
{
    /*
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
           QString("%1").arg(crossSection, 0, 'e', 5) <<
           QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
    */
}

// *************************************************************************************************************************************

ViewScalarFilterElectrostatic::ViewScalarFilterElectrostatic(Hermes::vector<MeshFunction *> sln,
                                                             std::string expression) :
    ViewScalarFilter(sln, expression)
{
    parser.DefineVar("epsr", &pepsr);
    parser.DefineVar("rho", &prho);
}

void ViewScalarFilterElectrostatic::prepareParser(SceneMaterial *material)
{
    SceneMaterialElectrostatic *marker = dynamic_cast<SceneMaterialElectrostatic *>(material);

    pepsr = marker->permittivity.number;
    prho = marker->charge_density.number;
}

// *************************************************************************************************************************************

SceneBoundaryElectrostatic::SceneBoundaryElectrostatic(const QString &name, PhysicFieldBC type, Value value)
    : SceneBoundary(name, type)
{
    this->value = value;
}

QString SceneBoundaryElectrostatic::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value.text);
}

QMap<QString, QString> SceneBoundaryElectrostatic::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Electrostatic_Potential:
        out["Potential (V)"] = value.text;
        break;
    case PhysicFieldBC_Electrostatic_SurfaceCharge:
        out["Surface charge density (C/m3)"] = value.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneBoundaryElectrostatic::showDialog(QWidget *parent)
{
    SceneBoundaryElectrostaticDialog *dialog = new SceneBoundaryElectrostaticDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialElectrostatic::SceneMaterialElectrostatic(const QString &name, Value charge_density, Value permittivity)
    : SceneMaterial(name)
{
    this->charge_density = charge_density;
    this->permittivity = permittivity;
}

QString SceneMaterialElectrostatic::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(charge_density.text).
            arg(permittivity.text);
}

QMap<QString, QString> SceneMaterialElectrostatic::data()
{
    QMap<QString, QString> out;
    out["Charge density (C/m3)"] = charge_density.text;
    out["Permittivity (-)"] = permittivity.text;
    return QMap<QString, QString>(out);
}

int SceneMaterialElectrostatic::showDialog(QWidget *parent)
{
    SceneMaterialElectrostaticDialog *dialog = new SceneMaterialElectrostaticDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryElectrostaticDialog::SceneBoundaryElectrostaticDialog(SceneBoundaryElectrostatic *material, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = material;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryElectrostaticDialog::createContent()
{
    lblValueUnit = new QLabel("");

    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Electrostatic_Potential), PhysicFieldBC_Electrostatic_Potential);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Electrostatic_SurfaceCharge), PhysicFieldBC_Electrostatic_SurfaceCharge);
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

void SceneBoundaryElectrostaticDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryElectrostatic *material = dynamic_cast<SceneBoundaryElectrostatic *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(material->type));
    txtValue->setValue(material->value);
}

bool SceneBoundaryElectrostaticDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;

    SceneBoundaryElectrostatic *material = dynamic_cast<SceneBoundaryElectrostatic *>(m_boundary);

    material->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        material->value  = txtValue->value();
    else
        return false;

    return true;
}

void SceneBoundaryElectrostaticDialog::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, (PhysicFieldBC) cmbType->itemData(index).toInt());

    // enable controls
    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_Electrostatic_Potential:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (V)").arg(QString::fromUtf8("φ")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    case PhysicFieldBC_Electrostatic_SurfaceCharge:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>%1</i><sub>0</sub> (C/m<sup>2</sup>)").arg(QString::fromUtf8("σ")));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialElectrostaticDialog::SceneMaterialElectrostaticDialog(SceneMaterialElectrostatic *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialElectrostaticDialog::createContent()
{
    txtPermittivity = new ValueLineEdit(this);
    txtPermittivity->setMinimumSharp(0.0);
    txtChargeDensity = new ValueLineEdit(this);
    connect(txtPermittivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtChargeDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("ε")),
                                  tr("Permittivity")), 10, 0);
    layout->addWidget(txtPermittivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (C/m<sup>3</sup>)").arg(QString::fromUtf8("ρ")),
                                  tr("Charge density")), 11, 0);
    layout->addWidget(txtChargeDensity, 11, 2);
}

void SceneMaterialElectrostaticDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialElectrostatic *material = dynamic_cast<SceneMaterialElectrostatic *>(m_material);

    txtPermittivity->setValue(material->permittivity);
    txtChargeDensity->setValue(material->charge_density);
}

bool SceneMaterialElectrostaticDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialElectrostatic *material = dynamic_cast<SceneMaterialElectrostatic *>(m_material);

    if (txtPermittivity->evaluate())
        material->permittivity = txtPermittivity->value();
    else
        return false;

    if (txtChargeDensity->evaluate())
        material->charge_density = txtChargeDensity->value();
    else
        return false;

    return true;
}
