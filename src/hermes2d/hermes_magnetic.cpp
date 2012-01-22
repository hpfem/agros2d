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

// *******************************************************************************************************

int HermesMagnetic::numberOfSolution() const
{
    return (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic) ? 2 : 1;
}

PhysicFieldVariable HermesMagnetic::contourPhysicFieldVariable()
{
    return (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic) ?
                PhysicFieldVariable_Magnetic_VectorPotential : PhysicFieldVariable_Magnetic_VectorPotentialReal;
}

PhysicFieldVariable HermesMagnetic::scalarPhysicFieldVariable()
{
    return (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic) ?
                PhysicFieldVariable_Magnetic_FluxDensity : PhysicFieldVariable_Magnetic_FluxDensityReal;
}

PhysicFieldVariableComp HermesMagnetic::scalarPhysicFieldVariableComp()
{
    return (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic) ?
                PhysicFieldVariableComp_Magnitude : PhysicFieldVariableComp_Magnitude;
}

PhysicFieldVariable HermesMagnetic::vectorPhysicFieldVariable()
{
    return (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic) ?
                PhysicFieldVariable_Magnetic_FluxDensityReal : PhysicFieldVariable_Magnetic_FluxDensityReal;
}


void HermesMagnetic::readBoundaryFromDomElement(QDomElement *element)
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

void HermesMagnetic::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(boundary->type));
    element->setAttribute("value_real", boundary->value_real.text);
    element->setAttribute("value_imag", boundary->value_imag.text);
}

void HermesMagnetic::readMaterialFromDomElement(QDomElement *element)
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

void HermesMagnetic::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
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
    element->setAttribute("velocity_angular", material->velocity_angular.text);}

LocalPointValue *HermesMagnetic::localPointValue(const Point &point)
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
            << "pj" << "wm" << "mur" << "gamma" << "Br" << "Brangle" << "vx" << "vy"
            << "Fx_real" << "Fy_real" << "Fx_imag" << "Fy_imag";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesMagnetic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueMagnetic();
}

QStringList HermesMagnetic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S" << "Fx" << "Fy" << "T";
    return QStringList(headers);
}

VolumeIntegralValue *HermesMagnetic::volumeIntegralValue()
{
    return new VolumeIntegralValueMagnetic();
}

QStringList HermesMagnetic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "Ie_real" << "Ie_imag" << "Iit_real" << "Iit_imag" << "Iiv_real" << "Iiv_imag" << "I_real" << "I_imag"
            << "Fx" << "Fy" << "T" << "Pj" << "Wm";
    return QStringList(headers);
}

SceneBoundary *HermesMagnetic::newBoundary()
{
    return new SceneBoundaryMagnetic(tr("new boundary"),
                                     PhysicFieldBC_Magnetic_VectorPotential,
                                     Value("0"),
                                     Value("0"));
}

SceneBoundary *HermesMagnetic::newBoundary(PyObject *self, PyObject *args)
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

SceneBoundary *HermesMagnetic::modifyBoundary(PyObject *self, PyObject *args)
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

SceneMaterial *HermesMagnetic::newMaterial()
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

SceneMaterial *HermesMagnetic::newMaterial(PyObject *self, PyObject *args)
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

SceneMaterial *HermesMagnetic::modifyMaterial(PyObject *self, PyObject *args)
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

