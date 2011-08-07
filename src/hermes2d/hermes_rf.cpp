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

#include "hermes_rf.h"

#include "scene.h"
#include "gui.h"

/*
class WeakFormRFHarmonic : public WeakFormAgros
{
public:
    WeakFormRFHarmonic() : WeakFormAgros(2) { }

    void registerForms()
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneBoundaryRF *boundary = dynamic_cast<SceneBoundaryRF *>(Util::scene()->edges[i]->boundary);

            if (boundary && Util::scene()->edges[i]->boundary != Util::scene()->boundaries[0])
            {
                // surface current
                if (boundary->type == PhysicFieldBC_RF_SurfaceCurrent)
                {
                    if (fabs(boundary->value_imag.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        - 2 * M_PI * Util::scene()->problemInfo()->frequency * boundary->value_imag.number,
                                                                                                        HERMES_PLANAR));
                    if (fabs(boundary->value_real.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(1,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        2 * M_PI * Util::scene()->problemInfo()->frequency * boundary->value_real.number,
                                                                                                        HERMES_PLANAR));
                }

                // port
                if (boundary->type == PhysicFieldBC_RF_Port)
                {
                    if (fabs(boundary->power.number) > EPS_ZERO)
                    {
                        int mode = 0;
                        switch (boundary->mode)
                        {
                        case Mode_0:
                            mode = 1;
                            break;
                        case Mode_01:
                            mode = 2;
                            break;
                        case Mode_02:
                            mode = 3;
                            break;
                        }

                        add_vector_form_surf(new CustomVectorFormSurfPort(0,
                                                                          QString::number(i + 1).toStdString(),
                                                                          boundary->power.number * cos(boundary->phase.number / 180.0 * M_PI),
                                                                          mode));
                        add_vector_form_surf(new CustomVectorFormSurfPort(1,
                                                                          QString::number(i + 1).toStdString(),
                                                                          boundary->power.number * sin(boundary->phase.number / 180.0 * M_PI),
                                                                          mode));
                    }
                }

                // matched boundary
                if ((boundary->type == PhysicFieldBC_RF_MatchedBoundary)
                        || (boundary->type == PhysicFieldBC_RF_Port))
                {
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(0, 1,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 1.0,
                                                                                 2 * M_PI * Util::scene()->problemInfo()->frequency));
                    add_matrix_form_surf(new CustomMatrixFormSurfMatchedBoundary(1, 0,
                                                                                 QString::number(i + 1).toStdString(),
                                                                                 - 1.0,
                                                                                 2 * M_PI * Util::scene()->problemInfo()->frequency));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneMaterialRF *material = dynamic_cast<SceneMaterialRF *>(Util::scene()->labels[i]->material);

            if (material && Util::scene()->labels[i]->material != Util::scene()->materials[0])
            {
                // real part
                add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(0, 0,
                                                                                                         QString::number(i).toStdString(),
                                                                                                         - 1.0 / (material->permeability.number * MU0),
                                                                                                         HERMES_NONSYM,
                                                                                                         convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                         (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 3)));

                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 0,
                                                                                          QString::number(i).toStdString(),
                                                                                          sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * (material->permittivity.number * EPS0),
                                                                                          HERMES_NONSYM,
                                                                                          HERMES_PLANAR));

                // imag part
                add_matrix_form(new WeakFormsMaxwell::VolumetricMatrixForms::DefaultLinearMagnetostatics(1, 1,
                                                                                                         QString::number(i).toStdString(),
                                                                                                         - 1.0 / (material->permeability.number * MU0),
                                                                                                         HERMES_NONSYM,
                                                                                                         convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                                                         (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 3)));


                add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 1,
                                                                                          QString::number(i).toStdString(),
                                                                                          sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * (material->permittivity.number * EPS0),
                                                                                          HERMES_NONSYM,
                                                                                          HERMES_PLANAR));

                // lossy environment
                if (fabs(material->conductivity.number) > EPS_ZERO)
                {
                    add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 1,
                                                                                              QString::number(i).toStdString(),
                                                                                              2 * M_PI * Util::scene()->problemInfo()->frequency * material->conductivity.number,
                                                                                              HERMES_NONSYM,
                                                                                              HERMES_PLANAR));

                    add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 0,
                                                                                              QString::number(i).toStdString(),
                                                                                              - 2 * M_PI * Util::scene()->problemInfo()->frequency * material->conductivity.number,
                                                                                              HERMES_NONSYM,
                                                                                              HERMES_PLANAR));
                }

                // external current density
                if (fabs(material->current_density_imag.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   2 * M_PI * Util::scene()->problemInfo()->frequency * material->current_density_imag.number,
                                                                                                   HERMES_PLANAR));

                if (fabs(material->current_density_real.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(1,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   - 2 * M_PI * Util::scene()->problemInfo()->frequency * material->current_density_real.number,
                                                                                                   HERMES_PLANAR));
            }
        }
    }

    class CustomVectorFormSurfPort : public WeakForm::VectorFormSurf
    {
    public:
        CustomVectorFormSurfPort(int i, std::string area, scalar power, int mode)
            : WeakForm::VectorFormSurf(i, area), power(power), mode(mode)
        {
            edge = Util::scene()->edges[QString::fromStdString(area).toInt() - 1];
        }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {            
            SceneMaterialRF *material = dynamic_cast<SceneMaterialRF *>(Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->elem_marker).c_str())]->material);

            double mu = material->permeability.number * MU0;
            double eps = material->permittivity.number * EPS0;
            double conductivity = material->conductivity.number;

            double length = sqrt(sqr(edge->nodeEnd->point.x - edge->nodeStart->point.x) +
                                 sqr(edge->nodeEnd->point.y - edge->nodeStart->point.y));

            scalar result = 0;
            for (int i = 0; i < n; i++)
            {
                scalar lengthPoint = sqrt(sqr(e->x[i] - edge->nodeStart->point.x) +
                                          sqr(e->y[i] - edge->nodeStart->point.y));

                result += wt[i] * sin((lengthPoint * M_PI) / length) * v->val[i];
            }
            return power * 1e4 * result;
            // sqrt((eps - conductivity / frequency) / mu)
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e,
                        ExtData<Ord> *ext) const {
            return int_v<Ord>(n, wt, v);
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::VectorFormSurf* clone() {
            return new CustomVectorFormSurfPort(*this);
        }

    private:
        int mode;
        scalar power;
        SceneEdge *edge;
    };

    class CustomMatrixFormSurfMatchedBoundary : public WeakForm::MatrixFormSurf
    {
    public:
        CustomMatrixFormSurfMatchedBoundary(int i, int j, std::string area, scalar coeff, scalar frequency)
            : WeakForm::MatrixFormSurf(i, j, area), coeff(coeff), frequency(frequency) { }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {

            SceneMaterialRF *material = dynamic_cast<SceneMaterialRF *>(Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->elem_marker).c_str())]->material);

            double mu = material->permeability.number * MU0;
            double eps = material->permittivity.number * EPS0;
            double conductivity = material->conductivity.number;

            return coeff * sqrt((eps - conductivity / frequency) / mu) * int_u_v<double, scalar>(n, wt, u, v);
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u,
                        Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
            return int_u_v<Ord, Ord>(n, wt, u, v);
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::MatrixFormSurf* clone() {
            return new CustomMatrixFormSurfMatchedBoundary(*this);
        }

    private:
        scalar coeff, frequency;
    };
};
*/
/*
template<typename Real, typename Scalar>
Scalar rf_vector_form_surf_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (rfEdge[e->edge_marker].type != PhysicFieldBC_RF_Port)
        return 0.0;

    //   return 0.0;
    // dodelat clen  + 2*j*beta*E0z
    // kde E0z je podle mode
    // pro mode = 1 => pulvlna sinusovky
    // e->x[i]

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    Scalar E0z = 0.0;
    double beta = 0.0;
    int mode = 0;
    Scalar length = sqrt(sqr(rfEdge[e->edge_marker].end.x - rfEdge[e->edge_marker].start.x) +
                         sqr(rfEdge[e->edge_marker].end.y - rfEdge[e->edge_marker].start.y));

    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        Scalar lengthPoint = sqrt(sqr(e->x[i] - rfEdge[e->edge_marker].start.x) +
                                  sqr(e->y[i] - rfEdge[e->edge_marker].start.y));

        switch (rfEdge[e->edge_marker].mode)
        {
        case TEMode_0:
        {
            mode = 1;
            if(!rfEdge[e->edge_marker].height == 0)
                beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            else
                beta = sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps;
            E0z = rfEdge[e->edge_marker].power * sin((lengthPoint * M_PI) / length);
        }

            break;
        case TEMode_1:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  rfEdge[e->edge_marker].power * cos((lengthPoint * M_PI) / (rfEdge[e->edge_marker].height));
        }
            break;
        case TEMode_2:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  0;
        }
            break;
        default:
            break;
        }

        E0z *= sin(rfEdge[e->edge_marker].phase / 180.0 * M_PI);

        if (isPlanar)
            result += wt[i] * E0z * 2 * beta * (v->val[i]);
        else
            result += 2 * M_PI * wt[i] * E0z * 2 * beta * (e->x[i] * v->val[i]);
    }

    return result;
}

template<typename Real, typename Scalar>
Scalar rf_vector_form_surf_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (rfEdge[e->edge_marker].type != PhysicFieldBC_RF_Port)
        return 0.0;

    //   return 0.0;
    // dodelat clen  + 2*j*beta*E0z
    // kde E0z je podle mode
    // pro mode = 1 => pulvlna sinusovky
    // e->x[i]

    double mu = rfLabel[e->elem_marker].permeability * MU0;
    double eps = rfLabel[e->elem_marker].permittivity * EPS0;
    Scalar E0z = 0.0;
    double beta = 0.0;
    int mode = 0;
    Scalar length = sqrt(sqr(rfEdge[e->edge_marker].end.x - rfEdge[e->edge_marker].start.x) +
                         sqr(rfEdge[e->edge_marker].end.y - rfEdge[e->edge_marker].start.y));

    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        Scalar lengthPoint = sqrt(sqr(e->x[i] - rfEdge[e->edge_marker].start.x) +
                                  sqr(e->y[i] - rfEdge[e->edge_marker].start.y));

        switch (rfEdge[e->edge_marker].mode)
        {
        case TEMode_0:
        {
            mode = 1;
            if(!rfEdge[e->edge_marker].height == 0)
                beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            else
                beta = sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps;
            E0z = rfEdge[e->edge_marker].power * sin((lengthPoint * M_PI) / length);
        }

            break;
        case TEMode_1:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  rfEdge[e->edge_marker].power * cos((lengthPoint * M_PI) / (rfEdge[e->edge_marker].height));
        }
            break;
        case TEMode_2:
        {
            mode = 1;
            beta = sqrt(sqr(2 * M_PI * Util::scene()->problemInfo()->frequency) * mu * eps - sqr(mode * M_PI / (rfEdge[e->edge_marker].height)));
            E0z =  0;
        }
            break;
        default:
            break;
        }

        E0z *= - cos(rfEdge[e->edge_marker].phase / 180.0 * M_PI);

        if (isPlanar)
            result += wt[i] * E0z * 2 * beta * (v->val[i]);
        else
            result += 2 * M_PI * wt[i] * E0z * 2 * beta * (e->x[i] * v->val[i]);
    }
    return result;
}

*/

