import agros2d as a2d
from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from math import pi, sqrt

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
        self.field.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        a2d.geometry.add_edge(0, 0, 1, 1, boundaries = {'magnetic' : 'A = 0'})

    def test_add_existing_boundary(self):
        self.field.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        with self.assertRaises(ValueError):
            self.field.add_boundary("A = 0", "wrong_boundary_type", {"magnetic_potential_real" : 0})
    def test_add_boundary_with_wrong_type(self):
        with self.assertRaises(ValueError):
            self.field.add_boundary("A = 0", "wrong_boundary_type", {"magnetic_potential_real" : 0})
    def test_add_boundary_with_wrong_parameter(self):
        with self.assertRaises(ValueError):
            self.field.add_boundary("A = 0", "magnetic_potential", {"wrong_parameter" : 0})

    """ modify_boundary """
    def test_modify_nonexistent_boundary(self):
        with self.assertRaises(ValueError):
            self.field.modify_boundary("Nonexistent boundary", "magnetic_potential", {"magnetic_potential_real" : 0})
    def test_modify_boundary_with_wrong_type(self):
        self.field.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        with self.assertRaises(ValueError):
            self.field.add_boundary("A = 0", "wrong_boundary_type", {"magnetic_potential_real" : 0})
    def test_modify_boundary_with_wrong_parameter(self):
        self.field.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        with self.assertRaises(ValueError):
            self.field.add_boundary("A = 0", "magnetic_potential", {"wrong_parameter" : 0})

    """ remove_boundary """
    def test_remove_boundary(self):
        self.field.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        self.field.remove_boundary("A = 0")

        with self.assertRaises(ValueError):
            a2d.geometry.add_edge(0, 0, 1, 1, boundaries = {'magnetic' : 'A = 0'})

    def test_remove_nonexistent_boundary(self):
        self.field.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        with self.assertRaises(ValueError):
            self.field.remove_boundary("Nonexistent boundary")

