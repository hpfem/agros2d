import agros2d as a2d
from math import sqrt

def transform():
    if ({{radDisplacement_checked}}):
      geometry.move_selection({{txtMoveX_text}}, {{txtMoveY_text}})
    elif ({{radRotation_checked}}):
      geometry.rotate_selection({{txtRotateX_text}}, {{txtRotateY_text}}, {{txtRotateAngle_text}})
    elif ({{radScale_checked}}):
      geometry.scale_selection({{txtScaleX_text}}, {{txtScaleY_text}}, {{txtScaleFactor_text}})

    geometry.select_none()
    a2d.view.zoom_best_fit()

def calculate():
    if ({{chkSolve_checked}}):
        problem.solve()

        if ('{{trvLocalValues_variable}}'):
            field = a2d.field('{{trvLocalValues_fieldid}}')
            result = field.local_values({{txtLocalValuesX_text}},
                                        {{txtLocalValuesY_text}})['{{trvLocalValues_variable}}']
            results['local'].append(result)

        if ('{{trvSurfaceIntefrals_variable}}'):
            field = a2d.field('{{trvSurfaceIntegrals_fieldid}}')
            result = field.surface_integrals({{lstSurfaceIntegralsEdges_list}})['{{trvSurfaceIntegrals_variable}}']
            results['surface'].append(result)

        if ('{{trvVolumeIntegrals_variable}}'):
            field = a2d.field('{{trvVolumeIntegrals_fieldid}}')
            result = field.volume_integrals({{lstSurfaceIntegralsEdges_list}})['{{trvVolumeIntegrals_variable}}']
            results['volume'].append(result)

    if ('{{txtExpression_text}}'):
        results['expression'].append({{txtExpression_text}})

def derivative(values, positions):
    values_derivative = list()
    derivative_positions = list()
    for i in range(len(values) - 1):
        step = positions[i+1]-positions[i]
        values_derivative.append((values[i]-values[i+1]) / step)
        derivative_positions.append(positions[i] + step/2.0)

    return values_derivative, derivative_positions

def eval_and_print(variable, value):
    globals()[variable] = value
    print('{0} = {1}'.format(variable, value))

results = {'transformation' : [0],
           'local' : list(), 'surface' : list(), 'volume' : list(),
           'expression' : list()}

problem = a2d.problem()
geometry = a2d.geometry

calculate()
for step in range({{txtSteps_text}}+1):
    if ({{lstNodes_list}}):
        geometry.select_nodes({{lstNodes_list}})
        transform()

    if ({{lstEdges_list}}):
        geometry.select_edges({{lstEdges_list}})
        transform()

    if ({{lstLabels_list}}):
        geometry.select_labels({{lstLabels_list}})
        transform()

    if ({{radDisplacement_checked}}):
        results['transformation'].append(results['transformation'][-1] + sqrt(({{txtMoveX_text}})**2 + ({{txtMoveY_text}})**2))
    elif ({{radRotation_checked}}):
      results['transformation'].append(results['transformation'][-1] + {{txtRotateAngle_text}})
    elif ({{radScale_checked}}):
      results['transformation'].append(results['transformation'][-1] + {{txtScaleFactor_text}})

    calculate()

if (results['local']):
  eval_and_print('{{trvLocalValues_variable}}', results['local'])

if (results['surface']):
  eval_and_print('{{trvSurfaceIntegrals_variable}}', results['surface'])

if (results['volume']):
  eval_and_print('{{trvVolumeIntegrals_variable}}', results['volume'])

eval_and_print('s', results['transformation'])

ds = []
if ({{chkLocalValueDerivative_checked}} and results['local']):
    values_derivative, ds = derivative(results['local'], results['transformation'])
    eval_and_print('d{{trvLocalValues_variable}}', values_derivative)
    
if ({{chkSurfaceIntegralDerivative_checked}} and results['surface']):
    values_derivative, ds = derivative(results['surface'], results['transformation'])
    eval_and_print('d{{trvSurfaceIntegrals_variable}}', values_derivative)

if ({{chkVolumeIntegralDerivative_checked}} and results['volume']):
    values_derivative, ds = derivative(results['volume'], results['transformation'])
    eval_and_print('d{{trvVolumeIntegrals_variable}}', values_derivative)

if ({{chkLocalValueDerivative_checked}} or
    {{chkSurfaceIntegralDerivative_checked}} or
    {{chkVolumeIntegralDerivative_checked}}):
        eval_and_print('ds', ds)

if (results['expression']):
  eval_and_print('expression_results', results['expression'])