newdocument("Electrostatic Axisymmetric Capacitor", "axisymmetric", "electrostatic", 0, 3, "disabled", 1, 1, 0, "steadystate", 0, 0, 0)
newdocument("Heat Transfer Axisymmetric Actuator", "axisymmetric", "heat", 0, 1, "hp-adaptivity", 5, 15, 0, "transient", 500, 15e3, 20)

result = pointresult(0.1, 0.1)
print("Potential = " + str(result["V"]))

result = volumeintegral(0.1, 0.1)
print("Volume = " + str(result["V"]))

result = surfaceintegral(0.1, 0.1)
print("Charge = " + str(result["V"]))
