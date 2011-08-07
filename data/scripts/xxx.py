# model
newdocument(name="Electrostatic Axisymmetric Capacitor", type="axisymmetric",
            physicfield="electrostatic", analysistype="steadystate",
            numberofrefinements=0, polynomialorder=3)

# startup script
U = 10
eps1 = 10
eps2 = 3

# boundaries
addboundary("Source", "electrostatic_potential", {"V" : U})
addboundary("Ground", "electrostatic_potential", {"V" : 0})
addboundary("Neumann", "electrostatic_surface_charge_density", {"sigma" : 0})

# materials
addmaterial("Air", {"epsr" : 1, "rho" : 0})
addmaterial("Dielectric 1", {"epsr" : eps1, "rho" : 0})
addmaterial("Dielectric 2", {"epsr" : eps2, "rho" : 0})

# edges
addedge(0, 0.2, 0, 0.08, boundary="Neumann")
addedge(0.01, 0.08, 0.01, 0, boundary="Source", refine=1)
addedge(0.01, 0, 0.03, 0, boundary="Neumann")
addedge(0.03, 0, 0.03, 0.08)
addedge(0.03, 0.08, 0.05, 0.08)
addedge(0.05, 0, 0.03, 0, boundary="Neumann")
addedge(0.05, 0.08, 0.05, 0, boundary="Ground")
addedge(0.06, 0, 0.06, 0.08, boundary="Ground")
addedge(0.05, 0.08, 0.06, 0.08, boundary="Ground")
addedge(0.06, 0, 0.2, 0, boundary="Neumann")
addedge(0.2, 0, 0, 0.2, boundary="Neumann", angle=90)
addedge(0.01, 0.08, 0.03, 0.08)
addedge(0.01, 0.08, 0, 0.08, boundary="Source")

# labels
addlabel(0.019, 0.021, material="Dielectric 1")
addlabel(0.0379, 0.051, material="Dielectric 2", area=0.0001, order=5)
addlabel(0.0284191, 0.123601, material="Air", order=2)
