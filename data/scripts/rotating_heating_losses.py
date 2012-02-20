# model
newdocument("Magnetic Steadystate Planar Rotating Heating", "planar", "magnetic", 1, 3, "disabled", 1, 1, 0, "steadystate", 1, 1, 0)

# variables
Jext = 3e7
sigma = 33e6
n = 1500
p = 1
f = p*n/60
omega = 2*pi*f

# boundaries
addboundary("Border", "magnetic_vector_potential", 0, 0)

# materials
addmaterial("Aluminum", 0, 0, 1, sigma, 0, 0, 0, 0, omega)
addmaterial("Air", 0, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Conductor 1", Jext, 0, 1, 0, 0, 0, 0, 0, 0)
addmaterial("Conductor 2", -Jext, 0, 1, 0, 0, 0, 0, 0, 0)

# edges
addedge(0.03, 0.0519615, 0.04, 0.069282, 0, "none")
addedge(0.04, -0.069282, 0.03, -0.0519615, 0, "none")
addedge(-0.03, -0.0519615, -0.04, -0.069282, 0, "none")
addedge(-0.03, 0.0519615, -0.04, 0.069282, 0, "none")
addedge(0.05, 0, 0, 0.05, 90, "none")
addedge(0, 0.05, -0.05, 0, 90, "none")
addedge(-0.05, 0, 0, -0.05, 90, "none")
addedge(0, -0.05, 0.05, 0, 90, "none")
addedge(0.25, 0, 0, 0.25, 90, "Border")
addedge(0, 0.25, -0.25, 0, 90, "Border")
addedge(-0.25, 0, 0, -0.25, 90, "Border")
addedge(0, -0.25, 0.25, 0, 90, "Border")
addedge(0.04, -0.069282, 0.08, 0, 60, "none")
addedge(0.08, 0, 0.04, 0.069282, 60, "none")
addedge(0.06, 0, 0.03, 0.0519615, 60, "none")
addedge(0.03, -0.0519615, 0.06, 0, 60, "none")
addedge(-0.03, 0.0519615, -0.06, 0, 60, "none")
addedge(-0.04, 0.069282, -0.08, 0, 60, "none")
addedge(-0.08, 0, -0.04, -0.069282, 60, "none")
addedge(-0.06, 0, -0.03, -0.0519615, 60, "none")

# labels
addlabel(0.0899971, 0.155764, 0, 0, "Air")
addlabel(-0.0184609, 0.0207686, 2e-05, 0, "Aluminum")
addlabel(0.0576904, 0.0403833, 0, 0, "Conductor 1")
addlabel(-0.0588442, 0.0380757, 0, 0, "Conductor 2")

zoombestfit()
solve()

# charts
import pylab as pl

def graphradius(angle):
	N = 30
	step = 0.05/N

	r = []
	Jvel = []
	for i in range(1, N+1):
		R = (i-1)*step
		point = pointresult(R*cos(angle/180.0*pi), R*sin(angle/180.0*pi))
		r.append(R)
		Jvel.append(point["Jiv_real"])
	return r, Jvel

def graphangle(radius):
	N = 15
	step = 360.0/N

	Jvel = 0
	for i in range(1, N+1):
		angle = (i-1)*step
		point = pointresult(radius*cos(angle/180.0*pi), radius*sin(angle/180.0*pi))
		Jvel = Jvel + abs(point["Jiv_real"])
 	return Jvel / N

# radius
r00, Jvel00 = graphradius(00)
r30, Jvel30 = graphradius(30)
r60, Jvel60 = graphradius(60)
r90, Jvel90 = graphradius(90)

pl.subplot(2,1,1)
# pl.plot(r00, Jvel00, "k-", r30, Jvel30, "k+-", r60, Jvel60, "k--", r90, Jvel90, "ko-")
pl.plot(r00, Jvel00, r30, Jvel30, r60, Jvel60, r90, Jvel90)
pl.grid(1)
pl.xlabel("$r\,\mathrm{(m)}$")
pl.ylabel("$J\,\mathrm{(A/m^2)}$")
pl.legend(("00 deg.", "30 deg.", "60 deg.", "90 deg."), "upper left")

# angle
N = 15;
r = []
Pj = []
for i in range(1, N):
	R = (i-1)*0.05/N;
	J = graphangle(R)
	r.append(R)
	Pj.append(J**2/sigma)

pl.subplot(2,1,2)
pl.plot(r, Pj)
pl.grid(1)
pl.xlabel("$r\,\mathrm{(m)}$")
pl.ylabel("$P_\mathrm{j}\,\mathrm{(W/m^3)}$")
pl.savefig("chart.png", dpi=60)
pl.close()

# show in console
pythonlab.image("chart.png")