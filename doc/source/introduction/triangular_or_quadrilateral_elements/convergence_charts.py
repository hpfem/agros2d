#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import pylab as pl
from matplotlib.ticker import ScalarFormatter

# quads
error_data = np.loadtxt("error-quads.dat")
steps_q = error_data[:, 0]
error_q = error_data[:, 1]
dofs_data = np.loadtxt("dofs-quads.dat")
dofs_q = dofs_data[:, 1]

# triangles
error_data = np.loadtxt("error-triangles.dat")
steps_t = error_data[:, 0]
error_t = error_data[:, 1]
dofs_data = np.loadtxt("dofs-triangles.dat")
dofs_t = dofs_data[:, 1]

pl.close()
pl.rcParams['figure.figsize'] = 10, 5

pl.figure()
pl.plot(dofs_q, error_q, '.-', label="$\mathrm{quadrilateral~elements}$")
pl.plot(dofs_t, error_t, '.-', label="$\mathrm{triangular~elements}$")
pl.axhline(y=10, linestyle='--', color='k', label="$\\varepsilon=10\%$")
pl.yscale('log')
pl.xlabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.ylabel("$\mathrm{DOFs\,(-)}$")
pl.legend(loc='lower right')
pl.grid()
pl.savefig('convergence.png', bbox_inches='tight')

pl.figure()
pl.plot(steps_q, error_q, '.-', label="$\mathrm{quadrilateral~elements}$")
pl.plot(steps_t, error_t, '.-', label="$\mathrm{triangular~elements}$")
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.legend(loc='lower left')
pl.grid()
pl.savefig('error.png', bbox_inches='tight')

pl.figure()
pl.plot(steps_q, dofs_q, '.-', label="$\mathrm{quadrilateral~elements}$")
pl.plot(steps_t, dofs_t, '.-', label="$\mathrm{triangular~elements}$")
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\mathrm{DOFs\,(-)}$")
pl.legend(loc='lower right')
pl.grid()
pl.savefig('dofs.png', bbox_inches='tight')