void HermesMagnetic::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    // harmonic
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_VectorPotential), PhysicFieldVariable_Magnetic_VectorPotential);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_VectorPotentialReal), PhysicFieldVariable_Magnetic_VectorPotentialReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_VectorPotentialImag), PhysicFieldVariable_Magnetic_VectorPotentialImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_FluxDensity), PhysicFieldVariable_Magnetic_FluxDensity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_FluxDensityReal), PhysicFieldVariable_Magnetic_FluxDensityReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_FluxDensityImag), PhysicFieldVariable_Magnetic_FluxDensityImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_MagneticField), PhysicFieldVariable_Magnetic_MagneticField);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_MagneticFieldReal), PhysicFieldVariable_Magnetic_MagneticFieldReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_MagneticFieldImag), PhysicFieldVariable_Magnetic_MagneticFieldImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensity), PhysicFieldVariable_Magnetic_CurrentDensity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityReal), PhysicFieldVariable_Magnetic_CurrentDensityReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityImag), PhysicFieldVariable_Magnetic_CurrentDensityImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform), PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal), PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag), PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity), PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal), PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag), PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityTotal), PhysicFieldVariable_Magnetic_CurrentDensityTotal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityTotalReal), PhysicFieldVariable_Magnetic_CurrentDensityTotalReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityTotalImag), PhysicFieldVariable_Magnetic_CurrentDensityTotalImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_PowerLosses), PhysicFieldVariable_Magnetic_PowerLosses);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_LorentzForce), PhysicFieldVariable_Magnetic_LorentzForce);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_EnergyDensity), PhysicFieldVariable_Magnetic_EnergyDensity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Permeability), PhysicFieldVariable_Magnetic_Permeability);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Conductivity), PhysicFieldVariable_Magnetic_Conductivity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Velocity), PhysicFieldVariable_Magnetic_Velocity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Remanence), PhysicFieldVariable_Magnetic_Remanence);
    }

    // steady state and transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState ||
            Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_VectorPotential), PhysicFieldVariable_Magnetic_VectorPotentialReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_FluxDensity), PhysicFieldVariable_Magnetic_FluxDensityReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_MagneticField), PhysicFieldVariable_Magnetic_MagneticFieldReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensity), PhysicFieldVariable_Magnetic_CurrentDensityReal);
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient) cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform), PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity), PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_CurrentDensityTotal), PhysicFieldVariable_Magnetic_CurrentDensityTotalReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_PowerLosses), PhysicFieldVariable_Magnetic_PowerLosses);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_LorentzForce), PhysicFieldVariable_Magnetic_LorentzForce);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_EnergyDensity), PhysicFieldVariable_Magnetic_EnergyDensity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Permeability), PhysicFieldVariable_Magnetic_Permeability);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Conductivity), PhysicFieldVariable_Magnetic_Conductivity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Velocity), PhysicFieldVariable_Magnetic_Velocity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Remanence), PhysicFieldVariable_Magnetic_Remanence);
    }
}

void HermesMagnetic::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    // harmonic
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_FluxDensity), PhysicFieldVariable_Magnetic_FluxDensityReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_MagneticField), PhysicFieldVariable_Magnetic_MagneticFieldReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Remanence), PhysicFieldVariable_Magnetic_Remanence);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_LorentzForce), PhysicFieldVariable_Magnetic_LorentzForce);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Velocity), PhysicFieldVariable_Magnetic_Velocity);
    }

    // steady state and transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState ||
            Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_FluxDensity), PhysicFieldVariable_Magnetic_FluxDensityReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_MagneticField), PhysicFieldVariable_Magnetic_MagneticFieldReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Remanence), PhysicFieldVariable_Magnetic_Remanence);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_LorentzForce), PhysicFieldVariable_Magnetic_LorentzForce);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Magnetic_Velocity), PhysicFieldVariable_Magnetic_Velocity);
    }
}


void HermesMagnetic::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueMagnetic *localPointValueMagnetic = dynamic_cast<LocalPointValueMagnetic *>(localPointValue);

    // magnetic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Magnetic field"));
    magneticNode->setExpanded(true);

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
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
        addTreeWidgetItemValue(magneticNode, "Lorentz force:", QString("%1").arg(sqrt(sqr(localPointValueMagnetic->FL_real.x) + sqr(localPointValueMagnetic->FL_imag.x) + sqr(localPointValueMagnetic->FL_real.y) + sqr(localPointValueMagnetic->FL_imag.y)), 0, 'e', 3), "N");

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
        addTreeWidgetItemValue(magneticNode, tr("Losses density:"), QString("%1").arg(localPointValueMagnetic->pj, 0, 'e', 3), "W/m3");

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

        addTreeWidgetItemValue(itemInducedCurrentDensity, tr("external:"), QString("%1").arg(localPointValueMagnetic->current_density_real, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemInducedCurrentDensity, tr("transform:"), QString("%1").arg(localPointValueMagnetic->current_density_induced_transform_real, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemInducedCurrentDensity, tr("velocity:"), QString("%1").arg(localPointValueMagnetic->current_density_induced_velocity_real, 0, 'e', 3), "A/m2");
        addTreeWidgetItemValue(itemInducedCurrentDensity, tr("total:"), QString("%1").arg(localPointValueMagnetic->current_density_total_real, 0, 'e', 3), "A/m2");

        // Power losses
        addTreeWidgetItemValue(magneticNode, tr("Losses density:"), QString("%1").arg(localPointValueMagnetic->pj, 0, 'e', 3), "W/m3");

        // Energy density
        addTreeWidgetItemValue(magneticNode, tr("Energy density:"), QString("%1").arg(localPointValueMagnetic->wm, 0, 'e', 3), "J/m3");
    }
}

