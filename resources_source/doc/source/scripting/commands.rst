.. highlight:: python

.. _scripting:

Commands
========

Scripting in Agros2D is based on scripting language `Python <http://www.python.org>`_.

General Commands
----------------

.. index:: version()

* **version()**
   Print Agros2D version.

.. index:: print()

* **print(** *string* **)**
   Print string to console (Python command).

.. index:: message()

* **message(** *string* **)**
   Show message box.

.. index:: input()

* **variable = input(** *string* **)**
   Show input box.

.. index:: mode()

* **mode(** *mode* **)**
   Set mode.

   Key words that match a modes can be found in the :ref:`keyword-list`.

.. index:: zoombestfit()

* **zoombestfit()**
   Zoom best fit.

.. index:: zoomin()

* **zoomin()**
   Zoom in.

.. index:: zoomout()

* **zoomout()**
   Zoom out.

.. index:: zoomregion()

* **zoomregion(** *x1, y1, x2, y2* **)**
   Zoom region from [x1, y1] to [x2, y2].

.. index:: solve()

* **solve()**
   Solve problem.

.. index:: mesh()

* **mesh()**
   Mesh geometry.

.. index:: solutionfilename()

* **solutionfilename()**
   Print name of file for saving solution.

.. index:: meshfilename()

* **meshfilename()**
   Print name of file for saving mesh.

.. index:: saveimage()

* **saveimage(** *filename* **)**
   Save image to file.

.. index:: quit()

* **quit()**
   Quit Agros2D.

Document Commands
-----------------

.. index:: newdocument()

* **newdocument(** *name, type, physicfield, numberofrefinements, polynomialorder, adaptivitytype, adaptivitysteps, adaptivitytolerance, frequency, analysistype, timestep, totaltime, initialcondition* **)**
   Create new document.

   - numberofrefinements >= 0
   - polynomialorder = 1 to 10
   - adaptivitysteps >= 0
   - adaptivitytolerance >= 0
   - frequency >= 0 (only for harmonic magnetic field)
   - timestep > 0
   - totaltime > 0

   Key words that match a problem types, physic fields, adaptivity types and analysis types can be found in the :ref:`keyword-list`.

An example::

    newdocument("Electrostatic Axisymmetric Capacitor", "axisymmetric", "electrostatic", 0, 3, "disabled", 1, 1, 0, "steadystate", 0, 0, 0)
    newdocument("Heat Transfer Axisymmetric Actuator", "axisymmetric", "heat", 0, 1, "hp-adaptivity", 5, 15, 0, "transient", 500, 15e3, 20)

.. index:: opendocument()

* **opendocument(** *filename* **)**
   Open document.

.. index:: savedocument()

* **savedocument(** *filename* **)**
   Save document.

Geometry Commands
-----------------

.. index:: addnode()

* **addnode(** *x, y* **)**
   Add new node with coordinates [x, y].

.. index:: addedge()

* **addedge(** *x1, y1, x2, y2, angle = 0, marker = "none"* **)**
   Add new edge with from coordinates [x1, y1] to [x2, y2], angle and marker.

.. index:: addlabel()

* **addlabel(** *x, y, area = 0, marker = "none"* **)**
   Add new label with coordinates [x, y], area of triangle and marker.

.. index:: selectnone()

* **selectnone()**
   Unselect elements.

.. index:: selectall()

* **selectall()**
   Select all elements.

.. index:: selectnode()

* **selectnode(** *index, ...* **)**
   Select nodes with given index.

.. index:: selectedge()

* **selectedge(** *index, ...* **)**
   Select edges with given index.

.. index:: selectlabel()

* **selectlabel(** *index, ...* **)**
   Select labels with given index.

.. index:: selectnodepoint()

* **selectnodepoint(** *x, y* **)**
   Select the closest node with coordinates [x, y].

.. index:: selectedgepoint()

* **selectedgepoint(** *x, y* **)**
   Select the closest edge with coordinates [x, y].

.. index:: selectlabelpoint()

* **selectlabelpoint(** *x, y* **)**
   Select the closest label with coordinates [x, y].

.. index:: deleteselection()

* **deleteselection()**
   Delete selected objects.

.. index:: moveselection()

* **moveselection(** *dx, dy, copy = False* **)**
   Move selected objects.

   - copy = {True, False}

.. index:: rotateselection()

* **rotateselection(** *x, y, angle, copy = False* **)**
   Rotate selected objects.

   - copy = {True, False}

.. index:: scaleselection()

* **scaleselection(** *x, y, scale, copy = False* **)**
   Scale selected objects.

   - copy = {True, False}

Materials and Boundaries Commands
---------------------------------

.. index:: addboundary()

