import agros2d

U = 1e3

problem = agros2d.Problem("Test", "planar")

electrostatic = agros2d.Field("electrostatic", "steadystate")
electrostatic.add_boundary("Source electrode", "electrostatic_potential", {"electrostatic_potential" : U})
electrostatic.add_material("Dieletric (electrostatic)", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})

heat = agros2d.Field("heat", "transient")
heat.add_boundary("Convection", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_heat_transfer_coefficient" : 5, "external_temperature": 2})
heat.add_material("Dielectric (heat)", {"heat_volume_heat" : 5e3, "heat_density" : 3600, "heat_conductivity" : 30, "heat_specific_heat" : 450, })

geometry = agros2d.Geometry()
geometry.add_node(0, 1)
geometry.add_edge(0, 0, 1, 1, 90, 2, {"electrostatic" : "Source electrode", "heat" : "Convection"})
geometry.add_label(.5, .5, materials = {"electrostatic" : "Dieletric (electrostatic)"})