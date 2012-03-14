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

#include "hermes_acoustic.h"

#include "scene.h"
#include "gui.h"

class CustomMatrixFormSurfMatchedBoundary : public WeakForm::MatrixFormSurf
{
public:
    CustomMatrixFormSurfMatchedBoundary(int i, int j, scalar coeff, GeomType gt = HERMES_PLANAR)
        : WeakForm::MatrixFormSurf(i, j), coeff(coeff), gt(gt) { }
    CustomMatrixFormSurfMatchedBoundary(int i, int j, std::string area, scalar coeff, GeomType gt = HERMES_PLANAR)
        : WeakForm::MatrixFormSurf(i, j, area), coeff(coeff), gt(gt) { }

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                         Geom<double> *e, ExtData<scalar> *ext) const {
        scalar result = 0;
        if (gt == HERMES_PLANAR) result = int_u_v<double, scalar>(n, wt, u, v);
        else if (gt == HERMES_AXISYM_X) result = int_y_u_v<double, scalar>(n, wt, u, v, e);
        else result = int_x_u_v<double, scalar>(n, wt, u, v, e);

        SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->elem_marker).c_str())]->material);
        return coeff / (material->density.number * material->speed.number) * result;
    }

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u,
                    Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
        Ord result = 0;
        if (gt == HERMES_PLANAR) result = int_u_v<Ord, Ord>(n, wt, u, v);
        else if (gt == HERMES_AXISYM_X) result = int_y_u_v<Ord, Ord>(n, wt, u, v, e);
        else result = int_x_u_v<Ord, Ord>(n, wt, u, v, e);
        return result;
    }

    // This is to make the form usable in rk_time_step().
    virtual WeakForm::MatrixFormSurf* clone() {
        return new CustomMatrixFormSurfMatchedBoundary(*this);
    }

private:
    scalar coeff;
    GeomType gt;
};

