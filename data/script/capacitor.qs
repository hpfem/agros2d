// model
newDocument("Electrostatic", "axisymmetric", "electrostatic", 1, 2, 0, 0);

// boundaries
addBoundary("U = 1 V", "potential", 1);
addBoundary("U = 0 V", "potential", 0);
addBoundary("neumann", "surface_charge_density", 0);

// materials
addMaterial("air", 0, 1);
addMaterial("diel 1", 0, 10);
addMaterial("diel 2", 0, 3);

// edges
addEdge(0, 0.2, 0, 0.08, 0, "neumann");
addEdge(0.01, 0.08, 0.01, 0, 0, "U = 1 V");
addEdge(0.01, 0, 0.03, 0, 0, "neumann");
addEdge(0.03, 0, 0.03, 0.08, 0, "none");
addEdge(0.03, 0.08, 0.05, 0.08, 0, "none");
addEdge(0.05, 0, 0.03, 0, 0, "neumann");
addEdge(0.05, 0.08, 0.05, 0, 0, "U = 0 V");
addEdge(0.06, 0, 0.06, 0.08, 0, "U = 0 V");
addEdge(0.05, 0.08, 0.06, 0.08, 0, "U = 0 V");
addEdge(0.06, 0, 0.2, 0, 0, "neumann");
addEdge(0.2, 0, 0, 0.2, 90, "neumann");
addEdge(0.01, 0.08, 0.03, 0.08, 0, "none");
addEdge(0.01, 0.08, 0, 0.08, 0, "U = 1 V");

// labels
addLabel(0.019, 0.021, 0, "diel 1");
addLabel(0.0379, 0.051, 0, "diel 2");
addLabel(0.0284191, 0.123601, 0, "air");

zoomBestFit();

// capacitance
selectNone();
print("Capacitance:");
dx = 0.01;
i = 0;
do
{
	// move edges
	if (i > 0)
	{			
		selectEdge(6, 7, 8);
		moveSelection(dx, 0);	
	}

	solve();

	integral = volumeIntegral();
	// Energy We = 1/2 C*U^2
	print((2*2*integral.Energy)/(1^2));

	i++;
} while (i<5);