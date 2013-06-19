import agros2d as a2d

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

# fields
# electrostatic
electrostatic = a2d.field("electrostatic")
electrostatic.analysis_type = "steadystate"
electrostatic.number_of_refinements = 0
electrostatic.polynomial_order = 1
electrostatic.adaptivity_type = "hp-adaptivity"
electrostatic.adaptivity_steps = 4
electrostatic.adaptivity_tolerance = 0.1
electrostatic.linearity_type = "newton"
electrostatic.nonlinear_tolerance = 0.001
electrostatic.nonlinear_steps = 10
electrostatic.nonlinear_convergence_measurement = "residual_norm_absolute"
electrostatic.damping_type = "automatic"
electrostatic.damping_coeff = 1
electrostatic.reuse_jacobian = True

# variables
r1 = 0.01
r2 = 0.03
r3 = 0.05
r4 = 0.055
l = 0.2
eps1 = 10
eps2 = 3
U = 10
dr = 0.003

# boundaries
electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 10})
electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
electrostatic.add_boundary("Neumann BC", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})

# materials
electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
electrostatic.add_material("Dielectric n.1", {"electrostatic_permittivity" : 10, "electrostatic_charge_density" : 0})
electrostatic.add_material("Dielectric n.2", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 0})

# edges
geometry = a2d.geometry
geometry.add_edge(0, 3/2*l, 0, l/2,  boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r1, l/2, r1, 0, boundaries = {"electrostatic" : "Source"})
geometry.add_edge(r1, 0, r2, 0,  boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r2, 0, r2, l/2)
geometry.add_edge(r2, l/2, r3, l/2)
geometry.add_edge(r3, 0, r2, 0,  boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r3, l/2, r3, 0, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(r4, 0, r4, l/2, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(r3, l/2, r4, l/2, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(r4, 0, 3/2*l, 0, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(3/2*l, 0, 0, 3/2*l, angle=90, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r1, l/2, r2, l/2)
geometry.add_edge(r1, l/2, 0, l/2, boundaries = {"electrostatic" : "Source"})

# labels
geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric n.1"})
geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric n.2"})
geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})
a2d.view.zoom_best_fit()

N = 4
for i in range(N):
	
	# solve one adaptive step
	problem.solve_adaptive_step()	

	# save order
	a2d.view.mesh.activate()	
	a2d.view.post2d.scalar = False
	a2d.view.mesh.initial_mesh = True
	a2d.view.mesh.solution_mesh = True
	a2d.view.save_image("electrostatics_mesh_%(#)02d.png" % {"#": i})

	# save scalar view
	a2d.view.post2d.activate()
	a2d.view.post2d.scalar_variable = 'electrostatic_potential'
	a2d.view.post2d.scalar = True
	a2d.view.mesh.initial_mesh = False
	a2d.view.mesh.solution_mesh = False
	a2d.view.save_image("electrostatics_potential_%(#)02d.png" % {"#": i})

