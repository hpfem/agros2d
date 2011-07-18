Basic Control
=============

Description of Agros2D
----------------------

Main window
^^^^^^^^^^^

Main window consists of several parts.

* Menu bar and toolbar

  Menu bar is the main control element; you can find all commands there. In toolbar the most frequently used commands are placed.

* Workspace

  Workspace is used for actual work with the model (geometry editing, viewing of the results, etc.). You can use the context menu, which is accessible by double left click in the Workspace area, to select commands from list.

* Problem panel

  Problem panel displays information about the problem. Here you can use the context menu to perform certain commands. It consists of a tree.

  - Problem

    Here are contained basic information about the problem and its solutions.

  - Boundary conditions

    This part contains a list of added boundary conditions.

  - Materials

    This part contains a list of added materials.

  - Geometry

    Information about elements of geometry that are added to the workspace is contained here.

* Local Values panel

  This panel displays calculated field quantity of local points.

* Volume Integral panel

  This panel displays calculated field quantity of volume integrals.

* Surface Integral panel

  This panel displays calculated field quantity of surface integrals.

* Terminal panel

  This panel is used to enter commands and display their output.

* Status bar

  Status bar is used to display information about the type of problem, solution of the physical field and cursor position in workspace.

.. image:: ./main_window.png

Fig. 1.: Main window

Script editor window
^^^^^^^^^^^^^^^^^^^^

The main part of the script editor window is editing area, which works with scripts. In the bottom of the window there is terminal panel. Script editor window also contains the file browser panel, menu bar and toolbar.

.. image:: ./script_editor_window.png

Fig. 2.: Script editor window

Chart window
^^^^^^^^^^^^

The main part of the chart window is the area to display the chart or data. You can switch among chart and data views through tabs at the top of the area. To enter the necessary data for the chart and to work with it controls on the right side of the window are used.

.. image:: ./chart_window.png

Fig. 3.: Chart window

Shortcut Keys
-------------

Basic shortcuts in main window
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

  Add node using coordinates.

* **Alt+E**

  Add edge using coordinates.

* **Alt+L**

  Add label using coordinates.

* **Alt+B**

  Add boundary condition.

* **Alt+M**

  Add material.

* **Alt+S**

  Solve problem.

Basic shortcuts in script editor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* **Ctrl+R**

  Run script.

* **Ctrl+M**

  Create script from model.

* **Ctrl+U**

  Uncoment selected code.

* **Alt+G**

  Go to line.

Advanced shortcuts
^^^^^^^^^^^^^^^^^^

* **Ctrl+left mouse**

  Add nodes, edges and labels directly by left mouse click.

* **Ctrl+Shift+N**

  Add nodes by dialog window using actual cursor position.

* **Space**

  Open dialog for setting markers (only with selected edges or labels).

* **Shift+left mouse**

  Pan the workspace (you can also use middle mouse button or arrows keys).

Tips
----

Both the main window and the script editor window can be customized. For adjustments, use context menu of the menu bar or toolbar (also label of each panel) and choose which parts you want to view.

You can use rulers in workspace after switching them on in program properties.
