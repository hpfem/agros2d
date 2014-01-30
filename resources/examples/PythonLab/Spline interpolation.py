import pylab as pl
import numpy as np
import pythonlab
from scipy import interpolate

data = np.loadtxt(pythonlab.datadir('resources/examples/PythonLab/Polynomial regression.csv'), delimiter=",")
x = data[:, 0]
y = data[:, 1]

xi = np.linspace(x[0], x[-1], 1e2)

f1 = interpolate.interp1d(x, y, kind = "linear")
y1 = f1(xi)

f2 = interpolate.interp1d(x, y, kind = "quadratic")
y2 = f2(xi)

f3 = interpolate.interp1d(x, y, kind = "cubic")
y3 = f3(xi)

f4 = interpolate.interp1d(x, y, kind = "nearest")
y4 = f4(xi)

pl.figure()
pl.subplot(2,2,1)
pl.plot(x, y, '.k', label="original data")
pl.plot(xi, y1, label="linear spline interpolation")
pl.title(r"linear")

pl.subplot(2,2,2)
pl.plot(x, y, '.k', label="original data")
pl.plot(xi, y2, label="quadratic spline interpolation")
pl.title(r"quadratic")

pl.subplot(2,2,3)
pl.plot(x, y, '.k', label="original data")
pl.plot(xi, y3, label="cubic spline interpolation")
pl.title(r"cubic")

pl.subplot(2,2,4)
pl.plot(x, y, '.k', label="original data")
pl.plot(xi, y4, label="nearest interpolation")
pl.title(r"nearest")

chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)