import agros2d as a2d
import pythonlab

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from scipy.misc import imread, imresize
from scipy.linalg import norm
from scipy import sum, average

import os.path

def simple_model():
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"

    field = a2d.field("electrostatic")
    field.adaptivity_type = "hp-adaptivity"
    field.number_of_refinements = 1
    field.polynomial_order = 2
    field.adaptivity_parameters["steps"] = 10
    field.adaptivity_parameters["tolerance"] = 1
    
    field.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
    field.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    field.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    field.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    
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

def adaptive_model():
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "planar"
    problem.frequency = 5000
    
    magnetic = a2d.field("magnetic")
    magnetic.analysis_type = "harmonic"
    magnetic.matrix_solver = "mumps"
    magnetic.number_of_refinements = 0
    magnetic.polynomial_order = 1
    magnetic.adaptivity_type = "hp-adaptivity"
    magnetic.adaptivity_parameters['steps'] = 10
    magnetic.adaptivity_parameters['tolerance'] = 0.05
    magnetic.adaptivity_parameters['threshold'] = 0.6
    magnetic.adaptivity_parameters['stopping_criterion'] = "singleelement"
    magnetic.adaptivity_parameters['error_calculator'] = "h1"
    magnetic.adaptivity_parameters['anisotropic_refinement'] = True
    magnetic.adaptivity_parameters['finer_reference_solution'] = False
    magnetic.adaptivity_parameters['space_refinement'] = True
    magnetic.adaptivity_parameters['order_increase'] = 1
    magnetic.solver = "linear"
    
    magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})
    magnetic.add_material("Conductor", {"magnetic_permeability" : 1, "magnetic_conductivity" : 1e7, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 70, "magnetic_total_current_imag" : 70})
    magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0, "magnetic_total_current_prescribed" : 0, "magnetic_total_current_real" : 0, "magnetic_total_current_imag" : 0})
    
    geometry = a2d.geometry
    geometry.add_edge(0, -0.01, 0.01, 0)
    geometry.add_edge(0.01, 0, 0, 0.01)
    geometry.add_edge(0, 0.01, -0.01, 0)
    geometry.add_edge(-0.01, 0, 0, -0.01)
    geometry.add_edge(-1.83697e-17, -0.1, 0.1, 0, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(0.1, 0, 6.12323e-18, 0.1, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(6.12323e-18, 0.1, -0.1, 1.22465e-17, boundaries = {"magnetic" : "A = 0"})
    geometry.add_edge(-0.1, 1.22465e-17, -1.83697e-17, -0.1, boundaries = {"magnetic" : "A = 0"})
    geometry.add_label(0, 0, materials = {"magnetic" : "Conductor"})
    geometry.add_label(0.05, 0, materials = {"magnetic" : "Air"})

class TestView(Agros2DTestCase):
    def resize(self, image, size):
        return imresize(image, size)

    def grayscale(self, image):
        if len(image.shape) == 3:
            return average(image, -1)
        else:
            return image

    def normalize(self, image):
        rng = image.max() - image.min()
        return (image - image.min())*255/rng

    def compare(self, image, reference_image):
        cmp_img = self.normalize(image)
        ref_img = self.normalize(reference_image)
        
        if (cmp_img.size > ref_img.size):
            cmp_img = self.resize(cmp_img, ref_img.shape)
        elif (cmp_img.size < ref_img.size):
            ref_img = self.resize(ref_img, cmp_img.shape)

        diff = cmp_img - ref_img
        Nm = sum(abs(diff))
        Nf = norm(diff.ravel(), 0)

        return Nm, Nf
 
    def process(self, name):
        image_file = pythonlab.tempname('png')
        reference_image_file = pythonlab.datadir('resources/test/test_suite/script/images/{0}.png'.format(name))

        if os.path.exists(reference_image_file):
            a2d.view.save_image(image_file)
            manhattan_norm, frobenius_norm = self.compare(self.grayscale(imread(image_file).astype(float)),
                                                          self.grayscale(imread(reference_image_file).astype(float)))
            print(manhattan_norm, frobenius_norm)
            self.assertLess(manhattan_norm, 1e6)
            self.assertAlmostEqual(frobenius_norm, 0, 1e5)
        else:
            a2d.view.save_image(reference_image_file)

class TestMeshViewSimpleProblem(TestView):
    @classmethod
    def setUpClass(cls):
        simple_model()
        a2d.problem().solve()

    def setUp(self):
        a2d.view.mesh.activate()
        a2d.view.mesh.disable()
        a2d.view.zoom_best_fit()

    def test_initial_mesh(self):
        a2d.view.mesh.initial_mesh = True
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-initial_mesh')

    def test_solution_mesh(self):
        a2d.view.mesh.solution_mesh = True
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-solution_mesh')

    def test_order(self):
        a2d.view.mesh.order = True
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-oder')

    def test_order_palette(self):
        a2d.view.mesh.order = True
        a2d.view.mesh.order_view_parameters['palette'] = 'jet'
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-oder_palette')

    def test_order_color_bar(self):
        a2d.view.mesh.order = True
        a2d.view.mesh.order_view_parameters['color_bar'] = False
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-oder_color_bar')

    def test_order_label(self):
        a2d.view.mesh.order = True
        a2d.view.mesh.order_view_parameters['label'] = True
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-oder_label')

class TestMeshViewAdaptiveProblem(TestView):
    @classmethod
    def setUpClass(cls):
        adaptive_model()
        a2d.problem().solve()

    def setUp(self):
        a2d.view.mesh.activate()
        a2d.view.mesh.initial_mesh = True
        a2d.view.mesh.solution_mesh = True
        a2d.view.mesh.order = True
        a2d.view.zoom_best_fit()

    """
    def test_mesh_adaptive_step(self):
        steps = len(a2d.field('magnetic').adaptivity_info()['dofs'])
        for i in range(1, steps+1):
            a2d.view.mesh.adaptivity_step = i
            a2d.view.mesh.solution_type = 'normal'
            a2d.view.mesh.refresh()
            self.process('adaptive_problem-mesh-adaptive_step_{0}'.format(i))
    """

    def test_adaptive_type(self):
        a2d.view.mesh.adaptivity_step = len(a2d.field('magnetic').adaptivity_info()['dofs'])
        a2d.view.mesh.solution_type = 'reference'
        a2d.view.mesh.refresh()
        self.process('adaptive_problem-mesh_view-reference_solution')

    def test_component(self):
        for i in [1, 2]:
            a2d.view.mesh.adaptivity_step = len(a2d.field('magnetic').adaptivity_info()['dofs'])
            a2d.view.mesh.solution_type = 'normal'
            a2d.view.mesh.component = i
            a2d.view.mesh.refresh()
            self.process('adaptive_problem-mesh_view-component-{0}'.format(i))

if __name__ == '__main__':
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMeshViewSimpleProblem))
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMeshViewAdaptiveProblem))
    suite.run(result)