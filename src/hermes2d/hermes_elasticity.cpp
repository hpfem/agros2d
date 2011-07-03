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

#include "hermes_elasticity.h"

#include "scene.h"
#include "gui.h"

class WeakFormElasticity : public WeakFormAgros
{
public:
    WeakFormElasticity() : WeakFormAgros(2) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryElasticity *boundary = dynamic_cast<SceneBoundaryElasticity *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->typeX == PhysicFieldBC_Elasticity_Free)
                {
                    if (fabs(boundary->forceX.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->forceX.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                }
                if (boundary->typeY == PhysicFieldBC_Elasticity_Free)
                {
                    if (fabs(boundary->forceY.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(1,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->forceY.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }

        // materials (Default forms not implemented axisymmetric problems!)
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialElasticity *material = dynamic_cast<SceneMaterialElasticity *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsElasticity::VolumetricMatrixForms::DefaultLinearXX(0, 0,
                                                                                                QString::number(i).toStdString(),
                                                                                                material->lambda(), material->mu(),
                                                                                                convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsElasticity::VolumetricMatrixForms::DefaultLinearXY(0, 1,
                                                                                                QString::number(i).toStdString(),
                                                                                                material->lambda(), material->mu(),
                                                                                                convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsElasticity::VolumetricMatrixForms::DefaultLinearYY(1, 1,
                                                                                                QString::number(i).toStdString(),
                                                                                                material->lambda(), material->mu(),
                                                                                                convertProblemType(Util::scene()->problemInfo()->problemType)));

                // inner forces
                if (fabs(material->forceX.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->forceX.number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));
                if (fabs(material->forceY.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(1,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->forceY.number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));

                // thermoelasticity
                if ((fabs(material->alpha.number) > EPS_ZERO) &&
                        (fabs(material->temp.number - material->temp_ref.number) > EPS_ZERO))
                    add_vector_form(new DefaultLinearThermoelasticityX(0, 0,
                                                                      QString::number(i).toStdString(),
                                                                      material->lambda(), material->mu(),
                                                                      material->alpha.number, material->temp.number, material->temp_ref.number,
                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));
                if ((fabs(material->alpha.number) > EPS_ZERO) &&
                        (fabs(material->temp.number - material->temp_ref.number) > EPS_ZERO))
                    add_vector_form(new DefaultLinearThermoelasticityY(1,
                                                                      QString::number(i).toStdString(),
                                                                      material->lambda(), material->mu(),
                                                                      material->alpha.number, material->temp.number, material->temp_ref.number,
                                                                      convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
        }
    }

    class DefaultLinearThermoelasticityX : public WeakForm::VectorFormVol
    {
    public:
        DefaultLinearThermoelasticityX(int i, scalar lambda, scalar mu, scalar alpha, scalar temp, scalar temp_ref, GeomType gt = HERMES_PLANAR)
            : WeakForm::VectorFormVol(i), lambda(lambda), mu(mu), alpha(alpha), temp(temp), temp_ref(temp_ref), gt(gt) { }

        DefaultLinearThermoelasticityX(int i, int j, std::string area, scalar lambda, scalar mu, scalar alpha, scalar temp, scalar temp_ref, GeomType gt = HERMES_PLANAR)
            : WeakForm::VectorFormVol(i, area), lambda(lambda), mu(mu), alpha(alpha), temp(temp), temp_ref(temp_ref), gt(gt) { }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {
            scalar result = 0.0;
            if (gt == HERMES_PLANAR)
                for (int i = 0; i < n; i++)
                    result += wt[i] * v->dx[i];
            else if (gt == HERMES_AXISYM_X)
                for (int i = 0; i < n; i++)
                    result += wt[i] * e->x[i] * v->dy[i];
            else
                for (int i = 0; i < n; i++)
                    result += wt[i] * (e->x[i] * v->dx[i] + v->val[i]);

            return (3*lambda + 2*mu) * alpha * (temp - temp_ref) * result;
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v,
                        Geom<Ord> *e, ExtData<Ord> *ext) const {
            Ord result = 0;
            if (gt == HERMES_PLANAR)
                for (int i = 0; i < n; i++)
                    result += wt[i] * v->dx[i];
            else if (gt == HERMES_AXISYM_X)
                for (int i = 0; i < n; i++)
                    result += wt[i] * e->x[i] * v->dy[i];
            else
                for (int i = 0; i < n; i++)
                    result += wt[i] * (e->x[i] * v->dx[i] + v->val[i]);

            return result;
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::VectorFormVol* clone() {
            return new DefaultLinearThermoelasticityX(*this);
        }

    private:
        scalar lambda, mu, alpha, temp, temp_ref, vel_ang;
        GeomType gt;
    };

    class DefaultLinearThermoelasticityY : public WeakForm::VectorFormVol
    {
    public:
        DefaultLinearThermoelasticityY(int i, scalar lambda, scalar mu, scalar alpha, scalar temp, scalar temp_ref, GeomType gt = HERMES_PLANAR)
            : WeakForm::VectorFormVol(i), lambda(lambda), mu(mu), alpha(alpha), temp(temp), temp_ref(temp_ref), gt(gt) { }

        DefaultLinearThermoelasticityY(int i, std::string area, scalar lambda, scalar mu, scalar alpha, scalar temp, scalar temp_ref, GeomType gt = HERMES_PLANAR)
            : WeakForm::VectorFormVol(i, area), lambda(lambda), mu(mu), alpha(alpha), temp(temp), temp_ref(temp_ref), gt(gt) { }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {
            scalar result = 0.0;
            if (gt == HERMES_PLANAR)
                for (int i = 0; i < n; i++)
                    result += wt[i] * v->dy[i];
            else if (gt == HERMES_AXISYM_X)
                for (int i = 0; i < n; i++)
                    result += wt[i] * (e->x[i] * v->dx[i] + v->val[i]);
            else
                for (int i = 0; i < n; i++)
                    result += wt[i] * e->x[i] * v->dy[i];

            return (3*lambda + 2*mu) * alpha * (temp - temp_ref) * result;
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v,
                        Geom<Ord> *e, ExtData<Ord> *ext) const {
            Ord result = 0;
            if (gt == HERMES_PLANAR)
                for (int i = 0; i < n; i++)
                    result += wt[i] * v->dy[i];
            else if (gt == HERMES_AXISYM_X)
                for (int i = 0; i < n; i++)
                    result += wt[i] * (e->x[i] * v->dx[i] + v->val[i]);
            else
                for (int i = 0; i < n; i++)
                    result += wt[i] * e->x[i] * v->dy[i];

            return result;
        }

        // This is to make the form usable in rk_time_step().
       virtual WeakForm::VectorFormVol* clone() {
            return new DefaultLinearThermoelasticityY(*this);
        }

    private:
        scalar lambda, mu, alpha, temp, temp_ref, vel_ang;
        GeomType gt;
    };
};

// ****************************************************************************************************************

void ParserElasticity::setParserVariables(SceneMaterial *material)
{
    SceneMaterialElasticity *marker = dynamic_cast<SceneMaterialElasticity *>(material);

    pe = marker->young_modulus.number;
    pnu = marker->poisson_ratio.number;
    pfx = marker->forceX.number;
    pfy = marker->forceY.number;
    palpha = marker->alpha.number;
    pt = marker->temp.number;
    ptref = marker->temp_ref.number;
}

// ****************************************************************************************************************

LocalPointValueElasticity::LocalPointValueElasticity(const Point &point) : LocalPointValue(point)
{
    parser = new ParserElasticity();
    initParser();

    parser->parser[0]->DefineVar("E", &static_cast<ParserElasticity *>(parser)->pe);
    parser->parser[0]->DefineVar("nu", &static_cast<ParserElasticity *>(parser)->pnu);
    parser->parser[0]->DefineVar("fx", &static_cast<ParserElasticity *>(parser)->pfx);
    parser->parser[0]->DefineVar("fy", &static_cast<ParserElasticity *>(parser)->pfy);
    parser->parser[0]->DefineVar("alpha", &static_cast<ParserElasticity *>(parser)->palpha);
    parser->parser[0]->DefineVar("T", &static_cast<ParserElasticity *>(parser)->pt);
    parser->parser[0]->DefineVar("Tr", &static_cast<ParserElasticity *>(parser)->ptref);

    calculate();
}

// ****************************************************************************************************************

SurfaceIntegralValueElasticity::SurfaceIntegralValueElasticity() : SurfaceIntegralValue()
{
    parser = new ParserElasticity();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("E", &static_cast<ParserElasticity *>(parser)->pe);
        ((mu::Parser *) *it)->DefineVar("nu", &static_cast<ParserElasticity *>(parser)->pnu);
        ((mu::Parser *) *it)->DefineVar("fx", &static_cast<ParserElasticity *>(parser)->pfx);
        ((mu::Parser *) *it)->DefineVar("fy", &static_cast<ParserElasticity *>(parser)->pfy);
        ((mu::Parser *) *it)->DefineVar("alpha", &static_cast<ParserElasticity *>(parser)->palpha);
        ((mu::Parser *) *it)->DefineVar("T", &static_cast<ParserElasticity *>(parser)->pt);
        ((mu::Parser *) *it)->DefineVar("Tr", &static_cast<ParserElasticity *>(parser)->ptref);
    }

    calculate();
}

// ****************************************************************************************************************

VolumeIntegralValueElasticity::VolumeIntegralValueElasticity() : VolumeIntegralValue()
{
    parser = new ParserElasticity();
    initParser();

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("E", &static_cast<ParserElasticity *>(parser)->pe);
        ((mu::Parser *) *it)->DefineVar("nu", &static_cast<ParserElasticity *>(parser)->pnu);
        ((mu::Parser *) *it)->DefineVar("fx", &static_cast<ParserElasticity *>(parser)->pfx);
        ((mu::Parser *) *it)->DefineVar("fy", &static_cast<ParserElasticity *>(parser)->pfy);
        ((mu::Parser *) *it)->DefineVar("alpha", &static_cast<ParserElasticity *>(parser)->palpha);
        ((mu::Parser *) *it)->DefineVar("T", &static_cast<ParserElasticity *>(parser)->pt);
        ((mu::Parser *) *it)->DefineVar("Tr", &static_cast<ParserElasticity *>(parser)->ptref);
    }

    sln.push_back(Util::scene()->sceneSolution()->sln(0));
    sln.push_back(Util::scene()->sceneSolution()->sln(1));

    calculate();
}

// *************************************************************************************************************************************

ViewScalarFilterElasticity::ViewScalarFilterElasticity(Hermes::vector<MeshFunction *> sln,
                                                             std::string expression) :
    ViewScalarFilter(sln)
{
    parser = new ParserElasticity();
    initParser(expression);

    parser->parser[0]->DefineVar("E", &static_cast<ParserElasticity *>(parser)->pe);
    parser->parser[0]->DefineVar("nu", &static_cast<ParserElasticity *>(parser)->pnu);
    parser->parser[0]->DefineVar("fx", &static_cast<ParserElasticity *>(parser)->pfx);
    parser->parser[0]->DefineVar("fy", &static_cast<ParserElasticity *>(parser)->pfy);
    parser->parser[0]->DefineVar("alpha", &static_cast<ParserElasticity *>(parser)->palpha);
    parser->parser[0]->DefineVar("T", &static_cast<ParserElasticity *>(parser)->pt);
    parser->parser[0]->DefineVar("Tr", &static_cast<ParserElasticity *>(parser)->ptref);
}

// **************************************************************************************************************************

LocalPointValue *ModuleElasticity::local_point_value(const Point &point)
{
    return new LocalPointValueElasticity(point);
}

SurfaceIntegralValue *ModuleElasticity::surface_integral_value()
{
    return new SurfaceIntegralValueElasticity();
}

VolumeIntegralValue *ModuleElasticity::volume_integral_value()
{
    return new VolumeIntegralValueElasticity();
}

ViewScalarFilter *ModuleElasticity::view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                           PhysicFieldVariableComp physicFieldVariableComp)
{
    Solution *sln1 = Util::scene()->sceneSolution()->sln(0);
    Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

    return new ViewScalarFilterElasticity(Hermes::vector<MeshFunction *>(sln1, sln2),
                                          get_expression(physicFieldVariable, physicFieldVariableComp));
}

Hermes::vector<SolutionArray *> ModuleElasticity::solve(ProgressItemSolve *progressItemSolve)
{
    // boundaries
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundaryElasticity *boundary = dynamic_cast<SceneBoundaryElasticity *>(Util::scene()->boundaries[i]);

        // evaluate script
        if (!boundary->displacementX.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!boundary->displacementY.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!boundary->forceX.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!boundary->forceY.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterialElasticity *material = dynamic_cast<SceneMaterialElasticity *>(Util::scene()->materials[i]);

        // evaluate script
        if (!material->young_modulus.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->poisson_ratio.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->forceX.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->forceY.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->alpha.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->temp.evaluate()) return Hermes::vector<SolutionArray *>();
        if (!material->temp_ref.evaluate()) return Hermes::vector<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bc1;
    EssentialBCs bc2;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundaryElasticity *boundary = dynamic_cast<SceneBoundaryElasticity *>(Util::scene()->edges[i]->boundary);

        if (boundary)
        {
            if (boundary->typeX == PhysicFieldBC_Elasticity_Fixed)
                bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->displacementX.number));
            if (boundary->typeY == PhysicFieldBC_Elasticity_Fixed)
                bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->displacementY.number));
        }
    }
    Hermes::vector<EssentialBCs> bcs(bc1, bc2);

    WeakFormElasticity wf;

    // QTime time;
    // time.start();
    Hermes::vector<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);
    // qDebug() << "solveSolutioArray: " << time.elapsed();

    return solutionArrayList;
}

