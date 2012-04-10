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

LocalPointValueMagnetic::LocalPointValueMagnetic(const Point &point) : LocalPointValue(point)
{
    permeability = 0;
    conductivity = 0;
    remanence = 0;
    remanence_angle = 0;

    potential_real = 0;
    potential_imag = 0;

    current_density_real = 0;
    current_density_imag = 0;
    current_density_induced_transform_real = 0;
    current_density_induced_transform_imag = 0;
    current_density_induced_velocity_real = 0;
    current_density_induced_velocity_imag = 0;
    current_density_total_real = 0;
    current_density_total_imag = 0;

    H_real = Point();
    H_imag = Point();
    B_real = Point();
    B_imag = Point();
    FL_real = Point();
    FL_imag = Point();

    pj = 0;
    wm = 0;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        // value real
        PointValue valueReal = PointValue(value, derivative, material);

        SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(valueReal.marker);
        // solution
        if (marker != NULL)
        {
            if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState ||
                    Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            {
                Point derReal;
                derReal = valueReal.derivative;

                if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
                {
                    // potential
                    potential_real = valueReal.value;

                    // flux density
                    B_real.x =   derReal.y;
                    B_real.y = - derReal.x;
                }
                else
                {
                    // potential
                    potential_real = valueReal.value;

                    // flux density
                    B_real.x = -  derReal.y;
                    B_real.y =   (derReal.x + ((point.x > 0.0) ? valueReal.value/point.x : 0.0));
                }

                permeability = marker->permeability.number;
                conductivity = marker->conductivity.number;
                remanence = marker->remanence.number;
                remanence_angle = marker->remanence_angle.number;
                velocity = Point(marker->velocity_x.number - marker->velocity_angular.number * point.y,
                                 marker->velocity_y.number + marker->velocity_angular.number * point.x);

                // current density
                current_density_real = marker->current_density_real.number;

                // induced transform current density
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                {
                    Solution *sln2;
                    if (Util::scene()->sceneSolution()->timeStep() > 0)
                        sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() - 1);
                    else
                        sln2 = Util::scene()->sceneSolution()->sln();

                    PointValue valuePrevious = pointValue(sln2, point);
                    current_density_induced_transform_real = - marker->conductivity.number * (valueReal.value - valuePrevious.value) / Util::scene()->problemInfo()->timeStep.number;
                }

                // induced current density velocity
                current_density_induced_velocity_real = - conductivity * (velocity.x * derReal.x + velocity.y * derReal.y);

                // total current density
                current_density_total_real = current_density_real + current_density_induced_transform_real + current_density_induced_velocity_real;

                // electric displacement
                H_real = B_real / (marker->permeability.number * MU0);

                // Ltorentz force
                FL_real.x = - current_density_total_real*B_real.y;
                FL_real.y =   current_density_total_real*B_real.x;

                // power losses
                pj = (marker->conductivity.number > 0.0) ?
                            1.0 / marker->conductivity.number * (sqr(current_density_total_real) + sqr(current_density_total_imag))
                          :
                            0.0;

                // energy density
                wm = 0.5 * (sqr(B_real.x) + sqr(B_real.y) + sqr(B_imag.x) + sqr(B_imag.y)) / (marker->permeability.number * MU0);
            }

            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            {
                Solution *sln2 = Util::scene()->sceneSolution()->sln(1);

                // value imag
                PointValue valueImag = pointValue(sln2, point);
                double frequency = Util::scene()->problemInfo()->frequency;

                Point derReal = valueReal.derivative;
                Point derImag = valueImag.derivative;

                if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
                {
                    // potential
                    potential_real = valueReal.value;
                    potential_imag = valueImag.value;

                    // flux density
                    B_real.x =  derReal.y;
                    B_real.y = -derReal.x;

                    B_imag.x =  derImag.y;
                    B_imag.y = -derImag.x;
                }
                else
                {
                    // potential
                    potential_real = valueReal.value;
                    potential_imag = valueImag.value;

                    // flux density
                    B_real.x = -  derReal.y;
                    B_real.y =   (derReal.x + ((point.x > 0.0) ? valueReal.value/point.x : 0.0));

                    B_imag.x = -  derImag.y;
                    B_imag.y =   (derImag.x + ((point.x > 0.0) ? valueImag.value/point.x : 0.0));
                }

                permeability = marker->permeability.number;
                conductivity = marker->conductivity.number;
                remanence = marker->remanence.number;
                remanence_angle = marker->remanence_angle.number;
                velocity = Point(marker->velocity_x.number - marker->velocity_angular.number * point.y,
                                 marker->velocity_y.number + marker->velocity_angular.number * point.x);

                // current density
                current_density_real = marker->current_density_real.number;
                current_density_imag = marker->current_density_imag.number;

                // induced current density transform (harmonic)
                current_density_induced_transform_real =   2 * M_PI * frequency * marker->conductivity.number * valueImag.value;
                current_density_induced_transform_imag = - 2 * M_PI * frequency * marker->conductivity.number * valueReal.value;

                // induced current density velocity
                current_density_induced_velocity_real = - conductivity * (velocity.x * derReal.x + velocity.y * derReal.y);
                current_density_induced_velocity_imag = - conductivity * (velocity.x * derImag.x + velocity.y * derImag.y);

                // total current density
                current_density_total_real = current_density_real + current_density_induced_transform_real + current_density_induced_velocity_real;
                current_density_total_imag = current_density_imag + current_density_induced_transform_imag + current_density_induced_velocity_imag;

                // electric displacement
                H_real = B_real / (marker->permeability.number * MU0);
                H_imag = B_imag / (marker->permeability.number * MU0);

                // Lorentz force
                FL_real.x = - (current_density_total_real*B_real.y - current_density_total_imag*B_imag.y);
                FL_real.y =   (current_density_total_real*B_real.x - current_density_total_imag*B_imag.x);
                FL_imag.x = - (current_density_total_imag*B_real.y + current_density_total_real*B_imag.y);
                FL_imag.y =   (current_density_total_imag*B_real.x + current_density_total_real*B_imag.x);
                // axisymmetric harmonic field
                if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
                {
                    FL_real = FL_real * (-1.0);
                    FL_imag = FL_imag * (-1.0);
                }

                // power losses
                pj = (marker->conductivity.number > 0.0) ?
                            0.5 / marker->conductivity.number * (sqr(current_density_total_real) + sqr(current_density_total_imag))
                          :
                            0.0;

                // energy density
                wm = 0.25 * (sqr(B_real.x) + sqr(B_real.y) + sqr(B_imag.x) + sqr(B_imag.y)) / (marker->permeability.number * MU0);
            }
        }
    }
}

double LocalPointValueMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Magnetic_VectorPotential:
        return sqrt(sqr(potential_real) + sqr(potential_imag));
    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
        return potential_real;
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
        return potential_imag;
    case PhysicFieldVariable_Magnetic_FluxDensity:
        return sqrt(sqr(B_real.x) + sqr(B_imag.x) + sqr(B_real.y) + sqr(B_imag.y));
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return B_real.x;
        case PhysicFieldVariableComp_Y:
            return B_real.y;
        case PhysicFieldVariableComp_Magnitude:
            return B_real.magnitude();
        }
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return B_imag.x;
        case PhysicFieldVariableComp_Y:
            return B_imag.y;
        case PhysicFieldVariableComp_Magnitude:
            return B_imag.magnitude();
        }
    case PhysicFieldVariable_Magnetic_MagneticField:
        return sqrt(sqr(H_real.x) + sqr(H_imag.x) + sqr(H_real.y) + sqr(H_imag.y));
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return H_real.x;
        case PhysicFieldVariableComp_Y:
            return H_real.y;
        case PhysicFieldVariableComp_Magnitude:
            return H_real.magnitude();
        }
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return H_imag.x;
        case PhysicFieldVariableComp_Y:
            return H_imag.y;
        case PhysicFieldVariableComp_Magnitude:
            return H_imag.magnitude();
        }
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
        return current_density_real;
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
        return current_density_imag;
    case PhysicFieldVariable_Magnetic_CurrentDensity:
        return sqrt(sqr(current_density_real) + sqr(current_density_imag));
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
        return current_density_induced_transform_real;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
        return current_density_induced_transform_imag;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
        return sqrt(sqr(current_density_induced_transform_real) + sqr(current_density_induced_transform_imag));
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
        return current_density_induced_velocity_real;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
        return current_density_induced_velocity_imag;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
        return sqrt(sqr(current_density_induced_velocity_real) + sqr(current_density_induced_velocity_imag));
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
        return current_density_total_real;
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
        return current_density_total_imag;
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
        return sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag));
    case PhysicFieldVariable_Magnetic_PowerLosses:
        return pj;
    case PhysicFieldVariable_Magnetic_EnergyDensity:
        return wm;
    case PhysicFieldVariable_Magnetic_Permeability:
        return permeability;
    case PhysicFieldVariable_Magnetic_Conductivity:
        return conductivity;
    case PhysicFieldVariable_Magnetic_Velocity:
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return velocity.x;
        case PhysicFieldVariableComp_Y:
            return velocity.y;
        case PhysicFieldVariableComp_Magnitude:
            return sqrt(sqr(velocity.x) + sqr(velocity.y));
        }
    case PhysicFieldVariable_Magnetic_Remanence:
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return remanence * cos(remanence_angle / 180.0 * M_PI);
        case PhysicFieldVariableComp_Y:
            return remanence * sin(remanence_angle / 180.0 * M_PI);
        case PhysicFieldVariableComp_Magnitude:
            return remanence;
        }
    case PhysicFieldVariable_Magnetic_LorentzForce:
        switch (physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
            return FL_real.x;
        case PhysicFieldVariableComp_Y:
            return FL_real.y;
        case PhysicFieldVariableComp_Magnitude:
            return FL_real.magnitude();
        }
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        return 0.0;
    }
}

