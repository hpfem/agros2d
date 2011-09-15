# model
newdocument(name="Acoustics - planar", type="axisymmetric",
            physicfield="acoustic", analysistype="harmonic",
            numberofrefinements=3, polynomialorder=2,
            nonlineartolerance=0.001, nonlinearsteps=10,
            frequency=700)

# boundaries
addboundary("Wall", "acoustic_normal_acceleration", {"anr" : 0, "ani" : 0})
addboundary("Source acceleration", "acoustic_normal_acceleration", {"anr" : 10, "ani" : 0})
addboundary("Matched boundary", "acoustic_impedance", {"Z0" : 1.25*343})
addboundary("Source pressure", "acoustic_pressure", {"pr" : 0.2, "pi" : 0})

# materials
addmaterial("Air", {"rho" : 1.25, "vel" : 343})

# edges
addedge(0, 1.5, 1.05, 1.25, boundary="Wall")
addedge(1.05, 1.25, 0.25, 0, boundary="Matched boundary")
addedge(0.25, 0, 0, 0, boundary="Source acceleration")
addedge(0, 0, 0, 0.7, boundary="Wall")
addedge(0, 1, 0, 1.5, boundary="Wall")
addedge(0, 0.7, 0.15, 0.85, boundary="Wall", angle=90)
addedge(0.15, 0.85, 0, 1, boundary="Wall", angle=90)
addedge(0.35, 1.15, 0.65, 1, boundary="Matched boundary")
addedge(0.65, 1, 0.35, 0.9, boundary="Matched boundary")
addedge(0.35, 1.15, 0.35, 0.9, boundary="Matched boundary")
addedge(0.6, 1.2, 0.6, 1.25, boundary="Source pressure")
addedge(0.6, 1.2, 0.65, 1.2, boundary="Source pressure")
addedge(0.6, 1.25, 0.65, 1.2, boundary="Source pressure")

# labels
addlabel(0.163662, 0.383133, material="Air")
addlabel(0.426096, 1.03031, material="none")
addlabel(0.616273, 1.21617, material="none")

# solve
zoombestfit()
solve()

# point value
point = pointresult(0.259371, 0.876998)
testp = test("Acoustic pressure", point["p"], 0.49271)
testp_real = test("Acoustic pressure - real", point["pr"], 0.395866)
testp_imag = test("Acoustic pressure - imag", point["pi"], 0.293348)
# testSPL = test("Acoustic sound level", point["SPL"], 84.820922)

# volume integral
volume = volumeintegral([0])
#testPv_real = test("Pressure - real", volume["p_real"], -0.030632)
#testPv_imag = test("Pressure - imag", volume["p_imag"], -0.010975)

# surface integral
surface = surfaceintegral([0])
#testPs_real = test("Pressure - real", surface["p_real"], 0.196756)
#testPs_imag = test("Pressure - imag", surface["p_imag"], -0.324708)

# print("Test: Acoustic - planar: " + str(testp and testp_real and testp_imag and testSPL and testPv_real and testPv_imag and testPs_real and testPs_imag))
print("Test: Acoustics - planar: " + str(testp and testp_real and testp_imag))