Introduction
============

Agros2D is a multiplatform C++ application for the solution of partial differential equations (PDE) based on the
`Hermes <http://hpfem.org/hermes>`_ library, developed by the `hpfem.org <http://hpfem.org>`_ group at the
`University of West Bohemia in Pilsen <http://www.zcu.cz>`_. Agros2D is distributed under the
`GNU General Public License <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>`_.

Supported physical fields
-------------------------
.. sidebar:: Quick start

  If you want to start working with the program Agros2D, go to the page: :ref:`getting_started`.

* :ref:`acoustic` 
* :ref:`current`
* :ref:`elasticity`
* :ref:`electrostatic`
* :ref:`flow`
* :ref:`heat`
* :ref:`magnetic`
* :ref:`rf`

Key Features
------------

Coupled physical fields
^^^^^^^^^^^^^^^^^^^^^^^
Agros2D supports coupling between physical fields, which means that it is possible to include several physical fields into one problem. There are two ways how 
couple fields. There are two ways to couple physical fields: 

*  Weak coupling - Fields are basically solved independently, but results of solution of one physical field are used as a source for another field.   
*  Hard coupling - Physical fields influence each other, no particular field can be solved separately. 

Agros2D allows using of both kinds of coupling. 

Nonlinear Problems
^^^^^^^^^^^^^^^^^^
Agros2D supports non-linear problems solving. At the moment Newton's and Picard's methods are implemented.

Space and time adaptivity
^^^^^^^^^^^^^^^^^^^^^^^^^
The space and time adaptivity is one of unique features of the Hermes library and Agros2D.   

Ilustrative example (:ref:`electrostatic-axisymmetric-sparkgap`):


.. figure:: ./introduction/hp-fem/mesh-h(p=1).png 

    :align: left                                   
    :width: 150      
    :figwidth: 30%                                
    :figclass: three_pictures
    :alt: h-adaptivity (p=1)    
    
    
    Fig. 1: *h*-adaptivity (p = 1)                        


.. figure:: ./introduction/hp-fem/mesh-h(p=2).png
    :align: center
    :width: 150
    :figwidth: 30%
    :figclass: three_pictures
    :alt: h-adaptivity (p=2)
    
    Fig. 2: *h*-adaptivity (p = 2)


.. figure:: ./introduction/hp-fem/mesh-hp.png
    :align: right    
    :width: 150
    :figwidth: 30%
    :figclass: three_pictures
    :alt: hp-adaptivity
   
    Fig. 3: *hp*-adaptivity

 
.. figure:: ./introduction/hp-fem/polynomial_order-p.png
   :align: left
   :width: 150
   :figwidth: 30%   
   :figclass: three_pictures
   :alt: p-adaptivity

   Fig. 4: *p*-adaptivity   
   

.. figure:: ./introduction/hp-fem/polynomial_order-hp.png
   :align: center
   :width: 150
   :figwidth: 30%   
   :figclass: three_pictures
   :alt: hp-adaptivity
   
   Fig. 5: *hp*-adaptivity 


.. figure:: ./introduction/hp-fem/convergence.png
   :align: left
   :width: 400
   :figwidth: 45%   
   :figclass: two-pictures
   :alt: Convergence

   Fig. 6: Polynomial order (*p*-adaptivity and *hp*-adaptivity)


.. figure:: ./introduction/hp-fem/error.png
   :align: right
   :width: 400
   :figwidth: 45%   
   :figclass: two-pictures
   :alt: Error
   
   Fig. 7: Adaptivity charts

.. figure:: ./introduction/hp-fem/dofs.png
   :align: left
   :width: 400
   :alt: DOFs

   Fig. 8: Adaptivity charts

Curvilinear elements
^^^^^^^^^^^^^^^^^^^^

.. image:: ./introduction/curvilinear_elements/elements.png
   :align: center
   :scale: 30%
   :alt: Curvilinear and normal elements at the same geometry

Fig: Curvilinear and normal elements at the same ilustrative geometry

.. figure:: ./introduction/curvilinear_elements/mesh-normal_elements.png
   :align: left
   :width: 400
   :figwidth: 45%   
   :figclass: two-pictures
   :alt: Normal elements mesh
   

.. figure:: ./introduction/curvilinear_elements/mesh-curvilinear_elements.png
   :align: center
   :width: 400
   :figwidth: 45%   
   :figclass: two-pictures
   :alt: Curvilinear elements mesh

   Fig.: Normal (left) and curvilinear (right) meshes


.. figure:: ./introduction/curvilinear_elements/convergence.png
   :align: center
   :figwidth: 45%   
   :figclass: two-pictures
   :alt: Convergence

.. figure:: ./introduction/curvilinear_elements/error.png
   :align: center
   :scale: 50%
   :alt: Error


.. figure:: ./introduction/curvilinear_elements/dofs.png
   :align: center
   :scale: 50%
   :alt: DOFs

Fig.: Adaptivity charts (*hp*-adaptivity)

Triangular or quadrilateral elements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Ilustrative example (:ref:`acoustic-axisymmetric-horn`):

.. figure:: ./introduction/triangular_or_quadrilateral_elements/mesh-quads.png
   :align: center
   :scale: 50%
   :alt: Normal elements mesh


.. figure:: ./introductio/triangular_or_quadrilateral_elements/mesh-triangles.png
   :align: center
   :scale: 50%
   :alt: Curvilinear elements mesh

Fig.: Quadrilateral (left) and triangular (right) elements mesh

.. figure:: ./introduction/triangular_or_quadrilateral_elements/polynomial_order-quads.png
   :align: center
   :scale: 50%
   :alt: Quadrilateral elements


.. figure:: ./introduction/triangular_or_quadrilateral_elements/polynomial_order-triangles.png
   :align: center
   :scale: 50%
   :alt: Triangular elements

Fig.: Polynomial order on quadrilateral elements mesh (left) and triangular elements mesh (right)

.. figure:: ./introduction/triangular_or_quadrilateral_elements/convergence.png
   :align: center
   :scale: 50%
   :alt: Convergence


.. figure:: ./introduction/triangular_or_quadrilateral_elements/error.png
   :align: center
   :scale: 50%
   :alt: Error


.. figure:: ./introduction/triangular_or_quadrilateral_elements/dofs.png
   :align: center
   :scale: 50%
   :alt: DOFs

Fig.: Adaptivity charts (*hp*-adaptivity)

