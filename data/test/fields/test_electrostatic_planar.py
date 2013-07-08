import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
electrostatic = agros2d.field("electrostatic")
electrostatic.analysis_type = "steadystate"
electrostatic.number_of_refinements = 2
electrostatic.polynomial_order = 3
electrostatic.solver = "linear"

electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
electrostatic.add_boundary("U = 0 V", "electrostatic_potential", {"electrostatic_potential" : 0})
electrostatic.add_boundary("U = 1000 V", "electrostatic_potential", {"electrostatic_potential" : 1000})

electrostatic.add_material("Dieletric", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 3})
electrostatic.add_material("Air", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 1})
electrostatic.add_material("Source", {"electrostatic_charge_density" : 4e-10, "electrostatic_permittivity" : 10})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(1, 2, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(4, 1, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(1, 2, 4, 2, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(4, 2, 4, 1, boundaries = {"electrostatic" : "U = 1000 V"})
geometry.add_edge(20, 24, 20, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(20, 1, 20, 0, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(4, 1, 20, 1, boundaries = {})
geometry.add_edge(0, 24, 0, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, 0, 0, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, 0, 20, 0, boundaries = {"electrostatic" : "U = 0 V"})
geometry.add_edge(0, 24, 20, 24, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, 1, 1, 1, boundaries = {})
geometry.add_edge(7, 13, 14, 13, boundaries = {})
geometry.add_edge(14, 13, 14, 18, boundaries = {})
geometry.add_edge(14, 18, 7, 18, boundaries = {})
geometry.add_edge(7, 18, 7, 13, boundaries = {})

# labels
geometry.add_label(2.78257, 1.37346, materials = {"electrostatic" : "none"})
geometry.add_label(10.3839, 15.7187, materials = {"electrostatic" : "Source"})
geometry.add_label(3.37832, 15.8626, materials = {"electrostatic" : "Air"})
geometry.add_label(12.3992, 0.556005, materials = {"electrostatic" : "Dieletric"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
local_values = electrostatic.local_values(13.257584, 11.117738)
testV = agros2d.test("Scalar potential", local_values["V"], 1111.544825)
testE = agros2d.test("Electric field", local_values["E"], 111.954358)
testEx = agros2d.test("Electric field - x", local_values["Ex"], 24.659054)
testEy = agros2d.test("Electric field - y", local_values["Ey"], -109.204896)
testD = agros2d.test("Displacement", local_values["D"], 9.912649e-10)
testDx = agros2d.test("Displacement - x", local_values["Dx"], 2.183359e-10)
testDy = agros2d.test("Displacement - y", local_values["Dy"], -9.669207e-10)
testwe = agros2d.test("Energy density", local_values["we"], 5.548821e-8)

# surface integral
surface_integrals = electrostatic.surface_integrals([0, 1, 2, 3])
testQ = agros2d.test("Electric charge", surface_integrals["Q"], 1.048981e-7)

# volume integral
volume_integrals = electrostatic.volume_integrals([1])
testEnergy = agros2d.test("Energy", volume_integrals["We"], 1.307484e-7)

elapsed_time = time() - start_time
print("Test: Electrostatic - planar ({0}): ".format(round(elapsed_time, 3)) + str(testV and testE and testEx and testEy and testD and testDx and testDy and testwe and testEnergy and testQ))