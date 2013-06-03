.. _rf:

RF waves
========
Solving TE waves in RF field.

Equation
^^^^^^^^
Harmonic steady state of TE waves is described by Helmholtz equation 

.. math::
   \curl \left( \frac{1}{\mu}\, \curl \vecphas{E} \right) - \mj \omega \left( \sigma + \mj \omega \varepsilon \right) \vecphas{E} = \mj \omega \vecphas{J}_{\mathrm{ext}}.
   
Boundary conditions
^^^^^^^^^^^^^^^^^^^
* *Electric field* is the Dirichlet boundary condition in the form 

.. math::
   \vecphas{E} = \vecphas{E}_0
   
* *Magnetic field* is the Neumann boundary condition in the form 

.. math::
   \vec{n} \times H = n \times H_0
   
* *Surface current* is the Neumann boundary condition in the form

.. math::
    \phas{J}_{t} = - \frac{1}{\omega \mu} \frac{\partial \vecphas{E}}{\partial n_0} = \phas{J}_0

* *Impedance* is the Newton boundary condition in the form 

.. math::
    - \frac{1}{\omega \mu} \frac{\partial \vecphas{E}}{\partial n_0} = \sqrt{\frac{\varepsilon - \mj \sigma / \omega}{\mu}} \vecphas{E}          
 