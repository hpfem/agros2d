print("Test: Electrostatic - axisymmetric");

// model
newDocument("Electrostatic", "axisymmetric", "electrostatic", 0, 3, 0, "disabled", 1, 0);

// boundaries
addBoundary("Source electrode", "potential", 10);
addBoundary("Ground electrode", "potential", 0);
addBoundary("Neumann BC", "surface_charge_density", 0);

// materials
addMaterial("Air", 0, 1);
addMaterial("Dielectric n.1", 0, 10);
addMaterial("Dielectric n.2", 0, 3);

// edges
addEdge(0, 0.2, 0, 0.08, 0, "Neumann BC");
addEdge(0.01, 0.08, 0.01, 0, 0, "Source electrode");
addEdge(0.01, 0, 0.03, 0, 0, "Neumann BC");
addEdge(0.03, 0, 0.03, 0.08, 0, "none");
addEdge(0.03, 0.08, 0.05, 0.08, 0, "none");
addEdge(0.05, 0, 0.03, 0, 0, "Neumann BC");
addEdge(0.05, 0.08, 0.05, 0, 0, "Ground electrode");
addEdge(0.06, 0, 0.06, 0.08, 0, "Ground electrode");
addEdge(0.05, 0.08, 0.06, 0.08, 0, "Ground electrode");
addEdge(0.06, 0, 0.2, 0, 0, "Neumann BC");
addEdge(0.2, 0, 0, 0.2, 90, "Neumann BC");
addEdge(0.01, 0.08, 0.03, 0.08, 0, "none");
addEdge(0.01, 0.08, 0, 0.08, 0, "Source electrode");

// labels
addLabel(0.019, 0.021, 5e-05, "Dielectric n.1");
addLabel(0.0379, 0.051, 5e-05, "Dielectric n.2");
addLabel(0.0284191, 0.123601, 0.0001, "Air");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(0.023834,0.068108);
testPotential = Math.abs(point.Potential - 6.699811) < 1e-2;
if (!testPotential) print(point.Potential);

// energy
integral = volumeIntegral();
testEnergy = Math.abs(integral.Energy - 8.311842e-10) < 1e-10;
if (!testEnergy) print(integral.Energy);

// charge
integral = surfaceIntegral(1, 12);
testQ = Math.abs(integral.Q - 1.658438e-10) < 1e-11;
if (!testQ) print(integral.Q);

print(testPotential && testEnergy && testQ);