SceneBoundary *ModuleRF::newBoundary()
{
    return new SceneBoundary(tr("new boundary").toStdString(), "");
}

// *************************************************************************************************************************************

SceneBoundaryRFDialog::SceneBoundaryRFDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryRFDialog::createContent()
{
    lblValueUnitReal = new QLabel("");
    lblValueUnitImag = new QLabel("");

    /*
    cmbType = new QComboBox(this);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_ElectricField), PhysicFieldBC_RF_ElectricField);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_SurfaceCurrent), PhysicFieldBC_RF_SurfaceCurrent);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_MatchedBoundary), PhysicFieldBC_RF_MatchedBoundary);
    cmbType->addItem(physicFieldBCString(PhysicFieldBC_RF_Port), PhysicFieldBC_RF_Port);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));
    */
    cmbType = new QComboBox(this);
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    txtValueReal = new ValueLineEdit(this, true);
    txtValueImag = new ValueLineEdit(this, true);

    cmbMode = new QComboBox(this);
    cmbMode->addItem(teModeString(Mode_0), Mode_0);
    cmbMode->addItem(teModeString(Mode_01), Mode_01);
    cmbMode->addItem(teModeString(Mode_02), Mode_02);

    // set active marker
    doTypeChanged(cmbType->currentIndex());

    connect(txtValueReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtValueImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutPhysicField = new QHBoxLayout();
    layoutPhysicField->addWidget(txtValueReal);
    layoutPhysicField->addWidget(lblValueUnitImag, 10, 0);
    layoutPhysicField->addWidget(txtValueImag);

    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    layout->addWidget(lblValueUnitReal, 10, 0);
    layout->addLayout(layoutPhysicField, 10, 2);
    layout->addWidget(new QLabel(tr("Mode:")), 12, 0);
    layout->addWidget(cmbMode, 12, 2);
}

void SceneBoundaryRFDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
    if (m_boundary->type == "rf_electric_field")
    {
        txtValueReal->setValue(m_boundary->get_value("rf_electric_field_real"));
        txtValueImag->setValue(m_boundary->get_value("rf_electric_field_imag"));
    }
    else if (m_boundary->type == "rf_magnetic_field")
    {
        txtValueReal->setValue(m_boundary->get_value("rf_magnetic_field_real"));
        txtValueImag->setValue(m_boundary->get_value("rf_magnetic_field_imag"));
    }
    else if (m_boundary->type == "rf_surface_current")
    {
        txtValueReal->setValue(m_boundary->get_value("rf_surface_current_real"));
        txtValueImag->setValue(m_boundary->get_value("rf_surface_current_imag"));
    }
    else if (m_boundary->type == "rf_port")
    {
        txtValueReal->setValue(m_boundary->get_value("rf_port_power"));
        txtValueImag->setValue(m_boundary->get_value("rf_port_phase"));
    }
}

