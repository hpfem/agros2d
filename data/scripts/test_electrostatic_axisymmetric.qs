print("Test: Electrostatic - axisymmetric");

// model
newDocument("Electrostatic", "axisymmetric", "electrostatic", 0, 3);

// boundaries
addBoundary("Source electrode", "electrostatic_potential", 10);
addBoundary("Ground electrode", "electrostatic_potential", 0);
addBoundary("Neumann BC", "electrostatic_surface_charge_density", 0);

// materials
addMaterial("Air", 0, 1);
addMaterial("Dielectric n.1", 1e-5, 10);
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
point = pointResult(0.026532, 0.070937);
testPotential = abs(abs(point.V) - 26.078318) < 0.1;
if (!testPotential) print("Scalar potential: ", abs(point.V), " == ", 26.078318);

// energy
integral = volumeIntegral();
testEnergy = abs(abs(integral.We) - 1.799349e-8) < 1e-10;
if (!testEnergy) print("Electric energy: ", abs(integral.We), " == ", 1.799349e-8);

// charge
integral = surfaceIntegral(1, 12);
testQ = abs(abs(integral.Q) - 1.291778e-9) < 1e-11;
if (!testQ) print("Electric charge: ", abs(integral.Q), " == ", 1.291778e-9);

print(testPotential && testEnergy && testQ);
