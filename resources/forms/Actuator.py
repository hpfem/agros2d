import agros2d as a2d

def magnetic_model(Rarm, Harm, Rcir, Hcir, delta, mur_iron, Iext, Next):
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"
    problem.mesh_type = "triangle"
    
    # fields
    # magnetic
    magnetic = a2d.field("magnetic")
    magnetic.analysis_type = "steadystate"
    magnetic.matrix_solver = "mumps"
    magnetic.number_of_refinements = 1
    magnetic.polynomial_order = 2
    magnetic.adaptivity_type = "disabled"
    magnetic.solver = "linear"
    
    # boundaries
    magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
    
    # materials
    Scoil = ((Hcir - Rarm - delta) - (Rarm + 2*delta)) * ((Rcir - Rarm - delta) - (Rarm + 2*delta))
    Jext = Next * Iext / Scoil

    magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
    magnetic.add_material("Iron", {"magnetic_permeability" : mur_iron, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
    magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : Jext})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(Rarm + delta, Rarm + delta, Rarm + delta, 0)
    geometry.add_edge(Rarm + delta, 0, Rcir, 0)
    geometry.add_edge(Rcir, 0, Rcir, Hcir)
    geometry.add_edge(Rcir, Hcir, 0, Hcir)
    geometry.add_edge(0, Hcir, 0, Hcir - 2*Rarm, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0, Hcir - 2*Rarm, Rarm + delta, Hcir - 2*Rarm)
    geometry.add_edge(Rarm + delta, Hcir - Rarm, Rcir - Rarm, Hcir - Rarm)
    geometry.add_edge(Rarm + delta, Hcir - Rarm, Rarm + delta, Hcir - 2*Rarm)
    geometry.add_edge(Rcir - Rarm, Hcir - Rarm, Rcir - Rarm, Rarm + delta)
    geometry.add_edge(Rarm + delta, Rarm + delta, Rcir - Rarm, Rarm + delta)
    geometry.add_edge(Rarm + 2*delta, Hcir - Rarm - delta, Rarm + 2*delta, Rarm + 2*delta)
    geometry.add_edge(Rarm + 2*delta, Rarm + 2*delta, Rcir - Rarm - delta, Rarm + 2*delta)
    geometry.add_edge(Rcir - Rarm - delta, Rarm + 2*delta, Rcir - Rarm - delta, Hcir - Rarm - delta)
    geometry.add_edge(Rcir - Rarm - delta, Hcir - Rarm - delta, Rarm + 2*delta, Hcir - Rarm - delta)
    geometry.add_edge(0, Harm, Rarm, Harm)
    geometry.add_edge(Rarm, Harm, Rarm, 0)
    geometry.add_edge(0, 0, Rarm, 0)
    geometry.add_edge(0, Harm, 0, 0, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0, 2*Hcir, 0, Hcir, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0, Hcir - 2*Rarm, 0, Harm, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0, 0, 0, -Hcir, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0, -Hcir, 1.5*Hcir, 0.5*Hcir, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(1.5*Hcir, 0.5*Hcir, 0, 2*Hcir, angle = 90, boundaries = {"magnetic" : "A = 0"})
    
    geometry.add_label(Rcir - Rarm / 2.0, Hcir / 2.0, materials = {"magnetic" : "Iron"})
    geometry.add_label(Rcir - Rarm - delta - Rarm, Harm / 2.0, materials = {"magnetic" : "Copper"})
    geometry.add_label(2*Rcir, 0.5 * Hcir, materials = {"magnetic" : "Air"})
    geometry.add_label(Rarm / 2.0, Harm / 2.0, materials = {"magnetic" : "Iron"})

    a2d.view.zoom_best_fit()

    # static characteristic
    steps = 10
    dz = Harm / (steps - 1)
    zWm = []
    Wm = []
    for i in range(steps):
        if i > 0:
            geometry.select_edges([14, 15, 16, 17])
            geometry.move_selection(0, -dz, False)
            geometry.select_labels([3])
            geometry.move_selection(0, -dz, False)
    
        problem.solve()
        a2d.view.zoom_best_fit()
        result = magnetic.volume_integrals([0, 1, 2, 3])
        zWm.append(i*dz)
        Wm.append(result["Wm"])
        # print(zWm[-1], Wm[-1])
    
    zFm = []
    Fm = []
    for i in range(steps - 1):
        Fm.append((Wm[i] - Wm[i+1]) / dz)
        zFm.append(zWm[i] + dz / 2)
        print(zFm[-1], Fm[-1])

	# chart
	import pylab as pl

	pl.close()
	pl.subplot(2,1,1)
	pl.plot(zWm, Wm)
	pl.title("Magnetic energy")
	pl.grid(True)
	pl.xlabel("z (m)")
	pl.ylabel("Wm (J)")
	pl.subplot(2,1,2)
	pl.plot(zFm, Fm)
	pl.title("Force")
	pl.grid(True)
	pl.xlabel("z (m)")
	pl.ylabel("F (N)")
	pl.show()

Rarm = {{txtArmatureRadius_text}}
Harm = {{txtArmatureHeight_text}}
Rcir = {{txtCircuitRadius_text}}
Hcir = {{txtCircuitHeight_text}}
delta = {{txtArmatureRadius_text}} / 10.0

mur_iron = {{txtIronPermeability_text}}
Iext = {{txtCoilCurrent_text}}
Next = {{txtCoilNumberOfTurns_text}}

"""
Rarm = 0.009
Harm = 0.075
Rcir = 0.04
Hcir = 0.1
delta = 0.001

mur_iron = 9300
Iext = 1
Next = 1500
"""

magnetic_model(Rarm, Harm, Rcir, Hcir, delta, mur_iron, Iext, Next)
