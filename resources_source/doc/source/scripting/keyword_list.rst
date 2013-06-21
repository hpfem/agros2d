.. _keyword-list:

************
Keyword List
************

Physical Fields
===============

Acoustic
--------

Module Id: acoustic 

Constants
^^^^^^^^^
+----+-------------------------+
| Id | Value                   |
+====+=========================+
| P0 | :math:`2 \cdot 10^{-5}` |
+----+-------------------------+

Variables
^^^^^^^^^
+--------+------------------------------------------------+
| Id     | Description                                    |
+========+================================================+
| ac_pr  | real part of acoustic pressure                 |
+--------+------------------------------------------------+
| ac_pi  | imaginary part of acoustic pressure            |
+--------+------------------------------------------------+
| ac_anr | real part of acoustic normal acceleration      |
+--------+------------------------------------------------+
| ac_ani | imaginary part of acoustic normal acceleration |
+--------+------------------------------------------------+


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

Linearity Types
---------------

* linear
* picard
* newton

Matrix Solver Types
-------------------

* umfpack
* petsc
* mumps
* superlu
* trilinos_amesos
* trilinos_aztecoo

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

Current field
^^^^^^^^^^^^^

* current_potential
* current_inward_current_flow

Magnetic field
^^^^^^^^^^^^^^

* magnetic_vector_potential
* magnetic_surface_current_density

TE Waves
^^^^^^^^
* rf_electric_field
* rf_surface_current
* rf_matched_boundary
* rf_port

Heat transfer
^^^^^^^^^^^^^

* heat_temperature
* heat_heat_flux

Acoustic field
^^^^^^^^^^^^^^
* acoustic_pressure
* acoustic_normal_acceleration
* acoustic_impedance
* acoustic_matched_boundary

Structural mechanics
^^^^^^^^^^^^^^^^^^^^

* elasticity_fixed
* elasticity_free

Incompressible flow
^^^^^^^^^^^^^^^^^^^
* flow_velocity
* flow_pressure
* flow_outlet
* flow_wall

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

Current field
^^^^^^^^^^^^^

* current_potential
* current_electic_field
* current_current_density
* current_power_losses
* current_conductivity

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

TE Waves
^^^^^^^^

* rf_electric_field
* rf_electric_field_real
* rf_electric_field_imag
* rf_magnetic_field
* rf_magnetic_field_x_real
* rf_magnetic_field_x_imag
* rf_magnetic_field_y_real
* rf_magnetic_field_y_imag
* rf_magnetic_flux_density
* rf_magnetic_flux_density_x_real
* rf_magnetic_flux_density_x_imag
* rf_magnetic_flux_density_y_real
* rf_magnetic_flux_density_y_imag
* rf_poynting_vector
* rf_poynting_vector_x
* rf_poynting_vector_y
* rf_power_losses
* rf_permittivity
* rf_permeability
* rf_conductivity
* rf_current_density_real
* rf_current_density_imag

Heat transfer
^^^^^^^^^^^^^

* heat_temperature
* heat_temperature_gradient
* heat_heat_flux
* heat_conductivity

Acoustic field
^^^^^^^^^^^^^^

* acoustic_pressure
* acoustic_pressure_real
* acoustic_pressure_imag
* acoustic_local_velocity
* acoustic_local_acceleration
* acoustic_density
* acoustic_speed

Structural mechanics
^^^^^^^^^^^^^^^^^^^^

* elasticity_von_mises_stress
* elasticity_displacement
* strain_xx
* strain_yy
* strain_zz
* strain_xy
* stress_xx
* stress_yy
* stress_zz
* stress_xy

Incompressible flow
^^^^^^^^^^^^^^^^^^^

* flow_velocity
* flow_velocity_x
* flow_velocity_y
* flow_velocity

Physic Field Variable Components
--------------------------------

* scalar
* magnitude
* x (for axysimmetric problem types r)
* y (for axysimmetric problem types z)

Point Results
-------------

General field
^^^^^^^^^^^^^

* X, Y, V, Gx, Gy, G, constant

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* X, Y, V, Ex, Ey, E, Dx, Dy, D, we, epsr

Current field
^^^^^^^^^^^^^

* X, Y, V, Jx, Jy, J, Ex, Ey, E, pj, gamma

Magnetic field
^^^^^^^^^^^^^^

* X, Y, A_real, A_imag, A, B, Bx_real, By_real, B_real, Bx_imag, By_imag, B_imag, H, Hx_real, Hy_real, H_real, Hx_imag, Hy_imag, H_imag, Je_real, Je_imag, Je, Jit_real, Jit_imag, Jit, Jiv_real, Jiv_imag, Jiv, J_real, J_imag, J, pj, wm, mur, gamma, Br, Brangle, vx, vy, Fx_real, Fy_real, Fx_imag, Fy_imag

TE Waves
^^^^^^^^

* X, Y, E_real, E_imag, E, B, Bx_real, By_real, B_real, Bx_imag, By_imag, B_imag, H, Hx_real, Hy_real, H_real, Hx_imag, Hy_imag, H_imag, Px, Py, P, epsr, mur, gamma, Je_real, Je_imag, Je

Heat transfer
^^^^^^^^^^^^^

* X, Y, t, T, Gx, Gy, G, Fx, Fy, F, lambda

Acoustic field
^^^^^^^^^^^^^^
* X, Y, p_real, p_imag, p, Lp, rho, c

Structural mechanics
^^^^^^^^^^^^^^^^^^^^

* X, Y, mises, tresca, u, v, disp, sxx, syy, szz, sxy, exx, eyy, ezz, exy

Incompressible flow
^^^^^^^^^^^^^^^^^^^

* X, Y, t, v, vx, vy, vz, p

Surface Integrals
-----------------

General field
^^^^^^^^^^^^^

* l, S

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* l, S, Q

Current field
^^^^^^^^^^^^^

* l, S, I

Magnetic field
^^^^^^^^^^^^^^

* l, S, Fx, Fy

TE Waves
^^^^^^^^

* l, S

Heat transfer
^^^^^^^^^^^^^

* l, S, T_avg, T_diff, F

Acoustic field
^^^^^^^^^^^^^^

* l, S, p_real, p_imag

Structural mechanics
^^^^^^^^^^^^^^^^^^^^

* l, S

Incompressible flow
^^^^^^^^^^^^^^^^^^^

* l, S

Volume Integrals
----------------

General field
^^^^^^^^^^^^^

* V, S

Electrostatic field
^^^^^^^^^^^^^^^^^^^

* V, S, We

Current field
^^^^^^^^^^^^^

* V, S, Jx_avg, Jy_avg, J_avg, Ex_avg, Ey_avg, E_avg, Pj

Magnetic field
^^^^^^^^^^^^^^

* V, S, Ie_real, Ie_imag, Iit_real, Iit_imag, Iiv_real, Iiv_imag, I_real, I_imag, Fx, Fy, T, Pj, Wm

TE Waves
^^^^^^^^

* V, S

Heat transfer
^^^^^^^^^^^^^

* V, S, T_avg, Gx_avg, Gy_avg, G_avg, Fx_avg, Fy_avg, F_avg

Acoustic field
^^^^^^^^^^^^^^

* V, S, p_real, p_imag

Structural mechanics
^^^^^^^^^^^^^^^^^^^^

* V, S

Incompressible flow
^^^^^^^^^^^^^^^^^^^

* V, S

Modes
-----

* node
* edge
* label
* postprocessor

Postprocessor Views
-------------------

* none
* scalar
* scalar3d
* order
