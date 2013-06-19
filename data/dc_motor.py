import agros2d as a2d

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

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

# fields
# magnetic
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "steadystate"
magnetic.polynomial_order = 1
magnetic.linearity_type = "linear"

# boundaries
magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})

# materials
magnetic.add_material("Air", {"magnetic_permeability": 1})
magnetic.add_material("Magnet - up", {"magnetic_permeability": 1, "magnetic_remanence" : Br_m, "magnetic_remanence_angle" : 90})
magnetic.add_material("Magnet - down", {"magnetic_permeability": 1, "magnetic_remanence" : Br_m, "magnetic_remanence_angle" : -90})

magnetic.add_material("Iron",{"magnetic_permeability": mur_iron})
magnetic.add_material("Armature", {"magnetic_permeability": mur_arm})
magnetic.add_material("Coil +", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : Jext})
magnetic.add_material("Coil -", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : -Jext})

# geometry
geometry = a2d.geometry

# edges
geometry.add_edge(0.018, -0.023, 0.0292062, 0, 51.953)
geometry.add_edge(0.0292062, 0, 0.018, 0.023, 51.953)
geometry.add_edge(-0.018, 0.023, -0.0292062, 0, 51.953)
geometry.add_edge(-0.0292062, 0, -0.018, -0.023, 51.953)
geometry.add_edge(-0.018, 0.01, -0.018, 0.023, 0)
geometry.add_edge(-0.018, 0.01, 0.018, 0.01, 0)
geometry.add_edge(0.018, 0.01, 0.018, 0.023, 0)
geometry.add_edge(0.018, -0.01, 0.018, -0.023, 0)
geometry.add_edge(0.018, -0.01, -0.018, -0.01, 0)
geometry.add_edge(-0.018, -0.01, -0.018, -0.023, 0)
geometry.add_edge(-0.0175, -0.0105, -0.0045, -0.0105, 0)
geometry.add_edge(-0.0045, -0.0105, -0.0045, -0.0185, 0)
geometry.add_edge(-0.0045, -0.0185, -0.0175, -0.0185, 0)
geometry.add_edge(-0.0175, -0.0105, -0.0175, -0.0185, 0)
geometry.add_edge(0.0045, -0.0185, 0.0045, -0.0105, 0)
geometry.add_edge(0.0045, -0.0105, 0.0175, -0.0105, 0)
geometry.add_edge(0.0175, -0.0105, 0.0175, -0.0185, 0)
geometry.add_edge(0.0175, -0.0185, 0.0045, -0.0185, 0)
geometry.add_edge(-0.0175, 0.0105, -0.0045, 0.0105, 0)
geometry.add_edge(-0.0045, 0.0105, -0.0045, 0.0185, 0)
geometry.add_edge(-0.0045, 0.0185, -0.0175, 0.0185, 0)
geometry.add_edge(-0.0175, 0.0105, -0.0175, 0.0185, 0)
geometry.add_edge(0.0045, 0.0185, 0.0045, 0.0105, 0)
geometry.add_edge(0.0045, 0.0105, 0.0175, 0.0105, 0)
geometry.add_edge(0.0175, 0.0105, 0.0175, 0.0185, 0)
geometry.add_edge(0.0175, 0.0185, 0.0045, 0.0185, 0)
geometry.add_edge(-0.023, 0.03, -0.023, 0.02, 0)
geometry.add_edge(-0.065, 0.03, -0.065, -0.03, 0)
geometry.add_edge(-0.065, -0.03, -0.023, -0.03, 0)
geometry.add_edge(0.023, -0.03, 0.065, -0.03, 0)
geometry.add_edge(0.023, -0.02, 0.023, -0.03, 0)
geometry.add_edge(0.065, -0.03, 0.065, 0.03, 0)
geometry.add_edge(0.023, 0.03, 0.023, 0.02, 0)
geometry.add_edge(-0.023, -0.02, -0.023, -0.03, 0)
geometry.add_edge(0.03, 0.113, 0.028, 0.115, 90)
geometry.add_edge(-0.028, 0.115, -0.03, 0.113, 90)
geometry.add_edge(0.03, 0.03, 0.023, 0.03, 0)
geometry.add_edge(-0.03, 0.03, -0.023, 0.03, 0)
geometry.add_edge(-0.028, 0.115, 0.028, 0.115, 0)
geometry.add_edge(-0.065, 0.09, -0.065, 0.03, 0)
geometry.add_edge(-0.065, 0.09, -0.03, 0.09, 0)
geometry.add_edge(-0.03, 0.09, -0.03, 0.03, 0)
geometry.add_edge(-0.03, 0.03, -0.065, 0.03, 0)
geometry.add_edge(0.03, 0.03, 0.03, 0.09, 0)
geometry.add_edge(0.03, 0.09, 0.065, 0.09, 0)
geometry.add_edge(0.065, 0.09, 0.065, 0.03, 0)
geometry.add_edge(0.065, 0.03, 0.03, 0.03, 0)
geometry.add_edge(0.03, 0.09, 0.03, 0.113, 0)
geometry.add_edge(-0.03, 0.113, -0.03, 0.09, 0)
geometry.add_edge(-0.03, 0.15, 0.03, 0.15, 0)
geometry.add_edge(-0.03, 0.15, -0.065, 0.115, 90)
geometry.add_edge(-0.065, 0.115, -0.065, 0.09, 0)
geometry.add_edge(0.065, 0.115, 0.03, 0.15, 90)
geometry.add_edge(0.065, 0.115, 0.065, 0.09, 0)
geometry.add_edge(0, -0.14, 0.2, 0.06, 90, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.2, 0.06, 0, 0.26, 90, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.26, -0.2, 0.06, 90, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(-0.2, 0.06, 0, -0.14, 90, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.023, -0.02, 0.0304795, 0, 41.0091)
geometry.add_edge(0.0304795, 0, 0.023, 0.02, 41.0091)
geometry.add_edge(-0.023, 0.02, -0.0304795, 0, 41.0091)
geometry.add_edge(-0.0304795, 0, -0.023, -0.02, 41.0091)

# labels
geometry.add_label(0, 0, 0, 0, materials = {"magnetic" : "Armature"})
geometry.add_label(-0.0110237, 0.0148242, 0, 0, materials = {"magnetic" :"Coil +"})
geometry.add_label(0.0110237, 0.0143919, 0, 0, materials = {"magnetic" :"Coil +"})
geometry.add_label(-0.0101591, -0.0145724, 0, 0, materials = {"magnetic" :"Coil -"})
geometry.add_label(0.00972684, -0.0145724, 0, 0, materials = {"magnetic" :"Coil -"})
geometry.add_label(0.0494383, -0.000732255, 0, 0, materials = {"magnetic" :"Iron"})
geometry.add_label(0.0349329, 0.134819, 0, 0, materials = {"magnetic" :"Iron"})
geometry.add_label(-0.0470982, -0.00973566, 0, 0, materials = {"magnetic" :"Iron"})
geometry.add_label(-0.0450974, 0.0662931, 0, 0, materials = {"magnetic" :"Magnet - up"})
geometry.add_label(0.0499385, 0.068794, 0, 0, materials = {"magnetic" :"Magnet - down"})
geometry.add_label(0.111462, 0.102807, 0, 0, materials = {"magnetic" :"Air"})

agros2d.view.zoom_best_fit()
agros2d.view.zoom_region(-0.04, -0.037, 0.04, 0.037)

# calculation of torque
dphi = 180.0/N
phi = []
T = []
Wm = []
for i in range(N+2):
	problem.solve()
	agros2d.view.post2d.activate
	agros2d.view.post2d.contours = True
	agros2d.view.post2d.scalar_variable = "magnetic_flux_density_real"
	agros2d.view.post2d.scalar_range_min = 0
	agros2d.view.post2d.scalar_range_max = 0.8
	agros2d.view.save_image("img_" + "%02d" % (i)	+ ".png")

	result = magnetic.volume_integrals([10])
	Wm.append(result['Wm'])	
	if i != 0:
		phi.append(i*dphi)					
		T.append((Wm[i] - Wm[i-1])/dphi)
	
	geometry.select_edges([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
	geometry.rotate_selection(0, 0, dphi, False)

	geometry.select_edges([10, 11, 12, 13, 14, 15, 16, 17])
	geometry.rotate_selection(0, 0, dphi, False)

	geometry.select_edges([18, 19, 20, 21, 22, 23, 24, 25])
	geometry.rotate_selection(0, 0, dphi, False)

	geometry.select_labels([1, 2, 3, 4])
	geometry.rotate_selection(0, 0, dphi, False)

	if ((i*dphi) > phi_commutation):
		magnetic.modify_material("Coil +", [-Jext, 0, 1, 0, 0, 0, 0, 0, 0])
		magnetic.modify_material("Coil -", [Jext, 0, 1, 0, 0, 0, 0, 0, 0])	

import pylab as pl

pl.plot(phi, T)
pl.xlabel("$\\varphi~\mathrm{(deg.)}$")
pl.ylabel("$T~\mathrm{(Nm)}")
fn_torque = pythonlab.tempname("png")
pl.savefig(fn_torque, dpi=60)
pl.close()

# show in console
pythonlab.image(fn_torque)