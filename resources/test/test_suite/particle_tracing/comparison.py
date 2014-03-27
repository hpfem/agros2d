#!/usr/bin/python

from pairparticle_tracing import load_data, plot_data
import pylab as pl

if __name__ == '__main__':
  data = load_data('free_fall')
  data_a2d = load_data('free_fall-a2d')
  
  pl.figure()
  """ pair-particle tracing """
  pl.plot(data[0]['x'], data[0]['y'], '-r')
  pl.plot(data[1]['x'], data[1]['y'], '-g')

  """ multi-particle tracing """
  pl.plot(data_a2d[0]['x'], data_a2d[0]['y'], '.r')
  pl.plot(data_a2d[1]['x'], data_a2d[1]['y'], '.g')

  pl.xlabel("$x\,\mathrm{(m)}$")
  pl.ylabel("$y\,\mathrm{(m)}$")
  pl.savefig('comparison-trajectory.pdf', bbox_inches='tight')
