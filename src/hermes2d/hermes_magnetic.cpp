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

#include "hermes_magnetic.h"

#include "scene.h"
#include "gui.h"

class WeakFormMagnetic : public WeakFormAgros
{
public:
    WeakFormMagnetic(int neq) : WeakFormAgros(neq) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == PhysicFieldBC_Magnetic_SurfaceCurrent)
                {
                    if (fabs(boundary->value_real.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->value_real.number,
                                                                                                        HERMES_PLANAR));
                    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                        if (fabs(boundary->value_imag.number) > EPS_ZERO)
                            add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                            QString::number(i + 1).toStdString(),
                                                                                                            boundary->value_imag.number,
                                                                                                            HERMES_PLANAR));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                // steady state and transient analysis
                add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(0, 0,
                                                                                                         QString::number(i).toStdString(),
                                                                                                         1.0 / (material->permeability.number * MU0),
                                                                                                         HERMES_NONSYM,
                                                                                                         convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                         (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 3)));

                // velocity
                if ((fabs(material->conductivity.number) > EPS_ZERO) &&
                        ((fabs(material->velocity_x.number) > EPS_ZERO) ||
                         (fabs(material->velocity_y.number) > EPS_ZERO) ||
                         (fabs(material->velocity_angular.number) > EPS_ZERO)))
                    add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostaticsVelocity(0, 0,
                                                                                                                     QString::number(i).toStdString(),
                                                                                                                     material->conductivity.number,
                                                                                                                     material->velocity_x.number,
                                                                                                                     material->velocity_y.number,
                                                                                                                     material->velocity_angular.number));

                // external current density
                if (fabs(material->current_density_real.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->current_density_real.number,
                                                                                                   HERMES_PLANAR));

                // remanence
                if (fabs(material->remanence.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostaticsRemanence(0,
                                                                                                                      QString::number(i).toStdString(),
                                                                                                                      material->permeability.number * MU0,
                                                                                                                      material->remanence.number,
                                                                                                                      material->remanence_angle.number,
                                                                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));

                // harmonic analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(1, 1,
                                                                                                             QString::number(i).toStdString(),
                                                                                                             1.0 / (material->permeability.number * MU0),
                                                                                                             HERMES_NONSYM,
                                                                                                             convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                             (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 5)));

                    if (fabs(material->conductivity.number) > EPS_ZERO)
                    {
                        add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 1,
                                                                                                  QString::number(i).toStdString(),
                                                                                                  - 2 * M_PI * Util::scene()->problemInfo()->frequency * material->conductivity.number,
                                                                                                  HERMES_NONSYM,
                                                                                                  HERMES_PLANAR));

                        add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 0,
                                                                                                  QString::number(i).toStdString(),
                                                                                                  2 * M_PI * Util::scene()->problemInfo()->frequency * material->conductivity.number,
                                                                                                  HERMES_NONSYM,
                                                                                                  HERMES_PLANAR));
                    }

                    // external current density
                    if (fabs(material->current_density_imag.number) > EPS_ZERO)
                        add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(1,
                                                                                                       QString::number(i).toStdString(),
                                                                                                       material->current_density_imag.number,
                                                                                                       HERMES_PLANAR));
                }

                // transient analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                {
                    if (fabs(material->conductivity.number) > EPS_ZERO)
                    {
                        if (solution.size() > 0)
                        {
                            add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                                      QString::number(i).toStdString(),
                                                                                                      material->conductivity.number / Util::scene()->problemInfo()->timeStep.number,
                                                                                                      HERMES_SYM,
                                                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));

                            add_vector_form(new CustomVectorFormTimeDep(0,
                                                                        QString::number(i).toStdString(),
                                                                        material->conductivity.number / Util::scene()->problemInfo()->timeStep.number,
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

void ParserMagnetic::setParserVariables(SceneMaterial *material)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    pmur = marker->permeability.number;
    pgamma = marker->conductivity.number;
    pjer = marker->current_density_real.number;
    pjei = marker->current_density_real.number;
    pbr = marker->remanence.number;
    pbra = marker->remanence_angle.number;
    pvx = marker->velocity_x.number;
    pvy = marker->velocity_y.number;
    pva = marker->velocity_angular.number;
}

// ****************************************************************************************************************

LocalPointValueMagnetic::LocalPointValueMagnetic(const Point &point) : LocalPointValue(point)
{
    parser = new ParserMagnetic();
    initParser();

    parser->parser[0]->DefineVar("mur", &static_cast<ParserMagnetic *>(parser)->pmur);
    parser->parser[0]->DefineVar("gamma", &static_cast<ParserMagnetic *>(parser)->pgamma);
    parser->parser[0]->DefineVar("Jer", &static_cast<ParserMagnetic *>(parser)->pjer);
    parser->parser[0]->DefineVar("Jei", &static_cast<ParserMagnetic *>(parser)->pjei);
    parser->parser[0]->DefineVar("Br", &static_cast<ParserMagnetic *>(parser)->pbr);
    parser->parser[0]->DefineVar("Bra", &static_cast<ParserMagnetic *>(parser)->pbra);
    parser->parser[0]->DefineVar("vx", &static_cast<ParserMagnetic *>(parser)->pvx);
    parser->parser[0]->DefineVar("vy", &static_cast<ParserMagnetic *>(parser)->pvy);
    parser->parser[0]->DefineVar("va", &static_cast<ParserMagnetic *>(parser)->pva);

    calculate();
}

// ****************************************************************************************************************

SurfaceIntegralValueMagnetic::SurfaceIntegralValueMagnetic() : SurfaceIntegralValue()
{
    parser = new ParserMagnetic();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("mur", &static_cast<ParserMagnetic *>(parser)->pmur);
        ((mu::Parser *) *it)->DefineVar("gamma", &static_cast<ParserMagnetic *>(parser)->pgamma);
        ((mu::Parser *) *it)->DefineVar("Jer", &static_cast<ParserMagnetic *>(parser)->pjer);
        ((mu::Parser *) *it)->DefineVar("Jei", &static_cast<ParserMagnetic *>(parser)->pjei);
        ((mu::Parser *) *it)->DefineVar("Br", &static_cast<ParserMagnetic *>(parser)->pbr);
        ((mu::Parser *) *it)->DefineVar("Bra", &static_cast<ParserMagnetic *>(parser)->pbra);
        ((mu::Parser *) *it)->DefineVar("vx", &static_cast<ParserMagnetic *>(parser)->pvx);
        ((mu::Parser *) *it)->DefineVar("vy", &static_cast<ParserMagnetic *>(parser)->pvy);
        ((mu::Parser *) *it)->DefineVar("va", &static_cast<ParserMagnetic *>(parser)->pva);
    }

    calculate();
}

