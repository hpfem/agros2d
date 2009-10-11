print("Test: Harmonic magnetic - planar");

// model
newDocument("Test", "planar", "harmonic magnetic", 1, 2, 50, "disabled", 0, 1);

// boundaries
addBoundary("A = 0", "vector_potential", 0);

// materials
addMaterial("Air", 0, 0, 1, 0);
addMaterial("Cond 1", 1e6, 0, 1, 57e6);
addMaterial("Cond 2", 1e6, 0, 1, 57e6);

// edges
addEdge(-0.075, 0.06, 0.075, 0.06, 0, "A = 0");
addEdge(0.075, 0.06, 0.075, -0.06, 0, "A = 0");
addEdge(0.075, -0.06, -0.075, -0.06, 0, "A = 0");
addEdge(-0.075, -0.06, -0.075, 0.06, 0, "A = 0");
addEdge(-0.015, -0.01, -0.015, 0.01, 0, "none");
addEdge(-0.015, 0.01, -0.005, 0.01, 0, "none");
addEdge(-0.015, -0.01, -0.005, -0.01, 0, "none");
addEdge(-0.005, -0.01, -0.005, 0.01, 0, "none");
addEdge(0.005, 0.02, 0.005, 0, 0, "none");
addEdge(0.005, 0, 0.015, 0, 0, "none");
addEdge(0.015, 0, 0.015, 0.02, 0, "none");
addEdge(0.015, 0.02, 0.005, 0.02, 0, "none");

// labels
addLabel(0.035349, 0.036683, 0, "Air");
addLabel(0.00778124, 0.00444642, 1e-05, "Cond 1");
addLabel(-0.0111161, -0.00311249, 1e-05, "Cond 2");

// solve
zoomBestFit();
solve();

// point value
pointPotential = pointResult(0.00589,-0.005301);
testPotentialReal = abs(abs(pointPotential.A_real) - 1.632192e-5) < 1e-7;
testPotentialImag = abs(abs(pointPotential.A_imag) - 4.083868e-5) < 1e-7;
if (!testPotentialReal) print("Magnetic potential - real: ", abs(pointPotential.A_real), " == ", 1.632192e-5);
if (!testPotentialImag) print("Magnetic potential - imag: ", abs(pointPotential.A_imag), " == ", 4.083868e-5);

pointTotalCurrentDensity = pointResult(0.013567, 0.017081);
testTotalCurentDensityReal = abs(abs(pointTotalCurrentDensity.J_real) - 2.408328e5) < 1e2;
testTotalCurentDensityImag = abs(abs(pointTotalCurrentDensity.J_imag) - 3.299946e5) < 1e2;
if (!testTotalCurentDensityReal) print("Total current density - real: ", abs(pointTotalCurrentDensity.J_real), " == ", 2.408328e5);
if (!testTotalCurentDensityImag) print("Total current density - imag: ", abs(pointTotalCurrentDensity.J_imag), " == ", 3.299946e5);

pointFluxDensity = pointResult(0.009454, -0.004136);
testFluxDensity = abs(abs(pointFluxDensity.B) - 0.00142) < 1e-4;
if (!testFluxDensity) print("Flux density: ", abs(pointFluxDensity.B), " == ", 0.00142);

// energy
integral = volumeIntegral();
testEnergy = abs(abs(integral.Wm) - 0.001923) < 1e-5;
if (!testEnergy) print("Magnetic energy: ", abs(integral.Wm), " == ", 0.001923);

// Lorentz force
integral = volumeIntegral(1);
testLorentzForceX = abs(abs(integral.Fx_real/2.0) - 0.020612) < 1e-4;
testLorentzForceY = abs(abs(integral.Fy_real/2.0) - 0.011107) < 1e-4;
if (!testLorentzForceX) print("Lorentz force - X: ", abs(integral.Fx_real/2.0), " == ", 0.020612);
if (!testLorentzForceY) print("Lorentz force - Y: ", abs(integral.Fy_real/2.0), " == ", 0.011107);

// Joule losses
integral = volumeIntegral(1);
testJouleLosses = abs(abs(integral.Pj) - 26.78469) < 1e-1;
if (!testJouleLosses) print("Joule losses: ", abs(integral.Pj), " == ", 26.78469);

print(testPotentialReal && testPotentialImag && testTotalCurentDensityReal && testTotalCurentDensityImag && 
testFluxDensity && testEnergy && testLorentzForceX && testLorentzForceY && testJouleLosses);