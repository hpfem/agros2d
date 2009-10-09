print("Test: Current field - axisymmetric");

// model
newDocument("Tube", "axisymmetric", "current field", 1, 2, 0, "disabled", 0, 1);

// boundaries
addBoundary("Neumann", "inward_current_flow", 0);
addBoundary("Ground", "potential", 0);
addBoundary("Voltage", "potential", 10);

// materials
addMaterial("Copper", 57e6);

// edges
addEdge(0, 0.45, 0, 0, 0, "Neumann");
addEdge(0, 0, 0.2, 0, 0, "Ground");
addEdge(0.2, 0, 0.2, 0.15, 0, "Neumann");
addEdge(0.2, 0.15, 0.35, 0.45, 0, "Neumann");
addEdge(0.35, 0.45, 0, 0.45, 0, "Voltage");

// labels
addLabel(0.125753, 0.324715, 0.001, "Copper");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(0.213175,0.25045);
testPotential = abs(abs(point.V) - 7.285427) < 1e-2;
if (!testPotential) print("Scalar potential: ", abs(point.V), " == ", 7.285427);

// current
integral = surfaceIntegral(1);
testI = abs(abs(integral.I) - 2.124955e8) < 1e6;
if (!testI) print("Current: ", abs(integral.I), " == ", 2.124955e8);

print(testPotential && testI);