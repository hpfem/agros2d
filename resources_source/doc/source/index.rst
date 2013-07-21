Introduction
============

Agros2D is a multiplatform C++ application for the solution of partial differential equations (PDE) based on the
`Hermes <http://hpfem.org/hermes>`_ library, developed by the `hpfem.org <http://hpfem.org>`_ group at the
`University of West Bohemia in Pilsen <http://www.zcu.cz>`_. Agros2D is distributed under the
`GNU General Public License <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>`_.

Supported physical fields
-------------------------
 
..  .. sidebar:: Quick start

.. If you want to start working with the program Agros2D, go to the page: :ref:`getting_started`.

* :ref:`acoustic` 
* :ref:`current`
* :ref:`elasticity`
* :ref:`electrostatic`
* :ref:`flow`
* :ref:`heat`
* :ref:`magnetic`
* :ref:`rf`

Capacity of the capacitor can be calculated according to formula

.. math::

    W_e = 4 \cdot \frac{1}{2} CU^2,

which can be adjusted to

.. math::

    C = \frac{4 \cdot 2 \cdot W_e}{U^2}.