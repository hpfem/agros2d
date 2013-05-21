Acoustics
=========
Acoustics is the interdisciplinary science that deals with the study of all mechanical waves in gases, liquids and solids.

Harmonic stationary acoustic field
----------------------------------

Equation
^^^^^^^^
The stationary harmonic acoustic field is described by Helmholtz differential equation

.. math::

   -\, \div \left( \frac{1}{\rho}\,\, \grad \phas{p} \right) - \frac{\omega^2}{\rho  c^2} \cdot \phas{p} = 0,

where $\phas{p}$ is a phasor of the acoustic preassure, $\rho$ is the density and $\omega$ is angular frequency.
Constant $c$ is speed of sound.

Boundary conditions
^^^^^^^^^^^^^^^^^^^
There are implemented three different kinds of boundary conditions

* Acoustic preassure, the Dirichlet boundary condition in the form

.. math::
    \phas{p}(\Gamma) = \phas{p}_0
    
* Normal acceleration, the Neumann boundary condition in the form

.. math::
      a_\mathrm{n} = \frac{1}{\rho} \frac{\partial \phas{p}}{\partial n_0} = \phas{a_0}

* Impedance boundary condition, the Newton boundary condition in the form

.. math::
    \frac{1}{\rho} \frac{\partial \phas{p}}{\partial n_0} = - \mj \omega \frac{\phas{p}}{Z_0}
