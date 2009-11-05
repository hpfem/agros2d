print("Test: Harmonic magnetic - axisymmetric");

// model
newDocument("Heating", "axisymmetric", "harmonicmagnetic", 1, 3, "disabled", 0, 0, 100);

// boundaries
addBoundary("A = 0", "harmonicmagnetic_vector_potential", 0);

// materials
addMaterial("Coil", 1e6, 0, 1, 0);
addMaterial("Iron", 0, 0, 50, 5e6);
addMaterial("Air", 0, 0, 1, 0);

// edges
addEdge(0, -0.17, 0.15, -0.17, 0, "A = 0");
addEdge(0.15, -0.17, 0.15, 0.17, 0, "A = 0");
addEdge(0.15, 0.17, 0, 0.17, 0, "A = 0");
addEdge(0.035, -0.03, 0.055, -0.03, 0, "none");
addEdge(0.055, -0.03, 0.055, 0.11, 0, "none");
addEdge(0.055, 0.11, 0.035, 0.11, 0, "none");
addEdge(0.035, 0.11, 0.035, -0.03, 0, "none");
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
pointPotential = pointResult(0.031809, 0.045406);
testPotentialReal = abs(abs(pointPotential.A_real) - 7.719762e-4) < 1e-6;
testPotentialImag = abs(abs(pointPotential.A_imag) - 1.729612e-4) < 1e-6;
if (!testPotentialReal) print("Magnetic potential - real: ", abs(pointPotential.A_real), " == ", 7.719762e-4);
if (!testPotentialImag) print("Magnetic potential - imag: ", abs(pointPotential.A_imag), " == ", 1.729612e-4);

pointTotalCurrentDensity = pointResult(0.02814, 0.047561);
testTotalCurentDensityReal = abs(abs(pointTotalCurrentDensity.J_real) - 1.024812e6) < 1e4;
testTotalCurentDensityImag = abs(abs(pointTotalCurrentDensity.J_imag) - 1.215725e6) < 1e4;
if (!testTotalCurentDensityReal) print("Total current density - real: ", abs(pointTotalCurrentDensity.J_real), " == ", 1.024812e6);
if (!testTotalCurentDensityImag) print("Total current density - imag: ", abs(pointTotalCurrentDensity.J_imag), " == ", 1.215725e6);

pointFluxDensity = pointResult(0.028979, 0.022765);
testFluxDensity = abs(abs(pointFluxDensity.B) - 0.378015) < 1e-2;
if (!testFluxDensity) print("Flux density: ", abs(pointFluxDensity.B), " == ", 0.378015);

// energy
integral = volumeIntegral();
testEnergy = abs(abs(integral.Wm) - 0.119061) < 1e-4;
if (!testEnergy) print("Magnetic energy: ", abs(integral.Wm), " == ", 0.119061);

// induced current
integral = volumeIntegral(2);
testInducedCurrentReal = abs(abs(integral.Ii_real) - 637.318482) < 1;
testInducedCurrentImag = abs(abs(integral.Ii_imag) - 334.407653) < 1;
if (!testInducedCurrentReal) print("Induced current - real: ", abs(integral.Ii_real), " == ", 637.318482);
if (!testInducedCurrentImag) print("Induced current - imag: ", abs(integral.Ii_imag), " == ", 334.407653);

// Lorentz force
integral = volumeIntegral(2);
testLorentzForceY = abs(abs(integral.Fy_real/2.0) - 0.33924) < 1e-3;
if (!testLorentzForceY) print("Lorentz force - Y: ", abs(integral.Fy_real/2.0), " == ", 0.33924);

// Joule losses
integral = volumeIntegral(2);
testJouleLosses = abs(abs(integral.Pj) - 26.78469) < 1e-1;
if (!testJouleLosses) print("Joule losses: ", abs(integral.Pj), " == ", 26.78469);


print(testPotentialReal && testPotentialImag && testTotalCurentDensityReal && testTotalCurentDensityImag && 
testFluxDensity && testEnergy && testInducedCurrentReal && testInducedCurrentImag &&
testLorentzForceY && testJouleLosses);