class WeakFormAcousticsHarmonic : public WeakFormAgros
{
public:
    WeakFormAcousticsHarmonic() : WeakFormAgros(2) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == PhysicFieldBC_Acoustic_NormalAcceleration)
                    if (fabs(boundary->value_real.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->value_real.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                if (boundary->type == PhysicFieldBC_Acoustic_Impedance)
                {
                    if (fabs(boundary->value_real.number) > EPS_ZERO)
                    {
                        add_matrix_form_surf(new WeakFormsH1::SurfaceMatrixForms::DefaultMatrixFormSurf(0, 1,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        - 2 * M_PI * Util::scene()->problemInfo()->frequency / boundary->value_real.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                        add_matrix_form_surf(new WeakFormsH1::SurfaceMatrixForms::DefaultMatrixFormSurf(1, 0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        2 * M_PI * Util::scene()->problemInfo()->frequency / boundary->value_real.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                    }
                }

                if (boundary->type == PhysicFieldBC_Acoustic_MatchedBoundary)
                {
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(0, 1,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 - 2 * M_PI * Util::scene()->problemInfo()->frequency,
                                                                                 convertProblemType(Util::scene()->problemInfo()->problemType)));
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(1, 0,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 2 * M_PI * Util::scene()->problemInfo()->frequency,
                                                                                 convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                // real part
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               1.0 / material->density.number,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                          QString::number(i).toStdString(),
                                                                                          - sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) / (material->density.number * sqr(material->speed.number)),
                                                                                          HERMES_SYM,
                                                                                          convertProblemType(Util::scene()->problemInfo()->problemType)));

                // imag part
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(1, 1,
                                                                                               QString::number(i).toStdString(),
                                                                                               1.0 / material->density.number,
                                                                                               HERMES_SYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 1,
                                                                                          QString::number(i).toStdString(),
                                                                                          - sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) / (material->density.number * sqr(material->speed.number)),
                                                                                          HERMES_SYM,
                                                                                          convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }
    }
};

// time dependent

// - 1/rho * \Delta p + 1/(rho * c^2) * \frac{\partial v}{\partial t} = 0.
// - v                +                 \frac{\partial p}{\partial t} = 0,

class WeakFormAcousticsTransient : public WeakFormAgros
{
public:
    WeakFormAcousticsTransient() : WeakFormAgros(2) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == PhysicFieldBC_Acoustic_NormalAcceleration)
                    if (fabs(boundary->value_real.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->value_real.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                if (boundary->type == PhysicFieldBC_Acoustic_Impedance)
                {
                    /*
                    if (fabs(boundary->value_real.number) > EPS_ZERO)
                    {
                        add_matrix_form_surf(new WeakFormsH1::SurfaceMatrixForms::DefaultMatrixFormSurf(0, 1,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        - 2 * M_PI * Util::scene()->problemInfo()->frequency / boundary->value_real.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                        add_matrix_form_surf(new WeakFormsH1::SurfaceMatrixForms::DefaultMatrixFormSurf(1, 0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        2 * M_PI * Util::scene()->problemInfo()->frequency / boundary->value_real.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                    }
                    */
                }

                if (boundary->type == PhysicFieldBC_Acoustic_MatchedBoundary)
                {
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(1, 0,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 100,
                                                                                 convertProblemType(Util::scene()->problemInfo()->problemType)));
                    /*
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(1, 0,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 - 1.0,
                                                                                 convertProblemType(Util::scene()->problemInfo()->problemType)));
                    */
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               1.0 / material->density.number,
                                                                                               HERMES_NONSYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 1,
                                                                                          QString::number(i).toStdString(),
                                                                                          1.0 / (material->density.number * sqr(material->speed.number)) / Util::scene()->problemInfo()->timeStep.number,
                                                                                          HERMES_NONSYM,
                                                                                          convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 0,
                                                                                          QString::number(i).toStdString(),
                                                                                          - 1.0 / Util::scene()->problemInfo()->timeStep.number,
                                                                                          HERMES_NONSYM,
                                                                                          convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 1,
                                                                                          QString::number(i).toStdString(),
                                                                                          1.0,
                                                                                          HERMES_NONSYM,
                                                                                          convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_vector_form(new CustomVectorFormTimeDep(0,
                                                            QString::number(i).toStdString(),
                                                            1.0 / (material->density.number * sqr(material->speed.number)) / Util::scene()->problemInfo()->timeStep.number,
                                                            solution[0],
                                                            convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_vector_form(new CustomVectorFormTimeDep(1,
                                                            QString::number(i).toStdString(),
                                                            1.0 / Util::scene()->problemInfo()->timeStep.number,
                                                            solution[1],
                                                            convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }
    }

private:
    class VectorFormVolWaveMass : public WeakForm::VectorFormVol
    {
    public:
        VectorFormVolWaveMass() : WeakForm::VectorFormVol(0) { }

        template<typename Real, typename Scalar>
        Scalar vector_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *v,
                           Geom<Real> *e, ExtData<Scalar> *ext) const {
            Scalar result = 0;

            for (int i = 0; i < n; i++)
                result += wt[i] * u_ext[1]->val[i] * v->val[i];

            return result;
        }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {
            return vector_form<double, scalar>(n, wt, u_ext, v, e, ext);
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e,
                        ExtData<Ord> *ext) const {
            return vector_form<Ord, Ord>(n, wt, u_ext, v, e, ext);
        }

        virtual WeakForm::VectorFormVol* clone() {
            return new VectorFormVolWaveMass(*this);
        }
    };

    class VectorFormVolWaveDiffusion : public WeakForm::VectorFormVol
    {
    public:
        VectorFormVolWaveDiffusion(double c_squared) : WeakForm::VectorFormVol(1), c_squared(c_squared) { }

        template<typename Real, typename Scalar>
        Scalar vector_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *v,
                           Geom<Real> *e, ExtData<Scalar> *ext) const {
            Scalar result = 0;

            for (int i = 0; i < n; i++)
                result += wt[i] * (u_ext[0]->dx[i] * v->dx[i] + u_ext[0]->dy[i] * v->dy[i]);

            return - c_squared * result;
        }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {
            return vector_form<double, scalar>(n, wt, u_ext, v, e, ext);
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e,
                        ExtData<Ord> *ext) const {
            return vector_form<Ord, Ord>(n, wt, u_ext, v, e, ext);
        }

        virtual WeakForm::VectorFormVol* clone() {
            return new VectorFormVolWaveDiffusion(*this);
        }

        double c_squared;
    };    
};

class CustomAcousticPressureBC : public EssentialBoundaryCondition
{
public:
    /// Constructors.
    CustomAcousticPressureBC(std::string marker, SceneBoundaryAcoustic *boundary) : EssentialBoundaryCondition(marker)
    {
        m_boundary = boundary;
    }

    /// Function reporting the type of the essential boundary condition.
    inline EssentialBCValueType get_value_type() const { return EssentialBoundaryCondition::BC_FUNCTION; }
    virtual scalar value(double x, double y, double n_x, double n_y, double t_x, double t_y) const
    {
        return m_boundary->value_real.number;
    }

private:
    SceneBoundaryAcoustic *m_boundary;
};

// ****************************************************************************************************

PhysicFieldVariable HermesAcoustic::contourPhysicFieldVariable()
{
    return PhysicFieldVariable_Acoustic_PressureReal;
}

PhysicFieldVariable HermesAcoustic::scalarPhysicFieldVariable()
{
    return PhysicFieldVariable_Acoustic_PressureReal;
}

PhysicFieldVariableComp HermesAcoustic::scalarPhysicFieldVariableComp()
{
    return PhysicFieldVariableComp_Scalar;
}

PhysicFieldVariable HermesAcoustic::vectorPhysicFieldVariable()
{
    return PhysicFieldVariable_Acoustic_LocalAcceleration;
}


void HermesAcoustic::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Acoustic_Pressure:
        Util::scene()->addBoundary(new SceneBoundaryAcoustic(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("pressure", "0"))));
        break;
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        Util::scene()->addBoundary(new SceneBoundaryAcoustic(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("acceleration", "0"))));
        break;
    case PhysicFieldBC_Acoustic_Impedance:
        Util::scene()->addBoundary(new SceneBoundaryAcoustic(element->attribute("name"),
                                                             type,
                                                             Value(element->attribute("impedance", "0"))));
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
        Util::scene()->addBoundary(new SceneBoundaryAcoustic(element->attribute("name"),
                                                             type));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesAcoustic::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(boundary->type));

    switch (boundary->type)
    {
    case PhysicFieldBC_Acoustic_Pressure:
        element->setAttribute("pressure", boundary->value_real.text);
        break;
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        element->setAttribute("acceleration", boundary->value_real.text);
        break;
    case PhysicFieldBC_Acoustic_Impedance:
        element->setAttribute("impedance", boundary->value_real.text);
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }

}

