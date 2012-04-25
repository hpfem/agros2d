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
    Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("prepare solver"));

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
    {
        num += field->fieldInfo()->module()->number_of_solution();
    }

    return num;
}

int Block::offset(Field *fieldParam) const
{
    int offset = 0;

    foreach (Field* field, m_fields)
    {
        if(field == fieldParam)
            return offset;
        else
            offset += field->fieldInfo()->module()->number_of_solution();
    }

    assert(0);
}

LinearityType Block::linearityType() const
{
    int linear = 0, newton = 0;
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->linearityType() == LinearityType_Linear)
            linear++;
        if(fieldInfo->linearityType() == LinearityType_Newton)
            newton++;
    }
    assert(linear * newton == 0); // all hard coupled fields has to be solved by the same method

    if(linear)
        return LinearityType_Linear;
    else if(newton)
        return  LinearityType_Newton;
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

int Block::numTimeSteps() const
{
    int timeSteps = 0;
    foreach(Field* field, m_fields)
    {
        if(field->fieldInfo()->analysisType() == AnalysisType_Transient)
        {
            int fieldTimeSteps = floor(Util::problem()->config()->timeTotal().number() / Util::problem()->config()->timeStep().number());
            if(fieldTimeSteps > timeSteps)
                timeSteps = fieldTimeSteps;
        }
    }
    return timeSteps;
}

double Block::timeStep() const
{
    double step = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->analysisType() == AnalysisType_Transient)
        {
            if (step == 0)
                step = Util::problem()->config()->timeStep().number();

            //TODO zatim moc nevim
            assert(step == Util::problem()->config()->timeStep().number());
        }
    }

    return step;
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

ostream& operator<<(ostream& output, const Block& id)
{
    output << "Block ";
    return output;
}


