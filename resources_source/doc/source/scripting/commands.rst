.. highlight:: python

.. _scripting:

Agros2D scripting reference guide
=================================

Scripting in Agros2D is based on scripting language `Python <http://www.python.org>`_.


Object *problem*
----------------

Object *problem* associate general informations about solved task.    


Methods
^^^^^^^

problem(clear)
""""""""""""""

creates problem object

Example of usage

.. code-block:: python
    
   problem = agros2d.problem(clear = True)


Variables
^^^^^^^^^

+-----------------+----------------------------------------+------------------------------------------------------------------------------------------+
| Name            | Example of usage                       | Possible values                                                                          |
+=================+========================================+==========================================================================================+
| coordinate_type | ``problem.coordinate_type = "planar"`` | planar, axisymmetric                                                                     |
+-----------------+----------------------------------------+------------------------------------------------------------------------------------------+
| mesh_type       | ``problem.mesh_type = "triangle"``     | triangle, triangle_quad_fine_division, triangle_quad_rough_division, triangle_quad_join, |
|                 |                                        | gmsh_triangle, gmsh_quad, gmsh_quad_delaunay                                             |
+-----------------+----------------------------------------+------------------------------------------------------------------------------------------+
  
   

Object *field*
--------------

Object *field* associates informations specific for given physical field. 


Methods
^^^^^^^

field(Name <string>)
""""""""""""""""""""
  
creates field object

Example of usage     

.. code-block:: python
      
   heat = agros2d.field("heat")
     

add_boundary(Name <String>, Type <String>, Value <Dict>)
""""""""""""""""""""""""""""""""""""""""""""""""""""""""
  
adds new boundary condition    
   
Example of usage

.. code-block:: python
   
   heat.add_boundary("Left", "heat_temperature", {"heat_temperature" : 10})
                    
                                                         

Variables
^^^^^^^^^

Solver settings
"""""""""""""""

+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| Name                                             | Example of usage                                                | Possible values                  |
+==================================================+=================================================================+==================================+
| analysis_type                                    | ``heat.analysis_type = "steadystate"``                          | steadystate, transient, harmonic |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| number_of_refinements                            | ``heat.number_of_refinements = 2``                              | 0, 1, ..., 10                    |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| polynomial_order                                 | ``heat.polynomial_order = 2``                                   | 1, 2, ..., 5                     |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| linearity_type                                   | ``heat.linearity_type = "newton"``                              | linear, picard, newton           |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| nonlinear_tolerance                              | ``heat.nonlinear_tolerance = 0.0001``                           |                                  |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| nonlinear_steps                                  | ``heat.nonlinear_steps = 30``                                   |                                  |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| damping_type                                     | ``heat.damping_type = "automatic"``                             | off, automatic, fixed            |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| damping_coeff                                    | ``heat.damping_coeff = 0.8``                                    |                                  |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| reuse_jacobian                                   | ``heat.reuse_jacobian = True``                                  | True, False                      |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| maximum_steps_with_reused_jacobian               | ``heat.maximum_steps_with_reused_jacobian = 10``                |                                  |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+
| sufficient_improvement_factor_for_jacobian_reuse | ``heat.sufficient_improvement_factor_for_jacobian_reuse = 0.3`` |                                  |
+--------------------------------------------------+-----------------------------------------------------------------+----------------------------------+



