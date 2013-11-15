import agros2d as a2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestField(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field('magnetic')

    """ field_id """
    def test_field_id(self):
        self.assertEqual(self.field.field_id, 'magnetic')

    """ analysis_type """
    def test_analysis_type(self):
        for type in ['steadystate', 'transient', 'harmonic']:
            self.field.analysis_type = type
            self.assertEqual(self.field.analysis_type, type)

    def test_set_wrong_analysis_type(self):
        with self.assertRaises(ValueError):
            self.field.analysis_type = 'wrong_analysis_type'

    """ solver """
    def test_solver(self):
        for solver in ['linear', 'newton']:
            self.field.solver = solver
            self.assertEqual(self.field.solver, solver)

    def test_set_wrong_solver(self):
        with self.assertRaises(ValueError):
            self.field.solver = 'wrong_solver'

    """ matrix_solver """
    def test_matrix_solver(self):
        for solver in ['umfpack', 'paralution_iterative', 'paralution_amg',
                       'mumps', 'external', 'empty']:
            self.field.matrix_solver = solver
            self.assertEqual(self.field.matrix_solver, solver)

    def test_set_wrong_matrix_solver(self):
        with self.assertRaises(ValueError):
            self.field.matrix_solver = 'wrong_solver'

    """ number_of_refinements """
    def test_number_of_refinements(self):
        self.field.number_of_refinements = 2
        self.assertEqual(self.field.number_of_refinements, 2)

    def test_set_wrong_number_of_refinements(self):
        with self.assertRaises(IndexError):
            self.field.number_of_refinements = -1

        with self.assertRaises(IndexError):
            self.field.number_of_refinements = 6

    """ polynomial_order """
    def test_polynomial_order(self):
        self.field.polynomial_order = 5
        self.assertEqual(self.field.polynomial_order, 5)

    def test_set_wrong_polynomial_order(self):
        with self.assertRaises(IndexError):
            self.field.polynomial_order = 0

        with self.assertRaises(IndexError):
            self.field.polynomial_order = 11

    """ adaptivity_type """
    def test_adaptivity_typer(self):
        for type in ['hp-adaptivity', 'h-adaptivity', 'p-adaptivity', 'disabled']:
            self.field.adaptivity_type = type
            self.assertEqual(self.field.adaptivity_type, type)

    def test_set_wrong_adaptivity_type(self):
        with self.assertRaises(ValueError):
            self.field.adaptivity_type = 'wrong_type'

    """ transient_initial_condition """
    def test_transient_initial_condition(self):
        self.field.transient_initial_condition= 293.15
        self.assertEqual(self.field.transient_initial_condition, 293.15)

    """ transient_time_skip """
    def test_transient_time_skip(self):
        self.field.transient_time_skip = 60
        self.assertEqual(self.field.transient_time_skip, 60)

class TestFieldBoundaries(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field('magnetic')
        self.field.analysis_type = 'harmonic'

    """ add_boundary """
    def test_add_boundary(self):
        self.field.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0,
                                                                "magnetic_potential_imag" : 0})

# TODO (Franta) :
"""
modify_boundary
remove_boundary
add_material
modify_material
remove_material
local_values
surface_integrals
volume_integrals
initial_mesh_info
solution_mesh_info
solver_info
adaptivity_info
filename_matrix
filename_rhs
"""

