import agros2d

import user_module # don't use from ... import ... - uset module deleter don't work
user_module.function(2)

problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.name = "Induction heating"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 1e3

magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "harmonic"
# FIXME! harmonicmagnetic.analysis_type = "harmonic"
magnetic.number_of_refinements = 2
magnetic.polynomial_order = 3
magnetic.linearity_type = "linear"
magnetic.nonlinear_tolerance = 0.001
magnetic.nonlinear_steps = 10

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
#magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
#magnetic.add_boundary("A = 0", "potential", {"magnetic_potential_real" : 0})
#magnetic.set_boundary("Dirichlet", "magnetic_potential", {"magnetic_potential_real" : 0})
#magnetic.set_boundary("A = 0", "potential", {"magnetic_potential_real" : 0})
#magnetic.set_boundary("A = 0", "magnetic_potential", {"potential_real" : 0})
#agnetic.add_boundary("Test", "magnetic_potential", {"magnetic_potential_real" : 0})
#agnetic.remove_boundary("Test")

magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e7})
#magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6})
#magnetic.add_material("Cu", {"permeability" : 1, "magnetic_current_density_external_real" : 1e6})
#magnetic.set_material("Copper", {"permeability" : 1.1})
#magnetic.set_material("Cu", {"permeability" : 1.1})
#magnetic.add_material("Test", {"magnetic_permeability" : 1})
#magnetic.remove_material("Test")
magnetic.add_material("Fe", {"magnetic_permeability" : 500, "magnetic_conductivity" : 1e5})
magnetic.add_material("Air", {"magnetic_permeability" : 1})

heat = agros2d.field("heat")
heat.analysis_type = "steadystate"
heat.linearity_type = "newton"
heat.initial_condition = 20

heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_heat_transfer_coefficient" : 10, "heat_convection_external_temperature" : 20})
heat.add_boundary("Flux", "heat_heat_flux", {"heat_heat_flux" : 0})

heat.add_material("Fe", {"heat_conductivity" : 60, "heat_volume_heat" : 0})

#heat.add_material("Al", {"heat_conductivity" : { "value" : 385, "x" : [0,100,400,900,1200,1800], "y" : [300,350,480,300,280,320] }, "heat_volume_heat" : 0})
#heat.set_material("Al", {"heat_conductivity" : { "value" : 500, "x" : [0,100,400,900,1200,1800], "y" : [320,280,300,480,350,300] }, "heat_volume_heat" : 0})
#heat.remove_material("Al")

geometry = agros2d.geometry

geometry.add_node(0, 0)
geometry.add_node(1, 0)
geometry.add_edge_by_nodes(0, 1, 90, refinements = {"magnetic" : 5, "heat" : 1})
geometry.remove_node(0)

geometry.add_edge(0, -0.1, 0.05, -0.1, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.05, -0.1, 0.05, 0.1, refinements = {"magnetic" : 3, "heat" : 2}, boundaries = {"heat" : "Convection"})
#geometry.add_edge(0.05, -0.1, 0.05, 0.1, refinements = {"agnetic" : 3, "heat" : 2}, boundaries = {"heat" : "Convection"})
#geometry.add_edge(0.05, -0.1, 0.05, 0.1, refinements = {"magnetic" : -1, "heat" : 2}, boundaries = {"heat" : "Convection"})
#geometry.add_edge(0.05, -0.1, 0.05, 0.1, refinements = {"magnetic" : 3, "heat" : 11}, boundaries = {"heat" : "Convection"})
geometry.add_edge(0.05, 0.1, 0, 0.1, boundaries = {"heat" : "Convection"})
geometry.add_edge(0, 0.1, 0, -0.1, boundaries = {"magnetic" : "A = 0", "heat" : "Flux"})

geometry.add_edge(0.06, -0.1, 0.08, -0.1)
geometry.add_edge(0.08, -0.1, 0.08, 0.1)
geometry.add_edge(0.08, 0.1, 0.06, 0.1)
geometry.add_edge(0.06, 0.1, 0.06, -0.1)

geometry.add_edge(0, -0.5, 0.5, -0.5, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, -0.5, 0.5, 0.5, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.5, 0.5, 0, 0.5, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.5, 0, 0.1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, -0.1, 0, -0.5, boundaries = {"magnetic" : "A = 0"})

#geometry.add_label(0.025, 0, 0, materials = {"magnetic" : "Fe", "heat" : "Fe"})
#geometry.add_label(0.025, 0, 0, refinements = {"magnetic" : 1}, materials = {"magnetic" : "Fe", "heat" : "Fe"})
#geometry.add_label(0.025, 0, 0, refinements = {"magnetic" : 2, "heat" : 1}, materials = {"magnetic" : "Fe", "heat" : "Fe"})
geometry.add_label(0.025, 0, 0, refinements = {"magnetic" : 2, "heat" : 1}, orders = {"magnetic" : 2, "heat" : 3}, materials = {"magnetic" : "Fe", "heat" : "Fe"})
#geometry.add_label(0.025, 0, -3, materials = {"magnetic" : "Fe", "heat" : "Fe"})
geometry.add_label(0.07, 0, materials = {"magnetic" : "Cu"})
geometry.add_label(0.25, 0, materials = {"magnetic" : "Air"})

geometry.zoom_best_fit()

#geometry.mesh()
#problem.solve()

"""
local_values = magnetic.local_values(0, 0.025)
print(local_values)

surface_integrals = heat.surface_integrals([0, 1, 2])
print(surface_integrals)

volume_integrals = magnetic.volume_integrals([0, 1, 2])
print(volume_integrals)
"""

view = agros2d.view

#view.post2d.scalar_view_show = True
#view.post2d.scalar_view_variable = "heat_heat_flux"
# FIXME! view.post2d.scalar_view_component = "magnitude"
#view.post2d.scalar_view_palette = "winter"
#view.post2d.scalar_view_palette_quality = "coarse"
#view.post2d.scalar_view_palette_steps = 20
#view.post2d.scalar_view_palette_filter = True

#view.post2d.scalar_view_log_scale = True
#view.post2d.scalar_view_log_base = 5

#view.post2d.scalar_view_color_bar = False
#view.post2d.scalar_view_decimal_place = 3

#view.post2d.scalar_view_auto_range = False
#view.post2d.scalar_view_range_min = 0
#view.post2d.scalar_view_range_max = 1

#view.post2d.contour_show = True
#view.post2d.contour_variable = ""
#view.post2d.contour_count = 15

#view.post2d.vector_show = True
#view.post2d.vector_variable = ""
#view.post2d.vector_count = 50
#view.post2d.vector_scale = 1.0
#view.post2d.vector_proportional = True
#view.post2d.vector_color = True

#view.mesh.activate
#view.mesh.initial_mesh_view_show = True
#view.mesh.solution_mesh_view_show = True

#view.mesh.order_view_show = True
#view.mesh.order_view_color_bar = True
#view.mesh.order_view_label = True
#view.mesh.order_view_palette = "jet"

"""
geometry.select_nodes([0])
geometry.select_nodes()
geometry.select_edges([1, 2, 3])
geometry.select_edges()
geometry.select_edges([1])
geometry.select_edges()
geometry.select_none()

geometry.zoom_best_fit()

geometry.select_nodes()
geometry.move_selection(1.5, 1.5, True)
geometry.rotate_selection(0.5, 0.5, 90)
geometry.scale_selection(0.5, 0.5, 2)
geometry.select_labels()
geometry.remove_selection()
"""