void HermesAcoustic::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialAcoustic(element->attribute("name"),
                                                         Value(element->attribute("density", "1.25")),
                                                         Value(element->attribute("speed", "343"))));
}

void HermesAcoustic::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(marker);

    element->setAttribute("density", material->density.text);
    element->setAttribute("speed", material->speed.text);
}

LocalPointValue *HermesAcoustic::localPointValue(const Point &point)
{
    return new LocalPointValueAcoustic(point);
}

QStringList HermesAcoustic::localPointValueHeader()
{
    QStringList headers;
    headers << "X" << "Y" << "p_real" << "p_imag" << "p" << "Lp" << "rho" << "c";
    return QStringList(headers);
}

SurfaceIntegralValue *HermesAcoustic::surfaceIntegralValue()
{
    return new SurfaceIntegralValueAcoustic();
}

QStringList HermesAcoustic::surfaceIntegralValueHeader()
{
    QStringList headers;
    headers << "l" << "S" << "p_real" << "p_imag";
    return QStringList(headers);
}

VolumeIntegralValue *HermesAcoustic::volumeIntegralValue()
{
    return new VolumeIntegralValueAcoustic();
}

QStringList HermesAcoustic::volumeIntegralValueHeader()
{
    QStringList headers;
    headers << "V" << "S" << "p_real" << "p_imag" << "W" ;
    return QStringList(headers);
}

SceneBoundary *HermesAcoustic::newBoundary()
{
    return new SceneBoundaryAcoustic(tr("new boundary"),
                                     PhysicFieldBC_Acoustic_Pressure,
                                     Value("0"));
}

SceneBoundary *HermesAcoustic::newBoundary(PyObject *self, PyObject *args)
{
    double pressure;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ss|d", &name, &type, &pressure))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        if (physicFieldBCFromStringKey(type) == PhysicFieldBC_Acoustic_MatchedBoundary)
            return new SceneBoundaryAcoustic(name,
                                             physicFieldBCFromStringKey(type));
        else
            return new SceneBoundaryAcoustic(name,
                                             physicFieldBCFromStringKey(type),
                                             Value(QString::number(pressure)));
    }

    return NULL;
}

SceneBoundary *HermesAcoustic::modifyBoundary(PyObject *self, PyObject *args)
{
    double pressure;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd", &name, &type, &pressure))
    {
        if (SceneBoundaryAcoustic *marker = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(type))
            {
                marker->type = physicFieldBCFromStringKey(type);
                if (marker->type == PhysicFieldBC_Acoustic_Pressure)
                {
                    marker->value_real = Value(QString::number(pressure));
                }
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

SceneMaterial *HermesAcoustic::newMaterial()
{
    return new SceneMaterialAcoustic(tr("new material"),
                                     Value("1.25"),
                                     Value("343"));
}

SceneMaterial *HermesAcoustic::newMaterial(PyObject *self, PyObject *args)
{
    double density, speed;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &density, &speed))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialAcoustic(name,
                                         Value(QString::number(density)),
                                         Value(QString::number(speed)));
    }

    return NULL;
}

