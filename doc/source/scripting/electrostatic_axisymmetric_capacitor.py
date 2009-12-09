# model
newdocument("Electrostatic axisymmetric capacitor", "axisymmetric", "electrostatic", 1, 2)

# variables
r1 = 0.01
r2 = 0.03
r3 = 0.05
r4 = 0.055
l = 0.16
eps1 = 10
eps2 = 3
U = 10
dr = 0.01

# boundaries
addboundary("Source electrode", "electrostatic_potential", U)
addboundary("Ground electrode", "electrostatic_potential", 0)
addboundary("Neumann BC", "electrostatic_surface_charge_density", 0)

# materials
addmaterial("Air", 0, 1)
addmaterial("Dielectric n.1", 0, eps1)
addmaterial("Dielectric n.2", 0, eps2)

# edges
addedge(0, 3/2*l, 0, l/2, 0, "Neumann BC")
addedge(r1, l/2, r1, 0, 0, "Source electrode")
addedge(r1, 0, r2, 0, 0, "Neumann BC")
addedge(r2, 0, r2, l/2, 0, "none")
addedge(r2, l/2, r3, l/2, 0, "none")
addedge(r3, 0, r2, 0, 0, "Neumann BC")
addedge(r3, l/2, r3, 0, 0, "Ground electrode")
addedge(r4, 0, r4, l/2, 0, "Ground electrode")
addedge(r3, l/2, r4, l/2, 0, "Ground electrode")
addedge(r4, 0, 3/2*l, 0, 0, "Neumann BC")
addedge(3/2*l, 0, 0, 3/2*l, 90, "Neumann BC")
addedge(r1, l/2, r2, l/2, 0, "none")
addedge(r1, l/2, 0, l/2, 0, "Source electrode")

# labels
addlabel(0.019, 0.021, 0, "Dielectric n.1")
addlabel(0.0379, 0.051, 0, "Dielectric n.2")
addlabel(0.0284191, 0.123601, 0, "Air")

zoombestfit()
selectnone()

# calculation of capacity
r = []
C = []
print "C = f(r) (F):"
for i in range(5):
	if i > 0:
		selectedge(6, 7, 8)
		moveselection(dr, 0, False)

	solve()
	integral = volumeintegral(0, 1, 2)

	r.append(r1 + (i*dr))
	C.append(2*2*integral["We"]/(U^2))

print r
print C

# plot chart (only if you have pylab in your computer)
#import pylab
#pylab.plot(r, C)
#pylab.grid(1)
#pylab.xlabel("r (m)")
#pylab.ylabel("C (F)")
#pylab.show()