QStringList LocalPointValueMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential_real, 0, 'e', 5) <<
            QString("%1").arg(potential_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(potential_real) + sqr(potential_imag)), 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(B_real.x) + sqr(B_imag.x) + sqr(B_real.y) + sqr(B_imag.y)), 0, 'e', 5) <<
            QString("%1").arg(B_real.x, 0, 'e', 5) <<
            QString("%1").arg(B_real.y, 0, 'e', 5) <<
            QString("%1").arg(B_real.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(B_imag.x, 0, 'e', 5) <<
            QString("%1").arg(B_imag.y, 0, 'e', 5) <<
            QString("%1").arg(B_imag.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(H_real.x) + sqr(H_imag.x) + sqr(H_real.y) + sqr(H_imag.y)), 0, 'e', 5) <<
            QString("%1").arg(H_real.x, 0, 'e', 5) <<
            QString("%1").arg(H_real.y, 0, 'e', 5) <<
            QString("%1").arg(H_real.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(H_imag.x, 0, 'e', 5) <<
            QString("%1").arg(H_imag.y, 0, 'e', 5) <<
            QString("%1").arg(H_imag.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(current_density_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_real) + sqr(current_density_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_transform_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_transform_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_induced_transform_real) + sqr(current_density_induced_transform_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_velocity_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_velocity_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_induced_velocity_real) + sqr(current_density_induced_velocity_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_total_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_total_imag, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag)), 0, 'e', 5) <<
            QString("%1").arg(pj, 0, 'e', 5) <<
            QString("%1").arg(wm, 0, 'e', 5) <<
            QString("%1").arg(permeability, 0, 'f', 3) <<
            QString("%1").arg(conductivity, 0, 'e', 5) <<
            QString("%1").arg(remanence, 0, 'e', 5) <<
            QString("%1").arg(remanence_angle, 0, 'e', 5) <<
            QString("%1").arg(velocity.x, 0, 'e', 5) <<
            QString("%1").arg(velocity.y, 0, 'e', 5) <<
            QString("%1").arg(FL_real.x, 0, 'e', 5) <<
            QString("%1").arg(FL_real.y, 0, 'e', 5) <<
            QString("%1").arg(FL_imag.x, 0, 'e', 5) <<
            QString("%1").arg(FL_imag.y, 0, 'e', 5);

    return QStringList(row);
}

