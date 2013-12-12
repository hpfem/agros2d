import agros2d as a2d
import pythonlab
import numpy as np
import os.path

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

from scipy.misc import imread, imsave, imresize

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
    
    def auto_crop(self, image):
      non_empty_columns = np.where(image.min(axis=0)<255)[0]
      non_empty_rows = np.where(image.min(axis=1)<255)[0]
      crop_box = (min(non_empty_rows), max(non_empty_rows), min(non_empty_columns), max(non_empty_columns))
      return image[crop_box[0]:crop_box[1]+1, crop_box[2]:crop_box[3]+1]

    def compare(self, image, reference_image):
        histograms = list()
        for img in [image, reference_image]:
            r, bins = np.histogram(img[:,:,0], bins=256, normed=True)
            g, bins = np.histogram(img[:,:,1], bins=256, normed=True)
            b, bins = np.histogram(img[:,:,2], bins=256, normed=True)
            histograms.append(np.array([r,g,b]).ravel())

        diff = histograms[0] - histograms[1]
        return np.sqrt(np.dot(diff, diff))
 
    def process(self, name):
        image_file = pythonlab.tempname('png')
        reference_image_file = pythonlab.datadir('resources/test/test_suite/script/images/{0}.png'.format(name))

        if os.path.exists(reference_image_file):
            a2d.view.save_image(image_file)
            image = self.auto_crop(imread(image_file))
            reference_image = imread(reference_image_file)

            difference = self.compare(image, reference_image)
            print(difference)
            self.assertLess(difference, 0.25)
        else:
            a2d.view.save_image(reference_image_file)
            imsave(reference_image_file, self.auto_crop(imread(reference_image_file)))

class TestMeshViewSimpleProblem(TestView):
    @classmethod
    def setUpClass(cls):
        simple_model()
        a2d.problem().solve()

    def setUp(self):
        a2d.view.config.workspace_parameters['rulers'] = False
        a2d.view.config.workspace_parameters['grid'] = False
        a2d.view.config.workspace_parameters['axes'] = False

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
        a2d.view.mesh.order_view_parameters['color_bar'] = False
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-oder')

    def test_order_palette(self):
        a2d.view.mesh.order = True
        a2d.view.mesh.order_view_parameters['palette'] = 'jet'
        a2d.view.mesh.refresh()
        self.process('simple_problem-mesh_view-oder_palette')

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
        a2d.view.config.workspace_parameters['rulers'] = False
        a2d.view.config.workspace_parameters['grid'] = False
        a2d.view.config.workspace_parameters['axes'] = False
        a2d.view.mesh.order_view_parameters['color_bar'] = False

        a2d.view.mesh.initial_mesh = True
        a2d.view.mesh.solution_mesh = True
        a2d.view.mesh.order = True

        a2d.view.mesh.activate()
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

    def test_solution_type(self):
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