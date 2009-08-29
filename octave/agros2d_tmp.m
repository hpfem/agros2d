% model
agros2d('newDocument(\"Electrostatic\", \"axisymmetric\", \"electrostatic\", 1, 2, 0, \"disabled\", 1, 0);');

% boundaries
agros2d('addBoundary(\"Source electrode\", \"potential\", 10);');
agros2d('addBoundary(\"Ground electrode\", \"potential\", 0);');
agros2d('addBoundary(\"Neumann BC\", \"surface_charge_density\", 0);');

% materials
agros2d('addMaterial(\"Air\", 0, 1);');
agros2d('addMaterial(\"Dielectric n.1\", 0, 10);');
agros2d('addMaterial(\"Dielectric n.2\", 0, 3);');

% edges
agros2d('addEdge(0, 0.2, 0, 0.08, 0, \"Neumann BC\");');
agros2d('addEdge(0.01, 0.08, 0.01, 0, 0, \"Source electrode\");');
agros2d('addEdge(0.01, 0, 0.03, 0, 0, \"Neumann BC\");');
agros2d('addEdge(0.03, 0, 0.03, 0.08, 0, \"none\");');
agros2d('addEdge(0.03, 0.08, 0.05, 0.08, 0, \"none\");');
agros2d('addEdge(0.05, 0, 0.03, 0, 0, \"Neumann BC\");');
agros2d('addEdge(0.05, 0.08, 0.05, 0, 0, \"Ground electrode\");');
agros2d('addEdge(0.06, 0, 0.06, 0.08, 0, \"Ground electrode\");');
agros2d('addEdge(0.05, 0.08, 0.06, 0.08, 0, \"Ground electrode\");');
agros2d('addEdge(0.06, 0, 0.2, 0, 0, \"Neumann BC\");');
agros2d('addEdge(0.2, 0, 0, 0.2, 90, \"Neumann BC\");');
agros2d('addEdge(0.01, 0.08, 0.03, 0.08, 0, \"none\");');
agros2d('addEdge(0.01, 0.08, 0, 0.08, 0, \"Source electrode\");');

% labels
agros2d('addLabel(0.019, 0.021, 0, \"Dielectric n.1\");');
agros2d('addLabel(0.0379, 0.051, 0, \"Dielectric n.2\");');
agros2d('addLabel(0.0284191, 0.123601, 0, \"Air\");');

% zoom
agros2d('zoomBestFit();');
agros2d('solve();');

agros2d('mode(\"postprocessor\");');
