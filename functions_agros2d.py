# import libraries
import pythonlab
import agros2d
import sys
from math import *

def add_semicircle(x0, y0, radius, boundaries = {}, materials = None):
    geometry = agros2d.geometry
    geometry.add_edge(x0, (y0)-radius, (x0)+radius, y0, boundaries=boundaries, angle=90)
    geometry.add_edge((x0)+radius, y0, x0, (y0)+radius, boundaries=boundaries, angle=90)
    geometry.add_edge(x0, (y0)+radius, x0, (y0)-radius, boundaries=boundaries, angle=0)
    if (materials != None):
        geometry.add_label((x0)+(radius/2.0), y0, materials=materials)
    return
setattr(agros2d, "add_semicircle", add_semicircle)

def add_circle(x0, y0, radius, boundaries = {}, materials = None):
    geometry = agros2d.geometry
    geometry.add_edge(x0, (y0)-radius, (x0)+radius, y0, boundaries=boundaries, angle=90)
    geometry.add_edge((x0)+radius, y0, x0, (y0)+radius, boundaries=boundaries, angle=90)
    geometry.add_edge(x0, (y0)+radius, (x0)-radius, y0, boundaries=boundaries, angle=90)
    geometry.add_edge((x0)-radius, y0, x0, (y0)-radius, boundaries=boundaries, angle=90)
    if (materials != None):
        geometry.add_label(x0, y0, materials=materials)
    return
setattr(agros2d, "add_circle", add_circle)

def add_rect(x0, y0, width, height, boundaries = {}, materials = None):
    geometry = agros2d.geometry
    geometry.add_edge(x0, y0, (x0)+width, y0, boundaries=boundaries)
    geometry.add_edge(x0+width, y0, (x0)+width, (y0)+height, boundaries=boundaries)
    geometry.add_edge(x0+width, (y0)+height, x0, (y0)+height, boundaries=boundaries)
    geometry.add_edge(x0, (y0)+height, x0, y0, boundaries=boundaries)
    if (materials != None):
        geometry.add_label((x0)+(width/2.0), (y0)+(height/2.0), materials=materials)
    return
setattr(agros2d, "add_rect", add_rect)

def test(text, value, normal, error = 0.03):
    if ((normal == 0.0) and abs(value < 1e-14)):
        return True
    test = abs((value - normal)/value) < error
    if (not test):    
        print(text + ": Agros2D: " + str(value) + ", correct: " + str(normal) + ")")
    return test
setattr(agros2d, "test", test)