// ****************************************************************************************************************
// rewrite

void ModuleElasticity::readBoundaryFromDomElement(QDomElement *element)
{
    PhysicFieldBC typeX = physicFieldBCFromStringKey(element->attribute("typex"));
    switch (typeX)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Elasticity_Fixed:
    case PhysicFieldBC_Elasticity_Free:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("typex")).toStdString() << endl;
        break;
    }

    PhysicFieldBC typeY = physicFieldBCFromStringKey(element->attribute("typey"));
    switch (typeY)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Elasticity_Fixed:
    case PhysicFieldBC_Elasticity_Free:
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("typey")).toStdString() << endl;
        break;
    }

    if ((typeX != PhysicFieldBC_Undefined) && (typeY != PhysicFieldBC_Undefined))
        Util::scene()->addBoundary(new SceneBoundaryElasticity(element->attribute("name"),
                                                               typeX, typeY,
                                                               Value(element->attribute("forcex", "0")),
                                                               Value(element->attribute("forcey", "0")),
                                                               Value(element->attribute("displacementx", "0")),
                                                               Value(element->attribute("displacementx", "0"))));
}

void ModuleElasticity::writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker)
{
    SceneBoundaryElasticity *edgeElasticityMarker = dynamic_cast<SceneBoundaryElasticity *>(marker);

    element->setAttribute("typex", physicFieldBCToStringKey(edgeElasticityMarker->typeX));
    element->setAttribute("typey", physicFieldBCToStringKey(edgeElasticityMarker->typeY));
    element->setAttribute("forcex", edgeElasticityMarker->forceX.text);
    element->setAttribute("forcey", edgeElasticityMarker->forceY.text);
}

