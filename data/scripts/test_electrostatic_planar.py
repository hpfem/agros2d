# model
newdocument("Elec2 (QuickField)", "planar", "electrostatic", 2, 3)

# boundaries
addboundary("Neumann", "electrostatic_surface_charge_density", 0)
addboundary("U =  0 V", "electrostatic_potential", 0)
addboundary("U =  1000 V", "electrostatic_potential", 1000)

# materials
addmaterial("Diel", 0, 3)
addmaterial("Air", 0, 1)
addmaterial("Source", 4e-10, 10)

# edges
addedge(1, 2, 1, 1, 0, "U =  1000 V")
addedge(4, 1, 1, 1, 0, "U =  1000 V")
addedge(1, 2, 4, 2, 0, "U =  1000 V")
addedge(4, 2, 4, 1, 0, "U =  1000 V")
addedge(20, 24, 20, 1, 0, "Neumann")
addedge(20, 1, 20, 0, 0, "Neumann")
addedge(4, 1, 20, 1, 0, "none")
addedge(0, 24, 0, 1, 0, "Neumann")
addedge(0, 0, 0, 1, 0, "Neumann")
addedge(0, 0, 20, 0, 0, "U =  0 V")
addedge(0, 24, 20, 24, 0, "Neumann")
addedge(0, 1, 1, 1, 0, "none")
addedge(7, 13, 14, 13, 0, "none")
addedge(14, 13, 14, 18, 0, "none")
addedge(14, 18, 7, 18, 0, "none")
addedge(7, 18, 7, 13, 0, "none")

# labels
addlabel(2.78257, 1.37346, 0, "none")
addlabel(10.3839, 15.7187, 0, "Source")
addlabel(3.37832, 15.8626, 0, "Air")
addlabel(12.3992, 0.556005, 0, "Diel")

# solve
zoombestfit()
solve()

# point value
point = pointresult(13.257584, 11.117738)
testPotential = abs(abs(point["V"]) - 1111.544813) < 0.5
if (not testPotential): 
    print "Scalar potential: ", abs(point["V"]), " == ", 1111.544813

# energy
integral = volumeintegral(1)
testEnergy = abs(abs(integral["We"]) - 1.307484e-7) < 1e-7
if (not testEnergy):
    print("Electric energy: " + str(abs(integral["We"])) + " == " + str(1.307484e-7))

# charge
integral = surfaceintegral(0, 1, 2, 3)
testCharge = abs(abs(integral["Q"]) - 1.048981e-7) < 5e-9
if (not testCharge):
    print("Electric charge: " + str(abs(integral["Q"])) + " == " + str(1.048981e-7))

print("Test: Electrostatic - planar: " + str(testPotential and testEnergy and testCharge))