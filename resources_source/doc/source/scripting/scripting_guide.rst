.. highlight:: python

.. _scripting:


*********************************
Agros2D scripting reference guide
*********************************

Scripting in Agros2D is based on scripting language `Python <http://www.python.org>`_.


Module Agros2d Overview
=======================

Classes:
--------

* :ref:`class-field`
* :ref:`class-geometry`
* :ref:`class-particletracing`
* :ref:`class-problem`
* :ref:`class-viewconfig`
* :ref:`class-viewmeshandpost`
* :ref:`class-viewmesh`
* :ref:`class-viewpost`
* :ref:`class-viewpost2d`
* :ref:`class-viewpost3d`
* :ref:`class-viewparticletracing`
* :ref:`class-view`

Functions:
----------

* app_time
* field
* memory_usage
* open_file
* problem
* save_file

Variables:
----------

* problem = <agros2d.Problem object>
* test = {}
* geometry = <agros2d.Geometry object>
* particle_tracing = <agros2d.ParticleTracing object>
* view = <agros2d.View object>


Module Agros2d Details
======================

.. _class-field:

class Field
-----------
 
Methods:
^^^^^^^^

field(Name)
"""""""""""
Creates ``field`` object.


**Example of usage**     

.. code-block:: python
      
   heat = agros2d.field("heat")
   

adaptivity_info
"""""""""""""""
 
Return dictionary with adaptivity process info.
 
.. code-block:: python
 
    adaptivity_info(time_step = None, solution_type = "normal")
 
 
**Keyword arguments**:

* time_step -- time step (default is None - use last time step)
* solution_type -- solution type (default is "normal")
 
add_boundary
""""""""""""

Add new boundary condition.
 
.. code-block:: python
 
   add_boundary(name, type, parameters = {})
 
**Keyword arguments:**

* name -- boundary condition name
* type -- boundary contition type
* parameters -- dict of boundary condition parameters (default is {})
 
**Example of usage**

.. code-block:: python
   
   heat.add_boundary("Left", "heat_temperature", {"heat_temperature" : 10})
   
add_material
""""""""""""

Add new material.
 
.. code-block:: python
 
     add_material(name, type, parameters = {})
 
**Keyword arguments**:
name -- material name
type -- material type
parameters -- dict of material parameters (default is {})
 
**Example of usage**

.. code-block:: python
   
   heat.add_material("Material", {"heat_conductivity" : 230, "heat_volume_heat" : 0}) 
 
initial_mesh_info
"""""""""""""""""
Return dictionary with initial mesh info.
 
local_values
""""""""""""
Compute local values in point and return dictionary with results.
 
.. code-block:: python

   local_values(x, y, time_step = None, adaptivity_step = None, solution_type = "normal")
 
**Keyword arguments**:

* x -- x or r coordinate of point
* y -- y or z coordinate of point
* time_step -- time step (default is None - use last time step)
* adaptivity_step -- adaptivity step (default is None - use adaptive step)
* solution_type -- solution type (default is "normal")
 
modify_boundary
"""""""""""""""
Modify existing boundary condition.
 
.. code-block:: python

   modify_boundary(name, type = "", parameters = {})
 
**Keyword arguments**:
* name -- boundary condition name
* type -- boundary contition type (default is "")
* parameters -- dict of boundary condition parameters (default is {})
 
modify_material
"""""""""""""""
Modify existing material.
 
.. code-block:: python
   
   modify_material(name, type = "", parameters = {})
 
**Keyword arguments**:

* name -- material name
* type -- material type (default is {})
* parameters -- dict of material parameters (default is {})
 
remove_boundary
"""""""""""""""
Remove existing boundary condition.
 
.. code-block:: python

   remove_boundary(name)
 
**Keyword arguments**:

* name -- boundary condition name
 
remove_material
"""""""""""""""
Remove existing material.
 
.. code-block:: python
   
   remove_material(name, type parameters = {})
 
**Keyword arguments**:

* name -- material name
 
solution_mesh_info
""""""""""""""""""
Return dictionary with solution mesh info.
 
.. code-block:: python

   solution_mesh_info(time_step = None, adaptivity_step = None, solution_type = "normal")
 
**Keyword arguments**:
* time_step -- time step (default is None - use last time step)
* adaptivity_step -- adaptivity step (default is None - use adaptive step)
* solution_type -- solution type (default is "normal")
 
solver_info
"""""""""""
Return dictionary with solver info.
 
