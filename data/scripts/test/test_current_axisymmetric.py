import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.name = "Tube"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
current = agros2d.field("current")
current.analysis_type = "steadystate"
current.number_of_refinements = 1
current.polynomial_order = 4
current.linearity_type = "linear"
current.weak_forms = "compiled"
current.nonlineartolerance = 0.001
current.nonlinearsteps = 10

current.add_boundary("Neumann", "current_inward_current_flow", {"Jn" : 0})
current.add_boundary("Ground", "current_potential", {"V" : 0})
current.add_boundary("Matched boundary", "current_impedance", {"Z0" : 1.25*343})
current.add_boundary("Voltage", "current_potential", {"V" : 10})
current.add_boundary("Inlet", "current_inward_current_flow", {"Jn" : -3e9})
current.add_material("Copper", {"gamma" : 5.7e7})

# geometry
geometry = agros2d.geometry()

# edges
geometry.add_edge(0, 0.45, 0, 0, boundaries = {"Neumann"})
geometry.add_edge(0, 0, 0.2, 0, boundaries = {"Ground"})
geometry.add_edge(0.2, 0, 0.2, 0.15, boundaries = {"Inlet"})
geometry.add_edge(0.2, 0.15, 0.35, 0.45, boundaries = {"Neumann"})
geometry.add_edge(0.35, 0.45, 0, 0.45, boundaries = {"Voltage"})

# labels
geometry.add_label(0.0933957, 0.350253, materials = {"Copper"})


geometry.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = current.local_values(0.213175, 0.25045)
testV = current.test("Scalar potential", point["V"], 5.566438)
testE = current.test("Electric field", point["E"], 32.059116)
testEr = current.test("Electric field - r", point["Er"], -11.088553)
testEz = current.test("Electric field - z", point["Ez"], -30.080408)
testJ = current.test("Current density", point["J"], 1.82737e9)
testJr = current.test("Current density - r", point["Jr"], -6.320475e8)
testJz = current.test("Current density - z", point["Jz"], -1.714583e9)
testpj = current.test("Losses", point["pj"], 5.858385e10)	

# volume integral
volume = current.volume_integrals([0])
testPj = current.test("Losses", volume["Pj"], 4.542019e9)

# surface integral
surface = current.surface_integrals([1])
testI = current.test("Current", surface["I"], -2.166256e8)

print("Test: Current field - axisymmetric: " + str(testV and testE and testEr and testEz and testJ and testJr and testJz and testpj and testI))