SceneMaterial *HermesAcoustic::modifyMaterial(PyObject *self, PyObject *args)
{
    double density, speed;
    char *name;
    if (PyArg_ParseTuple(args, "sdd", &name, &density, &speed))
    {
        if (SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(Util::scene()->getMaterial(name)))
        {
            marker->density = Value(QString::number(density));
            marker->speed = Value(QString::number(speed));
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

void HermesAcoustic::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    // harmonic
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Pressure), PhysicFieldVariable_Acoustic_Pressure);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureReal), PhysicFieldVariable_Acoustic_PressureReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureImag), PhysicFieldVariable_Acoustic_PressureImag);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureLevel), PhysicFieldVariable_Acoustic_PressureLevel);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalVelocity), PhysicFieldVariable_Acoustic_LocalVelocity);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalAcceleration), PhysicFieldVariable_Acoustic_LocalAcceleration);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Density), PhysicFieldVariable_Acoustic_Density);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Speed), PhysicFieldVariable_Acoustic_Speed);
    }
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Pressure), PhysicFieldVariable_Acoustic_PressureReal);

        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureReal), PhysicFieldVariable_Acoustic_PressureReal);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureImag), PhysicFieldVariable_Acoustic_PressureImag);

        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_PressureLevel), PhysicFieldVariable_Acoustic_PressureLevel);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalVelocity), PhysicFieldVariable_Acoustic_LocalVelocity);
        // cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalAcceleration), PhysicFieldVariable_Acoustic_LocalAcceleration);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Density), PhysicFieldVariable_Acoustic_Density);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_Speed), PhysicFieldVariable_Acoustic_Speed);
    }
}

void HermesAcoustic::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Acoustic_LocalAcceleration), PhysicFieldVariable_Acoustic_LocalAcceleration);
}

void HermesAcoustic::showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue)
{
    LocalPointValueAcoustic *localPointValueAcoustic = dynamic_cast<LocalPointValueAcoustic *>(localPointValue);

    // magnetic
    QTreeWidgetItem *acousticNode = new QTreeWidgetItem(trvWidget);
    acousticNode->setText(0, tr("Acoustic field"));
    acousticNode->setExpanded(true);

    // material
    addTreeWidgetItemValue(acousticNode, tr("Density:"), QString("%1").arg(localPointValueAcoustic->density, 0, 'f', 2), "kg/m3");
    addTreeWidgetItemValue(acousticNode, tr("Speed of sound:"), QString("%1").arg(localPointValueAcoustic->speed, 0, 'f', 2), "m/s");

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        // Pressure
        QTreeWidgetItem *itemPressure = new QTreeWidgetItem(acousticNode);
        itemPressure->setText(0, tr("Acoustic pressure"));
        itemPressure->setExpanded(true);

        addTreeWidgetItemValue(itemPressure, tr("real:"), QString("%1").arg(localPointValueAcoustic->pressure_real, 0, 'e', 3), "Pa");
        addTreeWidgetItemValue(itemPressure, tr("imag:"), QString("%1").arg(localPointValueAcoustic->pressure_imag, 0, 'e', 3), "Pa");
        addTreeWidgetItemValue(itemPressure, tr("magnitude:"), QString("%1").arg(sqrt(sqr(localPointValueAcoustic->pressure_real) + sqr(localPointValueAcoustic->pressure_imag)), 0, 'e', 3), "Pa");

        addTreeWidgetItemValue(acousticNode, tr("Sound pressure level:"), QString("%1").arg(localPointValueAcoustic->pressureLevel, 0, 'f', 2), "dB");

        // Local velocity
        /*
        QTreeWidgetItem *itemLocalVelocity = new QTreeWidgetItem(pressureNode);
        itemLocalVelocity->setText(0, tr("Local velocity"));
        itemLocalVelocity->setExpanded(true);

        addTreeWidgetItemValue(itemLocalVelocity, "v" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.x, 0, 'f', 5), tr("m/s"));
        addTreeWidgetItemValue(itemLocalVelocity, "v" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.y, 0, 'f', 5), tr("m/s"));
        addTreeWidgetItemValue(itemLocalVelocity, "v:", QString("%1").arg(localPointValueAcoustic->localAccelaration.magnitude(), 0, 'f', 5), "m/s");

        // Local acceleration
        QTreeWidgetItem *itemLocalAcceleration = new QTreeWidgetItem(pressureNode);
        itemLocalAcceleration->setText(0, tr("Local acceleration"));
        itemLocalAcceleration->setExpanded(true);

        addTreeWidgetItemValue(itemLocalAcceleration, "a" + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.x, 0, 'f', 5), tr("m/s2"));
        addTreeWidgetItemValue(itemLocalAcceleration, "a" + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(localPointValueAcoustic->localAccelaration.y, 0, 'f', 5), tr("m/s2"));
        addTreeWidgetItemValue(itemLocalAcceleration, "a:", QString("%1").arg(localPointValueAcoustic->localAccelaration.magnitude(), 0, 'f', 5), "m/s2");
        */
    }
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        // Pressure
        addTreeWidgetItemValue(acousticNode, tr("Acoustic pressure:"), QString("%1").arg(localPointValueAcoustic->pressure_real, 0, 'e', 3), "Pa");
        addTreeWidgetItemValue(acousticNode, tr("Sound pressure level:"), QString("%1").arg(localPointValueAcoustic->pressureLevel, 0, 'f', 2), "dB");
    }
}

