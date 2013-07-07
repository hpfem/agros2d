import agros2d as a2d
from math import *

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "steadystate"
magnetic.number_of_refinements = 0
magnetic.polynomial_order = 2
magnetic.solver = "linear"

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
magnetic.add_material("Fe", {"magnetic_permeability" : 500}) 
magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
magnetic.add_material("Cu rotor", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e6}) 
magnetic.add_material("Cu stator", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e6}) 

stator_diameter_outer = {{txtStatorDiameterOuter_text}}
stator_diameter_inner = {{txtStatorDiameterInner_text}}
stator_slots = {{txtStatorSlots_text}}

rotor_diameter_outer = {{txtRotorDiameter_text}}
rotor_slots = {{txtRotorSlots_text}}

shaft_diameter = 0.200

geometry = a2d.geometry

# rotor
def geometry_rotor_slots(N, A, B, C, D, E):
    k1x = -(rotor_diameter_outer / 2.0)
    k1y = E / 2.0
    k2x = -(rotor_diameter_outer / 2.0)
    k2y = -E / 2.0
    k3x = -(rotor_diameter_outer / 2.0) + D
    k3y = E / 2.0
    k4x = -(rotor_diameter_outer / 2.0) + D
    k4y = -E / 2.0
    k5x = -(rotor_diameter_outer / 2.0) + D
    k5y = A / 2.0
    k6x = -(rotor_diameter_outer / 2.0) + D
    k6y = -A / 2.0
    k7x = -(rotor_diameter_outer / 2.0) + D + C
    k7y = B / 2.0
    k8x = -(rotor_diameter_outer / 2.0) + D + C
    k8y = -B / 2.0
    
    angle = 2.0 * pi / N

    e1 = geometry.add_edge(k1x, k1y, k3x, k3y)
    e2 = geometry.add_edge(k3x, k3y, k5x, k5y)
    e3 = geometry.add_edge(k5x, k5y, k7x, k7y)
    e4 = geometry.add_edge(k7x, k7y, k8x, k8y)
    e5 = geometry.add_edge(k8x, k8y, k6x, k6y)
    e6 = geometry.add_edge(k6x, k6y, k4x, k4y)
    e7 = geometry.add_edge(k4x, k4y, k2x, k2y)

    geometry.select_edges([e1, e2, e3, e4, e5, e6, e7])   
    for i in range(N-1):
        geometry.rotate_selection(0.0, 0.0, angle / pi * 180.0, True)

    for i in range(N):
        geometry.add_edge(- sqrt(k1x**2 + k1y**2) * cos(i * angle + atan(k1y / k1x)), - sqrt(k1x**2 + k1y**2) * sin(i * angle + atan(k1y / k1x)), - sqrt(k2x**2 + k2y**2) * cos(i * angle + atan(k2y / k2x)), - sqrt(k2x**2 + k2y**2) * sin(i * angle + atan(k2y / k2x)), abs(2.0 * atan(k1y / k1x)) / pi * 180.0)            
        geometry.add_edge(- sqrt(k2x**2 + k2y**2) * cos(i * angle + atan(k2y / k2x)), - sqrt(k2x**2 + k2y**2) * sin(i * angle + atan(k2y / k2x)), - sqrt(k1x**2 + k1y**2) * cos((i+1) * angle + atan(k1y / k1x)), - sqrt(k1x**2 + k1y**2) * sin((i+1) * angle + atan(k1y / k1x)), (angle - abs(2.0 * atan(k1y / k1x))) / pi * 180.0)            
        geometry.add_label(- sqrt(((k5x + k7x) / 2.0)**2 + ((k1y + k2y) / 2.0)**2) * cos(i * angle), - sqrt(((k5x + k7x) / 2.0)**2 + ((k1y + k2y) / 2.0)**2) * sin(i * angle), materials = {"magnetic" : "Cu rotor"})