class TestFieldMaterials(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field('magnetic')
        self.field.analysis_type = 'harmonic'

    """ add_material """
    def test_add_material(self):
        self.field.add_material("Iron", {"magnetic_permeability" : 1e3})
        a2d.geometry.add_label(0, 0, materials = {'magnetic' : 'Iron'})

    def test_add_existing_material(self):
        self.field.add_material("Iron", {"magnetic_permeability" : 1e3})
        with self.assertRaises(ValueError):
            self.field.add_material("Iron", {"magnetic_permeability" : 1e3})

    def test_add_material_with_wrong_parameter(self):
        with self.assertRaises(ValueError):
            self.field.add_material("Iron", {"wrong_parameter" : 1e3})

    """
    TODO (Franta)
    def test_add_material_with_wrong_parameter(self):
        with self.assertRaises(ValueError):
            self.field.add_material("Iron", {"magnetic_permeability" : -1e3})
    """

    def add_material(self, interpolation = "piecewise_linear",
                           extrapolation = "constant",
                           derivative = "first"):
        self.field.add_material("Iron", {"magnetic_permeability" : { "value" : 1e3,
                                                                     "x" : [0, 0.454, 1.1733, 1.4147, 1.7552, 1.8595, 1.9037, 1.9418],
                                                                     "y" : [9300, 9264, 6717.2, 4710.5, 1664.8, 763.14, 453.7, 194.13],
                                                                     "interpolation" : interpolation,
                                                                     "extrapolation" : extrapolation,
                                                                     "derivative_at_endpoints" : derivative}})

    def test_add_nonlinear_material(self):
        self.add_material()
        a2d.geometry.add_label(0, 0, materials = {'magnetic' : 'Iron'})

    def test_add_nonlinear_material_with_wrong_x_length(self):
        with self.assertRaises(ValueError):
            self.field.add_material("Iron", {"magnetic_permeability" : { "value" : 1e3,
                                                                         "x" : [0, 0.454, 1.1733, 1.4147, 1.7552, 1.8595, 1.9037],
                                                                         "y" : [9300, 9264, 6717.2, 4710.5, 1664.8, 763.14, 453.7, 194.13]}})

    def test_add_nonlinear_material_with_wrong_y_length(self):
        with self.assertRaises(ValueError):
            self.field.add_material("Iron", {"magnetic_permeability" : { "value" : 1e3,
                                                                         "x" : [0, 0.454, 1.1733, 1.4147, 1.7552, 1.8595, 1.9037, 1.9418],
                                                                         "y" : [9300, 9264, 6717.2, 4710.5, 1664.8, 763.14, 453.7]}})

    def test_add_nonlinear_material_with_nonascending_order(self):
        with self.assertRaises(ValueError):
            self.field.add_material("Iron", {"magnetic_permeability" : { "value" : 1e3,
                                                                         "x" : [0, 0.454, 1.4147, 1.7552, 1.8595, 1.9037, 1.1733],
                                                                         "y" : [9300, 9264, 4710.5, 1664.8, 763.14, 453.7, 6717.2]}})

    def test_add_nonlinear_material_with_interpolation(self):
        for interpolation in ['cubic_spline', 'piecewise_linear', 'constant']:
            self.add_material(interpolation = interpolation)
            self.field.remove_material('Iron')

        with self.assertRaises(ValueError):
            self.add_material(interpolation = "wrong_interpolation")

    def test_add_nonlinear_material_with_extrapolation(self):
        for extrapolation in ['constant', 'linear']:
            self.add_material(extrapolation = extrapolation)
            self.field.remove_material('Iron')

        with self.assertRaises(ValueError):
            self.add_material(extrapolation = "wrong_extrapolation")

    def test_add_nonlinear_material_with_derivative_at_endpoints(self):
        for derivative in ['first', 'second']:
            self.add_material(derivative = derivative)
            self.field.remove_material('Iron')

        with self.assertRaises(ValueError):
            self.add_material(derivative = "wrong_derivative")

    """ remove_material """
    def test_remove_material(self):
        self.add_material()
        self.field.remove_material("Iron")

        with self.assertRaises(ValueError):
            a2d.geometry.add_label(0, 0, materials = {'magnetic' : 'Iron'})

    def test_remove_nonexistent_material(self):
        self.add_material()
        with self.assertRaises(ValueError):
            self.field.remove_material("Nonexistent material")

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

class TestFieldLocalValues(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field("magnetic")
        self.field.number_of_refinements = 0
        self.field.polynomial_order = 1

        self.B = 0.75
        self.field.add_boundary("A={0}*x".format(self.B), "magnetic_potential",
                                {"magnetic_potential_real" : { "expression" : "{0}*x".format(self.B)}})
        self.field.add_material("Air", {"magnetic_permeability" : 1})

        self.size = 3.141592653589793
        geometry = a2d.geometry
        geometry.add_edge(self.size, self.size, 0, self.size, boundaries = {"magnetic" : "A=0.75*x"})
        geometry.add_edge(0, self.size, 0, 0, boundaries = {"magnetic" : "A=0.75*x"})
        geometry.add_edge(0, 0, self.size, 0, boundaries = {"magnetic" : "A=0.75*x"})
        geometry.add_edge(self.size, 0, self.size, self.size, boundaries = {"magnetic" : "A=0.75*x"})
        geometry.add_label(self.size/2.0, self.size/2.0, area = 0.1, materials = {"magnetic" : "Air"})

    def test_local_values(self):
        self.problem.solve()
        for (x, y) in [(self.size/4.0, self.size/4.0),
                       (self.size/2.0, self.size/2.0),
                       (3*self.size/4.0, 3*self.size/4.0)]:
            self.assertAlmostEqual(self.field.local_values(x, y)['Brx'], 0, 3)
            self.assertAlmostEqual(self.field.local_values(x, y)['Bry'], -self.B, 3)

    def test_local_values_outside_area(self):
        self.problem.solve()
        self.assertEqual(len(self.field.local_values(-1, -1)), 0)

    def test_local_values_without_solution(self):
        with self.assertRaises(RuntimeError):
            self.field.local_values(-1, -1)

class TestFieldIntegrals(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.field = a2d.field("electrostatic")

        self.field.add_boundary("Dirichlet", "electrostatic_potential", {"electrostatic_potential" : 1000})
        self.field.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        self.field.add_material("Source", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 10})
        self.field.add_material("Material", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})

        geometry = a2d.geometry
        geometry.add_edge(0.25, 0, 0.75, 0, boundaries = {"electrostatic" : "Dirichlet"})
        geometry.add_edge(0.75, 0, 0.75, 0.25, boundaries = {"electrostatic" : "Dirichlet"})
        geometry.add_edge(0.75, 0.25, 1.25, 0.75, angle = 90, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(1.25, 0.75, 0.75, 1.25, angle = 90, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.75, 1.25, 0.25, 0.75, angle = 90, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.25, 0.75, 0, 0.75, boundaries = {"electrostatic" : "Dirichlet"})
        geometry.add_edge(0, 0.75, 0, 0.25, boundaries = {"electrostatic" : "Dirichlet"})
        geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"electrostatic" : "Dirichlet"})
        geometry.add_edge(0.75, 0.5, 1, 0.75, angle = 90)
        geometry.add_edge(1, 0.75, 0.75, 1, angle = 90)
        geometry.add_edge(0.75, 1, 0.5, 0.75, angle = 90)
        geometry.add_edge(0.5, 0.75, 0.75, 0.5, angle = 90)
        geometry.add_edge(0.4, 0.1, 0.6, 0.1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.6, 0.1, 0.6, 0.3, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.6, 0.3, 0.4, 0.3, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.4, 0.3, 0.4, 0.1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.15, 0.45, 0.4, 0.45, boundaries = {"electrostatic" : "Dirichlet"})
        geometry.add_edge(0.4, 0.45, 0.15, 0.55, boundaries = {"electrostatic" : "Dirichlet"})
        geometry.add_edge(0.15, 0.55, 0.15, 0.45, boundaries = {"electrostatic" : "Dirichlet"})

        geometry.add_label(0.75, 0.75, materials = {"electrostatic" : "Source"})
        geometry.add_label(0.25, 0.25, materials = {"electrostatic" : "Material"})
        geometry.add_label(0.2, 0.5, materials = {"electrostatic" : "none"})
        geometry.add_label(0.5, 0.2, materials = {"electrostatic" : "none"})
        a2d.view.zoom_best_fit()
        
        self.volume = 0.75**2 + 3*(pi*0.5**2)/4.0 - (pi*0.25**2)/4.0 - 0.2**2 - (0.1*0.25)/2.0 - (pi*0.25**2)
        self.surface = (2*pi*0.25) + (0.25+0.1+sqrt(0.25**2+0.1**2))

    """ surface_integrals """
    def test_surface_integrals(self):
        self.problem.solve()
        self.assertAlmostEqual(self.field.surface_integrals([8,9,10,11,16,17,18])['l'], self.surface, 5)

    def test_surface_integrals_on_nonexistent_edge(self):
        self.problem.solve()
        with self.assertRaises(IndexError):
            self.field.surface_integrals([99])['l']

    def test_surface_itegrals_without_solution(self):
        with self.assertRaises(RuntimeError):
            self.field.surface_integrals()

    """ volume_integrals """
    def test_volume_integrals(self):
        self.problem.solve()
        self.assertAlmostEqual(self.field.volume_integrals([1])['V'], self.volume, 5)

    def test_volume_integrals_on_nonexistent_edge(self):
        self.problem.solve()
        with self.assertRaises(IndexError):
            self.field.volume_integrals([5])['V']

    def test_volume_integrals_without_solution(self):
        with self.assertRaises(RuntimeError):
            self.field.volume_integrals()