void ModuleElasticity::readMaterialFromDomElement(QDomElement *element)
{
    Util::scene()->addMaterial(new SceneMaterialElasticity(element->attribute("name"),
                                                           Value(element->attribute("young_modulus")),
                                                           Value(element->attribute("poisson_ratio")),
                                                           Value(element->attribute("forcex", "0")),
                                                           Value(element->attribute("forcey", "0")),
                                                           Value(element->attribute("alpha", "0")),
                                                           Value(element->attribute("temp", "0")),
                                                           Value(element->attribute("temp_ref", "0"))));
}

void ModuleElasticity::writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker)
{
    SceneMaterialElasticity *labelElasticityMarker = dynamic_cast<SceneMaterialElasticity *>(marker);

    element->setAttribute("young_modulus", labelElasticityMarker->young_modulus.text);
    element->setAttribute("poisson_ratio", labelElasticityMarker->poisson_ratio.text);
    element->setAttribute("forcex", labelElasticityMarker->forceX.text);
    element->setAttribute("forcey", labelElasticityMarker->forceY.text);
    element->setAttribute("alpha", labelElasticityMarker->alpha.text);
    element->setAttribute("temp", labelElasticityMarker->temp.text);
    element->setAttribute("temp_ref", labelElasticityMarker->temp_ref.text);
}

