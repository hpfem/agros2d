import pythonlab
import agros2d as a2d

import scipy.io as sio
import pylab as pl
import numpy as np

def model(solver):
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"
    problem.mesh_type = "triangle"
    
    # fields
    # electrostatic
    electrostatic = a2d.field("electrostatic")
    electrostatic.analysis_type = "steadystate"
    electrostatic.matrix_solver = solver
    electrostatic.number_of_refinements = 1
    electrostatic.polynomial_order = 2
    electrostatic.adaptivity_type = "disabled"
    electrostatic.solver = "linear"
    
    # boundaries
    electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1e9})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 1})
    electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 20})
    electrostatic.add_material("Dielectric 2", {"electrostatic_permittivity" : 4, "electrostatic_charge_density" : 30})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0, 0.2, 0, 0.08, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.01, 0.08, 0.01, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0.01, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.03, 0, 0.03, 0.08)
    geometry.add_edge(0.03, 0.08, 0.05, 0.08)
    geometry.add_edge(0.05, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.05, 0.08, 0.05, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.06, 0, 0.06, 0.08, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.05, 0.08, 0.06, 0.08, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.06, 0, 0.2, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.2, 0, 0, 0.2, angle = 90, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.01, 0.08, 0.03, 0.08)
    geometry.add_edge(0.01, 0.08, 0, 0.08, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
    
    geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric 1"})
    geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric 2"})
    geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})
    
    a2d.view.zoom_best_fit()
    problem.solve()
    
    print (electrostatic.filename_matrix())
    x
    
    return electrostatic.filename_matrix(), electrostatic.filename_rhs()
    
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

def read_matrix_and_rhs(matrix_filename, rhs_filename):
    mat_object = sio.loadmat(matrix_filename)
    matrix = mat_object["matrix"]
    rhs_object = sio.loadmat(rhs_filename)
    rhs = rhs_object["rhs"]
    
    return matrix, rhs
        
# store state
save_matrix_and_rhs = a2d.options.save_matrix_and_rhs
dump_format = a2d.options.dump_format

# dump format
a2d.options.save_matrix_and_rhs = True
a2d.options.dump_format = "matlab_mat"

# read reference matrix and rhs from file
reference_mat, reference_rhs = read_matrix_and_rhs(pythonlab.datadir() + "/resources/test/internal/test_matrix_solvers_matrix.mat", 
                                                   pythonlab.datadir() + "/resources/test/internal/test_matrix_solvers_rhs.mat")

# MUMPS
filename_mumps_matrix, filename_mumps_rhs = model("mumps")
mumps_mat, mumps_rhs = read_matrix_and_rhs(filename_mumps_matrix, filename_mumps_rhs)
# UMFPACK
filename_umfpack_matrix, filename_umfpack_rhs = model("umfpack")
umfpack_mat, umfpack_rhs = read_matrix_and_rhs(filename_umfpack_matrix, filename_umfpack_rhs)
# PARALUTION ITERATIVE
filename_paralution_iterative_matrix, filename_paralution_iterative_rhs = model("paralution_iterative")
paralution_iterative_mat,paralution_iterative_rhs = read_matrix_and_rhs(filename_paralution_iterative_matrix, filename_paralution_iterative_rhs)

# restore state
a2d.options.save_matrix_and_rhs = save_matrix_and_rhs
a2d.options.dump_format = dump_format

test_mumps_matrix = np.allclose(reference_mat.todense(), mumps_mat.todense(), rtol=1e-15, atol=1e-15)
if (not test_mumps_matrix):    
    print("MUMPS matrix failed.")
test_mumps_rhs = np.allclose(reference_rhs, mumps_rhs, rtol=1e-15, atol=1e-15)
if (not test_mumps_rhs):    
    print("MUMPS rhs failed.")
test_umfpack_matrix = np.allclose(reference_mat.todense(), umfpack_mat.todense(), rtol=1e-15, atol=1e-15)
if (not test_umfpack_matrix):    
    print("UMFPACK matrix failed.")
test_umfpack_rhs = np.allclose(reference_rhs, umfpack_rhs, rtol=1e-15, atol=1e-15)
if (not test_umfpack_rhs):    
    print("UMFPACK rhs failed.")
test_paralution_iterative_matrix = np.allclose(reference_mat.todense(), paralution_iterative_mat.todense(), rtol=1e-15, atol=1e-15)
if (not test_paralution_iterative_matrix):    
    print("PARALUTION matrix failed.")
test_paralution_iterative_rhs = np.allclose(reference_rhs, paralution_iterative_rhs, rtol=1e-15, atol=1e-15)
if (not test_paralution_iterative_rhs):    
    print("PARALUTION rhs failed.")
    
print("Test: Internal - matrix solvers: " + str(test_mumps_matrix and test_mumps_rhs and test_umfpack_matrix and test_umfpack_rhs and test_paralution_iterative_matrix and test_paralution_iterative_rhs))