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

template<typename Real, typename Scalar>
Scalar int_u_dvdx_over_x_check(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e)
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * ((e->x[i] > 0) ? u->val[i] * v->dx[i] / e->x[i] : 0.0);
    return result;
}

class WeakFormMagnetics : public WeakForm
{
public:
    WeakFormMagnetics(int neq) : WeakForm(neq)
    {
        // boundary conditions
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(Util::scene()->edges[i]->marker);

            if (edgeMagneticMarker && Util::scene()->edges[i]->marker != Util::scene()->edgeMarkers[0])
            {
                if (edgeMagneticMarker->type == PhysicFieldBC_Magnetic_SurfaceCurrent)
                {
                    if (fabs(edgeMagneticMarker->value_real.number) > EPS_ZERO)
                        add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                        QString::number(i + 1).toStdString(),
                                                                                                        edgeMagneticMarker->value_real.number,
                                                                                                        convertProblemType(Util::scene()->problemInfo()->problemType)));
                    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                        if (fabs(edgeMagneticMarker->value_imag.number) > EPS_ZERO)
                            add_vector_form_surf(new WeakFormsH1::SurfaceVectorForms::DefaultVectorFormSurf(0,
                                                                                                            QString::number(i + 1).toStdString(),
                                                                                                            edgeMagneticMarker->value_imag.number,
                                                                                                            convertProblemType(Util::scene()->problemInfo()->problemType)));
                }
            }
        }

        // materials
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[i]->marker);

            if (labelMagneticMarker && Util::scene()->labels[i]->marker != Util::scene()->labelMarkers[0])
            {
                // steady state and transient analysis
                add_matrix_form(new DefaultLinearMagnetostatics(0, 0,
                                                                QString::number(i).toStdString(),
                                                                1.0 / (labelMagneticMarker->permeability.number * MU0), HERMES_NONSYM,
                                                                convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 10)));

                // velocity
                if ((fabs(labelMagneticMarker->conductivity.number) > EPS_ZERO) &&
                        ((fabs(labelMagneticMarker->velocity_x.number) > EPS_ZERO) ||
                        (fabs(labelMagneticMarker->velocity_y.number) > EPS_ZERO) ||
                        (fabs(labelMagneticMarker->velocity_angular.number) > EPS_ZERO)))
                    add_matrix_form(new DefaultLinearMagnetostaticsVelocity(0, 0,
                                                                            QString::number(i).toStdString(),
                                                                            labelMagneticMarker->conductivity.number,
                                                                            labelMagneticMarker->velocity_x.number,
                                                                            labelMagneticMarker->velocity_y.number,
                                                                            labelMagneticMarker->velocity_angular.number));

                // external current density
                if (fabs(labelMagneticMarker->current_density_real.number) > EPS_ZERO)
                    add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(0,
                                                                                                   QString::number(i).toStdString(),
                                                                                                   labelMagneticMarker->current_density_real.number,
                                                                                                   HERMES_PLANAR));

                // remanence
                if (fabs(labelMagneticMarker->remanence.number) > EPS_ZERO)
                    add_vector_form(new DefaultLinearMagnetostaticsRemanence(0,
                                                                             QString::number(i).toStdString(),
                                                                             labelMagneticMarker->permeability.number * MU0,
                                                                             labelMagneticMarker->remanence.number,
                                                                             labelMagneticMarker->remanence_angle.number));

                // harmonic analysis
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    add_matrix_form(new DefaultLinearMagnetostatics(1, 1,
                                                                    QString::number(i).toStdString(),
                                                                    1.0 / (labelMagneticMarker->permeability.number * MU0),
                                                                    HERMES_NONSYM,
                                                                    convertProblemType(Util::scene()->problemInfo()->problemType),
                                                                    (Util::scene()->problemInfo()->problemType == ProblemType_Planar ? 0 : 3)));

                    if (fabs(labelMagneticMarker->conductivity.number) > EPS_ZERO)
                    {
                        add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(0, 1,
                                                                                                  QString::number(i).toStdString(),
                                                                                                  2 * M_PI * Util::scene()->problemInfo()->frequency * labelMagneticMarker->conductivity.number,
                                                                                                  HERMES_NONSYM,
                                                                                                  HERMES_PLANAR));

                        add_matrix_form(new WeakFormsH1::VolumetricMatrixForms::DefaultLinearMass(1, 0,
                                                                                                  QString::number(i).toStdString(),
                                                                                                  - 2 * M_PI * Util::scene()->problemInfo()->frequency * labelMagneticMarker->conductivity.number,
                                                                                                  HERMES_NONSYM,
                                                                                                  HERMES_PLANAR));
                    }

                    // external current density
                    if (fabs(labelMagneticMarker->current_density_imag.number) > EPS_ZERO)
                        add_vector_form(new WeakFormsH1::VolumetricVectorForms::DefaultVectorFormConst(1,
                                                                                                       QString::number(i).toStdString(),
                                                                                                       labelMagneticMarker->current_density_imag.number,
                                                                                                       HERMES_PLANAR));
                }
            }
        }
    }

    class DefaultLinearMagnetostatics : public WeakForm::MatrixFormVol
    {
    public:
        // The optional order_increase takes into account the axisymmetric part.
        DefaultLinearMagnetostatics(int i, int j, scalar nu = 1.0, SymFlag sym = HERMES_SYM, GeomType gt = HERMES_PLANAR, int order_increase = 3)
            : WeakForm::MatrixFormVol(i, j, sym), nu(nu), gt(gt), order_increase(order_increase) { }

        DefaultLinearMagnetostatics(int i, int j, std::string area, scalar nu = 1.0, scalar gamma = 0.0, scalar vel_x = 0.0, scalar vel_y = 0.0,
                                    scalar vel_ang = 0.0, SymFlag sym = HERMES_SYM, GeomType gt = HERMES_PLANAR, int order_increase = 3)
            : WeakForm::MatrixFormVol(i, j, sym, area), nu(nu), gt(gt), order_increase(order_increase) { }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u,
                             Func<double> *v, Geom<double> *e, ExtData<scalar> *ext) const {
            scalar planar_part = int_grad_u_grad_v<double, scalar>(n, wt, u, v);
            scalar axisym_part = 0;
            if (gt == HERMES_AXISYM_X)
                axisym_part = int_u_dvdy_over_y<double, scalar>(n, wt, u, v, e);
            else if (gt == HERMES_AXISYM_Y)
                axisym_part = int_u_dvdx_over_x_check<double, scalar>(n, wt, u, v, e);

            return nu * (planar_part + axisym_part);
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v,
                        Geom<Ord> *e, ExtData<Ord> *ext) const {
            Ord planar_part = int_grad_u_grad_v<Ord, Ord>(n, wt, u, v);

            // This increase is for the axisymmetric part. We are not letting the
            // Ord class do it since it would automatically choose the highest order
            // due to the nonpolynomial 1/r term.
            return planar_part * Ord(order_increase);
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::MatrixFormVol* clone() {
            return new DefaultLinearMagnetostatics(*this);
        }

    private:
        scalar nu;
        GeomType gt;
        int order_increase;
    };

    class DefaultLinearMagnetostaticsRemanence : public WeakForm::VectorFormVol
    {
    public:
        DefaultLinearMagnetostaticsRemanence(int i, double perm, double rem, double rem_ang, GeomType gt = HERMES_PLANAR)
            : WeakForm::VectorFormVol(i), perm(perm), rem(rem), rem_ang(rem_ang), gt(gt) { }

        DefaultLinearMagnetostaticsRemanence(int i, std::string area, double perm, double rem, double rem_ang, GeomType gt = HERMES_PLANAR)
            : WeakForm::VectorFormVol(i, area), perm(perm), rem(rem), rem_ang(rem_ang), gt(gt) { }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {
            scalar result = 0;
            for (int i = 0; i < n; i++)
                result += wt[i] * rem/perm * (- sin(rem_ang / 180.0 * M_PI) * v->dx[i]
                                              + cos(rem_ang / 180.0 * M_PI) * v->dy[i]);

            return (gt == HERMES_PLANAR ? -1.0 : 1.0) * result;
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v,
                        Geom<Ord> *e, ExtData<Ord> *ext) const {
            Ord result = 0;
            for (int i = 0; i < n; i++)
                result += wt[i] * (v->dx[i] + v->dy[i]);

            return result;
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::VectorFormVol* clone() {
            return new DefaultLinearMagnetostaticsRemanence(*this);
        }

    private:
        double perm, rem, rem_ang;
        GeomType gt;
    };

    class DefaultLinearMagnetostaticsVelocity : public WeakForm::MatrixFormVol
    {
    public:
        DefaultLinearMagnetostaticsVelocity(int i, int j, double gamma, double vel_x, double vel_y, double vel_ang = 0.0)
            : WeakForm::MatrixFormVol(i, j), gamma(gamma), vel_x(vel_x), vel_y(vel_y), vel_ang(vel_ang) { }

        DefaultLinearMagnetostaticsVelocity(int i, int j, std::string area, double gamma, double vel_x, double vel_y, double vel_ang = 0.0)
            : WeakForm::MatrixFormVol(i, j, HERMES_NONSYM, area), gamma(gamma), vel_x(vel_x), vel_y(vel_y), vel_ang(vel_ang) { }

        virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, Func<double> *v,
                             Geom<double> *e, ExtData<scalar> *ext) const {
            scalar result = 0;
            for (int i = 0; i < n; i++)
                result += wt[i] * u->val[i] * ((vel_x - e->y[i] * vel_ang) * v->dx[i] +
                                               (vel_y + e->x[i] * vel_ang) * v->dy[i]);

            return -gamma * result;
        }

        virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v,
                        Geom<Ord> *e, ExtData<Ord> *ext) const {
            Ord result = 0;
            for (int i = 0; i < n; i++)
                result += wt[i] * u->val[i] * (v->dx[i] + v->dy[i]);

            return result;
        }

        // This is to make the form usable in rk_time_step().
        virtual WeakForm::MatrixFormVol* clone() {
            return new DefaultLinearMagnetostaticsVelocity(*this);
        }

    private:
        double gamma, vel_x, vel_y, vel_ang;
    };
};

