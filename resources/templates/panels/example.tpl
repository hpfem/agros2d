<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
	<link rel="stylesheet" href="problem_style.tpl" type="text/css" />	
</head>
<body>

<table>
<tr>
<td>
<div class="section">
<h2>{{NAME}}</h2>
<table>
<tr>
<td>
	<table>
	<tr>
		<td colspan="2">{{DESCRIPTION}}</td>
	</tr>		
	<tr>
	<td>&nbsp;</td>	
	</tr>
	
	<tr>
	<td><b>{{COORDINATE_TYPE_LABEL}}</b></td><td>{{COORDINATE_TYPE}}</td>
	</tr>
	{{#HARMONIC}}
	<tr>
    <td colspan="2">
        <h3>{{HARMONIC_LABEL}}</h3>
        <table>
            <tr><td><b>{{HARMONIC_FREQUENCY_LABEL}}</b></td><td>{{HARMONIC_FREQUENCY}}</td></tr>
        </table>
    </td>
	</tr>
	{{/HARMONIC}}
    {{#TRANSIENT}}
    <tr>
    <td colspan="2">
        <h3>{{TRANSIENT_LABEL}}</h3>
        <table>
            <tr><td><b>{{TRANSIENT_STEP_METHOD_LABEL}}</b></td><td>{{TRANSIENT_STEP_METHOD}}</td></tr>
            <tr><td><b>{{TRANSIENT_STEP_ORDER_LABEL}}</b></td><td>{{TRANSIENT_STEP_ORDER}}</td></tr>
            <tr><td><b>{{TRANSIENT_TOLERANCE_LABEL}}</b></td><td>{{TRANSIENT_TOLERANCE}}</td></tr>
            <tr><td><b>{{TRANSIENT_CONSTANT_NUM_STEPS_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_NUM_STEPS}}</td></tr>
            <tr><td><b>{{TRANSIENT_CONSTANT_STEP_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_STEP}}</td></tr>
            <tr><td><b>{{TRANSIENT_TOTAL_LABEL}}</b></td><td>{{TRANSIENT_TOTAL}}</td></tr>
        </table>
    </td>
    </tr>
    {{/TRANSIENT}}
	<td>
		<h3>{{GEOMETRY_LABEL}}</h3>
		<table>
			<tr><td><b>{{GEOMETRY_NODES_LABEL}}</b></td><td>{{GEOMETRY_NODES}}</td></tr>
			<tr><td><b>{{GEOMETRY_EDGES_LABEL}}</b></td><td>{{GEOMETRY_EDGES}}</td></tr>
			<tr><td><b>{{GEOMETRY_LABELS_LABEL}}</b></td><td>{{GEOMETRY_LABELS}}</td></tr>
		</table>
	</td>
	</td>
	</tr>
	</table>
</td>
<td>
	<div class="figure">{{GEOMETRY_SVG}}</div>
</td>
</tr>
</table>
</div>

{{#FIELD}}
<table>
<tr>
{{#FIELD_SECTION}}
<td>
<div class="section">
<h2>{{PHYSICAL_FIELD_LABEL}}</h2>
<table>
	<tr>
		<td><b>{{ANALYSIS_TYPE_LABEL}}</b></td><td>{{ANALYSIS_TYPE}}</td>
	</tr>
	<tr>
		<td><b>{{LINEARITY_TYPE_LABEL}}</b></td><td>{{LINEARITY_TYPE}}</td>
	</tr>	
</table>
</div>
</td>
{{/FIELD_SECTION}}
</tr>
</table>
{{/FIELD}}
	
{{#IMAGE_SECTION}}
<table>
	<tr>
		<td><img width="100%" src="{{EXAMPLE_IMAGE}}" /></td>
	</tr>
</table>
</div>
{{/IMAGE_SECTION}}

<div class="cleaner"></div>
