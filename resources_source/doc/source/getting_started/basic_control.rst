Basic Control
=============

Description of Agros2D
----------------------

Main window
^^^^^^^^^^^
When you start Agros2D, it opens to the *introductory mode* mode, where you can

* create new problem,

* perform basic settings fo the problem and add physical fields, 

* open example problem, 

* run PythonLab and work with scripts. 

.. image:: ./main_window.png
   :align: center
   :scale: 50%
   :alt: Main window
   

The important area of the Agros2D interface is *Mode selector*.   
     

Main window of Agros2D consists of several parts.

* Menu bar and toolbar

  Menu bar is the main control element, you can find all commands there. In toolbar the most frequently used commands are placed.

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

* Postprocessor properties panel

  Postprocessor properties panel is used for display results of solution.

* Local Values panel

  This panel displays calculated field quantity of local points.

* Volume Integral panel

  This panel displays calculated field quantity of volume integrals.

* Surface Integral panel

  This panel displays calculated field quantity of surface integrals.

* Terminal panel

  This panel is used to enter commands and display their output.

* Hints

  Hints displays short informations abouth actual modes or possible operations.

* Status bar

  Status bar is used to display information about the type of problem, solution of the physical field and cursor position in workspace.


PythonLab
^^^^^^^^^

The main part of the PythonLab is the editing area, which works with scripts. In the bottom of the window there is terminal panel. Script editor window also contains the file browser panel, menu bar and toolbar.

.. image:: ./script_editor.png
   :align: center
   :scale: 50%
   :alt: Script editor window

Chart window
^^^^^^^^^^^^

The main part of the chart window is the area to display the chart or data. You can switch among chart and data views through tabs at the top of the area. To enter the necessary data for the chart and to work with it controls on the right side of the window are used.

.. image:: ./chart.png
   :align: center
   :scale: 50%
   :alt: Chart window