/*
template<typename Real, typename Scalar>
Scalar magnetic_vector_form_surf_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double K = 0.0;

    if (magneticEdge[e->edge_marker].type == PhysicFieldBC_Magnetic_SurfaceCurrent)
        K = magneticEdge[e->edge_marker].value_real;

    if (isPlanar)
        return K * int_v<Real, Scalar>(n, wt, v);
    else
        return K * int_v<Real, Scalar>(n, wt, v);
}

template<typename Real, typename Scalar>
Scalar magnetic_vector_form_surf_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double K = 0.0;

    if (magneticEdge[e->edge_marker].type == PhysicFieldBC_Magnetic_SurfaceCurrent)
        K = magneticEdge[e->edge_marker].value_imag;

    if (isPlanar)
        return K * int_v<Real, Scalar>(n, wt, v);
    else
        return K * int_v<Real, Scalar>(n, wt, v);
}

template<typename Real, typename Scalar>
Scalar magnetic_matrix_form_real_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0.0;
    if (isPlanar)
        for (int i = 0; i < n; i++)
        {
            result += 1.0 / (magneticLabel[e->elem_marker].permeability * MU0)
                    * wt[i] * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i])
                    - magneticLabel[e->elem_marker].conductivity * (wt[i] * u->val[i] *
                                                                    ((magneticLabel[e->elem_marker].velocity_x - e->y[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dx[i] +
                                                                     (magneticLabel[e->elem_marker].velocity_y + e->x[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dy[i]))

                    + ((analysisType == AnalysisType_Transient) ? magneticLabel[e->elem_marker].conductivity * wt[i] * (u->val[i] * v->val[i]) / timeStep : 0.0);
        }
    else
        for (int i = 0; i < n; i++)
        {
            result += 1.0 / (magneticLabel[e->elem_marker].permeability * MU0)
                    * (wt[i] * (e->x[i] > 0.0 ? ((v->dx[i] * u->val[i]) / e->x[i]) : 0.0) + wt[i] * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i]))
                    - magneticLabel[e->elem_marker].conductivity * (wt[i] * u->val[i] *
                                                                    ((magneticLabel[e->elem_marker].velocity_x - e->y[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dx[i] +
                                                                     (magneticLabel[e->elem_marker].velocity_y + e->x[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dy[i]))

            + ((analysisType == AnalysisType_Transient) ? magneticLabel[e->elem_marker].conductivity * wt[i] * (u->val[i] * v->val[i]) / timeStep : 0.0);
        }

    return result;
}

template<typename Real, typename Scalar>
Scalar magnetic_matrix_form_real_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0.0;
    if (isPlanar)
        for (int i = 0; i < n; i++)
        {
            result += - 2 * M_PI * frequency * magneticLabel[e->elem_marker].conductivity * wt[i] * (u->val[i] * v->val[i]);
        }
    else
        for (int i = 0; i < n; i++)
        {
            result += - 2 * M_PI * frequency * magneticLabel[e->elem_marker].conductivity * wt[i] * (u->val[i] * v->val[i]);
        }

    return result;
}

template<typename Real, typename Scalar>
Scalar magnetic_matrix_form_imag_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0.0;
    if (isPlanar)
        for (int i = 0; i < n; i++)
        {
            result += 2 * M_PI * frequency * magneticLabel[e->elem_marker].conductivity * wt[i] * (u->val[i] * v->val[i]);
        }
    else
        for (int i = 0; i < n; i++)
        {
            result += 2 * M_PI * frequency * magneticLabel[e->elem_marker].conductivity * wt[i] * (u->val[i] * v->val[i]);
        }

    return result;
}

template<typename Real, typename Scalar>
Scalar magnetic_matrix_form_imag_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0.0;
    if (isPlanar)
        for (int i = 0; i < n; i++)
        {
            result += 1.0 / (magneticLabel[e->elem_marker].permeability * MU0)
                    * wt[i] * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i])
                    - magneticLabel[e->elem_marker].conductivity * (wt[i] * u->val[i] *
                                                                    ((magneticLabel[e->elem_marker].velocity_x - e->y[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dx[i] +
                                                                     (magneticLabel[e->elem_marker].velocity_y + e->x[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dy[i]));
        }
    else
        for (int i = 0; i < n; i++)
        {
            result += 1.0 / (magneticLabel[e->elem_marker].permeability * MU0)
                    * (wt[i] * (e->x[i] > 0.0 ? ((v->dx[i] * u->val[i]) / e->x[i]) : 0.0) + wt[i] * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i]))
                    - magneticLabel[e->elem_marker].conductivity * (wt[i] * u->val[i] *
                                                                    ((magneticLabel[e->elem_marker].velocity_x - e->y[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dx[i] +
                                                                     (magneticLabel[e->elem_marker].velocity_y + e->x[i] * magneticLabel[e->elem_marker].velocity_angular) * v->dy[i]));
        }

    return result;
}

template<typename Real, typename Scalar>
Scalar magnetic_vector_form_real(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0.0;
    if (isPlanar)
        for (int i = 0; i < n; i++)
        {
            result += wt[i] * magneticLabel[e->elem_marker].current_density_real * (v->val[i])
                    + wt[i] * magneticLabel[e->elem_marker].remanence / (magneticLabel[e->elem_marker].permeability * MU0)
                    * (- sin(magneticLabel[e->elem_marker].remanence_angle / 180.0 * M_PI) * v->dx[i]
                       + cos(magneticLabel[e->elem_marker].remanence_angle / 180.0 * M_PI) * v->dy[i])

            + ((analysisType == AnalysisType_Transient) ? magneticLabel[e->elem_marker].conductivity * wt[i] * (ext->fn[0]->val[i] * v->val[i]) / timeStep : 0.0);
        }
    else
        for (int i = 0; i < n; i++)
        {
            result += wt[i] * magneticLabel[e->elem_marker].current_density_real  * (v->val[i])
                    - wt[i] * magneticLabel[e->elem_marker].remanence / (magneticLabel[e->elem_marker].permeability * MU0)
                    * (- sin(magneticLabel[e->elem_marker].remanence_angle / 180.0 * M_PI) * v->dx[i]
                       + cos(magneticLabel[e->elem_marker].remanence_angle / 180.0 * M_PI) * v->dy[i])

            + ((analysisType == AnalysisType_Transient) ? magneticLabel[e->elem_marker].conductivity * wt[i] * (ext->fn[0]->val[i] * v->val[i]) / timeStep : 0.0);
        }

    return result;
}

template<typename Real, typename Scalar>
Scalar magnetic_vector_form_imag(int n, double *wt, Func<Real> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    Scalar result = 0.0;
    if (isPlanar)
        for (int i = 0; i < n; i++)
        {
            result += magneticLabel[e->elem_marker].current_density_imag * wt[i] * (v->val[i]);
        }
    else
        for (int i = 0; i < n; i++)
        {
            result += magneticLabel[e->elem_marker].current_density_imag * wt[i] * (v->val[i]);
        }

    return result;
}

void callbackMagneticWeakForm(WeakForm *wf, Hermes::vector<Solution *> slnArray)
{
    if (slnArray.size() == 1)
    {
        wf->add_matrix_form(0, 0, callback(magnetic_matrix_form_real_real));
        if (analysisType == AnalysisType_Transient)
            wf->add_vector_form(0, callback(magnetic_vector_form_real), HERMES_ANY, slnArray.at(0));
        else
            wf->add_vector_form(0, callback(magnetic_vector_form_real));
        wf->add_vector_form_surf(0, callback(magnetic_vector_form_surf_real));
    }
    else
    {
        wf->add_matrix_form(0, 0, callback(magnetic_matrix_form_real_real));
        wf->add_matrix_form(0, 1, callback(magnetic_matrix_form_real_imag));
        wf->add_matrix_form(1, 0, callback(magnetic_matrix_form_imag_real));
        wf->add_matrix_form(1, 1, callback(magnetic_matrix_form_imag_imag));
        wf->add_vector_form(0, callback(magnetic_vector_form_real));
        wf->add_vector_form(1, callback(magnetic_vector_form_imag));
        wf->add_vector_form_surf(0, callback(magnetic_vector_form_surf_real));
        wf->add_vector_form_surf(1, callback(magnetic_vector_form_surf_imag));
    }
}
*/
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


