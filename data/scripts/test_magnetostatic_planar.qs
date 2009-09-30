print("Test: Magnetostatic - planar");

// model
newDocument("Unnamed", "planar", "magnetostatic", 1, 4, 0, "disabled", 1, 2);

// boundaries
addBoundary("A = 0", "vector_potential", 0);

// materials
addMaterial("Cu", 1000000, 1);
addMaterial("Fe", 0, 1000);
addMaterial("Air", 0, 1);

// edges
addEdge(-0.5, 1, 0.5, 1, 0, "A = 0");
addEdge(0.5, 1, 0.5, -1, 0, "A = 0");
addEdge(0.5, -1, -0.5, -1, 0, "A = 0");
addEdge(-0.5, -1, -0.5, 1, 0, "A = 0");
addEdge(-0.2, 0.6, 0, 0.6, 0, "none");
addEdge(0, 0.3, 0, 0.6, 0, "none");
addEdge(-0.2, 0.3, 0, 0.3, 0, "none");
addEdge(-0.2, 0.6, -0.2, 0.3, 0, "none");
addEdge(-0.035, 0.135, 0.035, 0.135, 0, "none");
addEdge(0.035, 0.135, 0.035, -0.04, 0, "none");
addEdge(0.035, -0.04, -0.035, -0.04, 0, "none");
addEdge(-0.035, 0.135, -0.035, -0.04, 0, "none");

// labels
addLabel(-0.0959509, 0.445344, 0.01, "Fe");
addLabel(0.00301448, 0.0404858, 0.005, "Cu");
addLabel(-0.145434, -0.706253, 0.01, "Air");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(0.012862, 0.202579);
testFluxDensity = (Math.abs(point.B) - 0.015323) < 2e-4;
if (!testFluxDensity) print(point.B);

// energy
integral = volumeIntegral();
testEnergy = (Math.abs(integral.Wm) - 38.073345) < 1e-2;
if (!testEnergy) print(integral.Wm);

print(testFluxDensity && testEnergy);