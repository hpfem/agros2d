// model
newDocument("Electrostatic 2D", "axisymmetric", "electrostatic", 1, 3, 20, 2)
saveDocument("data/pokus.h2d");

// boundaries
addBoundary("U = 10 V", "1", 10)
addBoundary("U =  0 V", "1", 0)
addBoundary("neumann", "2", 0)

// materials
addMaterial("air", 0, 1)
addMaterial("diel 1", 0, 10)
addMaterial("diel 2", 0, 3)

// edges
addEdge(0, 0.2, 0, 0.08, 0, "neumann")
addEdge(0.01, 0.08, 0.01, 0, 0, "U = 10 V")
addEdge(0.01, 0, 0.03, 0, 0, "neumann")
addEdge(0.03, 0, 0.03, 0.08, 0, "none")
addEdge(0.03, 0.08, 0.05, 0.08, 0, "none")
addEdge(0.05, 0, 0.03, 0, 0, "neumann")
addEdge(0.05, 0.08, 0.05, 0, 0, "U =  0 V")
addEdge(0.06, 0, 0.06, 0.08, 0, "U =  0 V")
addEdge(0.05, 0.08, 0.06, 0.08, 0, "U =  0 V")
addEdge(0.06, 0, 0.2, 0, 0, "neumann")
addEdge(0.2, 0, 0, 0.2, 90, "neumann")
addEdge(0.01, 0.08, 0.03, 0.08, 0, "none")
addEdge(0.01, 0.08, 0, 0.08, 0, "U = 10 V")

// labels
addLabel(0.019, 0.021, 2e-05, "diel 1")
addLabel(0.0379, 0.051, 2e-05, "diel 2")
addLabel(0.0284191, 0.123601, 0.001, "air")
