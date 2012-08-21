I = 0.8
# I = 0.0
S = pi*0.0005**2
Jext = I/S*0.7
Br_m = 0.1
mur_m = 1.07
mur_iron = 180
mur_arm = 200
phi_commutation = 25
length = 0.035
N = 45

# model
newdocument("unnamed", "planar", "magnetic", 1, 3, "disabled", 1, 1, 0, "steadystate", 1.0, 1.0, 0.0)

# boundaries
addboundary("A = 0", "magnetic_vector_potential", 0, 0)

# materials
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Magnet - up", 0, 0, mur_m, 0, Br_m, 90, 0, 0, 0)
addmaterial("Magnet - down", 0, 0, mur_m, 0, Br_m, -90, 0, 0, 0)
addmaterial("Iron", 0, 0, mur_iron, 0, 0, 0, 0, 0, 0)
addmaterial("Armature", 0, 0, mur_arm, 0, 0, 0, 0, 0, 0)
addmaterial("Coil +", Jext, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Coil -", -Jext, 0, 1, 0, 0, 0, 0, 0, 0)

# edges
addedge(0.018, -0.023, 0.0292062, 0, 51.953, "none")
addedge(0.0292062, 0, 0.018, 0.023, 51.953, "none")
addedge(-0.018, 0.023, -0.0292062, 0, 51.953, "none")
addedge(-0.0292062, 0, -0.018, -0.023, 51.953, "none")
addedge(-0.018, 0.01, -0.018, 0.023, 0, "none")
addedge(-0.018, 0.01, 0.018, 0.01, 0, "none")
addedge(0.018, 0.01, 0.018, 0.023, 0, "none")
addedge(0.018, -0.01, 0.018, -0.023, 0, "none")
addedge(0.018, -0.01, -0.018, -0.01, 0, "none")
addedge(-0.018, -0.01, -0.018, -0.023, 0, "none")
addedge(-0.0175, -0.0105, -0.0045, -0.0105, 0, "none")
addedge(-0.0045, -0.0105, -0.0045, -0.0185, 0, "none")
addedge(-0.0045, -0.0185, -0.0175, -0.0185, 0, "none")
addedge(-0.0175, -0.0105, -0.0175, -0.0185, 0, "none")
addedge(0.0045, -0.0185, 0.0045, -0.0105, 0, "none")
addedge(0.0045, -0.0105, 0.0175, -0.0105, 0, "none")
addedge(0.0175, -0.0105, 0.0175, -0.0185, 0, "none")
addedge(0.0175, -0.0185, 0.0045, -0.0185, 0, "none")
addedge(-0.0175, 0.0105, -0.0045, 0.0105, 0, "none")
addedge(-0.0045, 0.0105, -0.0045, 0.0185, 0, "none")
addedge(-0.0045, 0.0185, -0.0175, 0.0185, 0, "none")
addedge(-0.0175, 0.0105, -0.0175, 0.0185, 0, "none")
addedge(0.0045, 0.0185, 0.0045, 0.0105, 0, "none")
addedge(0.0045, 0.0105, 0.0175, 0.0105, 0, "none")
addedge(0.0175, 0.0105, 0.0175, 0.0185, 0, "none")
addedge(0.0175, 0.0185, 0.0045, 0.0185, 0, "none")
addedge(-0.023, 0.03, -0.023, 0.02, 0, "none")
addedge(-0.065, 0.03, -0.065, -0.03, 0, "none")
addedge(-0.065, -0.03, -0.023, -0.03, 0, "none")
addedge(0.023, -0.03, 0.065, -0.03, 0, "none")
addedge(0.023, -0.02, 0.023, -0.03, 0, "none")
addedge(0.065, -0.03, 0.065, 0.03, 0, "none")
addedge(0.023, 0.03, 0.023, 0.02, 0, "none")
addedge(-0.023, -0.02, -0.023, -0.03, 0, "none")
addedge(0.03, 0.113, 0.028, 0.115, 90, "none")
addedge(-0.028, 0.115, -0.03, 0.113, 90, "none")
addedge(0.03, 0.03, 0.023, 0.03, 0, "none")
addedge(-0.03, 0.03, -0.023, 0.03, 0, "none")
addedge(-0.028, 0.115, 0.028, 0.115, 0, "none")
addedge(-0.065, 0.09, -0.065, 0.03, 0, "none")
addedge(-0.065, 0.09, -0.03, 0.09, 0, "none")
addedge(-0.03, 0.09, -0.03, 0.03, 0, "none")
addedge(-0.03, 0.03, -0.065, 0.03, 0, "none")
addedge(0.03, 0.03, 0.03, 0.09, 0, "none")
addedge(0.03, 0.09, 0.065, 0.09, 0, "none")
addedge(0.065, 0.09, 0.065, 0.03, 0, "none")
addedge(0.065, 0.03, 0.03, 0.03, 0, "none")
addedge(0.03, 0.09, 0.03, 0.113, 0, "none")
addedge(-0.03, 0.113, -0.03, 0.09, 0, "none")
addedge(-0.03, 0.15, 0.03, 0.15, 0, "none")
addedge(-0.03, 0.15, -0.065, 0.115, 90, "none")
addedge(-0.065, 0.115, -0.065, 0.09, 0, "none")
addedge(0.065, 0.115, 0.03, 0.15, 90, "none")
addedge(0.065, 0.115, 0.065, 0.09, 0, "none")
addedge(0, -0.14, 0.2, 0.06, 90, "A = 0")
addedge(0.2, 0.06, 0, 0.26, 90, "A = 0")
addedge(0, 0.26, -0.2, 0.06, 90, "A = 0")
addedge(-0.2, 0.06, 0, -0.14, 90, "A = 0")
addedge(0.023, -0.02, 0.0304795, 0, 41.0091, "none")
addedge(0.0304795, 0, 0.023, 0.02, 41.0091, "none")
addedge(-0.023, 0.02, -0.0304795, 0, 41.0091, "none")
addedge(-0.0304795, 0, -0.023, -0.02, 41.0091, "none")

# labels
addlabel(0, 0, 0, 0, "Armature")
addlabel(-0.0110237, 0.0148242, 0, 0, "Coil +")
addlabel(0.0110237, 0.0143919, 0, 0, "Coil +")
addlabel(-0.0101591, -0.0145724, 0, 0, "Coil -")
addlabel(0.00972684, -0.0145724, 0, 0, "Coil -")
addlabel(0.0494383, -0.000732255, 0, 0, "Iron")
addlabel(0.0349329, 0.134819, 0, 0, "Iron")
addlabel(-0.0470982, -0.00973566, 0, 0, "Iron")
addlabel(-0.0450974, 0.0662931, 0, 0, "Magnet - up")
addlabel(0.0499385, 0.068794, 0, 0, "Magnet - down")
addlabel(0.111462, 0.102807, 0, 0, "Air")

zoombestfit()
zoomregion(-0.04, -0.037, 0.04, 0.037)

# calculation of torque
dphi = 180.0/N
phi = []
T = []
for i in range(N+1):
	solve()
	showcontours(True)
	showscalar("scalar", "magnetic_flux_density_real", "magnitude", 0, 0.8)
	saveimage("img_" + "%02d" % (i)	+ ".png")

	result = surfaceintegral(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)

	phi.append(i*dphi)
	T.append(result["T"] * length)
	print(phi[-1], T[-1])

	selectedge(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
	rotateselection(0, 0, dphi, False)

	selectedge(10, 11, 12, 13, 14, 15, 16, 17)
	rotateselection(0, 0, dphi, False)

	selectedge(18, 19, 20, 21, 22, 23, 24, 25)
	rotateselection(0, 0, dphi, False)

	selectlabel(1, 2, 3, 4)
	rotateselection(0, 0, dphi, False)

	if ((i*dphi) > phi_commutation):
		modifymaterial("Coil +", -Jext, 0, 1, 0, 0, 0, 0, 0, 0)
		modifymaterial("Coil -",  Jext, 0, 1, 0, 0, 0, 0, 0, 0)

import pylab as pl

pl.plot(phi, T)
pl.xlabel("$\\varphi~\mathrm{(deg.)}$")
pl.ylabel("$T~\mathrm{(Nm)}")
fn_torque = pythonlab.tempname("png")
pl.savefig(fn_torque, dpi=60)
pl.close()

# show in console
pythonlab.image(fn_torque)