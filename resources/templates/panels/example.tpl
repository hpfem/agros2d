<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
	<link rel="stylesheet" href="problem_style.tpl" type="text/css" />
	<script type="text/javascript" src="{{JS_DIRECTORY}}/js/MathJax.js?config=TeX-AMS_HTML"> 
		MathJax.Hub.Config({
			jax: ["input/TeX","output/HTML-CSS"],
			displayAlign: "left", 
			showMathMenu: false,
			showProcessingMessages: false,
			messageStyle: "none"
		});
	</script>
</head>
<body>

<!-- <img style="float: right; margin-right: 10px; margin-top: 12px;" src="{{AGROS2D}}" /> -->
<h1>{{NAME}}</h1>
<div>
<table class="heading">
<tr>
	<td><b>{{COORDINATE_TYPE_LABEL}}</b></td><td>{{COORDINATE_TYPE}}</td>
</tr>
<tr>
	<td><b>{{MESH_TYPE_LABEL}}</b></td><td>{{MESH_TYPE}}</td>
</tr>
</table>
<p>{{DESCRIPTION}}</p>
</div>

<table>
<tr>
<td>
<div class="section">
<h2>{{GEOMETRY_LABEL}}</h2>
<hr/>
<table>
	<tr><td rowspan="6"><div class="figure">{{GEOMETRY_SVG}}</div></td><td><b>{{GEOMETRY_NODES_LABEL}}</b></td><td>{{GEOMETRY_NODES}}</td></tr>
	<tr><td><b>{{GEOMETRY_EDGES_LABEL}}</b></td><td>{{GEOMETRY_EDGES}}</td></tr>
	<tr><td><b>{{GEOMETRY_LABELS_LABEL}}</b></td><td>{{GEOMETRY_LABELS}}</td></tr>
	<tr><td><b>&nbsp;</b></td><td>&nbsp;</td></tr>
	<tr><td><b>{{GEOMETRY_MATERIALS_LABEL}}</b></td><td>{{GEOMETRY_MATERIALS}}</td></tr>
	<tr><td><b>{{GEOMETRY_BOUNDARIES_LABEL}}</b></td><td>{{GEOMETRY_BOUNDARIES}}</td></tr>
</table>
{{#HARMONIC}}
<h2>{{HARMONIC_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{HARMONIC_FREQUENCY_LABEL}}</b></td><td>{{HARMONIC_FREQUENCY}}</td></tr>
</table>
{{/HARMONIC}}
{{#TRANSIENT}}
<h2>{{TRANSIENT_LABEL}}</h2>
<hr/>
<table>
    <tr><td><b>{{TRANSIENT_STEP_METHOD_LABEL}}</b></td><td>{{TRANSIENT_STEP_METHOD}}</td></tr>
    <tr><td><b>{{TRANSIENT_STEP_ORDER_LABEL}}</b></td><td>{{TRANSIENT_STEP_ORDER}}</td></tr>
    <tr><td><b>{{TRANSIENT_TOLERANCE_LABELS}}</b></td><td>{{TRANSIENT_TOLERANCE}}</td></tr>
    <tr><td><b>{{TRANSIENT_CONSTANT_NUM_STEPS_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_NUM_STEPS}}</td></tr>
    <tr><td><b>{{TRANSIENT_CONSTANT_STEP_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_STEP}}</td></tr>
    <tr><td><b>{{TRANSIENT_TOTAL_LABEL}}</b></td><td>{{TRANSIENT_TOTAL}}</td></tr>
</table>
{{/TRANSIENT}}
</div>

<div class="section">
{{#COUPLING}}
<h2>{{COUPLING_MAIN_LABEL}}</h2>
<hr/>
{{#COUPLING_SECTION}}
<table>
	<tr><td><b>{{COUPLING_LABEL}}</b></td><td>&nbsp;</td>
	<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<b>{{COUPLING_SOURCE_LABEL}}</b></td><td>{{COUPLING_SOURCE}}</td></tr>
	<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<b>{{COUPLING_TARGET_LABEL}}</b></td><td>{{COUPLING_TARGET}}</td></tr>
	<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<b>{{COUPLING_TYPE_LABEL}}</b></td><td>{{COUPLING_TYPE}}</td></tr>
</table>
{{/COUPLING_SECTION}}
</div>
{{/COUPLING}}

<td>
{{#FIELD}}
{{#FIELD_SECTION}}
<div class="section">
<h2>{{PHYSICAL_FIELD_LABEL}}</h2>
<hr/>
<table>
	<tr>
		<td><b>{{ANALYSIS_TYPE_LABEL}}</b></td><td>{{ANALYSIS_TYPE}}</td>
	</tr>
	<tr>
		<td><b>{{LINEARITY_TYPE_LABEL}}</b></td><td>{{LINEARITY_TYPE}}</td>
	</tr>	
</table>
</div>
{{/FIELD_SECTION}}
{{/FIELD}}
</td>
</tr>
</table>
	
{{#IMAGE_SECTION}}
<table>
	<tr>
		<td><img width="100%" src="{{EXAMPLE_IMAGE}}" /></td>
	</tr>
</table>
</div>
{{/IMAGE_SECTION}}

{{PROBLEM_DETAILS}}

<div class="cleaner"></div>