SceneBoundary *ModuleElasticity::newBoundary()
{
    return new SceneBoundaryElasticity(tr("new boundary condition"),
                                       PhysicFieldBC_Elasticity_Free,
                                       PhysicFieldBC_Elasticity_Free,
                                       Value("0"),
                                       Value("0"),
                                       Value("0"),
                                       Value("0"));
}

SceneBoundary *ModuleElasticity::newBoundary(PyObject *self, PyObject *args)
{

    double forcex, forcey, displacementx, displacementy;
    char *name, *typex, *typey;
    if (PyArg_ParseTuple(args, "sssdddd", &name, &typex, &typey, &forcex, &forcey, &displacementx, &displacementy))
    {
        // check name
        if (Util::scene()->getBoundary(name)) return NULL;

        return new SceneBoundaryElasticity(name,
                                           physicFieldBCFromStringKey(typex),
                                           physicFieldBCFromStringKey(typey),
                                           Value(QString::number(forcex)),
                                           Value(QString::number(forcey)),
                                           Value(QString::number(displacementx)),
                                           Value(QString::number(displacementy)));
    }

    return Util::scene()->boundaries[0];
}

SceneBoundary *ModuleElasticity::modifyBoundary(PyObject *self, PyObject *args)
{
    double forcex, forcey, displacementx, displacementy;
    char *name, *typex, *typey;
    if (PyArg_ParseTuple(args, "sssdddd", &name, &typex, &typey, &forcex, &forcey, &displacementx, &displacementy))
    {
        if (SceneBoundaryElasticity *marker = dynamic_cast<SceneBoundaryElasticity *>(Util::scene()->getBoundary(name)))
        {
            if (physicFieldBCFromStringKey(typex) && physicFieldBCFromStringKey(typey))
            {
                marker->typeX = physicFieldBCFromStringKey(typex);
                marker->typeY = physicFieldBCFromStringKey(typey);
                marker->forceX = Value(QString::number(forcex));
                marker->forceY = Value(QString::number(forcey));
                marker->displacementX = Value(QString::number(displacementx));
                marker->displacementY = Value(QString::number(displacementy));
                return marker;
            }
            else
            {
                PyErr_SetString(PyExc_RuntimeError, QObject::tr("Boundary type '%1' or '%2' is not supported.").arg(typex, typey).toStdString().c_str());
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

SceneMaterial *ModuleElasticity::newMaterial()
{
    return new SceneMaterialElasticity(tr("new material"),
                                       Value("2e11"),
                                       Value("0.33"),
                                       Value("0"),
                                       Value("0"),
                                       Value("1.2e-5"),
                                       Value("0"),
                                       Value("0"));
}

SceneMaterial *ModuleElasticity::newMaterial(PyObject *self, PyObject *args)
{
    double young_modulus, poisson_ratio, forcex, forcey, alpha, temp, temp_ref;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddd", &name, &young_modulus, &poisson_ratio, &forcex, &forcey, &alpha, &temp, &temp_ref))
    {
        // check name
        if (Util::scene()->getMaterial(name)) return NULL;

        return new SceneMaterialElasticity(name,
                                           Value(QString::number(young_modulus)),
                                           Value(QString::number(poisson_ratio)),
                                           Value(QString::number(forcex)),
                                           Value(QString::number(forcey)),
                                           Value(QString::number(alpha)),
                                           Value(QString::number(temp)),
                                           Value(QString::number(temp_ref)));
    }

    return NULL;
}

SceneMaterial *ModuleElasticity::modifyMaterial(PyObject *self, PyObject *args)
{
    double young_modulus, poisson_ratio, forcex, forcey, alpha, temp, temp_ref;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddd", &name, &young_modulus, &poisson_ratio, &forcex, &forcey, &alpha, &temp, &temp_ref))
    {
        if (SceneMaterialElasticity *marker = dynamic_cast<SceneMaterialElasticity *>(Util::scene()->getMaterial(name)))
        {
            marker->young_modulus = Value(QString::number(young_modulus));
            marker->poisson_ratio = Value(QString::number(poisson_ratio));
            marker->forceX = Value(QString::number(forcex));
            marker->forceY = Value(QString::number(forcey));
            marker->alpha = Value(QString::number(alpha));
            marker->temp = Value(QString::number(temp));
            marker->temp_ref = Value(QString::number(temp_ref));
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

template <class T>
void deformShapeTemplate(T linVert, int count)
{
    double min =  numeric_limits<double>::max();
    double max = -numeric_limits<double>::max();
    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = Util::scene()->sceneSolution()->sln(0)->get_pt_value(x, y);
        double dy = Util::scene()->sceneSolution()->sln(1)->get_pt_value(x, y);

        double dm = sqrt(sqr(dx) + sqr(dy));

        if (dm < min) min = dm;
        if (dm > max) max = dm;
    }

    RectPoint rect = Util::scene()->boundingBox();
    double k = qMax(rect.width(), rect.height()) / qMax(min, max) / 15.0;

    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = Util::scene()->sceneSolution()->sln(0)->get_pt_value(x, y);
        double dy = Util::scene()->sceneSolution()->sln(1)->get_pt_value(x, y);

        linVert[i][0] += k*dx;
        linVert[i][1] += k*dy;
    }
}

void ModuleElasticity::deform_shape(double3* linVert, int count)
{
   deformShapeTemplate<double3 *>(linVert, count);
}

void ModuleElasticity::deform_shape(double4* linVert, int count)
{
   deformShapeTemplate<double4 *>(linVert, count);
}

// *************************************************************************************************************************************

SceneBoundaryElasticity::SceneBoundaryElasticity(const QString &name, PhysicFieldBC typeX,
                                                 PhysicFieldBC typeY, Value forceX, Value forceY,
                                                 Value displacementX, Value displacementY)
    : SceneBoundary(name, typeX)
{
    this->typeX = typeX;
    this->typeY = typeY;
    this->forceX = forceX;
    this->forceY = forceY;
    this->displacementX = displacementX;
    this->displacementY = displacementY;
}

QString SceneBoundaryElasticity::script()
{
    return QString("addboundary(\"%1\", \"%2\", \"%3\", %4, %5, %6, %7)").
            arg(name).
            arg(physicFieldBCToStringKey(typeX)).
            arg(physicFieldBCToStringKey(typeY)).
            arg(forceX.text).
            arg(forceY.text).
            arg(displacementX.text).
            arg(displacementY.text);
}

QMap<QString, QString> SceneBoundaryElasticity::data()
{
    QMap<QString, QString> out;
    switch (typeX)
    {
    case PhysicFieldBC_Elasticity_Free:
        out["Force X: (N)"] = forceX.number;
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        out["Displacement X: (N)"] = displacementX.number;
        break;
    }

    switch (typeY)
    {
    case PhysicFieldBC_Elasticity_Free:
        out["Force Y: (N)"] = forceY.number;
        break;
    case PhysicFieldBC_Elasticity_Fixed:
        out["Displacement Y: (N)"] = displacementY.number;
        break;
    }

    return QMap<QString, QString>(out);
}

int SceneBoundaryElasticity::showDialog(QWidget *parent)
{
    SceneBoundaryElasticityDialog *dialog = new SceneBoundaryElasticityDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneMaterialElasticity::SceneMaterialElasticity(const QString &name, Value young_modulus, Value poisson_ratio, Value forceX, Value forceY,
                                                 Value alpha, Value temp, Value temp_ref)
    : SceneMaterial(name)
{
    this->young_modulus = young_modulus;
    this->poisson_ratio = poisson_ratio;
    this->forceX = forceX;
    this->forceY = forceY;
    this->alpha = alpha;
    this->temp = temp;
    this->temp_ref = temp_ref;
}

QString SceneMaterialElasticity::script()
{
    return QString("addmaterial(\"%1\", %2, %3, %4, %5, %6, %7, %8)").
            arg(name).
            arg(young_modulus.text).
            arg(poisson_ratio.text).
            arg(forceX.text).
            arg(forceY.text).
            arg(alpha.text).
            arg(temp.text).
            arg(temp_ref.text);
}

QMap<QString, QString> SceneMaterialElasticity::data()
{
    QMap<QString, QString> out;
    out["Young modulus (Pa)"] = young_modulus.number;
    out["Poisson ratio (-)"] = poisson_ratio.number;
    out["Volumetric force X (N/m3)"] = forceX.number;
    out["Volumetric force Y (N/m3)"] = forceX.number;
    out["Thermal exp. coef. (1/K)"] = alpha.number;
    out["Temperature (K)"] = temp.number;
    out["Ref. temperature (K)"] = temp_ref.number;
    return QMap<QString, QString>(out);
}

int SceneMaterialElasticity::showDialog(QWidget *parent)
{
    SceneMaterialElasticityDialog *dialog = new SceneMaterialElasticityDialog(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneBoundaryElasticityDialog::SceneBoundaryElasticityDialog(SceneBoundaryElasticity *edgeEdgeElasticityMarker, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = edgeEdgeElasticityMarker;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryElasticityDialog::createContent()
{
    lblEquation->setVisible(false);
    lblEquationImage->setVisible(false);

    lblEquationX = new QLabel(tr("Equation:"));
    lblEquationImageX = new QLabel(this);
    lblEquationY = new QLabel(tr("Equation:"));
    lblEquationImageY = new QLabel(this);

    cmbTypeX = new QComboBox(this);
    cmbTypeX->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Free), PhysicFieldBC_Elasticity_Free);
    cmbTypeX->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Fixed), PhysicFieldBC_Elasticity_Fixed);
    connect(cmbTypeX, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeXChanged(int)));

    cmbTypeY = new QComboBox(this);
    cmbTypeY->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Free), PhysicFieldBC_Elasticity_Free);
    cmbTypeY->addItem(physicFieldBCString(PhysicFieldBC_Elasticity_Fixed), PhysicFieldBC_Elasticity_Fixed);
    connect(cmbTypeY, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeYChanged(int)));

    txtForceX = new ValueLineEdit(this);
    txtForceY = new ValueLineEdit(this);
    txtDisplacementX = new ValueLineEdit(this);
    txtDisplacementY = new ValueLineEdit(this);

    doTypeXChanged(cmbTypeX->currentIndex());
    doTypeYChanged(cmbTypeY->currentIndex());

    // X
    QGridLayout *layoutX = new QGridLayout();
    layoutX->addWidget(new QLabel(tr("BC Type:")), 0, 0);
    layoutX->addWidget(cmbTypeX, 0, 1);
    layoutX->addWidget(lblEquationX, 1, 0);
    layoutX->addWidget(lblEquationImageX, 1, 1);
    layoutX->addWidget(createLabel(tr("<i>f</i><sub>%1</sub> (N/m<sup>2</sup>)").arg(Util::scene()->problemInfo()->labelX()),
                                   tr("Force")), 2, 0);
    layoutX->addWidget(txtForceX, 2, 1);
    layoutX->addWidget(createLabel(tr("<i>u</i><sub>%1</sub> (m)").arg(Util::scene()->problemInfo()->labelX()),
                                   tr("Displacement")), 3, 0);
    layoutX->addWidget(txtDisplacementX, 3, 1);

    QGroupBox *grpX = new QGroupBox(tr("Direction %1").arg(Util::scene()->problemInfo()->labelX()), this);
    grpX->setLayout(layoutX);

    // X
    QGridLayout *layoutY = new QGridLayout();
    layoutY->addWidget(new QLabel(tr("BC Type:")), 0, 0);
    layoutY->addWidget(cmbTypeY, 0, 1);
    layoutY->addWidget(lblEquationY, 1, 0);
    layoutY->addWidget(lblEquationImageY, 1, 1);
    layoutY->addWidget(createLabel(tr("<i>f</i><sub>%1</sub> (N/m<sup>2</sup>)").arg(Util::scene()->problemInfo()->labelY()),
                                   tr("Force")), 2, 0);
    layoutY->addWidget(txtForceY, 2, 1);
    layoutY->addWidget(createLabel(tr("<i>u</i><sub>%1</sub> (m)").arg(Util::scene()->problemInfo()->labelY()),
                                   tr("Displacement")), 3, 0);
    layoutY->addWidget(txtDisplacementY, 3, 1);

    QGroupBox *grpY = new QGroupBox(tr("Direction %1").arg(Util::scene()->problemInfo()->labelY()), this);
    grpY->setLayout(layoutY);

    layout->addWidget(grpX, 10, 0, 1, 3);
    layout->addWidget(grpY, 11, 0, 1, 3);
}

void SceneBoundaryElasticityDialog::load()
{
    SceneBoundaryDialog::load();

    SceneBoundaryElasticity *edgeElasticityMarker = dynamic_cast<SceneBoundaryElasticity *>(m_boundary);

    cmbTypeX->setCurrentIndex(cmbTypeX->findData(edgeElasticityMarker->typeX));
    cmbTypeY->setCurrentIndex(cmbTypeY->findData(edgeElasticityMarker->typeY));

    txtForceX->setValue(edgeElasticityMarker->forceX);
    txtForceY->setValue(edgeElasticityMarker->forceY);

    txtDisplacementX->setValue(edgeElasticityMarker->displacementX);
    txtDisplacementY->setValue(edgeElasticityMarker->displacementY);
}

bool SceneBoundaryElasticityDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    SceneBoundaryElasticity *edgeElasticityMarker = dynamic_cast<SceneBoundaryElasticity *>(m_boundary);

    edgeElasticityMarker->typeX = (PhysicFieldBC) cmbTypeX->itemData(cmbTypeX->currentIndex()).toInt();
    edgeElasticityMarker->typeY = (PhysicFieldBC) cmbTypeY->itemData(cmbTypeY->currentIndex()).toInt();

    if (txtForceX->evaluate())
        edgeElasticityMarker->forceX = txtForceX->value();
    else
        return false;

    if (txtForceY->evaluate())
        edgeElasticityMarker->forceY = txtForceY->value();
    else
        return false;

    if (txtDisplacementX->evaluate())
        edgeElasticityMarker->displacementX = txtDisplacementX->value();
    else
        return false;

    if (txtDisplacementY->evaluate())
        edgeElasticityMarker->displacementY = txtDisplacementY->value();
    else
        return false;

    return true;
}

