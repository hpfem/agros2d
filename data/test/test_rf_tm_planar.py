import agros2d as a2d

# problem
problem = a2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 1e10

# fields
# rf_tm
rf_tm = a2d.field("rf_tm")
rf_tm.analysis_type = "harmonic"
rf_tm.number_of_refinements = 4
rf_tm.polynomial_order = 3
rf_tm.adaptivity_type = "disabled"
rf_tm.linearity_type = "linear"


# boundaries
rf_tm.add_boundary("Perfect electric conductor", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 0, "rf_tm_magnetic_field_imag" : 0})
rf_tm.add_boundary("Matched boundary", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 0, "rf_tm_magnetic_field_imag" : 0})
rf_tm.add_boundary("Surface current", "rf_tm_magnetic_field", {"rf_tm_magnetic_field_real" : 1, "rf_tm_magnetic_field_imag" : 0})


# materials
rf_tm.add_material("Air", {"rf_tm_permittivity" : 1, "rf_tm_permeability" : 1, "rf_tm_conductivity" : 0})

# geometry
geometry = a2d.geometry
geometry.add_edge(-0.01, 0.02286, -0.01, 0, boundaries = {"rf_tm" : "Surface current"})
geometry.add_edge(0.06907, 0.02286, 0.076, 0.01593, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.076, 0.01593, 0.081, 0.01593, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.01593, 0.081, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.086, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.081, 0.00693, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.00693, 0.05907, 0, angle = 20, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(-0.01, 0, 0.05907, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.17, 0.02286, 0.081, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.06907, 0.02286, -0.01, 0.02286, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.17, 0.02286, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.19286, -0.04, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.175, -0.015, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.015, 0.16, 0, angle = 90, boundaries = {"rf_tm" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.19286, -0.04, boundaries = {"rf_tm" : "Matched boundary"})
geometry.add_edge(0.086, 0, 0.16, 0, boundaries = {"rf_tm" : "Perfect electric conductor"})

geometry.add_label(0.0359418, 0.0109393, materials = {"rf_tm" : "Air"})
a2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = rf_tm.local_values(0.019107, 0.016725)
testH = agros2d.test("Magnetic field", point["H"], 2.26882983624)
testH_real = agros2d.test("Magnetic field - real", point["Hr"], 2.26882983624)
testH_imag = agros2d.test("Magnetic field - imag", point["Hi"], 0)

# testE = agros2d.test("Displacement", point["D"], 456.810483)
# testE_real = agros2d.test("Displacement - real", point["Dr"], 141.973049)
# testE_imag = agros2d.test("Displacement - imag", point["Di"], 434.18829)
#testB = agros2d.test("Flux density", point["B"], 1.115591e-6)
#testBx_real = agros2d.test("Flux density - x - real", point["Brx"], 6.483596e-7)
#testBx_imag = agros2d.test("Flux density - x - imag", point["Bix"], -2.301715e-7)
#testBy_real = agros2d.test("Flux density - y - real", point["Bry"], -4.300969e-7)
#testBy_imag = agros2d.test("Flux density - y - imag", point["Biy"], -7.656443e-7)
#testH = agros2d.test("Magnetic field", point["H"], 0.887759)
#testHx_real = agros2d.test("Magnetic field - x - real", point["Hrx"], 0.515948)
#testHx_imag = agros2d.test("Magnetic field - x - imag", point["Hix"], -0.183165)
#testHy_real = agros2d.test("Magnetic field - y - real", point["Hry"], -0.34226)
#testHy_imag = agros2d.test("Magnetic field - y - imag", point["Hiy"], -0.60928)
#testNx = agros2d.test("Poynting vector - x", point["Nx"], 156.567066)
#testNy = agros2d.test("Poynting vector - y", point["Ny"], -3.138616, 1)

# volume integral
# volume_integrals = rf.volume_integrals([0, 1, 2])
# testEnergy = agros2d.test("Energy", volume["We"], 1.799349e-8)

# surface integral
# surface_integrals = rf.surface_integrals([1, 12])
# testQ = agros2d.test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: RF TM - planar: " + str(testH and testH_real and testH_imag)) 
# and testB and testBx_real and testBx_imag and testBy_real and testBy_imag and
#testH and testHx_real and testHx_imag and testHy_real and testHy_imag and testNx and testNy))