Basic Control
=============

Description of Program Windows
------------------------------

Main window
^^^^^^^^^^^

Main window consists of several parts.

* Menubar and toolbar

  Menubar is the main control element. Here you can find all commands. In Toolbar is placed most frequently used commands.

* Workspace

  Workspace is used to work on the model (geometry editing, view the results, etc.). Here you can use the context menu to perform certain commands.

* Problem panel

  Problem panel display information about problem. Here you can use the context menu to perform certain commands. It consists of a tree.

  - Problem

    Here are contained basic information about the problem and its solutions.

  - Boundary conditions

    This part contains a list of added boundary conditions.

  - Materials

    This part contains a list of added materials.

  - Geometry

    Here is contained informations about elements of geometry that are added to the workspace.

* Local Values panel

  This panel display calculated field quantity of local points.

* Volume Integral panel

  This panel display calculated field quantity of volume integrals.

* Surface Integral panel

  This panel display calculated field quantity of surface integrals.

* Statusbar

  Statusbar is used to display important information. Here are displayed information about the type of problem, solution physical field and cursor position in workspace.

.. image:: ./main_window.png

Fig. 1.: Main window

Script editor window
^^^^^^^^^^^^^^^^^^^^

The main part of the Script editor window is editing area, which used to work with scripts. In the bottom of the window is located area for printing script warnings and messages. Script editor window also contain the menubar and toolbar.

.. image:: ./script_editor_window.png

Fig. 2.: Script editor window

Chart window
^^^^^^^^^^^^

The main part of the Chart window is the area to display the chart or data. you can switch among chart and data views through tabs at the top of this area. To enter the necessary data for the chart and work with him is used controls on the right side of the window.

.. image:: ./chart_window.png

Fig. 3.: Chart window

Shortcut Keys
-------------

Basic shortcuts
^^^^^^^^^^^^^^^

* **F4**

  Open script editor.

* **F5**

  Change mode to Operate on nodes.

* **F6**

  Change mode to Operate on edges.

* **F7**

  Change mode to Operate on labels.

* **F8**

  Change mode to Postprocessor.

* **Alt+N**

  Add node use coordinates.

* **Alt+E**

  Add edge use coordinates.

* **Alt+L**

  Add label use coordinates.

* **Alt+B**

  Add boundary condition.

* **Alt+M**

  Add material.

* **Alt+S**

  Solve problem.

Advanced shortcuts
^^^^^^^^^^^^^^^^^^

* **Ctrl+Left mouse button**

  Add elements of geometry use selected (with mouse) coordinates.

* **Ctrl+Shift+N**

  Add elements of geometry use enter selected (with mouse) coordinates.

* **Space**

  Open dialog for set markers (only with selected edges or labels).

* **Shift+Left mouse button**

  Move with workspace (also is possible to use Middle mouse button).

Tips
----

The main window and the Script Editor window you can customize to their needs and habits. For these adjustments, use context menu on the menubar and toolbar (also labels of each panels) and choose which parts you want to view.

You can used rulers in wokrspace. You must turn this rulers in program properties.
