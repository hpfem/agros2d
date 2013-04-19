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

#include "block.h"
#include "util/global.h"

#include "field.h"
#include "problem.h"
#include "coupling.h"
#include "scene.h"
#include "logview.h"
#include "solver.h"
#include "module.h"
#include "problem_config.h"
#include "plugin_interface.h"


Block::Block(QList<FieldInfo *> fieldInfos, QList<CouplingInfo*> couplings) :
    m_couplings(couplings), m_wf(NULL)
{
    foreach (FieldInfo* fi, fieldInfos)
    {
        Field* field = new Field(fi);
        foreach (CouplingInfo* couplingInfo, Agros2D::problem()->couplingInfos())
        {
            if (couplingInfo->isWeak() && (couplingInfo->targetField() == fi))
            {
                field->addCouplingInfo(couplingInfo);
            }
        }

        m_fields.append(field);
    }
}

Block::~Block()
{
    // clear fields
    foreach (Field *field, m_fields)
        delete field;
    m_fields.clear();

    // clear boundary conditions
    foreach (Hermes::Hermes2D::EssentialBCs<double> *bc, m_bcs)
    {
        for (Hermes::vector<Hermes::Hermes2D::EssentialBoundaryCondition<double> *>::const_iterator it = bc->begin(); it < bc->end(); ++it)
        {
            // delete bc
            delete *it;
        }

        delete bc;
    }
    m_bcs.clear();

    // delete weakform
    if (m_wf)
        delete m_wf;
    m_wf = NULL;
}

void Block::createBoundaryConditions()
{
    // todo: memory leak? boundary conditions are probably released in space, but really?
    m_bcs.clear();

    // essential boundary conditions
    for (int i = 0; i < numSolutions(); i++)
        m_bcs.push_back(new Hermes::Hermes2D::EssentialBCs<double>());

    m_exactSolutionFunctions.clear();

    foreach(Field* field, this->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        ProblemID problemId;

        problemId.sourceFieldId = fieldInfo->fieldId();
        problemId.analysisTypeSource = fieldInfo->analysisType();
        problemId.coordinateType = Agros2D::problem()->config()->coordinateType();
        problemId.linearityType = fieldInfo->linearityType();

        int index = 0;
        foreach(SceneEdge* edge, Agros2D::scene()->edges->items())
        {
            SceneBoundary *boundary = edge->marker(fieldInfo);

            if (boundary && (!boundary->isNone()))
            {
                Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

                foreach (FormInfo form, boundaryType.essential())
                {
                    // exact solution - Dirichlet BC
                    MeshFunctionSharedPtr<double> function = MeshFunctionSharedPtr<double>(fieldInfo->plugin()->exactSolution(problemId, &form, fieldInfo->initialMesh()));
                    static_cast<ExactSolutionScalarAgros<double> *>(function.get())->setMarkerSource(boundary);

                    // save function - boundary pairs, so thay can be easily updated in each time step;
                    m_exactSolutionFunctions[function] = boundary;

                    Hermes::Hermes2D::EssentialBoundaryCondition<double> *custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(), function);

                    this->bcs().at(form.i - 1 + this->offset(field))->add_boundary_condition(custom_form);
                    //  cout << "adding BC i: " << form->i - 1 + this->offset(field) << " ( form i " << form->i << ", " << this->offset(field) << "), expression: " << form->expression << endl;
                }
            }
            index++;
        }
    }
}

void Block::updateExactSolutionFunctions()
{
    foreach(MeshFunctionSharedPtr<double> function, m_exactSolutionFunctions.keys())
    {
        SceneBoundary* boundary = m_exactSolutionFunctions[function];
        static_cast<ExactSolutionScalarAgros<double> *>(function.get())->setMarkerSource(boundary);
    }
}

void Block::setWeakForm(WeakFormAgros<double> *wf)
{
    if (m_wf) delete m_wf; m_wf = wf;
}

ProblemSolver<double> *Block::prepareSolver()
{
    ProblemSolver<double> *solver = new ProblemSolver<double>();

    foreach (Field* field, m_fields)
    {
        // evaluate all values
        if (!field->solveInitVariables())
        {
            delete solver;
            return NULL;
        }
    }

    solver->init(this);

    return solver;
}

bool Block::isTransient() const
{
    foreach (Field *field, m_fields)
    {
        if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
            return true;
    }

    return false;
}

double Block::timeSkip() const
{
    double skip = 0.;
    foreach (Field *field, m_fields)
    {
        if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
            continue;

        double sActual = field->fieldInfo()->value(FieldInfo::TransientTimeSkip).toDouble();
        if ((skip == 0.) || (sActual < skip))
            skip = sActual;
    }
    return skip;
}

AdaptivityType Block::adaptivityType() const
{
    AdaptivityType at = m_fields.at(0)->fieldInfo()->adaptivityType();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->adaptivityType() == at);
    }

    return at;
}

int Block::adaptivitySteps() const
{
    int as = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivitySteps).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivitySteps).toInt() == as);
    }

    return as;
}

int Block::adaptivityBackSteps() const
{
    int abs = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityTransientBackSteps).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivityTransientBackSteps).toInt() == abs);
    }

    return abs;
}

int Block::adaptivityRedoneEach() const
{
    int re = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityTransientRedoneEach).toInt();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivityTransientRedoneEach).toInt() == re);
    }

    return re;
}

