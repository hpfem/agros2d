print("Test: Harmonic magnetic - axisymmetric");

// model
newDocument("unnamed", "axisymmetric", "harmonic magnetic", 1, 3, 100, "disabled", 1, 1);

// boundaries
addBoundary("A = 0", "vector_potential", 0);

// materials
addMaterial("Coil", 1e6, 0, 1, 0);
addMaterial("Iron", 0, 0, 50, 5e6);
addMaterial("Air", 0, 0, 1, 0);

// edges
addEdge(0, -0.17, 0.15, -0.17, 0, "A = 0");
addEdge(0.15, -0.17, 0.15, 0.17, 0, "A = 0");
addEdge(0.15, 0.17, 0, 0.17, 0, "A = 0");
addEdge(0.035, -0.07, 0.055, -0.07, 0, "none");
addEdge(0.055, -0.07, 0.055, 0.07, 0, "none");
addEdge(0.055, 0.07, 0.035, 0.07, 0, "none");
addEdge(0.035, 0.07, 0.035, -0.07, 0, "none");
addEdge(0, -0.05, 0.03, -0.05, 0, "none");
addEdge(0.03, -0.05, 0.03, 0.05, 0, "none");
addEdge(0.03, 0.05, 0, 0.05, 0, "none");
addEdge(0, 0.05, 0, -0.05, 0, "A = 0");
addEdge(0, 0.17, 0, 0.05, 0, "A = 0");
addEdge(0, -0.05, 0, -0.17, 0, "A = 0");

// labels
addLabel(0.109339, 0.112786, 0, "Air");
addLabel(0.0442134, 0.0328588, 0, "Coil");
addLabel(0.0116506, -0.00740064, 0, "Iron");

// solve
zoomBestFit();
solve();

// point value
pointPotential = pointResult(0.028979,0.053972);
testPotentialReal = abs(abs(pointPotential.A_real) - 4.216581e-4) < 1e-6;
testPotentialImag = abs(abs(pointPotential.A_imag) - 9.244775e-5) < 1e-6;
if (!testPotentialReal) print("Magnetic potential - real: ", abs(pointPotential.A_real), " == ", 4.216581e-4);
if (!testPotentialImag) print("Magnetic potential - imag: ", abs(pointPotential.A_imag), " == ", 9.244775e-5);

pointTotalCurrentDensity = pointResult(0.025649, 0.045722);
testTotalCurentDensityReal = abs(abs(pointTotalCurrentDensity.J_real) - 8.355636e5) < 1e3;
testTotalCurentDensityImag = abs(abs(pointTotalCurrentDensity.J_imag) - 93489.754133) < 1e3;
if (!testTotalCurentDensityReal) print("Total current density - real: ", abs(pointTotalCurrentDensity.J_real), " == ", 8.355636e5);
if (!testTotalCurentDensityImag) print("Total current density - imag: ", abs(pointTotalCurrentDensity.J_imag), " == ", 93489.754133);

pointFluxDensity = pointResult(0.031735, 0.024517);
testFluxDensity = abs(abs(pointFluxDensity.B) - 0.014782) < 1e-5;
if (!testFluxDensity) print("Flux density: ", abs(pointFluxDensity.B), " == ", 0.014782);

// energy
integral = volumeIntegral();
testEnergy = abs(abs(integral.Wm) - 0.136139) < 1e-5;
if (!testEnergy) print("Magnetic energy: ", abs(integral.Wm), " == ", 0.136139);

// induced current
integral = volumeIntegral(2);
testInducedCurrentReal = abs(abs(integral.Ii_real) - 756.712695) < 1;
testInducedCurrentImag = abs(abs(integral.Ii_imag) - 399.612036) < 1;
if (!testInducedCurrentReal) print("Induced current - real: ", abs(integral.Ii_real), " == ", 756.712695);
if (!testInducedCurrentImag) print("Induced current - imag: ", abs(integral.Ii_imag), " == ", 399.612036);

print(testPotentialReal && testPotentialImag && testTotalCurentDensityReal && testTotalCurentDensityImag && 
testFluxDensity && testEnergy && testInducedCurrentReal && testInducedCurrentImag);