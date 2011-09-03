#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import pylab as pl

convergence = np.loadtxt("convergence.dat")
DOFs = convergence[:, 0]
err = convergence[:, 1]

error = np.loadtxt("error.dat")
steps = error[:, 0]

pl.close()
pl.rcParams['figure.figsize'] = 10, 5

pl.figure()
pl.plot(steps, err, '.-')
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.legend(loc='upper right')
pl.grid()
pl.savefig('error.png', bbox_inches='tight')

pl.figure()
pl.plot(steps, DOFs, '.-')
pl.yscale('log')
pl.xlabel("$\mathrm{step\,(-)}$")
pl.ylabel("$\mathrm{DOFs\,(-)}$")
pl.legend(loc='upper right')
pl.grid()
pl.savefig('dofs.png', bbox_inches='tight')

pl.figure()
pl.plot(DOFs, err, '.-')
pl.yscale('log')
pl.xlabel("$\mathrm{DOFs\,(-)}$")
pl.ylabel("$\\varepsilon\,\mathrm{(\%)}$")
pl.legend(loc='upper right')
pl.grid()
pl.savefig('convergence.png', bbox_inches='tight')
