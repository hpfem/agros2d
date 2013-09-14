import pylab as pl
import numpy as np
import pythonlab

data = np.loadtxt(pythonlab.datadir('resources/examples/PythonLab/Polynomial regression.csv'), delimiter=",")
x = data[:, 0]
y = data[:, 1]

yp2 = pl.polyfit(x, y, 2)
yp3 = pl.polyfit(x, y, 4)

xi = np.linspace(x[0], x[-1], 1e2)
yi2 = pl.polyval(yp2, xi)
yi4 = pl.polyval(yp3, xi)

pl.figure()
pl.plot(x, y, '.k', label="original data")
pl.plot(xi, yi2, label="2th order polynomial")
pl.plot(xi, yi4, label="4th order polynomial")
pl.legend(loc='lower right')
chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)