print("Test: Magnetostatic - planar");

#model
newDocument("Unnamed", "planar", "magnetostatic", 1, 4);

#boundaries
addBoundary("A = 0", "magnetostatic_vector_potential", 0);

#materials
addMaterial("Cu", 1000000, 1, 0, 0);
addMaterial("Fe", 0, 1000, 0, 0);
addMaterial("Air", 0, 1, 0, 0);
addMaterial("Magnet", 0, 1.11, 0.1, -30);

#edges
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
addEdge(0, -0.5, 0.3, -0.5, 0, "none");
addEdge(0.3, -0.5, 0.3, -0.32, 0, "none");
addEdge(0.3, -0.32, 0, -0.32, 0, "none");
addEdge(0, -0.32, 0, -0.5, 0, "none");

#labels
addLabel(-0.0959509, 0.445344, 0.01, "Fe");
addLabel(0.00301448, 0.0404858, 0.005, "Cu");
addLabel(-0.145434, -0.706253, 0.01, "Air");
addLabel(0.143596, -0.364811, 0, "Magnet");

#solve
zoomBestFit();
solve();

#point value
point = pointResult(-0.011911, 0.170158);
testFluxDensity = abs(abs(point.B) - 0.022126) < 1e-4;
if (!testFluxDensity) print("Flux density: ", abs(point.B), " == ", 0.022126);

#energy
integral = volumeIntegral();
testEnergy = (abs(integral.Wm) - 144.196623) < 1;
if (!testEnergy) print("Magnetic energy: ", abs(integral.Wm), " == ", 144.196623);

print(testFluxDensity && testEnergy);
