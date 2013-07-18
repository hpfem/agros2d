.. _current:

Current field
=============
Electric current is a flow of electric charge through a medium. This charge is typically carried by moving electrons in a conductor such as wire. It can also be carried by ions in an electrolyte, or by both ions and electrons in a plasma.

Current field can be described by Laplace partial differential equation

.. math::

   \div \sigma\, \grad \varphi = 0,

where $\sigma$ is electric conductivity of the material and $\varphi$ is electrical scalar potential. Electric field can be written as

.. math::

   \vec{E} = - \grad \varphi

and electric current density is

.. math::

   \vec{J} = \sigma \vec{E}.

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Scalar potential $\varphi = f$ on the boundary is known.

* Neumann BC

  Current density $J_\mathrm{n} = \sigma \frac{\partial \varphi}{\partial n} = g$ on the boundary is known.

Boundary integrals
^^^^^^^^^^^^^^^^^^

Current:

.. math::

   I = \int_S J_\mathrm{n} \dif S = \int_S \sigma \frac{\partial \varphi}{\partial n} \dif S\,\,\,\mathrm{(A)}
   
   
.. include:: current.gen