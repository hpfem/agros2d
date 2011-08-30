<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<link href="{{STYLESHEET}}" rel="stylesheet" type="text/css" media="screen" />
</head>
<body>
<h1>{{NAME}}</h1>

<h2>{{DESCRIPTION_LABEL}}</h2>
<p>{{DESCRIPTION}}</p>

<h2>{{PROBLEM_LABEL}}</h2>
<table>
	<tr><td>{{PROBLEM_NAME_LABEL}}</td><td>{{PROBLEM_NAME}}</td></tr>
	<tr><td>{{PROBLEM_DATE_LABEL}}</td><td>{{PROBLEM_DATE}}</td></tr>
	<tr><td>{{PROBLEM_FILENAME_LABEL}}</td><td>{{PROBLEM_FILENAME}}</td></tr>
	<tr><td>{{PROBLEM_PROBLEMTYPE_LABEL}}</td><td>{{PROBLEM_PROBLEMTYPE}}</td></tr>
	<tr><td>{{PROBLEM_PHYSICFIELD_LABEL}}</td><td>{{PROBLEM_PHYSICFIELD}}</td></tr>
	<tr><td>{{PROBLEM_ANALYSISTYPE_LABEL}}</td><td>{{PROBLEM_ANALYSISTYPE}}</td></tr>
	<tr><td>{{PROBLEM_NUMBEROFREFINEMENTS_LABEL}}</td><td>{{PROBLEM_NUMBEROFREFINEMENTS}}</td></tr>
	<tr><td>{{PROBLEM_POLYNOMIALORDER_LABEL}}</td><td>{{PROBLEM_POLYNOMIALORDER}}</td></tr>
	<tr><td>{{PROBLEM_ADAPTIVITYTYPE_LABEL}}</td><td>{{PROBLEM_ADAPTIVITYTYPE}}</td></tr>
	<tr><td>{{PROBLEM_ADAPTIVITYSTEPS_LABEL}}</td><td>{{PROBLEM_ADAPTIVITYSTEPS}}</td></tr>
	<tr><td>{{PROBLEM_ADAPTIVITYTOLERANCE_LABEL}}</td><td>{{PROBLEM_ADAPTIVITYTOLERANCE}} %</td></tr>
	<tr><td>{{PROBLEM_MAXDOFS_LABEL}}</td><td>{{PROBLEM_MAXDOFS}}</td></tr>
	<tr><td>{{PROBLEM_FREQUENCY_LABEL}}</td><td>{{PROBLEM_FREQUENCY}} Hz</td></tr>
	<tr><td>{{PROBLEM_TIMESTEP_LABEL}}</td><td>{{PROBLEM_TIMESTEP}} s</td></tr>
	<tr><td>{{PROBLEM_TIMETOTAL_LABEL}}</td><td>{{PROBLEM_TIMETOTAL}} s</td></tr>
	<tr><td>{{PROBLEM_INITITALCONDITION_LABEL}}</td><td>{{PROBLEM_INITITALCONDITION}}</td></tr>
</table>

<h2>{{STARTUPSCRIPT_LABEL}}</h2>
<pre>{{STARTUPSCRIPT}}</pre>

<h2>{{PHYSICALPROPERTIES_LABEL}}</h2>
<h3>{{MATERIALS_LABEL}}</h3>
{{MATERIALS}}
<h3>{{BOUNDARIES_LABEL}}</h3>
{{BOUNDARIES}}

<h2>{{GEOMETRY_LABEL}}</h2>
{{FIGURE_GEOMETRY}}
<h3>{{GEOMETRY_NODES_LABEL}}</h3>
{{GEOMETRY_NODES}}
<h3>{{GEOMETRY_EDGES_LABEL}}</h3>
{{GEOMETRY_EDGES}}
<h3>{{GEOMETRY_LABELS_LABEL}}</h3>
{{GEOMETRY_LABELS}}

<h2>{{MESHANDSOLVER_LABEL}}</h2>
<h3>{{SOLVER_LABEL}}</h3>
<table>
	<tr><td>{{SOLVER_NODES_LABEL}}</td><td>{{SOLVER_NODES}}</td></tr>
	<tr><td>{{SOLVER_ELEMENTS_LABEL}}</td><td>{{SOLVER_ELEMENTS}}</td></tr>
	<tr><td>{{SOLVER_DOFS_LABEL}}</td><td>{{SOLVER_DOFS}}</td></tr>
	<tr><td>{{SOLVER_TIMEELAPSED_LABEL}}</td><td>{{SOLVER_TIMEELAPSED}}</td></tr>
	<tr><td>{{SOLVER_ADAPTIVEERROR_LABEL}}</td><td>{{SOLVER_ADAPTIVEERROR}}</td></tr>
	<tr><td>{{SOLVER_ADAPTIVESTEPS_LABEL}}</td><td>{{SOLVER_ADAPTIVESTEPS}}</td></tr>
</table>

<h2>{{FIGURE_LABEL}}</h2>
{{FIGURE_MESH}}
{{FIGURE_ORDER}}
{{FIGURE_SCALARVIEW}}
{{FIGURE_CONTOURVIEW}}
{{FIGURE_VECTORVIEW}}

<h2>{{SCRIPT_LABEL}}</h2>
<pre>{{SCRIPT}}</pre>

<p id=\"footer\">Computed by Agros2D (<a href=\"http://agros2d.org/\">http://agros2d.org/</a>)</p>
</body>
</html>
