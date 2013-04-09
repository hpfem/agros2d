import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# fields
current = agros2d.field("current")
current.analysis_type = "steadystate"
current.number_of_refinements = 3
current.polynomial_order = 5
current.linearity_type = "linear"

current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
current.add_boundary("Zero", "current_potential", {"current_potential" : 0})
current.add_boundary("Voltage", "current_potential", {"current_potential" : 1})

current.add_material("mat 1", {"current_conductivity" : 1e7})
current.add_material("mat 2", {"current_conductivity" : 1e5})
current.add_material("mat 3", {"current_conductivity" : 1e3})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, 0, 0.6, 0, boundaries = {"current" : "Zero"})
geometry.add_edge(0, 0.8, 0, 0.5, boundaries = {"current" : "Neumann"})
geometry.add_edge(0, 0.5, 0, 0, boundaries = {"current" : "Neumann"})
geometry.add_edge(0, 0, 0.35, 0.5,)
geometry.add_edge(0.35, 0.5, 0.6, 0.5,)
geometry.add_edge(0.6, 0.8, 0.6, 0.5, boundaries = {"current" : "Neumann"})
geometry.add_edge(0.6, 0.5, 0.6, 0, boundaries = {"current" : "Neumann"})
geometry.add_edge(0, 0.5, 0.35, 0.5,)
geometry.add_edge(0, 0.8, 0.6, 0.8, boundaries = {"current" : "Voltage"})

# labels
geometry.add_label(0.3, 0.670924, materials = {"current" : "mat 1"})
geometry.add_label(0.105779, 0.364111, materials = {"current" : "mat 2"})
geometry.add_label(0.394296, 0.203668, materials = {"current" : "mat 3"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = current.local_values(0.11879, 0.346203)
testV = agros2d.test("Scalar potential", point["V"], 0.928377)
testE = agros2d.test("Electric field", point["Er"], 0.486928)
testEx = agros2d.test("Electric field - x", point["Erx"], -0.123527)
testEy = agros2d.test("Electric field - y", point["Ery"], -0.470999)
testJ = agros2d.test("Current density", point["Jrc"], 48692.830437)
testJx = agros2d.test("Current density - x", point["Jrcx"], -12352.691339)
testJy = agros2d.test("Current density - y", point["Jrcy"], -47099.923064)
testpj = agros2d.test("Losses", point["pj"], 23709.917359)

# volume integral
volume = current.volume_integrals([0, 1, 2])
testPj = agros2d.test("Losses", volume["Pj"], 10070.23937)

# surface integral
surface = current.surface_integrals([0])
testI = agros2d.test("Current", surface["Ir"], 3629.425713)

print("Test: Current field - planar: " + str(testV and testE and testEx and testEy and testJ and testJx and testJy and testpj and testI))