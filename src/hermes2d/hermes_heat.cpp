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

#include "hermes_heat.h"

#include "scene.h"
#include "gui.h"

class WeakFormHeatTransfer : public WeakFormAgros
{
public:
    WeakFormHeatTransfer() : WeakFormAgros() { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == "heat_heat_flux")
                {
                    // vector flux term


                    double flux = boundary->get_value("heat_heat_flux").number +
                            boundary->get_value("heat_heat_transfer_coefficient").number * boundary->get_value("heat_external_temperature").number;

                    if (fabs(flux) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        flux,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                    if (fabs(boundary->get_value("heat_heat_transfer_coefficient").number) > EPS_ZERO)
                        add_matrix_form_surf(new WeakFormsH1::SurfaceMatrixForms::DefaultMatrixFormSurf(0, 0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->get_value("heat_heat_transfer_coefficient").number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               material->thermal_conductivity.number,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));

                if (fabs(material->volume_heat.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->volume_heat.number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));

                // transient analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                {
                    if ((fabs(material->density.number) > EPS_ZERO) && (fabs(material->specific_heat.number) > EPS_ZERO))
                    {
                        if (solution.size() > 0)
                        {
                            add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                                      QString::number(i).toStdString(),
                                                                                                      material->density.number * material->specific_heat.number / Util::scene()->problemInfo()->timeStep.number,
                                                                                                      HERMES_SYM,
                                                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));

                            add_vector_form(new CustomVectorFormTimeDep(0,
                                                                        QString::number(i).toStdString(),
                                                                        material->density.number * material->specific_heat.number / Util::scene()->problemInfo()->timeStep.number,
                                                                        solution[0],
                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                        }
                    }
                }
            }
        }
    }
};

// ****************************************************************************************************************

void ParserHeat::setParserVariables(SceneMaterial *material)
{
    SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(material);

    plambda = marker->thermal_conductivity.number;
    prho = marker->density.number;
    pcp = marker->specific_heat.number;
    pq = marker->volume_heat.number;
}

// ****************************************************************************************************************

LocalPointValueHeat::LocalPointValueHeat(const Point &point) : LocalPointValue(point)
{
    parser = new ParserHeat();
    initParser();

    parser->parser[0]->DefineVar("lambda", &static_cast<ParserHeat *>(parser)->plambda);
    parser->parser[0]->DefineVar("rho", &static_cast<ParserHeat *>(parser)->prho);
    parser->parser[0]->DefineVar("cp", &static_cast<ParserHeat *>(parser)->pcp);
    parser->parser[0]->DefineVar("q", &static_cast<ParserHeat *>(parser)->pq);

    calculate();
}

// ****************************************************************************************************************

SurfaceIntegralValueHeat::SurfaceIntegralValueHeat() : SurfaceIntegralValue()
{
    parser = new ParserHeat();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("lambda", &static_cast<ParserHeat *>(parser)->plambda);
        ((mu::Parser *) *it)->DefineVar("rho", &static_cast<ParserHeat *>(parser)->prho);
        ((mu::Parser *) *it)->DefineVar("cp", &static_cast<ParserHeat *>(parser)->pcp);
        ((mu::Parser *) *it)->DefineVar("q", &static_cast<ParserHeat *>(parser)->pq);
    }

    calculate();
}

// ****************************************************************************************************************

VolumeIntegralValueHeat::VolumeIntegralValueHeat() : VolumeIntegralValue()
{
    parser = new ParserHeat();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("lambda", &static_cast<ParserHeat *>(parser)->plambda);
        ((mu::Parser *) *it)->DefineVar("rho", &static_cast<ParserHeat *>(parser)->prho);
        ((mu::Parser *) *it)->DefineVar("cp", &static_cast<ParserHeat *>(parser)->pcp);
        ((mu::Parser *) *it)->DefineVar("q", &static_cast<ParserHeat *>(parser)->pq);
    }

    sln.push_back(Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep()));

    calculate();
}

// *************************************************************************************************************************************

