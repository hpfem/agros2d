<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}

		/**
		 * GitHub theme
		 *
		 * @author Craig Campbell
		 * @version 1.0.4
		 */
		pre {
		    border: 1px solid #ccc;
		    word-wrap: break-word;
		    padding: 6px 10px;
		    line-height: 19px;
		    margin-bottom: 20px;
		}

		code {
		    border: 1px solid #eaeaea;
		    margin: 0px 2px;
		    padding: 0px 5px;
		}

		pre code {
		    border: 0px;
		    padding: 0px;
		    margin: 0px;
		    -moz-border-radius: 0px;
		    -webkit-border-radius: 0px;
		    border-radius: 0px;
		}

		pre, code {
		    font-family: Consolas, 'Liberation Mono', Courier, monospace;
		    color: #333;
		    background: #f8f8f8;
		    -moz-border-radius: 3px;
		    -webkit-border-radius: 3px;
		    border-radius: 3px;
		}

		pre, pre code {
		    font-size: 12px;
		}

		pre .comment {
		    color: #998;
		}

		pre .support {
		    color: #0086B3;
		}

		pre .tag, pre .tag-name {
		    color: navy;
		}

		pre .keyword, pre .css-property, pre .vendor-prefix, pre .sass, pre .class, pre .id, pre .css-value, pre .entity.function, pre .storage.function {
		    font-weight: bold;
		}

		pre .css-property, pre .css-value, pre .vendor-prefix, pre .support.namespace {
		    color: #333;
		}

		pre .constant.numeric, pre .keyword.unit, pre .hex-color {
		    font-weight: normal;
		    color: #099;
		}

		pre .entity.class {
		    color: #458;
		}

		pre .entity.id, pre .entity.function {
		    color: #900;
		}

		pre .attribute, pre .variable {
		    color: teal;
		}

		pre .string, pre .support.value  {
		    font-weight: normal;
		    color: #d14;
		}

		pre .regexp {
		    color: #009926;
		}		
	</style>
	<script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/rainbow-python.js"></script>
</head>
<body>


<img style="float: right; margin-right: 10px; margin-top: 12px;" src="{{AGROS2D}}" /> 
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

{{PROBLEM_DETAILS}}

<h2>Script</h2>
<pre><code data-language="python">{{PROBLEM_PYTHON}}</code></pre>

<div class="cleaner"></div>

</body>
</html>
