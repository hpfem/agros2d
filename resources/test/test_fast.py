import agros2d as a2d
import unittest as ut
import sys

suite = ut.TestSuite()
result = a2d.Agros2DTestResult()

# fields
sys.path.append("fields")
import electrostatic
import current
import elasticity
import flow
import acoustic
import heat
import magnetic
import rf_te
import rf_tm

# electrostatic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(electrostatic.ElectrostaticPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(electrostatic.ElectrostaticAxisymmetric))

# current field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(current.CurrentPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(current.CurrentAxisymmetric))

# elasticity
suite.addTest(ut.TestLoader().loadTestsFromTestCase(elasticity.ElasticityPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(elasticity.ElasticityAxisymmetric))

# incompressible flow
suite.addTest(ut.TestLoader().loadTestsFromTestCase(flow.FlowPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(flow.FlowAxisymmetric))

# acoustic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(acoustic.AcousticHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(acoustic.AcousticHarmonicAxisymmetric))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(acoustic.AcousticTransientPlanar))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(acoustic.AcousticTransientAxisymmetric))  

# heat transfer
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatNonlinPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatTransientAxisymmetric))

# magnetic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticHarmonicAxisymmetric))

# rf te
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_te.RFTEHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_te.RFTEHarmonicAxisymmetric))

# rf tm
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_tm.RFTMHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_tm.RFTMHarmonicAxisymmetric))

# adaptivity
sys.path.append("adaptivity")
import adaptivity

suite.addTest(ut.TestLoader().loadTestsFromTestCase(adaptivity.AdaptivityElectrostatic))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(adaptivity.AdaptivityAcoustic))


# particle tracing
sys.path.append("particle_tracing")
import particle_tracing

suite.addTest(ut.TestLoader().loadTestsFromTestCase(particle_tracing.ParticleTracingPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(particle_tracing.ParticleTracingAxisymmetric))


# coupled fields
sys.path.append("coupled_problems")
import basic_coupled_problems

suite.addTest(ut.TestLoader().loadTestsFromTestCase(basic_coupled_problems.CoupledProblemsBasic1))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(basic_coupled_problems.CoupledProblemsBasic2))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(basic_coupled_problems.CoupledProblemsBasic3))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(basic_coupled_problems.CoupledProblemsBasic4))


# script
sys.path.append("script")
import problem
import geometry
import benchmark

suite.addTest(ut.TestLoader().loadTestsFromTestCase(problem.TestProblem))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(geometry.TestGeometry))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(geometry.TestGeometryTransformations))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(benchmark.BenchmarkGeometryTransformation))


# internal
sys.path.append("internal")
import matrix_solvers

suite.addTest(ut.TestLoader().loadTestsFromTestCase(matrix_solvers.InternalMatrixSolvers))

# run tests
suite.run(result)