ViewScalarFilterHeat::ViewScalarFilterHeat(Hermes::vector<MeshFunction *> sln,
                                           std::string expression) :
    ViewScalarFilter(sln)
{
    parser = new ParserHeat();
    initParser(expression);

    parser->parser[0]->DefineVar("lambda", &static_cast<ParserHeat *>(parser)->plambda);
    parser->parser[0]->DefineVar("rho", &static_cast<ParserHeat *>(parser)->prho);
    parser->parser[0]->DefineVar("cp", &static_cast<ParserHeat *>(parser)->pcp);
    parser->parser[0]->DefineVar("q", &static_cast<ParserHeat *>(parser)->pq);
}

// **************************************************************************************************************************

LocalPointValue *ModuleHeat::local_point_value(const Point &point)
{
    return new LocalPointValueHeat(point);
}

SurfaceIntegralValue *ModuleHeat::surface_integral_value()
{
    return new SurfaceIntegralValueHeat();
}

VolumeIntegralValue *ModuleHeat::volume_integral_value()
{
    return new VolumeIntegralValueHeat();
}

ViewScalarFilter *ModuleHeat::view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                 PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep());
    return new ViewScalarFilterHeat(sln1, get_expression(physicFieldVariable, physicFieldVariableComp));
}

// *******************************************************************************************************

Hermes::vector<SolutionArray *> ModuleHeat::solve(ProgressItemSolve *progressItemSolve)
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // edge markers
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->boundaries[i];

        // evaluate script
        for (std::map<Hermes::Module::BoundaryTypeVariable *, Value>::iterator it = boundary->values.begin(); it != boundary->values.end(); ++it)
            if (!it->second.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->thermal_conductivity.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->density.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->specific_heat.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->volume_heat.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bcs;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary)
        {
            if (boundary->type == "heat_temperature")
                bcs.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                       boundary->get_value("heat_temperature").number));
        }
    }

    WeakFormHeatTransfer wf;

    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

void ModuleHeat::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(Util::scene()->materials[i]);
        material->volume_heat.evaluate(time);
    }
}

// *************************************************************************************************************************************
// rewrite

void ModuleHeat::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialHeat(element->attribute("name"),
                                                     Value(element->attribute("volume_heat", "0")),
                                                     Value(element->attribute("thermal_conductivity", "0")),
                                                     Value(element->attribute("density", "0")),
                                                     Value(element->attribute("specific_heat", "0"))));
}

void ModuleHeat::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(marker);

    element->setAttribute("thermal_conductivity", material->thermal_conductivity.text);
    element->setAttribute("volume_heat", material->volume_heat.text);
    element->setAttribute("density", material->density.text);
    element->setAttribute("specific_heat", material->specific_heat.text);
}

SceneBoundary *ModuleHeat::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "heat_temperature");
}

SceneBoundary *ModuleHeat::newBoundary(PyObject *self, PyObject *args)
{
    /*
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Temperature)
            return new SceneBoundaryHeat(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(value)));
        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Flux)
            return new SceneBoundaryHeat(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(value)),
                                         Value(QString::number(h)),
                                         Value(QString::number(externaltemperature)));
    }

    return NULL;
    */
}

SceneBoundary *ModuleHeat::modifyBoundary(PyObject *self, PyObject *args)
{
    /*
    double value, h, externaltemperature;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|dd", &name, &type, &value, &h, &externaltemperature))
    {
        if (SceneBoundaryHeat *marker = dynamic_cast<SceneBoundaryHeat *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' is not supported.").arg(type).toStdString().c_str());
                return NULL;
            }

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Temperature)
                marker->temperature = Value(QString::number(value));

            if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Heat_Flux)
            {
                marker->heatFlux = Value(QString::number(value));
                marker->h = Value(QString::number(h));
                marker->externalTemperature = Value(QString::number(externaltemperature));
                return marker;
            }
        }
        else
        {
            PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary marker with name '%1' doesn't exists.").arg(name).toStdString().c_str());
            return NULL;
        }
    }

    return NULL;
    */
}