// ****************************************************************************************************************

SurfaceIntegralValueMagnetic::SurfaceIntegralValueMagnetic() : SurfaceIntegralValue()
{
    forceMaxwellX = 0.0;
    forceMaxwellY = 0.0;
    torque = 0.0;

    calculate();

    forceMaxwellX /= 2.0;
    forceMaxwellY /= 2.0;
    torque /= 2.0;
}

void SurfaceIntegralValueMagnetic::calculateVariables(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    if (fabs(marker->permeability.number - 1.0) < EPS_ZERO)
    {
        double nx =   tan[i][1];
        double ny = - tan[i][0];

        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            double Bx = - dudy1[i];
            double By =   dudx1[i];

            double dfX = pt[i][2] * tan[i][2] * 1.0 / (MU0 * marker->permeability.number) *
                    (Bx * (nx * Bx + ny * By) - 0.5 * nx * (sqr(Bx) + sqr(By)));
            double dfY = pt[i][2] * tan[i][2] * 1.0 / (MU0 * marker->permeability.number) *
                    (By * (nx * Bx + ny * By) - 0.5 * ny * (sqr(Bx) + sqr(By)));

            forceMaxwellX -= dfX;
            forceMaxwellY -= dfY;

            torque -= x[i] * dfY - y[i] * dfX;
        }
        else
        {
            double Bx = - dudy1[i];
            double By =  (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0));

            forceMaxwellX  = 0.0;

            forceMaxwellY -= 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * 1.0 / (MU0 * marker->permeability.number) *
                    (By * (nx * Bx + ny * By) - 0.5 * ny * (sqr(Bx) + sqr(By)));
        }
    }
}

void SurfaceIntegralValueMagnetic::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    sln2 = NULL;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() - 1);
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
}

QStringList SurfaceIntegralValueMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(length, 0, 'e', 5) <<
            QString("%1").arg(surface, 0, 'e', 5) <<
            QString("%1").arg(forceMaxwellX, 0, 'e', 5) <<
            QString("%1").arg(forceMaxwellY, 0, 'e', 5) <<
            QString("%1").arg(torque, 0, 'e', 5);
    return QStringList(row);
}


// ****************************************************************************************************************

VolumeIntegralValueMagnetic::VolumeIntegralValueMagnetic() : VolumeIntegralValue()
{
    currentReal = 0;
    currentImag = 0;
    currentInducedTransformReal = 0;
    currentInducedTransformImag = 0;
    currentInducedVelocityReal = 0;
    currentInducedVelocityImag = 0;
    currentTotalReal = 0;
    currentTotalImag = 0;
    powerLosses = 0;
    energy = 0;
    forceLorentzX = 0;
    forceLorentzY = 0;
    torque = 0;

    calculate();

    currentTotalReal = currentReal + currentInducedTransformReal + currentInducedVelocityReal;
    currentTotalImag = currentImag + currentInducedTransformImag + currentInducedVelocityImag;
}

void VolumeIntegralValueMagnetic::calculateVariables(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    // current - real
    result = 0.0;
    h1_integrate_expression(marker->current_density_real.number)
            currentReal += result;

    // current - imag
    result = 0.0;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        h1_integrate_expression(marker->current_density_imag.number)
    }
    currentImag += result;

    // current induced transform - real
    result = 0.0;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            h1_integrate_expression(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i])
                    else
                    h1_integrate_expression(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i])
    }
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            h1_integrate_expression(- marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number)
                    else
                    h1_integrate_expression(- marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number)
    }
    currentInducedTransformReal += result;

    // current induced transform - imag
    result = 0.0;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            h1_integrate_expression(- 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i])
                    else
                    h1_integrate_expression(- 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i])
    }
    currentInducedTransformImag += result;

    // current induced velocity - real
    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        h1_integrate_expression(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                else
                h1_integrate_expression(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                currentInducedVelocityReal += result;

    result = 0.0;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
            h1_integrate_expression(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]))
                    // TODO axisymmetric
    }
    currentInducedVelocityImag += result;

    calculateVariablesOther1(i);
    calculateVariablesOther2(i);
    calculateVariablesOther3(i);
    calculateVariablesOther4(i);
    calculateVariablesOther5(i);
}