void HermesMagnetic::readEdgeMarkerFromDomElement(QDomElement *element)
{
    PhysicFieldBC type = physicFieldBCFromStringKey(element->attribute("type"));
    switch (type)
    {
    case PhysicFieldBC_None:
    case PhysicFieldBC_Magnetic_VectorPotential:
    case PhysicFieldBC_Magnetic_SurfaceCurrent:
        Util::scene()->addEdgeMarker(new SceneEdgeMagneticMarker(element->attribute("name"),
                                                                 type,
                                                                 Value(element->attribute("value_real", "0")),
                                                                 Value(element->attribute("value_imag", "0"))));
        break;
    default:
        std::cerr << tr("Boundary type '%1' doesn't exists.").arg(element->attribute("type")).toStdString() << endl;
        break;
    }
}

void HermesMagnetic::writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker)
{
    SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(marker);

    element->setAttribute("type", physicFieldBCToStringKey(edgeMagneticMarker->type));
    element->setAttribute("value_real", edgeMagneticMarker->value_real.text);
    element->setAttribute("value_imag", edgeMagneticMarker->value_imag.text);
}

void HermesMagnetic::readLabelMarkerFromDomElement(QDomElement *element)
{
    Util::scene()->addLabelMarker(new SceneLabelMagneticMarker(element->attribute("name"),
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

void HermesMagnetic::writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker)
{
    SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(marker);

    element->setAttribute("current_density_real", labelMagneticMarker->current_density_real.text);
    element->setAttribute("current_density_imag", labelMagneticMarker->current_density_imag.text);
    element->setAttribute("permeability", labelMagneticMarker->permeability.text);
    element->setAttribute("conductivity", labelMagneticMarker->conductivity.text);
    element->setAttribute("remanence", labelMagneticMarker->remanence.text);
    element->setAttribute("remanence_angle", labelMagneticMarker->remanence_angle.text);
    element->setAttribute("velocity_x", labelMagneticMarker->velocity_x.text);
    element->setAttribute("velocity_y", labelMagneticMarker->velocity_y.text);
    element->setAttribute("velocity_angular", labelMagneticMarker->velocity_angular.text);}

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
    headers << "l" << "S" << "Fx" << "Fy";
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

SceneEdgeMarker *HermesMagnetic::newEdgeMarker()
{
    return new SceneEdgeMagneticMarker(tr("new boundary"),
                                       PhysicFieldBC_Magnetic_VectorPotential,
                                       Value("0"),
                                       Value("0"));
}

SceneEdgeMarker *HermesMagnetic::newEdgeMarker(PyObject *self, PyObject *args)
{
    double value_real;
    double value_imag = 0.0;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|d", &name, &type, &value_real, &value_imag))
    {
        // check name
        if (Util::scene()->getEdgeMarker(name)) return NULL;

        return new SceneEdgeMagneticMarker(name,
                                           physicFieldBCFromStringKey(type),
                                           Value(QString::number(value_real)),
                                           Value(QString::number(value_imag)));
    }

    return NULL;
}

SceneEdgeMarker *HermesMagnetic::modifyEdgeMarker(PyObject *self, PyObject *args)
{
    double value_real, value_imag = 0.0;
    char *name, *type;
    if (PyArg_ParseTuple(args, "ssd|d", &name, &type, &value_real, &value_imag))
    {
        if (SceneEdgeMagneticMarker *marker = dynamic_cast<SceneEdgeMagneticMarker *>(Util::scene()->getEdgeMarker(name)))
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

SceneLabelMarker *HermesMagnetic::newLabelMarker()
{
    return new SceneLabelMagneticMarker(tr("new material"),
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

SceneLabelMarker *HermesMagnetic::newLabelMarker(PyObject *self, PyObject *args)
{
    double current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity, &remanence, &remanence_angle, &velocity_x, &velocity_y, &velocity_angular))
    {
        // check name
        if (Util::scene()->getLabelMarker(name)) return NULL;

        return new SceneLabelMagneticMarker(name,
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

SceneLabelMarker *HermesMagnetic::modifyLabelMarker(PyObject *self, PyObject *args)
{
    double current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular;
    char *name;
    if (PyArg_ParseTuple(args, "sddddddddd", &name, &current_density_real, &current_density_imag, &permeability, &conductivity, &remanence, &remanence_angle, &velocity_x, &velocity_y, &velocity_angular))
    {
        if (SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->getLabelMarker(name)))
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
    for (int i = 1; i<Util::scene()->edgeMarkers.count(); i++)
    {
        SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(Util::scene()->edgeMarkers[i]);

        // evaluate script
        if (!edgeMagneticMarker->value_real.evaluate()) return QList<SolutionArray *>();
        if (!edgeMagneticMarker->value_imag.evaluate()) return QList<SolutionArray *>();
    }

    // label markers
    for (int i = 1; i<Util::scene()->labelMarkers.count(); i++)
    {
        SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labelMarkers[i]);

        // evaluate script
        if (!labelMagneticMarker->current_density_real.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->current_density_imag.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->permeability.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->conductivity.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->remanence.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->remanence_angle.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->velocity_x.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->velocity_y.evaluate()) return QList<SolutionArray *>();
        if (!labelMagneticMarker->velocity_angular.evaluate()) return QList<SolutionArray *>();
    }

    // boundary conditions
    EssentialBCs bc1;
    EssentialBCs bc2;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(Util::scene()->edges[i]->marker);

        if (edgeMagneticMarker)
        {
            if (edgeMagneticMarker->type == PhysicFieldBC_Magnetic_VectorPotential)
            {
                bc1.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), edgeMagneticMarker->value_real.number));
                bc2.add_boundary_condition(new DefaultEssentialBCConst(QString::number(i+1).toStdString(), edgeMagneticMarker->value_imag.number));
            }
        }
    }
    Hermes::vector<EssentialBCs> bcs(bc1, bc2);

    WeakFormMagnetics wf(numberOfSolution());

    QList<SolutionArray *> solutionArrayList = solveSolutioArray(progressItemSolve, bcs, &wf);

    return solutionArrayList;
}

void HermesMagnetic::updateTimeFunctions(double time)
{
    // update markers
    for (int i = 1; i<Util::scene()->labelMarkers.count(); i++)
    {
        SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labelMarkers[i]);
        labelMagneticMarker->current_density_real.evaluate(time);
        labelMagneticMarker->current_density_imag.evaluate(time);
    }
    // set values
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        // regular marker ("none" is reserved for holes)
        if (!(Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0))
        {
            // FIXME
            // SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[i]->marker);
            // magneticLabel[i].current_density_real = labelMagneticMarker->current_density_real.number;
            // magneticLabel[i].current_density_imag = labelMagneticMarker->current_density_imag.number;
        }
    }
}

