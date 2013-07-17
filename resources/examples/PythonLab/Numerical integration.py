import numpy as np
from math import pi

from scipy import integrate

def f(x):
    return np.sin(x)

a = 0.0
b = pi
r = 1e3

x = np.array(np.linspace(a, pi, 1e5))
I = 0.0

for i in np.linspace(a, b, r):
    I = I+(b-a)/r*f(i)

print(I, integrate.quad(f, a, b))