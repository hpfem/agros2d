<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
</head>
<body>
<h1>{{BASIC_INFORMATION_LABEL}}</h1>
<div class="section">
<table class="maintable">
	<tr>
		<td><b>{{NAME_LABEL}}</b></td><td>{{NAME}}</td>
	</tr>
	<tr>
		<td><b>{{COORDINATE_TYPE_LABEL}}</b></td><td>{{COORDINATE_TYPE}}</td>
	</tr>
	<tr>
		<td><b>{{MESH_TYPE_LABEL}}</b></td><td>{{MESH_TYPE}}</td>
	</tr>
	<tr>
		<td><b>{{FREQUENCY_LABEL}}</b></td><td>{{FREQUENCY}}</td>
	</tr>
	<tr>
		<td><b>{{TIME_STEP_LABEL}}</b></td><td>{{TIME_STEP}}</td>
	</tr>
	<tr>
		<td><b>{{TIME_TOTAL_LABEL}}</b></td><td>{{TIME_TOTAL}}</td>
	</tr>
</table>
</div>

{{#FIELD}}
<div class="section">
<h2>{{PHYSICAL_FIELD}}</h2>
<table class="maintable">
	<tr>
		<td><b>{{ANALYSIS_TYPE_LABEL}}</b></td><td>{{ANALYSIS_TYPE}}</td>
	</tr>
	<tr>
		<td><b>{{WEAK_FORMS_TYPE_LABEL}}</b></td><td>{{WEAK_FORMS_TYPE}}</td>
	</tr>
	{{#INITIAL_CONDITION_SECTION}}
	<tr>
		<td><b>{{INITIAL_CONDITION_LABEL}}</b></td><td>{{INITIAL_CONDITION}}</td>
	</tr>
	{{/INITIAL_CONDITION_SECTION}}
	<tr>
		<td><b>{{LINEARITY_TYPE_LABEL}}</b></td><td>{{LINEARITY_TYPE}}</td>
	</tr>
	{{#SOLVER_PARAMETERS_SECTION}}
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{NONLINEAR_STEPS_LABEL}}</b></td><td>{{NONLINEAR_STEPS}}</td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{NONLINEAR_TOLERANCE_LABEL}}</b></td><td>{{NONLINEAR_TOLERANCE}}</td>
	</tr>		
	{{/SOLVER_PARAMETERS_SECTION}}
	<tr>
		<td><b>{{ADAPTIVITY_TYPE_LABEL}}</b></td><td>{{ADAPTIVITY_TYPE}}</td>
	</tr>
	{{#ADAPTIVITY_PARAMETERS_SECTION}}
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{ADAPTIVITY_STEPS_LABEL}}</b></td><td>{{ADAPTIVITY_STEPS}}</td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{ADAPTIVITY_TOLERANCE_LABEL}}</b></td><td>{{ADAPTIVITY_TOLERANCE}}</td>
	</tr>
	{{/ADAPTIVITY_PARAMETERS_SECTION}}
	<tr>
		<td><b>{{REFINEMENS_NUMBER_LABEL}}</b></td><td>{{REFINEMENS_NUMBER}}</td>
	</tr>
	<tr>
		<td><b>{{POLYNOMIAL_ORDER_LABEL}}</b></td><td>{{POLYNOMIAL_ORDER}}</td>
	</tr>	
</table>

<table class="maintable">
    {{#MESH_PARAMETERS_SECTION}}
    <tr><td colspan=2><h3>{{MESH_LABEL}}</h3></td></tr>
    <tr>
        <td><b>{{INITIAL_MESH_LABEL}}</b></td><td>{{INITIAL_MESH_NODES}}</td>
    </tr>
    <tr>
        <td>&nbsp;</td><td>{{INITIAL_MESH_ELEMENTS}}</td>
    </tr>
    {{#MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    <tr>
        <td><b>{{SOLUTION_MESH_LABEL}}</b></td><td>{{SOLUTION_MESH_NODES}}</td>
    </tr>
    <tr>
        <td>&nbsp;</td><td>{{SOLUTION_MESH_ELEMENTS}}</td>
    </tr>
    {{/MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    {{#MESH_SOLUTION_DOFS_PARAMETERS_SECTION}}
    <tr>
        <td><b>{{DOFS_LABEL}}</b></td><td>{{DOFS}}</td>
    </tr>
    {{#MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    <tr>
        <td><b>{{ERROR_LABEL}}</b></td><td>{{ERROR}}</td>
    </tr>
    {{/MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    {{/MESH_SOLUTION_DOFS_PARAMETERS_SECTION}}
    {{/MESH_PARAMETERS_SECTION}}
</table>

</div>
{{/FIELD}}

<table class="maintable">
    {{#SOLUTION_PARAMETERS_SECTION}}
    <tr><td colspan=2><h2>{{SOLUTION_LABEL}}</h2></td></tr>
    <tr>
        <td><b>{{SOLUTION_ELAPSED_TIME_LABEL}}</b></td><td>{{SOLUTION_ELAPSED_TIME}}</td>
    </tr>
    <tr>
        <td><b>{{NUM_THREADS_LABEL}}</b></td><td>{{NUM_THREADS}}</td>
    </tr>
    {{/SOLUTION_PARAMETERS_SECTION}}
</table>
</body>
</html>

<!--
{{#SOLUTION_SECTION}}
<h1>{{SOLUTION_INFORMATION_LABEL}}</h1>
<div class="section">
<table class="maintable">
	<tr><td colspan=2><h2>{{INITIAL_MESH_LABEL}}</h2></td></tr>
	<tr><td colspan=2><div class="subsection">
		<table>
			<tr><td><b>{{INITIAL_MESH_NODES_LABEL}}</b></td><td>{{INITIAL_MESH_NODES}}</td></tr>
			<tr><td><b>{{INITIAL_MESH_ELEMENTS_LABEL}}</b></td><td>{{INITIAL_MESH_ELEMENTS}}</td></tr>
		</table>
	</div></td></tr>
	{{#SOLUTION_PARAMETERS_SECTION}}
	<tr><td><b>{{ELAPSED_TIME_LABEL}}</b></td><td>{{ELAPSED_TIME}}</td></tr>
	<tr><td><b>{{DOFS_LABEL}}</b></td><td>{{DOFS}}</td></tr>
	{{#ADAPTIVITY_SECTION}}
	<tr><td colspan=2><h2>{{ADAPTIVITY_LABEL}}</h2></td></tr>
	<tr><td colspan=2><div class="subsection">
		<table>
			<tr><td><b>{{ADAPTIVITY_ERROR_LABEL}}</b></td><td>{{ADAPTIVITY_ERROR}}</td></tr>
			<tr><td><b>{{ADAPTIVITY_TOLERANCE_LABEL}}</b></td><td>{{ADAPTIVITY_TOLERANCE}}</td></tr>
		</table>
	</div></td></tr>
	<tr><td colspan=2><h2>{{SOLUTION_MESH_LABEL}}</h2></td></tr>
	<tr><td colspan=2><div class="subsection">
		<table>
			<tr><td><b>{{SOLUTION_MESH_NODES_LABEL}}</b></td><td>{{SOLUTION_MESH_NODES}}</td></tr>
			<tr><td><b>{{SOLUTION_MESH_ELEMENTS_LABEL}}</b></td><td>{{SOLUTION_MESH_ELEMENTS}}</td></tr>
		</table>
	</div></td></tr>
	{{/ADAPTIVITY_SECTION}}
	{{/SOLUTION_PARAMETERS_SECTION}}
</table>
</div>
{{/SOLUTION_SECTION}}
-->
