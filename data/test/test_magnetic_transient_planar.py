import agros2d
import time as timer
start_time = timer.time()

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_total = 0.4
problem.time_steps = 50

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# magnetic
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "transient"
magnetic.initial_condition = 0
magnetic.number_of_refinements = 3
magnetic.polynomial_order = 2
magnetic.adaptivity_type = "disabled"
magnetic.linearity_type = "linear"

# boundaries
magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})

# materials
magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 57e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "1e7*(exp(-10/0.7*time) - exp(-12/0.7*time))" }})
magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})

# geometry
geometry = agros2d.geometry
geometry.add_edge(-0.75, 0.75, -0.75, -0.25, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.75, 0.75, 0.75, 0.75, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.75, 0.75, 0.75, -0.25, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.75, -0.25, -0.75, -0.25, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.25, 0, 0.2, 0.05)
geometry.add_edge(0.1, 0.2, 0.2, 0.05)
geometry.add_edge(0.1, 0.2, -0.2, 0.1)
geometry.add_edge(-0.2, 0.1, -0.25, 0)
geometry.add_edge(-0.2, 0.2, -0.05, 0.25)
geometry.add_edge(-0.05, 0.35, -0.05, 0.25)
geometry.add_edge(-0.05, 0.35, -0.2, 0.35)
geometry.add_edge(-0.2, 0.35, -0.2, 0.2)

geometry.add_label(0.1879, 0.520366, materials = {"magnetic" : "Air"})
geometry.add_label(-0.15588, 0.306142, materials = {"magnetic" : "Coil"})
geometry.add_label(-0.00331733, 0.106999, materials = {"magnetic" : "Copper"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(2.809e-02, 1.508e-01)
testA = agros2d.test("Magnetic potential", point["Ar"], 3.856287421197996E-4)
testB = agros2d.test("Flux density", point["Br"], 0.001116683242432341)
testBx = agros2d.test("Flux density - x", point["Brx"], 4.0104560252330734E-4)
testBy = agros2d.test("Flux density - y", point["Bry"], 0.001042182242829712)
testH = agros2d.test("Magnetic intensity", point["Hr"], 888.628289504962)
testHx = agros2d.test("Magnetic intensity - x", point["Hrx"], 319.1419502342593)
testHy = agros2d.test("Magnetic intensity - y", point["Hry"], 829.3422777447333)
testwm = agros2d.test("Energy density", point["wm"], 0.49615819725609656)
testpj = agros2d.test("Losses density ", point["pj"], 28.304)
testJitr = agros2d.test("Current density - induced transform", point["Jitr"], 40837.7342798334)
testJr = agros2d.test("Current density - total", point["Jr"], 40837.7342798334)
testFlx = agros2d.test("Lorenz force - x", point["Flx"], -41.847933217335076)
testFly = agros2d.test("Lorenz force - y", point["Fly"], 16.226339194728205)

# volume integral
volume = magnetic.volume_integrals([2])
testWm = agros2d.test("Energy", volume["Wm"], 0.04391581801480497)
testPj = agros2d.test("Losses", volume["Pj"], 0.7546173357026923)
testIntFlx = agros2d.test("Lorentz force integral - x", volume["Flx"], -0.06460752047773814)
testIntFlx = agros2d.test("Lorentz force integral - y", volume["Fly"], 0.3846384661715725)
testIitr = agros2d.test("Current - induced transform", volume["Iitr"], 812.2394578364593)
testIr1 = agros2d.test("Current - total1", volume["Ir"], 812.2394578364593)

volumeSource = magnetic.volume_integrals([1])
testIr2 = agros2d.test("Current - total2", volumeSource["Ir"], 421.23575)
testIer = agros2d.test("Current - external", volumeSource["Ier"], 421.23575)

# surface integral
# surface = magnetic.surface_integrals([0])

elapsed_time = timer.time() - start_time
print("Test: Magnetic transient - planar ({0}): ".format(round(elapsed_time, 3)) + str(point and testA and testB and testBx and testBy and testH and testHx and testHy and testwm and testpj
and testJitr and testJr and testFlx and testFly and testWm and testPj and testIer and testIitr and testIr1 and testIr2))