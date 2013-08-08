import pythonlab
import agros2d as a2d

def model():
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "planar"
    problem.mesh_type = "triangle"
    
    # fields
    # electrostatic
    electrostatic = a2d.field("electrostatic")
    electrostatic.analysis_type = "steadystate"
    electrostatic.matrix_solver = "mumps"
    electrostatic.number_of_refinements = 1
    electrostatic.polynomial_order = 2
    electrostatic.adaptivity_type = "disabled"
    electrostatic.solver = "linear"
    
    
    # boundaries
    electrostatic.add_boundary("Voltage", "electrostatic_potential", {"electrostatic_potential" : 1e4})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    electrostatic.add_material("Screen", {"electrostatic_permittivity" : 2, "electrostatic_charge_density" : 0})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0.075, -0.022, 0.075, 0.022, angle = 90)
    geometry.add_edge(0.075, -0.022, 0.079, -0.026)
    geometry.add_edge(0.079, -0.026, 0.079, 0.026, angle = 90)
    geometry.add_edge(0.079, 0.026, 0.075, 0.022)
    geometry.add_edge(0, 0.06, 0, -0.06, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0, -0.06, 0.118, -0.06, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.118, -0.06, 0.118, 0.06, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.118, 0.06, 0, 0.06, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.05, 0.01, 0.04, 0.01, boundaries = {"electrostatic" : "Voltage"})
    geometry.add_edge(0.04, 0.011, 0.04, 0.01, boundaries = {"electrostatic" : "Voltage"})
    geometry.add_edge(0.04, 0.011, 0.05, 0.011, boundaries = {"electrostatic" : "Voltage"})
    geometry.add_edge(0.05, 0.011, 0.05, 0.01, boundaries = {"electrostatic" : "Voltage"})
    geometry.add_edge(0.05, -0.01, 0.05, -0.011, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.05, -0.011, 0.04, -0.011, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.04, -0.01, 0.04, -0.011, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.04, -0.01, 0.05, -0.01, boundaries = {"electrostatic" : "Ground"})
    
    geometry.add_label(0.0805206, 0.0216256, materials = {"electrostatic" : "Screen"})
    geometry.add_label(0.0460776, 0.0105177, materials = {"electrostatic" : "none"})
    geometry.add_label(0.045095, -0.0105089, materials = {"electrostatic" : "none"})
    geometry.add_label(0.109507, 0.050865, materials = {"electrostatic" : "Air"})

    a2d.view.zoom_best_fit()
    problem.solve()

# solve model
model()

fn_geometry = pythonlab.tempname("vtk")
a2d.geometry.export_vtk(fn_geometry)
geometry = a2d.vtk_geometry_actor(fn_geometry)

fn_contour = pythonlab.tempname("vtk")
a2d.view.post2d.contour_view_parameters["variable"] = "electrostatic_potential"
a2d.view.post2d.export_contour_vtk(fn_contour)
contours = a2d.vtk_contours_actor(fn_contour, count = 15, color = False)

fn_scalar = pythonlab.tempname("vtk")
a2d.view.post2d.scalar_view_parameters["variable"] = "electrostatic_electric_field"
a2d.view.post2d.scalar_view_parameters["component"] = "magnitude"
a2d.view.post2d.export_scalar_vtk(fn_scalar)
scalar = a2d.vtk_scalar_actor(fn_scalar)

fn = pythonlab.tempname("png")
a2d.vtk_figure(output_filename = fn, 
               geometry = geometry,
               contours = contours,
               scalar = scalar,
               scalar_colorbar = True)
pythonlab.image(fn)