.. code-block:: python
   
   solver_info(time_step = None, adaptivity_step = None, solution_type = "normal")
 
**Keyword arguments**:

* time_step -- time step (default is None - use last time step)
* adaptivity_step -- adaptivity step (default is None - use adaptive step)
* solution_type -- solution type (default is "normal")
 
surface_integrals
"""""""""""""""""
Compute surface integrals on edges and return dictionary with results.
 
.. code-block:: python

   surface_integrals(edges = [], time_step = None, adaptivity_step = None, solution_type = "normal")
 
**Keyword arguments**:

* edges -- list of edges (default is [] - compute integrals on all edges)
* time_step -- time step (default is None - use last time step)
* adaptivity_step -- adaptivity step (default is None - use adaptive step)
* solution_type -- solution type (default is "normal")
 
volume_integrals
""""""""""""""""
Compute volume integrals on labels and return dictionary with results.
 
.. code-block:: python

   volume_integrals(labels = [], time_step = None, adaptivity_step = None, solution_type = "normal")
 
**Keyword arguments**:

* labels -- list of labels (default is [] - compute integrals on all labels)
* time_step -- time step (default is None - use last time step)
* adaptivity_step -- adaptivity step (default is None - use adaptive step)
* solution_type -- solution type (default is "normal")
 
Variables
^^^^^^^^^

* adaptivity_anisotropic_refinement 
* adaptivity_back_steps 
* adaptivity_finer_reference
* adaptivity_norm_type 
* adaptivity_redone_steps
* adaptivity_steps
* adaptivity_stopping_criterion
* adaptivity_threshold
* adaptivity_tolerance
* adaptivity_type
* analysis_type
* field_id
* matrix_iterative_solver_iterations
* matrix_iterative_solver_method
* matrix_iterative_solver_preconditioner
* matrix_iterative_solver_tolerance
* matrix_solver 
* newton_damping_decrease_ratio
* newton_damping_factor
* newton_damping_increase_steps
* newton_damping_type
* newton_jacobian_reuse
* newton_jacobian_reuse_ratio
* newton_jacobian_reuse_steps
* nonlinear_convergence_measurement
* nonlinear_steps
* nonlinear_tolerance
* number_of_refinements
* polynomial_order
* solver
* transient_initial_condition
* transient_time_skip
 
.. _class-geometry:

class Geometry
--------------

Methods
^^^^^^^
 
activate
""""""""
Activate preprocessor mode.
 
add_circle
""""""""""
Add circle by center point and radius.
 
.. code-block:: python

   add_circle(self, x0, y0, radius, boundaries = {}, materials = None)
 
**Keyword arguments**:

* x0 -- x or r coordinate of center point
* y0 -- y or z coordinate of center point
* radius -- radius of center
* boundaries -- boundaries {'field' : refinement} (default {})
* materials -- materials {'field' : 'material name'} (default None)
 
add_edge
""""""""
Add a new edge according to coordinates and return its index.
 
.. code-block:: python
   
   add_edge(x1, y1, x2, y2, angle = 0.0, refinements = {}, boundaries = {})
 
**Keyword arguments**:

* x1 -- x or r coordinate of start node
* y1 -- y or z coordinate of start node
* x2 -- x or r coordinate of end node
* y2 -- y or z coordinate of end node
* angle -- angle between connecting lines, which join terminal nodes of edge and center of arc (default 0.0)
* refinements -- refinement towards edge {'field' : refinement} (default {})
* boundaries -- boundary condition {'field' : 'boundary name'} (default {})
 
**Example of usage** 

* Edge with boundary condition

   .. code-block:: python
   
      geometry.add_edge(-0.25, 0.25, -0.1, 0.1, boundaries = {"heat" : "Convection"}) 

* Example of usage, edge without boundary condition**

   .. code-block:: python

      geometry.add_edge(-0.25, 0.25, -0.1, 0.1, boundaries = {}) 
   

add_edge_by_nodes
"""""""""""""""""
Add a new edge according to indexes of start and end node and return the index of edge.
 
.. code-block:: python
   
   add_edge(start_node_index, end_node_index, angle = 0.0, refinements = {}, boundaries = {})
 
**Keyword arguments**:

