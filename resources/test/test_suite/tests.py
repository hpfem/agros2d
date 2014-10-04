import test_suite
import inspect
import types

def __get_tests__(object):
    tests = list()
    if isinstance(object, list):
        for member in object:
            tests += __get_tests__(member)

    if isinstance(object, types.ModuleType):
        for name, obj in inspect.getmembers(object):
            for sub_name, sub_obj in inspect.getmembers(obj):
                if isinstance(sub_obj, types.ModuleType): continue
                test_or_benchmark = sub_name.startswith('Test') or sub_name.startswith('Benchmark')
                if inspect.isclass(sub_obj) and test_or_benchmark:
                    tests.append(sub_obj)

    for name, obj in inspect.getmembers(object):
        test_or_benchmark = name.startswith('Test') or name.startswith('Benchmark')
        if inspect.isclass(obj) and test_or_benchmark:
            tests.append(obj)

    return tests
__tests__ = dict()

# fields
__tests__["fields"] = __get_tests__(test_suite.fields)

# coupled
__tests__["coupled"] = __get_tests__(test_suite.coupled_problems)

# nonlin
__tests__["nonlin"] = [
test_suite.fields.heat.TestHeatNonlinPlanar,
test_suite.fields.magnetic_steady.TestMagneticNonlinPlanar,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric,
test_suite.fields.flow.TestFlowPlanar,
test_suite.fields.flow.TestFlowAxisymmetric
]

# adaptivity
__tests__["adaptivity"] = __get_tests__(test_suite.adaptivity)

# tracing
__tests__["tracing"] = __get_tests__(test_suite.particle_tracing)

# script
__tests__["script"] = __get_tests__(test_suite.script)

# examples
__tests__["examples"] = test_suite.examples.examples.tests

# core
__tests__["core"] = __get_tests__(test_suite.core.matrix_solvers) + \
                    __get_tests__(test_suite.core.generator) + \
                    test_suite.core.xslt.tests
# optilab
__tests__["optilab"] = __get_tests__(test_suite.optilab) + \
                       __get_tests__(test_suite.optilab.optimization)

# complete 
__tests__["complete"] = __tests__["fields"] + __tests__["coupled"] + __tests__["nonlin"] + \
                        __tests__["adaptivity"] + __tests__["tracing"] + \
                        __tests__["script"] + __tests__["examples"] + __tests__["core"] + \
                        __tests__["optilab"]

# fast 
__tests__["fast"] = [
# electrostatic field
test_suite.fields.electrostatic.TestElectrostaticPlanar, 
test_suite.fields.electrostatic.TestElectrostaticAxisymmetric,
# current field
test_suite.fields.current.TestCurrentPlanar,
test_suite.fields.current.TestCurrentAxisymmetric,
# elasticity
test_suite.fields.elasticity.TestElasticityPlanar,
test_suite.fields.elasticity.TestElasticityAxisymmetric,
# incompressible flow
test_suite.fields.flow.TestFlowPlanar,
test_suite.fields.flow.TestFlowAxisymmetric,
# acoustic field
test_suite.fields.acoustic.TestAcousticHarmonicPlanar,
test_suite.fields.acoustic.TestAcousticHarmonicAxisymmetric,
# heat transfer
test_suite.fields.heat.TestHeatPlanar,
test_suite.fields.heat.TestHeatAxisymmetric,
test_suite.fields.heat.TestHeatNonlinPlanar,
test_suite.fields.heat.TestHeatTransientAxisymmetric,
# magnetic field
test_suite.fields.magnetic_steady.TestMagneticPlanar,
test_suite.fields.magnetic_steady.TestMagneticAxisymmetric,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicPlanar,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicPlanarTotalCurrent,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetricTotalCurrent,
test_suite.fields.magnetic_steady.TestMagneticNonlinPlanar,
test_suite.fields.magnetic_steady.TestMagneticNonlinAxisymmetric,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar,
test_suite.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric,
# rf te
test_suite.fields.rf_te.TestRFTEHarmonicPlanar,
test_suite.fields.rf_te.TestRFTEHarmonicAxisymmetric,
# rf tm
test_suite.fields.rf_tm.TestRFTMHarmonicPlanar,
test_suite.fields.rf_tm.TestRFTMHarmonicAxisymmetric,
# math coeff
test_suite.fields.math_coeff.TestMathCoeffPlanar,
test_suite.fields.math_coeff.TestMathCoeffAxisymmetric,
# adaptivity
test_suite.adaptivity.adaptivity.TestAdaptivityElectrostatic,
test_suite.adaptivity.adaptivity.TestAdaptivityAcoustic,
test_suite.adaptivity.adaptivity.TestAdaptivityElasticityBracket,
test_suite.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor,
test_suite.adaptivity.adaptivity.TestAdaptivityRF_TE,
test_suite.adaptivity.adaptivity.TestAdaptivityHLenses,
test_suite.adaptivity.adaptivity.TestAdaptivityPAndHCoupled,
# particle tracing
test_suite.particle_tracing.particle_tracing.TestParticleTracingPlanar,
test_suite.particle_tracing.particle_tracing.TestParticleTracingAxisymmetric,
# coupled fields
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakWeak,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakHard,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardWeak,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardHard,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Weak,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Hard,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakWeak,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakHard,
test_suite.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4Weak,
test_suite.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakWeak,
test_suite.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakHard,
test_suite.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardWeak,
test_suite.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
# core
test_suite.core.matrix_solvers.TestInternalMatrixSolvers
] + __tests__["script"]

def all_tests():
    global __tests__
    return __tests__

def test(name):
    global __tests__
    return __tests__[name]
