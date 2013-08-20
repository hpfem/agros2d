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
suite.addTest(ut.TestLoader().loadTestsFromTestCase(acoustic.AcousticTransientPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(acoustic.AcousticTransientAxisymmetric))

# heat transfer
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatNonlinPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatTransientAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(heat.HeatTransientBenchmarkAxisymmetric))

# magnetic field
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticNonlinPlanar))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticNonlinAxisymmetric))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticHarmonicAxisymmetric))
#suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticHarmonicNonlinPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticTransientPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(magnetic.MagneticTransientAxisymmetric))

# rf te
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_te.RFTEHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_te.RFTEHarmonicAxisymmetric))

# rf tm
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_tm.RFTMHarmonicPlanar))
suite.addTest(ut.TestLoader().loadTestsFromTestCase(rf_tm.RFTMHarmonicAxisymmetric))

# run tests
suite.run(result)

from test import Test

test_complete = Test(error_file='test_complete.err') 

# contains sets of tests obtained by varying previously present coupling tests
# various combinations of hard/weak and linear/nonlinear are tested
# in some couplings, the forms are the same for one of fields being steadystate or transient. 
# in our implementation, however, two sets of forms are present (two identical copies). 
# Those forms are also tested by changing steadystate problem artificaly to transient and 
# setting coefficients of the time derivative term to zero, thus obtaining the same results

# based on test_coupling_1_planar.py
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_heat_steady_lin_elast_lin_weak_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_heat_trans_lin_elast_lin_weak_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_weak_with_heat_steady_lin_hard_with_elast_lin_planar.py")
test_complete.add("coupled_problems/test_coup1_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_planar.py")
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_planar.py")
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_hard_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_steady_nl_elast_nl_weak_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_hard_planar.py") 
test_complete.add("coupled_problems/test_coup1_curr_steady_nl_heat_trans_nl_elast_nl_weak_planar.py") 

# based on test_coupling_2_axisymmetric.py
test_complete.add("coupled_problems/test_coup2_mag_harm_lin_heat_steady_lin_weak_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_lin_heat_trans_lin_weak_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_steady_nl_hard_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_steady_nl_weak_axisym.py")
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_trans_nl_hard_axisym.py") 
test_complete.add("coupled_problems/test_coup2_mag_harm_nl_heat_trans_nl_weak_axisym.py") 

# based on test_coupling_3_axisymmetric_nonlin.py
test_complete.add("coupled_problems/test_coup3_curr_steady_lin_heat_steady_nl_elast_lin_axisym_weak.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_lin_heat_trans_nl_elast_lin_axisym_weak.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_lin_weak_with_heat_steady_nl_hard_with_elast_nl_axisym.py")
#test_complete.add("coupled_problems/test_coup3_curr_steady_nl_hard_with_heat_steady_nl_weak_with_elast_lin_axisym.py");
#test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_hard.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_steady_nl_elast_nl_axisym_weak.py")
#the following test works, even though it is one-step transient version of the steady state that does not
test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_hard.py")
test_complete.add("coupled_problems/test_coup3_curr_steady_nl_heat_trans_nl_elast_nl_axisym_weak.py")

# based on test_coupling_4_transient_planar.py
test_complete.add("coupled_problems/test_coup4_curr_steady_lin_heat_trans_lin_planar_weak.py")
test_complete.add("coupled_problems/test_coup4_curr_steady_nl_heat_trans_nl_planar_hard.py")
test_complete.add("coupled_problems/test_coup4_curr_steady_nl_heat_trans_nl_planar_weak.py") 

# particle tracing
test_complete.add("particle_tracing/test_particle_tracing_planar.py")
test_complete.add("particle_tracing/test_particle_tracing_axisymmetric.py")

# adaptivity
test_complete.add("adaptivity/test_adaptivity_acoustic.py")
test_complete.add("adaptivity/test_adaptivity_electrostatic.py")

# scripting
test_complete.add("test_script.py")

test_complete.run()