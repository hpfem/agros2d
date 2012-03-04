# model
newdocument("Electrostatic Osciloscope - particle tracing", "planar", "electrostatic", 2, 2, "disabled", 1, 1, 0, "steadystate", 1.0, 1.0, 0.0)

# model
newdocument("Electrostatic Osciloscope", "planar", "electrostatic", 2, 2, "disabled", 1, 1, 0, "steadystate", 1.0, 1.0, 0.0)

# boundaries
addboundary("Voltage", "electrostatic_potential", 2e3)
addboundary("Ground", "electrostatic_potential", 0)
addboundary("Neumann", "electrostatic_surface_charge_density", 0)

# materials
addmaterial("Air", 0, 1)
addmaterial("Screen", 0, 2)

# edges
addedge(0.075, -0.022, 0.075, 0.022, 0, "none")
addedge(0.075, -0.022, 0.085, -0.022, 0, "Neumann")
addedge(0.085, -0.022, 0.085, 0.022, 0, "none")
addedge(0.085, 0.022, 0.075, 0.022, 0, "none")
addedge(0, 0.06, 0, -0.06, 0, "Neumann")
addedge(0, -0.06, 0.118, -0.06, 0, "Neumann")
addedge(0.118, -0.06, 0.118, 0.06, 0, "Neumann")
addedge(0.118, 0.06, 0, 0.06, 0, "Neumann")
addedge(0.05, 0.01, 0.04, 0.01, 0, "Voltage")
addedge(0.04, 0.011, 0.04, 0.01, 0, "Voltage")
addedge(0.04, 0.011, 0.05, 0.011, 0, "Voltage")
addedge(0.05, 0.011, 0.05, 0.01, 0, "Voltage")
addedge(0.05, -0.01, 0.05, -0.011, 0, "Ground")
addedge(0.05, -0.011, 0.04, -0.011, 0, "Ground")
addedge(0.04, -0.01, 0.04, -0.011, 0, "Ground")
addedge(0.04, -0.01, 0.05, -0.01, 0, "Ground")

# labels
addlabel(0.0771576, 0.0174291, 0, 0, "Screen")
addlabel(0.0460776, 0.0105177, 0, 0, "none")
addlabel(0.045095, -0.0105089, 0, 0, "none")
addlabel(0.109507, 0.050865, 0, 0, "Air")

zoombestfit()
solve()

tracing = ParticleTracing()
tracing.set_initial_position(0.015, 0.0)
tracing.set_initial_velocity(3e7, 0.0)
tracing.mass = 9.109e-31
tracing.charge = -1.602e-19
tracing.drag_force_density = 1.2041
tracing.drag_force_reference_area = 0.0
tracing.drag_force_coefficient = 0.0
tracing.maximum_steps = 2000
tracing.tolerance = 1e-8

tracing.solve()
showparticletracing(True)

"""
ix, iy = tracing.initial_position()
ivx, ivy = velocity = tracing.initial_velocity()
print("initial position: x = {0} m, y = {1} m".format(ix, iy))
print("initial velocity: vx = {0} m/s, vy = {1} m/s".format(ivx, ivy))
print("mass = {0} kg".format(tracing.mass))
print("charge  {0} C".format(tracing.charge))
print("drag force: rho = {0} kg/m3, area = {1} m2, coeff = {2}".format(tracing.drag_force_density, tracing.drag_force_reference_area, tracing.drag_force_coefficient))
print("tolerance: steps = {0}, tolerance = {1}".format(tracing.maximum_steps, tracing.tolerance))

x, y, z = tracing.positions()
vx, vy, vz = tracing.velocities()

print("end position and velocity")
print("x", x[-1], "y", y[-1], "z", z[-1])
print("vx", vx[-1], "vy", vy[-1], "vz", vz[-1])
"""

# test
testStopY = test("end y", y[-1], 0.015709)

print("Test: Electrostatic tracing - planar: " + str(testStopY))
