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

<img style="float: right; margin-right: 10px; margin-top: 12px;" src="{{AGROS2D}}" /> 
<h1>{{NAME}}</h1>

<div class="section">
<h2>State</h2>
<hr/>
<table>
	<tr><td><b>Solved</b></td><td>{{SOLVED}}</td></td>
</table>
</div>

<div class="section">
<h2>{{PARAMETER_LABEL}}</h2>
<hr/>
<table>
{{#PARAM_SECTION}}
	<tr><td><b>{{PARAM_LABEL}}</b></td><td>{{PARAM_VALUE}} {{PARAM_UNIT}}</td></td>
{{/PARAM_SECTION}}
</table>
</div>

<div class="section">
<h2>{{VARIABLE_LABEL}}</h2>
<hr/>
<table>
{{#VAR_VALUE_SECTION}}
	<tr>
		<td><b>{{VAR_LABEL}}</b></td>
		<td>{{VAR_VALUE}} {{VAR_UNIT}}</td>
	</tr>		
{{/VAR_VALUE_SECTION}}
</table>
{{#VAR_CHART_SECTION}}
<div id="{{VAR_CHART_DIV}}" style="width:95%;height:150px;"></div>{{VAR_CHART}}
{{/VAR_CHART_SECTION}}
</div>

<div class="section">
<h2>{{INFO_LABEL}}</h2>
<hr/>
<table>
{{#INFO_SECTION}}
	<tr><td><b>{{INFO_LABEL}}</b></td><td>{{INFO_VALUE}}</td></td>
{{/INFO_SECTION}}
</table>
</div>

<div class="cleaner"></div>

</body>
</html>
