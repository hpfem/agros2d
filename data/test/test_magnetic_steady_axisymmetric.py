import agros2d
from time import time
start_time = time()

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"

problem.frequency = 100

# disable view
agros2d.view.mesh.disable()
agros2d.view.post2d.disable()

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "steadystate"
magnetic.number_of_refinements = 0
magnetic.polynomial_order = 7
magnetic.solver = "linear"

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})

magnetic.add_material("Cu", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e6}) 
magnetic.add_material("Fe", {"magnetic_permeability" : 300}) 
magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_remanence" : 0.6, "magnetic_remanence_angle" : 90}) 

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0.01, 0.01, 0.01, 0)
geometry.add_edge(0.01, 0, 0.04, 0,)
geometry.add_edge(0.04, 0.1, 0, 0.1,)
geometry.add_edge(0, 0.1, 0, 0.08, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.08, 0.01, 0.08)
geometry.add_edge(0.01, 0.09, 0.03, 0.09)
geometry.add_edge(0.01, 0.09, 0.01, 0.08)
geometry.add_edge(0.01, 0.01, 0.03, 0.01)
geometry.add_edge(0.012, 0.088, 0.012, 0.012)
geometry.add_edge(0.012, 0.012, 0.028, 0.012)
geometry.add_edge(0.028, 0.012, 0.028, 0.088)
geometry.add_edge(0.028, 0.088, 0.012, 0.088)
geometry.add_edge(0, 0.05, 0.009, 0.05)
geometry.add_edge(0.009, 0.05, 0.009, -0.02)
geometry.add_edge(0, 0.05, 0, -0.02, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, -0.02, 0.009, -0.02)
geometry.add_edge(0, 0.15, 0, 0.1, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.08, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, -0.02, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, -0.05, 0.1, 0.05, boundaries = {"magnetic" : "A = 0"}, angle=90)
geometry.add_edge(0.1, 0.05, 0, 0.15, boundaries = {"magnetic" : "A = 0"}, angle=90)
geometry.add_edge(0.03, 0.09, 0.03, 0.078)
geometry.add_edge(0.03, 0.078, 0.04, 0.078)
geometry.add_edge(0.04, 0.078, 0.04, 0.1)
geometry.add_edge(0.04, 0.078, 0.04, 0.052)
geometry.add_edge(0.03, 0.078, 0.03, 0.052)
geometry.add_edge(0.03, 0.052, 0.04, 0.052)
geometry.add_edge(0.04, 0.052, 0.04, 0)
geometry.add_edge(0.03, 0.01, 0.03, 0.052)

# labels
geometry.add_label(0.0348743, 0.0347237, materials = {"magnetic" : "Fe"})
geometry.add_label(0.00512569, -0.0070852, materials = {"magnetic" : "Fe"})
geometry.add_label(0.021206, 0.0692964, materials = {"magnetic" : "Cu"})
geometry.add_label(0.0141705, 0.12445, materials = {"magnetic" : "Air"})
geometry.add_label(0.0346923, 0.0892198, materials = {"magnetic" : "Fe"})
geometry.add_label(0.036093, 0.0654078, materials = {"magnetic" : "Magnet"})

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(0.005985, 0.043924)
testA = agros2d.test("Scalar potential", point["Ar"], 5.438198e-4)
testB = agros2d.test("Flux density", point["Br"], 0.195525)
testBr = agros2d.test("Flux density - r", point["Brr"], 0.059085)
testBz = agros2d.test("Flux density - z", point["Brz"], 0.186384)
testH = agros2d.test("Magnetic intensity", point["Hr"], 518.646027)
testHr = agros2d.test("Magnetic intensity - r", point["Hrr"], 156.728443)
testHz = agros2d.test("Magnetic intensity - z", point["Hrz"], 494.39852)
testwm = agros2d.test("Energy density", point["wm"], 50.704118)

point = magnetic.local_values(0.0199415, 0.0609942)
testFr_real = agros2d.test("Lorentz force - r - real", point["Flr"], -1.237274e5)
testFz_real = agros2d.test("Lorentz force - z - real", point["Flz"], -41951.726164)

# volume integral
volume = magnetic.volume_integrals([1])
testWm = agros2d.test("Energy", volume["Wm"], 0.002273)

volume = magnetic.volume_integrals([2])
testFLr = agros2d.test("Lorentz force - r", volume["Flx"], -8.069509) # TODO: Flr
testFLz = agros2d.test("Lorentz force - z", volume["Fly"], -5.288991) # TODO: Flz

# surface integral
surface = magnetic.surface_integrals([12, 13, 14, 15])
#testFz = agros2d.test("Maxwell force - z", surface["Fy"], 0.368232)

result = str(testA and testB and testBr and testBz and testH and testHr and testHz
             and testFr_real and testFz_real and testwm and testWm and testFLr and testFLz)

elapsed_time = time() - start_time
print("Test: Magnetic steady state - axisymmetric ({0}): ".format(round(elapsed_time, 3)) + result)