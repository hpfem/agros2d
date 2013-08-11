import pythonlab
import agros2d as a2d

def model():
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"
    problem.mesh_type = "triangle"
    
    # fields
    # electrostatic
    electrostatic = a2d.field("electrostatic")
    electrostatic.analysis_type = "steadystate"
    electrostatic.matrix_solver = "mumps"
    electrostatic.number_of_refinements = 1
    electrostatic.polynomial_order = 2
    electrostatic.solver = "linear"    
    
    # boundaries
    electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0.2, 1, 0, 0.5, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0, 0.5, 0, 0.25, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -0.25, 0, -1, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -1, 1.5, 0.5, angle = 90, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(1.5, 0.5, 0, 2, angle = 90, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, 1, 0.2, 1, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0, 2, 0, 1, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -0.25, 0.25, 0, angle = 90, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"electrostatic" : "Ground"})
    
    geometry.add_label(0.879551, 0.764057, area = 0.06, materials = {"electrostatic" : "Air"})
    a2d.view.zoom_best_fit()
    problem.solve()

# solve model
model()

fn_geometry = pythonlab.tempname("vtk")
a2d.geometry.export_vtk(fn_geometry)
geometry = a2d.vtk_geometry_actor(fn_geometry)

fn_contour = pythonlab.tempname("vtk")
#a2d.view.post2d.contour_view_parameters["width"] = 1.0
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
               scalar_colorbar = True, 
               width = 500, 
               height = 300)
pythonlab.image(fn)