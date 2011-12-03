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
<h1>{{LABEL}}</h1>
<div class="section">
<table class="maintable">

</table>
</div>

<div class="section">
<table class="maintable">
	<tr>
		<td><b>Point</b></td><td>{{LABELX}}</td>
		<td>{{POINTX}}</td><td>{{POINT_UNIT}}</td>
	</tr>
	<tr>
		<td></td><td>{{LABELY}}</td>
		<td>{{POINTY}}</td><td>{{POINT_UNIT}}</td>
	</tr>

	{{#ITEM}}
	<tr>
		<td><b>{{NAME}}<b></td>
		<td>{{SHORTNAME}}<sub>{{PART}}</sub></td>
		<td>{{VALUE}}</td><td>{{UNIT}}</td>
	</tr>
	{{/ITEM}}
</table>
</div>
</body>
</html>
