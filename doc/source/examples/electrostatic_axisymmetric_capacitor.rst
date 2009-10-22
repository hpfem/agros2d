.. highlight:: rest
.. _electrostatic-axisymmetric-capacitor:

Electrostatic Axisymmetric Capacitor
====================================

This example demonstrates the basics of scripting in Agros2D. The resulting script can be found if the file */data/scripts/electrostatic_axisymmetric_capacitor.qs*.

Introduction
------------

A special type of a capacitor is a cylindrical capacitor. Cylindrical capacitor has electrodes in the form of the concentric cylinders.

For more information visit http://en.wikipedia.org/wiki/Capacitor.

Preprocessing
-------------

Select "Script editor" in the "Tools" menu and create new solution by command newDocument in form.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
   :lines: 2

In the script it is usefull to use local variables.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
   :lines: 5-13

Boundary conditions
^^^^^^^^^^^^^^^^^^^

At first we define boundary conditions. They can be assigned to individual edges in the geometry. Adding boundary conditions is carried out with the command addBoundary.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
   :lines: 16-18

Materials
^^^^^^^^^

Next we define materials and then we assign them to individual areas (labels) in the geometry. Adding material is carried out with the command addMaterial.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
   :lines: 21-23

Geometry
^^^^^^^^

* Edges

   Use edges to create the desired geometry. Adding edges is carried out with the command addEdge.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
   :lines: 26-38

* Labels

  Use labels to define block labels. Adding bloct labels is carried out with the command addLabel.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
   :lines: 41-43

.. image:: ./editing_script.png

Fig 1.: Editing script

Processing
----------

In this example, we will investigate dependence the capacity on the distance of electrodes. For this calculation we will use "do - while" cycle. Type the following code.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
   :lines: 49-63

To run the script select "Run" in the menu "Tools", or or by pressing Ctrl+R.

Postprocessing
--------------

After finishing of the script you will get dependence the capacity on the distance of electrodes.

.. image:: ./calculation_of_capacity.png

Fig 2.: Calculation of the capacity

Complete script
---------------

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs
