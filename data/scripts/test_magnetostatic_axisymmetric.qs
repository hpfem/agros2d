print("Test: Magnetostatic - axisymmetric");

// model
newDocument("unnamed", "axisymmetric", "magnetostatic", 0, 3, 0, "disabled", 1, 2);

// boundaries
addBoundary("A = 0", "vector_potential", 0);

// materials
addMaterial("Air", 0, 1);
addMaterial("Fe", 0, 300);
addMaterial("Cu", 1000000, 1);

// edges
addEdge(0.01, 0.01, 0.01, 0, 0, "none");
addEdge(0.01, 0, 0.04, 0, 0, "none");
addEdge(0.04, 0, 0.04, 0.1, 0, "none");
addEdge(0.04, 0.1, 0, 0.1, 0, "none");
addEdge(0, 0.1, 0, 0.08, 0, "A = 0");
addEdge(0, 0.08, 0.01, 0.08, 0, "none");
addEdge(0.01, 0.09, 0.03, 0.09, 0, "none");
addEdge(0.01, 0.09, 0.01, 0.08, 0, "none");
addEdge(0.03, 0.09, 0.03, 0.01, 0, "none");
addEdge(0.01, 0.01, 0.03, 0.01, 0, "none");
addEdge(0.012, 0.088, 0.012, 0.012, 0, "none");
addEdge(0.012, 0.012, 0.028, 0.012, 0, "none");
addEdge(0.028, 0.012, 0.028, 0.088, 0, "none");
addEdge(0.028, 0.088, 0.012, 0.088, 0, "none");
addEdge(0, 0.05, 0.009, 0.05, 0, "none");
addEdge(0.009, 0.05, 0.009, -0.02, 0, "none");
addEdge(0, 0.05, 0, -0.02, 0, "A = 0");
addEdge(0, -0.02, 0.009, -0.02, 0, "none");
addEdge(0, 0.15, 0, 0.1, 0, "A = 0");
addEdge(0, 0.08, 0, 0.05, 0, "A = 0");
addEdge(0, -0.02, 0, -0.05, 0, "A = 0");
addEdge(0, -0.05, 0.1, 0.05, 90, "A = 0");
addEdge(0.1, 0.05, 0, 0.15, 90, "A = 0");

// labels
addLabel(0.0348743, 0.0347237, 0, "Fe");
addLabel(0.00512569, -0.0070852, 0, "Fe");
addLabel(0.021206, 0.0692964, 0, "Cu");
addLabel(0.0141705, 0.12445, 0, "Air");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(0.035324, 0.043477);
testFluxDensity = abs(abs(point.B) - 0.032477) < 1e-4;
if (!testFluxDensity) print("Flux density: ", abs(point.B), " == ", 0.032477);

// energy
integral = volumeIntegral();
testEnergy = (abs(integral.Wm) - 0.036347) < 1e-5;
if (!testEnergy) print("Magnetic energy: ", abs(integral.Wm), " == ", 0.036347);

print(testFluxDensity && testEnergy);