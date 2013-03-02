# model
newdocument(name="Electrostatics", type="axisymmetric",
            physicfield="electrostatic", analysistype="steadystate",
            numberofrefinements=1, polynomialorder=2,
            nonlineartolerance=0.001, nonlinearsteps=10)

# variables
r1 = 0.01
r2 = 0.03
r3 = 0.05
r4 = 0.055
l = 0.16
eps1 = 10
eps2 = 3
U = 10
dr = 0.003

# boundaries
addboundary("Source", "electrostatic_potential", {"V" : U})
addboundary("Ground", "electrostatic_potential", {"V" : 0})
addboundary("Neumann BC", "electrostatic_surface_charge_density", {"sigma" : 0})

# materials
addmaterial("Air", {"epsr" : 1})
addmaterial("Dielectric n.1", {"epsr" : eps1})
addmaterial("Dielectric n.2", {"epsr" : eps2})

# edges
addedge(0, 3/2*l, 0, l/2, boundary="Neumann BC")
addedge(r1, l/2, r1, 0, boundary="Source")
addedge(r1, 0, r2, 0, boundary="Neumann BC")
addedge(r2, 0, r2, l/2)
addedge(r2, l/2, r3, l/2)
addedge(r3, 0, r2, 0, boundary="Neumann BC")
addedge(r3, l/2, r3, 0, boundary="Ground")
addedge(r4, 0, r4, l/2, boundary="Ground")
addedge(r3, l/2, r4, l/2, boundary="Ground")
addedge(r4, 0, 3/2*l, 0, boundary="Neumann BC")
addedge(3/2*l, 0, 0, 3/2*l, angle=90, boundary="Neumann BC")
addedge(r1, l/2, r2, l/2)
addedge(r1, l/2, 0, l/2, boundary="Source")

# labels
addlabel(0.019, 0.021, material="Dielectric n.1")
addlabel(0.0379, 0.051, material="Dielectric n.2")
addlabel(0.0284191, 0.123601, material="Air")

zoombestfit()

# calculation of capacity
r = []
C = []
print("C = f(r) (F):")
for i in range(15):
	if i > 0:
		selectedge([6, 7, 8])
		moveselection(dr, 0, False)

	solve()
	result = volumeintegral([0, 1, 2])

	r.append(r1 + (i*dr))
	C.append(2*2*result["We"]/(U^2))
	print(r[-1], C[-1])

# plot chart
try:
	import pylab as pl

	pl.plot(r, C)
	pl.grid(1)
	pl.xlabel("r (m)")
	pl.ylabel("C (F)")
	pl.show()
except ImportError as err:
	print("Script error: " + err.message)