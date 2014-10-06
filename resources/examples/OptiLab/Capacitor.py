from variant import ModelBase, ModelGenerator

import agros2d as a2d

class Capacitor(ModelBase):
    def declare(self):
        self.parameters.declare('R1', float, 0.01)
        self.parameters.declare('w1', float, 0.02)
        self.parameters.declare('w2', float, 0.03)
        self.parameters.declare('l', float, 0.08)
        self.parameters.declare('RB', float, 0.2)

        self.parameters.declare('U', float, 10)
        self.parameters.declare('epsr1', float, 3)
        self.parameters.declare('epsr2', float, 4)

        self.variables.declare('C', float)

    def create(self):
        R1 = self.parameters['R1']
        R2 = self.parameters['R1'] + self.parameters['w1']
        R3 = self.parameters['R1'] + self.parameters['w1'] + self.parameters['w2']
        R4 = self.parameters['R1'] + self.parameters['w1'] + self.parameters['w2'] + self.parameters['R1']
        L = self.parameters['l']
        RB = self.parameters['RB']

        U = self.parameters['U']
        eps1 = self.parameters['epsr1']
        eps2 = self.parameters['epsr2']

        # problem
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"

        # electrostatic
        self.electrostatic = a2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 3
        self.electrostatic.adaptivity_type = "disabled"

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

        # disable view
        a2d.view.mesh.disable()
        a2d.view.post2d.disable()

    def solve(self):
        self.problem.solve()
        self.info['_geometry'] = a2d.geometry.export_svg_image()
        self.variables['C'] = 2 * self.electrostatic.volume_integrals()['We'] / self.parameters['U']**2

if __name__ == '__main__':
    mg = ModelGenerator(Capacitor)
    mg.add_parameter_by_interval('w1', 0.02, 0.04, 0.01)
    mg.add_parameter_by_interval('w2', 0.02, 0.04, 0.01)
    mg.add_parameter_by_interval('l', 0.03, 0.07, 0.01)
    mg.combination()

    mg.dictionary.solve(save = False)
    mg.dictionary.save(file_name='Capacitor.opt', problem='Capacitor.py')