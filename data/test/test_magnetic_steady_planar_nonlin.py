import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "steadystate"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 2
magnetic.linearity_type = "newton"
magnetic.nonlinear_tolerance = 0.2
magnetic.nonlinear_steps = 20
magnetic.damping_type = "fixed"
magnetic.reuse_jacobian = True
magnetic.maximum_steps_with_reused_jacobian = 10
magnetic.sufficient_improvement_factor_for_jacobian_reuse = 0.9

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})

magnetic.add_material("Civka 1", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : 5e6, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
magnetic.add_material("Civka 2", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : -5e6, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
magnetic.add_material("Iron", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : { "value" : 995, "x" : [0,0.2,0.5,0.8,1.15,1.3,1.45,1.6,1.69,2,2.2,2.5,3,5,10,20], "y" : [995,995,991,933,771,651,473,311,245,40,30,25,20,8,5,2] }, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
magnetic.add_material("Air", {"magnetic_conductivity" : 0, "magnetic_current_density_external_real" : 0, "magnetic_permeability" : 1, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(-0.05, -0.01, 0.05, -0.01, boundaries = {})
geometry.add_edge(0.05, -0.01, 0.05, -0.005, boundaries = {})
geometry.add_edge(0.05, -0.005, -0.05, -0.005, boundaries = {})
geometry.add_edge(-0.05, -0.005, -0.05, -0.01, boundaries = {})
geometry.add_edge(-0.05, 0.06, -0.05, 0.04, boundaries = {})
geometry.add_edge(-0.03, 0, -0.05, 0, boundaries = {})
geometry.add_edge(-0.05, 0, -0.05, 0.04, boundaries = {})
geometry.add_edge(-0.05, 0.06, 0.05, 0.06, boundaries = {})
geometry.add_edge(0.05, 0.06, 0.05, 0.04, boundaries = {})
geometry.add_edge(0.05, 0, 0.05, 0.04, boundaries = {})
geometry.add_edge(0.03, 0, 0.05, 0, boundaries = {})
geometry.add_edge(-0.028, 0.04, 0.028, 0.04, boundaries = {})
geometry.add_edge(-0.15, -0.1, 0.15, -0.1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.15, -0.1, 0.15, 0.15, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.15, 0.15, -0.15, 0.15, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.15, 0.15, -0.15, -0.1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.028, 0.074, 0.028, 0.074, boundaries = {})
geometry.add_edge(0.028, 0.074, 0.028, 0.062, boundaries = {})
geometry.add_edge(-0.028, 0.074, -0.028, 0.062, boundaries = {})
geometry.add_edge(-0.028, 0.062, 0.028, 0.062, boundaries = {})
geometry.add_edge(-0.028, 0.038, -0.028, 0.026, boundaries = {})
geometry.add_edge(-0.028, 0.038, 0.028, 0.038, boundaries = {})
geometry.add_edge(0.028, 0.038, 0.028, 0.026, boundaries = {})
geometry.add_edge(0.028, 0.026, -0.028, 0.026, boundaries = {})
geometry.add_edge(0.03, 0.038, 0.028, 0.04, boundaries = {}, angle = 90)
geometry.add_edge(0.03, 0, 0.03, 0.038, boundaries = {})
geometry.add_edge(-0.03, 0.038, -0.03, 0, boundaries = {})
geometry.add_edge(-0.028, 0.04, -0.03, 0.038, boundaries = {}, angle = 90)

# labels
geometry.add_label(0.076268, 0.119048, materials = {"magnetic" : "Air"})
geometry.add_label(-0.0126334, 0.0554136, materials = {"magnetic" : "Iron"}, area = 2e-05)
geometry.add_label(0.0173943, -0.00630466, materials = {"magnetic" : "Iron"})
geometry.add_label(0.00305095, 0.0688963, materials = {"magnetic" : "Civka 2"})
geometry.add_label(-0.00501291, 0.0328188, materials = {"magnetic" : "Civka 1"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(1.814e-02, -7.690e-03)
testB = agros2d.test("Flux density", point["Br"], 1.819661)
testmur = agros2d.test("Magnetic potential", point["mur"], 139.364773)

# volume integral
volume = magnetic.volume_integrals([2])
testWm = agros2d.test("Energy", volume["Wm"], 3.264012)

elapsed_time = time() - start_time
print("Test: Magnetic steady state - nonlin - planar ({0}): ".format(round(elapsed_time, 3)) + str(point and testmur and testB and testWm))
