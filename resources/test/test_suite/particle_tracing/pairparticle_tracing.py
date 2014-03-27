#!/usr/bin/python

from visual import *
from math import pi, sqrt
import json

import pylab as pl
import matplotlib.cm as cm

pl.rcParams['figure.figsize'] = 10, 10
pl.rcParams['font.size'] = 11
pl.rcParams['legend.fontsize'] = 9
pl.rcParams['text.usetex'] = True
pl.rcParams['text.latex.preamble'] = "\usepackage{mathptmx}"
pl.rcParams['axes.grid'] = False

def save_data(data, variant):
  file = open('{0}.dat'.format(variant), 'w')
  json.dump(data, file)
  file.close()

def load_data(variant):
  file = open('{0}.dat'.format(variant), 'r')
  data = json.load(file)
  file.close()

  return data

def compare_data(reference_data, data, coordinate = 'x'):
  assert(len(reference_data) == len(data))

  difference = 0
  for i in range(len(data)):
    for j in range(len(data[i][coordinate])):
      pass

def plot_data(data, variant):
  pl.figure()

  pair = True
  color = None
  for i in range(len(data)):
    if pair:
      color = cm.jet(i/float(len(data)), 1)
      pl.plot(data[i]['x'], data[i]['y'], '-', color=color)
      pair = False
    else:
      pl.plot(data[i]['x'], data[i]['y'], '--', color=color)
      pair = True

  pl.xlabel("$x\,\mathrm{(m)}$")
  pl.ylabel("$y\,\mathrm{(m)}$")
  pl.savefig('{0}-trajectory.pdf'.format(variant), bbox_inches='tight')

class MultiParticleTest(object):
  def __init__(self, variant = 'test', visualization = False,
                     gravity_force = True, electrostatic_force = True,
                     particle2particle_electric_force = True):

    scene.visible = visualization

    """ test properties """
    self.variant = variant
    self.x0 = 0.001
    self.y0 = 0.75
    self.v0 = 0.25

    self.dx = 0.003
    self.dy = 0.025
    
    self.time_step = 5e-6
    self.rate = 1e6

    self.time = [0]
    
    self.gravity_force = gravity_force
    self.electrostatic_force = electrostatic_force
    self.particle2particle_electric_force = particle2particle_electric_force

    """ particles """
    self.particles = []
    self.particles.append(sphere(radius = 0.02, pos=(self.x0, self.y0, 0), color= (0.75, 0, 0),
                            track = curve(radius = 0.01, color=(0.75, 0, 0))))

    self.particles[0].charge = -5e-10
    self.particles[0].vel = vector(0, -self.v0, 0)

    self.particles.append(sphere(radius = 0.02, pos=(self.x0-self.dx, self.y0-self.dy, 0), color= (0, 0, 0.75),
                            track = curve(radius = 0.01, color=(0, 0, 0.75))))

    self.particles[1].charge = 1e-10
    self.particles[1].vel = vector(0, 0, 0)
    
    for particle in self.particles:
      particle.run = True
      particle.particle2particle_electric_force = []
      particle.mass = 3.5e-5

  def force(self, particle1, particle2):
    Fg = vector(0, 0, 0)
    Fe = vector(0, 0, 0)

    """ electrostatic force """
    if self.electrostatic_force:
      pass
    
    distance = sqrt((particle1.pos.x - particle2.pos.x)**2 +
                    (particle1.pos.y - particle2.pos.y)**2 +
                    (particle1.pos.z - particle2.pos.z)**2)
    
    """ particle to particle electric force """
    if (self.particle2particle_electric_force and distance > 0):
      F = (particle1.charge * particle2.charge) / (4.0 * pi * 8.854e-12 * distance**3)
      Fe.x = F * (particle1.pos.x - particle2.pos.x)
      Fe.y = F * (particle1.pos.y - particle2.pos.y)
      Fe.z = F * (particle1.pos.z - particle2.pos.z)

    particle1.particle2particle_electric_force.append(sqrt(Fe.x**2 + Fe.y**2 + Fe.z**2))

    """ gravity force """
    if self.gravity_force:
      Fg.y = - particle1.mass * 9.823

    F = vector(Fe.x,
               Fg.y + Fe.y,
               Fe.z)
    return F

  def solve(self):
    run = True
    while(run):
      rate(self.rate)
      self.time.append(self.time[-1] + self.time_step)

      for particle in self.particles:
        if not particle.run:
          continue

        particle.pos = particle.pos + particle.vel * self.time_step
        particle.vel = particle.vel + self.force(particle, self.particles[self.particles.index(particle) - 1]) / particle.mass * self.time_step
        particle.track.append(pos = particle.pos)

        if (particle.pos.y < 0):
          particle.run = False
        
        if ((not self.particles[self.particles.index(particle) - 1].run) and
            (not particle.run)):
          run = False

    self.save()

  def save(self):
    data = []
    N = 50
    for particle in self.particles:
      data.append({'x' : list(particle.track.x)[::N], 'y' : list(particle.track.y)[::N], 'z' : list(particle.track.z)[::N]})
    save_data(data, self.variant)

def free_fall():
  variant = 'free_fall'
  test = MultiParticleTest(variant = variant,
                           particle2particle_electric_force = True)
  test.solve()

if __name__ == '__main__':
  free_fall()