void SceneBoundaryElasticityDialog::doTypeXChanged(int index)
{
    txtForceX->setEnabled(false);
    txtDisplacementX->setEnabled(false);

    // read equation
    readEquation(lblEquationImageX, (PhysicFieldBC) cmbTypeX->itemData(index).toInt());

    switch ((PhysicFieldBC) cmbTypeX->itemData(index).toInt())
    {
    case PhysicFieldBC_Elasticity_Fixed:
    {
        txtDisplacementX->setEnabled(true);
    }
        break;
    case PhysicFieldBC_Elasticity_Free:
    {
        txtForceX->setEnabled(true);
    }
        break;
    }
}

void SceneBoundaryElasticityDialog::doTypeYChanged(int index)
{
    txtForceY->setEnabled(false);
    txtDisplacementY->setEnabled(false);

    // read equation
    readEquation(lblEquationImageY, (PhysicFieldBC) cmbTypeY->itemData(index).toInt());

    switch ((PhysicFieldBC) cmbTypeY->itemData(index).toInt())
    {
    case PhysicFieldBC_Elasticity_Fixed:
    {
        txtDisplacementY->setEnabled(true);
    }
        break;
    case PhysicFieldBC_Elasticity_Free:
    {
        txtForceY->setEnabled(true);
    }
        break;
    }
}

