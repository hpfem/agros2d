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

/*
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

        SceneMaterial *material = Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->elem_marker).c_str())]->material;
        return coeff / (material->get_value("acoustic_density").number * material->get_value("acoustic_speed").number) * result;
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
*/

// time dependent

// - 1/rho * \Delta p + 1/(rho * c^2) * \frac{\partial v}{\partial t} = 0.
// - v                +                 \frac{\partial p}{\partial t} = 0,

class WeakFormAcousticsTransient : public WeakFormAgros<double>
{
public:
    WeakFormAcousticsTransient() : WeakFormAgros(2) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == "acoustic_normal_acceleration")
                    if (fabs(boundary->get_value("acoustic_normal_acceleration").number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->get_value("acoustic_normal_acceleration").number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                if (boundary->type == "acoustic_impedance")
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

                if (boundary->type == "acoustic_matched_boundary")
                {
                    /*
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(1, 0,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 100,
                                                                                 convertProblemType(Util::scene()->problemInfo()->problemType)));

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
            /*
            SceneMaterial *material = Util::scene()->labels[i]->material;

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearDiffusion(0, 0,
                                                                                               QString::number(i).toStdString(),
                                                                                               1.0 / material->get_value("acoustic_density").number,
                                                                                               HERMES_NONSYM,
                                                                                               convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 1,
                                                                                          QString::number(i).toStdString(),
                                                                                          1.0 / (material->get_value("acoustic_density").number * sqr(material->get_value("acoustic_speed").number)) / Util::scene()->problemInfo()->timeStep.number,
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
                                                            1.0 / (material->get_value("acoustic_density").number * sqr(material->get_value("acoustic_speed").number)) / Util::scene()->problemInfo()->timeStep.number,
                                                            solution[0],
                                                            convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_vector_form(new CustomVectorFormTimeDep(1,
                                                            QString::number(i).toStdString(),
                                                            1.0 / Util::scene()->problemInfo()->timeStep.number,
                                                            solution[1],
                                                            convertProblemType(Util::scene()->problemInfo()->problemType)));
            }
            */
        }
    }

private:
    class VectorFormVolWaveMass : public Hermes::Hermes2D::VectorFormVol<double>
    {
    public:
        VectorFormVolWaveMass() : WeakForm::VectorFormVol(0) { }

        template<typename Real, typename Scalar>
        Scalar vector_form(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<Real> *v,
                           Hermes::Hermes2D::Geom<Real> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const {
            Scalar result = 0;

            for (int i = 0; i < n; i++)
                result += wt[i] * u_ext[1]->val[i] * v->val[i];

            return result;
        }

        virtual scalar value(int n, double *wt, Hermes::Hermes2D::Func<double> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                             Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<double> *ext) const {
            return vector_form<double, double>(n, wt, u_ext, v, e, ext);
        }

        virtual Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Ord> *u_ext[], Hermes::Hermes2D::Func<Ord> *v, Hermes::Hermes2D::Geom<Ord> *e,
                        Hermes::Hermes2D::ExtData<Ord> *ext) const {
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
    CustomAcousticPressureBC(std::string marker, SceneBoundary *boundary) : EssentialBoundaryCondition(marker)
    {
        m_boundary = boundary;
    }

    /// Function reporting the type of the essential boundary condition.
    inline EssentialBCValueType get_value_type() const { return EssentialBoundaryCondition::BC_FUNCTION; }
    virtual scalar value(double x, double y, double n_x, double n_y, double t_x, double t_y) const
    {
        return m_boundary->get_value("acoustic_pressure").number;
    }

private:
    SceneBoundary *m_boundary;
};

// ****************************************************************************************************************

/*
Hermes::vector<SolutionArray *> ModuleAcoustic::solve(ProgressItemSolve *progressItemSolve)
{
    if (!solve_init_variables())
        return Hermes::vector<SolutionArray *>();

    // boundary conditions
    EssentialBCs bc1;
    EssentialBCs bc2;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary)
        {
            if (boundary->type == "acoustic_pressure")
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), boundary->get_value("acoustic_pressure").number));
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
*/

void ModuleAcoustic::update_time_functions(double time)
{
    // update boundaries
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->boundaries[i];
        boundary->get_value("acoustic_pressure").evaluate(time);
    }
}

// **************************************************************************************************************************
// rewrite

SceneBoundary *ModuleAcoustic::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "acoustic_pressure");
}

SceneBoundary *ModuleAcoustic::newBoundary(PyObject *self, PyObject *args)
{
    /*
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
    */
}

SceneBoundary *ModuleAcoustic::modifyBoundary(PyObject *self, PyObject *args)
{
    /*
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
    */
}

SceneMaterial *ModuleAcoustic::newMaterial()
{
    return new SceneMaterial(tr("new material").toStdString());
}

SceneMaterial *ModuleAcoustic::newMaterial(PyObject *self, PyObject *args)
{
    /*
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
    */
}

SceneMaterial *ModuleAcoustic::modifyMaterial(PyObject *self, PyObject *args)
{
    /*
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
    */
}

// *************************************************************************************************************************************

SceneBoundaryAcousticDialog::SceneBoundaryAcousticDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
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
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
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

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "acoustic_pressure")
    {
        txtValue->setValue(m_boundary->get_value("acoustic_pressure_real"));
    }
    else if (m_boundary->type == "acoustic_normal_acceleration")
    {
        txtValue->setValue(m_boundary->get_value("acoustic_normal_acceleration"));
    }
    else if (m_boundary->type == "acoustic_impedance")
    {
        txtValue->setValue(m_boundary->get_value("acoustic_impedance"));
    }
}

bool SceneBoundaryAcousticDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (txtValue->evaluate())
        if (m_boundary->type == "acoustic_pressure")
        {
            m_boundary->values["acoustic_pressure_real"] = txtValue->value();
        }
        else if (m_boundary->type == "acoustic_normal_acceleration")
        {
            m_boundary->values["acoustic_normal_acceleration"] = txtValue->value();
        }
        else if (m_boundary->type == "acoustic_impedance")
        {
            m_boundary->values["acoustic_impedance"] = txtValue->value();
        }
        else
            return false;

    return true;
}

void SceneBoundaryAcousticDialog::doTypeChanged(int index)
{
    txtValue->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "acoustic_pressure")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>p</i><sub>0</sub> (Pa)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "acoustic_normal_acceleration")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>a</i><sub>0</sub> (m/s<sup>2</sup>)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "acoustic_impedance")
    {
        txtValue->setEnabled(true);
        lblValueUnit->setText(tr("<i>Z</i><sub>0</sub> (Pa·s/m)"));
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "acoustic_matched_boundary")
    {
        lblValueUnit->setText("-");
        lblValueUnit->setToolTip(cmbType->itemText(index));
    }

    setMinimumSize(sizeHint());
}

// *************************************************************************************************************************************

SceneMaterialAcousticDialog::SceneMaterialAcousticDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
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

    txtDensity->setValue(m_material->get_value("acoustic_density"));
    txtSpeed->setValue(m_material->get_value("acoustic_speed"));
}

bool SceneMaterialAcousticDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    if (txtDensity->evaluate())
        m_material->values["acoustic_density"] = txtDensity->value();
    else
        return false;

    if (txtSpeed->evaluate())
        m_material->values["acoustic_speed"] = txtSpeed->value();
    else
        return false;

    return true;
}
