<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
	<link rel="stylesheet" href="problem_style.tpl" type="text/css" />	
</head>
<body>

<table>
<tr>
<td>
<div class="section">
<h2>{{BASIC_INFORMATION_LABEL}}</h2>
<table>
	<tr>
		<td><b>{{NAME_LABEL}}</b></td><td>{{NAME}}</td>
	</tr>
	<tr>
		<td><b>{{COORDINATE_TYPE_LABEL}}</b></td><td>{{COORDINATE_TYPE}}</td>
	</tr>
	<tr>
		<td><b>{{MESH_TYPE_LABEL}}</b></td><td>{{MESH_TYPE}}</td>
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
            <tr><td><b>{{TRANSIENT_TOLERANCE_LABELS}}</b></td><td>{{TRANSIENT_TOLERANCE}}</td></tr>
            <tr><td><b>{{TRANSIENT_CONSTANT_NUM_STEPS_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_NUM_STEPS}}</td></tr>
            <tr><td><b>{{TRANSIENT_CONSTANT_STEP_LABEL}}</b></td><td>{{TRANSIENT_CONSTANT_STEP}}</td></tr>
            <tr><td><b>{{TRANSIENT_TOTAL_LABEL}}</b></td><td>{{TRANSIENT_TOTAL}}</td></tr>
        </table>
    </td>
    </tr>
    {{/TRANSIENT}}
	<tr>
	<td>
		<h3>{{GEOMETRY_LABEL}}</h3>
		<table>
			<tr><td><b>{{GEOMETRY_NODES_LABEL}}</b></td><td>{{GEOMETRY_NODES}}</td></tr>
			<tr><td><b>{{GEOMETRY_EDGES_LABEL}}</b></td><td>{{GEOMETRY_EDGES}}</td></tr>
			<tr><td><b>{{GEOMETRY_LABELS_LABEL}}</b></td><td>{{GEOMETRY_LABELS}}</td></tr>
			<tr><td><b>{{GEOMETRY_MATERIALS_LABEL}}</b></td><td>{{GEOMETRY_MATERIALS}}</td></tr>
			<tr><td><b>{{GEOMETRY_BOUNDARIES_LABEL}}</b></td><td>{{GEOMETRY_BOUNDARIES}}</td></tr>
		</table>
	</td>
	<td><div class="figure">{{GEOMETRY_SVG}}</div></td>
	</tr>
</table>
</div>

