import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "unnamed"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 100

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

magnetic.add_boundary("A = 0", "magnetic_potential", {"Ar" : 0})
magnetic.add_material("Coil", {"mur" : 1, "Jer" : 1e6}) 
magnetic.add_material("Iron", {"mur" : 50, "gamma" : 5e3}) 
magnetic.add_material("Air", {"mur" : 1}) 

#newdocument("", "", "", 0, 3, "disabled", 1, 1, 50000, "", 2.5e-08, 2e-06, 0)

magnetic.add_boundary("Neumann", "magnetic_surface_current_density", {"Ar" : 0})
magnetic.add_boundary("A = 0", "magnetic_vector_potential", {"Ar" : 0})
magnetic.add_material("Air", {})   #, 0, 0, 1, 0, 0, 0, 0, 0, 0
magnetic.add_material("Copper", {})  #  1e6/4, 0, 1, 57e6, 0, 0, 0, 0, 0

# geometry
geometry = agros2d.geometry


# edges
geometry.add_edge(0, 0.002, 0, 0.000768, 0, boundaries = {""})
geometry.add_edge(0, 0.000768, 0, 0, 0, boundaries = {"Neumann"})
geometry.add_edge(0, 0, 0.000768, 0, 0, boundaries = {"Neumann"})
geometry.add_edge(0.000768, 0, 0.002, 0, 0, boundaries = {"Neumann"})
geometry.add_edge(0.002, 0, 0, 0.002, 90, boundaries = {"A = 0"})
geometry.add_edge(0.000768, 0, 0.000576, 0.000192, 90, boundaries = {"none"})
geometry.add_edge(0.000576, 0.000192, 0.000384, 0.000192, 0, boundaries = {"none"})
geometry.add_edge(0.000192, 0.000384, 0.000384, 0.000192, 90, boundaries = {"none"})
geometry.add_edge(0.000192, 0.000576, 0.000192, 0.000384, 0, boundaries = {"none"})
geometry.add_edge(0.000192, 0.000576, 0, 0.000768, 90, boundaries = {"none"})


# labels
geometry.add_label(0.000585418, 0.00126858, 0, 0, materials = {"Air"})
geometry.add_label(0.000109549, 8.6116e-05, 0, 0, materials = {"Copper"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(6.447965e-4,9.432763e-5)
testA = magnetic.test("Magnetic potential", point["A_real"], 7.574698e-9)
testB = magnetic.test("Flux density", point["B_real"], 7.107446e-6)
testBx = magnetic.test("Flux density - x", point["Bx_real"], -3.451261e-6)
testBy = magnetic.test("Flux density - y", point["By_real"], 6.213259e-6)
testH = magnetic.test("Magnetic intensity", point["H_real"], 5.655926)
testHx = magnetic.test("Magnetic intensity - x", point["Hx_real"], -2.746426)
testHy = magnetic.test("Magnetic intensity - y", point["Hy_real"], 4.944354)
testwm = magnetic.test("Energy density", point["wm"], 2.009959e-5)
testpj = magnetic.test("Losses density ", point["pj"], 63.874077)
testJe = magnetic.test("Current density - external", point["Je_real"], 2.5e5)
testJit = magnetic.test("Current density - induced transform", point["Jit_real"], -1.896608e5)
testJ = magnetic.test("Current density - total", point["J_real"], 60339.227467)

# volume integral
volume = magnetic.volume_integrals([1])
testWm = magnetic.test("Energy", volume["Wm"], 2.252801e-12)
testPj = magnetic.test("Losses", volume["Pj"], 6.475219e-6)
testIe = magnetic.test("Current - external", volume["Ie_real"], 0.062534)
testIit = magnetic.test("Current - induced transform", volume["Iit_real"], -0.054546)
testI = magnetic.test("Current - total", volume["I_real"], 0.007989)

# surface integral
# surface = magnetic.surface_integrals([0])

print("Test: Magnetic transient - planar: " + str(point and testA and testB and testBx and testBy and testH and testHx and testHy and testwm and testpj 
and testJe and testJit and testJ and testWm and testPj and testIe and testIit and testI))