void VolumeIntegralValueMagnetic::calculateVariablesOther1(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    // power losses
    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState)
        {
            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                        1.0 / marker->conductivity.number * sqr(
                                            marker->current_density_real.number
                                            - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                      :
                                        0.0)
        }
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            // TODO: add velocity
            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                        0.5 / marker->conductivity.number * (
                                            sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i])
                                            + sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]))
                                      :
                                        0.0)
        }
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        {
            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                        1.0 / marker->conductivity.number * sqr(
                                            marker->current_density_real.number
                                            - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                            - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                      :
                                        0.0)
        }
    }
    else
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState)
        {
            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                        2 * M_PI * x[i] * 1.0 / marker->conductivity.number * sqr(
                                            marker->current_density_real.number
                                            - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                      :
                                        0.0)
        }
        // TODO: add velocity
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                        2 * M_PI * x[i] * 0.5 / marker->conductivity.number * (
                                            sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i])
                                            + sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]))
                                      :
                                        0.0)
        }
        // TODO: add velocity
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        {
            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                        2 * M_PI * x[i] * 1.0 / marker->conductivity.number * sqr(
                                            marker->current_density_real.number
                                            - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                            - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                      :
                                        0.0)
        }
    }
    powerLosses += result;
}

void VolumeIntegralValueMagnetic::calculateVariablesOther2(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    // energy
    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression(0.25 * (sqr(dudx1[i]) + sqr(dudy1[i]) + sqr(dudx2[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0))
        }
        else
        {
            h1_integrate_expression(0.5 * (sqr(dudx1[i]) + sqr(dudy1[i])) / (marker->permeability.number * MU0))
        }
    }
    else
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression((2 * M_PI * x[i] * 0.25 * sqr(sqrt(sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0))
                                    + (2 * M_PI * x[i] * 0.25 * sqr(sqrt(sqr(dudy2[i]) + sqr(dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0)))
        }
        else
        {
            h1_integrate_expression(2 * M_PI * x[i] * 0.5 * sqr(sqrt(sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0))
        }
    }
    energy += result;
}

void VolumeIntegralValueMagnetic::calculateVariablesOther3(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    // Lorentz force X
    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression(- 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                             + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i]))
                                    +
                                    dudx1[i] * (marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
        }
        else
        {
            h1_integrate_expression(dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))

        }
    }
    else
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression(- 0.5 * (- (2 * M_PI * x[i] * (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))
                                             + (2 * M_PI * x[i] * (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * (dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0))))
                                    +
                                    dudx1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
        }
        else
        {
            h1_integrate_expression(dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))

        }
    }
    forceLorentzX += result;
}

void VolumeIntegralValueMagnetic::calculateVariablesOther4(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    // Lorentz force Y
    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression(- 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                             + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                    +
                                    dudy1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
        }
        else
        {
            h1_integrate_expression(dudy1[i] * (- marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))

        }
    }
    else
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression(- 2 * M_PI * x[i] * 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                               + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                    +
                                    2 * M_PI * x[i] * dudy1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                   (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
        }
        else
        {
            h1_integrate_expression(2 * M_PI * x[i] * dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))

        }
    }
    forceLorentzY += result;
}

