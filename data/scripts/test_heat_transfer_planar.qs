print("Test: Heat transfer - planar");

// model
newDocument("unnamed", "planar", "heat transfer", 1, 1, 0, "hp-adaptivity", 10, 0.1);

// boundaries
addBoundary("T inner", "temperature", -15);
addBoundary("T outer", "heat_flux", 0, 20, 20)
addBoundary("Neumann", "heat_flux", 0, 0, 0)

// materials
addMaterial("Material 1", 0, 2);
addMaterial("Material 2", 7e4, 10);

// edges
addEdge(0.1, 0.15, 0, 0.15, 0, "T outer");
addEdge(0, 0.15, 0, 0.1, 0, "Neumann");
addEdge(0, 0.1, 0.05, 0.1, 0, "T inner");
addEdge(0.05, 0.1, 0.05, 0, 0, "T inner");
addEdge(0.05, 0, 0.1, 0, 0, "Neumann");
addEdge(0.05, 0.1, 0.1, 0.1, 0, "none");
addEdge(0.1, 0.15, 0.1, 0.1, 0, "T outer");
addEdge(0.1, 0.1, 0.1, 0, 0, "T outer");

// labels
addLabel(0.0553981, 0.124595, 0.003, "Material 1");
addLabel(0.070091, 0.068229, 0.003, "Material 2");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(0.086266, 0.087725);
testTemperature = (Math.abs(point.T) - 4.363565) < 1e-2;
if (!testTemperature) print(point.T);

// average temperature
integral = volumeIntegral(0);
testTemperatureAvg = (Math.abs(integral.T_avg) - (0.023972/0.005)) < 1e-2;
if (!testTemperatureAvg) print(integral.T_avg);

// heat flux
integral = surfaceIntegral(0, 6, 7);
testFlux = (Math.abs(integral.F) - 103.220778) < 1;
if (!testFlux) print(integral.F);

print(testTemperature && testTemperatureAvg && testFlux);