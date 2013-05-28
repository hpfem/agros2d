import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_total = 0.8
problem.time_steps = 20

# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "transient"
magnetic.initial_condition = 0
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 3
magnetic.adaptivity_type = "disabled"
magnetic.linearity_type = "linear"

# boundaries
magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})


# materials
magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "1e7*(exp(-10/0.7*time) - exp(-12/0.7*time))" }})
magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 5.7e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, -0.5, 0.5, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, 0, 0, 0.5, angle = 90, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.5, 0, 0.15, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.15, 0, -0.15, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, -0.15, 0, -0.5, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.05, 0.15, 0, 0.15)
geometry.add_edge(0.05, 0.15, 0.1, -0.15)
geometry.add_edge(0.1, -0.15, 0, -0.15)
geometry.add_edge(0.15, -0.05, 0.25, -0.05)
geometry.add_edge(0.25, 0.05, 0.15, 0.05)
geometry.add_edge(0.15, 0.05, 0.15, -0.05)
geometry.add_edge(0.25, 0.05, 0.25, -0.05)

geometry.add_label(0.19253, -0.00337953, materials = {"magnetic" : "Coil"})
geometry.add_label(0.051315, -0.0143629, materials = {"magnetic" : "Copper"})
geometry.add_label(0.165856, 0.213151, materials = {"magnetic" : "Air"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(6.447965e-4, 9.432763e-5)
testA = agros2d.test("Magnetic potential", point["Ar"], 7.574698e-9)
testB = agros2d.test("Flux density", point["Br"], 7.107446e-6)
testBx = agros2d.test("Flux density - x", point["Brr"], -3.451261e-6)
testBy = agros2d.test("Flux density - y", point["Brz"], 6.213259e-6)
testH = agros2d.test("Magnetic intensity", point["Hr"], 5.655926)
testHx = agros2d.test("Magnetic intensity - x", point["Hrr"], -2.746426)
testHy = agros2d.test("Magnetic intensity - y", point["Hrz"], 4.944354)
testwm = agros2d.test("Energy density", point["wm"], 2.009959e-5)
# testpj = agros2d.test("Losses density ", point["pj"], 63.874077)
testpj = True
testJer = agros2d.test("Current density - external", point["Je"], 2.5e5)
# testJit = agros2d.test("Current density - induced transform", point["Jit"], -1.896608e5)
testJitr = True
# testJ = agros2d.test("Current density - total", point["Jr"], 60339.227467)
testJr = True

# volume integral
volume = magnetic.volume_integrals([1])
# testWm = agros2d.test("Energy", volume["Wm"], 2.252801e-12)
testWm = True
# testPj = agros2d.test("Losses", volume["Pj"], 6.475219e-6)
testPj = True
testIer = agros2d.test("Current - external", volume["Ier"], 0.062534)
# testIit = agros2d.test("Current - induced transform", volume["Iitr"], -0.054546)
testIitr = True
# testI = agros2d.test("Current - total", volume["Ir"], 0.007989)
testIr = True

# surface integral
# surface = magnetic.surface_integrals([0])

print("Test: Magnetic transient - planar: " + str(point and testA and testB and testBx and testBy and testH and testHx and testHy and testwm and testpj 
and testJer and testJitr and testJr and testWm and testPj and testIer and testIitr and testIr))
