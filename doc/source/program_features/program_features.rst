.. highlight:: rest

Program Features
================

Import and Export of Geometry
--------------------------

You can import and export geometry in AutoCAD DXF format. This feature is available in the menu "File". This functionality is based on dxflib_.

User Functions
--------------
You can write your own functions to use in your scripts. If you write these functions to the file *functions.qs* in Agros2D root directory, then you will be able to use them in the same way as other commands. You can also use :ref:`predefined-functions`.

.. index:: agros2d-remote

Remote Control
--------------

Agros2D support remote controlling. You can use *agros2d-remote* command for remote controlling of Agros2D.

An example: ::

 agros2d-remote "openDocument("data/electrostatic_planar_capacitor.a2d");"
 agros2d-remote "solve();"
 agros2d-remote "result = pointResult(0.12, 0.32);"
 agros2d-remote "print(result.V);"

If you want to run a script file you must use *-script* switch.

An example: ::

 agros2d-remote -script "data/script/electrostatic_axisymmetric_capacitor.qs"

You can use commands described in the section :ref:`scripting`.

Report
------

You can use the automatic generation of reports about solution problems in Agros2D. This feature is available in the menu "Tools". Report is generated to HTML file.

.. _dxflib: http://www.ribbonsoft.com/dxflib.html

