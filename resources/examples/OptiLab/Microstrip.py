import agros2d as a2d

from variant import optimization
from variant.optimization import genetic
from variant.optimization.genetic.method import ModelGenetic

from math import sqrt, log, pi

class Microstrip(ModelGenetic):       
    def declare(self):
        self.model_info.add_parameter('W', float)
        self.model_info.add_parameter('d', float)
        self.model_info.add_variable('Z0', float)
        self.model_info.add_variable('F', float)
        
    def create(self):
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "planar"
        self.problem.mesh_type = "triangle"

        # disable view
        a2d.view.mesh.disable()
        a2d.view.post2d.disable()
        
        # electrostic field        
        self.electrostatic = a2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.matrix_solver = "mumps"
        self.electrostatic.number_of_refinements = 0
        self.electrostatic.polynomial_order = 2
        self.electrostatic.adaptivity_type = "disabled"
        self.electrostatic.solver = "linear"

        self.electrostatic.add_boundary("Source electrode", "electrostatic_potential", {"electrostatic_potential" : 1})
        self.electrostatic.add_boundary("Ground electrode", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Zero charge", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        self.electrostatic.add_material("Dielectric substrate", {"electrostatic_permittivity" : 2.6285})
        self.electrostatic.add_material("Air", {"electrostatic_permittivity" : 1})

        # magnetic field
        self.magnetic = a2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 2
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        self.magnetic.add_material("Dielectric substrate", {"magnetic_permeability" : 1})
        self.magnetic.add_material("Conductor (source)", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0,
                                                          "magnetic_total_current_prescribed" : True, "magnetic_total_current_real" : 1})
        self.magnetic.add_material("Conductor (ground)", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0,
                                                          "magnetic_total_current_prescribed" : True, "magnetic_total_current_real" : -1})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})

        geometry = a2d.geometry
        W = self.parameters['W']
        d = self.parameters['d']
        t = 0.5e-4

        # dielectric substrate
        N = 15
        geometry.add_edge(-N*W/2.0, -d/2.0, +N*W/2.0, -d/2.0, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_edge(N*W/2.0, -d/2.0, N*W/2.0, d/2.0)
        geometry.add_edge(N*W/2.0, d/2.0, W/2.0, d/2.0)
        geometry.add_edge(W/2.0, d/2.0, -W/2.0, d/2.0, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_edge(-W/2.0, d/2.0, -N*W/2.0, d/2.0)
        geometry.add_edge(-N*W/2.0, d/2.0, -N*W/2.0, -d/2.0)
        geometry.add_label(0, 0, materials={"electrostatic" : "Dielectric substrate", "magnetic" : "Dielectric substrate"})

        # source electrode
        geometry.add_edge(W/2.0, d/2.0, W/2.0, d/2.0+t, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_edge(W/2.0, d/2.0+t, -W/2.0, d/2.0+t, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_edge(-W/2.0, d/2.0+t, -W/2.0, d/2.0, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_label(0, d/2.0+t/2.0, materials={"magnetic" : "Conductor (source)"})

        # source electrode
        geometry.add_edge(-N*W/2.0, -d/2.0, -N*W/2.0, -d/2.0-t, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_edge(-N*W/2.0, -d/2.0-t, N*W/2.0, -d/2.0-t, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_edge(N*W/2.0, -d/2.0-t, N*W/2.0, -d/2.0, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_label(0, -d/2.0-t/2.0, materials={"magnetic" : "Conductor (ground)"})

        # boundary
        geometry.add_circle(0, 0, 2*N*W, boundaries={"electrostatic" : "Zero charge", "magnetic" : "A = 0"})
        geometry.add_label(0, 2*N*W/2.0, materials={"electrostatic" : "Air", "magnetic" : "Air"})

    def solve(self):
        try:
            self.problem.solve()
            self.solved = True
        except:
            self.solved = False

    def process(self):
        # store geometry
        self.info['_geometry'] = a2d.geometry.export_svg_image()
        # variables            
        C = 2 * self.electrostatic.volume_integrals()['We'] / 1**2
        L = 2 * self.magnetic.volume_integrals()['Wm'] / 1**2
        self.variables['Z0'] = sqrt(L/C)
        self.variables['F'] = abs(75 - self.variables['Z0'])

if __name__ == '__main__':
    model = Microstrip()
    model.parameters['W'] = 5e-4
    model.parameters['d'] = 1e-3
    model.create()
    model.solve()
    model.process()
    print('Numerical solution: Z0 = {0}'.format(model.variables['Z0']))
    
    # analytical solution
    epsr = 2.6285
    W = model.parameters['W']
    d = model.parameters['d']
    epse = (epsr + 1)/2.0 + (epsr - 1)/2.0 * 1/(sqrt(1 + 12*d/W))
    if (W/d <= 1):
        Z0 = 60.0/sqrt(epse) * log(8*d/W + W/(4*d))
        print('Analytical solution: Z0 = {0}'.format(Z0))
    if (W/d > 1):
        Z0 = 120.0*pi/(sqrt(epse)*(W/d + 1.393 + 0.667*log(W/d + 1.444)))
        print('Analytical solution: Z0 = {0}'.format(Z0))
    
    # optimization
    parameters = optimization.Parameters([optimization.ContinuousParameter('W', 1e-4, 1e-3),
                                            optimization.ContinuousParameter('d', 1e-4, 1e-3)])
    
    functionals = optimization.Functionals([optimization.Functional("F", "min")])
    
    optimization = genetic.GeneticOptimization(parameters, functionals, Microstrip)
    
    #from variant import ModelDictExternal
    #optimization.model_dict = ModelDictExternal()
    #optimization.model_dict.solver = '{0}/agros2d_solver'.format(pythonlab.datadir())
    
    optimization.population_size = 20
    optimization.run(5, save = False)
    
    star = optimization.find_best(optimization.model_dict.models())
    print('Z0 = {0} Ohm (required 75 Ohm)'.format(star.variables['Z0']))
    print('Best variant parameters: W={0}, d={1}'.format(star.parameters['W'], star.parameters['d']))
    
    optimization.model_dict.save_to_zip(problem = 'Microstrip.py', filename = 'Microstrip.opt')