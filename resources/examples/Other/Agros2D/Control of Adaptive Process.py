import math
import agros2d as a2d

ENERGY_TOLERANCE = 1e-9

def adaptivity_callback(adaptivity_step):    
    if (adaptivity_step > 0):
        electrostatic = a2d.field("electrostatic")    
        energy_difference = math.fabs(electrostatic.volume_integrals([0], None, adaptivity_step)["We"] - electrostatic.volume_integrals([0], None, adaptivity_step - 1)["We"])

        print("step = " + str(adaptivity_step) + ", energy difference = " + str(energy_difference) + " J")

        if (energy_difference < ENERGY_TOLERANCE):
            print('Energy_difference is lower then {0} J'.format(ENERGY_TOLERANCE))
            return False
    
    return True

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

# electrostatic
electrostatic = a2d.field("electrostatic")
electrostatic.analysis_type = "steadystate"
electrostatic.matrix_solver = "mumps"
electrostatic.number_of_refinements = 0
electrostatic.polynomial_order = 2
electrostatic.adaptivity_type = "hp-adaptivity"
electrostatic.adaptivity_parameters['steps'] = 10
electrostatic.adaptivity_parameters['tolerance'] = 0.0
electrostatic.adaptivity_parameters['threshold'] = 0.7
electrostatic.adaptivity_parameters['stopping_criterion'] = "singleelement"
electrostatic.adaptivity_parameters['error_calculator'] = "h1"
electrostatic.adaptivity_parameters['anisotropic_refinement'] = True
electrostatic.adaptivity_parameters['finer_reference_solution'] = False
electrostatic.adaptivity_parameters['space_refinement'] = True
electrostatic.adaptivity_parameters['order_increase'] = 1
electrostatic.adaptivity_callback = adaptivity_callback
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