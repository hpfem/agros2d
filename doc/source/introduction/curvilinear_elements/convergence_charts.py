#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import pylab as pl
from matplotlib.ticker import ScalarFormatter

# edge elements
error_data = np.loadtxt("error-edge_elements.dat")
steps_e = error_data[:, 0]
error_e = error_data[:, 1]
dofs_data = np.loadtxt("dofs-edge_elements.dat")
dofs_e = dofs_data[:, 1]

# curvilinear elements
error_data = np.loadtxt("error-curvilinear_elements.dat")
steps_c = error_data[:, 0]
error_c = error_data[:, 1]
dofs_data = np.loadtxt("dofs-curvilinear_elements.dat")
dofs_c = dofs_data[:, 1]

pl.close()
pl.rcParams['figure.figsize'] = 10, 5

pl.figure()
pl.plot(dofs_e, error_e, '.-', label="$\mathrm{edge~elements}$")
pl.plot(dofs_c, error_c, '.-', label="$\mathrm{curvilinear~elements}$")
pl.yscale('log')
pl.xlabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.ylabel("$\mathrm{DOFs\,(-)}$")
pl.legend(loc='upper right')
pl.grid()
pl.savefig('convergence.png', bbox_inches='tight')

pl.figure()
pl.plot(steps_e, error_e, '.-', label="$\mathrm{edge~elements}$")
pl.plot(steps_c, error_c, '.-', label="$\mathrm{curvilinear~elements}$")
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.legend(loc='lower left')
pl.grid()
pl.savefig('error.png', bbox_inches='tight')

pl.figure()
pl.plot(steps_e, dofs_e, '.-', label="$\mathrm{edge~elements}$")
pl.plot(steps_c, dofs_c, '.-', label="$\mathrm{curvilinear~elements}$")
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\mathrm{DOFs\,(-)}$")
pl.legend(loc='lower right')
pl.grid()
pl.savefig('dofs.png', bbox_inches='tight')
