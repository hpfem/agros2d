.. _keyword-list:


Keyword List
************

Preprocessor
============

Modes
-----

* node
* edge
* label
* postprocessor


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

Postprocessor Views
-------------------

* none
* scalar
* scalar3d
* order


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

Available constants and variables are described in the  documentation of corresponding module.

* :ref:`acoustic` 
* :ref:`current`
* :ref:`elasticity`
* :ref:`electrostatic`
* :ref:`flow`
* :ref:`heat`
* :ref:`magnetic`
* :ref:`rf`