bool SceneBoundaryRFDialog::save()
{
    if (!SceneBoundaryDialog::save()) return false;;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    if (txtValueReal->evaluate() && txtValueImag->evaluate())
        if (m_boundary->type == "rf_electric_field")
        {
            m_boundary->values["rf_electric_field_real"] = txtValueReal->value();
            m_boundary->values["rf_electric_field_imag"] = txtValueImag->value();
        }
        else if (m_boundary->type == "rf_magnetic_field")
        {
            m_boundary->values["rf_magnetic_field_real"] = txtValueReal->value();
            m_boundary->values["rf_magnetic_field_imag"] = txtValueImag->value();
        }
        else if (m_boundary->type == "rf_surface_current")
        {
            m_boundary->values["rf_surface_current_real"] = txtValueReal->value();
            m_boundary->values["rf_surface_current_imag"] = txtValueImag->value();
        }
        else if (m_boundary->type == "rf_port")
        {
            m_boundary->values["rf_port_power"] = txtValueReal->value();
            m_boundary->values["rf_port_phase"] = txtValueImag->value();
        }
        else
            return false;

    return true;
}

void SceneBoundaryRFDialog::doTypeChanged(int index)
{
    txtValueReal->setEnabled(false);
    txtValueImag->setEnabled(false);
    cmbMode->setEnabled(false);

    // read equation
    readEquation(lblEquationImage, cmbType->itemData(index).toString());

    // enable controls
    if (cmbType->itemData(index) == "rf_electric_field")
    {
        txtValueReal->setEnabled(true);
        lblValueUnitReal->setText(tr("<i>E</i><sub>%1</sub>").arg(Util::scene()->problemInfo()->labelZ().toLower()));
        lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
        lblValueUnitImag->setText(tr(" + j "));
        lblValueUnitImag->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "rf_magnetic_field")
    {
        txtValueReal->setEnabled(true);
        lblValueUnitReal->setText(tr("<i>H</i><sub>0</sub>"));
        lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
        lblValueUnitImag->setText(tr(" + j "));
        lblValueUnitImag->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "rf_surface_current")
    {
        txtValueReal->setEnabled(true);
        lblValueUnitReal->setText(tr("<i>J</i><sub>0</sub>"));
        lblValueUnitReal->setToolTip(cmbType->itemText(index));
        txtValueImag->setEnabled(true);
        lblValueUnitImag->setText(tr(" + j "));
        lblValueUnitImag->setToolTip(cmbType->itemText(index));
    }
    else if (cmbType->itemData(index) == "rf_port")
    {
        txtValueReal->setEnabled(true);
        lblValueUnitReal->setText(tr("<i>P</i> (W):"));
        lblValueUnitReal->setToolTip(tr("Power"));
        txtValueImag->setEnabled(true);
        lblValueUnitImag->setText(tr("<i>%1</i> (°):").arg(QString::fromUtf8("φ")));
        lblValueUnitImag->setToolTip(tr("Phase"));
        cmbMode->setEnabled(true);
    }
}


