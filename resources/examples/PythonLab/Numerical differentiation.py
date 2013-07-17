import numpy as np
import pylab as pl
from math import pi
import pythonlab

def difference(f):
    def df(x, h):
        return (f(x+h)-f(x))/h

    return df

def f(x):
    return np.sin(x)

dy = difference(f)
x = np.linspace(0, 5*pi, 1e2)
y = np.array([])

for i in x:
    y = np.append(y, dy(i, 1e-2))

pl.figure()
pl.plot(x, f(x), label="$y=f(x)$") 
pl.plot(x, y, label="$y^{'}=f(x)$") 
pl.legend(loc='lower right')
chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)