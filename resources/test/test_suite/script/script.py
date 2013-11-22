import agros2d
import pythonlab

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

def compare(first, second):
    for solution, file in zip(first, second):
        solution_values = list(solution.values())
        file_values = list(file.values())

        for i in range(len(solution_values)):
            if (round(solution_values[i], 5) != round(file_values[i], 5)):
                return False

    return True

def save_solution_test():
    problem = agros2d.problem()
    field = agros2d.field('magnetic')
    problem.solve()

    values_from_solution = [field.local_values(0.05, 0),
                            field.surface_integrals([0, 1, 2]),
                            field.volume_integrals()]

    from os import path
    filename = '{0}/temp.a2d'.format(path.dirname(pythonlab.tempname()))
    agros2d.save_file(filename, True)
    agros2d.open_file(filename, True)

    field = agros2d.field('magnetic')
    values_from_file = [field.local_values(0.05, 0),
                        field.surface_integrals([0, 1, 2]),
                        field.volume_integrals()]

    return compare(values_from_solution, values_from_file)


class TestSaveAdaptiveSolution(Agros2DTestCase):
    def setUp(self):
        self.problem = agros2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"

        self.magnetic = agros2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 1
        self.magnetic.adaptivity_type = "hp-adaptivity"
        self.magnetic.adaptivity_parameters['steps'] = 10
        self.magnetic.adaptivity_parameters['tolerance'] = 1
        self.magnetic.adaptivity_parameters['threshold'] = 0.6
        self.magnetic.adaptivity_parameters['stopping_criterion'] = "singleelement"
        self.magnetic.adaptivity_parameters['error_calculator'] = "h1"
        self.magnetic.adaptivity_parameters['anisotropic_refinement'] = True
        self.magnetic.adaptivity_parameters['finer_reference_solution'] = False
        self.magnetic.adaptivity_parameters['space_refinement'] = True
        self.magnetic.adaptivity_parameters['order_increase'] = 1
        self.magnetic.solver = "linear"

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_total_current_real" : 200, "magnetic_total_current_prescribed" : 1})
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 100, "magnetic_conductivity" : 10e6})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})

        geometry = agros2d.geometry
        geometry.add_edge(0, -0.25, 0.1, -0.25)
        geometry.add_edge(0.1, -0.25, 0.1, 0.25)
        geometry.add_edge(0.1, 0.25, 0, 0.25)
        geometry.add_edge(0, 0.25, 0, -0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, -0.15, 0.25, -0.15)
        geometry.add_edge(0.25, -0.15, 0.25, 0.15)
        geometry.add_edge(0.25, 0.15, 0.15, 0.15)
        geometry.add_edge(0.15, 0.15, 0.15, -0.15)
        geometry.add_edge(4.59243e-17, -0.75, 0.75, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.75, 0, 4.59243e-17, 0.75, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(4.59243e-17, 0.75, 0, 0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.25, 4.59243e-17, -0.75, boundaries = {"magnetic" : "A = 0"})
        
        geometry.add_label(0.5, 0, materials = {"magnetic" : "Air"})
        geometry.add_label(0.2, 0, materials = {"magnetic" : "Copper"})
        geometry.add_label(0.05, 0, materials = {"magnetic" : "Iron"})
        agros2d.view.zoom_best_fit()

    def test_steady_state(self):
        self.problem.solve()
        self.assertTrue(save_solution_test())

    def test_transient(self):
        self.problem.time_step_method = "fixed"
        self.problem.time_total = 3
        self.problem.time_steps = 1

        self.magnetic.analysis_type = "transient"
        self.problem.solve()
        self.assertTrue(save_solution_test())

if __name__ == '__main__':
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestSaveAdaptiveSolution))
    suite.run(result)
