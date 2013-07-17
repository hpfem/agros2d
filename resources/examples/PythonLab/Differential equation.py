import pythonlab
from numpy import arange
from scipy.integrate import odeint
import pylab as pl

R = 1e3
L = 1e-3
U0 = 10

# initial condition
y0 = [0]

def func(y, t):
	return [- R/L*y[0] + U0/L]

t = arange(0, 5*L/R, L/R/10)
y = odeint(func, y0, t)

# chart
pl.close()
pl.plot(t, y)
pl.grid(1)
pl.xlabel("t (s)")
pl.ylabel("i (A)")
fn_chart = pythonlab.tempname("png")
pl.savefig(fn_chart, dpi=60)
pl.close()

# show in console
pythonlab.image(fn_chart)