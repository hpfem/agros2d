import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 100

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "steadystate"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 5
magnetic.linearity_type = "linear"

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})

magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6})
magnetic.add_material("Fe", {"magnetic_permeability" : 500}) 
magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
magnetic.add_material("Magnet", {"magnetic_permeability" : 1.11, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : -30}) 
magnetic.add_material("Velocity", {"magnetic_permeability" : 1, "magnetic_conductivity" : 1e7, "magnetic_velocity_x" : 2})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(-0.5, 1, 0.5, 1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, -1, -0.5, -1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.2, 0.6, 0, 0.6)
geometry.add_edge(0, 0.3, 0, 0.6)
geometry.add_edge(-0.2, 0.3, 0, 0.3)
geometry.add_edge(-0.2, 0.6, -0.2, 0.3)
geometry.add_edge(-0.035, 0.135, 0.035, 0.135)
geometry.add_edge(0.035, 0.135, 0.035, -0.04)
geometry.add_edge(0.035, -0.04, -0.035, -0.04)
geometry.add_edge(-0.035, 0.135, -0.035, -0.04)
geometry.add_edge(0, -0.5, 0.3, -0.5)
geometry.add_edge(0.3, -0.5, 0.3, -0.32)
geometry.add_edge(0.3, -0.32, 0, -0.32)
geometry.add_edge(0, -0.32, 0, -0.5)
geometry.add_edge(-0.5, 1, -0.5, -0.15, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.5, -0.15, -0.5, -0.2, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.5, -0.2, -0.5, -1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, 1, 0.5, -0.15, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, -0.15, 0.5, -0.2, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, -0.2, 0.5, -1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, -0.2, -0.5, -0.2)
geometry.add_edge(0.5, -0.15, -0.5, -0.15)

# labels
geometry.add_label(-0.0959509, 0.445344, 0.001, materials = {"magnetic" : "Fe"})
geometry.add_label(0.00301448, 0.0404858, 0.005, materials = {"magnetic" : "Cu"})
geometry.add_label(-0.145434, -0.706253, 0.01, materials = {"magnetic" : "Air"})
geometry.add_label(0.143596, -0.364811, 0, materials = {"magnetic" : "Magnet"})
geometry.add_label(0.310203, 0.631164, 0, materials = {"magnetic" : "Air"})
geometry.add_label(-0.295858, -0.182894, 0, materials = {"magnetic" : "Velocity"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(0.018895, -0.173495)
testA = agros2d.test("Magnetic potential", point["Ar"], 0.002978)
testB = agros2d.test("Flux density", point["Br"], 0.013047)
testBx = agros2d.test("Flux density - x", point["Brx"], 0.008413)
testBy = agros2d.test("Flux density - y", point["Bry"], -0.009972)
testH = agros2d.test("Magnetic intensity", point["Hr"], 10382.294319)
testHx = agros2d.test("Magnetic intensity - x", point["Hrx"], 6695.236126)
testHy = agros2d.test("Magnetic intensity - y", point["Hry"], -7935.102302)
testwm = agros2d.test("Energy density", point["wm"], 67.727733)
testpj = agros2d.test("Losses density ", point["pj"], 3977.255622)
testJiv = agros2d.test("Current density - induced velocity", point["Jivr"], -1.987738e5)
testJr = agros2d.test("Current density - total", point["Jr"], -1.987738e5)
testFx = agros2d.test("Lorentz force - x", point["Flx"], -1975.551246)
testFy = agros2d.test("Lorentz force - y", point["Fly"], -1671.99571)

# volume integral
volume = magnetic.volume_integrals([5])
testWm = agros2d.test("Energy", volume["Wm"], 3.088946)
testPj = agros2d.test("Losses", volume["Pj"], 220.022114)
testFxv = agros2d.test("Lorentz force - x", volume["Flx"], -110.011057)
testFyv = agros2d.test("Lorentz force - y", volume["Fly"], -36.62167)
testT = agros2d.test("Torque", volume["T"], 20.463818)

# surface integral
surface = magnetic.surface_integrals([2, 3, 4, 5])
# testFx = agros2d.test("Maxwell force - x", surface["Fx"], 2.531945, 0.05)
# testFy = agros2d.test("Maxwell force - y", surface["Fy"], -10.176192, 0.05)

result = str(point and testA and testB and testBx and testBy and testH and testHx and testHy
             and testJiv and testJr and testFx and testFy and testwm and testpj and testWm
             and testPj and testT and testFxv and testFyv)

elapsed_time = time() - start_time
print("Test: Magnetic steady state - planar ({0}): ".format(round(elapsed_time, 3)) + result)