// *************************************************************************************************************************************

SceneEdgeMagneticMarker::SceneEdgeMagneticMarker(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag) : SceneEdgeMarker(name, type)
{
    this->value_real = value_real;
    this->value_imag = value_imag;
}

QString SceneEdgeMagneticMarker::script()
{
    return QString("addboundary(\"%1\", \"%2\", %3, %4)").
            arg(name).
            arg(physicFieldBCToStringKey(type)).
            arg(value_real.text).
            arg(value_imag.text);
}

QMap<QString, QString> SceneEdgeMagneticMarker::data()
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

int SceneEdgeMagneticMarker::showDialog(QWidget *parent)
{
    DSceneEdgeMagneticMarker *dialog = new DSceneEdgeMagneticMarker(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabelMagneticMarker::SceneLabelMagneticMarker(const QString &name, Value current_density_real, Value current_density_imag, Value permeability, Value conductivity, Value remanence, Value remanence_angle, Value velocity_x, Value velocity_y, Value velocity_angular)
    : SceneLabelMarker(name)
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

QString SceneLabelMagneticMarker::script()
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

QMap<QString, QString> SceneLabelMagneticMarker::data()
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

int SceneLabelMagneticMarker::showDialog(QWidget *parent)
{
    DSceneLabelMagneticMarker *dialog = new DSceneLabelMagneticMarker(parent, this);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneEdgeMagneticMarker::DSceneEdgeMagneticMarker(SceneEdgeMagneticMarker *edgeMagneticMarker, QWidget *parent) : DSceneEdgeMarker(parent)
{
    m_edgeMarker = edgeMagneticMarker;

    createDialog();

    load();
    setSize();
}

void DSceneEdgeMagneticMarker::createContent()
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

void DSceneEdgeMagneticMarker::load()
{
    DSceneEdgeMarker::load();

    SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(m_edgeMarker);

    cmbType->setCurrentIndex(cmbType->findData(edgeMagneticMarker->type));
    txtValueReal->setValue(edgeMagneticMarker->value_real);
    txtValueImag->setValue(edgeMagneticMarker->value_imag);
}

bool DSceneEdgeMagneticMarker::save() {
    if (!DSceneEdgeMarker::save()) return false;;

    SceneEdgeMagneticMarker *edgeMagneticMarker = dynamic_cast<SceneEdgeMagneticMarker *>(m_edgeMarker);

    edgeMagneticMarker->type = (PhysicFieldBC) cmbType->itemData(cmbType->currentIndex()).toInt();

    if (txtValueReal->evaluate())
        edgeMagneticMarker->value_real  = txtValueReal->value();
    else
        return false;

    if (txtValueImag->evaluate())
        edgeMagneticMarker->value_imag  = txtValueImag->value();
    else
        return false;

    return true;
}

void DSceneEdgeMagneticMarker::doTypeChanged(int index)
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

DSceneLabelMagneticMarker::DSceneLabelMagneticMarker(QWidget *parent, SceneLabelMagneticMarker *labelMagneticMarker) : DSceneLabelMarker(parent)
{
    m_labelMarker = labelMagneticMarker;

    createDialog();

    load();
    setSize();
}

void DSceneLabelMagneticMarker::createContent()
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

void DSceneLabelMagneticMarker::load()
{
    DSceneLabelMarker::load();

    SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(m_labelMarker);

    txtPermeability->setValue(labelMagneticMarker->permeability);
    txtConductivity->setValue(labelMagneticMarker->conductivity);
    txtCurrentDensityReal->setValue(labelMagneticMarker->current_density_real);
    txtCurrentDensityImag->setValue(labelMagneticMarker->current_density_imag);
    txtRemanence->setValue(labelMagneticMarker->remanence);
    txtRemanenceAngle->setValue(labelMagneticMarker->remanence_angle);
    txtVelocityX->setValue(labelMagneticMarker->velocity_x);
    txtVelocityY->setValue(labelMagneticMarker->velocity_y);
    txtVelocityAngular->setValue(labelMagneticMarker->velocity_angular);
}

bool DSceneLabelMagneticMarker::save() {
    if (!DSceneLabelMarker::save()) return false;;

    SceneLabelMagneticMarker *labelMagneticMarker = dynamic_cast<SceneLabelMagneticMarker *>(m_labelMarker);

    if (txtPermeability->evaluate())
        labelMagneticMarker->permeability  = txtPermeability->value();
    else
        return false;

    if (txtConductivity->evaluate())
        labelMagneticMarker->conductivity  = txtConductivity->value();
    else
        return false;

    if (txtCurrentDensityReal->evaluate())
        labelMagneticMarker->current_density_real  = txtCurrentDensityReal->value();
    else
        return false;

    if (txtCurrentDensityImag->evaluate())
        labelMagneticMarker->current_density_imag  = txtCurrentDensityImag->value();
    else
        return false;

    if (txtRemanence->evaluate())
        labelMagneticMarker->remanence = txtRemanence->value();
    else
        return false;

    if (txtRemanenceAngle->evaluate())
        labelMagneticMarker->remanence_angle = txtRemanenceAngle->value();
    else
        return false;

    if (txtVelocityX->evaluate())
        labelMagneticMarker->velocity_x = txtVelocityX->value();
    else
        return false;

    if (txtVelocityY->evaluate())
        labelMagneticMarker->velocity_y = txtVelocityY->value();
    else
        return false;

    if (txtVelocityAngular->evaluate())
        labelMagneticMarker->velocity_angular = txtVelocityAngular->value();
    else
        return false;

    return true;
}
