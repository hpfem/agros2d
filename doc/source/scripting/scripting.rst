.. highlight:: rest
.. _scripting:

Scripting
=========

Scripting in Agros2D is based on QtScript_ (ECMAScript Programming Language).

Syntax
------

General Commands
----------------

.. index:: version()

* **version()**
   Print Agros2D version.

.. index:: quit()

* **quit()**
   Quit Agros2D.

.. index:: include()

* **include(** *filename* **)**
   Include and run script with filename.

An example: ::

 pathToScript = "data/scripts/";
 include(pathToScript +"electrostatic_axisymmetric_capacitor.qs");

.. index:: solve()

* **solve()**
   Solve problem.

.. index:: print()

* **print(** *string* **)**
   Print string to console.

* **printToFile(** *filename, string, mode* **)**
   Print string to file.

   - mode = {"write", "append"}

An example: ::

 printToFile("version.txt", "Agros2D version:", "write");
 printToFile("version.txt", version(), "append");

.. index:: input()

* **variable = input(** *string* **)**
   Show input box.

.. index:: message()

* **message(** *string* **)**
   Show message box.

.. index:: saveImage()

* **saveImage(** *filename* **)**
   Save image to file.

.. index:: zoomBestFit()

* **zoomBestFit()**
   Zoom best fit.

.. index:: zoomIn()

* **zoomIn()**
   Zoom in.

.. index:: zoomOut()

* **zoomOut()**
   Zoom out.

.. index:: zoomRegion()

* **zoomRegion(** *x1, y1, x2, y2* **)**
   Zoom region from [x1, y1] to [x2, y2].

Document Commands
-----------------

.. index:: newDocument()

* **newDocument(** *name, type, physicfield, numberofrefinements, polynomialorder, frequency, adaptivitytype, adaptivitysteps, adaptivitytolerance* **)**
   Create new document.

   - type = {"planar", "axisymmetric"}
   - numberofrefinements >= 0
   - polynomialorder = 1 to 10
   - physicfield = {"electrostatic", "magnetostatic", "current field", "heat transfer", "harmonic magnetic"}
   - adaptivitytype = {"disabled", "h-adaptivity", "p-adaptivity", "hp-adaptivity"}
   - frequency >= 0 (only for harmonic magnetic field)
   - adaptivitysteps > 0
   - adaptivitytolerance > 0

An example: ::

 newDocument("Electrostatic axisymmetric capacitor", "axisymmetric", "electrostatic", 1, 2, 0, "hp-adaptivity", 10, 1);

.. index:: openDocument()

* **openDocument(** *filename* )**
   Open document

.. index:: saveDocument()

* **saveDocument(** *filename* **)**
   Save document.

.. index:: mode()

* **mode(** *mode* **)**
   Set mode.

   - mode = {"node", "edge", "label", "postprocessor"}

Geometry Commands
-----------------

.. index:: addNode()

* **addNode(** *x, y* **)**
   Add new node with coordinates [x, y].

.. index:: addEdge()

* **addEdge(** *x1, y1, x2, y2, angle, marker* **)**
   Add new edge with from coordinates [x1, y1] to [x2, y2], angle and marker.

.. index:: addLabel()

* **addLabel(** *x, y, area, marker* )**
   Add new label with coordinates [x, y], area of triangle and marker.

.. index:: selectNone()

* **selectNone()**
   Unselect elements.

.. index:: selectAll()

* **selectAll()**
   Select all elements.

.. index:: selectNode()

* **selectNode(** *index, ...* **)**
   Select nodes with given index.

.. index:: selectEdge()

* **selectEdge(** *index, ...* **)**
   Select edges with given index.

.. index:: selectLabel()

* **selectLabel(** *index, ...* **)**
   Select labels with given index.

.. index:: selectNodePoint()

* **selectNodePoint(** *x, y* **)**
   Select the closest node with coordinates [x, y].

.. index:: selectEdgePoint()

* **selectEdgePoint(** *x, y* **)**
   Select the closest edge with coordinates [x, y].

.. index:: selectLabelPoint()

* **selectLabelPoint(** *x, y* **)**
   Select the closest label with coordinates [x, y].

.. index:: deleteSelection()

* **deleteSelection()**
   Delete selected objects.

.. index:: moveSelection()

* **moveSelection(** *dx, dy, copy* **)**
   Move selected objects.

   - copy = {True, False}

.. index:: rotateSelection()

* **rotateSelection(** *x, y, angle, copy* **)**
   Rotate selected objects.

   - copy = {True, False}

.. index:: scaleSelection()

* **scaleSelection(** *x, y, scale, copy* **)**
   Scale selected objects.

   - copy = {True, False}

Materials and Boundaries Commands
---------------------------------

.. index:: addBoundary()

* **addBoundary(** *name, type, value, ...* **)**
   Add boundary marker.

   - Electrostatic field
      addBoundary(name, type = {"electrostatic_potential", "electrostatic_surface_charge_density"}, value)
   - Electric current field
      addBoundary(name, type = {"current_potential", "current_inward_current_flow"}, value)
   - Magnetostatic field
      addBoundary(name, type = {"magnetostatic_vector_potential", "magnetostatic_surface_current_density"}, value)
   - Harmonic magnetic field
      addBoundary(name, type = {"harmonicmagnetic_vector_potential", "harmonicmagnetic_surface_current_density"}, value)
   - Heat transfer
      addBoundary(name, type = "heat_temperature", temperature)
      addBoundary(name, type = "heat_flux", heat_flux, h, external_temperature)

