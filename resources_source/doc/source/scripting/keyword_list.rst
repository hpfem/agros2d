.. _keyword-list:


Keyword List
************

Preprocessor
============


Problem Types
-------------

* planar
* axisymmetric

Example of usage: 

.. code-block:: python

   problem.coordinate_type = planar 


Analysis type
-------------

* steadystate
* transient
* harmonic

Example of usage:

.. code-block:: python

   acoustic = agros2d.field("acoustic")
   acoustic.analysis_type = "harmonic"


Coupling types
--------------

* hard 
* weak
* none

Example of usage:

.. code-block:: python
   
   problem = agros2d.problem(clear = True)
   problem.set_coupling_type("current", "heat", "weak")



Mesh types
----------

* triangle
* triangle_quad_fine_division
* triangle_quad_rough_division
* triangle_quad_join
* gmsh_triangle
* gmsh_quad
* gmsh_quad_delaunay

Example of usage:
.. code-block:: python
   
   problem = agros2d.problem(clear = True)
   problem.mesh_type = "triangle"


Solver settings
===============


Time step methods
-----------------

* fixed
* adaptive
* adaptive_numsteps
* bdf2_adaptive, 
* bdf2_combine, 
* fixed_bdf2b, 
* fixed_combine

Adaptivity types
----------------

* disabled 
* h-adaptivity
* p-adaptivity
* hp-adaptivity


Adaptivity stopping criterion types
-----------------------------------

* cumulative
* singleelement
* levels


Adaptivity norm types
---------------------

* h1_norm
* l2_norm 
* h1_seminorm     


Solution types
--------------

* normal
* reference
* finer


Matrix solver types
-------------------
* umfpack
* paralution
* mumps
* superlu
* petsc
* trilinos_amesos
* trilinos_aztecoo


Space types
-----------

* h1
* hcurl
* hdiv
* l2


Non-linear solvers settings
---------------------------

Linearity types
---------------

* linear
* picard
* newton


Damping types
-------------

* off
* automatic
* fixed

Data table types
----------------

* cubic_spline 
* piecewise_linear
* constant
   

Butcher table types
-------------------

* heun-euler 
* bogacki-shampine 
* fehlberg 
* cash-karp
* dormand-prince


Newton solver types
-------------------

* residual_norm_absolute
* residual_norm_relative_to_initial
* residual_norm_relative_to_previous
* residual_norm_ratio_to_initial
* residual_norm_ratio_to_previous
* solution_distance_from_previous_absolute
* solution_distance_from_previous_relative

Example of non-linear solver setting

.. code-block:: python

   heat = agros2d.field("heat")
   heat.analysis_type = "steadystate"
   heat.number_of_refinements = 2
   heat.polynomial_order = 2
   heat.linearity_type = "newton"
   heat.nonlinear_tolerance = 0.0001
   heat.nonlinear_steps = 30
   heat.damping_type = "automatic"
   heat.damping_coeff = 0.8
   heat.reuse_jacobian = True
   heat.maximum_steps_with_reused_jacobian = 10
   heat.sufficient_improvement_factor_for_jacobian_reuse = 0.3


Iterative solvers
-----------------

* cg
* gmres
* bicgstab, 


Preconditioners
---------------

* jacobi
* ilu
* multicoloredsgs
* multicoloredilu
* aichebyshev


Postprocessing
==============

Palette types
-------------

* agros2d
* jet 
* copper
* hot
* bone
* pink
* spring
* summer
* autumn
* winter
* hsv
* bw_ascending
* bw_descending
    

Palette quality
---------------

* extremely_coarse
* extra_coarse
* coarser
* normal
* fine
* finer
* extra_fine


Palette order types
-------------------

* hermes
* jet
* copper
* hot
* bone
* pink
* spring
* summer
* autumn
* winter
* hsv
* bw_ascending
* bw_descending


Vector types
------------

* arrow
* cone


Vector center types
-------------------

* tail
* head
* center


Postprocessing 3D
-----------------

* scalar
* scalarsolid
* model


Physical Fields
===============

General field
^^^^^^^^^^^^^

* none
* general_value
* general_derivative


Electrostatic field
^^^^^^^^^^^^^^^^^^^

* none
* electrostatic_potential
* electrostatic_surface_charge_density


Current field
^^^^^^^^^^^^^

* none
* current_potential, 
* current_inward_current_flow


Magnetic field
^^^^^^^^^^^^^^

* none
* magnetic_potential
* magnetic_surface_current



TE Waves
^^^^^^^^

