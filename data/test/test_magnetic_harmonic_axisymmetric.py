import agros2d

# model
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"
problem.frequency = 100

# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# fields
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "harmonic"
magnetic.number_of_refinements = 1
magnetic.polynomial_order = 3
magnetic.linearity_type = "linear"

magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6}) 
magnetic.add_material("Iron", {"magnetic_permeability" : 50, "magnetic_conductivity" : 5e3}) 
magnetic.add_material("Air", {"magnetic_permeability" : 1}) 

# geometry
geometry = agros2d.geometry

# edges
geometry.add_edge(0, -0.17, 0.15, -0.17, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.15, -0.17, 0.15, 0.17, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.15, 0.17, 0, 0.17, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0.035, -0.03, 0.055, -0.03)
geometry.add_edge(0.055, -0.03, 0.055, 0.11)
geometry.add_edge(0.055, 0.11, 0.035, 0.11)
geometry.add_edge(0.035, 0.11, 0.035, -0.03)
geometry.add_edge(0, -0.05, 0.03, -0.05)
geometry.add_edge(0.03, -0.05, 0.03, 0.05)
geometry.add_edge(0.03, 0.05, 0, 0.05)
geometry.add_edge(0, 0.05, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, 0.17, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
geometry.add_edge(0, -0.05, 0, -0.17, boundaries = {"magnetic" : "A = 0"})

# labels
geometry.add_label(0.109339, 0.112786, materials = {"magnetic" : "Air"}, area=0)
geometry.add_label(0.0442134, 0.0328588, materials = {"magnetic" : "Coil"}, area=0)
geometry.add_label(0.0116506, -0.00740064, materials = {"magnetic" : "Iron"}, area=0)

agros2d.view.zoom_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(0.027159, 0.039398)
testA = agros2d.test("Magnetic potential", point["A"], 0.001087)
testA_real = agros2d.test("Magnetic potential - real", point["Ar"], 0.001107)
testA_imag = agros2d.test("Magnetic potential - imag", point["Ai"], -5.24264e-6)
testB = agros2d.test("Flux density", point["B"], 0.099325)
testBr_real = agros2d.test("Flux density - r - real", point["Brr"], 0.027587)
testBr_imag = agros2d.test("Flux density - r - imag", point["Bir"], -2.430976e-4)
testBz_real = agros2d.test("Flux density - z - real", point["Brz"],  0.095414)
testBz_imag = agros2d.test("Flux density - z - imag", point["Biz"], 7.424088e-4)
testH = agros2d.test("Magnetic field", point["H"], 1580.808517)
testHr_real = agros2d.test("Magnetic field - r - real", point["Hrr"], 439.052884)
testHr_imag = agros2d.test("Magnetic field - r - imag", point["Hir"], -3.869019)
testHz_real = agros2d.test("Magnetic field - z - real", point["Hrz"], 1518.562988)
testHz_imag = agros2d.test("Magnetic field - z - imag", point["Hiz"], 11.815803)
testwm = agros2d.test("Energy density", point["wm"], 39.253502)
testpj = agros2d.test("Losses density ", point["pj"], 1210.138583)
testJit_real = agros2d.test("Current density - induced transform - real", point["Jitr"], -16.47024)
testJit_imag = agros2d.test("Current density - induced transform - imag", point["Jiti"], -3478.665629)
testJ_real = agros2d.test("Current density - total - real", point["Jr"], -16.47024)
testJ_imag = agros2d.test("Current density - total - imag", point["Ji"], -3478.665629)
testFr = agros2d.test("Lorentz force - r", point["Flr"], 0.505549)
testFz = agros2d.test("Lorentz force - z", point["Flz"], 0.650006)

# volume integral
volume = magnetic.volume_integrals([2])
testIit_real = agros2d.test("Current - induced transform - real", volume["Iitr"], -0.067164)
testIit_imag = agros2d.test("Current - induced transform - imag", volume["Iiti"], -5.723787)
testIe_real = agros2d.test("Current - external - real", volume["Ier"], 0.0)
testIe_imag = agros2d.test("Current - external - imag", volume["Iei"], 0.0)
testI_real = agros2d.test("Current - real", volume["Ir"], -0.067164)
testI_imag = agros2d.test("Current - imag", volume["Ii"], -5.723787)
testWm = agros2d.test("Energy", volume["Wm"], 0.009187)
testPj = agros2d.test("Losses", volume["Pj"], 0.228758)
testFLr = agros2d.test("Lorentz force - r", volume["Flx"], -4.018686e-4)
testFLz = agros2d.test("Lorentz force - z", volume["Fly"], -1.233904e-5)

print("Test: Magnetic harmonic - axisymmetric: " + str(testA and testA_real and testA_imag
                                                       and testB and testBr_real and testBr_imag and testBz_real and testBz_imag
                                                       and testH and testHr_real and testHr_imag and testHz_real and testHz_imag
                                                       and testJit_real and testJit_imag and testJ_real and testJ_imag
                                                       and testFr and testFz 
                                                       and testIe_real and testIe_imag and testIit_real and testIit_imag and testI_real and testI_imag
                                                       and testpj and testwm and testWm and testPj
                                                       and testFLr and testFLr))