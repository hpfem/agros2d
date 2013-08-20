import pythonlab
import agros2d 

import scipy.io as sio
import numpy as np

class InternalMatrixSolvers(agros2d.Agros2DTestCase):
    @classmethod
    def setUpClass(self): 
        # store state
        self.save_matrix_and_rhs = agros2d.options.save_matrix_and_rhs
        self.dump_format = agros2d.options.dump_format
        
        # dump format
        agros2d.options.save_matrix_and_rhs = True
        agros2d.options.dump_format = "matlab_mat"
        
        # read reference matrix and rhs from file
        self.reference_mat, self.reference_rhs = self.read_matrix_and_rhs(pythonlab.datadir("/resources/test/internal/matrix_solvers_matrix.mat"), 
                                                                          pythonlab.datadir("/resources/test/internal/matrix_solvers_rhs.mat"))

    @classmethod
    def tearDownClass(self):
        # restore state
        agros2d.options.save_matrix_and_rhs = self.save_matrix_and_rhs
        agros2d.options.dump_format = self.dump_format

    @classmethod
    def model(self, solver):
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # fields
        # electrostatic
        electrostatic = agros2d.field("electrostatic")
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
        geometry = agros2d.geometry
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
        
        agros2d.view.zoom_best_fit()
        problem.solve()
        
        return electrostatic.filename_matrix(), electrostatic.filename_rhs()
    
    @classmethod    
    def analyse_matrix_and_rhs(self, filename_matrix, filename_rhs):
        import pylab as pl
   
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

    @classmethod        
    def read_matrix_and_rhs(self, matrix_filename, rhs_filename):
        mat_object = sio.loadmat(matrix_filename)
        matrix = mat_object["matrix"]
        rhs_object = sio.loadmat(rhs_filename)
        rhs = rhs_object["rhs"]
        
        return matrix, rhs
        
    def test_mumps(self):
        # MUMPS
        filename_mumps_matrix, filename_mumps_rhs = self.model("mumps")
        mumps_mat, mumps_rhs = self.read_matrix_and_rhs(filename_mumps_matrix, filename_mumps_rhs)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), mumps_mat.todense(), rtol=1e-15, atol=1e-15), 
                        "MUMPS matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, mumps_rhs, rtol=1e-15, atol=1e-15), 
                        "MUMPS rhs failed.")

    def test_umfpack(self):
        # UMFPACK
        filename_umfpack_matrix, filename_umfpack_rhs = self.model("umfpack")
        umfpack_mat, umfpack_rhs = self.read_matrix_and_rhs(filename_umfpack_matrix, filename_umfpack_rhs)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), umfpack_mat.todense(), rtol=1e-15, atol=1e-15), 
                        "UMFPACK matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, umfpack_rhs, rtol=1e-15, atol=1e-15), 
                        "UMFPACK rhs failed.")

    def test_paralution_iter(self):
        # PARALUTION - iterative
        filename_paralution_iterative_matrix, filename_paralution_iterative_rhs = self.model("paralution_iterative")
        paralution_iterative_mat, paralution_iterative_rhs = self.read_matrix_and_rhs(filename_paralution_iterative_matrix, filename_paralution_iterative_rhs)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), paralution_iterative_mat.todense(), rtol=1e-15, atol=1e-15), 
                        "PARALUTION iterative matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, paralution_iterative_rhs, rtol=1e-15, atol=1e-15), 
                        "PARALUTION iterative rhs failed.")
                        
    def test_paralution_amg(self):
        # PARALUTION - amg
        filename_paralution_amg_matrix, filename_paralution_amg_rhs = self.model("paralution_amg")
        paralution_amg_mat, paralution_amg_rhs = self.read_matrix_and_rhs(filename_paralution_amg_matrix, filename_paralution_amg_rhs)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), paralution_amg_mat.todense(), rtol=1e-15, atol=1e-15), 
                        "PARALUTION AMG matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, paralution_amg_rhs, rtol=1e-15, atol=1e-15), 
                        "PARALUTION AMG rhs failed.")
        
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = agros2d.Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(InternalMatrixSolvers))
    suite.run(result)