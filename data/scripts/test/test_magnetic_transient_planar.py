# model
newdocument("unnamed", "planar", "magnetic", 0, 3, "disabled", 1, 1, 50000, "transient", 2.5e-08, 2e-06, 0)

# boundaries
addboundary("Neumann", "magnetic_surface_current_density", 0)
addboundary("A = 0", "magnetic_vector_potential", 0)

# materials
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Copper", 1e6/4, 0, 1, 57e6, 0, 0, 0, 0, 0)

# edges
addedge(0, 0.002, 0, 0.000768, 0, "Neumann")
addedge(0, 0.000768, 0, 0, 0, "Neumann")
addedge(0, 0, 0.000768, 0, 0, "Neumann")
addedge(0.000768, 0, 0.002, 0, 0, "Neumann")
addedge(0.002, 0, 0, 0.002, 90, "A = 0")
addedge(0.000768, 0, 0.000576, 0.000192, 90, "none")
addedge(0.000576, 0.000192, 0.000384, 0.000192, 0, "none")
addedge(0.000192, 0.000384, 0.000384, 0.000192, 90, "none")
addedge(0.000192, 0.000576, 0.000192, 0.000384, 0, "none")
addedge(0.000192, 0.000576, 0, 0.000768, 90, "none")

# labels
addlabel(0.000585418, 0.00126858, 0, "Air")
addlabel(0.000109549, 8.6116e-05, 0, "Copper")

# solve
zoombestfit()
solve()

# point value
point = pointresult(6.447965e-4,9.432763e-5)
testA = test("Magnetic potential", point["A_real"], 7.574698e-9)
testB = test("Flux density", point["B_real"], 7.107446e-6)
testBx = test("Flux density - x", point["Bx_real"], -3.451261e-6)
testBy = test("Flux density - y", point["By_real"], 6.213259e-6)
testH = test("Magnetic intensity", point["H_real"], 5.655926)
testHx = test("Magnetic intensity - x", point["Hx_real"], -2.746426)
testHy = test("Magnetic intensity - y", point["Hy_real"], 4.944354)
testwm = test("Energy density", point["wm"], 2.009959e-5)
testpj = test("Losses density ", point["pj"], 63.874077)
testJe = test("Current density - external", point["Je_real"], 2.5e5)
testJit = test("Current density - induced transform", point["Jit_real"], -1.896608e5)
testJ = test("Current density - total", point["J_real"], 60339.227467)

# volume integral
volume = volumeintegral(1)
testWm = test("Energy", volume["Wm"], 2.252801e-12)
testPj = test("Losses", volume["Pj"], 6.475219e-6)
testIe = test("Current - external", volume["Ie_real"], 0.062534)
testIit = test("Current - induced transform", volume["Iit_real"], -0.054546)
testI = test("Current - total", volume["I_real"], 0.007989)

# surface integral
# surface = surfaceintegral(0)

print("Test: Magnetic transient - planar: " + str(point and testA and testB and testBx and testBy and testH and testHx and testHy and testwm and testpj 
and testJe and testJit and testJ and testWm and testPj and testIe and testIit and testI))

