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
<div class="section">
<h2>{{LABEL_DATE}} - {{DATE}}</h2>
<hr/>
<table class="maintable">
	{{#ITEM}}
	<tr>
		<td><b>{{CLS}}</b></td>
		<td>{{MODULE}}</td>
		<td><b>{{NAME}}</b></td>
		<td>{{TIME}}</td>
		<td><b>{{STATUS}}</b></td>
	</tr>
	{{#ITEM_ERROR}}
	<tr>
		<td><b>&nbsp;</b></td>	
		<td colspan="4"><span style="color: red; word-break: break-all;">{{ERROR}}</span></td>		
	</tr>
	{{/ITEM_ERROR}}	
	{{/ITEM}}
</table>
<hr/>
<h2>{{LABEL_TOTAL_TIME}}: {{TOTAL_TIME}}</h2>
</div>

{{#TEST_CHART_SECTION}}
<br/><br/>
<script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.js"></script>
<script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.js"></script>
<script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.axislabels.js"></script>	

<div class="section">
<h2>{{TEST_CHART_LABEL}}</h2>
<hr/>
<div style="text-align: center; width: 100%; height: 200px;"><br/><div id="chart_test" style="width: 95%; height: 200px;"></div></div>
{{TEST_CHART}}	
</div>
{{/TEST_CHART_SECTION}}

<div class="cleaner"></div>

</body>
</html>
