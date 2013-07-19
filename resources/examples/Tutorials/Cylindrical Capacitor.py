import pythonlab
import pylab as pl
import agros2d as a2d

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"


# fields
# electrostatic
electrostatic = a2d.field("electrostatic")
electrostatic.analysis_type = "steadystate"
electrostatic.number_of_refinements = 1
electrostatic.polynomial_order = 2
electrostatic.adaptivity_type = "disabled"
electrostatic.solver = "linear"

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

# geometry
geometry = a2d.geometry

# edges
geometry = a2d.geometry
geometry.add_edge(0, 1.5*l, 0, l/2,  boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r1, l/2, r1, 0, boundaries = {"electrostatic" : "Source"})
geometry.add_edge(r1, 0, r2, 0,  boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r2, 0, r2, l/2)
geometry.add_edge(r2, l/2, r3, l/2)
geometry.add_edge(r3, 0, r2, 0,  boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r3, l/2, r3, 0, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(r4, 0, r4, l/2, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(r3, l/2, r4, l/2, boundaries = {"electrostatic" : "Ground"})
geometry.add_edge(r4, 0, 1.5*l, 0, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(1.5*l, 0, 0, 1.5*l, angle=90, boundaries = {"electrostatic" : "Neumann BC"})
geometry.add_edge(r1, l/2, r2, l/2)
geometry.add_edge(r1, l/2, 0, l/2, boundaries = {"electrostatic" : "Source"})

geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric n.1"})
geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric n.2"})
geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})
a2d.view.zoom_best_fit()

# calculation of capacity
r = []
C = []
print("C = f(r) (F):")
for i in range(15):
	if i > 0:
		geometry.select_edges([6, 7, 8])
		geometry.move_selection(dr, 0, False)

	problem.solve()
	result = electrostatic.volume_integrals([0, 1, 2])
	r.append(r1 + (i*dr))
	C.append(2*2*result["We"]/(U^2))
	print(r[-1], C[-1])

# chart
pl.close()
pl.plot(r, C)
pl.grid(True)
pl.xlabel("r (m)")
pl.ylabel("C (F)")
fn_chart = pythonlab.tempname("png")
pl.savefig(fn_chart, dpi=60)
pl.close()

# show in console
pythonlab.image(fn_chart)