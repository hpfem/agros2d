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
    os.system("pdfcrop --margins '0 6 0 6' --clip equation.pdf")
    
    # convert to png
    os.system("convert -transparent white -antialias -density 110 -quality 100 equation-crop.pdf " + fnimage)
    
    # rm files
    os.unlink("equation.aux")
    os.unlink("equation.log")
    os.unlink("equation.pdf")
    os.unlink("equation.tex")
    os.unlink("equation-crop.pdf")
    
# read latex template
f = open("{{LATEX_TEMPLATE}}", "r")
template = f.read()
f.close()

{{#EQUATION_SECTION}}create_image(template, r"{{EQUATION}}", "{{OUTPUT_DIRECTORY}}{{ID}}_equation_{{NAME}}.png")
{{/EQUATION_SECTION}}
