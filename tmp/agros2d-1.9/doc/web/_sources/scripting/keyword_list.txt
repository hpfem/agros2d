.. _keyword-list:

Keyword List
============

Physic Fields
-------------

* general

* electrostatic

* magnetic

* current

* heat

Problem Types
-------------

* planar

* axisymmetric

Adaptivity Types
----------------

* disabled

* h-adaptivity

* p-adaptivity

* hp-adaptivity

Analysis Types
--------------

* steadystate

* transient

* harmonic

Physic Field Variables
----------------------

General field
^^^^^^^^^^^^^

* general_variable

* general_gradient

* general_constant

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* electrostatic_potential

* electrostatic_electric_field

* electrostatic_displacement

* electrostatic_energy_density

* electrostatic_permittivity

Magnetic field
^^^^^^^^^^^^^^

* magnetic_vector_potential_real

* magnetic_vector_potential_imag

* magnetic_vector_potential

* magnetic_flux_density_real

* magnetic_flux_density_imag

* magnetic_flux_density

* magnetic_magnetic_field_real

* magnetic_magnetic_field_imag

* magnetic_magnetic_field

* magnetic_current_density_imag

* magnetic_current_density_imag

* magnetic_current_density

* magnetic_current_density_induced_transform_real

* magnetic_current_density_induced_transform_imag

* magnetic_current_density_induced_transform

* magnetic_current_density_induced_velocity_real

* magnetic_current_density_induced_velocity_imag

* magnetic_current_density_induced_velocity

* magnetic_current_density_total_real

* magnetic_current_density_total_imag

* magnetic_current_density_total

* magnetic_power_losses

* magnetic_lorentz_force

* magnetic_energy_density

* magnetic_permeability

* magnetic_conductivity

* magnetic_remanence

* magnetic_velocity

Current field
^^^^^^^^^^^^^

* current_potential

* current_electic_field

* current_current_density

* current_power_losses

* current_conductivity

Heat transfer
^^^^^^^^^^^^^

* heat_temperature

* heat_temperature_gradient

* heat_heat_flux

* heat_conductivity

Physic Field Variable Components
--------------------------------

* scalar

* magnitude

* x (for axysimmetric problem types r)

* y (for axysimmetric problem types z)

Boundary Conditions
-------------------

* none

General field
^^^^^^^^^^^^^

* general_value

* general_derivative

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* electrostatic_potential

* electrostatic_surface_charge_density

Magnetic field
^^^^^^^^^^^^^^

* magnetic_vector_potential

* magnetic_surface_current_density

Current field
^^^^^^^^^^^^^

* current_potential

* current_inward_current_flow

Heat transfer
^^^^^^^^^^^^^

* heat_temperature

* heat_heat_flux

Point Results
-------------

General field
^^^^^^^^^^^^^

* X, Y, V, Gx, Gy, G, constant

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* X, Y, V, Ex, Ey, E, Dx, Dy, D, we, epsr

Magnetic field
^^^^^^^^^^^^^^

* X, Y, A_real, A_imag, A, B, Bx_real, By_real, B_real, Bx_imag, By_imag, B_imag, H, Hx_real, Hy_real, H_real, Hx_imag, Hy_imag, H_imag, Je_real, Je_imag, Je, Jit_real, Jit_imag, Jit, Jiv_real, Jiv_imag, Jiv, J_real, J_imag, J, pj, wm, mur, gamma, Br, Brangle, vx, vy, Fx_real, Fy_real, Fx_imag, Fy_imag

Current field
^^^^^^^^^^^^^

* X, Y, V, Jx, Jy, J, Ex, Ey, E, pj, gamma

Heat transfer
^^^^^^^^^^^^^

* X, Y, t, T, Gx, Gy, G, Fx, Fy, F, lambda

Volume Integrals
----------------

General field
^^^^^^^^^^^^^

* V, S

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* V, S, We

Magnetic field
^^^^^^^^^^^^^^

* V, S, Ie_real, Ie_imag, Iit_real, Iit_imag, Iiv_real, Iiv_imag, I_real, I_imag, Fx, Fy, T, Pj, Wm

Current field
^^^^^^^^^^^^^

* V, S, Jx_avg, Jy_avg, J_avg, Ex_avg, Ey_avg, E_avg, Pj

Heat transfer
^^^^^^^^^^^^^

* V, S, T_avg, Gx_avg, Gy_avg, G_avg, Fx_avg, Fy_avg, F_avg

Surface Integrals
-----------------

General field
^^^^^^^^^^^^^

* l, S

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* l, S, Q

Magnetic field
^^^^^^^^^^^^^^

* l, S, Fx, Fy

Current field
^^^^^^^^^^^^^

* l, S, I

Heat transfer
^^^^^^^^^^^^^

* l, S, T_avg, T_diff, F

Postprocessor Views
-------------------

* none

* scalar

* scalar3d

* order