* start_node_index -- index of start node
* end_node_index -- index of end node
* angle -- angle between connecting lines, which join terminal nodes of edge and center of arc (default 0.0)
* refinements -- refinement towards edge {'field' : refinement} (default {})
* boundaries -- boundary condition {'field' : 'boundary name'} (default {})
 
add_label
"""""""""
Add a new label according to coordinates and return its index.
 
.. code-block:: python

   add_label(x, y, area = 0.0, refinements = {}, orders = {}, materials = {})
 
**Keyword arguments**:
 
* x -- x or r coordinate of label
* y -- y or z coordinate of label
* area -- cross section of circle which influence maximum cross section of elements (default 0.0)
* refinements -- refinement of area {'field' : refinement} (default {})
* orders -- polynomial order of area elements {'field' : refinement} (default {})
* materials -- materials {'field' : 'material name'} (default {})
 
add_node
""""""""
Add a new node according to coordinates and return its index.
 
.. code-block:: python
   
   add_node(x, y)
 
**Keyword arguments**:

* x -- x or r coordinate of node
* y -- y or z coordinate of node
 
add_rect
""""""""
Add rect by start point (lower left node), width and height.
 
.. code-block:: python

   add_rect(x0, y0, width, height, boundaries = {}, materials = None)
 
**Keyword arguments**:
 
* x0 -- x or r coordinate of start point
* y0 -- y or z coordinate of start points
* width -- width of rect
* height -- height of rect
* boundaries -- boundaries {'field' : refinement} (default {})
* materials -- materials {'field' : 'material name'} (default None)
 
add_semicircle
""""""""""""""
Add semicircle by center point of full circle and radius.
 
.. code-block:: python

   add_semicircle(self, x0, y0, radius, boundaries = {}, materials = None)

 
**Keyword arguments**:

* x0 -- x or r coordinate of center point
* y0 -- y or z coordinate of center point
* radius -- radius of center
* boundaries -- boundaries {'field' : refinement} (default {})
* materials -- materials {'field' : 'material name'} (default None)
 
edges_count
"""""""""""
Return count of existing edges.
 
geometry
""""""""
Creates geometry object.

**Example of usage**     

.. code-block:: python
      
   geometry = agros2d.geometry
   
labels_count
""""""""""""
Return count of existing labels.
 
modify_edge
"""""""""""
Modify parameters of existing edge.
 
.. code-block:: python

   modify_edge(index, angle = 0.0, refinements = {}, boundaries = {})
 
**Keyword arguments**:

* index -- edge index
* angle -- angle between connecting lines, which join terminal nodes of edge and center of arc (default 0.0)
* refinements -- refinement towards edge {'field' : refinement} (default {})
* boundaries -- boundary condition {'field' : 'boundary name'} (default {})
 
modify_label
""""""""""""
Modify parameters of existing label.
 
.. code-block:: python
   
   modify_label(index, area = 0.0, refinements = {}, orders = {}, materials = {})
 
**Keyword arguments**:

* index -- label index
* area -- cross section of circle which influence maximum cross section of elements (default 0.0)
* refinements -- refinement of area {'field' : refinement} (default {})
* orders -- polynomial order of area elements {'field' : refinement} (default {})
* materials -- materials {'field' : 'material name'} (default {})
 
move_selection
""""""""""""""
Move selected object (nodes, edges or labels).
 
.. code-block:: python

   move_selection(dx, dy, copy = False)
 
**Keyword arguments**:

* dx -- movement in x axis
* dy -- movement in y axis
* copy -- make a copy of moved objects (default is False)
 
nodes_count
"""""""""""
Return count of existing nodes.
 
remove_edges
""""""""""""
Remove edges according to their indexes.
 
.. code-block:: python

   remove_edges(edges = [])
 
**Keyword arguments**:

* edges -- list of indexes of removed edges (default [] - remove all edges)
 
remove_labels
"""""""""""""
Remove labels according to their indexes.
 
.. code-block:: python
   
   remove_labels(labels = [])

 
**Keyword arguments**:

* labels -- list of indexes of removed labels (default [] - remove all labels)
 
remove_nodes
""""""""""""
 
Remove nodes according to their indexes.
 
.. code-block:: python
   
   remove_nodes(nodes = [])
 
**Keyword arguments**:
* nodes -- list of indexes of removed nodes (default [] - remove all nodes)
 
