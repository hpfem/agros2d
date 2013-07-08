# http://www.compumag.org/jsite/images/stories/TEAM/problem30a.pdf
# http://www.compumag.org/jsite/images/stories/TEAM/problem30b.pdf

import agros2d as a2d

def model(omega):
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "planar"
    problem.mesh_type = "triangle"
    problem.frequency = 60
    
    # fields
    # magnetic
    magnetic = a2d.field("magnetic")
    magnetic.analysis_type = "harmonic"
    magnetic.matrix_solver = "mumps"
    magnetic.number_of_refinements = 0
    magnetic.polynomial_order = 3
    magnetic.adaptivity_type = "disabled"
    magnetic.solver = "linear"
    
    # boundaries
    magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})
    
    # materials
    magnetic.add_material("Stator steel", {"magnetic_permeability" : 30, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("Rotor steel", {"magnetic_permeability" : 30, "magnetic_conductivity" : 3.72e+06, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : omega, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("Aluminum", {"magnetic_permeability" : 1, "magnetic_conductivity" : 3.72e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : omega, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("J+", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : sqrt(2)*3.1e6, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("J-", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : -sqrt(2)*3.1e6, "magnetic_current_density_external_imag" : 0})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0, -0.02, 0.02, 0, angle = 90)
    geometry.add_edge(0.02, 0, 0, 0.02, angle = 90)
    geometry.add_edge(0, 0.02, -0.02, 0, angle = 90)
    geometry.add_edge(-0.02, 0, 0, -0.02, angle = 90)
    geometry.add_edge(0, -0.03, 0.03, 0, angle = 90)
    geometry.add_edge(0.03, 0, 0, 0.03, angle = 90)
    geometry.add_edge(0, 0.03, -0.03, 0, angle = 90)
    geometry.add_edge(-0.03, 0, 0, -0.03, angle = 90)
    geometry.add_edge(0, -0.057, 0.057, 0, angle = 90)
    geometry.add_edge(0.057, 0, 0, 0.057, angle = 90)
    geometry.add_edge(0, 0.057, -0.057, 0, angle = 90)
    geometry.add_edge(-0.057, 0, 0, -0.057, angle = 90)
    geometry.add_edge(0, 0.052, -0.0480417, 0.0198995, angle = 67.5)
    geometry.add_edge(0, -0.052, 0.0480417, -0.0198995, angle = 67.5)
    geometry.add_edge(0.0480417, 0.0198995, 0, 0.052, angle = 67.5)
    geometry.add_edge(0.0295641, -0.0122459, 0.0295641, 0.0122459, angle = 45)
    geometry.add_edge(0.0480417, -0.0198995, 0.0480417, 0.0198995, angle = 45)
    geometry.add_edge(0.0295641, 0.0122459, 0.0480417, 0.0198995)
    geometry.add_edge(0.0295641, -0.0122459, 0.0480417, -0.0198995)
    geometry.add_edge(-0.0480417, -0.0198995, 0, -0.052, angle = 67.5)
    geometry.add_edge(-0.0295641, 0.0122459, -0.0295641, -0.0122459, angle = 45)
    geometry.add_edge(-0.0480417, 0.0198995, -0.0480417, -0.0198995, angle = 45)
    geometry.add_edge(-0.0295641, 0.0122459, -0.0480417, 0.0198995)
    geometry.add_edge(-0.0295641, -0.0122459, -0.0480417, -0.0198995)
    geometry.add_edge(0, -0.5, 0.5, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0.5, 0, 0, 0.5, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0, 0.5, -0.5, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(-0.5, 0, 0, -0.5, angle = 90, boundaries = {"magnetic" : "A = 0"})
    
    geometry.add_label(-0.0219459, 0.0500867, materials = {"magnetic" : "Stator steel"})
    geometry.add_label(-0.00691988, 0.00711759, area = 6e-06, materials = {"magnetic" : "Rotor steel"})
    geometry.add_label(-0.0143011, 0.021221, area = 3e-06, materials = {"magnetic" : "Aluminum"})
    geometry.add_label(-0.0186507, 0.0351925, area = 5e-06, materials = {"magnetic" : "Air"})
    geometry.add_label(-0.0426396, 0.00276795, area = 5e-06, materials = {"magnetic" : "J-"})
    geometry.add_label(0.0415852, -0.00289976, area = 5e-06, materials = {"magnetic" : "J+"})
    geometry.add_label(-0.28823, 0.248321, materials = {"magnetic" : "Air"})

    a2d.view.zoom_best_fit()
    problem.solve()
    
    volume = magnetic.volume_integrals([1, 2])
    return volume["T"], volume["Pj"]

# analytical solution
# rpm        torque     rotor losses
# 0          0          341.7676 
# 39.79351  -0.052766   341.2465 
# 79.58701  -0.096143   340.4618 
# 119.3805  -0.14305    340.0396 
# 159.174   -0.19957    340.225 
# 198.9675  -0.2754     339.2994
# 238.761   -0.367972   333.6163
# 278.5546  -0.442137   317.9933
# 318.3481  -0.375496   288.079
# 358.1416   0.0707     256.6437

T, Pj = model(278.5546)

print("Torque")
print("analytical solution = -0.442137 Nm")
print("Agros2D = " + str(T) + " Nm")
print("difference = " + str(100 * (T + 0.442137) / 0.442137) + " %")

print("")
print("Rotor losses")
print("analytical solution = 317.9933 W")
print("Agros2D = " + str(Pj) + " W")
print("difference = " + str(100 * (Pj - 317.9933) / 317.9933) + " %")