import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "Feeder"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
current = agros2d.field("current")
current.analysis_type = "steadystate"
current.number_of_refinements = 3
current.polynomial_order = 5
current.linearity_type = "linear"
current.weak_forms = "compiled"
current.nonlineartolerance = 0.001
current.nonlinearsteps = 10

current.add_boundary("Neumann", "current_inward_current_flow", {"Jn" : 0})
current.add_boundary("Zero", "current_potential", {"V" : 0})
current.add_boundary("Voltage", "current_potential", {"V" : 1})

current.add_material("mat 1", {"gamma" : 1e7})
current.add_material("mat 2", {"gamma" : 1e5})
current.add_material("mat 3", {"gamma" : 1e3})

# geometry
geometry = agros2d.geometry()

# edges
geometry.add_edge(0, 0, 0.6, 0, boundaries = {"Zero"})
geometry.add_edge(0, 0.8, 0, 0.5, boundaries = {"Neumann"})
geometry.add_edge(0, 0.5, 0, 0, boundaries = {"Neumann"})
geometry.add_edge(0, 0, 0.35, 0.5,)
geometry.add_edge(0.35, 0.5, 0.6, 0.5,)
geometry.add_edge(0.6, 0.8, 0.6, 0.5, boundaries = {"Neumann"})
geometry.add_edge(0.6, 0.5, 0.6, 0, boundaries = {"Neumann"})
geometry.add_edge(0, 0.5, 0.35, 0.5,)
geometry.add_edge(0, 0.8, 0.6, 0.8, boundaries = {"Voltage"})

# labels
geometry.add_label(0.3, 0.670924, materials = {"mat 1"})
geometry.add_label(0.105779, 0.364111, materials = {"mat 2"})
geometry.add_label(0.394296, 0.203668, materials = {"mat 3"})

geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = current.local_values(0.11879, 0.346203)
testV = current.test("Scalar potential", point["V"], 0.928377)
testE = current.test("Electric field", point["E"], 0.486928)
testEx = current.test("Electric field - x", point["Ex"], -0.123527)
testEy = current.test("Electric field - y", point["Ey"], -0.470999)
testJ = current.test("Current density", point["J"], 48692.830437)
testJx = current.test("Current density - x", point["Jx"], -12352.691339)
testJy = current.test("Current density - y", point["Jy"], -47099.923064)
testpj = current.test("Losses", point["pj"], 23709.917359)

# volume integral
volume = current.volume_integrals([0, 1, 2])
testPj = current.test("Losses", volume["Pj"], 10070.23937)

# surface integral
surface = current.surface_integrals([0])
testI = current.test("Current", surface["I"], 3629.425713)

print("Test: Current field - planar: " + str(testV and testE and testEx and testEy and testJ and testJx and testJy and testpj and testI))