import agros2d as a2d
import pythonlab

from variant import ModelBase, ModelDict, ModelGenerator

class Capacitor(ModelBase):
    def create(self):
        # defaults and parameters
        self.defaults['R1'] = 0.01
        self.defaults['w1'] = 0.02
        self.defaults['w2'] = 0.03
        self.defaults['l'] = 0.08                              
        self.defaults['RB'] = 0.2

        self.defaults['U'] = 10
        self.defaults['epsr1'] = 3
        self.defaults['epsr2'] = 4
        
        # variables
        R1 = self.parameters['R1']
        R2 = self.parameters['R1'] + self.parameters['w1']
        R3 = self.parameters['R1'] + self.parameters['w1'] + self.parameters['w2']
        R4 = self.parameters['R1'] + self.parameters['w1'] + self.parameters['w2'] + 0.01
        L = self.parameters['l']
        RB = self.defaults['RB']
        
        U = self.defaults['U']
        eps1 = self.defaults['epsr1']
        eps2 = self.defaults['epsr2']
                
        # problem
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"

        # disable view
        a2d.view.mesh.disable()
        a2d.view.post2d.disable()
        
        # electrostatic
        self.electrostatic = a2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.matrix_solver = "mumps"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 3
        self.electrostatic.adaptivity_type = "disabled"
        self.electrostatic.solver = "linear"
                
        # boundaries
        self.electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : U})
        self.electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
                
        # materials
        self.electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
        self.electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : eps1, "electrostatic_charge_density" : 0})
        self.electrostatic.add_material("Dielectric 2", {"electrostatic_permittivity" : eps2, "electrostatic_charge_density" : 0})
        
        # geometry
        geometry = a2d.geometry
        geometry.add_edge(0, RB, 0, L, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(R1, L, R1, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(R1, 0, R2, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(R2, 0, R2, L)
        geometry.add_edge(R2, L, R3, L)
        geometry.add_edge(R3, 0, R2, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(R3, L, R3, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(R4, 0, R4, L, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(R3, L, R4, L, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(R4, 0, RB, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(RB, 0, 0, RB, angle = 90, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(R1, L, R2, L)
        geometry.add_edge(R1, L, 0, L, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
        
        geometry.add_label((R1+R2)/2.0, L/2.0, materials = {"electrostatic" : "Dielectric 1"})
        geometry.add_label((R2+R3)/2.0, L/2.0, materials = {"electrostatic" : "Dielectric 2"})
        geometry.add_label(R1, RB-R1, materials = {"electrostatic" : "Air"})
        
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
        self.variables['C'] = 2 * self.electrostatic.volume_integrals()['We'] / self.defaults['U']**2

if __name__ == '__main__':
    tempdir = pythonlab.tempname()
    
    # create generator
    mg = ModelGenerator(Capacitor)
    # add parameters
    mg.add_parameter_by_interval('w1', 0.02, 0.04, 0.01)
    mg.add_parameter_by_interval('w2', 0.02, 0.04, 0.01)
    mg.add_parameter_by_interval('l', 0.03, 0.07, 0.01)
    # make combinations and save to the directory
    mg.combination()
    # mg.save(tempdir)
    
    # create model directory
    md = ModelDict(models = mg.dict.models())
    
    # solve and save problems
    md.solve(save = False)
    md.save_to_zip(problem = 'Capacitor.py', filename = 'Capacitor.opt')