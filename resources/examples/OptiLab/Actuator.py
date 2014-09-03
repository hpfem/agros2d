import agros2d as a2d

from variant import optimization
from variant.optimization import genetic
from variant.optimization.genetic.method import ModelGenetic

from math import sqrt

class Actuator(ModelGenetic):
    def declare(self):
        self.declare_parameter('AR1', float)
        self.declare_parameter('AR2', float)
        self.declare_parameter('AR3', float)
        self.declare_parameter('AR4', float)
        self.declare_parameter('AR5', float)
        self.declare_variable('F', list)
        self.declare_variable('M', float)
        self.declare_variable('R', float)
        self.declare_variable('xMR', float)

    def create(self):
        # defaults and parameters
        dz = 0.05
        AH = 0.09
        AR = 0.01        
        SH = 0.1
        SR = 0.04
        M = 0.01        
        SPACE = 0.001

        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"

        # magneticfield        
        self.magnetic = a2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 2
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
            
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 300})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1,
                                              "magnetic_total_current_prescribed" : 1, "magnetic_total_current_real" : 1000*2.5})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})

        self.geometry = a2d.geometry
        AR1 = self.parameters['AR1']
        AR2 = self.parameters['AR2']
        AR3 = self.parameters['AR3']
        AR4 = self.parameters['AR4']
        AR5 = self.parameters['AR5']

        # geometry
        self.geometry = a2d.geometry
        self.geometry.add_edge(AR+SPACE, M, AR+SPACE, 0)
        self.geometry.add_edge(AR+SPACE, 0, SR, 0)
        self.geometry.add_edge(SR, 0, SR, SH)
        self.geometry.add_edge(SR, SH, 0, SH)
        self.geometry.add_edge(0, SH, 0, SH-2*M, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, SH-2*M, AR+SPACE, SH-2*M)
        self.geometry.add_edge(AR+SPACE, SH-M, SR-M, SH-M)
        self.geometry.add_edge(AR+SPACE, SH-M, AR+SPACE, SH-2*M)
        self.geometry.add_edge(SR-M, SH-M, SR-M, M)
        self.geometry.add_edge(AR+SPACE, M, SR-M, M)
        self.geometry.add_edge(AR+2*SPACE, SH-M-SPACE, AR+2*SPACE, M+SPACE)
        self.geometry.add_edge(AR+2*SPACE, M+SPACE, SR-M-SPACE, M+SPACE)
        self.geometry.add_edge(SR-M-SPACE, M+SPACE, SR-M-SPACE, SH-M-SPACE)
        self.geometry.add_edge(SR-M-SPACE, SH-M-SPACE, AR+2*SPACE, SH-M-SPACE)
        self.geometry.add_edge(0, -AH+dz, AR, -AH+dz)
        self.geometry.add_edge(0, dz, AR, dz)
        self.geometry.add_edge(0, 2*SH, 0, SH, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, -2*SH, 2*SH, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(2*SH, 0, 0, 2*SH, angle = 90, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, SH-2*M, 0, dz, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, -AH+dz, 0, -2*SH, boundaries = {"magnetic" : "A = 0"})

        self.geometry.add_edge(0, -AH+dz, 0, dz, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(AR , -AH*0/6+dz, AR1, -AH*1/6+dz)
        self.geometry.add_edge(AR1, -AH*1/6+dz, AR2, -AH*2/6+dz)
        self.geometry.add_edge(AR2, -AH*2/6+dz, AR3, -AH*3/6+dz)
        self.geometry.add_edge(AR3, -AH*3/6+dz, AR4, -AH*4/6+dz)
        self.geometry.add_edge(AR4, -AH*4/6+dz, AR5, -AH*5/6+dz)
        self.geometry.add_edge(AR5, -AH*5/6+dz, AR , -AH*6/6+dz)
        
        self.geometry.add_label(AR/2, SH-M, materials = {"magnetic" : "Iron"})
        self.geometry.add_label(((AR+2*SPACE)+(SR-M-SPACE))/2, ((M+SPACE)+(SH-M-SPACE))/2, materials = {"magnetic" : "Copper"})
        self.geometry.add_label(SH, 0, materials = {"magnetic" : "Air"})
        self.geometry.add_label(AR/10, -AH/2+dz, materials = {"magnetic" : "Iron"})
        
        # disable view
        a2d.view.mesh.disable()
        a2d.view.post2d.disable()

    def solve(self):
        try:
            dz = 0.003
            self.N = 9
            self.F = []
            for i in range(self.N):
                if (i > 0):
                    self.geometry.select_edges(range(21, 28))
                    self.geometry.move_selection(0, dz)
                    self.geometry.select_labels([3])
                    self.geometry.move_selection(0, dz)
                    
                self.problem.solve()
                self.F.append(self.magnetic.volume_integrals([3])['Fty'])

            self.solved = True
        except:
            self.solved = False

    def process(self):
        # store geometry
        self.info['_geometry'] = a2d.geometry.export_svg_image()

        # static characteristic
        self.variables['F'] = self.F

        # average force
        Favg = sum(self.F)/self.N
        self.variables['M'] = Favg

        # ripple
        R = 0
        for i in range(self.N):
            R += (self.F[i] - Favg)**2
        self.variables['R'] = sqrt(R)

        # TODO: multicriteria
        self.variables['xMR'] = Favg + (12 - sqrt(R))

if __name__ == '__main__':
    # optimization
    parameters = optimization.Parameters([optimization.ContinuousParameter('AR1', 0.001, 0.01),
                                          optimization.ContinuousParameter('AR2', 0.001, 0.01),
                                          optimization.ContinuousParameter('AR3', 0.001, 0.01),
                                          optimization.ContinuousParameter('AR4', 0.001, 0.01),
                                          optimization.ContinuousParameter('AR5', 0.001, 0.01)])
    
    # functionals = optimization.Functionals([optimization.Functional("M", "max"),
    #                                         optimization.Functional("R", "min")])
    functionals = optimization.Functionals([optimization.Functional("xMR", "max")])

    optimization = genetic.GeneticOptimization(parameters, functionals, Actuator)

    optimization.population_size = 20
    optimization.run(5, save = False)

    star = optimization.find_best(optimization.model_dict.models())
    print('Force = {0} N, ripple = {1} N'.format(star.variables['M'], star.variables['R']))
    optimization.model_dict.save('Actuator.opt', 'Actuator.py')