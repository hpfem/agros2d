newdocument("Electrostatic axisymmetric capacitor", "axisymmetric", "electrostatic", 1, 2, "hp-adaptivity", 10, 1)
newdocument("Heat transfer actuator", "axisymmetric", "heat", 0, 1, "hp-adaptivity", 4, 1, 0, "transient", 500, 10000, 20)

result = pointresult(0.1, 0.1)
print "Potential = ", result.V

integral = volumeintegral(0.1, 0.1)
print "Volume = ", integral.V

integral = surfaceintegral(0.1, 0.1)
print "Charge = ", integral.Q
