__all__ = ["scenario", "adaptivity", "coupled_problems", "fields", "core", "particle_tracing", "script"]

import scenario
import inspect

import adaptivity
import coupled_problems
import examples
import fields
import core
import particle_tracing
import script

def get_tests(object):
    tests = list()

    if isinstance(object, list):
        for member in object:
            tests += get_tests(member)
    else:
        for name, obj in inspect.getmembers(object):
            test_or_benchmark = name.startswith('Test') or name.startswith('Benchmark')
            if inspect.isclass(obj) and test_or_benchmark:
                tests.append(obj)

    return tests

""" fields """
test_fields = get_tests([fields.electrostatic, fields.current, fields.magnetic, fields.heat,
                         fields.elasticity, fields.flow, fields.acoustic, fields.rf_te, fields.rf_tm])

""" coupled """
test_coupled = [
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
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
]

""" nonlin """
test_nonlin = [
fields.heat.TestHeatNonlinPlanar,
fields.magnetic.TestMagneticNonlinPlanar,
fields.magnetic.TestMagneticHarmonicNonlinPlanar,
fields.magnetic.TestMagneticHarmonicNonlinAxisymmetric,
fields.flow.TestFlowPlanar,
fields.flow.TestFlowAxisymmetric
]

""" adaptivity """
test_adaptivity = get_tests(adaptivity.adaptivity)

""" fields """
test_tracing = get_tests(particle_tracing.particle_tracing)

""" script """
test_script = get_tests([script.problem, script.field, script.geometry,
                         script.benchmark, script.script])

""" examples """
test_examples = examples.examples.tests

""" core """
test_core = get_tests(core.matrix_solvers)
test_core += get_tests(core.generator)
test_core += core.xslt.tests

""" complete """
test_complete = test_fields + test_coupled + test_adaptivity + test_tracing +\
                test_script + test_examples + test_core

#print(len(test_complete))

""" fast """
test_fast = [
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
fields.magnetic.TestMagneticPlanar,
fields.magnetic.TestMagneticAxisymmetric,
fields.magnetic.TestMagneticHarmonicPlanar,
fields.magnetic.TestMagneticHarmonicAxisymmetric,
fields.magnetic.TestMagneticHarmonicPlanarTotalCurrent,
fields.magnetic.TestMagneticHarmonicAxisymmetricTotalCurrent,
fields.magnetic.TestMagneticNonlinPlanar,
fields.magnetic.TestMagneticNonlinAxisymmetric,
fields.magnetic.TestMagneticHarmonicNonlinPlanar,
fields.magnetic.TestMagneticHarmonicNonlinAxisymmetric,
# rf te
fields.rf_te.TestRFTEHarmonicPlanar,
fields.rf_te.TestRFTEHarmonicAxisymmetric,
# rf tm
fields.rf_tm.TestRFTMHarmonicPlanar,
fields.rf_tm.TestRFTMHarmonicAxisymmetric,
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
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
# core
core.matrix_solvers.TestInternalMatrixSolvers
] + test_script