void HermesMagnetic::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueMagnetic *surfaceIntegralValueMagnetic = dynamic_cast<SurfaceIntegralValueMagnetic *>(surfaceIntegralValue);

    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Magnetic field"));
    magneticNode->setExpanded(true);

    // force
    QTreeWidgetItem *itemForce = new QTreeWidgetItem(magneticNode);
    itemForce->setText(0, tr("Maxwell force"));
    itemForce->setExpanded(true);

    addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelX(), QString("%1").arg(surfaceIntegralValueMagnetic->forceMaxwellX, 0, 'e', 3), "N");
    addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelY(), QString("%1").arg(surfaceIntegralValueMagnetic->forceMaxwellY, 0, 'e', 3), "N");

    // torque
    addTreeWidgetItemValue(magneticNode, tr("Torque:"), QString("%1").arg(surfaceIntegralValueMagnetic->torque, 0, 'e', 3), tr("Nm"));
}

void HermesMagnetic::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueMagnetic *volumeIntegralValueMagnetic = dynamic_cast<VolumeIntegralValueMagnetic *>(volumeIntegralValue);

    // harmonic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Magnetic field"));
    magneticNode->setExpanded(true);

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        // external current
        QTreeWidgetItem *itemCurrentInducedExternal = new QTreeWidgetItem(magneticNode);
        itemCurrentInducedExternal->setText(0, tr("External current"));
        itemCurrentInducedExternal->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentInducedExternal, tr("real:"), QString("%1").arg(volumeIntegralValueMagnetic->currentReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedExternal, tr("imag:"), QString("%1").arg(volumeIntegralValueMagnetic->currentImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedExternal, tr("magnitude:"), QString("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentReal) + sqr(volumeIntegralValueMagnetic->currentImag)), 0, 'e', 3), "A");

        // transform induced current
        QTreeWidgetItem *itemCurrentInducedTransform = new QTreeWidgetItem(magneticNode);
        itemCurrentInducedTransform->setText(0, tr("Transform induced current"));
        itemCurrentInducedTransform->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentInducedTransform, tr("real:"), QString("%1").arg(volumeIntegralValueMagnetic->currentInducedTransformReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedTransform, tr("imag:"), QString("%1").arg(volumeIntegralValueMagnetic->currentInducedTransformImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedTransform, tr("magnitude:"), QString("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentInducedTransformReal) + sqr(volumeIntegralValueMagnetic->currentInducedTransformImag)), 0, 'e', 3), "A");

        // velocity induced current
        QTreeWidgetItem *itemCurrentInducedVelocity = new QTreeWidgetItem(magneticNode);
        itemCurrentInducedVelocity->setText(0, tr("Velocity induced current"));
        itemCurrentInducedVelocity->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentInducedVelocity, tr("real:"), QString("%1").arg(volumeIntegralValueMagnetic->currentInducedVelocityReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedVelocity, tr("imag:"), QString("%1").arg(volumeIntegralValueMagnetic->currentInducedVelocityImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentInducedVelocity, tr("magnitude:"), QString("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentInducedVelocityReal) + sqr(volumeIntegralValueMagnetic->currentInducedVelocityImag)), 0, 'e', 3), "A");

        // total current
        QTreeWidgetItem *itemCurrentTotal = new QTreeWidgetItem(magneticNode);
        itemCurrentTotal->setText(0, tr("Total current"));
        itemCurrentTotal->setExpanded(true);

        addTreeWidgetItemValue(itemCurrentTotal, tr("real:"), QString("%1").arg(volumeIntegralValueMagnetic->currentTotalReal, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentTotal, tr("imag:"), QString("%1").arg(volumeIntegralValueMagnetic->currentTotalImag, 0, 'e', 3), "A");
        addTreeWidgetItemValue(itemCurrentTotal, tr("magnitude:"), QString("%1").arg(sqrt(sqr(volumeIntegralValueMagnetic->currentTotalReal) + sqr(volumeIntegralValueMagnetic->currentTotalImag)), 0, 'e', 3), "A");

        addTreeWidgetItemValue(magneticNode, tr("Power Losses:"), QString("%1").arg(volumeIntegralValueMagnetic->powerLosses, 0, 'e', 3), tr("W"));
        addTreeWidgetItemValue(magneticNode, tr("Energy avg.:"), QString("%1").arg(volumeIntegralValueMagnetic->energy, 0, 'e', 3), tr("J"));

        // force
        QTreeWidgetItem *itemForce = new QTreeWidgetItem(magneticNode);
        itemForce->setText(0, tr("Lorentz force avg."));
        itemForce->setExpanded(true);

        addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelX(), QString("%1").arg(volumeIntegralValueMagnetic->forceLorentzX, 0, 'e', 3), "N");
        addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelY(), QString("%1").arg(volumeIntegralValueMagnetic->forceLorentzY, 0, 'e', 3), "N");

        addTreeWidgetItemValue(magneticNode, tr("Torque:"), QString("%1").arg(volumeIntegralValueMagnetic->torque, 0, 'e', 3), tr("Nm"));
    }
    else
    {
        addTreeWidgetItemValue(magneticNode, tr("External current:"), QString("%1").arg(volumeIntegralValueMagnetic->currentReal, 0, 'e', 3), tr("A"));
        addTreeWidgetItemValue(magneticNode, tr("Transform current:"), QString("%1").arg(volumeIntegralValueMagnetic->currentInducedTransformReal, 0, 'e', 3), tr("A"));
        addTreeWidgetItemValue(magneticNode, tr("Velocity current:"), QString("%1").arg(volumeIntegralValueMagnetic->currentInducedVelocityReal, 0, 'e', 3), tr("A"));
        addTreeWidgetItemValue(magneticNode, tr("Total current:"), QString("%1").arg(volumeIntegralValueMagnetic->currentTotalReal, 0, 'e', 3), tr("A"));
        addTreeWidgetItemValue(magneticNode, tr("Power Losses:"), QString("%1").arg(volumeIntegralValueMagnetic->powerLosses, 0, 'e', 3), tr("W"));
        addTreeWidgetItemValue(magneticNode, tr("Energy:"), QString("%1").arg(volumeIntegralValueMagnetic->energy, 0, 'e', 3), tr("J"));

        QTreeWidgetItem *itemForce = new QTreeWidgetItem(magneticNode);
        itemForce->setText(0, tr("Force"));
        itemForce->setExpanded(true);

        addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelX(), QString("%1").arg(volumeIntegralValueMagnetic->forceLorentzX, 0, 'e', 3), tr("N"));
        addTreeWidgetItemValue(itemForce, Util::scene()->problemInfo()->labelY(), QString("%1").arg(volumeIntegralValueMagnetic->forceLorentzY, 0, 'e', 3), tr("N"));

        addTreeWidgetItemValue(magneticNode, tr("Torque:"), QString("%1").arg(volumeIntegralValueMagnetic->torque, 0, 'e', 3), tr("Nm"));
    }
}

ViewScalarFilter *HermesMagnetic::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
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

Point3 HermesMagnetic::particleForce(Point point, double angle, Point3 velocity)
{
    LocalPointValueMagnetic *pointValue = dynamic_cast<LocalPointValueMagnetic *>(localPointValue(point));

    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        return Point3(- velocity.z * pointValue->B_real.y,
                      velocity.z * pointValue->B_real.x,
                      velocity.x * pointValue->B_real.y - velocity.y * pointValue->B_real.x);
    }
    else
    {
        Point3 fluxPlanar(pointValue->B_real.x * cos(angle),
                          pointValue->B_real.y,
                          pointValue->B_real.x * sin(angle));

        Point3 forcePlanar(velocity.y * fluxPlanar.z - velocity.z * fluxPlanar.y,
                           velocity.z * fluxPlanar.x - velocity.x * fluxPlanar.z,
                           velocity.x * fluxPlanar.y - velocity.y * fluxPlanar.x);

        return forcePlanar;
    }
}

double HermesMagnetic::particleMaterial(Point point)
{
    LocalPointValueMagnetic *pointValue = dynamic_cast<LocalPointValueMagnetic *>(localPointValue(point));

    return pointValue->permeability;
}

QList<SolutionArray *> HermesMagnetic::solve(ProgressItemSolve *progressItemSolve)
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return QList<SolutionArray *>();
    }

    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return QList<SolutionArray *>();
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return QList<SolutionArray *>();
    }

    // edge markers
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryMagnetic *boundary = dynamic_cast<SceneBoundaryMagnetic *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value_real.evaluate()) return QList<SolutionArray *>();
        if (!boundary->value_imag.evaluate()) return QList<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->current_density_real.evaluate()) return QList<SolutionArray *>();
        if (!material->current_density_imag.evaluate()) return QList<SolutionArray *>();
        if (!material->permeability.evaluate()) return QList<SolutionArray *>();
        if (!material->conductivity.evaluate()) return QList<SolutionArray *>();
        if (!material->remanence.evaluate()) return QList<SolutionArray *>();
        if (!material->remanence_angle.evaluate()) return QList<SolutionArray *>();
        if (!material->velocity_x.evaluate()) return QList<SolutionArray *>();
        if (!material->velocity_y.evaluate()) return QList<SolutionArray *>();
        if (!material->velocity_angular.evaluate()) return QList<SolutionArray *>();
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

    WeakFormMagnetic wf(numberOfSolution());

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

void HermesMagnetic::updateTimeFunctions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialMagnetic *material = dynamic_cast<SceneMaterialMagnetic *>(Util::scene()->materials[i]);

        material->current_density_real.evaluate(time);
        material->current_density_imag.evaluate(time);
    }
}

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
