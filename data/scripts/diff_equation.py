from numpy import arange
from scipy.integrate import odeint
import pylab

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
pylab.close()
pylab.plot(t, y)
pylab.grid(1)
pylab.xlabel("$t\,\mathrm{(s)}$")
pylab.ylabel("$i\,\mathrm{(A)}$")
# pylab.legend(("current"), "upper left")
pylab.show()