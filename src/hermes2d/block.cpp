#include "field.h"
#include "block.h"
#include "problem.h"
#include "coupling.h"
#include "scene.h"
#include "logview.h"
#include "solver.h"
#include "module.h"
#include "module_agros.h"

Block::Block(QList<FieldInfo *> fieldInfos, QList<CouplingInfo*> couplings) :
    m_couplings(couplings)
{
    foreach (FieldInfo* fi, fieldInfos)
    {
        Field* field = new Field(fi);
        foreach (CouplingInfo* couplingInfo, Util::problem()->couplingInfos())
        {
            if(couplingInfo->isWeak() && (couplingInfo->targetField() == fi))
            {
                field->m_couplingSources.push_back(couplingInfo);
            }
        }

        m_fields.append(field);
    }
}

Solver<double>* Block::prepareSolver()
{
    //Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("prepare solver"));

    Solver<double>* solver = new Solver<double>;

    foreach (Field* field, m_fields)
    {
        if(! field->solveInitVariables())
            assert(0); //TODO co to znamena?
    }

    m_wf = new WeakFormAgros<double>(this);

    solver->init(m_wf, this);

    return solver;
}

bool Block::isTransient() const
{
    foreach (Field *field, m_fields)
    {
        if(field->fieldInfo()->analysisType() == AnalysisType_Transient)
            return true;
    }

    return false;
}

bool Block::skipThisTimeStep(int timeStep) const
{
    if (timeStep == Util::problem()->config()->numTimeSteps())
        return false;

    int S = m_fields.at(0)->fieldInfo()->timeStepsSkip().number();
    foreach (Field *field, m_fields)
    {
        int sActual = field->fieldInfo()->timeStepsSkip().number();
        if (sActual < S)
            S = sActual;
    }

    return (S > 0) ? timeStep % S > 0 : false;
}

AdaptivityType Block::adaptivityType() const
{
    AdaptivityType at = m_fields.at(0)->fieldInfo()->adaptivityType();

    foreach (Field *field, m_fields)
    {
        assert(field->fieldInfo()->adaptivityType() == at);
    }

    return at;
}

int Block::adaptivitySteps() const
{
    int as = m_fields.at(0)->fieldInfo()->adaptivitySteps();

    foreach (Field *field, m_fields)
    {
        assert(field->fieldInfo()->adaptivitySteps() == as);
    }

    return as;
}

double Block::adaptivityTolerance() const
{
    double at = m_fields.at(0)->fieldInfo()->adaptivityTolerance();

    foreach (Field *field, m_fields)
    {
        assert(field->fieldInfo()->adaptivityTolerance() == at);
    }

    return at;
}

int Block::numSolutions() const
{
    int num = 0;
    foreach (Field *field, m_fields)
        num += field->fieldInfo()->module()->numberOfSolutions();

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
            offset += field->fieldInfo()->module()->numberOfSolutions();
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
        if(fieldInfo->nonlinearTolerance() < tolerance)
            tolerance = fieldInfo->nonlinearTolerance();
    }

    return tolerance;
}

int Block::nonlinearSteps() const
{
    int steps = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->nonlinearSteps() > steps)
            steps = fieldInfo->nonlinearSteps();
    }

    return steps;
}

bool Block::contains(FieldInfo *fieldInfo) const
{
    foreach(Field* field, m_fields)
    {
        if(field->fieldInfo() == fieldInfo)
            return true;
    }
    return false;
}

Field* Block::field(FieldInfo *fieldInfo) const
{
    foreach (Field* field, m_fields)
    {
        if(fieldInfo == field->fieldInfo())
            return field;
    }

    return NULL;
}

Hermes::vector<Hermes::Hermes2D::ProjNormType> Block::projNormTypeVector() const
{
    Hermes::vector<Hermes::Hermes2D::ProjNormType> vec;

    foreach (Field* field, m_fields)
    {
        for (int comp = 0; comp < field->fieldInfo()->module()->numberOfSolutions(); comp++)
        {
            Hermes::Hermes2D::SpaceType spaceType = field->fieldInfo()->module()->spaceType(comp);
            if(spaceType == Hermes::Hermes2D::HERMES_H1_SPACE)
                vec.push_back(Hermes::Hermes2D::HERMES_H1_NORM);
            else if(spaceType == Hermes::Hermes2D::HERMES_L2_SPACE)
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


