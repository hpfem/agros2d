import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.time_step_method = "fixed"
problem.time_method_order = 2
problem.time_total = 2e-06
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
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 3
magnetic.linearity_type = "linear"
magnetic.nonlinear_tolerance = 0.001
magnetic.nonlinear_steps = 10

# boundaries
magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
magnetic.add_boundary("Neumann", "magnetic_surface_current", {"magnetic_surface_current_real" : 0})

# materials
magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_conductivity" : 57e6, "magnetic_current_density_external_real" : { "expression" : "5e6*sin(2*pi*1.0/1e-06*time)*(time<1e-6)" }})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, 0.002, 0, 0.000768, boundaries = {"magnetic" : "Neumann"})
geometry.add_edge(0, 0.000768, 0, 0, boundaries = {"magnetic" : "Neumann"})
geometry.add_edge(0, 0, 0.000768, 0, boundaries = {"magnetic" : "Neumann"})
geometry.add_edge(0.000768, 0, 0.002, 0, boundaries = {"magnetic" : "Neumann"})
geometry.add_edge(0.002, 0, 0, 0.002, 90, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.000768, 0, 0.000576, 0.000192, 90)
geometry.add_edge(0.000576, 0.000192, 0.000384, 0.000192)
geometry.add_edge(0.000192, 0.000384, 0.000384, 0.000192, 90)
geometry.add_edge(0.000192, 0.000576, 0.000192, 0.000384)
geometry.add_edge(0.000192, 0.000576, 0, 0.000768, 90)

# labels
geometry.add_label(0.000585418, 0.00126858, 0, 0, materials = {"magnetic" : "Air"})
geometry.add_label(0.000109549, 8.6116e-05, 0, 0, materials = {"magnetic" : "Cu"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(6.447965e-4,9.432763e-5)
testA = agros2d.test("Magnetic potential", point["Ar"], 7.574698e-9)
testB = agros2d.test("Flux density", point["Br"], 7.107446e-6)
testBx = agros2d.test("Flux density - x", point["Brx"], -3.451261e-6)
testBy = agros2d.test("Flux density - y", point["Bry"], 6.213259e-6)
testH = agros2d.test("Magnetic intensity", point["Hr"], 5.655926)
testHx = agros2d.test("Magnetic intensity - x", point["Hrx"], -2.746426)
testHy = agros2d.test("Magnetic intensity - y", point["Hry"], 4.944354)
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
