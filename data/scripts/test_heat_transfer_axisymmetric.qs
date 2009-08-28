print("Test: Heat transfer - axisymmetric");

// model
newDocument("Heat transfer axisymmetric", "axisymmetric", "heat transfer", 1, 3, 0, "disabled", 10, 1);

// boundaries
addBoundary("Neumann", "heat_flux", 0, 0, 0)
addBoundary("Inlet", "heat_flux", 500000, 0, 0)
addBoundary("Temperature", "temperature", 0);
addBoundary("Convection", "heat_flux", 0, 20, 20)

// materials
addMaterial("Material", 6000000, 52);

// edges
addEdge(0.02, 0, 0.1, 0, 0, "Temperature");
addEdge(0.1, 0, 0.1, 0.14, 0, "Convection");
addEdge(0.1, 0.14, 0.02, 0.14, 0, "Temperature");
addEdge(0.02, 0.14, 0.02, 0.1, 0, "Neumann");
addEdge(0.02, 0.04, 0.02, 0, 0, "Neumann");
addEdge(0.02, 0.04, 0.02, 0.1, 0, "Inlet");

// labels
addLabel(0.0460134, 0.0867717, 0.0003, "Material");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(0.062926,0.038129);
testTemperature = (Math.abs(point.T) - 263.811616) < 1e-2;
if (!testTemperature) print(point.T);

// average temperature
integral = volumeIntegral(0);
testTemperatureAvg = (Math.abs(integral.T_avg) - (0.949673/0.004222)) < 1e-2;
if (!testTemperatureAvg) print(integral.T_avg);

// heat flux
integral = surfaceIntegral(1);
testFlux = (Math.abs(integral.F) - 333.504915) < 1;
if (!testFlux) print(integral.F);

print(testTemperature && testTemperatureAvg && testFlux);