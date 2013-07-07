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
    magnetic.add_material("Rotor steel", {"magnetic_permeability" : 30, "magnetic_conductivity" : 1.6e+06, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : omega, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("Aluminum", {"magnetic_permeability" : 1, "magnetic_conductivity" : 3.72e+07, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : omega, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("JA+", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : sqrt(2)*3.1e6, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("JA-", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : -sqrt(2)*3.1e6, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("JB+", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : sqrt(2)*3.1e6*cos(2.0*pi/3.0), "magnetic_current_density_external_imag" : sqrt(2)*3.1e6*sin(2.0*pi/3.0)})
    magnetic.add_material("JB-", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : -sqrt(2)*3.1e6*cos(2.0*pi/3.0), "magnetic_current_density_external_imag" : -sqrt(2)*3.1e6*sin(2.0*pi/3.0)})
    magnetic.add_material("JC+", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : sqrt(2)*3.1e6*cos(4.0*pi/3.0), "magnetic_current_density_external_imag" : sqrt(2)*3.1e6*sin(4.0*pi/3.0)})
    magnetic.add_material("JC-", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : -sqrt(2)*3.1e6*cos(4.0*pi/3.0), "magnetic_current_density_external_imag" : -sqrt(2)*3.1e6*sin(4.0*pi/3.0)})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0, -0.02, 0.02, 0, angle = 90)
    geometry.add_edge(0.02, 0, 0, 0.02, angle = 90)
    geometry.add_edge(0, 0.02, -0.02, 0, angle = 90)
    geometry.add_edge(-0.02, 0, 0, -0.02, angle = 90)
    geometry.add_edge(0, -0.057, 0.057, 0, angle = 90)
    geometry.add_edge(0.057, 0, 0, 0.057, angle = 90)
    geometry.add_edge(0, 0.057, -0.057, 0, angle = 90)
    geometry.add_edge(-0.057, 0, 0, -0.057, angle = 90)
    geometry.add_edge(0.0295641, -0.0122459, 0.0295641, 0.0122459, angle = 45)
    geometry.add_edge(0.0480417, -0.0198995, 0.0480417, 0.0198995, angle = 45)
    geometry.add_edge(0.0295641, 0.0122459, 0.0480417, 0.0198995)
    geometry.add_edge(0.0295641, -0.0122459, 0.0480417, -0.0198995)
    geometry.add_edge(-0.0295641, 0.0122459, -0.0295641, -0.0122459, angle = 45)
    geometry.add_edge(-0.0480417, 0.0198995, -0.0480417, -0.0198995, angle = 45)
    geometry.add_edge(-0.0295641, 0.0122459, -0.0480417, 0.0198995)
    geometry.add_edge(-0.0295641, -0.0122459, -0.0480417, -0.0198995)
    geometry.add_edge(0.0412543, 0.0316556, 0.00678738, 0.0515551, angle = 45)
    geometry.add_edge(-0.00678738, 0.0515551, -0.0412543, 0.0316556, angle = 45)
    geometry.add_edge(0.00678738, 0.0515551, -0.00678738, 0.0515551, angle = 15)
    geometry.add_edge(-0.0412543, 0.0316556, -0.0480417, 0.0198995, angle = 15)
    geometry.add_edge(-0.0480417, -0.0198995, -0.0412543, -0.0316556, angle = 15)
    geometry.add_edge(-0.0412543, -0.0316556, -0.00678738, -0.0515551, angle = 45)
    geometry.add_edge(-0.00678738, -0.0515551, 0.00678738, -0.0515551, angle = 15)
    geometry.add_edge(0.00678738, -0.0515551, 0.0412543, -0.0316556, angle = 46)
    geometry.add_edge(0.0412543, -0.0316556, 0.0480417, -0.0198995, angle = 15)
    geometry.add_edge(0.0480417, 0.0198995, 0.0412543, 0.0316556, angle = 15)
    geometry.add_edge(0.0253873, 0.0194803, 0.00417679, 0.0317262, angle = 45)
    geometry.add_edge(-0.00417679, 0.0317262, -0.0253873, 0.0194803, angle = 45)
    geometry.add_edge(-0.0253873, -0.0194803, -0.00417679, -0.0317262, angle = 45)
    geometry.add_edge(0.00417679, -0.0317262, 0.0253873, -0.0194803, angle = 45)
    geometry.add_edge(-0.0253873, -0.0194803, -0.0412543, -0.0316556)
    geometry.add_edge(-0.00417679, -0.0317262, -0.00678738, -0.0515551)
    geometry.add_edge(0.00417679, -0.0317262, 0.00678738, -0.0515551)
    geometry.add_edge(0.0253873, -0.0194803, 0.0412543, -0.0316556)
    geometry.add_edge(0.0253873, 0.0194803, 0.0412543, 0.0316556)
    geometry.add_edge(0.00678738, 0.0515551, 0.00417679, 0.0317262)
    geometry.add_edge(-0.00678738, 0.0515551, -0.00417679, 0.0317262)
    geometry.add_edge(-0.0412543, 0.0316556, -0.0253873, 0.0194803)
    geometry.add_edge(0, -0.03, 0.03, 0, angle = 90)
    geometry.add_edge(0.03, 0, 0, 0.03, angle = 90)
    geometry.add_edge(0, 0.03, -0.03, 0, angle = 90)
    geometry.add_edge(-0.03, 0, 0, -0.03, angle = 90)
    geometry.add_edge(0, -0.5, 0.5, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0.5, 0, 0, 0.5, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0, 0.5, -0.5, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(-0.5, 0, 0, -0.5, angle = 90, boundaries = {"magnetic" : "A = 0"})
    
    geometry.add_label(-0.0219459, 0.0500867, materials = {"magnetic" : "Stator steel"})
    geometry.add_label(-0.00691988, 0.00711759, area = 4e-06, materials = {"magnetic" : "Rotor steel"})
    geometry.add_label(-0.0143011, 0.021221, area = 4e-06, materials = {"magnetic" : "Aluminum"})
    geometry.add_label(-0.000596728, 0.039381, materials = {"magnetic" : "Air"})
    geometry.add_label(-0.0426396, 0.00276795, materials = {"magnetic" : "JA-"})
    geometry.add_label(0.0415852, -0.00289976, materials = {"magnetic" : "JA+"})
    geometry.add_label(0.0200038, 0.0368301, materials = {"magnetic" : "JB-"})
    geometry.add_label(-0.0243368, 0.0356746, materials = {"magnetic" : "JC+"})
    geometry.add_label(-0.0197149, -0.0359635, materials = {"magnetic" : "JB+"})
    geometry.add_label(0.0182706, -0.0336526, materials = {"magnetic" : "JC-"})
    geometry.add_label(-0.251489, 0.301533, materials = {"magnetic" : "Air"})
    
    a2d.view.zoom_best_fit()
    problem.solve()
    
    volume = magnetic.volume_integrals([1, 2])
    return volume["T"], volume["Pj"]

# analytical solution
# rpm    torque     rotor losses
# 0     -3.825857   1455.644 
# 200   -6.505013   1179.541 
# 400    3.89264    120.0092
# 600    5.75939    1314.613
# 800    3.59076    1548.24
# 1000   2.70051    1710.686
# 1200   2.24996    1878.926 

T, Pj = model(1000)

print("Torque")
print("analytical solution = 2.70051 Nm")
print("Agros2D = " + str(T) + " Nm")
print("difference = " + str(100 * (T - 2.70051) / 2.70051) + " %")

print("")
print("Rotor losses")
print("analytical solution = 1710.686 W")
print("Agros2D = " + str(Pj) + " W")
print("difference = " + str(100 * (Pj - 1710.686) / 1710.686) + " %")