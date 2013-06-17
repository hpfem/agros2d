.. _rf:

RF waves
========
Solving TE waves in RF field.

Maxwell Equations
^^^^^^^^^^^^^^^^^
Electric and magnetic phenomena at macroscopic level can be described by Maxwell's equations, that can be written in differential form as

.. math::
   
   \curl \vec{H} =  \vec{J} + \frac{\partial \vec D}{\partial t},  
     
   \curl \vec{E}  =  - \frac{\partial \vec B}{\partial t},
    
   \div \vec{D}  =  \rho,
   
   \div \vec{B}  =  0.

Assuming harmonic time dependance, the Maxwell equations can be rewritten into phasor form

.. math::
  
   \curl \vecphas{H}  =  \vecphas{J} +  \mj \omega \vecphas{D}, 
   
   \curl \vecphas{E}  =  - \mj \omega \vecphas{B},
   
   \div \vecphas{D}   =  \rho,
   
   \div \vecphas{B}   =  0.  


The Helmholtz Equation
^^^^^^^^^^^^^^^^^^^^^^
In a source-free, linear, isotropic, homogeneous area, harmonic electromagnetic field can be desribed by couple of Maxwell equations in phasor form

.. math::

   \curl \vecphas{H}  =  \mj \omega \vecphas{D}, 
   
   \curl \vecphas{E}  =  - \mj \omega \vecphas{B}.

If we apply operation $\curl$  on the first equation, we get

.. math::

   \curl \curl \vecphas{H} = \mj \omega \varepsilon \ \curl \vecphas{E},
   
so we can replace item $\curl \vecphas{E}$ by the right side    

.. math::
   \curl \left( \frac{1}{\mu}\, \curl \vecphas{E} \right) - \mj \omega \left( \sigma + \mj \omega \varepsilon \right) \vecphas{E} = \mj \omega \vecphas{J}_{\mathrm{ext}}.
   
Boundary conditions
^^^^^^^^^^^^^^^^^^^
* *Electric field* is the Dirichlet boundary condition in the form 

.. math::
   \vecphas{E} = \vecphas{E}_0
   
* *Magnetic field* is the Neumann boundary condition in the form 

.. math::
   \vec{n} \times H = n \times H_0
   
* *Surface current* is the Neumann boundary condition in the form

.. math::
    \phas{J}_{t} = - \frac{1}{\omega \mu} \frac{\partial \vecphas{E}}{\partial n_0} = \phas{J}_0

* *Impedance* is the Newton boundary condition in the form 

.. math::
    - \frac{1}{\omega \mu} \frac{\partial \vecphas{E}}{\partial n_0} = \sqrt{\frac{\varepsilon - \mj \sigma / \omega}{\mu}} \vecphas{E}          
 