class TestFieldNewtonSolver(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field('magnetic')
        self.field.solver = 'newton'

    """ relative_change_of_solutions """
    def test_relative_change_of_solutions(self):
        self.field.solver_parameters['relative_change_of_solutions'] = 10
        self.assertEqual(self.field.solver_parameters['relative_change_of_solutions'], 10)

    def test_set_wrong_relative_change_of_solutions(self):
        with self.assertRaises(IndexError):
            self.field.solver_parameters['relative_change_of_solutions'] = -10

    """ residual """
    def test_residual(self):
        self.field.solver_parameters['residual'] = 0.5
        self.assertEqual(self.field.solver_parameters['residual'], 0.5)

    def test_set_wrong_tolerance(self):
        with self.assertRaises(IndexError):
            self.field.solver_parameters['residual'] = -0.1

    """ damping """
    def test_damping(self):
        for type in ['automatic', 'fixed', 'disabled']:
            self.field.solver_parameters['damping'] = type
            self.assertEqual(self.field.solver_parameters['damping'], type)

    def test_set_wrong_damping(self):
        with self.assertRaises(ValueError):
            self.field.solver_parameters['damping'] = 'wrong_damping'

    """ damping_factor """
    def test_damping_factor(self):
        self.field.solver_parameters['damping_factor'] = 0.5
        self.assertEqual(self.field.solver_parameters['damping_factor'], 0.5)

    def test_set_wrong_damping_factor(self):
        with self.assertRaises(IndexError):
            self.field.solver_parameters['damping_factor'] = -0.1

        with self.assertRaises(IndexError):
            self.field.solver_parameters['damping_factor'] = 1.1

    """ damping_factor_decrease_ratio """
    def test_damping_factor_decrease_ratio(self):
        self.field.solver_parameters['damping_factor_decrease_ratio'] = 2.2
        self.assertEqual(self.field.solver_parameters['damping_factor_decrease_ratio'], 2.2)

    """ damping_factor_increase_steps """
    def test_damping_factor_increase_steps(self):
        self.field.solver_parameters['damping_factor_increase_steps'] = 5
        self.assertEqual(self.field.solver_parameters['damping_factor_increase_steps'], 5)

    def test_set_wrong_damping_factor_increase_steps(self):
        with self.assertRaises(IndexError):
            self.field.solver_parameters['damping_factor_increase_steps'] = 0

        with self.assertRaises(IndexError):
            self.field.solver_parameters['damping_factor_increase_steps'] = 6

    """ jacobian_reuse """
    def test_jacobian_reuse(self):
        self.field.solver_parameters['jacobian_reuse'] = False
        self.assertEqual(self.field.solver_parameters['jacobian_reuse'], False)

    """ jacobian_reuse_ratio """
    def test_jacobian_reuse_ratio(self):
        self.field.solver_parameters['jacobian_reuse_ratio'] = 2.2
        self.assertEqual(self.field.solver_parameters['jacobian_reuse_ratio'], 2.2)

    """ jacobian_reuse_steps """
    def test_jacobian_reuse_steps(self):
        self.field.solver_parameters['jacobian_reuse_steps'] = 10
        self.assertEqual(self.field.solver_parameters['jacobian_reuse_steps'], 10)

    def test_set_wrong_jacobian_reuse_steps(self):
        with self.assertRaises(IndexError):
            self.field.solver_parameters['jacobian_reuse_steps'] = -1

        with self.assertRaises(IndexError):
            self.field.solver_parameters['jacobian_reuse_steps'] = 101

class TestFieldMatrixSolver(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field('magnetic')
        self.field.matrix_solver = 'paralution_iterative'

    """ preconditioner """
    def test_preconditioner(self):
        for preconditioner in ['jacobi', 'multicoloredsgs', 'ilu', 'multicoloredilu']:
            self.field.matrix_solver_parameters['preconditioner'] = preconditioner
            self.assertEqual(self.field.matrix_solver_parameters['preconditioner'], preconditioner)

    def test_set_wrong_preconditioner(self):
        with self.assertRaises(ValueError):
            self.field.matrix_solver_parameters['preconditioner'] = 'wrong_preconditioner'

    """ method """
    def test_method(self):
        for method in ['cg', 'gmres', 'bicgstab']:
            self.field.matrix_solver_parameters['method'] = method
            self.assertEqual(self.field.matrix_solver_parameters['method'], method)

    def test_set_wrong_method(self):
        with self.assertRaises(ValueError):
            self.field.matrix_solver_parameters['method'] = 'wrong_method'

    """ tolerance """
    def test_tolerance(self):
        self.field.matrix_solver_parameters['tolerance'] = 1e-5
        self.assertEqual(self.field.matrix_solver_parameters['tolerance'], 1e-5)

    def test_set_wrong_tolerance(self):
        with self.assertRaises(IndexError):
            self.field.matrix_solver_parameters['tolerance'] = -1

    """ iterations """
    def test_iterations(self):
        self.field.matrix_solver_parameters['iterations'] = 1e2
        self.assertEqual(self.field.matrix_solver_parameters['iterations'], 1e2)

    def test_set_wrong_jacobian_reuse_steps(self):
        with self.assertRaises(IndexError):
            self.field.matrix_solver_parameters['iterations'] = 0

        with self.assertRaises(IndexError):
            self.field.matrix_solver_parameters['iterations'] = 1.1e4

class TestFieldAdaptivity(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field('magnetic')
        self.field.adaptivity_type = 'hp-adaptivity'

    """ steps """
    def test_steps(self):
        self.field.adaptivity_parameters['steps'] = 20
        self.assertEqual(self.field.adaptivity_parameters['steps'], 20)

    def test_set_wrong_steps(self):
        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['steps'] = 0

        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['steps'] = 101

    """ tolerance """
    def test_tolerance(self):
        self.field.adaptivity_parameters['tolerance'] = 3
        self.assertEqual(self.field.adaptivity_parameters['tolerance'], 3)

    def test_set_wrong_tolerance(self):
        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['tolerance'] = -1

    """ threshold """
    def test_threshold(self):
        self.field.adaptivity_parameters['threshold'] = 0.8
        self.assertEqual(self.field.adaptivity_parameters['threshold'], 0.8)

    def test_set_wrong_threshold(self):
        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['threshold'] = 0.0

        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['threshold'] = 1.1

    """ stopping_criterion """
    def test_stopping_criterion(self):
        for criterion in ['cumulative', 'singleelement', 'levels']:
            self.field.adaptivity_parameters['stopping_criterion'] = criterion
            self.assertEqual(self.field.adaptivity_parameters['stopping_criterion'], criterion)

    def test_set_wrong_preconditioner(self):
        with self.assertRaises(ValueError):
            self.field.adaptivity_parameters['stopping_criterion'] = 'wrong_criterion'

    """ norm """
    """
    def test_norm(self):
        for norm in ['l2_norm', 'h1_norm', 'h1_seminorm']:
            self.field.adaptivity_parameters['norm'] = norm
            self.assertEqual(self.field.adaptivity_parameters['norm'], norm)

    def test_set_wrong_norm(self):
        with self.assertRaises(ValueError):
            self.field.adaptivity_parameters['norm'] = 'wrong_norm'
    """

    """ order_increase """
    def test_order_increase(self):
        self.field.adaptivity_parameters['order_increase'] = 5
        self.assertEqual(self.field.adaptivity_parameters['order_increase'], 5)

    def test_set_wrong_order_increase(self):
        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['order_increase'] = 0

        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['order_increase'] = 11

    """ space_refinement """
    def test_space_refinement(self):
        self.field.adaptivity_parameters['space_refinement'] = False
        self.assertEqual(self.field.adaptivity_parameters['space_refinement'], False)

    """ anisotropic_refinement """
    def test_anisotropic_refinement(self):
        self.field.adaptivity_parameters['anisotropic_refinement'] = False
        self.assertEqual(self.field.adaptivity_parameters['anisotropic_refinement'], False)

    """ finer_reference_solution """
    def test_finer_reference_solution(self):
        self.field.adaptivity_parameters['finer_reference_solution'] = True
        self.assertEqual(self.field.adaptivity_parameters['finer_reference_solution'], True)

    """ transient_redone_steps """
    def test_transient_redone_steps(self):
        self.field.adaptivity_parameters['transient_redone_steps'] = 10
        self.assertEqual(self.field.adaptivity_parameters['transient_redone_steps'], 10)

    def test_set_wrong_transient_redone_steps(self):
        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['transient_redone_steps'] = 0

        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['transient_redone_steps'] = 101

    """ transient_back_steps """
    def test_transient_back_steps(self):
        self.field.adaptivity_parameters['transient_back_steps'] = 10
        self.assertEqual(self.field.adaptivity_parameters['transient_back_steps'], 10)

    def test_set_wrong_transient_back_steps(self):
        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['transient_back_steps'] = -1

        with self.assertRaises(IndexError):
            self.field.adaptivity_parameters['transient_back_steps'] = 101

if __name__ == '__main__':
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestField))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldBoundaries))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldNewtonSolver))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldMatrixSolver))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldAdaptivity))
    suite.run(result)
