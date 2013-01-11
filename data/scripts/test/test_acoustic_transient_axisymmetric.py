import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.name = "unnamed"
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_method_tolerance = 0.1
problem.time_total = 1
problem.time_steps = 10

# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# fields
# acoustic
acoustic = agros2d.field("acoustic")
acoustic.analysis_type = "transient"
acoustic.initial_condition = 0
acoustic.number_of_refinements = 1
acoustic.polynomial_order = 2
acoustic.linearity_type = "linear"
acoustic.adaptivity_type = "disabled"

# boundaries
acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 0})
acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"acoustic_impedance" : 428.75})
acoustic.add_boundary("Source", "acoustic_pressure", {"acoustic_pressure_real" : { "expression" : "sin(2*pi*(time/(1.0/1000)))" }, "acoustic_pressure_time_derivative" : { "expression" : "2*pi*(1.0/(1.0/1000)) * cos(2*pi*(time/(1.0/1000)))" }})

# materials
acoustic.add_material("Air", {"acoustic_density" : 1.25, "acoustic_speed" : 343})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, 1.25, 1.05, 1.25, refinements = {}, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.25, 0, 1.05, 1.25, angle = 90, refinements = {}, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.25, 0, 0, 0, refinements = {}, boundaries = {"acoustic" : "Source"})
geometry.add_edge(0, 0, 0, 0.5, refinements = {}, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0, 0.8, 0, 1.25, refinements = {}, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0, 0.5, 0.15, 0.65, angle = 90, refinements = {}, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.15, 0.65, 0, 0.8, angle = 90, refinements = {}, boundaries = {"acoustic" : "Wall"})
geometry.add_edge(0.35, 0.85, 0.7, 0.75, refinements = {}, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.7, 0.75, 0.45, 0.5, refinements = {}, boundaries = {"acoustic" : "Matched boundary"})
geometry.add_edge(0.35, 0.85, 0.45, 0.5, refinements = {}, boundaries = {"acoustic" : "Matched boundary"})

geometry.add_label(0.163662, 0.383133, materials = {"acoustic" : "Air"})
geometry.add_label(0.534868, 0.726801, materials = {"acoustic" : "none"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point 
point = acoustic.local_values(0.042132, -0.072959)
testp = agros2d.test("Acoustic pressure", point["pr"], 0.200436)
# testSPL = agros2d.test("Acoustic sound level", point["SPL"], 77.055706)

print("Test: Acoustic - transient - planar: " + str(testp))