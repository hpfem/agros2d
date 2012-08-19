<?xml version="1.0"?>
<template_interface_h>
<head>// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see &lt;http://www.gnu.org/licenses/&gt;.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/
#include &lt;QtPlugin&gt;


#include "weakform_interface.h"

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/localpoint.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"

#include "hermes2d/marker.h"
#include "hermes2d/localpoint.h"
#include "module_interface.h"
</head>

<includes>
#include "module.h"
</includes>

<condition_exact_solution>
        if ((problemId.coordinateType == CoordinateType_"CoordinateType")&amp;&amp; (problemId.linearityType == "LinearityType") &amp;&amp;(i == row_index))
            return new namespace::FunctionName&lt;double&gt;(mesh, boundary);
</condition_exact_solution>

<condition_matrix_vol>
        if ((problemId.coordinateType == CoordinateType_"CoordinateType")&amp;&amp; (problemId.linearityType == "LinearityType") &amp;&amp;(i == row_index)
        &amp;&amp;(j == column_index))
            return new namespace::FunctionName&lt;double&gt;(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);
</condition_matrix_vol> 

<condition_vector_vol>
        if ((problemId.coordinateType == CoordinateType_"CoordinateType")&amp;&amp; (problemId.linearityType == "LinearityType") &amp;&amp;(i == row_index)
        &amp;&amp;(j == column_index))
            return new namespace::FunctionName&lt;double&gt;(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);
</condition_vector_vol>

<condition_matrix_surf>
        if ((problemId.coordinateType == CoordinateType_"CoordinateType")&amp;&amp; (problemId.linearityType == "LinearityType") &amp;&amp;(i == row_index)
        &amp;&amp;(j == column_index)&amp;&amp;(boundary->getType() == "boundary_type"))
            return new namespace::FunctionName&lt;double&gt;(i-1+ offsetI, j-1+ offsetJ, area, boundary);
</condition_matrix_surf>

<condition_vector_surf>
        if ((problemId.coordinateType == CoordinateType_"CoordinateType")&amp;&amp; (problemId.linearityType == "LinearityType") &amp;&amp;(i == row_index)
        &amp;&amp;(j == column_index)&amp;&amp;(boundary->getType() == "boundary_type"))
            return new namespace::FunctionName&lt;double&gt;(i-1+ offsetI, j-1+ offsetJ, area, boundary);
</condition_vector_surf>

<CustomEssentialFormSurf>

Hermes::Hermes2D::ExactSolutionScalar&lt;double&gt; *Module::exactSolution(const ProblemID problemId, int i,Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
{
    //conditions 
    return NULL;
}
</CustomEssentialFormSurf>

<CustomMatrixFormVol>

Hermes::Hermes2D::MatrixFormVol&lt;double&gt; *Module::matrixFormVol(const ProblemID problemId, int i, int j,
                                                              const std::string &amp;area, Hermes::Hermes2D::SymFlag sym,
                                                              SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ )
{
    //conditions 
    return NULL;
}
</CustomMatrixFormVol>

<CustomVectorFormVol>

Hermes::Hermes2D::VectorFormVol&lt;double&gt; *Module::vectorFormVol(const ProblemID problemId, int i, int j,
                                                              const std::string &amp;area, SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ)
{
    //conditions 
    return NULL;
}
</CustomVectorFormVol>

<CustomMatrixFormSurf>

Hermes::Hermes2D::MatrixFormSurf&lt;double&gt; *Module::matrixFormSurf(const ProblemID problemId, int i, int j,
                                                                const std::string &amp;area, SceneBoundary *boundary, int offsetI, int offsetJ)
{
    //conditions 
    return NULL;
}
</CustomMatrixFormSurf>

<CustomVectorFormSurf>

Hermes::Hermes2D::VectorFormSurf&lt;double&gt; *Module::vectorFormSurf(const ProblemID problemId, int i, int j,
                                                                const std::string &amp;area, SceneBoundary *boundary, int offsetI, int offsetJ)
{
    //conditions 
    return NULL;
}
</CustomVectorFormSurf>

<footer>
Q_EXPORT_PLUGIN2(module, Module)
</footer>
</template_interface_h>