.. index:: addMaterial()

* **addMaterial(** *name, type, value, ...* **)**
   Add label marker.

   - Electrostatic field
      addMaterial(name, charge_density, permittivity)
   - Electric current field
      addMaterial(name, conductivity)
   - Magnetostatic field
      addMaterial(name, current_density, double permeability)
   - Harmonic magnetic field
      addMaterial(name, current_density_real, current_density_imag, double permeability, conductivity)
   - Heat transfer
      addMaterial(name, volume_heat, thermal_conductivity)

Postprocessor Commands
----------------------

.. index:: pointResult()

* **result = pointResult(** *x, y* **)**
   Local variables at point [x, y].

   - Electrostatic field
      Properties: X, Y, V, Ex, Ey, E, Dx, Dy, wj, epsr.
   - Magnetostatic field
      Properties: X, Y, A, Bx, By, B, Hx, Hy, H, wm, mur.
   - Electric current field
      Properties: X, Y, V, Jx, Jy, J, Ex, Ey, E, pj, gamma.
   - Harmonic magnetic field
      Properties: X, Y, A_real, A_imag, A, B, Bx_real, By_real, B_real, Bx_imag, By_imag, B_imag, H, Hx_real, Hy_real, H_real, Hx_imag, Hy_imag, H_imag, Ji_real, Ji_imag, Ji_real, J_real, J_imag, J_real, pj, wm, mur
   - Heat transfer
      Properties: X, Y, T, Gx, Gy, G, Fx, Fy, F, lambda.

An example: ::

 result = pointResult(0.1, 0.1);
 print("Potential = " + result.V);

.. index:: volumeIntegral()

* **integral = volumeIntegral(** *index, ...* **)**
   Volume integral in areas with given index.

   - Electrostatic field
      Properties: V, S, Ex_avg, Ey_avg, E_avg, Dx_avg, Dy_avg, D_avg, We.
   - Magnetostatic field
      Properties: V, S, Bx_avg, By_avg, B_avg, Hx_avg, Hy_avg, H_avg, Wm.
   - Electric current field
      Properties: V, S, Jx_avg, Jy_avg, J_avg, Ex_avg, Ey_avg, E_avg, Pj.
   - Harmonic magnetic field
      Properties: V, S, Ii_real, Ii_imag, It_real, It_imag, Pj, Fx_real, Fx_imag, Fy_real, Fy_imag, Wm.
   - Heat transfer
      Properties: V, S, T, Gx_avg, Gy_avg, G_avg, Fx_avg, Fy_avg, F_avg.

An example: ::

 integral = volumeIntegral(0.1, 0.1);
 print("Volume = " + integral.V);

.. index:: surfaceIntegral()

* **integral = surfaceIntegral(** *index, ...* **)**
   Surface integral in edges with given index.

   - Electrostatic field
      Properties: l, S, Q.
   - Electric current field
      Properties: l, S, I.
   - Magnetostatic field
      Properties: l, S.
   - Harmonic magnetic field
      Properties: l, S.
   - Heat transfer
      Properties: l, S, T_avg, T_diff, F.

An example: ::

 integral = surfaceIntegral(0.1, 0.1);
 print("Charge = " + integral.Q);

.. index:: showGrid()

* **showGrid(** *show* **)**
   Show grid.

    - show = {True, False}

.. index:: showGeometry()

* **showGeometry(** *show* **)**
   Show geometry.

    - show = {True, False}

.. index:: showInitialMesh()

* **showInitialMesh(** *show* **)**
   Show initial mesh.

    - show = {True, False}

.. index:: showSolutionMesh()

* **showSolutionMesh(** *show* **)**
   Show solution mesh.

    - show = {True, False}

.. index:: showContours()

* **showContours(** *show* **)**
   Show contours.

    - show = {True, False}

.. index:: showVectors()

* **showVectors(** *show* **)**
   Show vectrors.

    - show = {True, False}

.. index:: showScalar()

* **showScalar(** *type, variable, component, rangemin, rangemax* **)**
   Set view to type and show variable with component and in range (rangemin, rangemax).

    - type = {"none", "scalar", "scalar3d", "order"}

.. _`predefined-functions`:

Predefined User Functions
-------------------------

.. index:: addSemiCircle()

* **addSemiCircle(** *x0, y0, radius, marker* **)**
   Add semicircle with center coordinates [x0, y0], radius and marker.

.. index:: addCircle()

* **addCircle(** *x0, y0, radius, marker* **)**
   Add circle with center coordinates [x, y], radius and marker.

.. index:: addRect()

* **addRect(** *x0, y0, width, height, marker* **)**
   Add rect with starting coordinates [x0, y0], width, height and marker.

.. index:: sqr()

* **sqr(** *x* **)**
   Function returns the square of x.

.. index:: abs()

* **abs(** *x* **)**
   Function returns the absolute value of x.

.. index:: sin()

* **sin(** *x* **)**
   Function returns the sinus of x.

.. index:: cos()

* **cos(** *x* **)**
   Function returns the cosinus of x.

Examples
--------

Electrostatic Axisymmetric Capacitor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This script is detailed describe in example :ref:`electrostatic-axisymmetric-capacitor`.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.qs

Tips
----

You can run single commands and user functions. Select "Run command..." in the menu "Tools", or by pressing Alt+C and enter command, or user function.

.. _QtScript: http://doc.trolltech.com/latest/qtscript.html