remove_selection
""""""""""""""""
Remove selected objects (nodes, edges or labels).
 
rotate_selection
""""""""""""""""
Rotate selected object (nodes, edges or labels) around the point.
 
.. code-block:: python
   
   rotate_selection(x, y, angle, copy = False)
 
**Keyword arguments**:

* x -- x or r coordinate of point
* y -- y or r coordinate of point
* angle -- angle of rotation
* copy -- make a copy of moved objects (default is False)
 
scale_selection
"""""""""""""""
Scale selected object (nodes, edges or labels).
 
.. code-block:: python

   scale_selection(x, y, scale, copy = False)
 
**Keyword arguments**:

* x -- x or r coordinate of point
* y -- y or r coordinate of point
* scale -- scale factor
* copy -- make a copy of moved objects (default is False)
 
select_edge_by_point
""""""""""""""""""""

Select the closest edge around the point.
 
.. code-block:: python
   
   select_edge_by_point(x, y)
 
**Keyword arguments**:

* x -- x or r coordinate of point
* y -- y or r coordinate of point
 
select_edges
""""""""""""
Select edges according to their indexes.
 
.. code-block:: python
   
   select_edges(edges = [])
 
**Keyword arguments**:

* edges -- list of indexes (default is [] - select all edges)
 
select_label_by_point
"""""""""""""""""""""
Select the closest label around the point.
 
.. code-block:: python
  
   select_label_by_point(x, y)
 
**Keyword arguments**:

* x -- x or r coordinate of point
* y -- y or r coordinate of point
 
select_labels
"""""""""""""
Select labels according to their indexes.

.. code-block:: python 
   
   select_labels(labels = [])
 
**Keyword arguments**:

* labels -- list of indexes (default is [] - select all labels)
 
select_node_by_point
""""""""""""""""""""
Select the closest node around the point.

.. code-block:: python 
   
   select_node_by_point(x, y)
 
**Keyword arguments**:

* x -- x or r coordinate of point
* y -- y or r coordinate of point
 
select_nodes
""""""""""""
Select nodes according to their indexes.
 
.. code-block:: python
   
   select_nodes(nodes = [])
 
**Keyword arguments**:
nodes -- list of indexes (default is [] - select all nodes)
 
select_none
"""""""""""
Unselect all objects (nodes, edges or labels).
 
.. _class-particletracing: 
 
class ParticleTracing
---------------------

Methods:
^^^^^^^^

* positions
* solve
* times
* velocities
 
 
Variables:
^^^^^^^^^^
 
* blended_faces 
* butcher_table_type
* charge 
* coefficient_of_restitution
* collor_by_velocity
* custom_force
* drag_force_coefficient
* drag_force_density
* drag_force_reference_area
* include_relativistic_correction
* initial_position
* initial_velocity
* mass
* maximum_number_of_steps
* maximum_relative_error
* minimum_step
* multiple_show_particles
* number_of_particles
* particles_dispersion
* reflect_on_boundary
* reflect_on_different_material
* show_points
 
.. _class-problem:
 
class Problem
-------------
Class *problem* associate general informations about solved task.

Methods:
^^^^^^^^
 
problem(clear)
""""""""""""""
Creates an instance of the class *problem*. 
  
**Example of usage**

.. code-block:: python
    
   problem = agros2d.problem(clear = True)
   
   
clear(..)
"""""""""
Clear problem.
 
clear_solution
""""""""""""""
Clear solution.
 
elapsed_time
""""""""""""
Return elapsed time in seconds.
 
get_coupling_type
"""""""""""""""""
Return type of coupling.
 
.. code-block:: python
   
   get_coupling_type(source_field, target_field)
 

**Keyword arguments**:

* source_field -- source field id
* target_field -- target field id
 
mesh
""""
Area discretization.
 
refresh
"""""""
Refresh preprocessor and postprocessor.
 
set_coupling_type
"""""""""""""""""
Set type of coupling.
 
.. code-block:: python
   
   set_coupling_type(source_field, target_field, type)
 
**Keyword arguments**:

* source_field -- source field id
* target_field -- target field id
* type -- coupling type
 
solve
"""""
Solve problem.
 
solve_adaptive_step
"""""""""""""""""""
Solve one adaptive step.
 
time_steps_length
"""""""""""""""""
Return a list of time steps length.
 
time_steps_total
""""""""""""""""
Return a list of time steps.
 
 
Variables:
^^^^^^^^^^
 
