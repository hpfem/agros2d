#include "electrostatic_interface.h"

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/localpoint.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"

#include "hermes2d/marker.h"
#include "hermes2d/localpoint.h"
#include "hermes2d/weakform_parser.h"

Hermes::Hermes2D::MatrixFormVol<double> *ElectrostaticInterface::matrixFormVol(const ProblemID problemId, int i, int j,
                                                                        const std::string &area, Hermes::Hermes2D::SymFlag sym,
                                                                        SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ )
{
    qDebug() << "Hermes::Hermes2D::MatrixFormVol<double> *ElectrostaticInterface::matrixFormVol";
    assert(0);
    /*
    if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_linear_<double>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

    if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_newton_<double>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

    if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_linear_<double>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

    if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_newton_<double>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);
    */
    return NULL;
}

Hermes::Hermes2D::VectorFormVol<double> *ElectrostaticInterface::vectorFormVol(const ProblemID problemId, int i, int j,
                                                                        const std::string &area, SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ)
{
    qDebug() << "Hermes::Hermes2D::VectorFormVol<double> *ElectrostaticInterface::vectorFormVol";
    assert(0);
    /*
    if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateplanar::CustomVectorFormVol__1_1_steadystate_linear_<double>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

    if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateplanar::CustomVectorFormVol__1_1_steadystate_newton_<double>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

    if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateaxi::CustomVectorFormVol__1_1_steadystate_linear_<double>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

    if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
        return new electrostaticsteadystateaxi::CustomVectorFormVol__1_1_steadystate_newton_<double>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);
    */

    return NULL;
}

Hermes::Hermes2D::MatrixFormSurf<double> *ElectrostaticInterface::matrixFormSurf(const ProblemID problemId, int i, int j,
                                                                          const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ)
{
    qDebug() << "Hermes::Hermes2D::MatrixFormSurf<double> *ElectrostaticInterface::matrixFormSurf";
    assert(0);

    return NULL;
}

Hermes::Hermes2D::VectorFormSurf<double> *ElectrostaticInterface::vectorFormSurf(const ProblemID problemId, int i, int j,
                                                                          const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ)
{
    qDebug() << "Hermes::Hermes2D::VectorFormSurf<double> *ElectrostaticInterface::vectorFormSurf";
    assert(0);

    /*
    if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
        return new electrostaticsteadystateplanar::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<double>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

    if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
        return new electrostaticsteadystateplanar::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<double>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

    if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
        return new electrostaticsteadystateaxi::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<double>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

    if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
        return new electrostaticsteadystateaxi::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<double>(i-1+ offsetI, j-1+ offsetJ, area, boundary);
    */

    return NULL;
}

Hermes::Hermes2D::ExactSolutionScalar<double> *ElectrostaticInterface::exactSolution(const ProblemID problemId, int i,Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
{
    qDebug() << "Hermes::Hermes2D::ExactSolutionScalar<double> *ElectrostaticInterface::exactSolution";
    assert(0);
    /*
    if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1))
        return new electrostaticsteadystateplanar::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<double>(mesh, boundary);

    if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1))
        return new electrostaticsteadystateplanar::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<double>(mesh, boundary);

    if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1))
        return new electrostaticsteadystateaxi::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<double>(mesh, boundary);

    if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1))
        return new electrostaticsteadystateaxi::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<double>(mesh, boundary);
    */

    return NULL;
}

Q_EXPORT_PLUGIN2(electrostatic, ElectrostaticInterface)
