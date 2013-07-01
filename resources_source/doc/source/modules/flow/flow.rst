.. _flow:

Incompressible flow
===================
Incompressible flow model using Navier-Stokes equations.

Staedy-state analysis
---------------------

Equation
^^^^^^^^
Incompressible flow is described by Navier-Stokes equations

.. math::
   \rho \vec{v} \cdot \grad \vec{v} = -\, \grad p + \mu \triangle \vec{v} + \vec{f},~\div \vec{v} = 0,
   
where $\vec{v}$ is the velocity, $\mu$ is the flow viscosity, $\rho$ is the flow density, $\vec{f}$ is the force.

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Velocity is the Dirichlet boundary condition in the form 

.. math::
   v_x = v_{0,x},\,\,\, v_y = v_{0,y}.

*  Outlet is the Neumann boundary condition in the form

.. math::
   \frac{\partial v_x}{\partial n} = 0,\,\,\, \frac{\partial v_y}{\partial n} = 0. 

*  Pressure is the Dirichlet boundary condition in the form

.. math::
   p = p_0.
    
* Axial symmetry consist of two the boundary conditions in the form

.. math::
    v_r = 0,\,\,\, \frac{\partial v_z}{\partial n} = 0.
    
.. include:: flow_gen.rst