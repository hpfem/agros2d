// dimensions and materials
r1 = 0.01;
r2 = 0.03;
r3 = 0.05;
r4 = 0.06;
l =  0.16;
eps1 = 10;
eps2 = 3;
U0 = 1;

// model
newDocument("Electrostatic", "axisymmetric", "electrostatic", 1, 2, 0, 0);

// boundaries
addBoundary("U = 1 V", "potential", U0);
addBoundary("U = 0 V", "potential", 0);
addBoundary("neumann", "surface_charge_density", 0);

// materials
addMaterial("air", 0, 1);
addMaterial("diel 1", 0, eps1);
addMaterial("diel 2", 0, eps2);

// edges
addEdge(0, 3/2*l, 0, l/2, 0, "neumann");
addEdge(r1, l/2, r1, 0, 0, "U = 1 V");
addEdge(r1, 0, r2, 0, 0, "neumann");
addEdge(r2, 0, r2, l/2, 0, "none");
addEdge(r2, l/2, r3, l/2, 0, "none");
addEdge(r3, 0, r2, 0, 0, "neumann");
addEdge(r3, l/2, r3, 0, 0, "U = 0 V");
addEdge(r4, 0, r4, l/2, 0, "U = 0 V");
addEdge(r3, l/2, r4, l/2, 0, "U = 0 V");
addEdge(r4, 0, 3/2*l, 0, 0, "neumann");
addEdge(3/2*l, 0, 0, 3/2*l, 90, "neumann");
addEdge(r1, l/2, r2, l/2, 0, "none");
addEdge(r1, l/2, 0, l/2, 0, "U = 1 V");

// labels
addLabel(0.019, 0.021, 0, "diel 1");
addLabel(0.0379, 0.051, 0, "diel 2");
addLabel(0.0284191, 0.123601, 0, "air");

zoomBestFit();

// capacitance
selectNone();
print("Capacitance:");
dr = 0.01;
i = 0;
do
{
	// move edges
	if (i > 0)
	{			
		selectEdge(6, 7, 8);
		moveSelection(dr, 0);	
	}

	solve();

	integral = volumeIntegral();
	// Energy We = 1/2 C*U^2
	print((2*2*integral.Energy)/(U0^2));

	i++;
} while (i<5);