class TestFieldAdaptivityInfo(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"
        
        self.field = a2d.field("electrostatic")
        self.field.adaptivity_type = "hp-adaptivity"
        self.field.number_of_refinements = 1
        self.field.polynomial_order = 2
        self.field.adaptivity_parameters["steps"] = 10
        self.field.adaptivity_parameters["tolerance"] = 1
        self.field.solver = "linear"
        
        self.field.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
        self.field.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.field.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        self.field.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
        
        geometry = a2d.geometry
        geometry.add_edge(0.2, 0.6, 0, 0.1, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(0, 0.1, 0, -0.1, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -0.6, 0, -1.6, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, 0.6, 0.2, 0.6, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(0, 1.6, 0, 0.6, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -0.1, 0.2, -0.6, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0, -0.6, 0.2, -0.6, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0, 1.6, 1.6, 1.6, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(1.6, 1.6, 1.6, -1.6, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(1.6, -1.6, 0, -1.6, boundaries = {"electrostatic" : "Border"})
        
        geometry.add_label(0.8, 0.8, materials = {"electrostatic" : "Air"})

    def test_adaptivity_info(self):
        self.problem.solve()
        dofs = self.field.adaptivity_info()['dofs']
        error = self.field.adaptivity_info()['error']

        self.assertEqual(dofs, sorted(dofs))
        self.assertEqual(error, sorted(error, reverse=True))

    def test_adaptivity_info_without_solution(self):
        with self.assertRaises(RuntimeError):
            self.field.adaptivity_info()

    def test_adaptivity_info_without_adaptive_solution(self):
        self.field.adaptivity_type = "disabled"
        self.problem.solve()
        with self.assertRaises(RuntimeError):
            self.field.adaptivity_info()

    def test_adaptivity_info_with_solution_mode(self):
        self.problem.solve()
        normal_dofs = self.field.adaptivity_info(solution_type="normal")['dofs']
        reference_dofs = self.field.adaptivity_info(solution_type="reference")['dofs']
        self.assertGreater(sum(reference_dofs), sum(normal_dofs))

    def test_adaptivity_info_with_nonexisted_time_step(self):
        self.problem.solve()
        with self.assertRaises(IndexError):
            self.field.adaptivity_info(time_step=1)

    def test_initial_and_solution_mesh_info(self):
        self.problem.solve()
        init = self.field.initial_mesh_info()
        sol = self.field.solution_mesh_info()
        info = self.field.adaptivity_info()
        
        self.assertEqual(init['dofs'], info['dofs'][0])
        self.assertEqual(sol['dofs'], info['dofs'][-1])
        
        self.assertGreater(sol['dofs'], init['dofs'])
        self.assertGreater(sol['elements'], init['elements'])
        self.assertGreater(sol['nodes'], init['nodes'])

    def test_initial_mesh_info(self):
        self.problem.mesh()
        self.assertTrue(self.field.initial_mesh_info()['elements'])
        self.assertTrue(self.field.initial_mesh_info()['nodes'])

    def test_solution_mesh_info(self):
        self.problem.solve()
        self.assertTrue(self.field.solution_mesh_info()['dofs'])
        self.assertTrue(self.field.solution_mesh_info()['elements'])
        self.assertTrue(self.field.solution_mesh_info()['nodes'])

    def test_initial_mesh_info_without_mesh(self):
        with self.assertRaises(RuntimeError):
            self.field.initial_mesh_info()

    def test_solution_mesh_info_without_solution(self):
        with self.assertRaises(RuntimeError):
            self.field.solution_mesh_info()

class TestFieldAdaptivityInfoTransient(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.problem.time_step_method = "fixed"
        self.problem.time_method_order = 2
        self.problem.time_total = 10
        self.problem.time_steps = 10
        
        self.field = a2d.field("heat")
        self.field.analysis_type = "transient"
        self.field.transient_initial_condition = 293.15
        self.field.number_of_refinements = 0
        self.field.polynomial_order = 1
        self.field.adaptivity_type = "hp-adaptivity"

        self.field.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0,
                                                              "heat_convection_external_temperature" : 0, "heat_radiation_emissivity" : 0,
                                                              "heat_radiation_ambient_temperature" : 0})
        self.field.add_boundary("Dirichlet", "heat_temperature", {"heat_temperature" : { "expression" : "293.15*(time<4) + 393.15*(time>=4)" }})

        self.field.add_material("Material", {"heat_conductivity" : 237, "heat_volume_heat" : 0, "heat_density" : 2700, "heat_specific_heat" : 896})

        geometry = a2d.geometry
        geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0, 0.25, -0.25, 0, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(-0.25, 0, 0, -0.25, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0, -0.25, 0.25, 0, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.05, 0, 0, 0.05, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0, 0.05, -0.05, 0, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.05, 0, 0, -0.05, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0, -0.05, 0.05, 0, boundaries = {"heat" : "Dirichlet"})

        geometry.add_label(0, 0, materials = {"heat" : "none"})
        geometry.add_label(0.15, 0, materials = {"heat" : "Material"})
        a2d.view.zoom_best_fit()

    def test_adaptivity_info_transient(self):
        self.problem.solve()
        
        for step in range(self.problem.time_steps):
            dofs = self.field.adaptivity_info(time_step=step)['dofs']

            if (step < 4):
                self.assertEqual(len(dofs), 1)
            else:
                self.assertNotEqual(len(dofs), 1)

class TestFieldSolverInfo(Agros2DTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "planar"
        self.problem.mesh_type = "triangle"
        self.problem.time_step_method = "fixed"
        self.problem.time_method_order = 1
        self.problem.time_total = 5
        self.problem.time_steps = 5

        self.field = a2d.field("heat")
        self.field.analysis_type = "transient"
        self.field.matrix_solver = "mumps"
        self.field.transient_initial_condition = 293.15
        self.field.number_of_refinements = 0
        self.field.polynomial_order = 1
        self.field.adaptivity_type = "hp-adaptivity"
        self.field.adaptivity_parameters['steps'] = 5
        self.field.adaptivity_parameters['tolerance'] = 10
        self.field.adaptivity_parameters['threshold'] = 0.6
        self.field.adaptivity_parameters['stopping_criterion'] = "singleelement"
        self.field.adaptivity_parameters['error_calculator'] = "h1"
        self.field.adaptivity_parameters['anisotropic_refinement'] = False
        self.field.adaptivity_parameters['finer_reference_solution'] = False
        self.field.adaptivity_parameters['space_refinement'] = True
        self.field.adaptivity_parameters['order_increase'] = 1
        self.field.adaptivity_parameters['transient_back_steps'] = 3
        self.field.adaptivity_parameters['transient_redone_steps'] = 5
        self.field.solver = "picard"
        self.field.solver_parameters['residual'] = 0.001
        self.field.solver_parameters['relative_change_of_solutions'] = 0.5
        self.field.solver_parameters['damping'] = "automatic"
        self.field.solver_parameters['damping_factor'] = 0.8
        self.field.solver_parameters['damping_factor_increase_steps'] = 1
        self.field.solver_parameters['damping_factor_decrease_ratio'] = 1.2
        self.field.solver_parameters['anderson_acceleration'] = False
        
        self.field.add_boundary("Dirichlet", "heat_temperature", {"heat_temperature" : { "expression" : "293.15+1e3*(exp(-1/2*time) - exp(-10*time))" }})
        self.field.add_boundary("Neumann", "heat_heat_flux", {"heat_heat_flux" : 0,
                                                         "heat_convection_heat_transfer_coefficient" : 1,
                                                         "heat_convection_external_temperature" : 293.15,
                                                         "heat_radiation_emissivity" : 0.1,
                                                         "heat_radiation_ambient_temperature" : 293.15})
        
        self.field.add_material("Material", {"heat_conductivity" : { "value" : 385,
                                                                "x" : [0,10.2764,20.5528,30.8291,41.1055,51.3819,61.6583,71.9347,82.2111,92.4874,102.764,113.04,123.317,133.593,143.869,154.146,164.422,174.698,184.975,195.251,205.528,215.804,226.08,236.357,246.633,256.91,267.186,277.462,287.739,298.015,308.291,318.568,328.844,339.121,349.397,359.673,369.95,380.226,390.503,400.779,411.055,421.332,431.608,441.884,452.161,462.437,472.714,482.99,493.266,503.543,513.819,524.095,534.372,544.648,554.925,565.201,575.477,585.754,596.03,606.307,616.583,626.859,637.136,647.412,657.688,667.965,678.241,688.518,698.794,709.07,719.347,729.623,739.899,750.176,760.452,770.729,781.005,791.281,801.558,811.834,822.111,832.387,842.663,852.94,863.216,873.492,883.769,894.045,904.322,914.598,924.874,935.151,945.427,955.704,965.98,976.256,986.533,996.809,1007.09,1017.36,1027.64,1037.91,1048.19,1058.47,1068.74,1079.02,1089.3,1099.57,1109.85,1120.13,1130.4,1140.68,1150.95,1161.23,1171.51,1181.78,1192.06,1202.34,1212.61,1222.89,1233.17,1243.44,1253.72,1263.99,1274.27,1284.55,1294.82,1305.1,1315.38,1325.65,1335.93,1346.21,1356.48,1366.76,1377.04,1387.31,1397.59,1407.86,1418.14,1428.42,1438.69,1448.97,1459.25,1469.52,1479.8,1490.08,1500.35,1510.63,1520.9,1531.18,1541.46,1551.73,1562.01,1572.29,1582.56,1592.84,1603.12,1613.39,1623.67,1633.94,1644.22,1654.5,1664.77,1675.05,1685.33,1695.6,1705.88,1716.16,1726.43,1736.71,1746.98,1757.26,1767.54,1777.81,1788.09,1798.37,1808.64,1818.92,1829.2,1839.47,1849.75,1860.03,1870.3,1880.58,1890.85,1901.13,1911.41,1921.68,1931.96,1942.24,1952.51,1962.79,1973.07,1983.34,1993.62,2003.89,2014.17,2024.45,2034.72,2045],
                                                                "y" : [2.09699e-08,1221.98,474.48,202.952,137.155,106.398,92.888,85.2292,80.5891,78.6785,77.2066,76.1895,75.392,74.7574,74.2412,73.8093,73.4374,73.109,72.814,72.5478,72.3097,72.1012,71.925,71.7836,71.6776,71.6047,71.5582,71.5255,71.559,71.542,71.5294,71.5213,71.5175,71.518,71.5228,71.5318,71.545,71.5623,71.5838,71.6093,71.6389,71.6725,71.71,71.7514,71.7967,71.8459,71.8988,71.9555,72.0159,72.0799,72.1476,72.2189,72.2938,72.3722,72.454,72.5393,72.6279,72.72,72.8153,72.9139,73.0157,73.1208,73.229,73.3403,73.4547,73.5721,73.6925,73.8158,73.9421,74.0713,74.2033,74.338,74.4756,74.6158,74.7588,74.9043,75.0525,75.2032,75.3564,75.5121,75.6703,75.8308,75.9937,76.1589,76.3264,76.4961,76.668,76.842,77.0182,77.1965,77.3768,77.5591,77.7433,77.9295,78.1175,78.3074,78.4991,78.6925,78.8877,79.0844,79.283,79.4829,79.6846,79.8878,80.0922,80.2983,80.5059,80.7145,80.9247,81.1362,81.3487,81.5626,81.7774,81.9936,82.2108,82.4289,82.6482,82.8684,83.0894,83.3114,83.5343,83.7578,83.9822,84.2071,84.433,84.6594,84.8863,85.1139,85.3421,85.5705,85.7995,86.029,86.2586,86.4887,86.7191,86.9496,87.1804,87.4112,87.6423,87.8735,88.1046,88.3358,88.567,88.7979,89.0289,89.2598,89.4902,89.7206,89.9505,90.1803,90.4097,90.6385,90.8671,91.0953,91.3226,91.5497,91.7761,92.0017,92.2268,92.451,92.6746,92.8975,93.1193,93.3404,93.5606,93.7796,93.9978,94.215,94.4309,94.6459,94.8596,95.0722,95.2836,95.4935,95.7023,95.9098,96.1156,96.3202,96.5233,96.7248,96.9248,97.1232,97.3199,97.515,97.7082,97.8999,98.0897,98.2775,98.4637,98.6479,98.83,99.0102,99.1885,99.3644,99.5384,99.7101,99.8798,100.047,100.212,100.375],
                                                                "interpolation" : "piecewise_linear",
                                                                "extrapolation" : "constant",
                                                                "derivative_at_endpoints" : "first" },
                                         "heat_density" : { "value" : 0,
                                                            "x" : [0,10.0503,20.1005,30.1508,40.201,50.2513,60.3015,70.3518,80.402,90.4523,100.503,110.553,120.603,130.653,140.704,150.754,160.804,170.854,180.905,190.955,201.005,211.055,221.106,231.156,241.206,251.256,261.307,271.357,281.407,291.457,301.508,311.558,321.608,331.658,341.709,351.759,361.809,371.859,381.91,391.96,402.01,412.06,422.111,432.161,442.211,452.261,462.312,472.362,482.412,492.462,502.513,512.563,522.613,532.663,542.714,552.764,562.814,572.864,582.915,592.965,603.015,613.065,623.116,633.166,643.216,653.266,663.317,673.367,683.417,693.467,703.518,713.568,723.618,733.668,743.719,753.769,763.819,773.869,783.92,793.97,804.02,814.07,824.121,834.171,844.221,854.271,864.322,874.372,884.422,894.472,904.523,914.573,924.623,934.673,944.724,954.774,964.824,974.874,984.925,994.975,1005.03,1015.08,1025.13,1035.18,1045.23,1055.28,1065.33,1075.38,1085.43,1095.48,1105.53,1115.58,1125.63,1135.68,1145.73,1155.78,1165.83,1175.88,1185.93,1195.98,1206.03,1216.08,1226.13,1236.18,1246.23,1256.28,1266.33,1276.38,1286.43,1296.48,1306.53,1316.58,1326.63,1336.68,1346.73,1356.78,1366.83,1376.88,1386.93,1396.98,1407.04,1417.09,1427.14,1437.19,1447.24,1457.29,1467.34,1477.39,1487.44,1497.49,1507.54,1517.59,1527.64,1537.69,1547.74,1557.79,1567.84,1577.89,1587.94,1597.99,1608.04,1618.09,1628.14,1638.19,1648.24,1658.29,1668.34,1678.39,1688.44,1698.49,1708.54,1718.59,1728.64,1738.69,1748.74,1758.79,1768.84,1778.89,1788.94,1798.99,1809.05,1819.1,1829.15,1839.2,1849.25,1859.3,1869.35,1879.4,1889.45,1899.5,1909.55,1919.6,1929.65,1939.7,1949.75,1959.8,1969.85,1979.9,1989.95,2000],
                                                            "y" : [3.28135e-12,1.18602,8.02656,22.3954,39.2071,55.2185,68.8198,79.6117,87.9801,94.6733,100.378,105.293,108.95,112.192,114.966,117.34,119.377,121.131,122.655,123.992,125.179,126.251,127.231,128.142,128.996,129.803,130.563,131.274,131.926,132.465,132.776,133.085,133.391,133.696,133.998,134.298,134.596,134.892,135.186,135.478,135.768,136.056,136.342,136.627,136.91,137.191,137.47,137.748,138.025,138.299,138.573,138.845,139.115,139.385,139.653,139.92,140.185,140.45,140.713,140.976,141.237,141.498,141.757,142.016,142.274,142.531,142.787,143.043,143.298,143.553,143.807,144.06,144.313,144.566,144.818,145.07,145.322,145.574,145.825,146.076,146.327,146.579,146.83,147.081,147.332,147.584,147.836,148.088,148.34,148.593,148.846,149.099,149.353,149.607,149.862,150.118,150.374,150.631,150.889,151.148,151.407,151.667,151.929,152.191,152.454,152.718,152.984,153.25,153.518,153.787,154.058,154.33,154.603,154.877,155.153,155.431,155.71,155.991,156.274,156.558,156.844,157.132,157.421,157.713,158.007,158.302,158.6,158.899,159.201,159.505,159.811,160.12,160.431,160.744,161.059,161.377,161.698,162.021,162.347,162.675,163.006,163.34,163.677,164.016,164.358,164.704,165.052,165.403,165.757,166.114,166.475,166.838,167.205,167.576,167.949,168.326,168.706,169.09,169.477,169.868,170.263,170.661,171.063,171.468,171.877,172.291,172.708,173.129,173.554,173.983,174.416,174.853,175.294,175.74,176.19,176.644,177.102,177.565,178.033,178.504,178.981,179.462,179.948,180.438,180.933,181.432,181.937,182.446,182.96,183.479,184.003,184.533,185.067,185.606,186.151,186.701,187.256,187.816,188.382,188.953],
                                                            "interpolation" : "piecewise_linear",
                                                            "extrapolation" : "constant",
                                                            "derivative_at_endpoints" : "first" },
                                         "heat_specific_heat" : { "value" : 0,
                                                                  "x" : [10,19.8643,29.7286,39.593,49.4573,59.3216,69.1859,79.0503,88.9146,98.7789,108.643,118.508,128.372,138.236,148.101,157.965,167.829,177.693,187.558,197.422,207.286,217.151,227.015,236.879,246.744,256.608,266.472,276.337,286.201,296.065,305.93,315.794,325.658,335.523,345.387,355.251,365.116,374.98,384.844,394.709,404.573,414.437,424.302,434.166,444.03,453.894,463.759,473.623,483.487,493.352,503.216,513.08,522.945,532.809,542.673,552.538,562.402,572.266,582.131,591.995,601.859,611.724,621.588,631.452,641.317,651.181,661.045,670.91,680.774,690.638,700.503,710.367,720.231,730.095,739.96,749.824,759.688,769.553,779.417,789.281,799.146,809.01,818.874,828.739,838.603,848.467,858.332,868.196,878.06,887.925,897.789,907.653,917.518,927.382,937.246,947.111,956.975,966.839,976.704,986.568,996.432,1006.3,1016.16,1026.03,1035.89,1045.75,1055.62,1065.48,1075.35,1085.21,1095.08,1104.94,1114.8,1124.67,1134.53,1144.4,1154.26,1164.13,1173.99,1183.85,1193.72,1203.58,1213.45,1223.31,1233.18,1243.04,1252.9,1262.77,1272.63,1282.5,1292.36,1302.23,1312.09,1321.95,1331.82,1341.68,1351.55,1361.41,1371.28,1381.14,1391.01,1400.87,1410.73,1420.6,1430.46,1440.33,1450.19,1460.06,1469.92,1479.78,1489.65,1499.51,1509.38,1519.24,1529.11,1538.97,1548.83,1558.7,1568.56,1578.43,1588.29,1598.16,1608.02,1617.88,1627.75,1637.61,1647.48,1657.34,1667.21,1677.07,1686.93,1696.8,1706.66,1716.53,1726.39,1736.26,1746.12,1755.98,1765.85,1775.71,1785.58,1795.44,1805.31,1815.17,1825.04,1834.9,1844.76,1854.63,1864.49,1874.36,1884.22,1894.09,1903.95,1913.81,1923.68,1933.54,1943.41,1953.27,1963.14,1973],
                                                                  "y" : [21512.1,21511.9,21511.4,21510.1,21508.1,21505.5,21502.3,21499,21495.3,21491.3,21487,21482.6,21477.9,21473.1,21468.2,21463.1,21458,21452.8,21447.6,21442.3,21437,21431.7,21426.3,21420.8,21415.3,21409.7,21403.9,21398.1,21392.6,21386.8,21381,21375.3,21369.5,21363.6,21357.8,21352,21346.2,21340.3,21334.5,21328.6,21322.7,21316.8,21310.9,21305,21299.1,21293.2,21287.3,21281.3,21275.3,21269.4,21263.4,21257.4,21251.4,21245.4,21239.4,21233.3,21227.3,21221.2,21215.1,21209,21202.9,21196.8,21190.7,21184.6,21178.4,21172.2,21166.1,21159.9,21153.7,21147.5,21141.2,21135,21128.7,21122.5,21116.2,21109.9,21103.6,21097.2,21090.9,21084.5,21078.1,21071.8,21065.4,21058.9,21052.5,21046.1,21039.6,21033.1,21026.6,21020.1,21013.6,21007.1,21000.5,20993.9,20987.3,20980.7,20974.1,20967.5,20960.8,20954.2,20947.5,20940.8,20934.1,20927.3,20920.6,20913.8,20907,20900.2,20893.4,20886.6,20879.7,20872.9,20866,20859.1,20852.2,20845.2,20838.3,20831.3,20824.3,20817.3,20810.3,20803.2,20796.2,20789.1,20782,20774.9,20767.8,20760.6,20753.4,20746.3,20739,20731.8,20724.6,20717.3,20710,20702.8,20695.4,20688.1,20680.8,20673.4,20666,20658.6,20651.2,20643.7,20636.3,20628.8,20621.3,20613.8,20606.2,20598.7,20591.1,20583.5,20575.9,20568.3,20560.6,20552.9,20545.3,20537.6,20529.8,20522.1,20514.3,20506.5,20498.7,20490.9,20483.1,20475.2,20467.3,20459.4,20451.5,20443.6,20435.6,20427.7,20419.7,20411.7,20403.6,20395.6,20387.5,20379.4,20371.3,20363.2,20355.1,20346.9,20338.7,20330.5,20322.3,20314.1,20305.8,20297.5,20289.2,20280.9,20272.6,20264.2,20255.9,20247.5,20239.1,20230.6,20222.2,20213.7,20205.2,20196.7],
                                                                  "interpolation" : "piecewise_linear",
                                                                  "extrapolation" : "constant",
                                                                  "derivative_at_endpoints" : "first" }})
        
        geometry = a2d.geometry
        geometry.add_edge(0, -0.15, 0.15, 0, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.15, 0, 0, 0.15, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0, 0.15, -0.15, 0, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(-0.15, 0, 0, -0.15, angle = 90, boundaries = {"heat" : "Neumann"})
        geometry.add_edge(0.01, 0, 0.023097, 0.00956709, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0.023097, 0.00956709, 0.00707107, 0.00707107, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0.00707107, 0.00707107, 0.00956709, 0.023097, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0.00956709, 0.023097, 6.12323e-19, 0.01, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(6.12323e-19, 0.01, -0.00956709, 0.023097, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.00956709, 0.023097, -0.00707107, 0.00707107, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.00707107, 0.00707107, -0.023097, 0.00956709, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.023097, 0.00956709, -0.01, 1.22465e-18, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.01, 1.22465e-18, -0.023097, -0.00956709, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.023097, -0.00956709, -0.00707107, -0.00707107, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.00707107, -0.00707107, -0.00956709, -0.023097, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-0.00956709, -0.023097, -3.82857e-18, -0.01, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(-3.82857e-18, -0.01, 0.00956709, -0.023097, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0.00956709, -0.023097, 0.00707107, -0.00707107, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0.00707107, -0.00707107, 0.023097, -0.00956709, boundaries = {"heat" : "Dirichlet"})
        geometry.add_edge(0.023097, -0.00956709, 0.01, 0, boundaries = {"heat" : "Dirichlet"})
        geometry.add_label(0, 0, materials = {"heat" : "none"})
        geometry.add_label(0.075, 0, materials = {"heat" : "Material"})

    def test_solver_info_with_wrong_parameters(self):
        self.problem.solve()
        with self.assertRaises(IndexError):
            self.field.solver_info(time_step=99)

    def test_solver_info_without_solution(self):
        with self.assertRaises(RuntimeError):
            self.field.solver_info()

    """
    def test_solution_change(self):
        self.assertEqual(len(self.field.solver_info()['solution_change']),
                         len(self.field.solver_info()['residual']))

    def test_residual(self):
        self.problem.solve()
        residual = self.field.solver_info()['residual']
        
        self.assertEqual(residual, sorted(residual, reverse=True))
        self.assertEqual(len(residual), len(self.field.solver_info()['solution_change']))

    def test_solver_info(self):
        self.assertGreater(self.field.solver_info()['jacobian_calculations'], 0)
        self.assertLess(self.field.solver_info()['jacobian_calculations'],
                        len(self.field.solver_info()['residual']))
    """

if __name__ == '__main__':
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    """
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestField))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldBoundaries))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldMaterials))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldNewtonSolver))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldMatrixSolver))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldAdaptivity))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldLocalValues))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldIntegrals))
    """
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldAdaptivityInfo))
    #suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldAdaptivityInfoTransient))
    #suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestFieldSolverInfo))
    suite.run(result)

# TODO (Franta) :
"""
modify_material
filename_matrix
filename_rhs
"""