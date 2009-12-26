Physical Field Theory
=====================

In this chapter we will describe physical field equations used in Agros2D.

Electrostatic
-------------

Electrostatic field can be described by Poisson partial differential equation

.. math::

   \div \varepsilon\, \grad \varphi = \rho,

where $\varepsilon$ is permittivity of the material, $\varphi$ is electrical scalar potential and $\rho$ is subdomain charge density. Electric field can be written as 

.. math::

   \vec{E} = - \grad \varphi

and electric displacement is

.. math::

   \vec{D} = \varepsilon \vec{E}.

Maxwell stress tensor:

.. math::
   \vec{S}_\mathrm{M} = \vec{E} \otimes \vec{D} - \frac{1}{2} \vec{E} \vec{D} \cdot \delta 

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Scalar potential $\varphi = f$ on the boundary is known.

* Neumann BC

  Surface charge density $D_\mathrm{n} = g$ on the boundary is known.

Boundary integrals
^^^^^^^^^^^^^^^^^^

Charge: 

.. math::

   Q = \int_S D_\mathrm{n} \dif S = \int_S \varepsilon \frac{\partial \varphi}{\partial n} \dif S\,\,\,\mathrm{(C)}

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

Energy:

.. math::

   W_\mathrm{e} = \int_V \frac{1}{2} \vec{E} \vec{D} \dif V\,\,\,\mathrm{(J)}

Maxwell force:

.. math::
   \vec{F}_\mathrm{M} = \int_S \vec{S}_\mathrm{M} \dif S = \int_V \div \vec{S}_\mathrm{M} \dif V\,\,\,\mathrm{(N)}

Current Field
-------------

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

General Magnetic Field
----------------------

General magnetic field can be desribed by partial differential equation

.. math::

   \curl \frac{1}{\mu}\, \left( \curl \vec{A} - \vec{B}_\mathrm{r} \right) + \sigma \vec{v} \times \curl \vec{A} + \sigma \frac{\partial \vec{A}}{\partial t} = \vec{J}_\mathrm{ext},

where $\mu$ is permeability of the material, $\vec{A} = ( A_z\,\mathrm{or}\,A_{\varphi} )$ is component of the magnetic vector potential, $\vec{B}_\mathrm{r}$ is remanent flux density, $\vec{v}$ is velocity, $\sigma$ is electric conductivity and finally $J_\mathrm{ext} = ( \vec{J}_z\,\mathrm{or}\,J_{\varphi} )$ is component of source current density. Magnetic flux density is given by form

.. math::

   \vec{B} = \curl \vec{A},

magnetic field is

.. math::

   \vec{H} = \frac{\vec{B}}{\mu},

eddy current density is

.. math::
  
   J_\mathrm{trans} = \sigma \frac{\partial \vec{A}}{\partial t},

velocity current density is 

.. math::
  
   J_\mathrm{vel} = \sigma \vec{v} \times \vec{B} = \sigma \vec{v} \times \curl \vec{A},

and total current density is

.. math::

   J_\mathrm{tot} = J_\mathrm{ext} + J_\mathrm{trans} + J_\mathrm{vel}.

Maxwell stress tensor:

.. math::
   \mat{S}_\mathrm{M} = \vec{H} \otimes \vec{B} - \frac{1}{2} \vec{H} \vec{B} \cdot \mat{I}

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Component of the magnetic vector potential $A = f$ on the boundary is known.

* Neumann BC

  Normal derivative of magnetic vector potential $\frac{\partial A}{\partial n} = g$ on the boundary is known.

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

External current:

.. math::

   I_\mathrm{ext} = \int_V J_\mathrm{ext} \dif V\,\,\,\mathrm{(A)}

Eddy current:

.. math::
  
   I_\mathrm{trans} = \int_V J_\mathrm{trans} \dif V\,\,\,\mathrm{(A)}

Velocity current:

.. math::

   I_\mathrm{vel} = \int_V J_\mathrm{vel} \dif V\,\,\,\mathrm{(A)}

Total current:

.. math::

   I_\mathrm{tot} = I_\mathrm{ext} + I_\mathrm{trans} + I_\mathrm{vel}

Power losses:

.. math::

   P_\mathrm{j} = \int_V \frac{J_\mathrm{tot}^2}{\sigma} \dif V\,\,\,\mathrm{(W)}

Energy:

.. math::

   W_\mathrm{m} = \int_V \frac{1}{2} \vec{H} \vec{B} \dif V\,\,\,\mathrm{(J)}