// *************************************************************************************************************************************

SceneMaterialElasticityDialog::SceneMaterialElasticityDialog(SceneMaterialElasticity *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialElasticityDialog::createContent()
{
    txtYoungModulus = new ValueLineEdit(this);
    txtPoissonNumber = new ValueLineEdit(this);
    txtForceX = new ValueLineEdit(this);
    txtForceY = new ValueLineEdit(this);
    txtAlpha = new ValueLineEdit(this);
    txtTemp = new ValueLineEdit(this);
    txtTempRef = new ValueLineEdit(this);

    // forces
    QGridLayout *layoutForces = new QGridLayout();
    layoutForces->addWidget(createLabel(tr("<i>f</i><sub>%1</sub> (N/m<sup>3</sup>)").arg(Util::scene()->problemInfo()->labelX().toLower()),
                                        tr("Force")), 0, 0);
    layoutForces->addWidget(txtForceX, 0, 2);
    layoutForces->addWidget(createLabel(tr("<i>f</i><sub>%1</sub> (N/m<sup>3</sup>)").arg(Util::scene()->problemInfo()->labelY().toLower()),
                                        tr("Force")), 1, 0);
    layoutForces->addWidget(txtForceY, 1, 2);

    QGroupBox *grpForces = new QGroupBox(tr("Volumetric forces"), this);
    grpForces->setLayout(layoutForces);

    // thermal expansion
    QGridLayout *layoutThermalExpansion = new QGridLayout();
    layoutThermalExpansion->addWidget(createLabel(tr("<i>%1</i><sub>T</sub> (1/K)").arg(QString::fromUtf8("α")),
                                                  tr("Thermal exp. coef.")), 0, 0);
    layoutThermalExpansion->addWidget(txtAlpha, 0, 2);
    layoutThermalExpansion->addWidget(createLabel(tr("<i>T</i> (K)"),
                                                  tr("Temperature")), 1, 0);
    layoutThermalExpansion->addWidget(txtTemp, 1, 2);
    layoutThermalExpansion->addWidget(createLabel(tr("<i>T</i><sub>ref</sub> (K)"),
                                                  tr("Ref. temperature")), 2, 0);
    layoutThermalExpansion->addWidget(txtTempRef, 2, 2);

    QGroupBox *grpThermalExpansion = new QGroupBox(tr("Thermal expansion"), this);
    grpThermalExpansion->setLayout(layoutThermalExpansion);

    // elasticity coeffs \lambda and \mu
    layout->addWidget(new QLabel(tr("Lame's coefficients:")), 2, 0, 1, 2);
    QLabel *lblEquationImageCoeffs = new QLabel(this);
    layout->addWidget(lblEquationImageCoeffs, 2, 2);
    readPixmap(lblEquationImageCoeffs,
               QString(":/images/equations/%1/%1_coeffs.png")
               .arg(QString::fromStdString(Util::scene()->problemInfo()->module()->id)));

    layout->addWidget(createLabel(tr("<i>E</i> (Pa)"),
                                  tr("Young modulus")), 10, 0);
    layout->addWidget(txtYoungModulus, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (-)").arg(QString::fromUtf8("υ")),
                                  tr("Poisson number")), 11, 0);
    layout->addWidget(txtPoissonNumber, 11, 2);
    layout->addWidget(grpForces, 12, 0, 1, 3);
    layout->addWidget(grpThermalExpansion, 13, 0, 1, 3);
}

