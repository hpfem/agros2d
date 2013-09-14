<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.axislabels.js"></script>
</head>
<body>

<h1>{{NAME}}</h1>
<div>
<p>{{DESCRIPTION}}</p>
</div>
<br />

<table width="100%">
{{#PROPERTIES_SECTION}}
<tr>
<td>
<div class="section">
<h2>{{PROPERTY_LABEL}}</h2>
<hr/>
<table>
	<p>{{PROPERTY_SOURCE}}</p>
	<tr>
		<td><b>{{PROPERTY_SHORTNAME_LABEL}}</b></td><td>{{PROPERTY_SHORTNAME}}</td>
		{{#PROPERTY_NONLINEAR}}<td rowspan="7"><div id="{{PROPERTY_CHART}}" style="width:400px;height:155px;"></div></td>{{/PROPERTY_NONLINEAR}}
	</tr>
	<tr><td><b>{{PROPERTY_UNIT_LABEL}}</b></td><td>{{PROPERTY_UNIT}}</td></tr>
	<tr><td><b>&nbsp;</b></td><td>&nbsp;</td></tr>
	<tr><td><b>{{PROPERTY_CONSTANT_LABEL}}</b></td><td>{{PROPERTY_CONSTANT}} {{PROPERTY_UNIT}}</td></tr>
	{{#PROPERTY_NONLINEAR}}
	<tr><td><b>&nbsp;</b></td><td>&nbsp;</td></tr>
	<tr><td><b>{{PROPERTY_INDEPENDENT_SHORTNAME_LABEL}}</b></td><td>{{PROPERTY_INDEPENDENT_SHORTNAME}}</td></tr>
	<tr><td><b>{{PROPERTY_INDEPENDENT_UNIT_LABEL}}</b></td><td>{{PROPERTY_INDEPENDENT_UNIT}}</td></tr>
	{{/PROPERTY_NONLINEAR}}
</table>
{{#PROPERTY_SELECTABLE}}
<a href="property?id={{PROPERTY_ID}}&amp;constant={{PROPERTY_CONSTANT}}&amp;x={{PROPERTY_X}}&amp;y={{PROPERTY_Y}}" style="float: right;"><button>Select property</button></a>
{{/PROPERTY_SELECTABLE}}
</div>
</td>
</tr>
{{PROPERTY_CHART_SCRIPT}}
{{/PROPERTIES_SECTION}}
</table>


{{PROBLEM_DETAILS}}

<div class="cleaner"></div>
</body>
</html>
