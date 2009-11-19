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

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Scalar potential $\varphi = f$ on the boundary is known.

* Neumann BC

  Surface charge density $D_\mathrm{n} = g$ on the boundary is known.

Boundary integrals
^^^^^^^^^^^^^^^^^^

Charge: $Q = \int_S D_\mathrm{n} \dif S = \int_S \varepsilon \frac{\partial \varphi}{\partial n} \dif S\,\,\,\mathrm{(C)}$

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

Average electric field:

.. math::

   E_\mathrm{avg} = \frac{1}{V} \int_V \sqrt{E_x^2 + E_y^2} \dif V\,\,\,\mathrm{(V/m)}

Average displacement:

.. math::

   D_\mathrm{avg} = \frac{1}{V} \int_V \sqrt{D_x^2 + D_y^2} \dif V\,\,\,\mathrm{(C/m^2)}

Energy:

.. math::

   W_\mathrm{e} = \int_V \frac{1}{2} E D \dif V\,\,\,\mathrm{(J)}

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

Magnetostatic Field
-------------------

Magnetostatic field can be desribed by Poisson partial differential equation

.. math::

   \curl \frac{1}{\mu}\, \left( \curl \vec{A} - \vec{B}_\mathrm{r} \right) = \vec{J}_\mathrm{ext},

where $\mu$ is permeability of the material, $\vec{A} = ( A_z\,\mathrm{or}\,A_{\varphi} )$ is component of the magnetic vector potential, $\vec{B}_\mathrm{r}$ is remanent flux density and finally $J_\mathrm{ext} = ( \vec{J}_z\,\mathrm{or}\,J_{\varphi} )$ is component of source current density. Magnetic flux density is given by form

.. math::

   \vec{B} = \curl \vec{A}

and magnetic field is

.. math::

   \vec{H} = \frac{\vec{B}}{\mu}.

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Component of the magnetic vector potential $A = f$ on the boundary is known.

* Neumann BC

  Normal derivative of magnetic vector potential $\frac{\partial A}{\partial n} = g$ on the boundary is known.

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

Average magnetic field:

.. math::

   H_\mathrm{avg} = \frac{1}{V} \int_V \sqrt{H_x^2 + H_y^2} \dif V\,\,\,\mathrm{(V/m)}

Average flux density:

.. math::

   B_\mathrm{avg} = \frac{1}{V} \int_V \sqrt{B_x^2 + B_y^2} \dif V\,\,\,\mathrm{(C/m^2)}

Energy:

.. math::

   W_\mathrm{m} = \int_V \frac{1}{2} \vec{H} \vec{B} \dif V\,\,\,\mathrm{(J)}

Harmonic Magnetic Field
-----------------------

Harmonic magnetic field can be described by partial differential equation

.. math::

   \curl \frac{1}{\mu}\, \curl \vecfaz{A} + \mj \omega \sigma \vecfaz{A} = \vecfaz{J}_\mathrm{ext},

where $\mu$ is permeability of the material, $\faz{A} = ( \faz{A}_z\,\mathrm{or}\,\faz{A}_{\varphi} )$ is component of the magnetic vector potential, $\omega = 2 \pi f$ is frequency, $\sigma$ is electric conductivity and finally $\faz{J}_\mathrm{ext} = ( \faz{J}_z\,\mathrm{or}\,\faz{J}_{\varphi} )$ is component of source current density. Magnetic flux density is given by form

.. math::

   \vecfaz{B} = \curl \vecfaz{A}

and magnetic field is

.. math::

   \vecfaz{H} = \frac{\vecfaz{B}}{\mu}.

Eddy current density is

.. math::

   \vecfaz{J}_\mathrm{eddy} = \mj \omega \sigma \vecfaz{A}.

Boundary conditions
^^^^^^^^^^^^^^^^^^^

* Dirichlet BC

  Component of the magnetic vector potential $\faz{A} = \faz{f}$ on the boundary is known.

* Neumann BC

  Normal derivative of magnetic vector potential $\frac{\partial \faz{A}}{\partial n} = \faz{g}$ on the boundary is known.

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

Eddy current:

.. math::

   \faz{I}_\mathrm{eddy} = \int_S \vecfaz{J}_\mathrm{eddy} \dif S\,\,\,\mathrm{(A)}

Total current: 

.. math::

   \faz{I}_\mathrm{total} = \int_S \vecfaz{J}_\mathrm{eddy} + \vecfaz{J}_\mathrm{ext} \dif S\,\,\,\mathrm{(A/m)}

Power losses:

.. math::

   P = \int_V \frac{\left( \vecfaz{J}_\mathrm{total} \cdot \vecfaz{J}_\mathrm{total}^* \right)}{\sigma} \dif V\,\,\,\mathrm{(W)}

Lorentz force:

.. math::

   F_\mathrm{L} = \int_V \vecfaz{J} \times \vecfaz{B} \dif V\,\,\,\mathrm{(N)}

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

Average temperature: $T_\mathrm{avg} = \frac{1}{S} \int_S T \dif S\,\,\,\mathrm{(deg.)}$

Heat flux: $F = \int_S \lambda \frac{\partial T}{\partial n} \dif S\,\,\,\mathrm{(W)}$

Subdomain integrals
^^^^^^^^^^^^^^^^^^^

Average temperature:

.. math::

   T_\mathrm{avg} = \frac{1}{V} \int_V T \dif V\,\,\,\mathrm{(deg.)}

Average heat flux:

.. math::

   F_\mathrm{avg} = \frac{1}{V} \int_V \sqrt{F_x^2 + F_y^2} \dif V\,\,\,\mathrm{(W)}

Average temperature gradient:

.. math::

   G_\mathrm{avg} = \frac{1}{V} \int_V \sqrt{G_x^2 + G_y^2} \dif V\,\,\,\mathrm{(Km)}

