import test_suite

tests = [
# electrostatic field
test_suite.fields.electrostatic.ElectrostaticPlanar, 
test_suite.fields.electrostatic.ElectrostaticAxisymmetric,
# current field
test_suite.fields.current.CurrentPlanar,
test_suite.fields.current.CurrentAxisymmetric,
# elasticity
test_suite.fields.elasticity.ElasticityPlanar,
test_suite.fields.elasticity.ElasticityAxisymmetric,
# incompressible flow
test_suite.fields.flow.FlowPlanar,
test_suite.fields.flow.FlowAxisymmetric,
# acoustic field
test_suite.fields.acoustic.AcousticHarmonicPlanar,
test_suite.fields.acoustic.AcousticHarmonicAxisymmetric,
# heat transfer
test_suite.fields.heat.HeatPlanar,
test_suite.fields.heat.HeatAxisymmetric,
test_suite.fields.heat.HeatNonlinPlanar,
test_suite.fields.heat.HeatTransientAxisymmetric,
# magnetic field
test_suite.fields.magnetic.MagneticPlanar,
test_suite.fields.magnetic.MagneticAxisymmetric,
test_suite.fields.magnetic.MagneticHarmonicPlanar,
test_suite.fields.magnetic.MagneticHarmonicAxisymmetric,
test_suite.fields.magnetic.MagneticHarmonicPlanarTotalCurrent,
test_suite.fields.magnetic.MagneticHarmonicAxisymmetricTotalCurrent,
# rf te
test_suite.fields.rf_te.RFTEHarmonicPlanar,
test_suite.fields.rf_te.RFTEHarmonicAxisymmetric,
# rf tm
test_suite.fields.rf_tm.RFTMHarmonicPlanar,
test_suite.fields.rf_tm.RFTMHarmonicAxisymmetric,
# adaptivity
test_suite.adaptivity.adaptivity.AdaptivityElectrostatic,
test_suite.adaptivity.adaptivity.AdaptivityAcoustic,
# particle tracing
test_suite.particle_tracing.particle_tracing.ParticleTracingPlanar,
test_suite.particle_tracing.particle_tracing.ParticleTracingAxisymmetric,
# coupled fields
test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic1,
test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic2,
test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic3,
test_suite.coupled_problems.basic_coupled_problems.CoupledProblemsBasic4,
# script
test_suite.script.problem.TestProblem,
test_suite.script.geometry.TestGeometry,
test_suite.script.geometry.TestGeometryTransformations,
test_suite.script.benchmark.BenchmarkGeometryTransformation,
# internal
test_suite.internal.matrix_solvers.InternalMatrixSolvers
]

if __name__ == '__main__':        
    import unittest as ut
    import agros2d
    
    suite = ut.TestSuite()
    
    for test in tests:
        suite.addTest(ut.TestLoader().loadTestsFromTestCase(test))
    
    result = agros2d.Agros2DTestResult()
    suite.run(result)

    if (not result.wasSuccessful()):
        raise Exception('Failure', result.failures)