void HermesAcoustic::showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue)
{
    SurfaceIntegralValueAcoustic *surfaceIntegralValueAcoustic = dynamic_cast<SurfaceIntegralValueAcoustic *>(surfaceIntegralValue);

    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Acoustic field"));
    magneticNode->setExpanded(true);

    // pressure
    QTreeWidgetItem *itemPressure = new QTreeWidgetItem(magneticNode);
    itemPressure->setText(0, tr("Pressure"));
    itemPressure->setExpanded(true);

    addTreeWidgetItemValue(itemPressure, tr("real:"), QString("%1").arg(surfaceIntegralValueAcoustic->pressureReal, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(itemPressure, tr("imag:"), QString("%1").arg(surfaceIntegralValueAcoustic->pressureImag, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(itemPressure, tr("magnitude:"), QString("%1").arg(sqrt(sqr(surfaceIntegralValueAcoustic->pressureReal) + sqr(surfaceIntegralValueAcoustic->pressureImag)), 0, 'e', 3), "Pa");
}

void HermesAcoustic::showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue)
{
    VolumeIntegralValueAcoustic *volumeIntegralValueAcoustic = dynamic_cast<VolumeIntegralValueAcoustic *>(volumeIntegralValue);

    // harmonic
    QTreeWidgetItem *magneticNode = new QTreeWidgetItem(trvWidget);
    magneticNode->setText(0, tr("Acoustic field"));
    magneticNode->setExpanded(true);

    // pressure
    QTreeWidgetItem *itemPressure = new QTreeWidgetItem(magneticNode);
    itemPressure->setText(0, tr("Pressure"));
    itemPressure->setExpanded(true);

    addTreeWidgetItemValue(itemPressure, tr("real:"), QString("%1").arg(volumeIntegralValueAcoustic->pressureReal, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(itemPressure, tr("imag:"), QString("%1").arg(volumeIntegralValueAcoustic->pressureImag, 0, 'e', 3), "Pa");
    addTreeWidgetItemValue(itemPressure, tr("magnitude:"), QString("%1").arg(sqrt(sqr(volumeIntegralValueAcoustic->pressureReal) + sqr(volumeIntegralValueAcoustic->pressureImag)), 0, 'e', 3), "Pa");

    // energy
    QTreeWidgetItem *itemEnergy = new QTreeWidgetItem(magneticNode);
    itemEnergy->setText(0, tr("Sound Energy"));
    itemEnergy->setExpanded(true);

    addTreeWidgetItemValue(itemEnergy, tr("Energy:"), QString("%1").arg(volumeIntegralValueAcoustic->pressureEnergy, 0, 'e', 3), "J/m3");
    addTreeWidgetItemValue(itemEnergy, tr("Energy Density Level:"), QString("%1").arg(((volumeIntegralValueAcoustic->pressureEnergy > SOUND_ENERGY_DENSITY_REF) ? 10.0 * log10(volumeIntegralValueAcoustic->pressureEnergy / SOUND_ENERGY_DENSITY_REF) : 0.0), 0, 'e', 3), "dB");

}

ViewScalarFilter *HermesAcoustic::viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    Solution *sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
    return new ViewScalarFilterAcoustic(Hermes::vector<MeshFunction *>(sln1, sln2),
                                        physicFieldVariable,
                                        physicFieldVariableComp);
}

QList<SolutionArray *> HermesAcoustic::solve(ProgressItemSolve *progressItemSolve)
{
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
        SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value_real.evaluate()) return QList<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->density.evaluate()) return QList<SolutionArray *>();
        if (!material->speed.evaluate()) return QList<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bc1;
    EssentialBCs bc2;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->type == PhysicFieldBC_Acoustic_Pressure)
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->value_real.number));
                    bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), 0.0));
                }
                else
                {
                    bc1.add_boundary_condition(new CustomAcousticPressureBC(QString::number(i+1).toStdString(), boundary));
                    bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), 0.0));
                }
            }
        }
    }
    Hermes::vector<EssentialBCs> bcs(bc1, bc2);

    WeakFormAgros *wf;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        wf = new WeakFormAcousticsHarmonic();
    else
        wf = new WeakFormAcousticsTransient();

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, wf);

    delete wf;

    return solutionArrayList;
}

