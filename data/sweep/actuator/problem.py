import agros2d as a2d
from variant import model
import math

class Model(model.ModelBase):
    def __create_model__(self, dz):              
        # startup script
        
        dz = dz + 0.01
        
        AH = 0.12
        AR = 0.01
        AR1 = self.parameters["AR1"]
        AH1 = -0*AH/5
        AR2 = self.parameters["AR2"]
        AH2 = -1*AH/5
        AR3 = self.parameters["AR3"]
        AH3 = -2*AH/5
        AR4 = self.parameters["AR4"]
        AH4 = -3*AH/5
        AR5 = self.parameters["AR5"]
        AH5 = -4*AH/5
        
        SH = 0.12
        SR = 0.05
        M = 0.01
        
        SPACE = 0.002
        
        Itotal = 5000
        aw = (SR-M-SPACE)-(AR+2*SPACE)
        ah = (SH/2-M/2-SPACE)-(M+SPACE)
        Jext = Itotal / (aw * ah) / 2
                
        # problem
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"
        
        # magnetic
        self.magnetic = a2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 2
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"
        
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        # materials
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 300})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : Jext})
        
        # geometry
        geometry = a2d.geometry
        geometry.add_edge(AR+SPACE, M, AR+SPACE, 0)
        geometry.add_edge(AR+SPACE, 0, SR, 0)
        geometry.add_edge(SR, 0, SR, SH)
        geometry.add_edge(SR, SH, 0, SH)
        geometry.add_edge(0, SH, 0, SH-2*M, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, SH-2*M, AR+SPACE, SH-2*M)
        geometry.add_edge(AR+SPACE, SH-M, SR-M, SH-M)
        geometry.add_edge(AR+SPACE, SH-M, AR+SPACE, SH-2*M)
        geometry.add_edge(AR+SPACE, M, SR-M, M)
        geometry.add_edge(AR+2*SPACE, SH/2-M/2-SPACE, AR+2*SPACE, M+SPACE)
        geometry.add_edge(AR+2*SPACE, M+SPACE, SR-M-SPACE, M+SPACE)
        geometry.add_edge(SR-M-SPACE, M+SPACE, SR-M-SPACE, SH/2-M/2-SPACE)
        geometry.add_edge(SR-M-SPACE, SH/2-M/2-SPACE, AR+2*SPACE, SH/2-M/2-SPACE)
        geometry.add_edge(0, AH5+dz, AR5, AH5+dz)
        geometry.add_edge(0, AH5+dz, 0, AH1+dz, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, AH1+dz, AR1, AH1+dz)
        geometry.add_edge(0, 2*SH, 0, SH, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -2*SH, 2*SH, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(2*SH, 0, 0, 2*SH, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, SH-2*M, 0, AH1+dz, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, AH5+dz, 0, -2*SH, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(AR1, AH1+dz, AR2, AH2+dz)
        geometry.add_edge(AR2, AH2+dz, AR3, AH3+dz)
        geometry.add_edge(AR3, AH3+dz, AR4, AH4+dz)
        geometry.add_edge(AR5, AH5+dz, AR4, AH4+dz)
        geometry.add_edge(AR+2*SPACE, SH-M-SPACE, AR+2*SPACE, SH/2+M/2+SPACE)
        geometry.add_edge(SR-M-SPACE, SH/2+M/2+SPACE, AR+2*SPACE, SH/2+M/2+SPACE)
        geometry.add_edge(SR-M-SPACE, SH-M-SPACE, SR-M-SPACE, SH/2+M/2+SPACE)
        geometry.add_edge(SR-M-SPACE, SH-M-SPACE, AR+2*SPACE, SH-M-SPACE)
        geometry.add_edge(SR-M, SH-M, SR-M, SH/2+M/2)
        geometry.add_edge(AR+SPACE, SH/2+M/2, SR-M, SH/2+M/2)
        geometry.add_edge(AR+SPACE, SH/2-M/2, AR+SPACE, SH/2+M/2)
        geometry.add_edge(SR-M, SH/2-M/2, AR+SPACE, SH/2-M/2)
        geometry.add_edge(SR-M, SH/2-M/2, SR-M, M)
        
        geometry.add_label(((AR+2*SPACE)+(SR-M-SPACE))/2, (SH/2-M/2-SPACE+M+SPACE)/2, materials = {"magnetic" : "Copper"})
        geometry.add_label(((AR+2*SPACE)+(SR-M-SPACE))/2, (SH-M-SPACE+SH/2+M/2+SPACE)/2, materials = {"magnetic" : "Copper"})
        geometry.add_label(AR/2, SH-M, materials = {"magnetic" : "Iron"})
        geometry.add_label(AR3/5, AH3+dz, materials = {"magnetic" : "Iron"})
        geometry.add_label(SH, 0, materials = {"magnetic" : "Air"})
        a2d.view.zoom_best_fit()

    def create(self):
        self.__create_model__(0.0)
        
    def solve(self):
        try:
            N = 10
            h = 0.085
            step = h / (N-1)
            
            self._displacement = []
            self._force = []
            for i in range(N):
                z = i*step

                self.__create_model__(z)  
                if (i == 0):
                    # store geometry
                    self.geometry = a2d.geometry.export_svg_image()        
                    
                self.problem.solve()
                volume_integrals = self.magnetic.volume_integrals([3])
                
                self._displacement.append(z)
                self._force.append(volume_integrals["Fty"])
                
            self.solved = True
        except:
            self.solved = False         

    def process(self):
        # chart
        self.variables["Fty"] = [self._displacement, self._force] 
        # value
        self.variables["M"] = sum(self._force) / len(self._force)
        diffsqr = 0
        for i in range(len(self._force)):
            diffsqr = diffsqr + (self._force[i] - self.variables["M"])**2
        self.variables["R"] = math.sqrt(diffsqr)

if __name__ == '__main__':
    model = Model()
    
    model.parameters["AR1"] = 0.01
    model.parameters["AR2"] = 0.009
    model.parameters["AR3"] = 0.008
    model.parameters["AR4"] = 0.007
    model.parameters["AR5"] = 0.01
    
    model.create()
    model.solve()
    model.process()
    model.save("tmp.var")
    
    model = Model()    
    model.load("tmp.var")

    import os
    os.remove("tmp.var")  
    
    print("Test - actuator: " + str(abs(model.variables['R'] - 68.8905318562) < 1e-20))