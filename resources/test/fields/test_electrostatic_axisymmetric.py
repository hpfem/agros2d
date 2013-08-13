import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
electrostatic = agros2d.field("electrostatic")
electrostatic.analysis_type = "steadystate"
electrostatic.number_of_refinements = 1
electrostatic.polynomial_order = 5
electrostatic.solver = "linear"

electrostatic.add_boundary("Source electrode", "electrostatic_potential", {"electrostatic_potential" : 10})
electrostatic.add_boundary("Ground electrode", "electrostatic_potential", {"electrostatic_potential" : 0})
electrostatic.add_boundary("Neumann BC", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})

electrostatic.add_material("Air", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 1})
electrostatic.add_material("Dielectric n.1", {"electrostatic_charge_density" : 1e-5, "electrostatic_permittivity" : 10})
electrostatic.add_material("Dielectric n.2", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 3})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, 0.2, 0, 0.08, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(0.01, 0.08, 0.01, 0, boundaries = {"electrostatic" : "Source electrode"})
geometry.add_edge(0.01, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(0.03, 0, 0.03, 0.08)
geometry.add_edge(0.03, 0.08, 0.05, 0.08)
geometry.add_edge(0.05, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(0.05, 0.08, 0.05, 0, boundaries = {"electrostatic" : "Ground electrode"})
geometry.add_edge(0.06, 0, 0.06, 0.08, boundaries = {"electrostatic" : "Ground electrode"})
geometry.add_edge(0.05, 0.08, 0.06, 0.08, boundaries = {"electrostatic" : "Ground electrode"})
geometry.add_edge(0.06, 0, 0.2, 0, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(0.01, 0.08, 0.03, 0.08)
geometry.add_edge(0.01, 0.08, 0, 0.08, boundaries = {"electrostatic" : "Source electrode"})
geometry.add_edge(0.2, 0, 0, 0.2, boundaries = {"electrostatic" : "Neumann BC"}, angle=90)

# labels
geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric n.1"})
geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric n.2"})
geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})

# solve
agros2d.view.zoom_best_fit()
problem.solve()

# point value
point = electrostatic.local_values(0.0255872, 0.0738211)

testV = agros2d.test("Scalar potential", point["V"], 25.89593)
testE = agros2d.test("Electric field", point["E"], 151.108324)
testEr = agros2d.test("Electric field - r", point["Er"], 94.939342)
testEz = agros2d.test("Electric field - z", point["Ez"], 117.559546)
testD = agros2d.test("Displacement", point["D"], 1.337941e-8)
testDr = agros2d.test("Displacement - r", point["Dr"], 8.406108e-9)
testDz = agros2d.test("Displacement - z", point["Dz"], 1.040894e-8)
testwe = agros2d.test("Energy density", point["we"], 1.01087e-6)

# volume integral
volume = electrostatic.volume_integrals([0, 1, 2])
testEnergy = agros2d.test("Energy", volume["We"], 1.799349e-8)

# surface integral
surface = electrostatic.surface_integrals([1, 12])
testQ = agros2d.test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: Electrostatic - axisymmetric: " + str(testV and testE and testEr and testEz and testD and testDr and testDz and testwe and testEnergy and testQ))