// *************************************************************************************************************************************

SceneMaterialRFDialog::SceneMaterialRFDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;

    createDialog();

    load();
    setSize();
}

void SceneMaterialRFDialog::createContent()
{
    txtPermittivity = new ValueLineEdit(this);
    txtPermeability = new ValueLineEdit(this);
    txtConductivity = new ValueLineEdit(this);
    txtCurrentDensityReal = new ValueLineEdit(this);
    txtCurrentDensityImag = new ValueLineEdit(this);

    connect(txtPermeability, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtConductivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityReal, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtCurrentDensityImag, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPermittivity, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QHBoxLayout *layoutCurrentDensity = new QHBoxLayout();
    layoutCurrentDensity->addWidget(txtCurrentDensityReal);
    layoutCurrentDensity->addWidget(new QLabel(" + j "));
    layoutCurrentDensity->addWidget(txtCurrentDensityImag);

    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("ε")),
                                  tr("Permittivity")), 10, 0);
    layout->addWidget(txtPermittivity, 10, 2);
    layout->addWidget(createLabel(tr("<i>%1</i><sub>r</sub> (-)").arg(QString::fromUtf8("μ")),
                                  tr("Permeability")), 11, 0);
    layout->addWidget(txtPermeability, 11, 2);
    layout->addWidget(createLabel(tr("<i>%1</i> (S/m)").arg(QString::fromUtf8("σ")),
                                  tr("Conductivity")), 12, 0);
    layout->addWidget(txtConductivity, 12, 2);
    layout->addWidget(new QLabel(), 13, 0);
    layout->addWidget(createLabel(tr("<i>J</i><sub>ext</sub> (A/m<sup>2</sup>)"),
                                  tr("Current density")), 13, 0);
    layout->addLayout(layoutCurrentDensity, 13, 2);
}

void SceneMaterialRFDialog::load()
{
    SceneMaterialDialog::load();

    txtPermittivity->setValue(m_material->get_value("rf_permittivity"));
    txtPermeability->setValue(m_material->get_value("rf_permeability"));
    txtConductivity->setValue(m_material->get_value("rf_conductivity"));
    txtCurrentDensityReal->setValue(m_material->get_value("rf_surface_current_external_real"));
    txtCurrentDensityImag->setValue(m_material->get_value("rf_surface_current_external_imag"));
}

bool SceneMaterialRFDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    if (txtPermittivity->evaluate())
        m_material->values["rf_permittivity"] = txtPermittivity->value();
    else
        return false;

    if (txtPermeability->evaluate())
        m_material->values["rf_permeability"] = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        m_material->values["rf_conductivity"] = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        m_material->values["rf_surface_current_external_real"] = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        m_material->values["rf_surface_current_external_imag"] = txtCurrentDensityImag->value();
    else
        return false;

    return true;
}
