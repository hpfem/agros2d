.. _heat:

Heat Transfer
=============
Heat transfer is a discipline of thermal engineering that concerns the exchange of thermal energy from one physical system to another. Heat transfer is classified into various mechanisms, such as heat conduction, convection and thermal radiation.

Equation
^^^^^^^^
Heat transfer can be described by partial differential equation

.. math::

   \div \lambda\, \grad T - \rho c_\mathrm{p} \frac{\partial T}{\partial t} = -Q,

where $\lambda$ is thermal conductivity, $T$ is temperature, $\rho$ is density, $c_\mathrm{p}$ is specific heat and finally $Q$ is source of the inner heat (eddy current, chemical source, ...). Term with partial derivative is in steady-state analysis neglected. Thermal flux can be written as

.. math::

   \vec{F} = \lambda\, \grad T

and temperature gradient is

.. math::

   \vec{G} = \grad T.

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Temperature $T = f$ on the boundary is known.

* Neumann BC

  Thermal heat flux $q = - \lambda \frac{\partial T}{\partial n}$ on the boundary is known.

* Mixed BC

  Thermal heat flux due to convection into the environment $q = - \lambda \frac{\partial T}{\partial n} = \alpha \left( T - T_{\mathrm{ext}}\right)$ on the boundary is known.

Boundary integrals
^^^^^^^^^^^^^^^^^^

Average temperature: 

.. math::

   T_\mathrm{avg} = \frac{1}{S} \int_S T \dif S\,\,\,\mathrm{(deg.)}

Heat flux:

.. math::

   F = \int_S \lambda \frac{\partial T}{\partial n} \dif S\,\,\,\mathrm{(W)}

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

Average temperature:

.. math::

   T_\mathrm{avg} = \frac{1}{V} \int_V T \dif V\,\,\,\mathrm{(deg.)}
   
.. include:: heat.gen