<div class="section">
{{#COUPLING}}
<h2>{{COUPLING_MAIN_LABEL}}</h2>
<table>
	{{#COUPLING_SECTION}}
	<tr>
		<td colspan=2><h3>{{COUPLING_LABEL}}</h3></td>
	<tr>
	<tr>
		<td><b>{{COUPLING_SOURCE_LABEL}}</b></td><td>{{COUPLING_SOURCE}}</td>
	<tr>
	</tr>
		<td><b>{{COUPLING_TARGET_LABEL}}</b></td><td>{{COUPLING_TARGET}}</td>
	<tr>
	</tr>
		<td><b>{{COUPLING_TYPE_LABEL}}</b></td><td>{{COUPLING_TYPE}}</td>		
	</tr>
	{{/COUPLING_SECTION}}
</table>
</div>
{{/COUPLING}}

{{#SOLUTION_PARAMETERS_SECTION}}
<div class="section">
<h2>{{SOLUTION_LABEL}}</h2>
<table>
    <tr>
        <td><b>{{SOLUTION_ELAPSED_TIME_LABEL}}</b></td><td>{{SOLUTION_ELAPSED_TIME}}</td>
    </tr>
    <tr>
        <td><b>{{NUM_THREADS_LABEL}}</b></td><td>{{NUM_THREADS}}</td>
    </tr>
    {{#TRANSIENT_ADAPTIVE}}
    <tr>
        <td colspan="2"><div style="text-align: center;">Time step length</div><div id="chart_time_step_length" style="width:100%; height:130px;"></div></td>
    </tr>
    {{/TRANSIENT_ADAPTIVE}}
</table>
</div>
{{/SOLUTION_PARAMETERS_SECTION}}
</td>

<td>
{{#FIELD}}
{{#FIELD_SECTION}}
<div class="section">
<h2>{{PHYSICAL_FIELD_LABEL}}</h2>
<table>
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

<table>
    {{#MESH_PARAMETERS_SECTION}}
    <tr><td colspan=2><h3>{{MESH_LABEL}}</h3></td></tr>
    <tr>
        <td><b>{{INITIAL_MESH_LABEL}}</b></td><td>{{INITIAL_MESH_NODES}}</td>
    </tr>
    <tr>
        <td>&nbsp;</td><td>{{INITIAL_MESH_ELEMENTS}}</td>
    </tr>
    {{#MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    <tr>
        <td><b>{{SOLUTION_MESH_LABEL}}</b></td><td>{{SOLUTION_MESH_NODES}}</td>
    </tr>
    <tr>
        <td>&nbsp;</td><td>{{SOLUTION_MESH_ELEMENTS}}</td>
    </tr>
    {{/MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    {{#MESH_SOLUTION_DOFS_PARAMETERS_SECTION}}
    <tr>
        <td><b>{{DOFS_LABEL}}</b></td><td>{{DOFS}}</td>
    </tr>
    {{#MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    <tr>
        <td><b>{{ERROR_LABEL}}</b></td><td>{{ERROR}}</td>
    </tr>
    <tr>
        <td colspan="2"><div style="text-align: center;">Relative error</div><div id="chart_error_steps_{{PHYSICAL_FIELD_ID}}" style="width:100%; height:130px;"></div></td>
    </tr>   
    <tr>
        <td colspan="2"><div style="text-align: center;">DOFs</div><div id="chart_dofs_steps_{{PHYSICAL_FIELD_ID}}" style="width:100%; height:130px;"></div></td>
    </tr>   
    <!--
	<script type="text/javascript">
	$(function () 
	{    
		var previousPoint = null;
		$("#chart_dofs_steps_{{PHYSICAL_FIELD_ID}}").bind("plothover", 
		function (event, pos, item) 
		{
	        if (item) 
	        {
			    $("#x").text(pos.x.toFixed(2));
			    $("#y").text(pos.y.toFixed(2));
		    
  	            if (previousPoint != item.dataIndex) 
  	            {
	                previousPoint = item.dataIndex;
	                
	                $("#tooltip").remove();
	                var x = item.datapoint[0].toFixed(2),
	                    y = item.datapoint[1].toFixed(2);
	                
	                showTooltip(item.pageX, item.pageY,
	                            item.series.label + " of " + x + " = " + y);
	            }
	        }
	        else 
	        {
	            $("#tooltip").remove();
	            previousPoint = null;            
	        }
		});
	});
	</script>    
	-->
    {{/MESH_SOLUTION_ADAPTIVITY_PARAMETERS_SECTION}}
    {{/MESH_SOLUTION_DOFS_PARAMETERS_SECTION}}
    {{/MESH_PARAMETERS_SECTION}}
</table>

</div>
{{/FIELD_SECTION}}
{{/FIELD}}
</td>
</tr>
</table>

<div class="cleaner"></div>

<!--
<script type="text/javascript">
function showTooltip(x, y, contents) 
{
    $('<div id="tooltip">' + contents + '</div>').css({
        position: 'absolute',
        display: 'none',
        top: y + 5,
        left: x + 5,
        border: '1px solid #fdd',
        padding: '2px', 
        background-color: '#fee',
        opacity: 0.80
    }).appendTo("body").fadeIn(200);
}
</script>
-->
</body>
</html>

<!--
{{#SOLUTION_SECTION}}
<h1>{{SOLUTION_INFORMATION_LABEL}}</h1>
<div class="section">
<table>
	<tr><td colspan=2><h2>{{INITIAL_MESH_LABEL}}</h2></td></tr>
	<tr><td colspan=2><div class="subsection">
		<table>
			<tr><td><b>{{INITIAL_MESH_NODES_LABEL}}</b></td><td>{{INITIAL_MESH_NODES}}</td></tr>
			<tr><td><b>{{INITIAL_MESH_ELEMENTS_LABEL}}</b></td><td>{{INITIAL_MESH_ELEMENTS}}</td></tr>
		</table>
	</div></td></tr>
	{{#SOLUTION_PARAMETERS_SECTION}}
	<tr><td><b>{{ELAPSED_TIME_LABEL}}</b></td><td>{{ELAPSED_TIME}}</td></tr>
	<tr><td><b>{{DOFS_LABEL}}</b></td><td>{{DOFS}}</td></tr>
	{{#ADAPTIVITY_SECTION}}
	<tr><td colspan=2><h2>{{ADAPTIVITY_LABEL}}</h2></td></tr>
	<tr><td colspan=2><div class="subsection">
		<table>
			<tr><td><b>{{ADAPTIVITY_ERROR_LABEL}}</b></td><td>{{ADAPTIVITY_ERROR}}</td></tr>
			<tr><td><b>{{ADAPTIVITY_TOLERANCE_LABEL}}</b></td><td>{{ADAPTIVITY_TOLERANCE}}</td></tr>
		</table>
	</div></td></tr>
	<tr><td colspan=2><h2>{{SOLUTION_MESH_LABEL}}</h2></td></tr>
	<tr><td colspan=2><div class="subsection">
		<table>
			<tr><td><b>{{SOLUTION_MESH_NODES_LABEL}}</b></td><td>{{SOLUTION_MESH_NODES}}</td></tr>
			<tr><td><b>{{SOLUTION_MESH_ELEMENTS_LABEL}}</b></td><td>{{SOLUTION_MESH_ELEMENTS}}</td></tr>
		</table>
	</div></td></tr>
	{{/ADAPTIVITY_SECTION}}
	{{/SOLUTION_PARAMETERS_SECTION}}
</table>
</div>
{{/SOLUTION_SECTION}}
-->