double Block::adaptivityTolerance() const
{
    double at = m_fields.at(0)->fieldInfo()->value(FieldInfo::AdaptivityTolerance).toDouble();

    foreach (Field *field, m_fields)
    {
        // todo: ensure in GUI
        assert(field->fieldInfo()->value(FieldInfo::AdaptivityTolerance).toDouble() == at);
    }

    return at;
}

int Block::numSolutions() const
{
    int num = 0;
    foreach (Field *field, m_fields)
        num += field->fieldInfo()->numberOfSolutions();

    return num;
}

int Block::offset(Field *fieldParam) const
{
    int offset = 0;

    foreach (Field* field, m_fields)
    {
        if (field == fieldParam)
            return offset;
        else
            offset += field->fieldInfo()->numberOfSolutions();
    }

    assert(0);
}

LinearityType Block::linearityType() const
{
    int linear = 0;
    int newton = 0;
    int picard = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->linearityType() == LinearityType_Linear)
            linear++;
        if (fieldInfo->linearityType() == LinearityType_Newton)
            newton++;
        if (fieldInfo->linearityType() == LinearityType_Picard)
            picard++;
    }
    assert(linear * newton * picard == 0); // all hard coupled fields has to be solved by the same method

    if (linear)
        return LinearityType_Linear;
    else if (newton)
        return  LinearityType_Newton;
    else if (picard)
        return  LinearityType_Picard;
    else
        assert(0);
}

double Block::nonlinearTolerance() const
{
    double tolerance = 10e20;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NonlinearTolerance).toDouble() < tolerance)
            tolerance = fieldInfo->value(FieldInfo::NonlinearTolerance).toDouble();
    }

    return tolerance;
}

int Block::nonlinearSteps() const
{
    int steps = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NonlinearSteps).toInt() > steps)
            steps = fieldInfo->value(FieldInfo::NonlinearSteps).toInt();
    }

    return steps;
}

Hermes::Hermes2D::NewtonSolverConvergenceMeasurement Block::nonlinearConvergenceMeasurement() const
{
    // how to set properly?
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        return (Hermes::Hermes2D::NewtonSolverConvergenceMeasurement) fieldInfo->value(FieldInfo::NonlinearConvergenceMeasurement).toInt();
    }

    return Hermes::Hermes2D::ResidualNormAbsolute;
}

bool Block::newtonAutomaticDamping() const
{
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (!fieldInfo->value(FieldInfo::NewtonAutomaticDamping).toBool())
            return false;
    }

    return true;
}

double Block::newtonDampingCoeff() const
{
    double coeff = 1.0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NewtonDampingCoeff).toDouble() < coeff)
            coeff = fieldInfo->value(FieldInfo::NewtonDampingCoeff).toDouble();
    }

    return coeff;
}

double Block::newtonAutomaticDampingCoeff() const
{
    double coeff = 1.0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NewtonAutomaticDampingCoeff).toDouble() < coeff)
            coeff = fieldInfo->value(FieldInfo::NewtonAutomaticDampingCoeff).toDouble();
    }

    return coeff;
}

int Block::newtonDampingNumberToIncrease() const
{
    int number = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NewtonDampingNumberToIncrease).toInt() > number)
            number = fieldInfo->value(FieldInfo::NewtonDampingNumberToIncrease).toInt();
    }

    return number;
}

double Block::sufficientImprovementFactorJacobian() const
{
    double number = 1.0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NewtonSufficientImprovementFactorJacobian).toDouble() < number)
            number = fieldInfo->value(FieldInfo::NewtonSufficientImprovementFactorJacobian).toDouble();
    }

    return number;
}

int Block::maxStepsWithReusedJacobian() const
{
    int number = 10;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::NewtonMaximumStepsWithReusedJacobian).toInt() < number)
            number = fieldInfo->value(FieldInfo::NewtonMaximumStepsWithReusedJacobian).toInt();
    }

    return number;
}

bool Block::picardAndersonAcceleration() const
{
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (!fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool())
            return false;
    }

    return true;
}

double Block::picardAndersonBeta() const
{
    double number = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        // TODO: check ">"
        if (fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble() > number)
            number = fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble();
    }

    return number;
}

int Block::picardAndersonNumberOfLastVectors() const
{
    int number = 1;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if (fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt() > number)
            number = fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt();
    }

    return number;
}

bool Block::contains(FieldInfo *fieldInfo) const
{
    foreach(Field* field, m_fields)
    {
        if (field->fieldInfo() == fieldInfo)
            return true;
    }
    return false;
}

Field* Block::field(FieldInfo *fieldInfo) const
{
    foreach (Field* field, m_fields)
    {
        if (fieldInfo == field->fieldInfo())
            return field;
    }

    return NULL;
}

Hermes::vector<Hermes::Hermes2D::ProjNormType> Block::projNormTypeVector() const
{
    Hermes::vector<Hermes::Hermes2D::ProjNormType> vec;

    foreach (Field* field, m_fields)
    {
        for (int comp = 0; comp < field->fieldInfo()->numberOfSolutions(); comp++)
        {
            Hermes::Hermes2D::SpaceType spaceType = field->fieldInfo()->spaces()[comp+1].type();
            if (spaceType == Hermes::Hermes2D::HERMES_H1_SPACE)
                vec.push_back(Hermes::Hermes2D::HERMES_H1_NORM);
            else if (spaceType == Hermes::Hermes2D::HERMES_L2_SPACE)
                vec.push_back(Hermes::Hermes2D::HERMES_L2_NORM);
            else
                assert(0);
        }
    }
    return vec;
}

ostream& operator<<(ostream& output, const Block& id)
{
    output << "Block ";
    return output;
}