void HermesAcoustic::updateTimeFunctions(double time)
{
    // update boundaries
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->boundaries[i]);
        boundary->value_real.evaluate(time);
    }
}

// ****************************************************************************************************************

LocalPointValueAcoustic::LocalPointValueAcoustic(const Point &point) : LocalPointValue(point)
{
    density = 0;
    speed = 0;
    pressure_real = 0;
    pressure_imag = 0;
    pressureLevel = 0;
    localAccelaration = Point();
    localVelocity = Point();

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // value real
        PointValue valueReal = PointValue(value, derivative, material);

        SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(valueReal.marker);
        // solution
        if (marker)
        {            
            density = marker->density.number;
            speed = marker->speed.number;

            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            {
                Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

                // value imag
                PointValue valueImag = pointValue(sln2, point);
                double frequency = Util::scene()->problemInfo()->frequency;

                Point derReal = valueReal.derivative;
                Point derImag = valueImag.derivative;

                pressure_real = valueReal.value;
                pressure_imag = valueImag.value;

                localAccelaration.x = - derReal.x / density;
                localAccelaration.y = - derReal.y / density;

                localVelocity.x = localAccelaration.x / (2 * M_PI * frequency);
                localVelocity.y = localAccelaration.y / (2 * M_PI * frequency);

                pressureLevel = (sqrt(sqr(valueReal.value) + sqr(valueImag.value)) > PRESSURE_MIN_AIR) ?
                            20.0 * log10(sqrt(sqr(valueReal.value) + sqr(valueImag.value)) / sqrt(2.0) / PRESSURE_MIN_AIR) : 0.0;
            }
            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            {
                Point derReal = valueReal.derivative;

                pressure_real = valueReal.value;

                pressureLevel = (valueReal.value > PRESSURE_MIN_AIR) ? 20.0 * log10(valueReal.value / PRESSURE_MIN_AIR) : 0.0;
            }
        }
    }
}

double LocalPointValueAcoustic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Acoustic_Pressure:
        return sqrt(sqr(pressure_real) + sqr(pressure_imag));
    case PhysicFieldVariable_Acoustic_PressureReal:
        return pressure_real;
    case PhysicFieldVariable_Acoustic_PressureImag:
        return pressure_imag;
    case PhysicFieldVariable_Acoustic_LocalVelocity:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return localVelocity.x;
        case PhysicFieldVariableComp_Y:
            return localVelocity.y;
        case PhysicFieldVariableComp_Magnitude:
            return localVelocity.magnitude();
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_LocalAcceleration:
    {
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return localAccelaration.x;
        case PhysicFieldVariableComp_Y:
            return localAccelaration.y;
        case PhysicFieldVariableComp_Magnitude:
            return localAccelaration.magnitude();
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureLevel:
        return pressureLevel;
    case PhysicFieldVariable_Acoustic_Density:
        return density;
    case PhysicFieldVariable_Acoustic_Speed:
        return speed;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueAcoustic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueAcoustic::variables()
{
    QStringList row;
    row << QString("%1").arg(point.x, 0, 'e', 5) <<
           QString("%1").arg(point.y, 0, 'e', 5) <<
           QString("%1").arg(pressure_real, 0, 'e', 5) <<
           QString("%1").arg(pressure_imag, 0, 'e', 5) <<
           QString("%1").arg(sqrt(sqr(pressure_real) + sqr(pressure_imag)), 0, 'e', 5) <<
           QString("%1").arg(pressureLevel, 0, 'e', 5) <<
           QString("%1").arg(density, 0, 'f', 2) <<
           QString("%1").arg(speed, 0, 'f', 2);
    // QString("%1").arg(localAccelaration.x, 0, 'e', 5) <<
    // QString("%1").arg(localAccelaration.y, 0, 'e', 5);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueAcoustic::SurfaceIntegralValueAcoustic() : SurfaceIntegralValue()
{
    pressureReal = 0.0;
    pressureImag = 0.0;

    calculate();

    pressureReal /= 2.0;
    pressureImag /= 2.0;
}

void SurfaceIntegralValueAcoustic::calculateVariables(int i)
{
    SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(material);

    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        pressureReal += pt[i][2] * tan[i][2] * value1[i];
    else
        pressureReal += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * value1[i];

    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        pressureImag += pt[i][2] * tan[i][2] * value2[i];
    else
        pressureImag += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * value2[i];
}

void SurfaceIntegralValueAcoustic::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
}

QStringList SurfaceIntegralValueAcoustic::variables()
{
    QStringList row;
    row << QString("%1").arg(length, 0, 'e', 5) <<
           QString("%1").arg(surface, 0, 'e', 5) <<
           QString("%1").arg(pressureReal, 0, 'e', 5) <<
           QString("%1").arg(pressureImag, 0, 'e', 5);
    return QStringList(row);
}


// ****************************************************************************************************************

VolumeIntegralValueAcoustic::VolumeIntegralValueAcoustic() : VolumeIntegralValue()
{
    pressureReal = 0.0;
    pressureImag = 0.0;
    pressureEnergy = 0.0;

    calculate();
}

void VolumeIntegralValueAcoustic::calculateVariables(int i)
{
    SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(material);

    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        h1_integrate_expression(value1[i]);
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * value1[i]);
    }
    pressureReal += result;

    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        h1_integrate_expression(value2[i]);
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * value2[i]);
    }
    pressureImag += result;

    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        h1_integrate_expression(sqrt(sqr(value1[i]) + sqr(value2[i])) / (2 * (marker->density.number) * sqr(marker->speed.number)));
    }
    else
    {
        h1_integrate_expression(2 * M_PI * x[i] * (sqrt(sqr(value1[i]) + sqr(value2[i])) / (2 * (marker->density.number) * sqr(marker->speed.number))));
    }
    pressureEnergy += result;
}

