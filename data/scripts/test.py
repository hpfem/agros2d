U = 1e3

problem = agros2d.Problem("Test", "planar")

electrostatic = agros2d.Field("electrostatic", "steadystate")
#electrostatic.add_boundary("Source electrode", "electrostatic_potential", {"electrostatic_potential" : U})

heat = agros2d.Field("heat", "transient")
#heat.add_boundary("Convection", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_heat_transfer_coefficient" : 5, "external_temperature": 2})

geometry = agros2d.Geometry()
geometry.add_edge(0, 0, 1, 1, 90, 2)