// ****************************************************************************************************************

VolumeIntegralValueMagnetic::VolumeIntegralValueMagnetic() : VolumeIntegralValue()
{
    parser = new ParserMagnetic();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("mur", &static_cast<ParserMagnetic *>(parser)->pmur);
        ((mu::Parser *) *it)->DefineVar("gamma", &static_cast<ParserMagnetic *>(parser)->pgamma);
        ((mu::Parser *) *it)->DefineVar("Jer", &static_cast<ParserMagnetic *>(parser)->pjer);
        ((mu::Parser *) *it)->DefineVar("Jei", &static_cast<ParserMagnetic *>(parser)->pjei);
        ((mu::Parser *) *it)->DefineVar("Br", &static_cast<ParserMagnetic *>(parser)->pbr);
        ((mu::Parser *) *it)->DefineVar("Bra", &static_cast<ParserMagnetic *>(parser)->pbra);
        ((mu::Parser *) *it)->DefineVar("vx", &static_cast<ParserMagnetic *>(parser)->pvx);
        ((mu::Parser *) *it)->DefineVar("vy", &static_cast<ParserMagnetic *>(parser)->pvy);
        ((mu::Parser *) *it)->DefineVar("va", &static_cast<ParserMagnetic *>(parser)->pva);
    }

    sln.push_back(Util::scene()->sceneSolution()->sln(0));

    calculate();
}

// *************************************************************************************************************************************