void VolumeIntegralValueMagnetic::calculateVariablesOther5(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    // torque
    result = 0.0;
    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            h1_integrate_expression(y[i] * (
                                        - ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                        + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i])
                                        +
                                        dudx1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                    -
                                    x[i] * (
                                        - ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                        + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i])
                                        +
                                        dudy1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                    )
        }
        else
        {
            h1_integrate_expression(y[i] *
                                    dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                    -
                                    x[i] *
                                    dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))

        }
    }
    else
    {
        h1_integrate_expression(0.0);
    }
    torque += result;
}

void VolumeIntegralValueMagnetic::initSolutions()
{
    sln1 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution());
    sln2 = NULL;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() - 1);
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        sln2 = Util::scene()->sceneSolution()->sln(Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1);
}

QStringList VolumeIntegralValueMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(currentReal, 0, 'e', 5) <<
            QString("%1").arg(currentImag, 0, 'e', 5) <<
            QString("%1").arg(currentInducedTransformReal, 0, 'e', 5) <<
            QString("%1").arg(currentInducedTransformImag, 0, 'e', 5) <<
            QString("%1").arg(currentInducedVelocityReal, 0, 'e', 5) <<
            QString("%1").arg(currentInducedVelocityImag, 0, 'e', 5) <<
            QString("%1").arg(currentTotalReal, 0, 'e', 5) <<
            QString("%1").arg(currentTotalImag, 0, 'e', 5) <<
            QString("%1").arg(forceLorentzX, 0, 'e', 5) <<
            QString("%1").arg(forceLorentzY, 0, 'e', 5) <<
            QString("%1").arg(torque, 0, 'e', 5) <<
            QString("%1").arg(powerLosses, 0, 'e', 5) <<
            QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// *************************************************************************************************************************************

