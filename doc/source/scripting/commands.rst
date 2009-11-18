.. highlight:: python

Commands
========

Scripting in Agros2D is based on scripting language Python_.

General Commands
----------------

.. index:: version()

* **version()**
   Print Agros2D version.

.. index:: print

* **print** *string*
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

   - mode = {"node", "edge", "label", "postprocessor"}

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

   - type = {"planar", "axisymmetric"}
   - physicfield = {"electrostatic", "magnetostatic", "current field", "heat transfer", "harmonic magnetic"}
   - numberofrefinements >= 0
   - polynomialorder = 1 to 10
   - adaptivitytype = {"disabled", "h-adaptivity", "p-adaptivity", "hp-adaptivity"}
   - adaptivitysteps >= 0
   - adaptivitytolerance >= 0
   - frequency >= 0 (only for harmonic magnetic field)
   - analysistype = {"steadystate", "transient"}
   - timestep > 0
   - totaltime > 0

An example: ::

 newdocument("Electrostatic axisymmetric capacitor", "axisymmetric", "electrostatic", 1, 2, "hp-adaptivity", 10, 1)
 newdocument("Transient", "planar", "heat", 0, 1, "disabled", 5, 1, 0, "transient", 700, 3000, 20)

.. index:: opendocument()

* **opendocument(** *filename* )**
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

* **addlabel(** *x, y, area = 0, marker = "none"* )**
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

* **moveselection(** *dx, dy, copy* **)**
   Move selected objects.

   - copy = {True, False}

.. index:: rotateselection()

* **rotateselection(** *x, y, angle, copy* **)**
   Rotate selected objects.

   - copy = {True, False}

.. index:: scaleselection()

* **scaleselection(** *x, y, scale, copy* **)**
   Scale selected objects.

   - copy = {True, False}

Materials and Boundaries Commands
---------------------------------

.. index:: addboundary()

* **addboundary(** *name, type, value, ...* **)**
   Add boundary marker.

   - General field
      addboundary(name, type = {"general_value", "general_derivative"}, value)
   - Electrostatic field
      addboundary(name, type = {"electrostatic_potential", "electrostatic_surface_charge_density"}, value)
   - Electric current field
      addboundary(name, type = {"current_potential", "current_inward_current_flow"}, value)
   - Magnetostatic field
      addboundary(name, type = {"magnetostatic_vector_potential", "magnetostatic_surface_current_density"}, value)
   - Harmonic magnetic field
      addboundary(name, type = {"harmonicmagnetic_vector_potential", "harmonicmagnetic_surface_current_density"}, value)
   - Heat transfer
      addboundary(name, type = {"heat_temperature"}, temperature)
      addboundary(name, type = {"heat_flux"}, heat_flux, h, external_temperature)

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
   - Electric current field
      addmaterial(name, conductivity)
   - Magnetostatic field
      addmaterial(name, current_density, permeability, density, specificheat)
   - Harmonic magnetic field
      addmaterial(name, current_density_real, current_density_imag, permeability, conductivity)
   - Heat transfer
      addmaterial(name, volume_heat, thermal_conductivity)

.. index:: modifymaterial()

* **modifymaterial(** *name, type, value, ...* **)**
   Modify label marker with marker name. 

Postprocessor Commands
----------------------

.. index:: pointresult()

* **result = pointresult(** *x, y* **)**
   Local variables at point [x, y].

   - General field
       Properties: X, Y, V, Gx, Gy, G, constant.
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

 result = pointresult(0.1, 0.1)
 print "Potential = ", result.V

.. index:: volumeintegral()

* **integral = volumeintegral(** *index, ...* **)**
   Volume integral in areas with given index.

   - General field
       Properties: V, S.
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

 integral = volumeintegral(0.1, 0.1)
 print "Volume = ", integral.V

.. index:: surfaceintegral()

* **integral = surfaceintegral(** *index, ...* **)**
   Surface integral in edges with given index.

   - General field
       Properties: l, S.
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

 integral = surfaceintegral(0.1, 0.1);
 print "Charge = ", integral.Q

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

.. index:: settimestep()

* **settimestep(** *timeStep* **)**
   Set time level of transient problem.

.. _`predefined-functions`:

Predefined User Functions
-------------------------

.. index:: addsemicircle()

* **addsemicircle(** *x0, y0, radius, marker* **)**
   Add semicircle with center coordinates [x0, y0], radius and marker.

.. index:: addcircle()

* **addcircle(** *x0, y0, radius, marker* **)**
   Add circle with center coordinates [x, y], radius and marker.

.. index:: addrect()

* **addrect(** *x0, y0, width, height, marker* **)**
   Add rect with starting coordinates [x0, y0], width, height and marker.

Examples
--------

Electrostatic Axisymmetric Capacitor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This script is detailed describe in example :ref:`electrostatic-axisymmetric-capacitor`.

.. literalinclude:: ./electrostatic_axisymmetric_capacitor.py

Tips
----

You can run single commands and user functions. Select "Run command..." in the menu "Tools", or by pressing Alt+C and enter command, or user function.

You can be found all keywords of the command parameters in :ref:`keyword-list`.

.. _Python: http://www.python.org
