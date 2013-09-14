import agros2d as a2d

import os
if (os.name == "nt"):    
    import win32com.client as win32
    
    # problem
    problem = a2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"
    problem.mesh_type = "triangle"
    
    U = 10
    
    # electrostatic
    electrostatic = a2d.field("electrostatic")
    electrostatic.analysis_type = "steadystate"
    electrostatic.matrix_solver = "mumps"
    electrostatic.number_of_refinements = 1
    electrostatic.polynomial_order = 4
    electrostatic.adaptivity_type = "disabled"
    electrostatic.solver = "linear"
    
    # boundaries
    electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : U})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 0})
    electrostatic.add_material("Dielectric 2", {"electrostatic_permittivity" : 4, "electrostatic_charge_density" : 0})
    
    # geometry
    geometry = a2d.geometry
    geometry.add_edge(0, 0.2, 0, 0.08, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.01, 0.08, 0.01, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0.01, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.03, 0, 0.03, 0.08)
    geometry.add_edge(0.03, 0.08, 0.05, 0.08)
    geometry.add_edge(0.05, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.05, 0.08, 0.05, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.06, 0, 0.06, 0.08, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.05, 0.08, 0.06, 0.08, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.06, 0, 0.2, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.2, 0, 0, 0.2, angle = 90, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(0.01, 0.08, 0.03, 0.08)
    geometry.add_edge(0.01, 0.08, 0, 0.08, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
    
    geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric 1"})
    geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric 2"})
    geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})
    
    a2d.view.zoom_best_fit()
    problem.solve()
    
    volume = electrostatic.volume_integrals()
    capacity = 2.0 * volume["We"] / (U**2)
    
    # Excel object
    xl = win32.gencache.EnsureDispatch('Excel.Application')
    ss = xl.Workbooks.Add()
    sh = ss.ActiveSheet
    
    xl.Visible = True
    sh.Cells(1, 1).Value = 'Agros2D Excel Link'
    sh.Cells(3, 1).Value = 'Capacity'
    sh.Cells(3, 2).Value = capacity
    
    # ss.Close(False)
    # xl.Application.Quit()