from test_suite import adaptivity
from test_suite import coupled_problems
from test_suite import examples
from test_suite import fields
from test_suite import core
from test_suite import particle_tracing
from test_suite import script
from test_suite import optilab

import inspect
def __get_tests__(object):
    tests = list()

    if isinstance(object, list):
        for member in object:
            tests += __get_tests__(member)
    else:
        for name, obj in inspect.getmembers(object):
            test_or_benchmark = name.startswith('Test') or name.startswith('Benchmark')
            if inspect.isclass(obj) and test_or_benchmark:
                tests.append(obj)

    return tests

__tests__ = dict()

# fields 
__tests__["fields"] = __get_tests__([fields.electrostatic, fields.current, fields.magnetic_steady, fields.magnetic_harmonic, 
                            fields.magnetic_transient, fields.heat, fields.elasticity, fields.flow, fields.acoustic, 
                            fields.rf_te, fields.rf_tm])                             


# coupled
__tests__["coupled"] = [
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakWeak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakHard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardWeak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardHard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Weak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Hard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakWeak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakHard,
#coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3HardWeak,
#coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3HardHard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4Weak,
#coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4Hard,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakWeak,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakHard,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardWeak,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
]

# nonlin
__tests__["nonlin"] = [
fields.heat.TestHeatNonlinPlanar,
fields.magnetic_steady.TestMagneticNonlinPlanar,
fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar,
fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric,
fields.flow.TestFlowPlanar,
fields.flow.TestFlowAxisymmetric
]

# adaptivity 
__tests__["adaptivity"] = __get_tests__(adaptivity.adaptivity)

# tracing 
__tests__["tracing"] = __get_tests__(particle_tracing.particle_tracing)

# script 
__tests__["script"] = __get_tests__([script.problem, script.field, script.geometry,
                                     script.benchmark, script.script])      
        
# examples 
__tests__["examples"] = examples.examples.tests

# core 
__tests__["core"] = __get_tests__(core.matrix_solvers) + __get_tests__(core.generator) + core.xslt.tests

# optilab
# __tests__["optilab"] = __get_tests__([optilab.model_set_manager, optilab.genetic])

# complete 
__tests__["complete"] = __tests__["fields"] + __tests__["coupled"] + __tests__["nonlin"] + \
                        __tests__["adaptivity"] + __tests__["tracing"] + \
                        __tests__["script"] + __tests__["examples"] + __tests__["core"]


# fast 
__tests__["fast"] = [
# electrostatic field
fields.electrostatic.TestElectrostaticPlanar, 
fields.electrostatic.TestElectrostaticAxisymmetric,
# current field
fields.current.TestCurrentPlanar,
fields.current.TestCurrentAxisymmetric,
# elasticity
fields.elasticity.TestElasticityPlanar,
fields.elasticity.TestElasticityAxisymmetric,
# incompressible flow
fields.flow.TestFlowPlanar,
fields.flow.TestFlowAxisymmetric,
# acoustic field
fields.acoustic.TestAcousticHarmonicPlanar,
fields.acoustic.TestAcousticHarmonicAxisymmetric,
# heat transfer
fields.heat.TestHeatPlanar,
fields.heat.TestHeatAxisymmetric,
fields.heat.TestHeatNonlinPlanar,
fields.heat.TestHeatTransientAxisymmetric,
# magnetic field
fields.magnetic_steady.TestMagneticPlanar,
fields.magnetic_steady.TestMagneticAxisymmetric,
fields.magnetic_harmonic.TestMagneticHarmonicPlanar,
fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric,
fields.magnetic_harmonic.TestMagneticHarmonicPlanarTotalCurrent,
fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetricTotalCurrent,
fields.magnetic_steady.TestMagneticNonlinPlanar,
fields.magnetic_steady.TestMagneticNonlinAxisymmetric,
fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar,
fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric,
# rf te
fields.rf_te.TestRFTEHarmonicPlanar,
fields.rf_te.TestRFTEHarmonicAxisymmetric,
# rf tm
fields.rf_tm.TestRFTMHarmonicPlanar,
fields.rf_tm.TestRFTMHarmonicAxisymmetric,
# math coeff
fields.math_coeff.TestMathCoeffPlanar,
fields.math_coeff.TestMathCoeffAxisymmetric,
# adaptivity
adaptivity.adaptivity.TestAdaptivityElectrostatic,
adaptivity.adaptivity.TestAdaptivityAcoustic,
adaptivity.adaptivity.TestAdaptivityElasticityBracket,
adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor,
adaptivity.adaptivity.TestAdaptivityRF_TE,
adaptivity.adaptivity.TestAdaptivityHLenses,
adaptivity.adaptivity.TestAdaptivityPAndHCoupled,
# particle tracing
particle_tracing.particle_tracing.TestParticleTracingPlanar,
particle_tracing.particle_tracing.TestParticleTracingAxisymmetric,
# coupled fields
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakWeak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakHard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardWeak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardHard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Weak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Hard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakWeak,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakHard,
#coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3HardWeak,
#coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3HardHard,
coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4Weak,
#coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4Hard,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakWeak,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakHard,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardWeak,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
# core
core.matrix_solvers.TestInternalMatrixSolvers
] + __tests__["script"]

def all_tests():
    global __tests__
    return __tests__

def test(name):
    global __tests__
    return __tests__[name]