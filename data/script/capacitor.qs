// model
newDocument("Levitation", "axisymmetric", "harmonic magnetic", 0, 1, 0, 1);

R1 = 0.0025;
R2 = 0.005;
Jext = 1e6;

// boundaries
addBoundary("A = 0", "vector_potential", 0);

// materials
addMaterial("air", 0, 0, 1, 0);
addMaterial("aluminium", 0, 0, 1, 33e6);
addMaterial("coil+",  Jext, 0, 1, 0);
addMaterial("coil-", -Jext, 0, 1, 0);

// edges
RS =  0.0125;
ZS = -0.0050;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil+");

RS = 0.0225;
ZS = 0.0025;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil+");

RS = 0.03;
ZS = 0.0125;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil+");

RS = 0.0325;
ZS = 0.025;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil+");

RS = 0.0325;
ZS = 0.0375;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil+");

RS = 0.0325;
ZS = 0.05;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil+");

RS = 0.0325;
ZS = 0.0625;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil+");

RS = 0.025;
ZS = 0.0825;
addEdge(RS+R1, ZS,    RS,    ZS+R1, 90, "none");
addEdge(RS,    ZS+R1, RS-R1, ZS,    90, "none");
addEdge(RS-R1, ZS,    RS,    ZS-R1, 90, "none");
addEdge(RS,    ZS-R1, RS+R1, ZS,    90, "none");
addEdge(RS+R2, ZS,    RS,    ZS+R2, 90, "none");
addEdge(RS,    ZS+R2, RS-R2, ZS,    90, "none");
addEdge(RS-R2, ZS,    RS,    ZS-R2, 90, "none");
addEdge(RS,    ZS-R2, RS+R2, ZS,    90, "none");
addLabel(RS, ZS, 0, "air");
addLabel(RS+(R1+R2)/2.0, ZS, 0, "coil-");

Z = 0.06;
R = 0.011;
addEdge(0, Z-R, R, Z, 90, "none");
addEdge(R, Z, 0, Z+R, 90, "none");
addEdge(0, Z-R, 0, Z+R, 0, "A = 0");
addLabel(R/2.0, Z, 0, "aluminium");

addEdge(0, Z-R, 0, -0.1, 0, "A = 0");
addEdge(0, -0.1, 0.145, 0.05, 90, "A = 0");
addEdge(0.145, 0.05, 0, 0.2, 90, "A = 0");
addEdge(0, 0.2, 0, Z+R, 0, "A = 0");
addLabel(0.1, 0.05, 0, "air");

zoomBestFit();