void VolumeIntegralValueAcoustic::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
}

QStringList VolumeIntegralValueAcoustic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
           QString("%1").arg(crossSection, 0, 'e', 5) <<
           QString("%1").arg(pressureReal, 0, 'e', 5) <<
           QString("%1").arg(pressureImag, 0, 'e', 5) <<
           QString("%1").arg(pressureEnergy, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterAcoustic::calculateVariable(int i)
{
    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Acoustic_Pressure:
    {
        node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureReal:
    {
        node->values[0][0][i] = value1[i];
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureImag:
    {
        node->values[0][0][i] = value2[i];
    }
        break;
    case PhysicFieldVariable_Acoustic_PressureLevel:
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            node->values[0][0][i] = (sqrt(sqr(value1[i]) + sqr(value2[i])) > PRESSURE_MIN_AIR) ?
                        20.0 * log10(sqrt(sqr(value1[i]) + sqr(value2[i])) / sqrt(2.0) / PRESSURE_MIN_AIR) : 0.0;
        else if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            node->values[0][0][i] = (value1[i] > PRESSURE_MIN_AIR) ?
                        20.0 * log10(value1[i] / PRESSURE_MIN_AIR) : 0.0;
    }
        break;
    case PhysicFieldVariable_Acoustic_LocalVelocity:
    {
        SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(material);

        switch (m_physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
        {
            node->values[0][0][i] = - dudx1[i] / marker->density.number / (2 * M_PI * Util::scene()->problemInfo()->frequency);
        }
            break;
        case PhysicFieldVariableComp_Y:
        {
            node->values[0][0][i] = - dudy1[i] / marker->density.number / (2 * M_PI * Util::scene()->problemInfo()->frequency);
        }
            break;
        case PhysicFieldVariableComp_Magnitude:
        {
            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i])) / marker->density.number / (2 * M_PI * Util::scene()->problemInfo()->frequency);;
        }
            break;
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_LocalAcceleration:
    {
        SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(material);

        switch (m_physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
        {
            node->values[0][0][i] = - dudx1[i] / marker->density.number;
        }
            break;
        case PhysicFieldVariableComp_Y:
        {
            node->values[0][0][i] = - dudy1[i] / marker->density.number;
        }
            break;
        case PhysicFieldVariableComp_Magnitude:
        {
            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i])) / marker->density.number;
        }
            break;
        }
    }
        break;
    case PhysicFieldVariable_Acoustic_Density:
    {
        SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(material);
        node->values[0][0][i] = marker->density.number;
    }
        break;
    case PhysicFieldVariable_Acoustic_Speed:
    {
        SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(material);
        node->values[0][0][i] = marker->speed.number;
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterAcoustic::calculateVariable()" << endl;
        throw;
        break;
    }
}

// *************************************************************************************************************************************

SceneBoundaryAcoustic::SceneBoundaryAcoustic(const QString &name, PhysicFieldBC type, Value value_real) : SceneBoundary(name, type)
{
    this->value_real = value_real;
}

SceneBoundaryAcoustic::SceneBoundaryAcoustic(const QString &name, PhysicFieldBC type) : SceneBoundary(name, type)
{
}