void SceneMaterialElasticityDialog::load()
{
    SceneMaterialDialog::load();

    SceneMaterialElasticity *labelElasticityMarker = dynamic_cast<SceneMaterialElasticity *>(m_material);

    txtYoungModulus->setValue(labelElasticityMarker->young_modulus);
    txtPoissonNumber->setValue(labelElasticityMarker->poisson_ratio);

    txtForceX->setValue(labelElasticityMarker->forceX);
    txtForceY->setValue(labelElasticityMarker->forceY);

    txtAlpha->setValue(labelElasticityMarker->alpha);
    txtTemp->setValue(labelElasticityMarker->temp);
    txtTempRef->setValue(labelElasticityMarker->temp_ref);
}

bool SceneMaterialElasticityDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;

    SceneMaterialElasticity *labelElasticityMarker = dynamic_cast<SceneMaterialElasticity *>(m_material);

    if (txtYoungModulus->evaluate())
        labelElasticityMarker->young_modulus = txtYoungModulus->value();
    else
        return false;

    if (txtPoissonNumber->evaluate())
        labelElasticityMarker->poisson_ratio = txtPoissonNumber->value();
    else
        return false;

    if (txtForceX->evaluate())
        labelElasticityMarker->forceX = txtForceX->value();
    else
        return false;

    if (txtForceY->evaluate())
        labelElasticityMarker->forceY = txtForceY->value();
    else
        return false;

    if (txtAlpha->evaluate())
        labelElasticityMarker->alpha = txtAlpha->value();
    else
        return false;

    if (txtTemp->evaluate())
        labelElasticityMarker->temp = txtTemp->value();
    else
        return false;

    if (txtTempRef->evaluate())
        labelElasticityMarker->temp_ref = txtTempRef->value();
    else
        return false;


    return true;
}
