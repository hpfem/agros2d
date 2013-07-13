import agros2d as a2d

nodes = {{lstNodes_list}}
edges = {{lstEdges_list}}
labels = {{lstLabels_list}}
steps = {{txtSteps_text}}

move_x = {{txtMoveX_text}}
move_y = {{txtMoveY_text}}

rotate_x = {{txtRotateX_text}}
rotate_y = {{txtRotateY_text}}
rotate_angle = {{txtRotateAngle_text}}

scale_x = {{txtScaleX_text}}
scale_y = {{txtScaleY_text}}
scale_factor = {{txtScaleFactor_text}}

local_values = list()
surface_integrals = list()
volume_integrals = list()
expression = list()

problem = a2d.problem()
geometry = a2d.geometry

def transform():
  if (move_x != 0.0 or move_y != 0.0):
    geometry.move_selection(move_x, move_y)

  if (rotate_angle != 0.0):
    geometry.rotate_selection(rotate_x, rotate_y, rotate_angle)

  if (scale_factor != 0.0):
    geometry.scale_selection(scale_x, scale_y, scale_factor)

  geometry.select_none()

def calculate():
  if ({{chkSolve_checked}}):
    problem.solve()

    if ('{{trvLocalValues_variable}}'):
      field = a2d.field('{{trvLocalValues_fieldid}}')
      local_values.append(field.local_values({{txtLocalValuesX_text}},
                                     {{txtLocalValuesY_text}})['{{trvLocalValues_variable}}'])

    if ('{{trvSurfaceIntefrals_variable}}'):
      field = a2d.field('{{trvSurfaceIntegrals_fieldid}}')
      surface_integrals.append(field.surface_integrals({{lstSurfaceIntegralsEdges_list}})['{{trvSurfaceIntegrals_variable}}'])

    if ('{{trvVolumeIntegrals_variable}}'):
      field = a2d.field('{{trvVolumeIntegrals_fieldid}}')
      volume_integrals.append(field.volume_integrals({{lstSurfaceIntegralsEdges_list}})['{{trvVolumeIntegrals_variable}}'])

  if ('{{txtExpression_text}}'):
    expression.append({{txtExpression_text}})

calculate()
for step in range(steps):
  if (nodes):
    geometry.select_nodes(nodes)
    transform()

  if (edges):
    geometry.select_edges(edges)
    transform()

  if (labels):
    geometry.select_labels(labels)
    transform()

  calculate()

if (local_values):
  print('{{trvLocalValues_variable}} = {0}'.format(local_values))

if (surface_integrals):
  print('{{trvSurfaceIntegrals_variable}} = {0}'.format(surface_integrals))

if (volume_integrals):
  print('{{trvVolumeIntegrals_variable}} = {0}'.format(volume_integrals))

if (expression):
  print('expression = {0}'.format(expression))
