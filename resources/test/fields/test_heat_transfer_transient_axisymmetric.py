import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_method_tolerance = 1
problem.time_total = 10000
problem.time_steps = 20

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.number_of_refinements = 1
heat.polynomial_order = 5
heat.solver = "linear"
heat.solver_parameters['tolerance'] = 0.001
heat.solver_parameters['steps'] = 10
heat.transient_initial_condition = 20

heat.add_boundary("Flux", "heat_heat_flux", {"heat_heat_flux" : 0})
heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_heat_transfer_coefficient" : 10, "heat_convection_external_temperature" : 20})

heat.add_material("Air", {"heat_conductivity" : 0.02, "heat_volume_heat" : 0, "heat_density" : 1.2, "heat_specific_heat" : 1000}) 
heat.add_material("Cu", {"heat_conductivity" : 200, "heat_volume_heat" : 26000, "heat_density" : 8700, "heat_specific_heat" : 385}) 
heat.add_material("Fe", {"heat_conductivity" : 60, "heat_volume_heat" : 0, "heat_density" : 7800, "heat_specific_heat" : 460}) 
heat.add_material("Brass", {"heat_conductivity" : 100, "heat_volume_heat" : 0, "heat_density" : 8400, "heat_specific_heat" : 378}) 

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, 0.18, 0.035, 0.18, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.035, 0.18, 0.035, 0.03, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.035, 0.03, 0.0135, 0.03, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.0135, 0.03, 0.0135, 0.038, 0,)
geometry.add_edge(0.0135, 0.038, 0.019, 0.038, 0,)
geometry.add_edge(0.027, 0.038, 0.027, 0.172, 0,)
geometry.add_edge(0.027, 0.172, 0.008, 0.172, 0,)
geometry.add_edge(0.008, 0.172, 0.008, 0.16, 0,)
geometry.add_edge(0.008, 0.16, 0, 0.16, 0,)
geometry.add_edge(0, 0.16, 0, 0.18, 0, boundaries = {"heat" : "Flux"})
geometry.add_edge(0, 0.13, 0.008, 0.13, 0,)
geometry.add_edge(0.008, 0.13, 0.008, 0.0395, 0,)
geometry.add_edge(0.008, 0, 0, 0, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0, 0, 0, 0.13, 0, boundaries = {"heat" : "Flux"})
geometry.add_edge(0.019, 0.038, 0.027, 0.038, 0,)
geometry.add_edge(0.019, 0.038, 0.019, 0.0395, 0,)
geometry.add_edge(0, 0.13, 0, 0.16, 0, boundaries = {"heat" : "Flux"})
geometry.add_edge(0.01, 0.041, 0.01, 0.1705, 0,)
geometry.add_edge(0.025, 0.041, 0.01, 0.041, 0,)
geometry.add_edge(0.01, 0.1705, 0.025, 0.1705, 0,)
geometry.add_edge(0.025, 0.1705, 0.025, 0.041, 0,)
geometry.add_edge(0.008, 0.0395, 0.008, 0.03, 0,)
geometry.add_edge(0.008, 0.03, 0.008, 0, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.019, 0.0395, 0.009, 0.0395, 0,)
geometry.add_edge(0.008, 0.03, 0.009, 0.03, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.009, 0.0395, 0.009, 0.03, 0,)
geometry.add_edge(0.009, 0.03, 0.0135, 0.03, 0, boundaries = {"heat" : "Convection"})

# labels
geometry.add_label(0.0308709, 0.171031, materials = {"heat" : "Fe"})
geometry.add_label(0.00316251, 0.011224, materials = {"heat" : "Fe"})
geometry.add_label(0.0163723, 0.144289, materials = {"heat" : "Cu"})
geometry.add_label(0.00380689, 0.151055, materials = {"heat" : "Air"})
geometry.add_label(0.0112064, 0.0336487, materials = {"heat" : "Brass"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(0.00503, 0.134283)
testT = agros2d.test("Temperature", point["T"], 44.012004)
testF = agros2d.test("Heat flux", point["F"], 16.739787)
testFr = agros2d.test("Heat flux - r", point["Fr"], -7.697043)
testFz = agros2d.test("Heat flux - z", point["Fz"], -14.865261)

# volume integral
volume = heat.volume_integrals([3])
testTavg = agros2d.test("Temperature", volume["T"], 0.002839)

# surface integral
surface = heat.surface_integrals([26])
testFlux = agros2d.test("Heat flux", surface["f"], 0.032866, error = 0.05)

elapsed_time = time() - start_time
print("Test: Heat transfer - transient - axisymmetric ({0}): ".format(round(elapsed_time, 3)) + str(testT and testF and testFr and testFz and testTavg and testFlux))