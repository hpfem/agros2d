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
testPotentialReal = (Math.abs(pointPotential.A_real) - 1.632192e-5) < 1e-8;
testPotentialImag = (Math.abs(pointPotential.A_imag) - 4.083868e-5) < 1e-8;
if (!testPotentialReal) print(pointPotential.A_real);
if (!testPotentialImag) print(pointPotential.A_imag);

pointTotalCurrentDensity = pointResult(0.013567, 0.017081);
testTotalCurentDensityReal = (Math.abs(pointTotalCurrentDensity.J_real) - 2.408328e5) < 1e2;
testTotalCurentDensityImag = (Math.abs(pointTotalCurrentDensity.J_imag) - 3.299946e5) < 1e2;
if (!testTotalCurentDensityReal) print(pointTotalCurrentDensity.J_real);
if (!testTotalCurentDensityImag) print(pointTotalCurrentDensity.J_imag);

pointFluxDensity = pointResult(0.009454, -0.004136);
testFluxDensity = (Math.abs(pointFluxDensity.B) - 0.00142) < 1e-8;
if (!testFluxDensity) print(pointFluxDensity.B);

// energy
integral = volumeIntegral();
testEnergy = (Math.abs(integral.Wm) - 0.001923) < 1e-5;
if (!testEnergy) print(integral.Wm);

// Lorentz force
integral = volumeIntegral(1);
testLorentzForceX = (Math.abs(integral.Fx_real/2.0) - 0.020612) < 1e-4;
testLorentzForceY = (Math.abs(integral.Fy_real/2.0) - 0.011107) < 1e-4;
if (!testLorentzForceX) print(integral.Fx_real/2.0);
if (!testLorentzForceY) print(integral.Fy_real/2.0);

print(pointPotential && pointTotalCurrentDensity && pointFluxDensity && testEnergy 
&& testLorentzForceX && testLorentzForceY);