* none
* rf_te_electric_field
* rf_te_magnetic_field
* rf_te_surface_current
* rf_te_impedance


TM Waves
^^^^^^^^

* none
* rf_te_electric_field
* rf_te_magnetic_field
* rf_te_surface_current
* rf_te_impedance


Heat transfer
^^^^^^^^^^^^^
 
 * none
 * heat_temperature
 * heat_heat_flux

Acoustic field
^^^^^^^^^^^^^^

* acoustic_pressure
* acoustic_normal_acceleration
* acoustic_impedance


Structural mechanics
^^^^^^^^^^^^^^^^^^^^

* elasticity_fixed_fixed
* elasticity_fixed_free
* elasticity_free_fixed
* elasticity_free_free

Incompressible flow
^^^^^^^^^^^^^^^^^^^

* flow_velocity
* flow_pressure
* flow_outlet
* flow_symmetry


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
* electrostatic_charge_density

Current field
^^^^^^^^^^^^^

* current_potential
* current_electic_field
* current_current_density_conductive_real
* current_current_density_total_real
* current_joule_losses
* current_conductivity

Magnetic field
^^^^^^^^^^^^^^

* magnetic_potential
* magnetic_potential_real
* magnetic_potential_imag
* magnetic_potential_lines
* magnetic_potential_lines_real
* magnetic_potential_lines_imag
* magnetic_flux_density 
* magnetic_flux_density_real
* magnetic_flux_density_imag
* magnetic_magnetic_field
* magnetic_magnetic_field_real
* magnetic_magnetic_field_imag
* magnetic_energy_density
* magnetic_permeability
* magnetic_conductivity
* magnetic_remanence
* magnetic_velocity
* magnetic_current_density_external
* magnetic_current_density_external_real
* magnetic_current_density_external_imag
* magnetic_current_density_induced_velocity
* magnetic_current_density_induced_velocity_real
* magnetic_current_density_induced_velocity_imag
* magnetic_current_density_induced_transform
* magnetic_current_density_induced_transform_real
* magnetic_current_density_induced_transform_imag
* magnetic_current_density_total
* magnetic_current_density_total_real
* magnetic_current_density_total_imag 
* magnetic_losses_density
* magnetic_lorentz_force


TE Waves
^^^^^^^^

* rf_te_electric_field
* rf_te_electric_field_real
* rf_te_electric_field_imag
* rf_te_displacement
* rf_te_displacement_real
* rf_te_displacement_imag
* rf_te_surface_current_real
* rf_te_surface_current_imag
* rf_te_magnetic_field 
* rf_te_magnetic_field_real
* rf_te_magnetic_field_imag
* rf_te_flux_density
* rf_te_flux_density_real
* rf_te_flux_density_imag
* rf_te_poynting_vector
* rf_te_permittivity
* rf_te_permeability
* rf_te_conductivity
 

TM Waves
^^^^^^^^

* rf_te_electric_field
* rf_te_electric_field_real
* rf_te_electric_field_imag
* rf_te_displacement
* rf_te_displacement_real
* rf_te_displacement_imag
* rf_te_surface_current_real
* rf_te_surface_current_imag
* rf_te_magnetic_field 
* rf_te_magnetic_field_real
* rf_te_magnetic_field_imag
* rf_te_flux_density
* rf_te_flux_density_real
* rf_te_flux_density_imag
* rf_te_poynting_vector
* rf_te_permittivity
* rf_te_permeability
* rf_te_conductivity


Heat transfer
^^^^^^^^^^^^^

* heat_temperature
* heat_temperature_gradient
* heat_heat_flux
* heat_conductivity
* heat_volume_heat
* heat_density
* heat_specific_heat
* heat_velocity


Acoustic field
^^^^^^^^^^^^^^

* acoustic_pressure
* acoustic_pressure_real
* acoustic_pressure_imag
* acoustic_sound_pressure_level
* acoustic_local_acceleration_real
* acoustic_local_acceleration_imag
* acoustic_pressure_derivative
* acoustic_density
* acoustic_speed


Structural mechanics
^^^^^^^^^^^^^^^^^^^^

* elasticity_displacement
* elasticity_young_modulus
* elasticity_von_mises_stress
* elasticity_poisson_ratio
* elasticity_inner_force
* elasticity_alpha
* elasticity_temperature_difference
* elasticity_temperature_reference


Incompressible flow
^^^^^^^^^^^^^^^^^^^

* flow_velocity
* flow_pressure
* flow_vorticity
* flow_density
* flow_viscosity
* flow_inner_force


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
