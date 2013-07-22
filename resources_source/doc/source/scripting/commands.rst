.. highlight:: python

Agros2D scripting reference guide
=================================

Scripting in Agros2D is based on scripting language `Python <http://www.python.org>`_.


Class *problem*
---------------

Class *problem* associate general informations about solved task.    


Methods
^^^^^^^

**problem**\(clear)
  Creates an instance of the class *problem*.  

+-----------+------+---------+
| Parameter | Type | Meaning |
+===========+======+=========+
| clear     | bool |         |
+-----------+------+---------+


Example of usage

.. code-block:: python
    
   problem = agros2d.problem(clear = True)

**solve**\()
  Solves the problem.

Example of usage

.. code-block:: python

   problem.solve()


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
  
   

Class *field*
-------------

Object *field* associates informations specific for given physical field. 


Methods
^^^^^^^

**field**\(Name)
  creates field object

Example of usage     

.. code-block:: python
      
   heat = agros2d.field("heat")
     

**add_boundary**\(Name, Type, Value)
  Adds new boundary condition to the problem.    
   
.. tabularcolumns:: lll 

+---------------+----------+--------------------------------+--------------------------------+-+
| **Parameter** | **Type** | **Meaning**                    |                                | |
+---------------+----------+--------------------------------+--------------------------------+-+
| Name          | string   | Name of the boundary condition |                                | |
+---------------+----------+--------------------------------+--------------------------------+-+
| Type          | string   |                                | Type of the boundary condition | |
|               |          |                                | (See module documentation)     | |
+---------------+----------+--------------------------------+--------------------------------+-+



Example of usage

.. code-block:: python
   
   heat.add_boundary("Left", "heat_temperature", {"heat_temperature" : 10})
                    

**add_material**\(Name, Type, Value)
  Adds new material to the problem.     
   
Example of usage

.. code-block:: python
   
   heat.add_material("Material", {"heat_conductivity" : 230, "heat_volume_heat" : 0})

                                                         

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


Class *geometry*
----------------

Class *geometry* associates informations about geometry of the model.
 
Methods
^^^^^^^

geometry
""""""""
  
creates geometry object

Example of usage     

.. code-block:: python
      
   geometry = agros2d.geometry
   
   
**add_edge**\(x1 <double>, x2 <double>, y1 <double>, y2 <double>, boundaries = <dict>, angle = <double> )


creates edge and assign boundary conditions

Example of usage, edge with boundary condition

.. code-block:: python

   geometry.add_edge(-0.25, 0.25, -0.1, 0.1, boundaries = {"heat" : "Convection"}) 

Example of usage, edge without boundary condition

.. code-block:: python

   geometry.add_edge(-0.25, 0.25, -0.1, 0.1, boundaries = {}) 
