#include "weakform_factory.h"

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
#include "flow_transient_axi.h"
#include "flow_steadystate_planar.h"
#include "flow_transient_planar.h"
#include "flow_steadystate_axi.h"
#include "acoustic_transient_axi.h"
#include "acoustic_harmonic_axi.h"
#include "acoustic_harmonic_planar.h"
#include "acoustic_transient_planar.h"
#include "current_harmonic_axi.h"
#include "current_harmonic_planar.h"
#include "current_steadystate_planar.h"
#include "current_steadystate_axi.h"
#include "elasticity_steadystate_axi.h"
#include "elasticity_steadystate_planar.h"
#include "electrostatic_steadystate_planar.h"
#include "electrostatic_steadystate_axi.h"
#include "heat_transient_axi.h"
#include "heat_transient_planar.h"
#include "heat_steadystate_axi.h"
#include "heat_steadystate_planar.h"
#include "magnetic_harmonic_planar.h"
#include "magnetic_steadystate_planar.h"
#include "magnetic_steadystate_axi.h"
#include "magnetic_transient_axi.h"
#include "magnetic_transient_planar.h"
#include "magnetic_harmonic_axi.h"
#include "rf_harmonic_axi.h"
#include "rf_harmonic_planar.h"
#include "current_heat_steadystate_transient_planar.h"
#include "current_heat_steadystate_steadystate_axi.h"
#include "current_heat_steadystate_transient_axi.h"
#include "current_heat_steadystate_steadystate_planar.h"
#include "heat_elasticity_steadystate_steadystate_planar.h"
#include "heat_elasticity_transient_steadystate_planar.h"
#include "heat_elasticity_transient_steadystate_axi.h"
#include "heat_elasticity_steadystate_steadystate_axi.h"
#include "magnetic_heat_harmonic_transient_planar.h"
#include "magnetic_heat_steadystate_transient_planar.h"
#include "magnetic_heat_steadystate_steadystate_axi.h"
#include "magnetic_heat_steadystate_steadystate_planar.h"
#include "magnetic_heat_steadystate_transient_axi.h"
#include "magnetic_heat_harmonic_transient_axi.h"
#include "magnetic_heat_harmonic_steadystate_axi.h"
#include "magnetic_heat_harmonic_steadystate_planar.h"
#include "flow_heat_transient_transient_planar.h"
#include "flow_heat_transient_transient_axi.h"

