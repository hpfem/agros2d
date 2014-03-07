from variant import model

class Model(model.ModelBase):
    def create(self):
        print "create"
        import agros2d as a2d

        # startup script
        left = self.parameters["left"]
        right = self.parameters["right"]
        bottom = self.parameters["bottom"]
        top = self.parameters["top"]
        
        U = 10
        eps = self.parameters["eps"]
        
        
        # problem
        problem = a2d.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # fields
        # electrostatic
        self.electrostatic = a2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.matrix_solver = "mumps"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 4
        self.electrostatic.adaptivity_type = "disabled"
        self.electrostatic.solver = "linear"
        
        
        # boundaries
        self.electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : U})
        self.electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        
        
        # materials
        self.electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : eps, "electrostatic_charge_density" : 0})
        
        # geometry
        geometry = a2d.geometry
        geometry.add_edge(left, top, left, bottom, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(left, bottom, right, bottom, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(right, bottom, right, top, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(right, top, left, top, boundaries = {"electrostatic" : "Neumann"})
        
        geometry.add_label(0.0237419, 0.0239324, materials = {"electrostatic" : "Dielectric 1"})
        a2d.view.zoom_best_fit()


    def solve(self):
#        import agros2d as a2d

        print "solve"
        try:
            # store geometry
 #           self.geometry = a2d.geometry.export_svg_image()
        
            self.problem.solve()
            self.solved = True
        except:
            self.solved = False         

    def process(self):
        print "process, solved: ", self.solved 
        volume_integrals = self.electrostatic.volume_integrals()        
        self.variables["C"] = 2.0 * volume_integrals["We"] / (self.U**2)


