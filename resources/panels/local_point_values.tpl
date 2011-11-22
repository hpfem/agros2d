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
<h4>{{LABEL}}</h4>

<table>
	<tr><td>Point<td><td>{{LABELX}}<td><td>{{LABELY}}<td><td></td></tr>
	<tr><td><td><td>{{POINTX}}<td><td>{{POINTY}}<td><td></td></tr>
</table>
<table>
	{{#ITEM}}
	<tr><td>{{NAME}}</td><td>{{SHORTNAME}}<sub>{{PART}}</sub></td><td>{{VALUE}}</td><td>{{UNIT}}</td></tr>
	{{/ITEM}}
</table>
</body>
</html>