QString SceneBoundaryAcoustic::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value_real.text);
}

QMap<QString, QString> SceneBoundaryAcoustic::data()
{
    QMap<QString, QString> out;
    switch (type)
    {
    case PhysicFieldBC_Acoustic_Pressure:
        out["Pressure (Pa)"] = value_real.text;
        break;
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        out["Normal acceleration (m/s2)"] = value_real.text;
        break;
    case PhysicFieldBC_Acoustic_Impedance:
        out["Input impedance (Pa.s/m)"] = value_real.text;
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
        break;
    }
    return QMap<QString, QString>(out);
}

int SceneBoundaryAcoustic::showDialog(QWidget *parent)
{
    SceneBoundaryAcousticDialog *dialog = new SceneBoundaryAcousticDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialAcoustic::SceneMaterialAcoustic(const QString &name, Value density, Value speed)
    : SceneMaterial(name)
{
    this->density = density;
    this->speed = speed;
}

QString SceneMaterialAcoustic::script()
{
    return QString("addmaterial(\"%1\", %2, %3)").
            arg(name).
            arg(density.text).
            arg(speed.text);
}

QMap<QString, QString> SceneMaterialAcoustic::data()
{
    QMap<QString, QString> out;
    out["Density (-)"] = density.text;
    out["Speed (-)"] = speed.text;
    return QMap<QString, QString>(out);
}

int SceneMaterialAcoustic::showDialog(QWidget *parent)
{
    SceneMaterialAcousticDialog *dialog = new SceneMaterialAcousticDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryAcousticDialog::SceneBoundaryAcousticDialog(SceneBoundaryAcoustic *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryAcousticDialog::createContent()
{
    lblValueUnit = new QLabel("");

    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_Pressure), PhysicFieldBC_Acoustic_Pressure);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_NormalAcceleration), PhysicFieldBC_Acoustic_NormalAcceleration);
    // FIX impedance boundary for transient
    // if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_MatchedBoundary), PhysicFieldBC_Acoustic_MatchedBoundary);
        cmbType->addItem(physicFieldBCString(PhysicFieldBC_Acoustic_Impedance), PhysicFieldBC_Acoustic_Impedance);
    }
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValue = new ValueLineEdit(this, true);

    connect(txtValue, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnit, 11, 0);
    layout->addWidget(txtValue, 11, 2);
}

void SceneBoundaryAcousticDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(m_boundary);

    cmbType->setCurrentIndex(cmbType->findData(boundary->type));
    txtValue->setValue(boundary->value_real);
}

bool SceneBoundaryAcousticDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(m_boundary);

    boundary->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValue->evaluate())
        boundary->value_real = txtValue->value();
    else
        return false;

    return true;
}

void SceneBoundaryAcousticDialog::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, (PhysicFieldBC) cmbType->itemData(index).toInt());

    // enable controls
    switch ((PhysicFieldBC) cmbType->itemData(index).toInt())
    {
    case PhysicFieldBC_Acoustic_Pressure:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>p</i><sub>0</sub> (Pa)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    case PhysicFieldBC_Acoustic_NormalAcceleration:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>a</i><sub>0</sub> (m/s<sup>2</sup>)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    case PhysicFieldBC_Acoustic_Impedance:
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>Z</i><sub>0</sub> (Pa·s/m)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    case PhysicFieldBC_Acoustic_MatchedBoundary:
    {
        lblValueUnit->setText("-");
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
        break;
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialAcousticDialog::SceneMaterialAcousticDialog(SceneMaterialAcoustic *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialAcousticDialog::createContent()
{
    txtDensity = new ValueLineEdit(this);
    txtSpeed = new ValueLineEdit(this);

    connect(txtDensity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtSpeed, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    layout->addWidget(createLabel(tr("<i>%1</i> (kg/m<sup>3</sup>)").arg(QString::fromUtf8("ρ")),
                                  tr("Mass density")), 10, 0);
    layout->addWidget(txtDensity, 10, 2);
    layout->addWidget(createLabel(tr("<i>c</i> (m/s)"),
                                  tr("Speed of sound")), 11, 0);
    layout->addWidget(txtSpeed, 11, 2);
}

void SceneMaterialAcousticDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(m_material);

    txtDensity->setValue(material->density);
    txtSpeed->setValue(material->speed);
}

bool SceneMaterialAcousticDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(m_material);

    if (txtDensity->evaluate())
        material->density  = txtDensity->value();
    else
        return false;

    if (txtSpeed->evaluate())
        material->speed  = txtSpeed->value();
    else
        return false;

    return true;
}
