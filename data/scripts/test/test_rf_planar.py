import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "planar"
problem.name = "RF_waveguide_R100"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 1.6e10

# fields
rf = agros2d.field("rf")
rf.analysis_type = "harmonic"
rf.number_of_refinements = 2
rf.polynomial_order = 3
rf.linearity_type = "linear"
rf.weak_forms = "compiled"

#newdocument("", "", "", 1, 3, "disabled", 1, 1, , "", 1.0, 1.0, 0.0)

# boundaries
rf.add_boundary("Perfect electric conductor", "rf_te_electric_field")
# rf.add_boundary("Matched boundary", "rf_te_matched_boundary")
rf.add_boundary("Matched boundary", "rf_te_electric_field")
rf.add_boundary("Surface current", "rf_te_surface_current", {"rf_te_surface_current_real" : 1, "rf_te_surface_current_imag" : 0.5})

rf.add_material("Air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 3e-2})

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(-0.01, 0.02286, -0.01, 0, 0, boundaries = {"rf" : "Surface current"})
geometry.add_edge(0.06907, 0.02286, 0.076, 0.01593, 90, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.076, 0.01593, 0.081, 0.01593, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.01593, 0.081, 0.02286, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.086, 0, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.086, 0.00693, 0.081, 0.00693, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.081, 0.00693, 0.05907, 0, 20, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(-0.01, 0, 0.05907, 0, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.17, 0.02286, 0.081, 0.02286, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.06907, 0.02286, -0.01, 0.02286, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.17, 0.02286, 90, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.19286, 0, 0.19286, -0.04, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.175, -0.015, 0, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.015, 0.16, 0, 90, boundaries = {"rf" : "Perfect electric conductor"})
geometry.add_edge(0.175, -0.04, 0.19286, -0.04, 0, boundaries = {"rf" : "Matched boundary"})
geometry.add_edge(0.086, 0, 0.16, 0, 0, boundaries = {"rf" : "Perfect electric conductor"})

# labels
geometry.add_label(0.0359418, 0.0109393, 5e-06, 0, materials = {"rf" : "Air"})

geometry.zoom_best_fit()

# solve problem
problem.solve()


# point value
local_values = rf.local_values(0.019107, 0.016725)
testE = agros2d.test("Electric field", point["E"], 456.810483)
testE_real = agros2d.test("Electric field - real", point["E_real"], 141.973049)
testE_imag = agros2d.test("Electric field - imag", point["E_imag"], 434.18829)
testB = test("Flux density", point["B"], 1.115591e-6)
testBx_real = agros2d.test("Flux density - x - real", point["Bx_real"], 6.483596e-7)
testBx_imag = agros2d.test("Flux density - x - imag", point["Bx_imag"], -2.301715e-7)
testBy_real = agros2d.test("Flux density - y - real", point["By_real"], -4.300969e-7)
testBy_imag = agros2d.test("Flux density - y - imag", point["By_imag"], -7.656443e-7)
testH = agros2d.test("Magnetic field", point["H"], 0.887759)
testHx_real = agros2d.test("Magnetic field - x - real", point["Hx_real"], 0.515948)
testHx_imag = agros2d.test("Magnetic field - x - imag", point["Hx_imag"], -0.183165)
testHy_real = agros2d.test("Magnetic field - y - real", point["Hy_real"], -0.34226)
testHy_imag = agros2d.test("Magnetic field - y - imag", point["Hy_imag"], -0.60928)
testPx = agros2d.test("Poynting vector - x", point["Px"], 156.567066)
testPy = agros2d.test("Poynting vector - y", point["Py"], -3.138616, 0.2)
testP = agros2d.test("Poynting vector", point["P"], 156.598521)

# volume integral
# volume_integrals = rf.volume_integrals([0, 1, 2])
# testEnergy = agros2d.test("Energy", volume["We"], 1.799349e-8)

# surface integral
# surface_integrals = rf.surface_integrals([1, 12])
# testQ = agros2d.test("Electric charge", surface["Q"], -1.291778e-9)

print("Test: RF TE - planar: " + str(
testE and testE_real and testE_imag and 
testB and testBx_real and testBx_imag and testBy_real and testBy_imag and
testH and testHx_real and testHx_imag and testHy_real and testHy_imag and
testP and testPx and testPy))