<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
	<style type="text/css">
        * { margin: 0; padding: 0; }
        #page-wrap { width: 960px; margin: 100px auto; }
        .group:after { visibility: hidden; display: block; font-size: 0; content: " "; clear: both; height: 0; }
        p { margin: 0 0 10px 0; }
        
        .tabs { list-style: none; }
        .tabs li { display: inline; }
        .tabs li a { color: black; float: left; display: block; padding: 4px 10px; margin-left: -1px; position: relative; left: 1px; background: white; text-decoration: none; }
        .tabs li a:hover { background: #ccc; }

 		.tabbed-area { margin: 0 0 120px 0; }
        .box-wrap { position: relative; min-height: 150px; }
        .tabbed-area div div { background: white; padding: 20px; min-height: 150px; position: absolute; top: -1px; left: 0; width: 100%; }
        .tabbed-area div div, .tabs li a { border: 1px solid #ccc; }
        #box-one:target, #box-two:target, #box-three:target { z-index: 1; }

		.adjacent { position: relative; margin-top: 50px; min-height: 300px; }
		.adjacent div { border: 1px solid #ccc; background: white; padding: 20px; min-height: 150px; position: absolute; top: -1px; left: 0; width: 100%; }
		.adjacent .tabs { position: absolute; bottom: 100%; left: 0; z-index: 2; }
		.adjacent .tabs li a { background: -moz-linear-gradient(top, white, #eee); }
		#box-thirteen:target,
		#box-fourteen:target,
		#box-fifteen:target { z-index: 1; }
		#box-thirteen:target ~ .tabs a[href='#box-thirteen'],
		#box-fourteen:target ~ .tabs a[href='#box-fourteen'],
		#box-fifteen:target ~ .tabs a[href='#box-fifteen'] { background: white; border-bottom: 1px solid white; }
	</style>
</head>
<body>

<h1>{{BASIC_INFORMATION_LABEL}}</h1>
<div class="section">
<table class="maintable">
	<tr>
		<td><b>{{COORDINATE_TYPE_LABEL}}</b></td><td>{{COORDINATE_TYPE}}</td>
	</tr>
    {{#HARMONIC}}
    <tr>
    <td colspan="2">
        <h3>{{HARMONIC_LABEL}}</h3>
        <table>
            <tr><td><b>{{HARMONIC_FREQUENCY_LABEL}}</b></td><td>{{HARMONIC_FREQUENCY}}</td></tr>
        </table>
    </td>
    </tr>
    {{/HARMONIC}}
    {{#TRANSIENT}}
    <tr>
    <td colspan="2">
        <h3>{{TRANSIENT_LABEL}}</h3>
        <table>
            <tr><td><b>{{TRANSIENT_STEP_METHOD_LABEL}}</b></td><td>{{TRANSIENT_STEP_METHOD}}</td></tr>
            <tr><td><b>{{TRANSIENT_STEP_ORDER_LABEL}}</b></td><td>{{TRANSIENT_STEP_ORDER}}</td></tr>
            <tr><td><b>{{TRANSIENT_TOLERANCE_LABEL}}</b></td><td>{{TRANSIENT_TOLERANCE}}</td></tr>
            <tr><td><b>{{TRANSIENT_CONSTANT_NUM_STEPS_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_NUM_STEPS}}</td></tr>
            <tr><td><b>{{TRANSIENT_CONSTANT_STEP_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_STEP}}</td></tr>
            <tr><td><b>{{TRANSIENT_TOTAL_LABEL}}</b></td><td>{{TRANSIENT_TOTAL}}</td></tr>
        </table>
    </td>
    </tr>
    {{/TRANSIENT}}
</table>
</div>

{{#FIELD}}
<div class="section">
<h2>{{PHYSICAL_FIELD}}</h2>
<table class="maintable">
	<tr>
		<td><b>{{ANALYSIS_TYPE_LABEL}}</b></td><td>{{ANALYSIS_TYPE}}</td>
	</tr>
	<tr>
		<td><b>{{WEAK_FORMS_TYPE_LABEL}}</b></td><td>{{WEAK_FORMS_TYPE}}</td>
	</tr>
	{{#INITIAL_CONDITION_SECTION}}
	<tr>
		<td><b>{{INITIAL_CONDITION_LABEL}}</b></td><td>{{INITIAL_CONDITION}}</td>
	</tr>
	{{/INITIAL_CONDITION_SECTION}}
	<tr>
		<td><b>{{LINEARITY_TYPE_LABEL}}</b></td><td>{{LINEARITY_TYPE}}</td>
	</tr>
	{{#SOLVER_PARAMETERS_SECTION}}
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{NONLINEAR_STEPS_LABEL}}</b></td><td>{{NONLINEAR_STEPS}}</td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{NONLINEAR_TOLERANCE_LABEL}}</b></td><td>{{NONLINEAR_TOLERANCE}}</td>
	</tr>		
	{{/SOLVER_PARAMETERS_SECTION}}
	<tr>
		<td><b>{{ADAPTIVITY_TYPE_LABEL}}</b></td><td>{{ADAPTIVITY_TYPE}}</td>
	</tr>
	{{#ADAPTIVITY_PARAMETERS_SECTION}}
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{ADAPTIVITY_STEPS_LABEL}}</b></td><td>{{ADAPTIVITY_STEPS}}</td>
	</tr>
	<tr>
		<td>&nbsp;&nbsp;&nbsp;<b>{{ADAPTIVITY_TOLERANCE_LABEL}}</b></td><td>{{ADAPTIVITY_TOLERANCE}}</td>
	</tr>
	{{/ADAPTIVITY_PARAMETERS_SECTION}}
	<tr>
		<td><b>{{REFINEMENS_NUMBER_LABEL}}</b></td><td>{{REFINEMENS_NUMBER}}</td>
	</tr>
	<tr>
		<td><b>{{POLYNOMIAL_ORDER_LABEL}}</b></td><td>{{POLYNOMIAL_ORDER}}</td>
	</tr>	
</table>
</div>
{{/FIELD}}

</body>
</html>

