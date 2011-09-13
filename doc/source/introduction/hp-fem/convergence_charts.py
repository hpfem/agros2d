#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import pylab as pl
from matplotlib.ticker import ScalarFormatter

# h-adaptivity (p = 1)
error_data = np.loadtxt("error-h(p=1).dat")
steps_h1 = error_data[:, 0]
error_h1 = error_data[:, 1]
dofs_data = np.loadtxt("dofs-h(p=1).dat")
dofs_h1 = dofs_data[:, 1]

# h-adaptivity (p = 2)
error_data = np.loadtxt("error-h(p=2).dat")
steps_h2 = error_data[:, 0]
error_h2 = error_data[:, 1]
dofs_data = np.loadtxt("dofs-h(p=2).dat")
dofs_h2 = dofs_data[:, 1]

# p-adaptivity
error_data = np.loadtxt("error-p.dat")
steps_p = error_data[:, 0]
error_p = error_data[:, 1]
dofs_data = np.loadtxt("dofs-p.dat")
dofs_p = dofs_data[:, 1]

# hp-adaptivity
error_data = np.loadtxt("error-hp.dat")
steps_hp = error_data[:, 0]
error_hp = error_data[:, 1]
dofs_data = np.loadtxt("dofs-hp.dat")
dofs_hp = dofs_data[:, 1]

pl.close()
pl.rcParams['figure.figsize'] = 10, 5

pl.figure()
pl.plot(dofs_h1, error_h1, '.-', label="$h-\mathrm{adaptivity\,(p = 1)}$")
pl.plot(dofs_h2, error_h2, '.-', label="$h-\mathrm{adaptivity\,(p = 2)}$")
pl.plot(dofs_p, error_p, '.-', label="$p-\mathrm{adaptivity}$")
pl.plot(dofs_hp, error_hp, '.-', label="$hp-\mathrm{adaptivity}$")
pl.axhline(y=1, linestyle='--', color='k', label="$\\varepsilon=1\%$")
pl.yscale('log')
pl.xlabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.ylabel("$\mathrm{DOFs\,(-)}$")
pl.legend(loc='upper right')
pl.grid()
pl.savefig('convergence.png', bbox_inches='tight')

pl.figure()
pl.plot(steps_h1, error_h1, '.-', label="$h-\mathrm{adaptivity\,(p = 1)}$")
pl.plot(steps_h2, error_h2, '.-', label="$h-\mathrm{adaptivity\,(p = 2)}$")
pl.plot(steps_p, error_p, '.-', label="$p-\mathrm{adaptivity}$")
pl.plot(steps_hp, error_hp, '.-', label="$hp-\mathrm{adaptivity}$")
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.legend(loc='lower left')
pl.grid()
pl.savefig('error.png', bbox_inches='tight')

pl.figure()
pl.plot(steps_h1, dofs_h1, '.-', label="$h-\mathrm{adaptivity\,(p = 1)}$")
pl.plot(steps_h2, dofs_h2, '.-', label="$h-\mathrm{adaptivity\,(p = 2)}$")
pl.plot(steps_p, dofs_p, '.-', label="$p-\mathrm{adaptivity}$")
pl.plot(steps_hp, dofs_hp, '.-', label="$hp-\mathrm{adaptivity}$")
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\mathrm{DOFs\,(-)}$")
pl.legend(loc='lower right')
pl.grid()
pl.savefig('dofs.png', bbox_inches='tight')