ViewScalarFilterMagnetic::ViewScalarFilterMagnetic(Hermes::vector<MeshFunction *> sln,
                                                             std::string expression) :
    ViewScalarFilter(sln)
{
    parser = new ParserMagnetic();
    initParser(expression);

    parser->parser[0]->DefineVar("mur", &static_cast<ParserMagnetic *>(parser)->pmur);
    parser->parser[0]->DefineVar("gamma", &static_cast<ParserMagnetic *>(parser)->pgamma);
    parser->parser[0]->DefineVar("Jer", &static_cast<ParserMagnetic *>(parser)->pjer);
    parser->parser[0]->DefineVar("Jei", &static_cast<ParserMagnetic *>(parser)->pjei);
    parser->parser[0]->DefineVar("Br", &static_cast<ParserMagnetic *>(parser)->pbr);
    parser->parser[0]->DefineVar("Bra", &static_cast<ParserMagnetic *>(parser)->pbra);
    parser->parser[0]->DefineVar("vx", &static_cast<ParserMagnetic *>(parser)->pvx);
    parser->parser[0]->DefineVar("vy", &static_cast<ParserMagnetic *>(parser)->pvy);
    parser->parser[0]->DefineVar("va", &static_cast<ParserMagnetic *>(parser)->pva);
}

// **************************************************************************************************************************

int ModuleMagnetic::number_of_solution() const
{
    return (get_analysis_type() == AnalysisType_Harmonic) ? 2 : 1;
}

LocalPointValue *ModuleMagnetic::local_point_value(const Point &point)
{
    return new LocalPointValueMagnetic(point);
}

SurfaceIntegralValue *ModuleMagnetic::surface_integral_value()
{
    return new SurfaceIntegralValueMagnetic();
}

VolumeIntegralValue *ModuleMagnetic::volume_integral_value()
{
    return new VolumeIntegralValueMagnetic();
}

ViewScalarFilter *ModuleMagnetic::view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                          PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    return new ViewScalarFilterMagnetic(sln1, get_expression(physicFieldVariable, physicFieldVariableComp));
}

Hermes::vector<SolutionArray *> ModuleMagnetic::solve(ProgressItemSolve *progressItemSolve)
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return Hermes::vector<SolutionArray *>();
    }

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
        SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value_real.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!boundary->value_imag.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->current_density_real.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->current_density_imag.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->permeability.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->conductivity.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->remanence.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->remanence_angle.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->velocity_x.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->velocity_y.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->velocity_angular.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bc1;
    EssentialBCs bc2;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->type == PhysicFieldBC_Magnetic_VectorPotential)
            {
                bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value_real.number));
                bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value_imag.number));
            }
        }
    }
    Hermes::vector<EssentialBCs> bcs(bc1, bc2);

    WeakFormMagnetic wf(number_of_solution());

    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

void ModuleMagnetic::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(Util::scene()->materials[i]);

        material->current_density_real.evaluate(time);
        material->current_density_imag.evaluate(time);
    }
}

// **************************************************************************************************************************
// rewrite

void ModuleMagnetic::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Magnetic_VectorPotential:
    case PhysicFieldBC_Magnetic_SurfaceCurrent:
        Util::scene()->addBoundary(new SceneBoundaryMagnetic(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("value_real", "0")),
                                                             Value(element->attribute("value_imag", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void ModuleMagnetic::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(boundary->type));
    element->setAttribute("value_real", boundary->value_real.text);
    element->setAttribute("value_imag", boundary->value_imag.text);
}

void ModuleMagnetic::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialMagnetic(element->attribute("name"),
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

void ModuleMagnetic::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(marker);

    element->setAttribute("current_density_real", material->current_density_real.text);
    element->setAttribute("current_density_imag", material->current_density_imag.text);
    element->setAttribute("permeability", material->permeability.text);
    element->setAttribute("conductivity", material->conductivity.text);
    element->setAttribute("remanence", material->remanence.text);
    element->setAttribute("remanence_angle", material->remanence_angle.text);
    element->setAttribute("velocity_x", material->velocity_x.text);
    element->setAttribute("velocity_y", material->velocity_y.text);
    element->setAttribute("velocity_angular", material->velocity_angular.text);
}

SceneBoundary *ModuleMagnetic::newBoundary()
{
    return new SceneBoundaryMagnetic(tr("new boundary"),
                                     PhysicFieldBC_Magnetic_VectorPotential,
                                     Value("0"),
                                     Value("0"));
}

SceneBoundary *ModuleMagnetic::newBoundary(PyObject *self, PyObject *args)
{
    double value_real;
    double value_imag = 0.0;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|d", &name, &type, &value_real, &value_imag))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryMagnetic(name,
                                         physicFieldBCFromStringKey(type),
                                         Value(QString::number(value_real)),
                                         Value(QString::number(value_imag)));
    }

    return NULL;
}

