#!/usr/bin/python

import os

def create_image(template, equation, fnimage):
    # replace equation
    latex = template.replace("equation", equation)
    
    # save file
    f = open("equation.tex", "w")
    f.write(latex)
    f.close()
    
    # run latex
    os.system("pdflatex -interaction=nonstopmode equation.tex")
    
    # crop image
    os.system("pdfcrop equation.pdf")
    
    # convert to png
    os.system("convert -transparent white -antialias -density 110 -quality 100 equation-crop.pdf " + fnimage)
    
    # rm files
    os.unlink("equation.aux")
    os.unlink("equation.log")
    os.unlink("equation.pdf")
    os.unlink("equation.tex")
    os.unlink("equation-crop.pdf")
    
# read latex template
f = open("template.tex", "r")
template = f.read()
print template
f.close()

# dir
dir = "./"

# current
create_image(template, r"-\, \div \left( \sigma\,\, \grad \varphi \right) = 0", dir + "current/current_steadystate.png")
create_image(template, r"\varphi = \varphi_0", dir + "current/current_potential.png")
create_image(template, r"J_\mathrm{n} = \sigma \frac{\partial \varphi}{\partial n_0} = J_0", dir + "current/current_inward_current_flow.png")

# electrostatic
create_image(template, r"-\, \div \left( \varepsilon\,\, \grad \varphi \right) = \rho", dir + "electrostatic/electrostatic_steadystate.png")
create_image(template, r"\varphi = \varphi_0", dir + "electrostatic/electrostatic_potential.png")
create_image(template, r"\sigma = D_\mathrm{n} = \varepsilon \frac{\partial \varphi}{\partial n_0} = \sigma_0", dir + "electrostatic/electrostatic_surface_charge_density.png")

# elasticity
create_image(template, r"-\, (\lambda + \mu)~\grad \div \vec{u} -\, \mu \triangle \vec{u} - \alpha_\mathrm{T} \left( 3 \lambda + 2 \mu \right) \grad T = \vec{f}", dir + "elasticity/elasticity_steadystate.png")
create_image(template, r"\lambda = \frac{\nu E}{(1 + \nu) (1 - 2 \nu)},\,\,\, \mu = \frac{E}{2(1 + \nu)}", dir + "elasticity/elasticity_coeffs.png")
create_image(template, r"f = f_0", dir + "elasticity/elasticity_free.png")
create_image(template, r"u = u_0", dir + "elasticity/elasticity_fixed.png")

# heat
create_image(template, r"-\, \div \left( \lambda\,\, \grad T \right) = Q", dir + "heat/heat_steadystate.png")
create_image(template, r"-\, \div \left( \lambda\,\, \grad T \right) + \rho c_\mathrm{p} \frac{\partial T}{\partial t} = Q", dir + "heat/heat_transient.png")
create_image(template, r"T = T_0", dir + "heat/heat_temperature.png")
create_image(template, r"f = - \lambda \frac{\partial T}{\partial n_0} = f_0 + \alpha \left ( T_\mathrm{ext} - T \right) + \epsilon \sigma \left ( T_\mathrm{amb}^4 - T^4 \right)", dir + "heat/heat_heat_flux.png")

# magnetic
create_image(template, r"\curl \left( \frac{1}{\mu}\, \left( \curl \vec{A} - \vec{B}_\mathrm{r} \right) \right) - \sigma \vec{v} \times \curl \vec{A} = \vec{J}_\mathrm{ext}", dir + "magnetic/magnetic_steadystate.png")
create_image(template, r"\curl \left( \frac{1}{\mu}\, \left( \curl \vec{A} - \vec{B}_\mathrm{r} \right) \right) - \sigma \vec{v} \times \curl \vec{A} + \sigma \frac{\partial \vec{A}}{\partial t} = \vec{J}_\mathrm{ext}", dir + "magnetic/magnetic_transient.png")
create_image(template, r"\curl \left( \frac{1}{\mu}\, \left( \curl \vecfaz{A} - \vec{B}_\mathrm{r} \right) \right) - \sigma \vec{v} \times \curl \vecfaz{A} + \mj \omega \sigma \vecfaz{A} = \vec{J}_\mathrm{ext}", dir + "magnetic/magnetic_harmonic.png")
create_image(template, r"A = A_0", dir + "magnetic/magnetic_potential.png")
create_image(template, r"K = - \frac{1}{\mu} \frac{\partial A}{\partial n_0} = K_0", dir + "magnetic/magnetic_surface_current.png")

# rf
create_image(template, r"\curl \left( \frac{1}{\mu}\, \curl \vecfaz{E} \right) - \mj \omega \left( \sigma + \mj \omega \varepsilon) \vecfaz{E} = \mj \omega \vecfaz{J}_\mathrm{ext}", dir + "rf_te/rf_te_harmonic.png")
create_image(template, r"J_{t} = - \frac{1}{\omega \mu} \frac{\partial E}{\partial n_0} = J_0", dir + "rf_te/rf_te_surface_current.png")
create_image(template, r"E = E_0", dir + "rf_te/rf_te_electric_field.png")
create_image(template, r" - \frac{1}{\omega \mu} \frac{\partial E}{\partial n_0} = \sqrt{\frac{\varepsilon - \mj \sigma / \omega}{\mu}} E", dir + "rf_te/rf_te_impedance.png")
#create_image(template, r" - \frac{1}{\omega \mu} \frac{\partial E}{\partial n_0} = \sqrt{\frac{\varepsilon - \mj \sigma / \omega}{\mu}} E", dir + "rf_te/rf_te_impedance.png")

# acoustic
create_image(template, r"-\, \div \left( \frac{1}{\rho}\,\, \grad p \right) - \frac{\omega^2}{\rho  c^2} \cdot p = 0", dir + "acoustic/acoustic_harmonic.png")
create_image(template, r"-\, \div \left( \frac{1}{\rho}\,\, \grad p \right) + \frac{1}{\rho  c^2} \frac{\partial^2 p}{\partial t^2} = 0", dir + "acoustic/acoustic_transient.png")
create_image(template, r"p = p_0", dir + "acoustic/acoustic_pressure.png")
create_image(template, r"a_\mathrm{n} = \frac{1}{\rho} \frac{\partial p}{\partial n_0} = a_0", dir + "acoustic/acoustic_normal_acceleration.png")
create_image(template, r"\frac{1}{\rho} \frac{\partial p}{\partial n_0} = - \mj \omega \frac{p}{Z_0}", dir + "acoustic/acoustic_impedance.png")
# create_image(template, r"\frac{1}{\rho} \frac{\partial p}{\partial n_0} = - \frac{1}{Z_0} \frac{\partial p}{\partial t}", dir + "acoustic/acoustic_impedance_transient.png")

# flow
create_image(template, r"\rho \vec{v} \cdot \grad \vec{v} = -\, \grad p + \mu \triangle \vec{v} + \vec{f},~\div \vec{v} = 0", dir + "flow/flow_steadystate.png")
create_image(template, r"\rho \left( \frac{\partial \vec{v}}{\partial t} + \vec{v} \cdot \grad \vec{v} \right) = -\, \grad p + \mu \triangle \vec{v} + \vec{f},~\div \vec{v} = 0", dir + "flow/flow_transient.png")

