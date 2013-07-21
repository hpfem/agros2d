<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}

		div.example {
		  float: left;
		  width: 96px;
		  height: 116px;
		  vertical-align: bottom;
		  text-align: center;
		  padding-left: 8px;
		  padding-right: 8px;
		  padding-top: 8px;
		  padding-bottom: 8px;
		}
		a.example:link, a.example:hover {
			color: black;
		}
		div.example:hover {
 		  background-color: #F5F5F5;
		}
		div.link {
		  float: right;
		  text-align: center;
		  padding-left: 8px;
		  padding-right: 8px;
		  padding-top: 8px;
		  padding-bottom: 8px;
		}
		a.link:link, a.link:hover {
			color: black;
		}
		div.link:hover { 		  
		}		
		a.recent_name { 		  
			color: #811517;
			font-size: 110%;
		}
		span.recent_filename { 		  
			color: #949494;
			font-size: 80%;
		}	
		ul.recent {
			padding-left: 20px;
		}
	</style>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.axislabels.js"></script>
</head>
<body>

<img style="float: right; margin-right: 10px; margin-top: 12px;" src="{{AGROS2D}}" /> 
<h1>{{GETTING_STARTED_LABEL}}</h1>

<table>
<tr>
<td>
<div class="section">
<h2>{{EXAMPLES_LABEL}}</h2>
<hr />
{{#EXAMPLE_SECTION}}
<div class="example">
<a href="example?group={{EXAMPLE_GROUP}}" class="example">
<img src="{{EXAMPLES_DIRECTORY}}/{{EXAMPLE_GROUP}}.png" alt="" with="96" height="96" /><br />
<strong>{{EXAMPLE_GROUP}}</strong>
</a>
</div>
{{/EXAMPLE_SECTION}}
</div>
</td>
</tr>
</table>

<table>
<tr>
<td>
<div class="section">
<h2>{{RECENT_PROBLEMS_LABEL}}</h2>
<hr/>
<ul class="recent">
{{#RECENT_PROBLEM_SECTION}}
<li><a href="open?filename={{PROBLEM_FILENAME}}" class="recent_name">{{PROBLEM_BASE}}</a><br />
	<span class="recent_filename">{{PROBLEM_FILENAME}}</span></li>
{{/RECENT_PROBLEM_SECTION}}
</ul>
</div>
</td>
<td>
<div class="section">
<h2>{{RECENT_SCRIPTS_LABEL}}</h2>
<hr/>
<ul class="recent">
{{#RECENT_SCRIPT_SECTION}}
<li><a href="open?filename={{SCRIPT_FILENAME}}" class="recent_name">{{SCRIPT_BASE}}</a><br />
	<span class="recent_filename">{{SCRIPT_FILENAME}}</span></li>
{{/RECENT_SCRIPT_SECTION}}
</ul>
</div>
</td>
</tr>
</table>

<div class="cleaner"></div>
<br />

<table width="100%">
<tr>
<td>
<div class="section">
<h2>{{LINKS_LABEL}}</h2>
<hr />

<div class="link">
<a href="https://www.facebook.com/pages/Agros2D/132524130149770" class="link">
<img src="{{PANELS_DIRECTORY}}/social/facebook.png" alt="" /><br />
</a>
</div>

<div class="link">
<a href="https://www.youtube.com/channel/UC7WzdNsW6Si-8e96mSluPPg" class="link">
<img src="{{PANELS_DIRECTORY}}/social/youtube.png" alt="" /><br />
</a>
</div>

<div class="link">
<a href="http://www.agros2d.org" class="link">
<img src="{{PANELS_DIRECTORY}}/social/agros2d.png" alt="" /><br />
</a>
</div>

</div>
</td>
</tr>
</table>

<div class="cleaner"></div>

</body>
</html>