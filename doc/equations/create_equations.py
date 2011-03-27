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
    os.unlink("equation-crop.pdf")
    
# read latex template
f = open("template.tex", "r")
template = f.read()
print template
f.close()

# dir
dir = "../../src/images/equations/"

create_image(template, r"-\, \div \left( \sigma\,\, \grad \varphi \right) = 0", dir + "current_steadystate.png")
create_image(template, r"-\, \div \left( \varepsilon\,\, \grad \varphi \right) = \rho", dir + "electrostatic_steadystate.png")
create_image(template, r"-\, (\lambda + \mu)~\grad \div \vec{u} -\, \mu \triangle \vec{u} = \vec{f}", dir + "elasticity_steadystate.png")
create_image(template, r"-\, \div \left( \lambda\,\, \grad T \right) = Q", dir + "heat_steadystate.png")
create_image(template, r"-\, \div \left( \lambda\,\, \grad T \right) + \rho c_\mathrm{p} \frac{\partial T}{\partial t} = Q", dir + "heat_transient.png")
create_image(template, r"-\, \div \left( c\,\, \grad u \right) = r", dir + "general_steadystate.png")
create_image(template, r"\curl \left( \frac{1}{\mu}\, \left( \curl \vec{A} - \vec{B}_\mathrm{r} \right) \right) - \sigma \vec{v} \times \curl \vec{A} = \vec{J}_\mathrm{ext}", dir + "magnetic_steadystate.png")
create_image(template, r"\curl \left( \frac{1}{\mu}\, \left( \curl \vec{A} - \vec{B}_\mathrm{r} \right) \right) - \sigma \vec{v} \times \curl \vec{A} + \sigma \frac{\partial \vec{A}}{\partial t} = \vec{J}_\mathrm{ext}", dir + "magnetic_transient.png")
create_image(template, r"\curl \left( \frac{1}{\mu}\, \left( \curl \vecfaz{A} - \vec{B}_\mathrm{r} \right) \right) - \sigma \vec{v} \times \curl \vecfaz{A} + \mj \omega \sigma \vecfaz{A} = \vec{J}_\mathrm{ext}", dir + "magnetic_harmonic.png")
create_image(template, r"\curl \left( \frac{1}{\mu}\, \curl \vecfaz{E} \right) - \mj \omega \left( \sigma + \mj \omega \varepsilon) \vecfaz{E} = \mj \omega \vecfaz{J}_\mathrm{ext}", dir + "rf_harmonic.png")
create_image(template, r"-\, \div \left( \frac{1}{\rho}\,\, \grad p \right) - \frac{\omega^2}{\rho  c^2} \cdot p = 0", dir + "acoustic_harmonic.png")
create_image(template, r"\rho \vec{v} \cdot \grad \vec{v} = -\, \grad p + \mu \triangle \vec{v} + \vec{f},~\div \vec{v} = 0", dir + "flow_steadystate.png")
create_image(template, r"\rho \left( \frac{\partial \vec{v}}{\partial t} + \vec{v} \cdot \grad \vec{v} \right) = -\, \grad p + \mu \triangle \vec{v} + \vec{f},~\div \vec{v} = 0", dir + "flow_transient.png")
















