import agros2d as a2d

class MagneticModel:
  def __init__(self, Rarm, Harm, Rcir, Hcir, delta, mur_iron, Iext, Next):
    Scoil = ((Hcir - Rarm - delta) - (Rarm + 2*delta)) * ((Rcir - Rarm - delta) - (Rarm + 2*delta))
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

    self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : Jext})
    self.magnetic.add_material("Iron", {"magnetic_permeability" : mur_iron})
    self.magnetic.add_material("Air", {"magnetic_permeability" : 1})

    self.geometry = a2d.geometry
    self.geometry.add_edge(Rarm + delta, Rarm + delta, Rarm + delta, 0)
    self.geometry.add_edge(Rarm + delta, 0, Rcir, 0)
    self.geometry.add_edge(Rcir, 0, Rcir, Hcir)
    self.geometry.add_edge(Rcir, Hcir, 0, Hcir)
    self.geometry.add_edge(0, Hcir, 0, Hcir - 2*Rarm, boundaries = {"magnetic" : "A = 0"})
    self.geometry.add_edge(0, Hcir - 2*Rarm, Rarm + delta, Hcir - 2*Rarm)
    self.geometry.add_edge(Rarm + delta, Hcir - Rarm, Rcir - Rarm, Hcir - Rarm)
    self.geometry.add_edge(Rarm + delta, Hcir - Rarm, Rarm + delta, Hcir - 2*Rarm)
    self.geometry.add_edge(Rcir - Rarm, Hcir - Rarm, Rcir - Rarm, Rarm + delta)
    self.geometry.add_edge(Rarm + delta, Rarm + delta, Rcir - Rarm, Rarm + delta)
    self.geometry.add_edge(Rarm + 2*delta, Hcir - Rarm - delta, Rarm + 2*delta, Rarm + 2*delta)
    self.geometry.add_edge(Rarm + 2*delta, Rarm + 2*delta, Rcir - Rarm - delta, Rarm + 2*delta)
    self.geometry.add_edge(Rcir - Rarm - delta, Rarm + 2*delta, Rcir - Rarm - delta, Hcir - Rarm - delta)
    self.geometry.add_edge(Rcir - Rarm - delta, Hcir - Rarm - delta, Rarm + 2*delta, Hcir - Rarm - delta)
    self.geometry.add_edge(0, Harm, Rarm, Harm)
    self.geometry.add_edge(Rarm, Harm, Rarm, 0)
    self.geometry.add_edge(0, 0, Rarm, 0)
    self.geometry.add_edge(0, Harm, 0, 0, boundaries = {"magnetic" : "A = 0"})
    self.geometry.add_edge(0, 2*Hcir, 0, Hcir, boundaries = {"magnetic" : "A = 0"})
    self.geometry.add_edge(0, Hcir - 2*Rarm, 0, Harm, boundaries = {"magnetic" : "A = 0"})
    self.geometry.add_edge(0, 0, 0, -Hcir, boundaries = {"magnetic" : "A = 0"})
    self.geometry.add_edge(0, -Hcir, 1.5*Hcir, 0.5*Hcir, angle = 90, boundaries = {"magnetic" : "A = 0"})
    self.geometry.add_edge(1.5*Hcir, 0.5*Hcir, 0, 2*Hcir, angle = 90, boundaries = {"magnetic" : "A = 0"})

    self.geometry.add_label(Rcir - Rarm / 2.0, Hcir / 2.0, materials = {"magnetic" : "Iron"})
    self.geometry.add_label(Rcir - Rarm - delta - Rarm, Harm / 2.0, materials = {"magnetic" : "Copper"})
    self.geometry.add_label(2*Rcir, 0.5 * Hcir, materials = {"magnetic" : "Air"})
    self.geometry.add_label(Rarm / 2.0, Harm / 2.0, materials = {"magnetic" : "Iron"})

    a2d.view.zoom_best_fit()

Rarm = {{txtArmatureRadius_text}}
Harm = {{txtArmatureHeight_text}}
Rcir = {{txtCircuitRadius_text}}
Hcir = {{txtCircuitHeight_text}}
delta = {{txtArmatureRadius_text}} / 10.0

mur_iron = {{txtIronPermeability_text}}
Iext = {{txtCoilCurrent_text}}
Next = {{txtCoilNumberOfTurns_text}}

"""
Rarm = 0.009
Harm = 0.075
Rcir = 0.04
Hcir = 0.1
delta = 0.001

mur_iron = 9300
Iext = 1
Next = 1500
"""

magnetic_model = MagneticModel(Rarm, Harm, Rcir, Hcir, delta, mur_iron, Iext, Next)