SceneBoundary *ModuleMagnetic::modifyBoundary(PyObject *self, PyObject *args)
{
    double value_real, value_imag = 0.0;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|d", &name, &type, &value_real, &value_imag))
    {
        if (SceneBoundaryMagnetic *marker = dynamic_cast<SceneBoundaryMagnetic *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                marker->value_real = Value(QString::number(value_real));
                marker->value_imag = Value(QString::number(value_imag));
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

SceneMaterial *ModuleMagnetic::newMaterial()
{
    return new SceneMaterialMagnetic(tr("new material"),
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

SceneMaterial *ModuleMagnetic::newMaterial(PyObject *self, PyObject *args)
{
    double current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity, &remanence, &remanence_angle, &velocity_x, &velocity_y, &velocity_angular))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialMagnetic(name,
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

SceneMaterial *ModuleMagnetic::modifyMaterial(PyObject *self, PyObject *args)
{
    double current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity, &remanence, &remanence_angle, &velocity_x, &velocity_y, &velocity_angular))
    {
        if (SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(Util::scene()->getMaterial(name)))
        {
            marker->current_density_real = Value(QString::number(current_density_real));
            marker->current_density_imag = Value(QString::number(current_density_imag));
            marker->permeability = Value(QString::number(permeability));
            marker->conductivity = Value(QString::number(conductivity));
            marker->remanence = Value(QString::number(remanence));
            marker->remanence_angle = Value(QString::number(remanence_angle));
            marker->velocity_x = Value(QString::number(velocity_x));
            marker->velocity_y = Value(QString::number(velocity_y));
            marker->velocity_angular = Value(QString::number(velocity_angular));
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
ViewScalarFilter *ModuleMagnetic::viewScalarFilter(PhysicFieldVariableDeprecated physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState)
    {
        return new ViewScalarFilterMagnetic(Hermes::vector<MeshFunction *>(sln1),
                                            physicFieldVariable,
                                            physicFieldVariableComp);
    }

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        Solution *sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
        return new ViewScalarFilterMagnetic(Hermes::vector<MeshFunction *>(sln1, sln2),
                                            physicFieldVariable,
                                            physicFieldVariableComp);
    }
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        Solution *sln2;
        if (Util::scene()->sceneSolution()->timeStep() > 0)
            sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() - 1);
        else
            sln2 = sln1;

        return new ViewScalarFilterMagnetic(Hermes::vector<MeshFunction *>(sln1, sln2),
                                            physicFieldVariable,
                                            physicFieldVariableComp);
    }
}
*/

// *************************************************************************************************************************************

SceneBoundaryMagnetic::SceneBoundaryMagnetic(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag) : SceneBoundary(name, type)
{
    this->value_real = value_real;
    this->value_imag = value_imag;
}

QString SceneBoundaryMagnetic::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3, %4)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value_real.text).
            arg(value_imag.text);
}

