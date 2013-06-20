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
   :label: m_r_1
   
   \curl \vecphas{H}  =  \vecphas{J} + \mj \omega \vecphas{D}, 
      
.. math::
   :label: m_r_2    
   
   \curl \vecphas{E}  =  - \mj \omega \vecphas{B}.
   
    
If we apply operation $\curl$  on the equation :eq:`m_r_2`, we get

.. math::

   \curl \curl \vecphas{E} = - \mj \omega \ \curl  \left( \mu \vecphas{H} \right),
   
In many radio-frequency applications the permeability can be supposed as constant, so we can move $\mu$ in front of $\curl$ operator and then
replace item $\curl \vecphas{H}$ by the right side of equation :eq:`m_r_1`. We get he the second order partial differential equation in the form     

.. math::
   \curl \curl \vecphas{E} = -\mj \omega \mu \left( \vecphas{J}  + \mj \omega \vecphas{D} \right),
   
or after few manipulations in the form

.. math::
   \curl \curl \vecphas{E} = -\mj \omega \mu \vecphas{J}  +  \omega^2 \mu \varepsilon \vecphas{E},

It can be useful to formaly divide current density $\vecphas{J}$ to induced current density $\vecphas{J}_\mathrm{i} = \gamma \vecphas{E}$ and external current density
$\vecphas{J}_\mathrm{ext}$, then equation can be changed into 

.. math::
   :label: vector_helmholtz
   
   \curl \curl \vecphas{E}  + \left(\mj \omega \mu \gamma -  \omega^2 \mu \varepsilon \right) \vecphas{E} = - \mj \omega \mu \vecphas{J}_\mathrm{ext}.
   
Agros2D allows solving of two dimensional problems in Cartesian and cylindrical coordinates. In Cartesian coordinates there are two situation where it is possible to reduce dimension of problems  from 3D  to 2D:

* vectors of electric field are perpendicular to $x-y$ the plane and does not depend on the $z$ axis,

* vectors of electric field lie in the plane $x-y$ and does not depend on the $z$ axis.   

In the first case we can reduce vector equation :eq:`vector_helmholtz` into the Helmholtz equation for the component $\phas{E}_z$ in the form

.. math::
   
   \Delta \phas{E}_z + \left(\omega^2 \mu \varepsilon - \mj \omega \mu \gamma \right) \phas{E}_z = \mj \omega \mu \phas{J}_{z,\mathrm{ext}}.


By the similar procedure it is possible to derive the Helmholtz equation for the component $\phas{H}$ of magnetic field in the form

.. math::

   

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
 