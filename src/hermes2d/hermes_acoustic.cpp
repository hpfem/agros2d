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

// ****************************************************************************************************************

void ParserAcoustic::setParserVariables(SceneMaterial *material)
{
    SceneMaterialAcoustic *marker = dynamic_cast<SceneMaterialAcoustic *>(material);

    prho = marker->density.number;
    pv = marker->speed.number;
}

// ****************************************************************************************************************

LocalPointValueAcoustic::LocalPointValueAcoustic(const Point &point) : LocalPointValue(point)
{
    parser = new ParserAcoustic();
    initParser();

    parser->parser[0]->DefineVar("rho", &static_cast<ParserAcoustic *>(parser)->prho);
    parser->parser[0]->DefineVar("v", &static_cast<ParserAcoustic *>(parser)->pv);

    calculate();
}

// ****************************************************************************************************************

SurfaceIntegralValueAcoustic::SurfaceIntegralValueAcoustic() : SurfaceIntegralValue()
{
    parser = new ParserAcoustic();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("rho", &static_cast<ParserAcoustic *>(parser)->prho);
        ((mu::Parser *) *it)->DefineVar("v", &static_cast<ParserAcoustic *>(parser)->pv);
    }

    calculate();
}

// ****************************************************************************************************************

VolumeIntegralValueAcoustic::VolumeIntegralValueAcoustic() : VolumeIntegralValue()
{
    parser = new ParserAcoustic();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("rho", &static_cast<ParserAcoustic *>(parser)->prho);
        ((mu::Parser *) *it)->DefineVar("v", &static_cast<ParserAcoustic *>(parser)->pv);
    }

    sln.push_back(Util::scene()->sceneSolution()->sln(0));

    calculate();
}

// *************************************************************************************************************************************

ViewScalarFilterAcoustic::ViewScalarFilterAcoustic(Hermes::vector<MeshFunction *> sln,
                                                             std::string expression) :
    ViewScalarFilter(sln)
{
    parser = new ParserAcoustic();
    initParser(expression);

    parser->parser[0]->DefineVar("rho", &static_cast<ParserAcoustic *>(parser)->prho);
    parser->parser[0]->DefineVar("v", &static_cast<ParserAcoustic *>(parser)->pv);
}

// **************************************************************************************************************************

LocalPointValue *ModuleAcoustic::local_point_value(const Point &point)
{
    return new LocalPointValueAcoustic(point);
}

SurfaceIntegralValue *ModuleAcoustic::surface_integral_value()
{
    return new SurfaceIntegralValueAcoustic();
}

VolumeIntegralValue *ModuleAcoustic::volume_integral_value()
{
    return new VolumeIntegralValueAcoustic();
}

ViewScalarFilter *ModuleAcoustic::view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                           PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

    return new ViewScalarFilterAcoustic(Hermes::vector<MeshFunction *>(sln1, sln2),
                                        get_expression(physicFieldVariable, physicFieldVariableComp));
}

Hermes::vector<SolutionArray *> ModuleAcoustic::solve(ProgressItemSolve *progressItemSolve)
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
        SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->value_real.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->density.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->speed.evaluate()) return Hermes::vector<SolutionArray *>();
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

    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, wf);

    delete wf;

    return solutionArrayList;
}

void ModuleAcoustic::update_time_functions(double time)
{
    // update boundaries
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryAcoustic *boundary = dynamic_cast<SceneBoundaryAcoustic *>(Util::scene()->boundaries[i]);
        boundary->value_real.evaluate(time);
    }
}

// **************************************************************************************************************************
// rewrite

void ModuleAcoustic::readBoundaryFromDomElement(QDomElement *element)
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

void ModuleAcoustic::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
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

void ModuleAcoustic::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialAcoustic(element->attribute("name"),
                                                         Value(element->attribute("density", "1.25")),
                                                         Value(element->attribute("speed", "343"))));
}

void ModuleAcoustic::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialAcoustic *material = dynamic_cast<SceneMaterialAcoustic *>(marker);

    element->setAttribute("density", material->density.text);
    element->setAttribute("speed", material->speed.text);
}

SceneBoundary *ModuleAcoustic::newBoundary()
{
    return new SceneBoundaryAcoustic(tr("new boundary"),
                                     PhysicFieldBC_Acoustic_Pressure,
                                     Value("0"));
}

SceneBoundary *ModuleAcoustic::newBoundary(PyObject *self, PyObject *args)
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

SceneBoundary *ModuleAcoustic::modifyBoundary(PyObject *self, PyObject *args)
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

SceneMaterial *ModuleAcoustic::newMaterial()
{
    return new SceneMaterialAcoustic(tr("new material"),
                                     Value("1.25"),
                                     Value("343"));
}

SceneMaterial *ModuleAcoustic::newMaterial(PyObject *self, PyObject *args)
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

SceneMaterial *ModuleAcoustic::modifyMaterial(PyObject *self, PyObject *args)
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
