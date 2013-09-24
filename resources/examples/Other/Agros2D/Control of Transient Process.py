import agros2d as a2d

TEMP_COIL = 330

def time_callback(time_step):
    heat = a2d.field("heat")    
    volume = heat.volume_integrals([2])
    
    time = problem.time_steps_total()[time_step]
    avg = volume["T"] / volume["V"]
    print('time = {0} s, average coil temperature = {1} K'.format(time, avg))
    if (avg > TEMP_COIL):
        print('Average coil temperature reached {0} K'.format(TEMP_COIL))
        return False

    return True

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_total = 1500
problem.time_steps = 5
problem.time_callback = time_callback

# heat
heat = a2d.field("heat")
heat.analysis_type = "transient"
heat.matrix_solver = "mumps"
heat.transient_initial_condition = 293.15
heat.number_of_refinements = 0
heat.polynomial_order = 2
heat.adaptivity_type = "disabled"
heat.solver = "linear"

# boundaries
heat.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0, "heat_radiation_emissivity" : 0, "heat_radiation_ambient_temperature" : 0})
heat.add_boundary("Convection", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 10, "heat_convection_external_temperature" : 293, "heat_radiation_emissivity" : 0, "heat_radiation_ambient_temperature" : 0})

# materials
heat.add_material("Air", {"heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_conductivity" : 0.02, "heat_volume_heat" : 0, "heat_density" : 1.2, "heat_specific_heat" : 1000})
heat.add_material("Copper", {"heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_conductivity" : 200, "heat_volume_heat" : 2e5, "heat_density" : 8700, "heat_specific_heat" : 385})
heat.add_material("Iron", {"heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_conductivity" : 60, "heat_volume_heat" : 0, "heat_density" : 7800, "heat_specific_heat" : 460})
heat.add_material("Brass", {"heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_conductivity" : 100, "heat_volume_heat" : 0, "heat_density" : 8400, "heat_specific_heat" : 378})

# geometry
geometry = a2d.geometry
geometry.add_edge(0, 0.18, 0.035, 0.18, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.035, 0.18, 0.035, 0.03, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.035, 0.03, 0.0135, 0.03, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.0135, 0.03, 0.0135, 0.038)
geometry.add_edge(0.0135, 0.038, 0.019, 0.038)
geometry.add_edge(0.027, 0.038, 0.027, 0.172)
geometry.add_edge(0.027, 0.172, 0.008, 0.172)
geometry.add_edge(0.008, 0.172, 0.008, 0.16)
geometry.add_edge(0.008, 0.16, 0, 0.16)
geometry.add_edge(0, 0.16, 0, 0.18, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0, 0.13, 0.008, 0.13)
geometry.add_edge(0.008, 0.13, 0.008, 0.0395)
geometry.add_edge(0.008, 0, 0, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0, 0, 0, 0.13, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0.019, 0.038, 0.027, 0.038)
geometry.add_edge(0.019, 0.038, 0.019, 0.0395)
geometry.add_edge(0, 0.13, 0, 0.16, boundaries = {"heat" : "Neumann"})
geometry.add_edge(0.01, 0.041, 0.01, 0.1705)
geometry.add_edge(0.025, 0.041, 0.01, 0.041)
geometry.add_edge(0.01, 0.1705, 0.025, 0.1705)
geometry.add_edge(0.025, 0.1705, 0.025, 0.041)
geometry.add_edge(0.008, 0.0395, 0.008, 0.03)
geometry.add_edge(0.008, 0.03, 0.008, 0, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.019, 0.0395, 0.009, 0.0395)
geometry.add_edge(0.008, 0.03, 0.009, 0.03, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.009, 0.0395, 0.009, 0.03)
geometry.add_edge(0.009, 0.03, 0.0135, 0.03, boundaries = {"heat" : "Convection"})

geometry.add_label(0.0308709, 0.171031, materials = {"heat" : "Iron"})
geometry.add_label(0.00316251, 0.011224, materials = {"heat" : "Iron"})
geometry.add_label(0.0163723, 0.144289, materials = {"heat" : "Copper"})
geometry.add_label(0.00380689, 0.151055, materials = {"heat" : "Air"})
geometry.add_label(0.0112064, 0.0336487, materials = {"heat" : "Brass"})
a2d.view.zoom_best_fit()

problem.solve()