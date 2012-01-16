import agros2d

U = 1e3
J = 3e7
Q = 1e4

problem = agros2d.Problem("planar", name = "Test", time_step = 10, time_total = 60)

electrostatic = agros2d.Field("electrostatic", "steadystate")

electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
electrostatic.add_boundary("Dirichlet", "electrostatic_surface_charge_density")
electrostatic.set_boundary("Dirichlet", "electrostatic_potential", {"electrostatic_potential" : 1.4*U})
electrostatic.set_boundary("Dirichlet", parameters = {"electrostatic_potential" : U/2.0})

electrostatic.add_boundary("Test", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
electrostatic.remove_boundary("Test")

electrostatic.add_material("Dieletric", {"electrostatic_permittivity" : 7, "electrostatic_charge_density" : 0})
electrostatic.add_material("Conductor", {"electrostatic_permittivity" : 15, "electrostatic_charge_density" : J})
electrostatic.set_material("Conductor", {"electrostatic_permittivity" : 5})

heat = agros2d.Field("heat", "transient")

heat.add_boundary("Dirichlet", "heat_temperature", {"heat_temperature" : 20})
heat.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_heat_transfer_coefficient" : 5, "external_temperature": 20})
heat.set_boundary("Neumann", parameters = {"heat_heat_transfer_coefficient" : 2})

heat.add_material("Conductor", {"heat_volume_heat" : Q, "heat_density" : 8700, "heat_conductivity" : 385, "heat_specific_heat" : 430})
heat.set_material("Conductor", {"heat_volume_heat" : Q/3.0})

geometry = agros2d.Geometry()
geometry.add_node(-1, -1)
geometry.add_node(0, 0)
geometry.remove_node(0)

geometry.add_edge(0, 0, 1, 0, boundaries = {"electrostatic" : "Dirichlet", "heat" : "Dirichlet"})
geometry.add_edge(1, 0, 1, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(1, 1, 0, 1, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, 1, 0, 0, boundaries = {"electrostatic" : "Dirichlet", "heat" : "Dirichlet"})
geometry.add_edge(0, 1, 1, 0, angle = 45, refinement = 2, boundaries = {"heat" : "Neumann"})
geometry.add_label(.25, .25, materials = {"electrostatic" : "Conductor", "heat" : "Conductor"})
geometry.add_label(.75, .75, materials = {"electrostatic" : "Dieletric"})

geometry.select_nodes([0])
geometry.select_nodes()
geometry.select_edges([1, 2, 3])
geometry.select_edges()
geometry.select_edges([1])
geometry.select_edges()
geometry.select_none()

geometry.zoom_best_fit()

#geometry.select_nodes()
#geometry.move_selection(1.5, 1.5, True)
#geometry.rotate_selection(0.5, 0.5, 90)
#geometry.scale_selection(0.5, 0.5, 2)

#geometry.select_labels()
#geometry.remove_selection()

geometry.mesh()