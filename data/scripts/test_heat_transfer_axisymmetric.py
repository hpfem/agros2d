# model
newdocument("Heat transfer axisymmetric", "axisymmetric", "heat", 1, 3)

# boundaries
addboundary("Neumann", "heat_heat_flux", 0, 0, 0)
addboundary("Inlet", "heat_heat_flux", 500000, 0, 0)
addboundary("Temperature", "heat_temperature", 0)
addboundary("Convection", "heat_heat_flux", 0, 20, 20)

# materials
addmaterial("Material", 6000000, 52, 0, 0)

# edges
addedge(0.02, 0, 0.1, 0, 0, "Temperature")
addedge(0.1, 0, 0.1, 0.14, 0, "Convection")
addedge(0.1, 0.14, 0.02, 0.14, 0, "Temperature")
addedge(0.02, 0.14, 0.02, 0.1, 0, "Neumann")
addedge(0.02, 0.04, 0.02, 0, 0, "Neumann")
addedge(0.02, 0.04, 0.02, 0.1, 0, "Inlet")

# labels
addlabel(0.0460134, 0.0867717, 0.0003, "Material")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.062926, 0.038129)
testTemperature = abs(abs(point["T"]) - 263.811616) < 1e-2
if (not testTemperature): 
    print "Temperature: ", abs(point["T"]), " == ", 263.811616

# average temperature
integral = volumeintegral(0)
testTemperatureAvg = abs(abs(integral["T_avg"]) - (0.949673/0.004222)) < 1e-1
if (not testTemperatureAvg):
    print("Avg. temperature: " + str(abs(integral["T_avg"])) + " == " + str(0.949673/0.004222))

# heat flux
integral = surfaceintegral(1)
testFlux = abs(abs(integral["F"]) - 333.504915) < 1
if (not testFlux):
    print("Heat flux: " + str(abs(integral["F"])) + " == " + str(333.504915))

print("Test: Heat transfer - axisymmetric: " + str(testTemperature and testTemperatureAvg and testFlux))