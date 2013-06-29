.. _acoustic:

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
    \phas{p} = \phas{p}_0.
    
* Normal acceleration, the Neumann boundary condition in the form

.. math::
      a_\mathrm{n} = \frac{1}{\rho} \frac{\partial \phas{p}}{\partial n_0} = \phas{a_0}.

* Impedance boundary condition, the Newton boundary condition in the form

.. math::
    \frac{1}{\rho} \frac{\partial \phas{p}}{\partial n_0} = - \mj \omega \frac{\phas{p}}{Z_0}.
 

Time dependent acoustic field
-----------------------------

Equation
^^^^^^^^
.. math::
    -\, \div \left( \frac{1}{\rho}\,\, \grad p \right) + \frac{1}{\rho  c^2} \frac{\partial^2 p}{\partial t^2} = 0

Boundary conditions
^^^^^^^^^^^^^^^^^^^
* Acoustic preassure, the Dirichlet boundary condition in the form

.. math::
    p = p_0.

* Normal acceleration, the Neumann boundary condition in the form

.. math::
      a_\mathrm{n} = \frac{1}{\rho} \frac{\partial p}{\partial n_0} = a_0.

* Impedance boundary condition, the Newton boundary condition in the form

.. math::
    \frac{1}{\rho} \frac{\partial p}{\partial n_0} = - \frac{1}{Z_0} \frac{\partial p}{\partial t}.
  
Postprocessor
-------------

Local values
------------

+-----------+------------------------------------------+----------------------+-----------------+
| Variable  | Units                                    | Meaning              | Agros variables |
+===========+==========================================+======================+=================+
| :math:`p` | Pa                                       | Acoustic pressure    | p, pi, pr       |
+-----------+------------------------------------------+----------------------+-----------------+
| `SPL`     | dB                                       | Sound pressure level | SPL             |
+-----------+------------------------------------------+----------------------+-----------------+
| `a`       | :math:`\mathrm{m} \cdot \mathrm{s}^{-2}` | Local acceleration   | ar, ai          |
+-----------+------------------------------------------+----------------------+-----------------+
| `c`       |                                          |                      |                 |
+-----------+------------------------------------------+----------------------+-----------------+
