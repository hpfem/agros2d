# model
newdocument("unnamed", "planar", "heat", 2, 3);

# boundaries
addboundary("T inner", "heat_temperature", -15);
addboundary("T outer", "heat_heat_flux", 0, 20, 20)
addboundary("Neumann", "heat_heat_flux", 0, 0, 0)

# materials
addmaterial("Material 1", 0, 2, 0, 0);
addmaterial("Material 2", 7e4, 10, 0, 0);

# edges
addedge(0.1, 0.15, 0, 0.15, 0, "T outer");
addedge(0, 0.15, 0, 0.1, 0, "Neumann");
addedge(0, 0.1, 0.05, 0.1, 0, "T inner");
addedge(0.05, 0.1, 0.05, 0, 0, "T inner");
addedge(0.05, 0, 0.1, 0, 0, "Neumann");
addedge(0.05, 0.1, 0.1, 0.1, 0, "none");
addedge(0.1, 0.15, 0.1, 0.1, 0, "T outer");
addedge(0.1, 0.1, 0.1, 0, 0, "T outer");

#labels
addlabel(0.0553981, 0.124595, 0.003, "Material 1");
addlabel(0.070091, 0.068229, 0.003, "Material 2");

# solve
zoombestfit();
solve();

# point value
point = pointresult(0.086266, 0.087725);
testTemperature = abs(abs(point["T"]) - 4.363565) < 1e-2;
if (not testTemperature): 
    print "Temperature: ", abs(point["T"]), " == ", 4.363565

# average temperature
integral = volumeintegral(0);
testTemperatureAvg = abs(abs(integral["T_avg"]) - (0.023972/0.005)) < 1e-2;
if (not testTemperatureAvg):
    print "Avg. temperature: ", abs(integral["T_avg"]), " == ", (0.023972/0.005)

# heat flux
integral = surfaceintegral(0, 6, 7);
testFlux = abs(abs(integral["F"]) - 103.220778) < 1;
if (not testFlux):
    print "Heat flux: ", abs(integral["F"]), " == ", 103.220778

print "Test: Heat transfer - planar: ", (testTemperature and testTemperatureAvg and testFlux)
