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
<div class="">
<!--<h3>{{APPLICATION_LOG}}</h3>-->
{{#ITEM}}
{{#ITEM_HEADING}}<br/><h2>{{ITEM_HEADING_MESSAGE}}</h2><hr/>{{/ITEM_HEADING}}
{{#ITEM_TEXT}}<span style="color: gray;">{{ITEM_TIME}}</span><span style="color: {{ITEM_COLOR}};"><strong>{{ITEM_MODULE}}</strong>: {{ITEM_MESSAGE}}</span><br/>{{/ITEM_TEXT}}
{{/ITEM}}
</div>
</body>
</html>
