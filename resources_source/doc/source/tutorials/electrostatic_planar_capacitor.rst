Electrostatic Planar Capacitor
==============================

Introduction
------------

Capacitor is a passive electrical component, which consists of electrodes that are separated by a dielectric and have different electrical potential. It is used to store electric charge. The basic type of the capacitor is a plate capacitor.

For more information visit http://en.wikipedia.org/wiki/Capacitor.

Preprocessing
-------------

Select "New" in the menu "File" and enter problem properties.

* Name: Electrostatic planar capacitor
* Date: Today
* Problem type: Planar
* Physic field: Electrostatic
* Number of refinements: 1
* Polynomial order: 2
* Adaptivity: hp-adaptivity
* Adaptivity steps: 10
* Adaptivity tolerance: 5

.. image:: ./editing_problem.png

Fig. 1.: Editing of a problem

Geometry
^^^^^^^^

* Nodes

  Select "Operate on nodes" in the menu "Problem" and place 6 points with coordinates [0.1, 0.0], [0.2, 0.0], [0.0, 0.2], [0.0, 0.3], [0.1, 0.2] and [0.2, 0.3]. You can move mouse to requested place and press left button together with Ctrl key, or by pressing Alt+N and entering the node coordinates.

* Edges

  Select "Operate on edges" in the menu "Problem" and draw 6 edges according to the definition area. Press Ctrl key and select start and end node of the line, or press Alt+E and select these points from a list.

* Labels

  Select "Operate on labels" in menu "Problem" and place block label into an inner area. To place label you can use left mouse button together with Ctrl key or pressing Alt+L and entering the label point coordinates.

.. image:: ./editing_geometry.png

Fig. 2.: Editing of a geometry

Boundary conditions
^^^^^^^^^^^^^^^^^^^

Select "New boundary condition" from the menu "Problem", or press Alt+B and gradually define three boundary conditions.

* Name: Source electrode
* BC type: Fixed voltage
* Value: 30

* Name: Ground electrode
* BC type: Fixed voltage
* Value: 0

* Name: Neumann BC
* BC type: Surface charge density
* Value: 0

Materials
^^^^^^^^^

Select "New material" from the menu "Problem", or press Alt+M and define dielectric material embedded between the electrodes.

* Name: Dieletric
* Permitivity: 10
* Charge density (C/m3): 0 

Processing
----------

Mesh
^^^^

Select "Mesh area" from the menu "Problem".

Solve
^^^^^

Select "Solve problem" from the menu "Problem", or press Alt+S.

Postprocessing
--------------

Calculation of capacity
^^^^^^^^^^^^^^^^^^^^^^^

Select "Volume integrals" in menu "File" and mark area "Dielectric". In panel "Volume integral" find value of "Energy" in group "Electrostatic field".

.. image:: ./calculation_of_energy.png

Fig. 3.: Calculation of energy

Capacity of the capacitor can be calculated according to formula

.. math::

    W_e = 4 \cdot \frac{1}{2} CU^2,

which can be adjusted to

.. math::

    C = \frac{4 \cdot 2 \cdot W_e}{U^2}.

Chart of electric potential
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Select "Chart" in menu "Tools". This opens a window for plotting charts. As a starting coordinates enter point [0.1, 0.2] and as ending coordinates point [0.2, 0.3]. Select "X" in "Horizontal axis" and select "Plot".

.. image:: ./chart_plotting.png

Fig. 4.: Chart plotting
