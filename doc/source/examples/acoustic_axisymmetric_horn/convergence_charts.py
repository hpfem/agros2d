#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import pylab as pl
from matplotlib.ticker import ScalarFormatter

data = np.loadtxt("convergence_chart.dat")
DOFs = data[:, 0]
err = data[:, 1]

pl.rcParams['figure.figsize'] = 10, 5
pl.figure()
pl.plot(DOFs, err, '.-')
pl.axhline(y=10.0, linestyle='--', color='k', label="$\zeta=10\%$")
pl.xscale('log')
pl.yscale('log')
pl.xlabel("$\mathrm{DOFs\,(-)}$")
pl.ylabel("$\\zeta\,\mathrm{(\%)}$")
pl.legend(loc='upper right')
pl.grid()
pl.savefig('convergence_chart.png', bbox_inches='tight')
