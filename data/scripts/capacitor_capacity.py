# model
newdocument("Electrostatic Axisymmetric Capacitor", "axisymmetric", "electrostatic", 1, 3, "disabled", 1, 1, 0, "steadystate", 0, 0, 0)

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
addboundary("Source", "electrostatic_potential", U)
addboundary("Ground", "electrostatic_potential", 0)
addboundary("Border", "electrostatic_surface_charge_density", 0)

# materials
addmaterial("Air", 0, 1)
addmaterial("Dielectric 1", 0, eps1)
addmaterial("Dielectric 2", 0, eps2)

# edges
addedge(0, 3/2*l, 0, l/2, 0, "Border")
addedge(r1, l/2, r1, 0, 0, "Source")
addedge(r1, 0, r2, 0, 0, "Border")
addedge(r2, 0, r2, l/2, 0, "none")
addedge(r2, l/2, r3, l/2, 0, "none")
addedge(r3, 0, r2, 0, 0, "Border")
addedge(r3, l/2, r3, 0, 0, "Ground")
addedge(r4, 0, r4, l/2, 0, "Ground")
addedge(r3, l/2, r4, l/2, 0, "Ground")
addedge(r4, 0, 3/2*l, 0, 0, "Border")
addedge(3/2*l, 0, 0, 3/2*l, 90, "Border")
addedge(r1, l/2, r2, l/2, 0, "none")
addedge(r1, l/2, 0, l/2, 0, "Source")

# labels
addlabel(0.019, 0.021, 0, 0, "Dielectric 1")
addlabel(0.0379, 0.051, 0, 0, "Dielectric 2")
addlabel(0.0284191, 0.123601, 0, 0, "Air")

zoombestfit()

# calculation of capacity
r = []
C = []
print("C = f(r) (F):")
for i in range(15):
    if i > 0:
        selectedge(6, 7, 8)
        moveselection(dr, 0, False)

    solve()
    result = volumeintegral(0, 1, 2)

    r.append(r1 + (i*dr))
    C.append(2*2*result["We"]/(U^2))
    print(r[-1], C[-1])

# plot chart
import pylab as pl

pl.plot(r, C)
pl.grid(True)
pl.xlabel("$r\,\mathrm{(m)}$")
pl.ylabel("$C\,\mathrm{(F)}$")
fn_capacity = pythonlab.tempname("png")
pl.savefig(fn_capacity, dpi=60)
pl.close()

# show in console
pythonlab.image(fn_capacity)
