import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
# electrostatic
electrostatic = agros2d.field("electrostatic")
electrostatic.analysis_type = "steadystate"
electrostatic.polynomial_order = 2

electrostatic.adaptivity_type = "hp-adaptivity"
electrostatic.adaptivity_parameters['steps'] = 10
electrostatic.adaptivity_parameters['tolerance'] = 1
electrostatic.solver = "linear"

# boundaries
electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
electrostatic.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})


# materials
electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0.2, 1, 0, 0.5, boundaries = {"electrostatic" : "Source"})
geometry.add_edge(0, 0.5, 0, 0.25, boundaries = {"electrostatic" : "Border"})
geometry.add_edge(0, -0.25, 0, -1, boundaries = {"electrostatic" : "Border"})
geometry.add_edge(0, -1, 1.5, 0.5, angle = 90, boundaries = {"electrostatic" : "Border"})
geometry.add_edge(1.5, 0.5, 0, 2, angle = 90, boundaries = {"electrostatic" : "Border"})
geometry.add_edge(0, 1, 0.2, 1, boundaries = {"electrostatic" : "Source"})
geometry.add_edge(0, 2, 0, 1, boundaries = {"electrostatic" : "Border"})
geometry.add_edge(0, -0.25, 0.25, 0, angle = 90, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"electrostatic" : "Ground"})

geometry.add_label(0.879551, 0.764057, area = 0.06, materials = {"electrostatic" : "Air"})
agros2d.view.zoom_best_fit()

problem.solve()

# exact values in this test are taken from Agros -> not a proper test
# only to see if adaptivity works, should be replaced with comsol values
point = electrostatic.local_values(3.278e-2, 4.624e-1)
test = agros2d.test("Electrostatic potential", point["V"], 5.569e2)

print("Test: Adaptivity - electrostatic: " + str(test))