import pythonlab
import agros2d as a2d

import scipy.io as sio
import pylab as pl

def model_electrostatic():
    print("Electrostatic field + adaptivity")
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"
    problem.mesh_type = "triangle"
    
    # fields
    # electrostatic
    electrostatic = a2d.field("electrostatic")
    electrostatic.analysis_type = "steadystate"
    electrostatic.matrix_solver = "mumps"
    electrostatic.number_of_refinements = 0
    electrostatic.polynomial_order = 2
    electrostatic.adaptivity_type = "hp-adaptivity"
    
    electrostatic.adaptivity_parameters['steps'] = 10
    electrostatic.adaptivity_parameters['tolerance'] = 0.5
    electrostatic.adaptivity_parameters['threshold'] = 0.7
    electrostatic.adaptivity_parameters['stopping_criterion'] = "singleelement"
    electrostatic.adaptivity_parameters['error_calculator'] = "h1"
    electrostatic.adaptivity_parameters['anisotropic_refinement'] = True
    electrostatic.adaptivity_parameters['finer_reference_solution'] = False
    electrostatic.solver = "linear"
    
    # boundaries
    electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0.2, 1, 0, 0.5, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0, 0.5, 0, 0.25, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -0.25, 0, -1, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -1, 1.5, 0.5, angle = 90, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(1.5, 0.5, 0, 2, angle = 90, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, 1, 0.2, 1, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0, 2, 0, 1, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -0.25, 0.25, 0, angle = 90, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"electrostatic" : "Ground"})
    
    geometry.add_label(0.879551, 0.764057, area = 0.06, materials = {"electrostatic" : "Air"})
    
    a2d.view.zoom_best_fit()
    problem.solve()
    
    return electrostatic.filename_matrix(), electrostatic.filename_rhs()
    
def model_harmonic_magnetic():
    print("Harmonic magnetic field (two components)")
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "planar"
    problem.mesh_type = "triangle"
    problem.frequency = 50000
    
    # fields
    # magnetic
    magnetic = a2d.field("magnetic")
    magnetic.analysis_type = "harmonic"
    magnetic.matrix_solver = "mumps"
    magnetic.number_of_refinements = 2
    magnetic.polynomial_order = 3
    magnetic.adaptivity_type = "disabled"
    magnetic.solver = "linear"
    
    # boundaries
    magnetic.add_boundary("Neumann", "magnetic_surface_current", {"magnetic_surface_current_real" : 0, "magnetic_surface_current_imag" : 0})
    magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})
    
    # materials
    magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
    magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 57e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 1e6, "magnetic_current_density_external_imag" : 0})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0, 0.002, 0, 0.000768, boundaries = {"magnetic" : "Neumann"})
    geometry.add_edge(0, 0.000768, 0, 0, boundaries = {"magnetic" : "Neumann"})
    geometry.add_edge(0, 0, 0.000768, 0, boundaries = {"magnetic" : "Neumann"})
    geometry.add_edge(0.000768, 0, 0.002, 0, boundaries = {"magnetic" : "Neumann"})
    geometry.add_edge(0.002, 0, 0, 0.002, angle = 90, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0.000768, 0, 0.000576, 0.000192, angle = 90)
    geometry.add_edge(0.000576, 0.000192, 0.000384, 0.000192)
    geometry.add_edge(0.000192, 0.000384, 0.000384, 0.000192, angle = 90)
    geometry.add_edge(0.000192, 0.000576, 0.000192, 0.000384)
    geometry.add_edge(0.000192, 0.000576, 0, 0.000768, angle = 90)
    
    geometry.add_label(0.000585418, 0.00126858, materials = {"magnetic" : "Air"})
    geometry.add_label(0.000109549, 8.6116e-05, materials = {"magnetic" : "Copper"})
    
    a2d.view.zoom_best_fit()
    problem.solve()
    
    return magnetic.filename_matrix(), magnetic.filename_rhs()


def analyse_matrix_and_rhs(filename_matrix, filename_rhs):  
    # read matrix and rhs from file
    mat_object = sio.loadmat(filename_matrix)
    matrix = mat_object["matrix"]
    rhs_object = sio.loadmat(filename_rhs)
    rhs = rhs_object["rhs"]
    
    # size of the matrix
    print("Matrix size: " + str(len(rhs)))
    print("Number of nonzeros: " + str(matrix.getnnz()) + " (" + str(round(float(matrix.getnnz()) / (len(rhs)**2) * 100.0, 3)) + " %)")
    
    # visualize matrix sparsity pattern
    fig = pl.figure()
    pl.spy(matrix, markersize=1)
    fn_pattern = pythonlab.tempname("png")
    pl.savefig(fn_pattern, dpi=60)
    pl.close(fig)   
    # show in console
    pythonlab.image(fn_pattern)

# store state
save_matrix_and_rhs = a2d.options.save_matrix_and_rhs
dump_format = a2d.options.dump_format

# dump format
a2d.options.save_matrix_and_rhs = True
a2d.options.dump_format = "matlab_mat"

# solve model
filename_matrix, filename_rhs = model_electrostatic()
analyse_matrix_and_rhs(filename_matrix, filename_rhs)
filename_matrix, filename_rhs = model_harmonic_magnetic()
analyse_matrix_and_rhs(filename_matrix, filename_rhs)

# restore state
a2d.options.save_matrix_and_rhs = save_matrix_and_rhs
a2d.options.dump_format = dump_format