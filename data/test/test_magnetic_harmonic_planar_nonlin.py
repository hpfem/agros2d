import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 50

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "harmonic"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 2
magnetic.linearity_type = "newton"
magnetic.nonlinear_tolerance = 0.001
magnetic.nonlinear_steps = 15

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_imag" : 0, "magnetic_potential_real" : 0})

magnetic.add_material("Zelezo", {"magnetic_conductivity" : 5e6, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 3e6, "magnetic_permeability" : { "value" : 5000, "x" : [0,0.227065,0.45413,0.681195,0.90826,1.13533,1.36239,1.58935,1.81236,2.01004,2.13316,2.19999,2.25479,2.29993,2.34251,2.37876,2.41501,2.45126,2.4875,2.52375,2.56,3,5,10,20], "y" : [13001,13001,13001,12786,12168,10967,7494,1409,315,90,41,26,19,15,12,11,9,8,8,7,6,4,3,3,2] }, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})


# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, 0.0035, -0.0035, 0, boundaries = {"magnetic" : "A = 0"}, angle = 90)
geometry.add_edge(-0.0035, 0, 0, -0.0035, boundaries = {"magnetic" : "A = 0"}, angle = 90)
geometry.add_edge(0, -0.0035, 0.0035, 0, boundaries = {"magnetic" : "A = 0"}, angle = 90)
geometry.add_edge(0.0035, 0, 0, 0.0035, boundaries = {"magnetic" : "A = 0"}, angle = 90)

# labels
geometry.add_label(0, 0, materials = {"magnetic" : "Zelezo"}, area = 5e-07)

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(0.00299, 0.001355)
testB = agros2d.test("Flux density", point["B"], 1.396825)
testmur = agros2d.test("Magnetic potential", point["mur"], 6666.8723)
testJ_real = agros2d.test("Current density - total - real", point["Jr"], 6.24881e5)
testJ_imag = agros2d.test("Current density - total - imag", point["Ji"], -3.112391e5)

# volume integral
volume = magnetic.volume_integrals([2])
testWm = agros2d.test("Energy", volume["Wm"], 6.219017e-4)

elapsed_time = time() - start_time
print("Test: Magnetic harmonic - nonlin - planar ({0}): ".format(round(elapsed_time, 3)) + str(point and testmur and testB and testJ_real and testJ_imag and testWm))