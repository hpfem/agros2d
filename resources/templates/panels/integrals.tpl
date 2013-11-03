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
<hr/>
<div class="section">
{{#FIELD}}
<h3>{{FIELDNAME}}</h3>
<table class="maintable">
	{{#ITEM}}
	<tr>
		<td class="description"><b>{{NAME}}</b></td>
		<td class="quantity">{{SHORTNAME}}</td>
		<td>{{VALUE}}</td>
		<td class="unit">{{UNIT}}</td>
	</tr>
	{{/ITEM}}
</table>
{{/FIELD}}
</div>
</body>
</html>
