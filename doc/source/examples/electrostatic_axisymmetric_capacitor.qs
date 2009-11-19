// model
newDocument("Electrostatic axisymmetric capacitor", "axisymmetric", "electrostatic", 1, 2);

// variables
r1 = 0.01;
r2 = 0.03;
r3 = 0.05;
r4 = 0.06;
l = 0.16;
eps1 = 10;
eps2 = 3;
U = 10;
dr = 0.01;

// boundaries
addBoundary("Source electrode", "electrostatic_potential", U);
addBoundary("Ground electrode", "electrostatic_potential", 0);
addBoundary("Neumann BC", "electrostatic_surface_charge_density", 0);

// materials
addMaterial("Air", 0, 1);
addMaterial("Dielectric n.1", 0, eps1);
addMaterial("Dielectric n.2", 0, eps2);

// edges
addEdge(0, 3/2*l, 0, l/2, 0, "Neumann BC");
addEdge(r1, l/2, r1, 0, 0, "Source electrode");
addEdge(r1, 0, r2, 0, 0, "Neumann BC");
addEdge(r2, 0, r2, l/2, 0, "none");
addEdge(r2, l/2, r3, l/2, 0, "none");
addEdge(r3, 0, r2, 0, 0, "Neumann BC");
addEdge(r3, l/2, r3, 0, 0, "Ground electrode");
addEdge(r4, 0, r4, l/2, 0, "Ground electrode");
addEdge(r3, l/2, r4, l/2, 0, "Ground electrode");
addEdge(r4, 0, 3/2*l, 0, 0, "Neumann BC");
addEdge(3/2*l, 0, 0, 3/2*l, 90, "Neumann BC");
addEdge(r1, l/2, r2, l/2, 0, "none");
addEdge(r1, l/2, 0, l/2, 0, "Source electrode");

// labels
addLabel(0.019, 0.021, 0, "Dielectric n.1");
addLabel(0.0379, 0.051, 0, "Dielectric n.2");
addLabel(0.0284191, 0.123601, 0, "Air");

zoomBestFit();
selectNone();

// calculation of capacity
print("C = f(r) (F):");
i = 0;
do
{
	if (i > 0)
	{
		selectEdge(6, 7, 8);
		moveSelection(dr, 0);
	}
	solve();
	integral = volumeIntegral();

	print((2*2*integral.We)/(U^2));
	i++;
} while (i<5);
