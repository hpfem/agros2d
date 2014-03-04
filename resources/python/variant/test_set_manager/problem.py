from variant import model

class Model(model.ModelBase):
    def create(self):
        import agros2d as a2d

        # startup script
        R1 = self.parameters["R1"]
        R2 = self.parameters["R2"]
        R3 = self.parameters["R3"]
        R4 = self.parameters["R4"]
        L = self.parameters["L"]
        RB = 3.0*R4
        
        self.U = 10
        eps1 = 3
        eps2 = 4
        
        # problem
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"
        
        # electrostatic
        self.electrostatic = a2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.matrix_solver = "mumps"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 4
        self.electrostatic.adaptivity_type = "disabled"
        self.electrostatic.solver = "linear"        
        
        # boundaries
        self.electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : self.U})
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
        a2d.view.zoom_best_fit()

    def solve(self):
        try:
            self.problem.solve()
            self.solved = True
        except:
            self.solved = False         

    def process(self):
        volume_integrals = self.electrostatic.volume_integrals()        
        self.variables["C"] = 2.0 * volume_integrals["We"] / (self.U**2)


if __name__ == '__main__':
    model = Model()
    
    model.parameters["R1"] = 0.01
    model.parameters["R2"] = 0.03
    model.parameters["R3"] = 0.05
    model.parameters["R4"] = 0.06
    model.parameters["L"] = 0.04
    
    model.create()
    model.solve()
    model.process()
    model.save("tmp.var")
    
    model = Model()    
    model.load("tmp.var")
    print(model.variables)
    print(model.parameters)
    
    import os
    os.remove("tmp.var")    
    
    print("Test - capacitor: " + str(abs(model.variables['C'] - 4.972876655348628e-12) < 1e-20))