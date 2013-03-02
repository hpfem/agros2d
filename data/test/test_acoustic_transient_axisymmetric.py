import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_total = 0.008
problem.time_steps = 200

# fields
# acoustic
acoustic = agros2d.field("acoustic")
acoustic.analysis_type = "transient"
acoustic.initial_condition = 0
acoustic.number_of_refinements = 2
acoustic.polynomial_order = 2
acoustic.adaptivity_type = "disabled"
acoustic.linearity_type = "linear"


# boundaries
acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 0})
acoustic.add_boundary("Source", "acoustic_pressure", {"acoustic_pressure_real" : { "expression" : "sin(2*pi*(time/(1.0/500)))" }, "acoustic_pressure_time_derivative" : { "expression" : "2*pi*(1.0/(1.0/500))*cos(2*pi*(time/(1.0/500)))" }})
acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"acoustic_impedance" : { "expression" : "345*1.25" }})
acoustic.add_boundary("Pressure", "acoustic_pressure", {"acoustic_pressure_real" : 0, "acoustic_pressure_time_derivative" : 0})


# materials
acoustic.add_material("Air", {"acoustic_density" : 1.25, "acoustic_speed" : 343})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, -0.8, 0.5, -1.05, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.5, -1.05, 0.6, -1.05, boundaries = {"acoustic" : "Source"})
geometry.add_edge(0.6, -1.05, 0.9, -0.55, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.2, -0.25, 0.9, -0.55, angle = 60, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.5, 0.15, 0.2, -0.25, angle = 60, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.5, 0.15, 0.65, 0.65, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.65, 0.65, 1.5, 1.2, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(1.5, 1.2, 0, 1.2, angle = 90, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0, 1.2, 0, 1, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0, 1, 0.2, 0.55, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.2, 0.55, 0, -0.8, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.4, 0.8, 0.5, 0.9, angle = 90, boundaries = {"acoustic" : "Pressure"})
geometry.add_edge(0.5, 0.9, 0.4, 1, angle = 90, boundaries = {"acoustic" : "Pressure"})
geometry.add_edge(0.3, 0.9, 0.4, 0.8, angle = 90, boundaries = {"acoustic" : "Pressure"})
geometry.add_edge(0.4, 1, 0.3, 0.9, angle = 90, boundaries = {"acoustic" : "Pressure"})

geometry.add_label(0.663165, 1.16386, materials = {"acoustic" : "Air"})
geometry.add_label(0.387614, 0.929226, materials = {"acoustic" : "none"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point 
point = acoustic.local_values(0.413503,0.499528)
testp = agros2d.test("Acoustic pressure", point["pr"], 0.106095)
# testSPL = agros2d.test("Acoustic sound level", point["SPL"], 77.055706)

print("Test: Acoustic - transient - axisymmetric: " + str(testp))