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

double get_lambda(double young_modulus, double poisson_ratio)
{
    return (young_modulus * poisson_ratio) / ((1.0 + poisson_ratio) * (1.0 - 2.0*poisson_ratio));
}
double get_mu(double young_modulus, double poisson_ratio)
{
    return young_modulus / (2.0*(1.0 + poisson_ratio));
}

/*
class WeakFormElasticity : public WeakFormAgros
{
public:
    WeakFormElasticity() : WeakFormAgros(2) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                if (boundary->type == "elasticity_free_fixed" || boundary->type == "elasticity_free_free")
                {
                    if (fabs(boundary->get_value("elasticity_force_x").number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->get_value("elasticity_force_x").number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));

                }
                if (boundary->type == "elasticity_fixed_free" || boundary->type == "elasticity_free_free")
                {
                    if (fabs(boundary->get_value("elasticity_force_y").number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(1,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        boundary->get_value("elasticity_force_y").number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }

        // materials (Default forms not implemented axisymmetric problems!)
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterial *material = Util::scene()->labels[i]->material;

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                double lambda = get_lambda(material->get_value("elasticity_young_modulus").number,
                                           material->get_value("elasticity_poisson_ratio").number);
                double mu = get_mu(material->get_value("elasticity_young_modulus").number,
                                   material->get_value("elasticity_poisson_ratio").number);

                add_matrix_form(new WeakFormsElasticity::VolumetricMatrixForms::DefaultLinearXX(0, 0,
                                                                                                QString::number(i).toStdString(),
                                                                                                lambda, mu,
                                                                                                convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsElasticity::VolumetricMatrixForms::DefaultLinearXY(0, 1,
                                                                                                QString::number(i).toStdString(),
                                                                                                lambda, mu,
                                                                                                convertProblemType(Util::scene()->problemInfo()->problemType)));

                add_matrix_form(new WeakFormsElasticity::VolumetricMatrixForms::DefaultLinearYY(1, 1,
                                                                                                QString::number(i).toStdString(),
                                                                                                lambda, mu,
                                                                                                convertProblemType(Util::scene()->problemInfo()->problemType)));

                // inner forces
                if (fabs(material->get_value("elasticity_force_x").number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->get_value("elasticity_force_x").number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));
                if (fabs(material->get_value("elasticity_force_y").number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(1,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   material->get_value("elasticity_force_y").number,
                                                                                                   convertProblemType(Util::scene()->problemInfo()->problemType)));

                // thermoelasticity
                if ((fabs(material->get_value("elasticity_alpha").number) > EPS_ZERO) &&
                        (fabs(material->get_value("elasticity_temperature").number - material->get_value("elasticity_temperature_reference").number) > EPS_ZERO))
                {
                    add_vector_form(new DefaultLinearThermoelasticityX(0,
                                                                       QString::number(i).toStdString(),
                                                                       lambda, mu,
                                                                       material->get_value("elasticity_alpha").number,
                                                                       material->get_value("elasticity_temperature").number,
                                                                       material->get_value("elasticity_temperature_reference").number,
                                                                       convertProblemType(Util::scene()->problemInfo()->problemType)));

                    add_vector_form(new DefaultLinearThermoelasticityY(1,
                                                                       QString::number(i).toStdString(),
                                                                       lambda, mu,
                                                                       material->get_value("elasticity_alpha").number,
                                                                       material->get_value("elasticity_temperature").number,
                                                                       material->get_value("elasticity_temperature_reference").number,
                                                                       convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }
    }

    class DefaultLinearThermoelasticityX : public WeakForm::VectorFormVol
    {
    public:
        DefaultLinearThermoelasticityX(int i, scalar lambda, scalar mu, scalar alpha, scalar temp, scalar temp_ref, GeomType gt = HERMES_PLANAR)
            : WeakForm::VectorFormVol(i), lambda(lambda), mu(mu), alpha(alpha), temp(temp), temp_ref(temp_ref), gt(gt) { }

        DefaultLinearThermoelasticityX(int i, std::string area, scalar lambda, scalar mu, scalar alpha, scalar temp, scalar temp_ref, GeomType gt = HERMES_PLANAR)
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
*/
// ****************************************************************************************************************

/*
Hermes::vector<SolutionArray *> ModuleElasticity::solve(ProgressItemSolve *progressItemSolve)
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
            if (boundary->type == "elasticity_fixed_free" || boundary->type == "elasticity_fixed_fixed")
                bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                       boundary->get_value("elasticity_displacement_x").number));
            if (boundary->type == "elasticity_free_fixed" || boundary->type == "elasticity_fixed_fixed")
                bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(),
                                                                       boundary->get_value("elasticity_displacement_y").number));
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
*/

// ****************************************************************************************************************
// rewrite

SceneBoundary *ModuleElasticity::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "FIXME");
}

SceneBoundary *ModuleElasticity::newBoundary(PyObject *self, PyObject *args)
{
    /*
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
    */
}

SceneBoundary *ModuleElasticity::modifyBoundary(PyObject *self, PyObject *args)
{
    /*
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
    */
}

SceneMaterial *ModuleElasticity::newMaterial()
{
    return new SceneMaterial(tr("new material").toStdString());
}

SceneMaterial *ModuleElasticity::newMaterial(PyObject *self, PyObject *args)
{
    /*
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
    */
}