template <typename Scalar>
Hermes::Hermes2D::MatrixFormVol<Scalar> *WeakFormFactory<Scalar>::MatrixFormVol(const std::string &problemId, int i, int j,
                                                                  const std::string &area, Hermes::Hermes2D::SymFlag sym,
                                                                  SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ )
    {

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new flowsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new flowsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1)&&(j == 2))
            return new flowsteadystateplanar::CustomMatrixFormVol__1_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1)&&(j == 2))
            return new flowsteadystateplanar::CustomMatrixFormVol__1_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1)&&(j == 3))
            return new flowsteadystateplanar::CustomMatrixFormVol__1_3_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1)&&(j == 3))
            return new flowsteadystateplanar::CustomMatrixFormVol__1_3_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2)&&(j == 1))
            return new flowsteadystateplanar::CustomMatrixFormVol__2_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2)&&(j == 1))
            return new flowsteadystateplanar::CustomMatrixFormVol__2_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2)&&(j == 2))
            return new flowsteadystateplanar::CustomMatrixFormVol__2_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2)&&(j == 2))
            return new flowsteadystateplanar::CustomMatrixFormVol__2_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2)&&(j == 3))
            return new flowsteadystateplanar::CustomMatrixFormVol__2_3_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2)&&(j == 3))
            return new flowsteadystateplanar::CustomMatrixFormVol__2_3_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 3)&&(j == 1))
            return new flowsteadystateplanar::CustomMatrixFormVol__3_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 3)&&(j == 1))
            return new flowsteadystateplanar::CustomMatrixFormVol__3_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 3)&&(j == 2))
            return new flowsteadystateplanar::CustomMatrixFormVol__3_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 3)&&(j == 2))
            return new flowsteadystateplanar::CustomMatrixFormVol__3_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 3)&&(j == 3))
            return new flowsteadystateplanar::CustomMatrixFormVol__3_3_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 3)&&(j == 3))
            return new flowsteadystateplanar::CustomMatrixFormVol__3_3_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new flowsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new flowsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new flowsteadystateaxi::CustomMatrixFormVol__1_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new flowsteadystateaxi::CustomMatrixFormVol__1_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 3))
            return new flowsteadystateaxi::CustomMatrixFormVol__1_3_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 3))
            return new flowsteadystateaxi::CustomMatrixFormVol__1_3_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new flowsteadystateaxi::CustomMatrixFormVol__2_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new flowsteadystateaxi::CustomMatrixFormVol__2_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new flowsteadystateaxi::CustomMatrixFormVol__2_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new flowsteadystateaxi::CustomMatrixFormVol__2_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 3))
            return new flowsteadystateaxi::CustomMatrixFormVol__2_3_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 3))
            return new flowsteadystateaxi::CustomMatrixFormVol__2_3_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 3)&&(j == 1))
            return new flowsteadystateaxi::CustomMatrixFormVol__3_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 3)&&(j == 1))
            return new flowsteadystateaxi::CustomMatrixFormVol__3_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 3)&&(j == 2))
            return new flowsteadystateaxi::CustomMatrixFormVol__3_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 3)&&(j == 2))
            return new flowsteadystateaxi::CustomMatrixFormVol__3_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 3)&&(j == 3))
            return new flowsteadystateaxi::CustomMatrixFormVol__3_3_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 3)&&(j == 3))
            return new flowsteadystateaxi::CustomMatrixFormVol__3_3_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new flowtransientplanar::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new flowtransientplanar::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1)&&(j == 2))
            return new flowtransientplanar::CustomMatrixFormVol__1_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1)&&(j == 2))
            return new flowtransientplanar::CustomMatrixFormVol__1_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1)&&(j == 3))
            return new flowtransientplanar::CustomMatrixFormVol__1_3_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1)&&(j == 3))
            return new flowtransientplanar::CustomMatrixFormVol__1_3_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2)&&(j == 1))
            return new flowtransientplanar::CustomMatrixFormVol__2_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2)&&(j == 1))
            return new flowtransientplanar::CustomMatrixFormVol__2_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2)&&(j == 2))
            return new flowtransientplanar::CustomMatrixFormVol__2_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2)&&(j == 2))
            return new flowtransientplanar::CustomMatrixFormVol__2_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2)&&(j == 3))
            return new flowtransientplanar::CustomMatrixFormVol__2_3_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2)&&(j == 3))
            return new flowtransientplanar::CustomMatrixFormVol__2_3_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 3)&&(j == 1))
            return new flowtransientplanar::CustomMatrixFormVol__3_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 3)&&(j == 1))
            return new flowtransientplanar::CustomMatrixFormVol__3_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 3)&&(j == 2))
            return new flowtransientplanar::CustomMatrixFormVol__3_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 3)&&(j == 2))
            return new flowtransientplanar::CustomMatrixFormVol__3_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 3)&&(j == 3))
            return new flowtransientplanar::CustomMatrixFormVol__3_3_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 3)&&(j == 3))
            return new flowtransientplanar::CustomMatrixFormVol__3_3_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new flowtransientaxi::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new flowtransientaxi::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new flowtransientaxi::CustomMatrixFormVol__1_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new flowtransientaxi::CustomMatrixFormVol__1_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1)&&(j == 3))
            return new flowtransientaxi::CustomMatrixFormVol__1_3_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1)&&(j == 3))
            return new flowtransientaxi::CustomMatrixFormVol__1_3_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new flowtransientaxi::CustomMatrixFormVol__2_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new flowtransientaxi::CustomMatrixFormVol__2_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new flowtransientaxi::CustomMatrixFormVol__2_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new flowtransientaxi::CustomMatrixFormVol__2_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2)&&(j == 3))
            return new flowtransientaxi::CustomMatrixFormVol__2_3_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2)&&(j == 3))
            return new flowtransientaxi::CustomMatrixFormVol__2_3_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 3)&&(j == 1))
            return new flowtransientaxi::CustomMatrixFormVol__3_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 3)&&(j == 1))
            return new flowtransientaxi::CustomMatrixFormVol__3_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 3)&&(j == 2))
            return new flowtransientaxi::CustomMatrixFormVol__3_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 3)&&(j == 2))
            return new flowtransientaxi::CustomMatrixFormVol__3_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 3)&&(j == 3))
            return new flowtransientaxi::CustomMatrixFormVol__3_3_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 3)&&(j == 3))
            return new flowtransientaxi::CustomMatrixFormVol__3_3_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new acousticharmonicplanar::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new acousticharmonicplanar::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new acousticharmonicplanar::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new acousticharmonicplanar::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new acousticharmonicaxi::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new acousticharmonicaxi::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new acousticharmonicaxi::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new acousticharmonicaxi::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new acoustictransientplanar::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new acoustictransientplanar::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 2)&&(j == 2))
            return new acoustictransientplanar::CustomMatrixFormVol__2_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 2)&&(j == 2))
            return new acoustictransientplanar::CustomMatrixFormVol__2_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new acoustictransientaxi::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new acoustictransientaxi::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new acoustictransientaxi::CustomMatrixFormVol__2_2_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new acoustictransientaxi::CustomMatrixFormVol__2_2_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new currentsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new currentsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new currentsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new currentsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new currentharmonicplanar::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new currentharmonicplanar::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new currentharmonicplanar::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new currentharmonicplanar::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new currentharmonicaxi::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new currentharmonicaxi::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new currentharmonicaxi::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new currentharmonicaxi::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__1_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__1_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__2_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__2_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__2_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateplanar::CustomMatrixFormVol__2_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__1_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__1_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__2_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__2_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__2_2_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateaxi::CustomMatrixFormVol__2_2_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new heatsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new heatsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new heatsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new heatsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new heattransientplanar::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new heattransientplanar::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new heattransientaxi::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new heattransientaxi::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateplanar::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateaxi::CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new magnetictransientplanar::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new magnetictransientplanar::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new magnetictransientaxi::CustomMatrixFormVol__1_1_transient_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new magnetictransientaxi::CustomMatrixFormVol__1_1_transient_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new magneticharmonicplanar::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new magneticharmonicplanar::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new magneticharmonicplanar::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new magneticharmonicplanar::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 1)&&(j == 2))
            return new magneticharmonicplanar::CustomMatrixFormVol__1_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 1)&&(j == 2))
            return new magneticharmonicplanar::CustomMatrixFormVol__1_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 2)&&(j == 1))
            return new magneticharmonicplanar::CustomMatrixFormVol__2_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 2)&&(j == 1))
            return new magneticharmonicplanar::CustomMatrixFormVol__2_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new magneticharmonicaxi::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new magneticharmonicaxi::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new magneticharmonicaxi::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new magneticharmonicaxi::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new magneticharmonicaxi::CustomMatrixFormVol__1_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new magneticharmonicaxi::CustomMatrixFormVol__1_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new magneticharmonicaxi::CustomMatrixFormVol__2_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new magneticharmonicaxi::CustomMatrixFormVol__2_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new rfharmonicplanar::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new rfharmonicplanar::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new rfharmonicplanar::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new rfharmonicplanar::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1)&&(j == 2))
            return new rfharmonicplanar::CustomMatrixFormVol__1_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1)&&(j == 2))
            return new rfharmonicplanar::CustomMatrixFormVol__1_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2)&&(j == 1))
            return new rfharmonicplanar::CustomMatrixFormVol__2_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2)&&(j == 1))
            return new rfharmonicplanar::CustomMatrixFormVol__2_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new rfharmonicaxi::CustomMatrixFormVol__1_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new rfharmonicaxi::CustomMatrixFormVol__1_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new rfharmonicaxi::CustomMatrixFormVol__2_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new rfharmonicaxi::CustomMatrixFormVol__2_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new rfharmonicaxi::CustomMatrixFormVol__1_2_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new rfharmonicaxi::CustomMatrixFormVol__1_2_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new rfharmonicaxi::CustomMatrixFormVol__2_1_harmonic_linear_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new rfharmonicaxi::CustomMatrixFormVol__2_1_harmonic_newton_<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateplanar::CustomMatrixFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateplanar::CustomMatrixFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateaxi::CustomMatrixFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateaxi::CustomMatrixFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientplanar::CustomMatrixFormVol__2_1_steadystate_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientplanar::CustomMatrixFormVol__2_1_steadystate_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientaxi::CustomMatrixFormVol__2_1_steadystate_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientaxi::CustomMatrixFormVol__2_1_steadystate_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomMatrixFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomMatrixFormVol__3_1_steadystate_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomMatrixFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomMatrixFormVol__3_1_steadystate_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomMatrixFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomMatrixFormVol__3_1_steadystate_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomMatrixFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomMatrixFormVol__3_1_steadystate_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomMatrixFormVol__2_1_transient_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomMatrixFormVol__3_1_transient_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomMatrixFormVol__2_1_transient_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomMatrixFormVol__3_1_transient_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomMatrixFormVol__2_1_transient_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomMatrixFormVol__3_1_transient_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomMatrixFormVol__2_1_transient_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomMatrixFormVol__3_1_transient_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateplanar::CustomMatrixFormVol__3_1_harmonic_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateplanar::CustomMatrixFormVol__3_2_harmonic_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateplanar::CustomMatrixFormVol__3_1_harmonic_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateplanar::CustomMatrixFormVol__3_2_harmonic_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateaxi::CustomMatrixFormVol__3_1_harmonic_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateaxi::CustomMatrixFormVol__3_2_harmonic_steadystate_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateaxi::CustomMatrixFormVol__3_1_harmonic_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateaxi::CustomMatrixFormVol__3_2_harmonic_steadystate_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientplanar::CustomMatrixFormVol__3_1_harmonic_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientplanar::CustomMatrixFormVol__3_2_harmonic_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientplanar::CustomMatrixFormVol__3_1_harmonic_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientplanar::CustomMatrixFormVol__3_2_harmonic_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientaxi::CustomMatrixFormVol__3_1_harmonic_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientaxi::CustomMatrixFormVol__3_2_harmonic_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientaxi::CustomMatrixFormVol__3_1_harmonic_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientaxi::CustomMatrixFormVol__3_2_harmonic_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_linear_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientplanar::CustomMatrixFormVol__4_1_transient_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_linear_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientplanar::CustomMatrixFormVol__4_2_transient_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_newton_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientplanar::CustomMatrixFormVol__4_1_transient_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_newton_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientplanar::CustomMatrixFormVol__4_2_transient_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_linear_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientaxi::CustomMatrixFormVol__4_1_transient_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_linear_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientaxi::CustomMatrixFormVol__4_2_transient_transient_linear_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_newton_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientaxi::CustomMatrixFormVol__4_1_transient_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_newton_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientaxi::CustomMatrixFormVol__4_2_transient_transient_newton_hard<Scalar>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);

        return NULL;
    }

    template <typename Scalar>
    Hermes::Hermes2D::VectorFormVol<Scalar> *WeakFormFactory<Scalar>::VectorFormVol(const std::string &problemId, int i, int j,
                                                                  const std::string &area, SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ)
    {

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new flowsteadystateplanar::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new flowsteadystateplanar::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1)&&(j == 2))
            return new flowsteadystateplanar::CustomVectorFormVol__1_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1)&&(j == 2))
            return new flowsteadystateplanar::CustomVectorFormVol__1_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1)&&(j == 3))
            return new flowsteadystateplanar::CustomVectorFormVol__1_3_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1)&&(j == 3))
            return new flowsteadystateplanar::CustomVectorFormVol__1_3_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2)&&(j == 1))
            return new flowsteadystateplanar::CustomVectorFormVol__2_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2)&&(j == 1))
            return new flowsteadystateplanar::CustomVectorFormVol__2_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2)&&(j == 2))
            return new flowsteadystateplanar::CustomVectorFormVol__2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2)&&(j == 2))
            return new flowsteadystateplanar::CustomVectorFormVol__2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2)&&(j == 3))
            return new flowsteadystateplanar::CustomVectorFormVol__2_3_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2)&&(j == 3))
            return new flowsteadystateplanar::CustomVectorFormVol__2_3_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 3)&&(j == 1))
            return new flowsteadystateplanar::CustomVectorFormVol__3_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 3)&&(j == 1))
            return new flowsteadystateplanar::CustomVectorFormVol__3_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 3)&&(j == 2))
            return new flowsteadystateplanar::CustomVectorFormVol__3_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 3)&&(j == 2))
            return new flowsteadystateplanar::CustomVectorFormVol__3_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 3)&&(j == 3))
            return new flowsteadystateplanar::CustomVectorFormVol__3_3_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 3)&&(j == 3))
            return new flowsteadystateplanar::CustomVectorFormVol__3_3_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new flowsteadystateaxi::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new flowsteadystateaxi::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new flowsteadystateaxi::CustomVectorFormVol__1_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new flowsteadystateaxi::CustomVectorFormVol__1_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 3))
            return new flowsteadystateaxi::CustomVectorFormVol__1_3_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 3))
            return new flowsteadystateaxi::CustomVectorFormVol__1_3_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new flowsteadystateaxi::CustomVectorFormVol__2_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new flowsteadystateaxi::CustomVectorFormVol__2_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new flowsteadystateaxi::CustomVectorFormVol__2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new flowsteadystateaxi::CustomVectorFormVol__2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 3))
            return new flowsteadystateaxi::CustomVectorFormVol__2_3_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 3))
            return new flowsteadystateaxi::CustomVectorFormVol__2_3_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 3)&&(j == 1))
            return new flowsteadystateaxi::CustomVectorFormVol__3_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 3)&&(j == 1))
            return new flowsteadystateaxi::CustomVectorFormVol__3_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 3)&&(j == 2))
            return new flowsteadystateaxi::CustomVectorFormVol__3_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 3)&&(j == 2))
            return new flowsteadystateaxi::CustomVectorFormVol__3_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 3)&&(j == 3))
            return new flowsteadystateaxi::CustomVectorFormVol__3_3_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 3)&&(j == 3))
            return new flowsteadystateaxi::CustomVectorFormVol__3_3_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new flowtransientplanar::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new flowtransientplanar::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1)&&(j == 2))
            return new flowtransientplanar::CustomVectorFormVol__1_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1)&&(j == 2))
            return new flowtransientplanar::CustomVectorFormVol__1_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1)&&(j == 3))
            return new flowtransientplanar::CustomVectorFormVol__1_3_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1)&&(j == 3))
            return new flowtransientplanar::CustomVectorFormVol__1_3_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2)&&(j == 1))
            return new flowtransientplanar::CustomVectorFormVol__2_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2)&&(j == 1))
            return new flowtransientplanar::CustomVectorFormVol__2_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2)&&(j == 2))
            return new flowtransientplanar::CustomVectorFormVol__2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2)&&(j == 2))
            return new flowtransientplanar::CustomVectorFormVol__2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2)&&(j == 3))
            return new flowtransientplanar::CustomVectorFormVol__2_3_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2)&&(j == 3))
            return new flowtransientplanar::CustomVectorFormVol__2_3_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 3)&&(j == 1))
            return new flowtransientplanar::CustomVectorFormVol__3_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 3)&&(j == 1))
            return new flowtransientplanar::CustomVectorFormVol__3_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 3)&&(j == 2))
            return new flowtransientplanar::CustomVectorFormVol__3_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 3)&&(j == 2))
            return new flowtransientplanar::CustomVectorFormVol__3_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 3)&&(j == 3))
            return new flowtransientplanar::CustomVectorFormVol__3_3_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 3)&&(j == 3))
            return new flowtransientplanar::CustomVectorFormVol__3_3_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new flowtransientaxi::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new flowtransientaxi::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new flowtransientaxi::CustomVectorFormVol__1_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new flowtransientaxi::CustomVectorFormVol__1_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1)&&(j == 3))
            return new flowtransientaxi::CustomVectorFormVol__1_3_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1)&&(j == 3))
            return new flowtransientaxi::CustomVectorFormVol__1_3_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new flowtransientaxi::CustomVectorFormVol__2_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new flowtransientaxi::CustomVectorFormVol__2_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new flowtransientaxi::CustomVectorFormVol__2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new flowtransientaxi::CustomVectorFormVol__2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2)&&(j == 3))
            return new flowtransientaxi::CustomVectorFormVol__2_3_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2)&&(j == 3))
            return new flowtransientaxi::CustomVectorFormVol__2_3_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 3)&&(j == 1))
            return new flowtransientaxi::CustomVectorFormVol__3_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 3)&&(j == 1))
            return new flowtransientaxi::CustomVectorFormVol__3_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 3)&&(j == 2))
            return new flowtransientaxi::CustomVectorFormVol__3_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 3)&&(j == 2))
            return new flowtransientaxi::CustomVectorFormVol__3_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 3)&&(j == 3))
            return new flowtransientaxi::CustomVectorFormVol__3_3_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 3)&&(j == 3))
            return new flowtransientaxi::CustomVectorFormVol__3_3_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new acousticharmonicplanar::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new acousticharmonicplanar::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new acousticharmonicplanar::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new acousticharmonicplanar::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new acousticharmonicaxi::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new acousticharmonicaxi::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new acousticharmonicaxi::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new acousticharmonicaxi::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new acoustictransientplanar::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new acoustictransientplanar::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 2)&&(j == 2))
            return new acoustictransientplanar::CustomVectorFormVol__2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 2)&&(j == 2))
            return new acoustictransientplanar::CustomVectorFormVol__2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new acoustictransientaxi::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new acoustictransientaxi::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new acoustictransientaxi::CustomVectorFormVol__2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new acoustictransientaxi::CustomVectorFormVol__2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new currentsteadystateplanar::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new currentsteadystateplanar::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new currentsteadystateaxi::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new currentsteadystateaxi::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new currentharmonicplanar::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new currentharmonicplanar::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new currentharmonicplanar::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new currentharmonicplanar::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new currentharmonicaxi::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new currentharmonicaxi::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new currentharmonicaxi::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new currentharmonicaxi::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateplanar::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateplanar::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateplanar::CustomVectorFormVol__1_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateplanar::CustomVectorFormVol__1_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateplanar::CustomVectorFormVol__2_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateplanar::CustomVectorFormVol__2_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateplanar::CustomVectorFormVol__2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateplanar::CustomVectorFormVol__2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateaxi::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new elasticitysteadystateaxi::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateaxi::CustomVectorFormVol__1_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new elasticitysteadystateaxi::CustomVectorFormVol__1_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateaxi::CustomVectorFormVol__2_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new elasticitysteadystateaxi::CustomVectorFormVol__2_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateaxi::CustomVectorFormVol__2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new elasticitysteadystateaxi::CustomVectorFormVol__2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateplanar::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateplanar::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateaxi::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new electrostaticsteadystateaxi::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new heatsteadystateplanar::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new heatsteadystateplanar::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new heatsteadystateaxi::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new heatsteadystateaxi::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new heattransientplanar::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new heattransientplanar::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new heattransientaxi::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new heattransientaxi::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_planar_linear_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateplanar::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_planar_newton_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateplanar::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateaxi::CustomVectorFormVol__1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new magneticsteadystateaxi::CustomVectorFormVol__1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_planar_linear_")&&(i == 1)&&(j == 1))
            return new magnetictransientplanar::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_planar_newton_")&&(i == 1)&&(j == 1))
            return new magnetictransientplanar::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new magnetictransientaxi::CustomVectorFormVol__1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new magnetictransientaxi::CustomVectorFormVol__1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new magneticharmonicplanar::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new magneticharmonicplanar::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new magneticharmonicplanar::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new magneticharmonicplanar::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 1)&&(j == 2))
            return new magneticharmonicplanar::CustomVectorFormVol__1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 1)&&(j == 2))
            return new magneticharmonicplanar::CustomVectorFormVol__1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 2)&&(j == 1))
            return new magneticharmonicplanar::CustomVectorFormVol__2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 2)&&(j == 1))
            return new magneticharmonicplanar::CustomVectorFormVol__2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new magneticharmonicaxi::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new magneticharmonicaxi::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new magneticharmonicaxi::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new magneticharmonicaxi::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 2))
            return new magneticharmonicaxi::CustomVectorFormVol__1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 2))
            return new magneticharmonicaxi::CustomVectorFormVol__1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 1))
            return new magneticharmonicaxi::CustomVectorFormVol__2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 1))
            return new magneticharmonicaxi::CustomVectorFormVol__2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1)&&(j == 1))
            return new rfharmonicplanar::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1)&&(j == 1))
            return new rfharmonicplanar::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2)&&(j == 2))
            return new rfharmonicplanar::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2)&&(j == 2))
            return new rfharmonicplanar::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1))
            return new rfharmonicaxi::CustomVectorFormVol__1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1))
            return new rfharmonicaxi::CustomVectorFormVol__1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2))
            return new rfharmonicaxi::CustomVectorFormVol__2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2))
            return new rfharmonicaxi::CustomVectorFormVol__2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatesteadystateplanar::CustomVectorFormVol__1_1_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatesteadystateplanar::CustomVectorFormVol__1_1_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateplanar::CustomVectorFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateplanar::CustomVectorFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatesteadystateaxi::CustomVectorFormVol__1_1_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatesteadystateaxi::CustomVectorFormVol__1_1_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateaxi::CustomVectorFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_steadystate_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatesteadystateaxi::CustomVectorFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatetransientplanar::CustomVectorFormVol__1_1_steadystate_transient_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatetransientplanar::CustomVectorFormVol__1_1_steadystate_transient_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientplanar::CustomVectorFormVol__2_1_steadystate_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientplanar::CustomVectorFormVol__2_1_steadystate_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatetransientaxi::CustomVectorFormVol__1_1_steadystate_transient_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new currentheatsteadystatetransientaxi::CustomVectorFormVol__1_1_steadystate_transient_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientaxi::CustomVectorFormVol__2_1_steadystate_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "current_heat_steadystate_transient_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new currentheatsteadystatetransientaxi::CustomVectorFormVol__2_1_steadystate_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__1_1_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__1_1_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_linear_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__2_2_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_newton_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__2_2_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__3_1_steadystate_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateplanar::CustomVectorFormVol__3_1_steadystate_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__1_1_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__1_1_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_linear_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__2_2_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_newton_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__2_2_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__2_1_steadystate_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__2_1_steadystate_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__3_1_steadystate_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_steadystate_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitysteadystatesteadystateaxi::CustomVectorFormVol__3_1_steadystate_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__1_1_transient_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__1_1_transient_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_linear_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__2_2_transient_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_newton_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__2_2_transient_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__2_1_transient_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__2_1_transient_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__3_1_transient_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateplanar::CustomVectorFormVol__3_1_transient_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__1_1_transient_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__1_1_transient_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_linear_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__2_2_transient_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_newton_weak")&&(i == 2)&&(j == 2))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__2_2_transient_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_linear_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__2_1_transient_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_newton_hard")&&(i == 2)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__2_1_transient_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__3_1_transient_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "heat_elasticity_transient_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new heatelasticitytransientsteadystateaxi::CustomVectorFormVol__3_1_transient_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonicsteadystateplanar::CustomVectorFormVol__1_1_harmonic_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonicsteadystateplanar::CustomVectorFormVol__1_1_harmonic_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateplanar::CustomVectorFormVol__3_2_harmonic_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateplanar::CustomVectorFormVol__3_2_harmonic_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateplanar::CustomVectorFormVol__3_1_harmonic_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateplanar::CustomVectorFormVol__3_1_harmonic_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonicsteadystateaxi::CustomVectorFormVol__1_1_harmonic_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonicsteadystateaxi::CustomVectorFormVol__1_1_harmonic_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateaxi::CustomVectorFormVol__3_2_harmonic_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonicsteadystateaxi::CustomVectorFormVol__3_2_harmonic_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateaxi::CustomVectorFormVol__3_1_harmonic_steadystate_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_steadystate_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonicsteadystateaxi::CustomVectorFormVol__3_1_harmonic_steadystate_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonictransientplanar::CustomVectorFormVol__1_1_harmonic_transient_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonictransientplanar::CustomVectorFormVol__1_1_harmonic_transient_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientplanar::CustomVectorFormVol__3_2_harmonic_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientplanar::CustomVectorFormVol__3_2_harmonic_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientplanar::CustomVectorFormVol__3_1_harmonic_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_planar_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientplanar::CustomVectorFormVol__3_1_harmonic_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonictransientaxi::CustomVectorFormVol__1_1_harmonic_transient_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatharmonictransientaxi::CustomVectorFormVol__1_1_harmonic_transient_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_linear_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientaxi::CustomVectorFormVol__3_2_harmonic_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_newton_hard")&&(i == 3)&&(j == 2))
            return new magneticheatharmonictransientaxi::CustomVectorFormVol__3_2_harmonic_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_linear_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientaxi::CustomVectorFormVol__3_1_harmonic_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_harmonic_transient_axisymmetric_newton_hard")&&(i == 3)&&(j == 1))
            return new magneticheatharmonictransientaxi::CustomVectorFormVol__3_1_harmonic_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_steadystate_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatesteadystateplanar::CustomVectorFormVol__1_1_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_steadystate_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatesteadystateplanar::CustomVectorFormVol__1_1_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_steadystate_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatesteadystateaxi::CustomVectorFormVol__1_1_steadystate_steadystate_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_steadystate_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatesteadystateaxi::CustomVectorFormVol__1_1_steadystate_steadystate_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_transient_planar_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatetransientplanar::CustomVectorFormVol__1_1_steadystate_transient_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_transient_planar_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatetransientplanar::CustomVectorFormVol__1_1_steadystate_transient_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_transient_axisymmetric_linear_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatetransientaxi::CustomVectorFormVol__1_1_steadystate_transient_linear_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "magnetic_heat_steadystate_transient_axisymmetric_newton_weak")&&(i == 1)&&(j == 1))
            return new magneticheatsteadystatetransientaxi::CustomVectorFormVol__1_1_steadystate_transient_newton_weak<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_linear_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientplanar::CustomVectorFormVol__4_1_transient_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_newton_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientplanar::CustomVectorFormVol__4_1_transient_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_linear_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientplanar::CustomVectorFormVol__4_2_transient_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_planar_newton_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientplanar::CustomVectorFormVol__4_2_transient_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_linear_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientaxi::CustomVectorFormVol__4_1_transient_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_newton_hard")&&(i == 4)&&(j == 1))
            return new flowheattransienttransientaxi::CustomVectorFormVol__4_1_transient_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_linear_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientaxi::CustomVectorFormVol__4_2_transient_transient_linear_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        if ((problemId == "flow_heat_transient_transient_axisymmetric_newton_hard")&&(i == 4)&&(j == 2))
            return new flowheattransienttransientaxi::CustomVectorFormVol__4_2_transient_transient_newton_hard<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);

        return NULL;
    }

    template <typename Scalar>
    Hermes::Hermes2D::MatrixFormSurf<Scalar> *WeakFormFactory<Scalar>::MatrixFormSurf(const std::string &problemId, int i, int j,
                                                                    const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ)
    {

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomMatrixFormSurf_acoustic_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomMatrixFormSurf_acoustic_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomMatrixFormSurf_acoustic_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomMatrixFormSurf_acoustic_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomMatrixFormSurf_acoustic_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomMatrixFormSurf_acoustic_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomMatrixFormSurf_acoustic_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomMatrixFormSurf_acoustic_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateplanar::CustomMatrixFormSurf_heat_heat_flux_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateplanar::CustomMatrixFormSurf_heat_heat_flux_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateaxi::CustomMatrixFormSurf_heat_heat_flux_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateaxi::CustomMatrixFormSurf_heat_heat_flux_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientplanar::CustomMatrixFormSurf_heat_heat_flux_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientplanar::CustomMatrixFormSurf_heat_heat_flux_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientaxi::CustomMatrixFormSurf_heat_heat_flux_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientaxi::CustomMatrixFormSurf_heat_heat_flux_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomMatrixFormSurf_rf_te_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomMatrixFormSurf_rf_te_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomMatrixFormSurf_rf_te_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomMatrixFormSurf_rf_te_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomMatrixFormSurf_rf_te_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomMatrixFormSurf_rf_te_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomMatrixFormSurf_rf_te_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomMatrixFormSurf_rf_te_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        return NULL;
    }

    template <typename Scalar>
    Hermes::Hermes2D::VectorFormSurf<Scalar> *WeakFormFactory<Scalar>::VectorFormSurf(const std::string &problemId, int i, int j,
                                                                    const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ)
    {

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateplanar::CustomVectorFormSurf_fluid_outlet_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateplanar::CustomVectorFormSurf_fluid_outlet_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateplanar::CustomVectorFormSurf_fluid_outlet_2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateplanar::CustomVectorFormSurf_fluid_outlet_2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateaxi::CustomVectorFormSurf_fluid_outlet_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateaxi::CustomVectorFormSurf_fluid_outlet_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateaxi::CustomVectorFormSurf_fluid_outlet_2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowsteadystateaxi::CustomVectorFormSurf_fluid_outlet_2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientplanar::CustomVectorFormSurf_fluid_outlet_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientplanar::CustomVectorFormSurf_fluid_outlet_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientplanar::CustomVectorFormSurf_fluid_outlet_2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientplanar::CustomVectorFormSurf_fluid_outlet_2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientaxi::CustomVectorFormSurf_fluid_outlet_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientaxi::CustomVectorFormSurf_fluid_outlet_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientaxi::CustomVectorFormSurf_fluid_outlet_2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "fluid_outlet"))
            return new flowtransientaxi::CustomVectorFormSurf_fluid_outlet_2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicplanar::CustomVectorFormSurf_acoustic_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "acoustic_impedance"))
            return new acousticharmonicaxi::CustomVectorFormSurf_acoustic_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientplanar::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientplanar::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientplanar::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientplanar::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientaxi::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientaxi::CustomVectorFormSurf_acoustic_normal_acceleration_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientaxi::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "acoustic_normal_acceleration"))
            return new acoustictransientaxi::CustomVectorFormSurf_acoustic_normal_acceleration_2_2_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentsteadystateplanar::CustomVectorFormSurf_current_inward_current_flow_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentsteadystateplanar::CustomVectorFormSurf_current_inward_current_flow_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentsteadystateaxi::CustomVectorFormSurf_current_inward_current_flow_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentsteadystateaxi::CustomVectorFormSurf_current_inward_current_flow_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicplanar::CustomVectorFormSurf_current_inward_current_flow_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicplanar::CustomVectorFormSurf_current_inward_current_flow_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicplanar::CustomVectorFormSurf_current_inward_current_flow_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicplanar::CustomVectorFormSurf_current_inward_current_flow_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicaxi::CustomVectorFormSurf_current_inward_current_flow_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicaxi::CustomVectorFormSurf_current_inward_current_flow_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicaxi::CustomVectorFormSurf_current_inward_current_flow_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "current_inward_current_flow"))
            return new currentharmonicaxi::CustomVectorFormSurf_current_inward_current_flow_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_fixed_free"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_fixed_free_2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_fixed_free"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_fixed_free_2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_fixed"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_free_fixed_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_fixed"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_free_fixed_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_free_free_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_free_free_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_free_free_2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateplanar::CustomVectorFormSurf_elasticity_free_free_2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_fixed_free"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_fixed_free_2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_fixed_free"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_fixed_free_2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_fixed"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_free_fixed_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_fixed"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_free_fixed_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_free_free_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_free_free_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_free_free_2_2_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "elasticity_free_free"))
            return new elasticitysteadystateaxi::CustomVectorFormSurf_elasticity_free_free_2_2_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
            return new electrostaticsteadystateplanar::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
            return new electrostaticsteadystateplanar::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
            return new electrostaticsteadystateaxi::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "electrostatic_surface_charge_density"))
            return new electrostaticsteadystateaxi::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateplanar::CustomVectorFormSurf_heat_heat_flux_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateplanar::CustomVectorFormSurf_heat_heat_flux_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateaxi::CustomVectorFormSurf_heat_heat_flux_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heatsteadystateaxi::CustomVectorFormSurf_heat_heat_flux_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientplanar::CustomVectorFormSurf_heat_heat_flux_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientplanar::CustomVectorFormSurf_heat_heat_flux_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientaxi::CustomVectorFormSurf_heat_heat_flux_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "heat_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "heat_heat_flux"))
            return new heattransientaxi::CustomVectorFormSurf_heat_heat_flux_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_steadystate_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticsteadystateplanar::CustomVectorFormSurf_magnetic_surface_current_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_steadystate_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticsteadystateplanar::CustomVectorFormSurf_magnetic_surface_current_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_steadystate_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticsteadystateaxi::CustomVectorFormSurf_magnetic_surface_current_1_1_steadystate_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_steadystate_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticsteadystateaxi::CustomVectorFormSurf_magnetic_surface_current_1_1_steadystate_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_transient_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magnetictransientplanar::CustomVectorFormSurf_magnetic_surface_current_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_transient_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magnetictransientplanar::CustomVectorFormSurf_magnetic_surface_current_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_transient_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magnetictransientaxi::CustomVectorFormSurf_magnetic_surface_current_1_1_transient_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_transient_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magnetictransientaxi::CustomVectorFormSurf_magnetic_surface_current_1_1_transient_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicplanar::CustomVectorFormSurf_magnetic_surface_current_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicplanar::CustomVectorFormSurf_magnetic_surface_current_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicplanar::CustomVectorFormSurf_magnetic_surface_current_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicplanar::CustomVectorFormSurf_magnetic_surface_current_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicaxi::CustomVectorFormSurf_magnetic_surface_current_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicaxi::CustomVectorFormSurf_magnetic_surface_current_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicaxi::CustomVectorFormSurf_magnetic_surface_current_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "magnetic_surface_current"))
            return new magneticharmonicaxi::CustomVectorFormSurf_magnetic_surface_current_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_magnetic_field_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_magnetic_field_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_magnetic_field_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_magnetic_field_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_surface_current_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_surface_current_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_surface_current_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_surface_current_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicplanar::CustomVectorFormSurf_rf_te_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_magnetic_field_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_magnetic_field_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_magnetic_field_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_magnetic_field"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_magnetic_field_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_surface_current_1_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 1)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_surface_current_1_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_surface_current_2_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 2)&&(boundary->getType() == "rf_te_surface_current"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_surface_current_2_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_impedance_1_2_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1)&&(j == 2)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_impedance_1_2_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_impedance_2_1_harmonic_linear_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2)&&(j == 1)&&(boundary->getType() == "rf_te_impedance"))
            return new rfharmonicaxi::CustomVectorFormSurf_rf_te_impedance_2_1_harmonic_newton_<Scalar>(i-1+ offsetI, j-1+ offsetJ, area, boundary);

        return NULL;
    }

    template <typename Scalar>
    Hermes::Hermes2D::ExactSolutionScalar<Scalar> *WeakFormFactory<Scalar>::ExactSolution(const std::string &problemId, int i,Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
    {

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 1))
            return new flowsteadystateplanar::CustomEssentialFormSurf_flow_velocity_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 1))
            return new flowsteadystateplanar::CustomEssentialFormSurf_flow_velocity_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 2))
            return new flowsteadystateplanar::CustomEssentialFormSurf_flow_velocity_2_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 2))
            return new flowsteadystateplanar::CustomEssentialFormSurf_flow_velocity_2_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_planar_linear_")&&(i == 3))
            return new flowsteadystateplanar::CustomEssentialFormSurf_fluid_pressure_3_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_planar_newton_")&&(i == 3))
            return new flowsteadystateplanar::CustomEssentialFormSurf_fluid_pressure_3_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 1))
            return new flowsteadystateaxi::CustomEssentialFormSurf_flow_velocity_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 1))
            return new flowsteadystateaxi::CustomEssentialFormSurf_flow_velocity_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 2))
            return new flowsteadystateaxi::CustomEssentialFormSurf_flow_velocity_2_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 2))
            return new flowsteadystateaxi::CustomEssentialFormSurf_flow_velocity_2_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_linear_")&&(i == 3))
            return new flowsteadystateaxi::CustomEssentialFormSurf_fluid_pressure_3_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_steadystate_axisymmetric_newton_")&&(i == 3))
            return new flowsteadystateaxi::CustomEssentialFormSurf_fluid_pressure_3_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 1))
            return new flowtransientplanar::CustomEssentialFormSurf_flow_velocity_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 1))
            return new flowtransientplanar::CustomEssentialFormSurf_flow_velocity_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_planar_linear_")&&(i == 2))
            return new flowtransientplanar::CustomEssentialFormSurf_flow_velocity_2_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_planar_newton_")&&(i == 2))
            return new flowtransientplanar::CustomEssentialFormSurf_flow_velocity_2_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 1))
            return new flowtransientaxi::CustomEssentialFormSurf_flow_velocity_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 1))
            return new flowtransientaxi::CustomEssentialFormSurf_flow_velocity_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_axisymmetric_linear_")&&(i == 2))
            return new flowtransientaxi::CustomEssentialFormSurf_flow_velocity_2_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "flow_transient_axisymmetric_newton_")&&(i == 2))
            return new flowtransientaxi::CustomEssentialFormSurf_flow_velocity_2_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 1))
            return new acousticharmonicplanar::CustomEssentialFormSurf_acoustic_pressure_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 1))
            return new acousticharmonicplanar::CustomEssentialFormSurf_acoustic_pressure_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_planar_linear_")&&(i == 2))
            return new acousticharmonicplanar::CustomEssentialFormSurf_acoustic_pressure_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_planar_newton_")&&(i == 2))
            return new acousticharmonicplanar::CustomEssentialFormSurf_acoustic_pressure_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 1))
            return new acousticharmonicaxi::CustomEssentialFormSurf_acoustic_pressure_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 1))
            return new acousticharmonicaxi::CustomEssentialFormSurf_acoustic_pressure_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_linear_")&&(i == 2))
            return new acousticharmonicaxi::CustomEssentialFormSurf_acoustic_pressure_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_harmonic_axisymmetric_newton_")&&(i == 2))
            return new acousticharmonicaxi::CustomEssentialFormSurf_acoustic_pressure_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 1))
            return new acoustictransientplanar::CustomEssentialFormSurf_acoustic_pressure_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 1))
            return new acoustictransientplanar::CustomEssentialFormSurf_acoustic_pressure_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_planar_linear_")&&(i == 2))
            return new acoustictransientplanar::CustomEssentialFormSurf_acoustic_pressure_2_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_planar_newton_")&&(i == 2))
            return new acoustictransientplanar::CustomEssentialFormSurf_acoustic_pressure_2_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 1))
            return new acoustictransientaxi::CustomEssentialFormSurf_acoustic_pressure_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 1))
            return new acoustictransientaxi::CustomEssentialFormSurf_acoustic_pressure_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_linear_")&&(i == 2))
            return new acoustictransientaxi::CustomEssentialFormSurf_acoustic_pressure_2_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "acoustic_transient_axisymmetric_newton_")&&(i == 2))
            return new acoustictransientaxi::CustomEssentialFormSurf_acoustic_pressure_2_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "current_steadystate_planar_linear_")&&(i == 1))
            return new currentsteadystateplanar::CustomEssentialFormSurf_current_potential_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "current_steadystate_planar_newton_")&&(i == 1))
            return new currentsteadystateplanar::CustomEssentialFormSurf_current_potential_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "current_steadystate_axisymmetric_linear_")&&(i == 1))
            return new currentsteadystateaxi::CustomEssentialFormSurf_current_potential_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "current_steadystate_axisymmetric_newton_")&&(i == 1))
            return new currentsteadystateaxi::CustomEssentialFormSurf_current_potential_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 1))
            return new currentharmonicplanar::CustomEssentialFormSurf_current_potential_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 1))
            return new currentharmonicplanar::CustomEssentialFormSurf_current_potential_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_planar_linear_")&&(i == 2))
            return new currentharmonicplanar::CustomEssentialFormSurf_current_potential_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_planar_newton_")&&(i == 2))
            return new currentharmonicplanar::CustomEssentialFormSurf_current_potential_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 1))
            return new currentharmonicaxi::CustomEssentialFormSurf_current_potential_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 1))
            return new currentharmonicaxi::CustomEssentialFormSurf_current_potential_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_axisymmetric_linear_")&&(i == 2))
            return new currentharmonicaxi::CustomEssentialFormSurf_current_potential_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "current_harmonic_axisymmetric_newton_")&&(i == 2))
            return new currentharmonicaxi::CustomEssentialFormSurf_current_potential_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_fixed_fixed_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_fixed_fixed_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_fixed_fixed_2_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_fixed_fixed_2_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 1))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_fixed_free_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 1))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_fixed_free_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_linear_")&&(i == 2))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_free_fixed_2_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_planar_newton_")&&(i == 2))
            return new elasticitysteadystateplanar::CustomEssentialFormSurf_elasticity_free_fixed_2_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_fixed_fixed_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_fixed_fixed_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_fixed_fixed_2_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_fixed_fixed_2_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 1))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_fixed_free_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 1))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_fixed_free_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_linear_")&&(i == 2))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_free_fixed_2_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "elasticity_steadystate_axisymmetric_newton_")&&(i == 2))
            return new elasticitysteadystateaxi::CustomEssentialFormSurf_elasticity_free_fixed_2_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "electrostatic_steadystate_planar_linear_")&&(i == 1))
            return new electrostaticsteadystateplanar::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "electrostatic_steadystate_planar_newton_")&&(i == 1))
            return new electrostaticsteadystateplanar::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "electrostatic_steadystate_axisymmetric_linear_")&&(i == 1))
            return new electrostaticsteadystateaxi::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "electrostatic_steadystate_axisymmetric_newton_")&&(i == 1))
            return new electrostaticsteadystateaxi::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "heat_steadystate_planar_linear_")&&(i == 1))
            return new heatsteadystateplanar::CustomEssentialFormSurf_heat_temperature_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "heat_steadystate_planar_newton_")&&(i == 1))
            return new heatsteadystateplanar::CustomEssentialFormSurf_heat_temperature_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "heat_steadystate_axisymmetric_linear_")&&(i == 1))
            return new heatsteadystateaxi::CustomEssentialFormSurf_heat_temperature_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "heat_steadystate_axisymmetric_newton_")&&(i == 1))
            return new heatsteadystateaxi::CustomEssentialFormSurf_heat_temperature_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "heat_transient_planar_linear_")&&(i == 1))
            return new heattransientplanar::CustomEssentialFormSurf_heat_temperature_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "heat_transient_planar_newton_")&&(i == 1))
            return new heattransientplanar::CustomEssentialFormSurf_heat_temperature_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "heat_transient_axisymmetric_linear_")&&(i == 1))
            return new heattransientaxi::CustomEssentialFormSurf_heat_temperature_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "heat_transient_axisymmetric_newton_")&&(i == 1))
            return new heattransientaxi::CustomEssentialFormSurf_heat_temperature_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_steadystate_planar_linear_")&&(i == 1))
            return new magneticsteadystateplanar::CustomEssentialFormSurf_magnetic_potential_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_steadystate_planar_newton_")&&(i == 1))
            return new magneticsteadystateplanar::CustomEssentialFormSurf_magnetic_potential_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_steadystate_axisymmetric_linear_")&&(i == 1))
            return new magneticsteadystateaxi::CustomEssentialFormSurf_magnetic_potential_1_0_steadystate_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_steadystate_axisymmetric_newton_")&&(i == 1))
            return new magneticsteadystateaxi::CustomEssentialFormSurf_magnetic_potential_1_0_steadystate_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_transient_planar_linear_")&&(i == 1))
            return new magnetictransientplanar::CustomEssentialFormSurf_magnetic_potential_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_transient_planar_newton_")&&(i == 1))
            return new magnetictransientplanar::CustomEssentialFormSurf_magnetic_potential_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_transient_axisymmetric_linear_")&&(i == 1))
            return new magnetictransientaxi::CustomEssentialFormSurf_magnetic_potential_1_0_transient_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_transient_axisymmetric_newton_")&&(i == 1))
            return new magnetictransientaxi::CustomEssentialFormSurf_magnetic_potential_1_0_transient_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 1))
            return new magneticharmonicplanar::CustomEssentialFormSurf_magnetic_potential_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 1))
            return new magneticharmonicplanar::CustomEssentialFormSurf_magnetic_potential_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_planar_linear_")&&(i == 2))
            return new magneticharmonicplanar::CustomEssentialFormSurf_magnetic_potential_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_planar_newton_")&&(i == 2))
            return new magneticharmonicplanar::CustomEssentialFormSurf_magnetic_potential_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 1))
            return new magneticharmonicaxi::CustomEssentialFormSurf_magnetic_potential_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 1))
            return new magneticharmonicaxi::CustomEssentialFormSurf_magnetic_potential_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_linear_")&&(i == 2))
            return new magneticharmonicaxi::CustomEssentialFormSurf_magnetic_potential_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "magnetic_harmonic_axisymmetric_newton_")&&(i == 2))
            return new magneticharmonicaxi::CustomEssentialFormSurf_magnetic_potential_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 1))
            return new rfharmonicplanar::CustomEssentialFormSurf_rf_te_electric_field_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 1))
            return new rfharmonicplanar::CustomEssentialFormSurf_rf_te_electric_field_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_planar_linear_")&&(i == 2))
            return new rfharmonicplanar::CustomEssentialFormSurf_rf_te_electric_field_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_planar_newton_")&&(i == 2))
            return new rfharmonicplanar::CustomEssentialFormSurf_rf_te_electric_field_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 1))
            return new rfharmonicaxi::CustomEssentialFormSurf_rf_te_electric_field_1_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 1))
            return new rfharmonicaxi::CustomEssentialFormSurf_rf_te_electric_field_1_0_harmonic_newton_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_linear_")&&(i == 2))
            return new rfharmonicaxi::CustomEssentialFormSurf_rf_te_electric_field_2_0_harmonic_linear_<Scalar>(mesh, boundary);

        if ((problemId == "rf_harmonic_axisymmetric_newton_")&&(i == 2))
            return new rfharmonicaxi::CustomEssentialFormSurf_rf_te_electric_field_2_0_harmonic_newton_<Scalar>(mesh, boundary);

        return NULL;
    }

    template class WeakFormFactory<double>;
