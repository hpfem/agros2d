print("Test: Current field - planar");

// model
newDocument("Feeder", "planar", "current field", 2, 5, 0, "disabled", 1, 1);

// boundaries
addBoundary("Neumann", "inward_current_flow", 0);
addBoundary("Zero", "potential", 0);
addBoundary("Voltage", "potential", 1);

// materials
addMaterial("mat 1", 1e7);
addMaterial("mat 2", 1e5);
addMaterial("mat 3", 1e3);

// edges
addEdge(0, 0, 0.6, 0, 0, "Zero");
addEdge(0, 0.8, 0, 0.5, 0, "Neumann");
addEdge(0, 0.5, 0, 0, 0, "Neumann");
addEdge(0, 0, 0.35, 0.5, 0, "none");
addEdge(0.35, 0.5, 0.6, 0.5, 0, "none");
addEdge(0.6, 0.8, 0.6, 0.5, 0, "Neumann");
addEdge(0.6, 0.5, 0.6, 0, 0, "Neumann");
addEdge(0, 0.5, 0.35, 0.5, 0, "none");
addEdge(0, 0.8, 0.6, 0.8, 0, "Voltage");

// labels
addLabel(0.3, 0.670924, 0, "mat 1");
addLabel(0.105779, 0.364111, 0, "mat 2");
addLabel(0.394296, 0.203668, 0, "mat 3");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(0.11879,0.346203);
testPotential = (Math.abs(point.V) - 0.928377) < 1e-2;
if (!testPotential) print(point.V);

// charge
integral = surfaceIntegral(0);
testI = (Math.abs(integral.I) - 3629.425713) < 10.0;
if (!testI) print(integral.I);

print(testPotential && testI);