QMap<QString, QString> SceneBoundaryMagnetic::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Magnetic_VectorPotential:
        out["Vector potential - real (Wb/m)"] = value_real.text;
        out["Vector potential - real (Wb/m)"] = value_imag.text;
        break;
    case PhysicFieldBC_Magnetic_SurfaceCurrent:
        out["Surface current - real (A/m)"] = value_real.text;
        out["Surface current - imag (A/m)"] = value_imag.text;
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneBoundaryMagnetic::showDialog(QWidget *parent)
{
    SceneEdgeMagneticDialog *dialog = new SceneEdgeMagneticDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialMagnetic::SceneMaterialMagnetic(const QString &name, Value current_density_real, Value current_density_imag, Value permeability, Value conductivity, Value remanence, Value remanence_angle, Value velocity_x, Value velocity_y, Value velocity_angular)
    : SceneMaterial(name)
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

QString SceneMaterialMagnetic::script()
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

QMap<QString, QString> SceneMaterialMagnetic::data()
{
    QMap<QString, QString> out;
    out["Current density - real (A/m2)"] = current_density_real.text;
    out["Current density - imag (A/m2)"] = current_density_imag.text;
    out["Permeability (-)"] = permeability.text;
    out["Conductivity (S/m)"] = conductivity.text;
    out["Remanence (T)"] = remanence.text;
    out["Remanence angle (-)"] = remanence_angle.text;
    out["Velocity x (m/s)"] = velocity_x.text;
    out["Velocity y (m/s)"] = velocity_y.text;
    out["Angular velocity (m/s)"] = velocity_angular.text;
    return QMap<QString, QString>(out);
}

int SceneMaterialMagnetic::showDialog(QWidget *parent)
{
    SceneMaterialMagneticDialog *dialog = new SceneMaterialMagneticDialog(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneEdgeMagneticDialog::SceneEdgeMagneticDialog(SceneBoundaryMagnetic *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneEdgeMagneticDialog::createContent()
{
    lblValueUnit = new QLabel("");

    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Magnetic_VectorPotential), PhysicFieldBC_Magnetic_VectorPotential);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Magnetic_SurfaceCurrent), PhysicFieldBC_Magnetic_SurfaceCurrent);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValueReal = new ValueLineEdit(this);
    txtValueImag = new ValueLineEdit(this);

    connect(txtValueReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtValueImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtValueReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtValueImag);

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnit, 11, 0);
    layout->addLayout(layoutCurrentDensity, 11, 2);
}

void SceneEdgeMagneticDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(boundary->type));
    txtValueReal->setValue(boundary->value_real);
    txtValueImag->setValue(boundary->value_imag);
}

bool SceneEdgeMagneticDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(m_boundary);

    boundary->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValueReal->evaluate())
        boundary->value_real  = txtValueReal->value();
    else
        return false;

    if (txtValueImag->evaluate())
        boundary->value_imag  = txtValueImag->value();
    else
        return false;

    return true;
}

