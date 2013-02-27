import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# fields
# electrostatic
electrostatic = agros2d.field("electrostatic")
electrostatic.analysis_type = "steadystate"
electrostatic.number_of_refinements = 1
electrostatic.polynomial_order = 2
electrostatic.adaptivity_type = "disabled"
electrostatic.linearity_type = "linear"


# boundaries
electrostatic.add_boundary("Voltage", "electrostatic_potential", {"electrostatic_potential" : 1e4})
electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})


# materials
electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
electrostatic.add_material("Screen", {"electrostatic_permittivity" : 2, "electrostatic_charge_density" : 0})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0.075, -0.022, 0.075, 0.022, angle = 90)
geometry.add_edge(0.075, -0.022, 0.079, -0.026)
geometry.add_edge(0.079, -0.026, 0.079, 0.026, angle = 90)
geometry.add_edge(0.079, 0.026, 0.075, 0.022)
geometry.add_edge(0, 0.06, 0, -0.06, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0, -0.06, 0.118, -0.06, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0.118, -0.06, 0.118, 0.06, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0.118, 0.06, 0, 0.06, boundaries = {"electrostatic" : "Neumann"})
geometry.add_edge(0.062, 0.01, 0.04, 0.01, boundaries = {"electrostatic" : "Voltage"})
geometry.add_edge(0.04, 0.017, 0.04, 0.01, boundaries = {"electrostatic" : "Voltage"})
geometry.add_edge(0.04, 0.017, 0.062, 0.017, boundaries = {"electrostatic" : "Voltage"})
geometry.add_edge(0.062, 0.017, 0.062, 0.01, boundaries = {"electrostatic" : "Voltage"})
geometry.add_edge(0.05, -0.01, 0.05, -0.017, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(0.05, -0.017, 0.04, -0.017, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(0.04, -0.01, 0.04, -0.017, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(0.04, -0.01, 0.05, -0.01, boundaries = {"electrostatic" : "Ground"})

geometry.add_label(0.0805206, 0.0216256, materials = {"electrostatic" : "Screen"})
geometry.add_label(0.0504445, 0.0129438, materials = {"electrostatic" : "none"})
geometry.add_label(0.0452163, -0.0148758, materials = {"electrostatic" : "none"})
geometry.add_label(0.109507, 0.050865, materials = {"electrostatic" : "Air"})
geometry.zoom_best_fit()

problem.solve()

tracing = agros2d.particle_tracing
tracing.drag_force_density = 1.2041
tracing.drag_force_coefficient = 0
tracing.drag_force_reference_area = 1e-06
tracing.mass = 9.109e-31
tracing.charge = -1.602e-19

tracing.reflect_on_different_material = True
tracing.reflect_on_boundary = False
tracing.coefficient_of_restitution = 0

tracing.maximum_number_of_steps = 1e3
tracing.tolerance = 0.0001
tracing.minimum_step = 0.0003

tracing.initial_position = (0.01, 0.0)
tracing.initial_velocity = (8e7, 0)

tracing.solve()
x, y, z = tracing.positions()

testPositionX = agros2d.test("Particle position", x[-1], 0.080043)
testPositionY = agros2d.test("Particle position", y[-1], 0.015374)

print("Test: Particle tracing planar: " + str(testPositionX and testPositionY))