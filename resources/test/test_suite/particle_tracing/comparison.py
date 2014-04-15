#!/usr/bin/python

from .pairparticle_tracing import load_data, plot_data
import pylab as pl

pl.rcParams['figure.figsize'] = 6, 6
pl.rcParams['font.size'] = 14
pl.rcParams['legend.fontsize'] = 12
pl.rcParams['text.usetex'] = True
pl.rcParams['text.latex.preamble'] = "\usepackage{mathptmx}"
pl.rcParams['axes.grid'] = False

if __name__ == '__main__':
  data = load_data('convergence-euler-1e-06')
  data_a2d = load_data('convergence-fehlberg-1e-09')
  
  pl.figure()
  """ multi-particle tracing """
  pl.plot(data_a2d[0]['x'], data_a2d[0]['y'], '--r', label="$\mathrm{Runge-Kutta-Fehlberg~method}$")
  pl.plot(data_a2d[1]['x'], data_a2d[1]['y'], '--g')

  """ pair-particle tracing """
  pl.plot(data[0]['x'], data[0]['y'], '-r', label="$\mathrm{Euler~method}$")
  pl.plot(data[1]['x'], data[1]['y'], '-g')

  pl.xlabel("$x\,\mathrm{(m)}$")
  pl.ylabel("$y\,\mathrm{(m)}$")
  pl.legend(loc = 'upper right')
  pl.xlim([-0.015, 0.015])
  pl.ylim([0, 0.8])
  pl.savefig('comparison-trajectory.pdf', bbox_inches='tight')
