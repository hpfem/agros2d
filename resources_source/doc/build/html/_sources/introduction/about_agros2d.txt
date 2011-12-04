About Agros2D
=============

.. include:: ../basic_information.rst

Key Features
------------

*hp*-FEM
^^^^^^^^

Ilustrative example (:ref:`electrostatic-axisymmetric-sparkgap`):

.. image:: ./hp-fem/mesh-h(p=1).png
   :align: center
   :scale: 50%
   :alt: h-adaptivity (p=1)

.. image:: ./hp-fem/mesh-h(p=2).png
   :align: center
   :scale: 50%
   :alt: h-adaptivity (p=1)

.. image:: ./hp-fem/mesh-hp.png
   :align: center
   :scale: 50%
   :alt: hp-adaptivity

Fig.: Initial (orange) and solution (red) mesh (*h*-adaptivity (p = 1), *h*-adaptivity (p = 2) and *hp*-adaptivity)

.. image:: ./hp-fem/polynomial_order-p.png
   :align: center
   :scale: 50%
   :alt: p-adaptivity

.. image:: ./hp-fem/polynomial_order-hp.png
   :align: center
   :scale: 50%
   :alt: hp-adaptivity

Fig.: Polynomial order (*p*-adaptivity and *hp*-adaptivity)

.. image:: ./hp-fem/convergence.png
   :align: center
   :scale: 50%
   :alt: Convergence

.. image:: ./hp-fem/error.png
   :align: center
   :scale: 50%
   :alt: Error

.. image:: ./hp-fem/dofs.png
   :align: center
   :scale: 50%
   :alt: DOFs

Fig.: Adaptivity charts

Curvilinear elements
^^^^^^^^^^^^^^^^^^^^

.. image:: ./curvilinear_elements/elements.png
   :align: center
   :scale: 30%
   :alt: Curvilinear and normal elements at the same geometry

Fig: Curvilinear and normal elements at the same ilustrative geometry

Ilustrative example ():

.. image:: ./curvilinear_elements/mesh-normal_elements.png
   :align: center
   :scale: 50%
   :alt: Normal elements mesh

.. image:: ./curvilinear_elements/mesh-curvilinear_elements.png
   :align: center
   :scale: 50%
   :alt: Curvilinear elements mesh

Fig.: Normal (left) and curvilinear (right) meshes

.. image:: ./curvilinear_elements/convergence.png
   :align: center
   :scale: 50%
   :alt: Convergence

.. image:: ./curvilinear_elements/error.png
   :align: center
   :scale: 50%
   :alt: Error

.. image:: ./curvilinear_elements/dofs.png
   :align: center
   :scale: 50%
   :alt: DOFs

Fig.: Adaptivity charts (*hp*-adaptivity)

Triangular or quadrilateral elements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Ilustrative example (:ref:`acoustic-axisymmetric-horn`):

.. image:: ./triangular_or_quadrilateral_elements/mesh-quads.png
   :align: center
   :scale: 50%
   :alt: Normal elements mesh

.. image:: ./triangular_or_quadrilateral_elements/mesh-triangles.png
   :align: center
   :scale: 50%
   :alt: Curvilinear elements mesh

Fig.: Quadrilateral (left) and triangular (right) elements mesh

.. image:: ./triangular_or_quadrilateral_elements/polynomial_order-quads.png
   :align: center
   :scale: 50%
   :alt: Quadrilateral elements

.. image:: ./triangular_or_quadrilateral_elements/polynomial_order-triangles.png
   :align: center
   :scale: 50%
   :alt: Triangular elements

Fig.: Polynomial order on quadrilateral elements mesh (left) and triangular elements mesh (right)

.. image:: ./triangular_or_quadrilateral_elements/convergence.png
   :align: center
   :scale: 50%
   :alt: Convergence

.. image:: ./triangular_or_quadrilateral_elements/error.png
   :align: center
   :scale: 50%
   :alt: Error

.. image:: ./triangular_or_quadrilateral_elements/dofs.png
   :align: center
   :scale: 50%
   :alt: DOFs

Fig.: Adaptivity charts (*hp*-adaptivity)
