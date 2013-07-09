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

  return {{txtExpression_text}}

results = list()
results.append(calculate())

for loop in range(steps):
  if (nodes):
    geometry.select_nodes(nodes)
    transform()

  if (edges):
    geometry.select_edges(edges)
    transform()

  if (labels):
    geometry.select_labels(labels)
    transform()

  results.append(calculate())

print(results)