SceneMaterial *ModuleElasticity::modifyMaterial(PyObject *self, PyObject *args)
{
    /*
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
    */
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

SceneBoundaryElasticityDialog::SceneBoundaryElasticityDialog(SceneBoundary *edgeEdgeElasticityMarker, QWidget *parent) : SceneBoundaryDialog(parent)
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

    cmbType = new QComboBox(this);
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtForceX = new ValueLineEdit(this);
    txtForceY = new ValueLineEdit(this);
    txtDisplacementX = new ValueLineEdit(this);
    txtDisplacementY = new ValueLineEdit(this);

    // X
    QGridLayout *layoutX = new QGridLayout();
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

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(grpX, 10, 0, 1, 3);
    layout->addWidget(grpY, 11, 0, 1, 3);
}

void SceneBoundaryElasticityDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));

    if (m_boundary->type == "elasticity_fixed_fixed")
    {
        txtDisplacementX->setValue(m_boundary->get_value("elasticity_displacement_x"));
        txtDisplacementY->setValue(m_boundary->get_value("elasticity_displacement_y"));
    }
    else if (m_boundary->type == "elasticity_fixed_free")
    {
        txtDisplacementX->setValue(m_boundary->get_value("elasticity_displacement_x"));
        txtForceY->setValue(m_boundary->get_value("elasticity_force_y"));
    }
    else if (m_boundary->type == "elasticity_free_fixed")
    {
        txtForceX->setValue(m_boundary->get_value("elasticity_force_x"));
        txtDisplacementY->setValue(m_boundary->get_value("elasticity_displacement_y"));
    }
    else if (m_boundary->type == "elasticity_free_free")
    {
        txtForceX->setValue(m_boundary->get_value("elasticity_force_x"));
        txtForceY->setValue(m_boundary->get_value("elasticity_force_y"));
    }
}

bool SceneBoundaryElasticityDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (m_boundary->type == "elasticity_fixed_fixed")
    {
        if (txtDisplacementX->evaluate())
            m_boundary->values["elasticity_displacement_x"] = txtDisplacementX->value();
        else
            return false;

        if (txtDisplacementY->evaluate())
            m_boundary->values["elasticity_displacement_y"] = txtDisplacementY->value();
        else
            return false;
    }
    else if (m_boundary->type == "elasticity_fixed_free")
    {
        if (txtDisplacementX->evaluate())
            m_boundary->values["elasticity_displacement_x"] = txtDisplacementX->value();
        else
            return false;

        if (txtForceY->evaluate())
            m_boundary->values["elasticity_force_y"] = txtForceY->value();
        else
            return false;
    }
    else if (m_boundary->type == "elasticity_free_fixed")
    {
        if (txtForceX->evaluate())
            m_boundary->values["elasticity_force_x"] = txtForceX->value();
        else
            return false;

        if (txtDisplacementY->evaluate())
            m_boundary->values["elasticity_displacement_y"] = txtDisplacementY->value();
        else
            return false;
    }
    else if (m_boundary->type == "elasticity_free_free")
    {
        if (txtForceX->evaluate())
            m_boundary->values["elasticity_force_x"] = txtForceX->value();
        else
            return false;

        if (txtForceY->evaluate())
            m_boundary->values["elasticity_force_y"] = txtForceY->value();
        else
            return false;
    }

    return true;
}

void SceneBoundaryElasticityDialog::doTypeChanged(int index)
{
    txtForceX->setEnabled(false);
    txtDisplacementX->setEnabled(false);
    txtForceY->setEnabled(false);
    txtDisplacementY->setEnabled(false);

    // read equation
    // readEquation(lblEquationImageX, (PhysicFieldBC) cmbTypeX->itemData(index).toInt());

    if (cmbType->itemData(index) == "elasticity_fixed_fixed")
    {
        txtDisplacementX->setEnabled(true);
        txtDisplacementY->setEnabled(true);
    }
    else if (cmbType->itemData(index) == "elasticity_fixed_free")
    {
        txtDisplacementX->setEnabled(true);
        txtForceY->setEnabled(true);
    }
    else if (cmbType->itemData(index) == "elasticity_free_fixed")
    {
        txtForceX->setEnabled(true);
        txtDisplacementY->setEnabled(true);
    }
    else if (cmbType->itemData(index) == "elasticity_free_free")
    {
        txtForceX->setEnabled(true);
        txtForceY->setEnabled(true);
    }
}

// *************************************************************************************************************************************

SceneMaterialElasticityDialog::SceneMaterialElasticityDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
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

    txtYoungModulus->setValue(m_material->get_value("elasticity_young_modulus"));
    txtPoissonNumber->setValue(m_material->get_value("elasticity_poisson_ratio"));

    txtForceX->setValue(m_material->get_value("elasticity_force_x"));
    txtForceY->setValue(m_material->get_value("elasticity_force_y"));

    txtAlpha->setValue(m_material->get_value("elasticity_alpha"));
    txtTemp->setValue(m_material->get_value("elasticity_temperature"));
    txtTempRef->setValue(m_material->get_value("elasticity_temperature_reference"));
}

bool SceneMaterialElasticityDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;

    if (txtYoungModulus->evaluate())
        m_material->values["elasticity_young_modulus"] = txtYoungModulus->value();
    else
        return false;

    if (txtPoissonNumber->evaluate())
        m_material->values["elasticity_poisson_ratio"] = txtPoissonNumber->value();
    else
        return false;

    if (txtForceX->evaluate())
        m_material->values["elasticity_force_x"] = txtForceX->value();
    else
        return false;

    if (txtForceY->evaluate())
        m_material->values["elasticity_force_y"] = txtForceY->value();
    else
        return false;

    if (txtAlpha->evaluate())
        m_material->values["elasticity_alpha"] = txtAlpha->value();
    else
        return false;

    if (txtTemp->evaluate())
        m_material->values["elasticity_temperature"] = txtTemp->value();
    else
        return false;

    if (txtTempRef->evaluate())
        m_material->values["elasticity_temperature_reference"] = txtTempRef->value();
    else
        return false;


    return true;
}