SceneMaterial *ModuleHeat::newMaterial()
{
    return new SceneMaterialHeat(tr("new material"),
                                 Value("0"),
                                 Value("385"),
                                 Value("0"),
                                 Value("0"));
}

SceneMaterial *ModuleHeat::newMaterial(PyObject *self, PyObject *args)
{
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialHeat(name,
                                     Value(QString::number(volume_heat)),
                                     Value(QString::number(thermal_conductivity)),
                                     Value(QString::number(density)),
                                     Value(QString::number(specific_heat)));
    }

    return NULL;
}

SceneMaterial *ModuleHeat::modifyMaterial(PyObject *self, PyObject *args)
{
    double volume_heat, thermal_conductivity, density, specific_heat;
    char *name;
    if (PyArg_ParseTuple(args, "sdddd", &name, &volume_heat, &thermal_conductivity, &density, &specific_heat))
    {
        if (SceneMaterialHeat *marker = dynamic_cast<SceneMaterialHeat *>(Util::scene()->getMaterial(name)))
        {
            marker->volume_heat = Value(QString::number(volume_heat));
            marker->thermal_conductivity = Value(QString::number(thermal_conductivity));
            marker->density = Value(QString::number(density));
            marker->specific_heat = Value(QString::number(specific_heat));
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

/*
void VolumeIntegralValueHeat::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep());
    sln2 = NULL;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() - 1);
}
*/

// *************************************************************************************************************************************

SceneMaterialHeat::SceneMaterialHeat(const QString &name, Value volume_heat, Value thermal_conductivity, Value density, Value specific_heat)
    : SceneMaterial(name)
{
    this->thermal_conductivity = thermal_conductivity;
    this->volume_heat = volume_heat;
    this->density = density;
    this->specific_heat = specific_heat;
}

QString SceneMaterialHeat::script()
{
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState)
        return QString("addmaterial(\"%1\", %2, %3, %4, %5)").
                arg(name).
                arg(volume_heat.text).
                arg(thermal_conductivity.text).
                arg(density.text).
                arg(specific_heat.text);
    else
        return QString("addmaterial(\"%1\", \"%2\", %3, %4, %5)").
                arg(name).
                arg(volume_heat.text).
                arg(thermal_conductivity.text).
                arg(density.text).
                arg(specific_heat.text);
}

QMap<QString, QString> SceneMaterialHeat::data()
{
    QMap<QString, QString> out;
    out["Volume heat (W/m3)"] = volume_heat.text;
    out["Thermal conductivity (W/m.K)"] = thermal_conductivity.text;
    out["Density (kg/m3)"] = density.text;
    out["Specific heat (J/kg.K)"] = specific_heat.text;
    return QMap<QString, QString>(out);
}

int SceneMaterialHeat::showDialog(QWidget *parent)
{
    SceneMaterialHeatDialog *dialog = new SceneMaterialHeatDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryHeatDialog::SceneBoundaryHeatDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryHeatDialog::createContent()
{
    cmbType = new QComboBox(this);
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtHeatFlux = new ValueLineEdit(this);
    txtTemperature = new ValueLineEdit(this);
    txtHeatTransferCoefficient = new ValueLineEdit(this);
    txtExternalTemperature = new ValueLineEdit(this);

    connect(txtHeatFlux, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtHeatTransferCoefficient, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtExternalTemperature, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());
    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>0</sub> (K)"),
                                  tr("Temperature")), 11, 0);
    layout->addWidget(txtTemperature, 11, 2);
    layout->addWidget(createLabel(tr("<i>f</i><sub>0</sub> (W/m<sup>2</sup>)"),
                                  tr("Heat flux")), 12, 0);
    layout->addWidget(txtHeatFlux, 12, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (W/m<sup>2</sup>·K)").arg(QString::fromUtf8("α")),
                                  tr("Heat transfer coef.")), 13, 0);
    layout->addWidget(txtHeatTransferCoefficient, 13, 2);
    layout->addWidget(createLabel(tr("<i>T</i><sub>ext</sub> (K)"),
                                  tr("External temperature")), 14, 0);
    layout->addWidget(txtExternalTemperature, 14, 2);

}

void SceneBoundaryHeatDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "heat_temperature")
    {
        txtTemperature->setValue(m_boundary->get_value("heat_temperature"));
    }
    else if (m_boundary->type == "heat_heat_flux")
    {
        txtHeatFlux->setValue(m_boundary->get_value("heat_heat_flux"));
        txtHeatTransferCoefficient->setValue(m_boundary->get_value("heat_heat_transfer_coefficient"));
        txtExternalTemperature->setValue(m_boundary->get_value("heat_external_temperature"));
    }
}

bool SceneBoundaryHeatDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (m_boundary->type == "heat_temperature")
    {
        if (txtTemperature->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_temperature")] = txtTemperature->value();
        else
            return false;
    }
    else if (m_boundary->type == "heat_heat_flux")
    {
        if (txtHeatFlux->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_heat_flux")] = txtHeatFlux->value();
        else
            return false;
        if (txtHeatTransferCoefficient->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_heat_transfer_coefficient")] = txtHeatTransferCoefficient->value();
        else
            return false;
        if (txtExternalTemperature->evaluate())
            m_boundary->values[m_boundary->get_boundary_type_variable("heat_external_temperature")] = txtExternalTemperature->value();
        else
            return false;
    }

    return true;
}

void SceneBoundaryHeatDialog::doTypeChanged(int index)
{
    txtTemperature->setEnabled(false);
    txtHeatFlux->setEnabled(false);
    txtHeatTransferCoefficient->setEnabled(false);
    txtExternalTemperature->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "heat_temperature")
    {
        txtTemperature->setEnabled(true);
    }
    else if (cmbType->itemData(index) == "heat_heat_flux")
    {
        txtHeatFlux->setEnabled(true);
        txtHeatTransferCoefficient->setEnabled(true);
        txtExternalTemperature->setEnabled(true);
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialHeatDialog::SceneMaterialHeatDialog(SceneMaterialHeat *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialHeatDialog::createContent()
{
    txtThermalConductivity = new ValueLineEdit(this);
    txtThermalConductivity->setMinimumSharp(0.0);
    txtVolumeHeat = new ValueLineEdit(this, true);
    txtDensity = new ValueLineEdit(this);
    txtDensity->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);
    txtSpecificHeat = new ValueLineEdit(this);
    txtSpecificHeat->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);

    connect(txtThermalConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtSpecificHeat, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i> (W/m·K)").arg(QString::fromUtf8("λ")),
                                  tr("Thermal conductivity")), 10, 0);
    layout->addWidget(txtThermalConductivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>Q</i> (W/m<sup>3</sup>)"),
                                  tr("Volume heat")), 11, 0);
    layout->addWidget(txtVolumeHeat, 11, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (kg/m<sup>3</sup>)").arg(QString::fromUtf8("ρ")),
                                  tr("Mass density")), 12, 0);
    layout->addWidget(txtDensity, 12, 2);
    layout->addWidget(createLabel(tr("<i>c</i><sub>p</sub> (J/kg·K)"),
                                  tr("Specific heat")), 13, 0);
    layout->addWidget(txtSpecificHeat, 13, 2);
}

void SceneMaterialHeatDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(m_material);

    txtThermalConductivity->setValue(material->thermal_conductivity);
    txtVolumeHeat->setValue(material->volume_heat);
    txtDensity->setValue(material->density);
    txtSpecificHeat->setValue(material->specific_heat);
}

bool SceneMaterialHeatDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialHeat *material = dynamic_cast<SceneMaterialHeat *>(m_material);

    if (txtThermalConductivity->evaluate())
        material->thermal_conductivity  = txtThermalConductivity->value();
    else
        return false;

    if (txtVolumeHeat->evaluate())
        material->volume_heat  = txtVolumeHeat->value();
    else
        return false;

    if (txtDensity->evaluate())
        material->density  = txtDensity->value();
    else
        return false;

    if (txtSpecificHeat->evaluate())
        material->specific_heat  = txtSpecificHeat->value();
    else
        return false;

    return true;
}
