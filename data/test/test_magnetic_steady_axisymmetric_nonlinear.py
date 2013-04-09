import agros2d

# problem
problem = agros2d.problem(clear = True)
problem.coordinate_type = "axisymmetric"
problem.mesh_type = "triangle"
problem.matrix_solver = "umfpack"

# disable view
agros2d.view.mesh.initial_mesh = False
agros2d.view.mesh.solution_mesh = False
agros2d.view.mesh.order = False
agros2d.view.post2d.scalar = False
agros2d.view.post2d.contours = False
agros2d.view.post2d.vectors = False

# magnetic
magnetic = agros2d.field("magnetic")
magnetic.analysis_type = "steadystate"
magnetic.number_of_refinements = 2
magnetic.polynomial_order = 2
magnetic.adaptivity_type = "disabled"
magnetic.linearity_type = "newton"
magnetic.nonlinear_tolerance = 1
magnetic.nonlinear_steps = 20
magnetic.automatic_damping = True
magnetic.damping_number_to_increase = 3



# boundaries
magnetic.add_boundary("A=0", "magnetic_potential", {"magnetic_potential_real" : 0})


# materials
magnetic.add_material("air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
magnetic.add_material("steel", {"magnetic_permeability" : { "value" : 1000, "x" : [0,0.454,1.1733,1.4147,1.7552,1.8595,1.9037,1.9418,2,5], "y" : [9300,9264,6717.2,4710.5,1664.8,763.14,453.7,194.13,100,1] }, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
magnetic.add_material("coil", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : { "expression" : "1e6" }})

# geometry
geometry = agros2d.geometry
geometry.add_edge(0, 0.15, 0, -0.15, boundaries = {"magnetic" : "A=0"})
geometry.add_edge(0, -0.15, 0.015, -0.15)
geometry.add_edge(0.015, -0.15, 0.015, 0.15)
geometry.add_edge(0, 0.15, 0.015, 0.15)
geometry.add_edge(0.1, 0.15, 0.1, -0.15)
geometry.add_edge(0.1, -0.15, 0.15, -0.15)
geometry.add_edge(0.15, -0.15, 0.15, 0.15)
geometry.add_edge(0.15, 0.15, 0.1, 0.15)
geometry.add_edge(0, 0.5, 0, 0.15, boundaries = {"magnetic" : "A=0"})
geometry.add_edge(0, -0.5, 0, -0.15, boundaries = {"magnetic" : "A=0"})
geometry.add_edge(0, -0.5, 0.5, 0, angle = 90, boundaries = {"magnetic" : "A=0"})
geometry.add_edge(0.5, 0, 0, 0.5, angle = 90, boundaries = {"magnetic" : "A=0"})

geometry.add_label(0.235433, 0.273031, materials = {"magnetic" : "air"})
geometry.add_label(0.00629508, 0.0397221, materials = {"magnetic" : "steel"})
geometry.add_label(0.125663, 0.0344, materials = {"magnetic" : "coil"})

agros2d.view.zoome_best_fit()

# solve problem
problem.solve()

# point value
point = magnetic.local_values(0.005985, 0.043924)
testA = agros2d.test("Scalar potential", point["Ar"], 0.00592236689936)
testB = agros2d.test("Flux density", point["Br"], 1.97915224026)
testBr = agros2d.test("Flux density - r", point["Brr"], 0.0101765610227)
testBz = agros2d.test("Flux density - z", point["Brz"], 1.97912607677)
testH = agros2d.test("Magnetic intensity", point["Hr"], 15189.5109368)
testHr = agros2d.test("Magnetic intensity - r", point["Hrr"], 78.1026248556)
testHz = agros2d.test("Magnetic intensity - z", point["Hrz"], 15189.3101384)
testwm = agros2d.test("Energy density", point["wm"], 15031.1772995)

point = magnetic.local_values(1.165e-01, 3.387e-02)
testFr_real = agros2d.test("Lorentz force - r - real", point["Flr"], 24645.3658523)
testFz_real = agros2d.test("Lorentz force - z - real", point["Flz"], -4734.00737537)

# volume integral
volume = magnetic.volume_integrals([0, 1, 2])
testWm = agros2d.test("Energy", volume["Wm"], 1.948e+01)

volume = magnetic.volume_integrals([2])
testFLr = agros2d.test("Lorentz force - r", volume["Flx"], 151.589357375) # TODO: Flr
testFLz = agros2d.test("Lorentz force - z", volume["Fly"], -0.00108431776502) # TODO: Flz

# surface integral
surface = magnetic.surface_integrals([0, 1, 2, 3])
#testFz = agros2d.test("Maxwell force - z", surface["Fy"], 0.368232)

print("Test: Magnetic steady state - axisymmetric: " + str(point and testA
                                                           and testB and testBr and testBz
                                                           and testH and testHr and testHz
                                                           and testFr_real and testFz_real
                                                           and testwm and testWm
                                                           and testFLr and testFLz))