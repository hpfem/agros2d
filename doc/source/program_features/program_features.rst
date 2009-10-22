.. highlight:: rest

Program Features
================

Startup Script
--------------

User Functions
--------------
You can write a functions that you can use in your scripts. If you write this functions to the file *functions.qs* in Agros2D root directory, then you will be able to use the other commands. You can also use :ref:`predefined-functions`.

Remote Control
--------------

.. index:: agros2d-remote

Agros2D support remote control. You can use *agros2d-remote* command for remote controling of Agros2D.

An example: ::

 agros2d-remote "openDocument("data/electrostatic_planar_capacitor.a2d");"
 agros2d-remote "solve();"
 agros2d-remote "result = pointResult(0.12, 0.32);"
 agros2d-remote "print(result.V);"

If you can run script file you must using *-script* switch.

An example: ::

 agros2d-remote -script "data/script/electrostatic_axisymmetric_capacitor.qs"

You can use commands described in the section :ref:`scripting`.

Report
------