* coordinate_type
* frequency
* mesh_type
* time_method_order
* time_method_tolerance
* time_step_method
* time_steps
* time_total
 
 
.. _class-viewconfig:

class ViewConfig
----------------

Variables
^^^^^^^^^
  
 * axes 
 * grid 
 * grid_step 
 * post_font_family 
 * post_font_size 
 * rulers 
 * rulers_font_family 
 * rulers_font_size
 
 
.. _class-viewmeshandpost:

class ViewMeshAndPost
---------------------
 
Variables:
^^^^^^^^^^
 
* adaptivity_step
* solution_type
* time_step
 
 
.. _class-viewmesh:

class ViewMesh(ViewMeshAndPost)
-------------------------------
 
Methods:
^^^^^^^^
 
* ViewMesh
* ViewMeshAndPost
* activate
* disable
 
 
Variables:
^^^^^^^^^^
 
 * field
 * initial_mesh
 * order
 * order_color_bar
 * order_label
 * order_palette
 * solution_mesh
 
 
  
Variables inherited from ViewMeshAndPost:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* adaptivity_step 
* solution_type
* time_step


.. _class-viewparticletracing:

class ViewParticleTracing
-------------------------

Methods:
^^^^^^^^
 
* activate


.. _class-viewpost2d:

class ViewPost2D(ViewPost)
--------------------------

Methods:
^^^^^^^^
 
* activate 
* disable

Inherited methods:
^^^^^^^^^^^^^^^^^^
 
* ViewPost2D
* ViewPost
* ViewMeshAndPost
* builtin.object
 

Variables:
^^^^^^^^^^
 
* contours
* contours_count
* contours_variable
* scalar
* vectors
* vectors_center
* vectors_color
* vectors_count
* vectors_proportional
* vectors_scale
* vectors_type
* vectors_variable

 
Inherited variables:
^^^^^^^^^^^^^^^^^^^^
 
* field
* scalar_auto_range 
* scalar_color_bar
* scalar_component
* scalar_decimal_place
* scalar_log_base
* scalar_log_scale
* scalar_palette
* scalar_palette_filter
* scalar_palette_quality
* scalar_palette_steps
* scalar_range_max
* scalar_range_min
* scalar_variable
* adaptivity_step
* solution_type
* time_step


.. _class-viewpost3D:

class ViewPost3D(ViewPost)
--------------------------

Methods:
^^^^^^^^

* activate

Inherited methods:
^^^^^^^^^^^^^^^^^^

* ViewPost3D
* ViewPost
* ViewMeshAndPost
* builtin.object
 

Variables
^^^^^^^^^
 
* mode
 
Inherited Variables
^^^^^^^^^^^^^^^^^^^
 
* field
* scalar_auto_range
* scalar_color_bar
* scalar_component
* scalar_decimal_place
* scalar_log_base
* scalar_log_scale
* scalar_palette
* scalar_palette_filter
* scalar_palette_quality
* scalar_palette_steps
* scalar_range_max
* scalar_range_min
* scalar_variable
* adaptivity_step
* solution_type
* time_step

.. _class-viewpost:

class ViewPost(ViewMeshAndPost)
-------------------------------
 
Methods:
^^^^^^^^

* ViewPost
* ViewMeshAndPost
* builtin.object
 

Variables:
^^^^^^^^^^
 
* field
* scalar_auto_range
* scalar_color_bar
* scalar_component
* scalar_decimal_place
* scalar_log_base
* scalar_log_scale
* scalar_palette
* scalar_palette_filter
* scalar_palette_quality
* scalar_palette_steps
* scalar_range_max
* scalar_range_min
* scalar_variable
 
 
Inherited variables:
^^^^^^^^^^^^^^^^^^^^

* adaptivity_step
* solution_type
* time_step

.. _class-view:

class View
----------

Methods:
^^^^^^^^
 
* save_image
* zoom_best_fit
* zoom_in
* zoom_out
* zoom_region
 
 
Variables:
^^^^^^^^^^

* config = <agros2d.ViewConfig object>
* mesh = <agros2d.ViewMesh object>
* particle_tracing = <agros2d.ViewParticleTracing object>
* post2d = <agros2d.ViewPost2D object>
* post3d = <agros2d.ViewPost3D object>
  