* **addboundary(** *name, type, value, ...* **)**
   Add boundary marker.

   - General field
      addboundary(name, type, value)
   - Electrostatic field
      addboundary(name, type, value)
   - Current field
      addboundary(name, type, value)
   - Magnetic field
      addboundary(name, type, value)
   - TE Waves
      addboundary(name, type, value_real, value_imag)
      addboundary(name, type, mode, power, phase)
      addboundary(name, type)
   - Heat transfer
      addboundary(name, type, temperature)
      addboundary(name, type, heat_flux, h, external_temperature)
   - Acoustic field
      addboundary(name, type, value)
      addboundary(name, type)
   - Structural mechanics
      addboundary(name, type_x, type_y, force_x, force_y, displacement_x, displacement_y)
   - Incompressible flow
      addboundary(name, type, velocity_x, velocity_y, pressure)

   Key words that match a boundary condition types can be found in the :ref:`keyword-list`.

.. index:: modifyboundary()

* **modifyboundary(** *name, type, value, ...* **)**
   Modify boundary marker with marker name. 

.. index:: addmaterial()

* **addmaterial(** *name, type, value, ...* **)**
   Add label marker.

   - General field
      addmaterial(name, rightside, constant)
   - Electrostatic field
      addmaterial(name, charge_density, permittivity)
   - Current field
      addmaterial(name, conductivity)
   - Magnetic field
      addmaterial(name, current_density_real, current_density_imag, permeability, conductivity, remanence, remanence_angle, velocity_x, velocity_y, velocity_angular)
   - TE Waves
      addmaterial(name, permittivity, permeability, conductivity, current_density_real, current_density_imag)
   - Heat transfer
      addmaterial(name, volume_heat, thermal_conductivity, density, specific_heat)
   - Acoustic field
      addmaterial(name, density, speed)
   - Structural mechanics
      addmaterial(name, young_modulus, poisson_ratio, force_x, force_y, alpha, temp, temp_ref)
   - Incompressible flow
      addmaterial(name, dynamic_viscosity, density)

.. index:: modifymaterial()

* **modifymaterial(** *name, type, value, ...* **)**
   Modify label marker with marker name. 

Postprocessor Commands
----------------------

.. index:: pointresult()

* **result = pointresult(** *x, y* **)**
   Local variables at point [x, y]. Key words that match a point results can be found in the :ref:`keyword-list`.

An example::

    result = pointresult(0.1, 0.1)
    print("Potential = " + str(result["V"]))

.. index:: volumeintegral()

* **result = volumeintegral(** *index, ...* **)**
   Volume integral in areas with given index. Key words that match a volume integrals can be found in the :ref:`keyword-list`.

An example::

    result = volumeintegral(0.1, 0.1)
    print("Volume = " + str(result["V"]))

.. index:: surfaceintegral()

* **result = surfaceintegral(** *index, ...* **)**
   Surface integral in edges with given index. Key words that match a surface integrals can be found in the :ref:`keyword-list`.

An example::

    result = surfaceintegral(0.1, 0.1)
    print("Charge = " + str(result["V"]))

.. index:: showgrid()

* **showgrid(** *show* **)**
   Show grid.

    - show = {True, False}

.. index:: showgeometry()

* **showgeometry(** *show* **)**
   Show geometry.

    - show = {True, False}

.. index:: showinitialmesh()

* **showinitialmesh(** *show* **)**
   Show initial mesh.

    - show = {True, False}

.. index:: showsolutionmesh()

* **showsolutionmesh(** *show* **)**
   Show solution mesh.

    - show = {True, False}

.. index:: showcontours()

* **showcontours(** *show* **)**
   Show contours.

    - show = {True, False}

.. index:: showvectors()

* **showvectors(** *show* **)**
   Show vectrors.

    - show = {True, False}

.. index:: showscalar()

* **showscalar(** *type, variable, component, rangemin, rangemax* **)**
   Set view to type and show variable with component and in range (rangemin, rangemax).

    - type = {"none", "scalar", "scalar3d", "order"}

   Key words that match a view types, physic field variables and components can be found in the :ref:`keyword-list`.

.. index:: timestep()

* **timestep(** *timeStep* **)**
   Set time level of transient problem.

Mathematical Operations
-----------------------

You can use full of basic mathematical operations in Python. When dividing you must use a float type in the the denominator. For example: x/2.0.

.. _`predefined-functions`:

Predefined User Functions
-------------------------

.. index:: addsemicircle()

* **addsemicircle(** *x0, y0, radius, marker = "none", label* **)**
   Add semicircle with center coordinates [x0, y0], radius, marker and label.

.. index:: addcircle()

* **addcircle(** *x0, y0, radius, marker = "none", label* **)**
   Add circle with center coordinates [x, y], radius, marker and label.

.. index:: addrect()

* **addrect(** *x0, y0, width, height, marker = "none", label* **)**
   Add rect with starting coordinates [x0, y0], width, height, marker and label.

Examples
--------

Electrostatic Axisymmetric Capacitor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This script is detailed describe in example :ref:`electrostatic-axisymmetric-capacitor`.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.py

Tips
----

You can run single commands and user functions. Select "Run command..." in the menu "Tools", or by pressing Alt+C and enter command, or user function.

You can use the full possibilities of Python while writing your scripts. For example, you can draw graphs, or perform other calculations.

You can be found all keywords of the command parameters in :ref:`keyword-list`.
