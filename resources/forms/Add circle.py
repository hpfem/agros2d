import agros2d as a2d

x0 = {{txtCenterX_text}}
y0 = {{txtCenterY_text}}
radius = {{txtRadius_text}}
angle = {{cmbAngle_text}}

if (angle == 360):
  a2d.geometry.add_circle(x0, y0, radius)
elif (angle == 180):
  a2d.geometry.add_semicircle(x0, y0, radius)
elif (angle == 90):
  a2d.geometry.add_edge(x0, y0, x0+radius, y0)
  a2d.geometry.add_edge(x0+radius, y0, x0, y0+radius, angle=90)
  a2d.geometry.add_edge(x0, y0+radius, x0, y0)