void ViewScalarFilterMagnetic::calculateVariable(int i)
{
    SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);

    switch (m_physicFieldVariable)
    {
    case PhysicFieldVariable_Magnetic_VectorPotential:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i])) * x[i];
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value1[i];
        }
        else
        {
            node->values[0][0][i] = - value1[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = value2[i];
        }
        else
        {
            node->values[0][0][i] = - value2[i] * x[i];
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_FluxDensity:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudx2[i]) +
                                         sqr(dudy1[i]) + sqr(dudy2[i]));
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudy2[i]) +
                                         sqr(dudx1[i] + ((x[i] > EPS_ZERO) ? value1[i] / x[i] : 0.0)) +
                                         sqr(dudx2[i] + ((x[i] > EPS_ZERO) ? value2[i] / x[i] : 0.0)));
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy1[i];
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - dudx1[i];
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudx1[i]));
            }
                break;
            }
        }
        else
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy1[i];
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - dudx1[i] - ((x[i] > EPS_ZERO) ? value1[i] / x[i] : 0.0);
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy1[i]) +
                                             sqr(dudx1[i] + ((x[i] > EPS_ZERO) ? value1[i] / x[i] : 0.0)));
            }
                break;
            }
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy2[i];
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - dudx2[i];
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy2[i]) + sqr(dudx2[i]));
            }
                break;
            }
        }
        else
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy2[i];
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - dudx2[i] - ((x[i] > EPS_ZERO) ? value2[i] / x[i] : 0.0);
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy2[i]) +
                                             sqr(dudx2[i] + ((x[i] > EPS_ZERO) ? value2[i] / x[i] : 0.0)));
            }
                break;
            }
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_MagneticField:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudx2[i]) + sqr(dudy1[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0);
        }
        else
        {
            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudy2[i]) +
                                         sqr(dudx1[i] + ((x[i] > EPS_ZERO) ? value1[i] / x[i] : 0.0)) +
                                         sqr(dudx2[i] + ((x[i] > EPS_ZERO) ? value2[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy1[i] / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - dudx1[i] / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudx1[i])) / (marker->permeability.number * MU0);
            }
                break;
            }
        }
        else
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy1[i] / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - (dudx1[i] - ((x[i] > EPS_ZERO) ? value1[i] / x[i] : 0.0)) / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > EPS_ZERO) ? value1[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
            }
                break;
            }
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy2[i] / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - dudx2[i] / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy2[i]) + sqr(dudx2[i])) / (marker->permeability.number * MU0);
            }
                break;
            }
        }
        else
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = dudy2[i] / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = - (dudx2[i] - ((x[i] > EPS_ZERO) ? value2[i] / x[i] : 0.0)) / (marker->permeability.number * MU0);
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(dudy2[i]) + sqr(dudx2[i] + ((x[i] > EPS_ZERO) ? value2[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
            }
                break;
            }
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensity:
    {
        node->values[0][0][i] = sqrt(
                    sqr(marker->current_density_real.number) +
                    sqr(marker->current_density_imag.number));
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
    {
        SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);
        node->values[0][0][i] = marker->current_density_real.number;
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
    {
        SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);
        node->values[0][0][i] = marker->current_density_imag.number;
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
    {
        SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            node->values[0][0][i] = 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];
        }
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        {
            node->values[0][0][i] = - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number;
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
    {
        node->values[0][0][i] = - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i];
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
    {
        node->values[0][0][i] = sqrt(
                    sqr(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                    sqr(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]));
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
    {
        node->values[0][0][i] = - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
    {
        node->values[0][0][i] = - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]);
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
    {
        node->values[0][0][i] = - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * sqrt(sqr(dudx1[i]) + sqr(dudx2[i])) +
                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * sqrt(sqr(dudy1[i]) + sqr(dudy2[i])));
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
    {
        SceneMaterialMagnetic *marker = dynamic_cast<SceneMaterialMagnetic *>(material);
        node->values[0][0][i] = marker->current_density_real.number -
                marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            node->values[0][0][i] += 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];

        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            node->values[0][0][i] -= marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number;
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
    {
        node->values[0][0][i] = marker->current_density_imag.number +
                marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
            node->values[0][0][i] += 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i];
    }
        break;
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
    {
        node->values[0][0][i] = sqrt(
                    sqr(marker->current_density_real.number +
                        2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i] +
                        marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                    +
                    sqr(marker->current_density_imag.number +
                        2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i] +
                        marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]))

                    );
    }
        break;
    case PhysicFieldVariable_Magnetic_PowerLosses:
    {
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_SteadyState)
        {
            node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                        1.0 / marker->conductivity.number * sqr(
                            marker->current_density_real.number +
                            - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                      :
                        0.0;
        }
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        {
            // TODO: add velocity
            node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                        0.5 / marker->conductivity.number * (
                            sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                            sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]))
                      :
                        0.0;
        }
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        {
            node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                        1.0 / marker->conductivity.number * sqr(
                            marker->current_density_real.number +
                            - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                            - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number)
                      :
                        0.0;
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_LorentzForce:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    node->values[0][0][i] = - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                                     +          ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i]))
                            +
                            dudx1[i] * (marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]));
                }
                else
                {
                    node->values[0][0][i] = (dudx1[i] * (
                                                 marker->current_density_real.number +
                                                 - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                                 - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                                 ));
                }
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    node->values[0][0][i] = - (0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                      +           ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                               +
                                               dudy1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                            (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));
                }
                else
                {
                    node->values[0][0][i] = (dudy1[i] * (
                                                 marker->current_density_real.number +
                                                 - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                                 - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                                 ));
                }
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    node->values[0][0][i] = sqrt(sqr(
                                                     0.5 * ( - ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                                             + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i]))
                                                     +
                                                     dudx1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                     ) + sqr(
                                                     node->values[0][0][i] = - (0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                                                       +           ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                                                                +
                                                                                dudy1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                            ));
                }
                else
                {
                    node->values[0][0][i] = sqrt(
                                sqr(dudx1[i] * (
                                        marker->current_density_real.number +
                                        - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                        + ((Util::scene()->problemInfo()->analysisType == AnalysisType_Transient) ?
                                               - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number : 0.0)
                                        ))
                                + sqr(dudy1[i] * (
                                          marker->current_density_real.number +
                                          - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                          + ((Util::scene()->problemInfo()->analysisType == AnalysisType_Transient) ?
                                                 - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number : 0.0)
                                          )));
                }

            }
                break;
            }
        }
        else
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    node->values[0][0][i] = - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))
                                                     + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * (dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0))))
                            +
                            (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)) * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]));
                }
                else
                {
                    node->values[0][0][i] = ((dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)) * (
                                                 marker->current_density_real.number +
                                                 - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                                 - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                                 ));
                }
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    node->values[0][0][i] = - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                     + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                            +
                            - dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]));
                }
                else
                {
                    node->values[0][0][i] = (dudy1[i] * (
                                                 marker->current_density_real.number +
                                                 - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                                 - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                                 ));

                }
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                {
                    node->values[0][0][i] = sqrt(sqr(
                                                     - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))
                                                              + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * (dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0))))
                                                     +
                                                     (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)) * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                                                (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                     ) + sqr(
                                                     - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                              + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                                     +
                                                     - dudy1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                        (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                     ));
                }
                else
                {
                    node->values[0][0][i] = sqrt(
                                sqr((dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)) * (
                                        marker->current_density_real.number +
                                        - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                        - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                        ))
                                + sqr(dudy1[i] * (
                                          marker->current_density_real.number +
                                          - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])
                                          - marker->conductivity.number * (value1[i] - value2[i]) / Util::scene()->problemInfo()->timeStep.number
                                          )));
                }
            }
                break;
            }
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_EnergyDensity:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            node->values[0][0][i] = 0.25 * (sqr(dudx1[i]) + sqr(dudy1[i])) / (marker->permeability.number * MU0);
            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                node->values[0][0][i] += 0.25 * (sqr(dudx2[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0);
        }
        else
        {
            node->values[0][0][i] = 0.25 * (sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > EPS_ZERO) ? value1[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
            if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
                node->values[0][0][i] += 0.25 * (sqr(dudy2[i]) + sqr(dudx2[i] + ((x[i] > EPS_ZERO) ? value2[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_Permeability:
    {
        node->values[0][0][i] = marker->permeability.number;
    }
        break;
    case PhysicFieldVariable_Magnetic_Conductivity:
    {
        node->values[0][0][i] = marker->conductivity.number;
    }
        break;
    case PhysicFieldVariable_Magnetic_Velocity:
    {
        if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = marker->velocity_x.number - marker->velocity_angular.number * y[i];
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = marker->velocity_y.number + marker->velocity_angular.number * x[i];
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = sqrt(sqr(marker->velocity_x.number - marker->velocity_angular.number * y[i]) +
                                             sqr(marker->velocity_y.number + marker->velocity_angular.number * x[i]));
            }
                break;
            }
        }
        else
        {
            switch (m_physicFieldVariableComp)
            {
            case PhysicFieldVariableComp_X:
            {
                node->values[0][0][i] = 0;
            }
                break;
            case PhysicFieldVariableComp_Y:
            {
                node->values[0][0][i] = marker->velocity_y.number;
            }
                break;
            case PhysicFieldVariableComp_Magnitude:
            {
                node->values[0][0][i] = fabs(marker->velocity_y.number);
            }
                break;
            }
        }
    }
        break;
    case PhysicFieldVariable_Magnetic_Remanence:
    {
        node->values[0][0][i] = marker->remanence.number;

        switch (m_physicFieldVariableComp)
        {
        case PhysicFieldVariableComp_X:
        {
            node->values[0][0][i] = marker->remanence.number * cos(marker->remanence_angle.number / 180.0 * M_PI);
        }
            break;
        case PhysicFieldVariableComp_Y:
        {
            node->values[0][0][i] = marker->remanence.number * sin(marker->remanence_angle.number / 180.0 * M_PI);
        }
            break;
        case PhysicFieldVariableComp_Magnitude:
        {
            node->values[0][0][i] = marker->remanence.number;
        }
            break;
        }
    }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilterMagnetic::calculateVariable()" << endl;
        throw;
        break;
    }
}
