# model
newdocument("Magnetic Steadystate Axisymmetric Actuator", "axisymmetric", "magnetic", 0, 4, "disabled", 1, 1, 0, "steadystate", 1, 1, 0)

# boundaries
addboundary("Border", "magnetic_vector_potential", 0, 0)

# materials
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Iron", 0, 0, 300, 0, 0, 0, 0, 0, 0)
addmaterial("Copper", 1e6, 0, 1, 0, 0, 0, 0, 0, 0)

# edges
addedge(0.01, 0.01, 0.01, 0, 0, "none")
addedge(0.01, 0, 0.04, 0, 0, "none")
addedge(0.04, 0, 0.04, 0.1, 0, "none")
addedge(0.04, 0.1, 0, 0.1, 0, "none")
addedge(0, 0.1, 0, 0.08, 0, "Border")
addedge(0, 0.08, 0.01, 0.08, 0, "none")
addedge(0.01, 0.09, 0.03, 0.09, 0, "none")
addedge(0.01, 0.09, 0.01, 0.08, 0, "none")
addedge(0.03, 0.09, 0.03, 0.01, 0, "none")
addedge(0.01, 0.01, 0.03, 0.01, 0, "none")
addedge(0.012, 0.088, 0.012, 0.012, 0, "none")
addedge(0.012, 0.012, 0.028, 0.012, 0, "none")
addedge(0.028, 0.012, 0.028, 0.088, 0, "none")
addedge(0.028, 0.088, 0.012, 0.088, 0, "none")
addedge(0, 0.05, 0.009, 0.05, 0, "none")
addedge(0.009, 0.05, 0.009, -0.02, 0, "none")
addedge(0, 0.05, 0, -0.02, 0, "Border")
addedge(0, -0.02, 0.009, -0.02, 0, "none")
addedge(0, 0.15, 0, 0.1, 0, "Border")
addedge(0, 0.08, 0, 0.05, 0, "Border")
addedge(0, -0.02, 0, -0.05, 0, "Border")
addedge(0, -0.05, 0.1, 0.05, 90, "Border")
addedge(0.1, 0.05, 0, 0.15, 90, "Border")

# labels
addlabel(0.0348743, 0.0347237, 0, 0, "Iron")
addlabel(0.00512569, 0.040852, 0, 0, "Iron")
addlabel(0.021206, 0.0692964, 0, 0, "Copper")
addlabel(0.0141705, 0.12445, 0, 0, "Air")

zoombestfit()
selectnone()

# calculation of force
zmax = (0.077-0.05)
N = 10
dz = zmax/(N-1)
z = []
Fz = []
print("F = f(z) (N):")
for i in range(N):
	if i > 0:
		selectedge(14, 15, 16, 17)
		moveselection(0, dz, False)

	solve()
	result = surfaceintegral(14, 15, 16, 17)

	z.append(0 + (i*dz))
	Fz.append(result["Fy"])
	print(z[-1], Fz[-1])

import pylab as pl
import numpy as np
from scipy import interpolate
from scipy.integrate import odeint

# static characteristic
# interpolate 
fint = interpolate.interp1d(z, Fz, 1)

# plot
zint = np.linspace(0, zmax)
Fzint = fint(zint)

pl.subplot(3,1,1)
pl.plot(z, Fz, 'o', zint, Fzint, '-')
pl.grid(1)
pl.xlabel("$z\,\mathrm{(s)}$")
pl.ylabel("$F_\mathrm{L}\,\mathrm{(m)}$")

# dynamic characteristic
# initial condition
y0 = [0, 0]

def func(y, t):
	return [y[1], fint(y[0])/(1.781e-5*7800)]

# differential equation
t = np.linspace(0, 0.08)
y = odeint(func, y0, t)

# plot
pl.subplot(3,1,2)
pl.plot(t, y[:,0])
pl.grid(1)
pl.xlabel("$t\,\mathrm{(s)}$")
pl.ylabel("$z\,\mathrm{(m)}$")
pl.xlim([0, 0.07])

pl.subplot(3,1,3)
pl.plot(t, y[:,1])
pl.grid(1)
pl.xlabel("$t\,\mathrm{(s)}$")
pl.ylabel("$v\,\mathrm{(m/s)}$")
pl.xlim([0, 0.07])
fn_chart = pythonlab.tempname("png")
pl.savefig(fn_chart, dpi=60)
pl.close()

# show in console
pythonlab.image(fn_chart)