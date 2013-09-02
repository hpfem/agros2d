__all__ = ["scenario", "adaptivity", "coupled_problems", "fields", "internal", "particle_tracing", "script"]

#from scenario import Agros2DTestCase
#from scenario import Agros2DTestResult
#from scenario import find_all_tests
#from scenario import run

import scenario

import adaptivity
import coupled_problems
import fields
import internal
import particle_tracing
import script

test_pokus = [
fields.current.CurrentPlanar
]

test_fast = [
# electrostatic field
fields.electrostatic.ElectrostaticPlanar, 
fields.electrostatic.ElectrostaticAxisymmetric,
# current field
fields.current.CurrentPlanar,
fields.current.CurrentAxisymmetric,
# elasticity
fields.elasticity.ElasticityPlanar,
fields.elasticity.ElasticityAxisymmetric,
# incompressible flow
fields.flow.FlowPlanar,
fields.flow.FlowAxisymmetric,
# acoustic field
fields.acoustic.AcousticHarmonicPlanar,
fields.acoustic.AcousticHarmonicAxisymmetric,
# heat transfer
fields.heat.HeatPlanar,
fields.heat.HeatAxisymmetric,
fields.heat.HeatNonlinPlanar,
fields.heat.HeatTransientAxisymmetric,
# magnetic field
fields.magnetic.MagneticPlanar,
fields.magnetic.MagneticAxisymmetric,
fields.magnetic.MagneticHarmonicPlanar,
fields.magnetic.MagneticHarmonicAxisymmetric,
fields.magnetic.MagneticHarmonicPlanarTotalCurrent,
fields.magnetic.MagneticHarmonicAxisymmetricTotalCurrent,
# rf te
fields.rf_te.RFTEHarmonicPlanar,
fields.rf_te.RFTEHarmonicAxisymmetric,
# rf tm
fields.rf_tm.RFTMHarmonicPlanar,
fields.rf_tm.RFTMHarmonicAxisymmetric,
# adaptivity
adaptivity.adaptivity.AdaptivityElectrostatic,
adaptivity.adaptivity.AdaptivityAcoustic,
# particle tracing
particle_tracing.particle_tracing.ParticleTracingPlanar,
particle_tracing.particle_tracing.ParticleTracingAxisymmetric,
# coupled fields
coupled_problems.basic_coupled_problems.CoupledProblemsBasic1,
coupled_problems.basic_coupled_problems.CoupledProblemsBasic2,
coupled_problems.basic_coupled_problems.CoupledProblemsBasic3,
coupled_problems.basic_coupled_problems.CoupledProblemsBasic4,
# script
script.problem.TestProblem,
script.geometry.TestGeometry,
script.geometry.TestGeometryTransformations,
script.benchmark.BenchmarkGeometryTransformation,
# internal
internal.matrix_solvers.InternalMatrixSolvers
]

test_nonlin = [
# heat transfer
fields.heat.HeatNonlinPlanar,
# magnetic field
fields.magnetic.MagneticNonlinPlanar,
#fields.magnetic.MagneticNonlinAxisymmetric,
#fields.magnetic.MagneticHarmonicNonlinPlanar,
# incompressible flow
fields.flow.FlowPlanar,
fields.flow.FlowAxisymmetric
]

test_script = [
script.problem.TestProblem,
script.geometry.TestGeometry,
script.geometry.TestGeometryTransformations,
script.benchmark.BenchmarkGeometryTransformation
]