void SceneEdgeMagneticDialog::doTypeChanged(int index)
{
    txtValueReal->setEnabled(false);
    txtValueImag->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, (PhysicFieldBC) cmbType->itemData(index).toInt());

    // enable controls
    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_Magnetic_VectorPotential:
    {
        txtValueReal->setEnabled(true);
        txtValueImag->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic);
        lblValueUnit->setText(tr("<i>A</i><sub>0</sub> (Wb/m)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    case PhysicFieldBC_Magnetic_SurfaceCurrent:
    {
        txtValueReal->setEnabled(true);
        txtValueImag->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic);
        lblValueUnit->setText(tr("<i>K</i><sub>0</sub> (A/m)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialMagneticDialog::SceneMaterialMagneticDialog(QWidget *parent, SceneMaterialMagnetic *material) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialMagneticDialog::createContent()
{
    txtPermeability = new ValueLineEdit(this);
    txtConductivity = new ValueLineEdit(this);
    txtCurrentDensityReal = new ValueLineEdit(this, true);
    txtCurrentDensityImag = new ValueLineEdit(this, true);
    txtCurrentDensityImag->setEnabled(Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic);
    txtRemanence = new ValueLineEdit(this);
    txtRemanenceAngle = new ValueLineEdit(this);
    txtVelocityX = new ValueLineEdit(this);
    txtVelocityX->setEnabled(Util::scene()->problemInfo()->problemType == ProblemType_Planar);
    txtVelocityY = new ValueLineEdit(this);
    txtVelocityAngular = new ValueLineEdit(this);
    txtVelocityAngular->setEnabled(Util::scene()->problemInfo()->problemType == ProblemType_Planar);

    connect(txtPermeability, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRemanence, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtRemanenceAngle, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtVelocityX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtVelocityY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtVelocityAngular, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtCurrentDensityReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtCurrentDensityImag);

    // remanence
    QGridLayout *layoutRemanence = new QGridLayout();
    layoutRemanence->addWidget(createLabel(tr("<i>B</i><sub>r</sub> (T)"),
                                           tr("Rem. flux dens.")), 0, 0);
    layoutRemanence->addWidget(txtRemanence, 0, 1);
    layoutRemanence->addWidget(createLabel(tr("<i>%1</i> (deg.)").arg(QString::fromUtf8("α")),
                                           tr("Direction of rem.")), 1, 0);
    layoutRemanence->addWidget(txtRemanenceAngle, 1, 1);

    QGroupBox *grpRemanence = new QGroupBox(tr("Permanent magnet"), this);
    grpRemanence->setLayout(layoutRemanence);

    // velocity
    QGridLayout *layoutVelocity = new QGridLayout();
    layoutVelocity->addWidget(createLabel(tr("<i>v</i><sub>%1</sub> (m/s)").arg(Util::scene()->problemInfo()->labelX().toLower()),
                                          tr("Velocity")), 0, 0);
    layoutVelocity->addWidget(txtVelocityX, 0, 1);
    layoutVelocity->addWidget(createLabel(tr("<i>v</i><sub>%1</sub> (m/s)").arg(Util::scene()->problemInfo()->labelY().toLower()),
                                          tr("Velocity")), 1, 0);
    layoutVelocity->addWidget(txtVelocityY, 1, 1);
    layoutVelocity->addWidget(createLabel(tr("<i>%1</i> (rad/s)").arg(QString::fromUtf8("ω")),
                                          tr("Velocity angular")), 2, 0);
    layoutVelocity->addWidget(txtVelocityAngular, 2, 1);

    QGroupBox *grpVelocity = new QGroupBox(tr("Velocity"), this);
    grpVelocity->setLayout(layoutVelocity);

    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("μ")),
                                  tr("Permeability")), 10, 0);
    layout->addWidget(txtPermeability, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 11, 0);
    layout->addWidget(txtConductivity, 11, 2);
    layout->addWidget(createLabel(tr("<i>J</i><sub>ext</sub> (A/m<sup>2</sup>)"),
                                  tr("Current density")), 12, 0);
    layout->addLayout(layoutCurrentDensity, 12, 2);
    layout->addWidget(grpRemanence, 13, 0, 1, 3);
    layout->addWidget(grpVelocity, 14, 0, 1, 3);
}

void SceneMaterialMagneticDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(m_material);

    txtPermeability->setValue(material->permeability);
    txtConductivity->setValue(material->conductivity);
    txtCurrentDensityReal->setValue(material->current_density_real);
    txtCurrentDensityImag->setValue(material->current_density_imag);
    txtRemanence->setValue(material->remanence);
    txtRemanenceAngle->setValue(material->remanence_angle);
    txtVelocityX->setValue(material->velocity_x);
    txtVelocityY->setValue(material->velocity_y);
    txtVelocityAngular->setValue(material->velocity_angular);
}

bool SceneMaterialMagneticDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(m_material);

    if (txtPermeability->evaluate())
        material->permeability  = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        material->conductivity  = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        material->current_density_real  = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        material->current_density_imag  = txtCurrentDensityImag->value();
    else
        return false;

    if (txtRemanence->evaluate())
        material->remanence = txtRemanence->value();
    else
        return false;

    if (txtRemanenceAngle->evaluate())
        material->remanence_angle = txtRemanenceAngle->value();
    else
        return false;

    if (txtVelocityX->evaluate())
        material->velocity_x = txtVelocityX->value();
    else
        return false;

    if (txtVelocityY->evaluate())
        material->velocity_y = txtVelocityY->value();
    else
        return false;

    if (txtVelocityAngular->evaluate())
        material->velocity_angular = txtVelocityAngular->value();
    else
        return false;

    return true;
}
