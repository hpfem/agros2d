import agros2d as a2d
import unittest as ut

import test_suite

suite = ut.TestSuite()

# fields
# electrostatic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.electrostatic.ElectrostaticPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.electrostatic.ElectrostaticAxisymmetric))

# current field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.current.CurrentPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.current.CurrentAxisymmetric))

# elasticity
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.elasticity.ElasticityPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.elasticity.ElasticityAxisymmetric))

# incompressible flow
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.flow.FlowPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.flow.FlowAxisymmetric))

# acoustic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.acoustic.AcousticHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.acoustic.AcousticHarmonicAxisymmetric))

# heat transfer
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.heat.HeatPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.heat.HeatAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.heat.HeatNonlinPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.heat.HeatTransientAxisymmetric))

# magnetic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticHarmonicAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticHarmonicPlanarTotalCurrent))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.magnetic.MagneticHarmonicAxisymmetricTotalCurrent))

# rf te
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.rf_te.RFTEHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.rf_te.RFTEHarmonicAxisymmetric))

# rf tm
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.rf_tm.RFTMHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.fields.rf_tm.RFTMHarmonicAxisymmetric))

# adaptivity
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.adaptivity.adaptivity.AdaptivityElectrostatic))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.adaptivity.adaptivity.AdaptivityAcoustic))


# particle tracing
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.particle_tracing.particle_tracing.ParticleTracingPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.particle_tracing.particle_tracing.ParticleTracingAxisymmetric))


# coupled fields
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic1))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic2))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic3))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic4))


# script
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.problem.TestProblem))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.geometry.TestGeometry))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.geometry.TestGeometryTransformations))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.script.benchmark.BenchmarkGeometryTransformation))


# internal
suite.addTest(ut.TestLoader().loadTestsFromTestCase(test_suite.internal.matrix_solvers.InternalMatrixSolvers))

# run tests
result = a2d.Agros2DTestResult()
suite.run(result)

if (not result.wasSuccessful()):
   raise Exception('Failure', result.failures)
