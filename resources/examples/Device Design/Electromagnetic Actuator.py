import agros2d as a2d

class MagneticModel:
    def __init__(self, Rarm, Harm, S, Rcir, Hcir, Tcir, Hsp, Tsc,
                       G, Iext, Next, mur_arm, mur_cir):
        Scoil = (Hcir-2*Tcir-2*G) * ((Rcir-Tcir-G)-(Rarm+G))
        Jext = Next * Iext / Scoil

        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"
    
        self.magnetic = a2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 2
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"
    
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})

        self.magnetic.add_material("Armature", {"magnetic_permeability" : mur_arm})
        self.magnetic.add_material("Magnetic circuit", {"magnetic_permeability" : mur_cir})
        self.magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : Jext})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})
    
        self.geometry = a2d.geometry
    
        # magnetic circuit
        self.geometry.add_edge(Rarm + G + Tsc, 0, Rcir, 0)
        self.geometry.add_edge(Rcir, 0, Rcir, Hcir)
        self.geometry.add_edge(Rcir, Hcir, 0, Hcir)
        self.geometry.add_edge(0, Hcir, 0, Hcir - Hsp, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, Hcir - Hsp, Rarm, Hcir - Hsp)
        self.geometry.add_edge(Rarm, Hcir - Hsp, Rarm, Hcir - Tcir)
        self.geometry.add_edge(Rarm, Hcir - Tcir, Rcir - Tcir, Hcir - Tcir)
        self.geometry.add_edge(Rcir - Tcir, Hcir - Tcir, Rcir - Tcir, Tcir)
        self.geometry.add_edge(Rcir-Tcir, Tcir, Rarm + G + Tsc, Tcir)
        self.geometry.add_edge(Rarm + G + Tsc, Tcir, Rarm + G + Tsc, 0)
        self.geometry.add_label(Rcir - Tcir/2.0, Hcir/2.0, area = 5e-06, materials = {"magnetic" : "Magnetic circuit"})
    
        # coil
        self.geometry.add_edge(Rarm + G, Tcir + G, Rcir - Tcir - G, Tcir + G)
        self.geometry.add_edge(Rcir - Tcir - G, Tcir + G, Rcir - Tcir - G, Hcir - Tcir - G)
        self.geometry.add_edge(Rcir - Tcir - G, Hcir - Tcir - G, Rarm + G, Hcir - Tcir - G)
        self.geometry.add_edge(Rarm + G, Hcir - Tcir - G, Rarm + G, Tcir + G)
        self.geometry.add_label(Rarm + G + ((Rcir - Tcir - G) - (Rarm + G))/2.0, Hcir/2.0, materials = {"magnetic" : "Coil"})
    
        # armature
        Darm = Hcir - Hsp - S
        last_edges_count = self.geometry.edges_count()
        self.geometry.add_edge(0, Darm - Harm, Rarm, Darm - Harm)
        self.geometry.add_edge(Rarm, Darm - Harm, Rarm, Darm)
        self.geometry.add_edge(Rarm, Darm, 0, Darm)
        self.geometry.add_edge(0, Darm, 0, Darm - Harm, boundaries = {"magnetic" : "A = 0"})
        self.moved_label = self.geometry.add_label(Rarm/2.0, Darm - Harm/2.0, area = 5e-06, materials = {"magnetic" : "Armature"})
        self.moved_edges = range(last_edges_count, self.geometry.edges_count())

        # sliding contact
        #self.geometry.add_edge(Rarm + G, 0, Rarm + G + Tsc, 0)
        #self.geometry.add_edge(Rarm + G + Tsc, Tcir, Rarm + G, Tcir)
        #self.geometry.add_edge(Rarm + G, Tcir, Rarm + G, 0)
    
        # fictinious boundary
        Dfic = 0.15
        self.geometry.add_edge(0, Darm - Harm, 0, Darm - Harm - Dfic, boundaries = {"magnetic" : "A = 0"})
        self.moved_edges.append(self.geometry.add_edge(0, Darm - Harm - Dfic, Dfic, Darm - Harm - Dfic, boundaries = {"magnetic" : "A = 0"}))
        self.geometry.add_edge(Dfic, Darm - Harm - Dfic, Dfic, Hcir + Dfic, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(Dfic, Hcir + Dfic, 0, Hcir + Dfic, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, Hcir, 0, Hcir + Dfic, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0, Hcir - Hsp, 0, Darm, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_label(Dfic/2.0, Hcir/2.0, materials = {"magnetic" : "Air"})
    
        a2d.view.zoom_best_fit()

Rarm = {{txtArmatureRadius_text}}
Harm = {{txtArmatureHeight_text}}
S = {{txtArmatureShift_text}}
mur_arm = {{txtArmaturePermeability_text}}
Rcir = {{txtCircuitRadius_text}}
Hcir = {{txtCircuitHeight_text}}
Tcir = {{txtCircuitThickness_text}}
Hsp = {{txtCircuitSpineHeight_text}}
G = {{txtGap_text_text}}
mur_cir = {{txtCircuitPermeability_text}}
Iext = {{txtCoilCurrent_text}}
Next = {{txtCoilNumberOfTurns_text}}

"""
Rarm = 0.008
Harm = 0.13
S  = 0.01
mur_arm = 1000

Rcir = 0.035
Hcir = 0.15
Tcir = 0.008
Hsp = 0.02
G = 0.002
mur_cir = 1000

Iext = 1
Next = 1500
"""

magnetic_model = MagneticModel(Rarm, Harm, S, Rcir, Hcir, Tcir, Hsp, 0, G,
                               Iext, Next, mur_arm, mur_cir)