# stator
def geometry_stator_slots(N, A, B, C, D, E):
    k1x = -(stator_diameter_inner / 2)
    k1y = C / 2
    k2x = -(stator_diameter_inner / 2)
    k2y = -C / 2
    k3x = -(stator_diameter_inner / 2) - D
    k3y = C / 2
    k4x = -(stator_diameter_inner / 2) - D
    k4y = -C / 2
    k5x = -(stator_diameter_inner / 2) - D - B
    k5y = (C / 2) + E
    k6x = -(stator_diameter_inner / 2) - D - B
    k6y = -(C / 2) - E
    k7x = -(stator_diameter_inner / 2) - D - B
    k7y = C / 2
    k8x = -(stator_diameter_inner / 2) - D - B
    k8y = -C / 2
    k9x = -(stator_diameter_inner / 2) - D - B - A
    k9y = C / 2
    k10x = -(stator_diameter_inner / 2) - D - B - A
    k10y = -C / 2
    
    angle = 2.0 * pi / N

    e1 = geometry.add_edge(k1x, k1y, k3x, k3y)
    e2 = geometry.add_edge(k3x, k3y, k5x, k5y)
    e3 = geometry.add_edge(k5x, k5y, k7x, k7y)
    e4 = geometry.add_edge(k7x, k7y, k9x, k9y)
    e5 = geometry.add_edge(k9x, k9y, k10x, k10y)
    e6 = geometry.add_edge(k10x, k10y, k8x, k8y)
    e7 = geometry.add_edge(k8x, k8y, k6x, k6y)
    e8 = geometry.add_edge(k6x, k6y, k4x, k4y)
    e9 = geometry.add_edge(k4x, k4y, k2x, k2y)
    
    geometry.select_edges([e1, e2, e3, e4, e5, e6, e7, e8, e9])
    for i in range(N-1):        
        geometry.rotate_selection(0.0, 0.0, angle / pi * 180.0, True)

    for i in range(N):
        geometry.add_edge(- sqrt(k1x**2 + k1y**2) * cos(i * angle + atan(k1y / k1x)), - sqrt(k1x**2 + k1y**2) * sin(i * angle + atan(k1y / k1x)), - sqrt(k2x**2 + k2y**2) * cos(i * angle + atan(k2y / k2x)), - sqrt(k2x**2 + k2y**2) * sin(i * angle + atan(k2y / k2x)), abs(2.0 * atan(k1y / k1x)) / pi * 180.0)            
        geometry.add_edge(- sqrt(k2x**2 + k2y**2) * cos(i * angle + atan(k2y / k2x)), - sqrt(k2x**2 + k2y**2) * sin(i * angle + atan(k2y / k2x)), - sqrt(k1x**2 + k1y**2) * cos((i+1) * angle + atan(k1y / k1x)), - sqrt(k1x**2 + k1y**2) * sin((i+1) * angle + atan(k1y / k1x)), (angle - abs(2.0 * atan(k1y / k1x))) / pi * 180.0)            
        geometry.add_label(- sqrt(((k5x + k7x) / 2.0)**2 + ((k1y + k2y) / 2.0)**2) * cos(i * angle), - sqrt(((k5x + k7x) / 2.0)**2 + ((k1y + k2y) / 2.0)**2) * sin(i * angle), materials = {"magnetic" : "Cu stator"})

def geometry_cooling_system(diameter, hole, angle, count):
	for i in range(count):
		geometry.add_circle(diameter / 2.0 * cos(angle * pi / 180.0 + i * 2*pi / count), diameter / 2.0 * sin(angle * pi / 180.0 + i * 2*pi / count), hole, materials = {"magnetic" : "Air"})

# rotor
geometry_rotor_slots(rotor_slots, {{txtRotorSlotA_text}}, {{txtRotorSlotB_text}}, {{txtRotorSlotC_text}}, {{txtRotorSlotD_text}}, {{txtRotorSlotE_text}})
geometry.add_label(1.1 * shaft_diameter / 2.0, 0.0, materials = {"magnetic" : "Fe"})

# rotor cooling system
geometry_cooling_system({{txtRotorCoolingSystemDiameter_text}}, {{txtRotorCoolingSystemHoleDiameter_text}}, {{txtRotorCoolingSystemAngle_text}}, {{txtRotorCoolingSystemCount_value}})
# geometry_cooling_system(0.500, 0.01, 18.0, 10)

# stator
geometry_stator_slots(stator_slots, {{txtStatorSlotA_text}}, {{txtStatorSlotB_text}}, {{txtStatorSlotC_text}}, {{txtStatorSlotD_text}}, {{txtStatorSlotE_text}})

geometry.add_circle(0.0, 0.0, stator_diameter_outer / 2.0, boundaries = {"magnetic" : "A = 0"})
geometry.add_label((stator_diameter_outer - 0.02 * stator_diameter_inner) / 2.0, 0.0, materials = {"magnetic" : "Fe"})

# stator cooling system
geometry_cooling_system({{txtStatorCoolingSystemDiameter_text}}, {{txtStatorCoolingSystemHoleDiameter_text}}, {{txtStatorCoolingSystemAngle_text}}, {{txtStatorCoolingSystemCount_value}})
# geometry_cooling_system(1.620, 0.01, 18.0, 10)

# shaft
geometry.add_circle(0.0, 0.0, shaft_diameter / 2.0)
geometry.add_label(0.0, 0.0, materials = {"magnetic" : "Fe"})

# airgap
geometry.add_label((stator_diameter_inner + rotor_diameter_outer) / 4.0, 0.0, materials = {"magnetic" : "Air"})

a2d.view.zoom_best_fit()
