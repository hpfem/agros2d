// This file is part of Agros2D.
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
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "problem_config.h"

#include "util/global.h"

#include "field.h"
#include "block.h"
#include "solutionstore.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "module.h"
#include "module_agros.h"
#include "coupling.h"
#include "solver.h"
#include "meshgenerator_triangle.h"
#include "meshgenerator_gmsh.h"
#include "logview.h"

ProblemConfig::ProblemConfig(QWidget *parent) : QObject(parent)
{
    clear();
}

void ProblemConfig::clear()
{
    m_coordinateType = CoordinateType_Planar;
    m_name = QObject::tr("unnamed");
    m_fileName = "";
    m_startupscript = "";
    m_description = "";

    // matrix solver
    m_matrixSolver = Hermes::SOLVER_UMFPACK;

    // mesh type
    m_meshType = MeshType_Triangle;

    // harmonic
    m_frequency = 0.0;

    // transient
    m_timeStepMethod = TimeStepMethod_BDFNumSteps;
    m_timeOrder = 2;
    m_timeMethodTolerance = Value("0.1", false);
    m_timeTotal = Value("1.0", false);
    m_numConstantTimeSteps = 10;
}

bool ProblemConfig::isTransientAdaptive() const
{
    if((m_timeStepMethod == TimeStepMethod_BDFTolerance) || (m_timeStepMethod == TimeStepMethod_BDFNumSteps))
        return true;
    else if(m_timeStepMethod == TimeStepMethod_Fixed)
        return false;

    assert(0);
}

// todo: put to gui
const double initialTimeStepRatio = 500;
double ProblemConfig::initialTimeStepLength()
{
    if(timeStepMethod() == TimeStepMethod_BDFTolerance)
        return timeTotal().value() / initialTimeStepRatio;
    else if(timeStepMethod() == TimeStepMethod_BDFNumSteps)
        return constantTimeStepLength() / 3.;
    else if (timeStepMethod() == TimeStepMethod_Fixed)
        return constantTimeStepLength();
    else
        assert(0);
}