Lorentz force:

.. math::

   \vec{F} = \int_V J_\mathrm{tot} \times \vec{B} \dif V = \int_V J_\mathrm{tot} \times \curl \vec{A} \dif V\,\,\,\mathrm{(N)}

Maxwell force:

.. math::
   \vec{F}_\mathrm{M} = \oint_S \mat{S}_\mathrm{M} \dif S \,\,\,\mathrm{(N)}

Torque (planar arrangement only):

.. math::

   T_\mathrm{z} = \int_V \vec{r} \times \vec{F} \dif V\,\,\,\mathrm{(Nm)}

Harmonic Magnetic Field
-----------------------

Harmonic magnetic field can be described by partial differential equation

.. math::

   \curl \frac{1}{\mu}\, \left( \curl \vecfaz{A} - \vec{B}_\mathrm{r} \right) + \sigma \vec{v} \times \curl \vecfaz{A} + \mj \omega \sigma \vecfaz{A} = \vecfaz{J}_\mathrm{ext},

where $\mu$ is permeability of the material, $\faz{A} = ( \faz{A}_z\,\mathrm{or}\,\faz{A}_{\varphi} )$ is component of the magnetic vector potential, $\omega = 2 \pi f$ is frequency, $\sigma$ is electric conductivity, $\vec{v}$ is velocity and finally $\faz{J}_\mathrm{ext} = ( \faz{J}_z\,\mathrm{or}\,\faz{J}_{\varphi} )$ is component of source current density. Magnetic flux density is given by form

.. math::

   \vecfaz{B} = \curl \vecfaz{A},

magnetic field is

.. math::

   \vecfaz{H} = \frac{\vecfaz{B}}{\mu},

eddy current density is

.. math::

   \vecfaz{J}_\mathrm{trans} = \mj \omega \sigma \vecfaz{A},

velocity current density is 

.. math::
  
   \vecfaz{J}_\mathrm{vel} = \sigma \vec{v} \times \vecfaz{B} = \sigma \vec{v} \times \curl \vecfaz{A},

and total current density is

.. math::

   \vecfaz{J}_\mathrm{tot} = \vecfaz{J}_\mathrm{ext} + \vecfaz{J}_\mathrm{trans} + \vecfaz{J}_\mathrm{vel}.

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Component of the magnetic vector potential $\faz{A} = \faz{f}$ on the boundary is known.

* Neumann BC

  Normal derivative of magnetic vector potential $\frac{\partial \faz{A}}{\partial n} = \faz{g}$ on the boundary is known.

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

External current:

.. math::

   \faz{I}_\mathrm{ext} = \int_S \vecfaz{J}_\mathrm{ext} \dif S\,\,\,\mathrm{(A)}

Eddy current:

.. math::

   \faz{I}_\mathrm{trans} = \int_S \vecfaz{J}_\mathrm{trans} \dif S\,\,\,\mathrm{(A)}

Velocity current:

.. math::

   \faz{I}_\mathrm{vel} = \int_S \vecfaz{J}_\mathrm{vel} \dif S\,\,\,\mathrm{(A)}

Total current: 

.. math::

   \faz{I}_\mathrm{tot} = \faz{I}_\mathrm{ext} + \faz{I}_\mathrm{trans} + \faz{I}_\mathrm{vel}

Power losses:

.. math::

   P = \int_V \frac{\left( \vecfaz{J}_\mathrm{tot} \cdot \vecfaz{J}_\mathrm{tot}^* \right)}{\sigma} \dif V\,\,\,\mathrm{(W)}

Lorentz force:

.. math::

   F_\mathrm{L} = \int_V \vecfaz{J}_\mathrm{tot} \times \vecfaz{B} \dif V\,\,\,\mathrm{(N)}

Average energy:

.. math::

   W_\mathrm{m} = \int_V \frac{1}{2} \vecfaz{H} \vecfaz{B} \dif V\,\,\,\mathrm{(N)}

Heat Transfer
-------------

Heat transfer can be described by partial differential equation

.. math::

   \div \lambda\, \grad T - \rho c_\mathrm{p} \frac{\partial T}{\partial t} = -w,

where $\lambda$ is thermal conductivity, $T$ is temperature, $\rho$ is density, $c_\mathrm{p}$ is specific heat and finally $w$ is source of the inner heat (eddy current, chemical source, ...). Term with partial derivative is in steady-state analysis neglected. Thermal flux can be written as

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

