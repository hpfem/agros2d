import pythonlab
import numpy as np
from scipy.sparse import coo_matrix
import pylab as pl

import agros2d as a2d

def model():
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
    electrostatic.adaptivity_type = "disabled"
    electrostatic.solver = "linear"
    
    # boundaries
    electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 10})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 0})
    electrostatic.add_material("Dielectric 2", {"electrostatic_permittivity" : 4, "electrostatic_charge_density" : 0})
    
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
    
    return electrostatic.filename_matrix(), electrostatic.filename_rhs()


# store state
save_matrix_and_rhs = a2d.options.save_matrix_and_rhs
a2d.options.save_matrix_and_rhs = True
# dump format
a2d.options.dump_format = "plain_ascii"

# solve model
filename_matrix, filename_rhs = model()

# restore state
a2d.options.save_matrix_and_rhs = save_matrix_and_rhs

# read matrix and rhs from file
matrix_triplet = np.loadtxt(filename_matrix)
rhs_vector = np.loadtxt(filename_rhs)

# size of the matrix
size = len(rhs_vector)
print("Matrix size: " + str(len(rhs_vector)))
print("Number of nonzeros: " + str(len(matrix_triplet)))

# convert matrix from COO triplet to CSR
matrix = coo_matrix((matrix_triplet[:, 2], (matrix_triplet[:, 0], matrix_triplet[:, 1])), shape=(len(rhs_vector), len(rhs_vector))).tocsr()
rhs = rhs_vector

# visualize matrix sparsity pattern
fig = pl.figure()
pl.spy(matrix, markersize=1)
fn_pattern = pythonlab.tempname("png")
pl.savefig(fn_pattern, dpi=60)
pl.close(fig)   
# show in console
pythonlab.image(fn_pattern)