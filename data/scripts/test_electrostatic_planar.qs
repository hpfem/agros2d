print("Test: Electrostatic - planar");

// model
newDocument("Elec2 (QuickField)", "planar", "electrostatic", 2, 3, 0, "disabled", 1, 5);

// boundaries
addBoundary("Neumann", "electrostatic_surface_charge_density", 0);
addBoundary("U =  0 V", "electrostatic_potential", 0);
addBoundary("U =  1000 V", "electrostatic_potential", 1000);

// materials
addMaterial("Diel", 0, 3);
addMaterial("Air", 0, 1);
addMaterial("Source", 4e-10, 10);

// edges
addEdge(1, 2, 1, 1, 0, "U =  1000 V");
addEdge(4, 1, 1, 1, 0, "U =  1000 V");
addEdge(1, 2, 4, 2, 0, "U =  1000 V");
addEdge(4, 2, 4, 1, 0, "U =  1000 V");
addEdge(20, 24, 20, 1, 0, "Neumann");
addEdge(20, 1, 20, 0, 0, "Neumann");
addEdge(4, 1, 20, 1, 0, "none");
addEdge(0, 24, 0, 1, 0, "Neumann");
addEdge(0, 0, 0, 1, 0, "Neumann");
addEdge(0, 0, 20, 0, 0, "U =  0 V");
addEdge(0, 24, 20, 24, 0, "Neumann");
addEdge(0, 1, 1, 1, 0, "none");
addEdge(7, 13, 14, 13, 0, "none");
addEdge(14, 13, 14, 18, 0, "none");
addEdge(14, 18, 7, 18, 0, "none");
addEdge(7, 18, 7, 13, 0, "none");

// labels
addLabel(2.78257, 1.37346, 0, "none");
addLabel(10.3839, 15.7187, 0, "Source");
addLabel(3.37832, 15.8626, 0, "Air");
addLabel(12.3992, 0.556005, 0, "Diel");

// solve
zoomBestFit();
solve();

// point value
point = pointResult(13.257584, 11.117738);
testPotential = (Math.abs(point.V) - 1111.544813) < 0.5;
if (!testPotential) print("Scalar potential: ", abs(point.V), " == ", 1111.544813);

// energy
integral = volumeIntegral();
testEnergy = (Math.abs(integral.We) - 6.79882e-5) < 1e-7;
if (!testEnergy) print("Electric energy: ", abs(integral.We), " == ", 6.79882e-5);

// charge
integral = surfaceIntegral(0, 1, 2, 3);
testQ = (Math.abs(integral.Q) - 1.048981e-7) < 5e-9;
if (!testQ) print("Electric charge: ", abs(integral.Q), " == ", 1.048981e-7);

print(testPotential && testEnergy && testQ);
