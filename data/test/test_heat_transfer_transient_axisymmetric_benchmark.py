import agros2d
from time import time
start_time = time()

# benchmark 
#
# A.D. Cameron, J. A. Casey, and G.B. Simpson: 
# NAFEMS Benchmark Tests for Thermal Analysis (Summary), 
# NAFEMS Ltd., Glasgow, 1986

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.time_step_method = "adaptive"
problem.time_method_order = 3
problem.time_method_tolerance = 1.0
problem.time_steps = 20
problem.time_total = 190

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
# heat
heat = agros2d.field("heat")
heat.analysis_type = "transient"
heat.initial_condition = 0
heat.number_of_refinements = 2
heat.polynomial_order = 3
heat.linearity_type = "linear"

heat.add_boundary("Symmetry", "heat_heat_flux", {"heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0, "heat_radiation_emissivity" : 0, "heat_heat_flux" : 0, "heat_radiation_ambient_temperature" : 0})
heat.add_boundary("Temperature", "heat_temperature", {"heat_temperature" : 1000})

heat.add_material("Material", {"heat_volume_heat" : 0, "heat_conductivity" : 52, "heat_velocity_x" : 0, "heat_density" : 7850, "heat_velocity_y" : 0, "heat_specific_heat" : 460, "heat_velocity_angular" : 0})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, 0.4, 0, 0, boundaries = {"heat" : "Symmetry"})
geometry.add_edge(0.3, 0.4, 0.3, 0, boundaries = {"heat" : "Temperature"})
geometry.add_edge(0.3, 0, 0, 0, boundaries = {"heat" : "Temperature"})
geometry.add_edge(0, 0.4, 0.3, 0.4, boundaries = {"heat" : "Temperature"})

geometry.add_label(0.151637, 0.112281, materials = {"heat" : "Material"}, area = 0.01)

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = heat.local_values(0.1, 0.3)
testT = agros2d.test("Temperature", point["T"], 186.5, 0.0004) # permissible error 0.02 %

elapsed_time = time() - start_time
print("Test: Heat transfer transient - axisymmetric benchmark ({0}): ".format(round(elapsed_time, 3)) + str(testT))
