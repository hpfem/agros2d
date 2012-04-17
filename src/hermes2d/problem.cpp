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

#include "problem.h"

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
#include "meshgenerator.h"
#include "logview.h"

Field::Field(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{

}

bool Field::solveInitVariables()
{
    //TODO transient
    //
    //
    //    // transient
    //    if (Util::problem()->config()->analysisType() == AnalysisType_Transient)
    //    {
    //        if (!Util::problem()->config()->timeStep.evaluate()) return false;
    //        if (!Util::problem()->config()->timeTotal.evaluate()) return false;
    //        if (!Util::problem()->config()->initialCondition.evaluate()) return false;
    //    }

    if(!Util::scene()->boundaries->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    if(!Util::scene()->materials->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    return true;
}

Block::Block(QList<FieldInfo *> fieldInfos, QList<CouplingInfo*> couplings, ProgressItemSolve* progressItemSolve, Problem* parent) :
    m_progressItemSolve(progressItemSolve), m_couplings(couplings), m_parentProblem(parent)
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

    solver->init(m_progressItemSolve, m_wf, this);

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
    AdaptivityType at = m_fields.at(0)->fieldInfo()->adaptivityType;

    foreach (Field *field, m_fields)
    {
        assert(field->fieldInfo()->adaptivityType == at);
    }

    return at;
}

int Block::adaptivitySteps() const
{
    int as = m_fields.at(0)->fieldInfo()->adaptivitySteps;

    foreach (Field *field, m_fields)
    {
        assert(field->fieldInfo()->adaptivitySteps == as);
    }

    return as;
}

double Block::adaptivityTolerance() const
{
    double at = m_fields.at(0)->fieldInfo()->adaptivityTolerance;

    foreach (Field *field, m_fields)
    {
        assert(field->fieldInfo()->adaptivityTolerance == at);
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
        if(fieldInfo->linearityType == LinearityType_Linear)
            linear++;
        if(fieldInfo->linearityType == LinearityType_Newton)
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
        if(fieldInfo->nonlinearTolerance < tolerance)
            tolerance = fieldInfo->nonlinearTolerance;
    }

    return tolerance;
}

int Block::nonlinearSteps() const
{
    int steps = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->nonlinearSteps > steps)
            steps = fieldInfo->nonlinearSteps;
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



ProblemConfig::ProblemConfig(QWidget *parent) : QObject(parent)
{
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
    m_timeStep = Value("1.0", false);
    m_timeTotal = Value("1.0", false);
}

FieldInfo::FieldInfo(QString fieldId)
{
    m_module = NULL;

    if (fieldId.isEmpty())
    {
        // default
        // read default field (Util::config() is not set)
        QSettings settings;
        m_fieldId = settings.value("General/DefaultPhysicField", "electrostatic").toString();

        bool check = false;
        std::map<std::string, std::string> modules = availableModules();
        for (std::map<std::string, std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
            if (m_fieldId.toStdString() == it->first)
            {
                check = true;
                break;
            }
        if (!check)
            m_fieldId = "electrostatic";
    }
    else
    {
        m_fieldId = fieldId;
    }

    clear();
}

FieldInfo::~FieldInfo()
{
    if (m_module) delete m_module;
}

void FieldInfo::clear()
{
    // module object
    setAnalysisType(AnalysisType_SteadyState);

    numberOfRefinements = 1;
    polynomialOrder = 2;
    adaptivityType = AdaptivityType_None;
    adaptivitySteps = 0;
    adaptivityTolerance = 1.0;

    initialCondition = Value("0.0", false);

    // weakforms
    weakFormsType = WeakFormsType_Interpreted;

    // linearity
    linearityType = LinearityType_Linear;
    nonlinearTolerance = 1e-3;
    nonlinearSteps = 10;
}

void FieldInfo::setAnalysisType(AnalysisType analysisType)
{
    m_analysisType = analysisType;

    if (m_module) delete m_module;
    m_module = moduleFactory(m_fieldId.toStdString(),
                             Util::problem()->config()->coordinateType(),
                             m_analysisType);
}

Problem::Problem()
{
    m_timeStep = 0;
    m_isSolved = false;
    m_isSolving = false;

    m_config = new ProblemConfig();

    connect(m_config, SIGNAL(changed()), this, SLOT(clearSolution()));
}

Problem::~Problem()
{
    clearSolution();
    clearFieldsAndConfig();

    delete m_config;
}

Hermes::Hermes2D::Mesh* Problem::activeMeshInitial()
{
    return meshInitial(Util::scene()->activeViewField());
}

void Problem::clearSolution()
{
    if (Util::problem()->isSolved())
        Util::solutionStore()->clearAll();
    Util::solutionStore()->clearAll();

    foreach(Hermes::Hermes2D::Mesh* mesh, m_meshesInitial)
        if (mesh)
            delete mesh;
    m_meshesInitial.clear();

    m_timeStep = 0;
    m_isSolved = false;
    m_isSolving = false;
}

void Problem::clearFieldsAndConfig()
{
    // clear couplings
    foreach (CouplingInfo* couplingInfo, m_couplingInfos)
        delete couplingInfo;
    m_couplingInfos.clear();

    QMapIterator<QString, FieldInfo *> i(m_fieldInfos);
    while (i.hasNext())
    {
        i.next();
        delete i.value();
    }
    m_fieldInfos.clear();

    // clear config
    m_config->clear();
}

void Problem::addField(FieldInfo *field)
{
    // add to the collection
    m_fieldInfos[field->fieldId()] = field;

    // couplings
    synchronizeCouplings();

    emit fieldsChanged();
}

void Problem::removeField(FieldInfo *field)
{
    // first remove references to markers of this field from all edges and labels
    Util::scene()->edges->removeFieldMarkers(field);
    Util::scene()->labels->removeFieldMarkers(field);

    // then remove them from lists of markers - here they are really deleted
    Util::scene()->boundaries->removeFieldMarkers(field);
    Util::scene()->materials->removeFieldMarkers(field);

    // remove from the collection
    m_fieldInfos.remove(field->fieldId());

    synchronizeCouplings();

    emit fieldsChanged();
}

const bool REVERSE_ORDER_IN_BLOCK_DEBUG_REMOVE = false;

void Problem::createStructure()
{
    foreach(Block* block, m_blocks)
    {
        delete block;
    }
    m_blocks.clear();

    Util::problem()->synchronizeCouplings();

    //copy lists, items will be removed from them
    QList<FieldInfo *> fieldInfos = Util::problem()->fieldInfos().values();
    QList<CouplingInfo* > couplingInfos = Util::problem()->couplingInfos().values();

    while (!fieldInfos.empty()){
        QList<FieldInfo*> blockFieldInfos;
        QList<CouplingInfo*> blockCouplingInfos;

        //first find one field, that is not weakly coupled and dependent on other fields
        bool dependent;
        foreach (FieldInfo* fi, fieldInfos)
        {
            dependent = false;

            foreach (CouplingInfo* ci, couplingInfos)
            {
                if(ci->isWeak() && (ci->targetField() == fi) && fieldInfos.contains(ci->sourceField()))
                    dependent = true;
            }

            // this field is not weakly dependent, we can put it into this block
            if(! dependent){
                blockFieldInfos.push_back(fi);
                fieldInfos.removeOne(fi);
                break;
            }
        }
        assert(! dependent);

        // find hardly coupled fields to construct block
        bool added = true;
        while(added)
        {
            added = false;

            // first check whether there is related coupling
            foreach (CouplingInfo* checkedCouplingInfo, couplingInfos)
            {
                foreach (FieldInfo* checkedFieldInfo, blockFieldInfos)
                {
                    if(checkedCouplingInfo->isHard() && checkedCouplingInfo->isRelated(checkedFieldInfo))
                    {
                        //this coupling is related, add it to the block
                        added = true;
                        blockCouplingInfos.push_back(checkedCouplingInfo);
                        couplingInfos.removeOne(checkedCouplingInfo);
                    }
                }
            }

            // check for fields related to allready included couplings
            foreach (FieldInfo* checkedFieldInfo, fieldInfos)
            {
                foreach (CouplingInfo* checkedCouplingInfo, blockCouplingInfos)
                {
                    if(checkedCouplingInfo->isHard() && checkedCouplingInfo->isRelated(checkedFieldInfo))
                    {
                        //this field is related (by this coupling)
                        added = true;

                        //TODO for debugging only
                        if(REVERSE_ORDER_IN_BLOCK_DEBUG_REMOVE)
                            blockFieldInfos.push_front(checkedFieldInfo);
                        else
                            blockFieldInfos.push_back(checkedFieldInfo);

                        fieldInfos.removeOne(checkedFieldInfo);
                    }
                }
            }
        }

        // now all hard-coupled fields are here, create block
        m_blocks.append(new Block(blockFieldInfos, blockCouplingInfos, m_progressItemSolve, this));
    }

}

bool Problem::mesh()
{
    clearSolution();

    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("mesh generation"));

    MeshGeneratorTriangle pim;
    if (pim.mesh())
    {
        emit meshed();
        return true;
    }

    return false;
}

void Problem::solve()
{
    if (isSolving()) return;

    clearSolution();
    m_isSolving = true;
    bool isError = false;

    // open indicator progress
    Indicator::openProgress();

    // save problem
    ErrorResult result = Util::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

    createStructure();

    if (!isMeshed())
        isError = !mesh();

    if (!isError)
    {
        assert(isMeshed());

        QMap<Block*, Solver<double>* > solvers;

        // check geometry
        if (!Util::scene()->checkGeometryAssignement())
            return;

        if (Util::problem()->fieldInfos().count() == 0)
        {
            Util::log()->printError(QObject::tr("Solver"), QObject::tr("no field defined."));
            return;
        }

        Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("solving problem"));

        Util::scene()->setActiveViewField(Util::problem()->fieldInfos().values().at(0));

        foreach (Block* block, m_blocks)
        {
            solvers[block] = block->prepareSolver();
        }

        foreach (Block* block, m_blocks)
        {
            Solver<double>* solver = solvers[block];

            if (block->isTransient())
            {
                if (solver->solveInitialTimeStep())
                {
                    for (int i = 0; i < block->numTimeSteps(); i++)
                        if (!solver->solveTimeStep(block->timeStep()))
                        {
                            isError = true;
                            break; // inner loop
                        }
                }
                else
                {
                    isError = true;
                }

                if (isError)
                    break; // block solver loop
            }
            else
            {
                if (block->adaptivityType() == AdaptivityType_None)
                {
                    if (!solver->solveSimple())
                    {
                        isError = true;
                        break; // block solver loop
                    }
                }
                else
                {
                    if (!solver->solveInitialAdaptivityStep(0))
                    {
                        isError = true;
                        break; // block solver loop
                    }
                    int adaptStep = 1;
                    bool continueSolve = true;
                    while (continueSolve && (adaptStep <= block->adaptivitySteps()))
                    {
                        continueSolve = solver->solveAdaptivityStep(0, adaptStep);
                        cout << "step " << adaptStep << " / " << block->adaptivitySteps() << ", continueSolve " << continueSolve << endl;
                        adaptStep++;
                    }
                }
            }

            if (!isError)
            {
                Util::scene()->setActiveTimeStep(Util::solutionStore()->lastTimeStep(Util::scene()->activeViewField(), SolutionType_Normal));
                Util::scene()->setActiveAdaptivityStep(Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionType_Normal));
                Util::scene()->setActiveSolutionType(SolutionType_Normal);
                cout << "setting active adapt step to " << Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionType_Normal) << endl;
            }
        }

        // delete temp file
        if (Util::problem()->config()->fileName() == tempProblemFileName() + ".a2d")
        {
            QFile::remove(Util::problem()->config()->fileName());
            Util::problem()->config()->setFileName("");
        }

        if (!isError)
        {
            m_isSolved = true;
            emit solved();
        }
    }

    m_isSolving = false;

    // close indicator progress
    Indicator::closeProgress();
}

void Problem::solveAdaptiveStep()
{

}

void Problem::synchronizeCouplings()
{
    CouplingInfo::synchronizeCouplings(m_fieldInfos, m_couplingInfos);
}

//*************************************************************************************************

const int notFoundSoFar = -999;

void SolutionStore::clearAll()
{
    m_multiSolutions.clear();
}

void SolutionStore::clearOne(FieldSolutionID solutionID)
{
    m_multiSolutions.remove(solutionID);
}

SolutionArray<double> SolutionStore::solution(FieldSolutionID solutionID, int component)
{
    return multiSolution(solutionID).component(component);
}

MultiSolutionArray<double> SolutionStore::multiSolution(FieldSolutionID solutionID)
{
    if(m_multiSolutions.contains(solutionID))
        return m_multiSolutions[solutionID];

    return MultiSolutionArray<double>();
}

bool SolutionStore::contains(FieldSolutionID solutionID)
{
    return m_multiSolutions.contains(solutionID);
}

MultiSolutionArray<double> SolutionStore::multiSolution(BlockSolutionID solutionID)
{
    MultiSolutionArray<double> msa;
    foreach(Field *field, solutionID.group->m_fields)
    {
        msa.append(multiSolution(solutionID.fieldSolutionID(field->fieldInfo())));
    }

    return msa;
}

void SolutionStore::saveSolution(FieldSolutionID solutionID,  MultiSolutionArray<double> multiSolution)
{
    assert(!m_multiSolutions.contains(solutionID));
    replaceSolution(solutionID, multiSolution);
}

void SolutionStore::removeSolution(FieldSolutionID solutionID)
{
    assert(m_multiSolutions.contains(solutionID));
    m_multiSolutions.remove(solutionID);
}

void SolutionStore::replaceSolution(FieldSolutionID solutionID,  MultiSolutionArray<double> multiSolution)
{
    cout << "$$$$$$$$  Saving solution " << solutionID << ", now solutions: " << m_multiSolutions.size() << ", time " << multiSolution.component(0).time << endl;
    assert(solutionID.timeStep >= 0);
    assert(solutionID.adaptivityStep >= 0);
    m_multiSolutions[solutionID] = multiSolution;
}

void SolutionStore::saveSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    foreach(Field* field, solutionID.group->m_fields)
    {
        FieldSolutionID fieldSID = solutionID.fieldSolutionID(field->fieldInfo());
        MultiSolutionArray<double> fieldMultiSolution = multiSolution.fieldPart(solutionID.group, field->fieldInfo());
        saveSolution(fieldSID, fieldMultiSolution);
    }
}

void SolutionStore::replaceSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    foreach(Field* field, solutionID.group->m_fields)
    {
        FieldSolutionID fieldSID = solutionID.fieldSolutionID(field->fieldInfo());
        MultiSolutionArray<double> fieldMultiSolution = multiSolution.fieldPart(solutionID.group, field->fieldInfo());
        replaceSolution(fieldSID, fieldMultiSolution);
    }
}

void SolutionStore::removeSolution(BlockSolutionID solutionID)
{
    foreach(Field* field, solutionID.group->m_fields)
    {
        FieldSolutionID fieldSID = solutionID.fieldSolutionID(field->fieldInfo());
        removeSolution(fieldSID);
    }
}

int SolutionStore::lastTimeStep(FieldInfo *fieldInfo, SolutionType solutionType)
{
    int timeStep = notFoundSoFar;
    foreach(FieldSolutionID sid, m_multiSolutions.keys())
    {
        if((sid.group == fieldInfo) && (sid.solutionType == solutionType) && (sid.timeStep > timeStep))
            timeStep = sid.timeStep;
    }

    return timeStep;
}

int SolutionStore::lastTimeStep(Block *block, SolutionType solutionType)
{
    int timeStep = lastTimeStep(block->m_fields.at(0)->fieldInfo(), solutionType);

    foreach(Field* field, block->m_fields)
    {
        assert(lastTimeStep(field->fieldInfo(), solutionType) == timeStep);
    }

    return timeStep;
}

double SolutionStore::lastTime(FieldInfo *fieldInfo)
{
    int timeStep = lastTimeStep(fieldInfo, SolutionType_Normal);
    double time = notFoundSoFar;

    foreach(FieldSolutionID id, m_multiSolutions.keys())
    {
        if((id.group == fieldInfo) && (id.timeStep == timeStep) && (id.exists()))
        {
            if(time == notFoundSoFar)
                time = m_multiSolutions[id].component(0).time;
            else
                assert(time == m_multiSolutions[id].component(0).time);
        }
    }
    assert(time != notFoundSoFar);
    return time;
}

double SolutionStore::lastTime(Block *block)
{
    double time = lastTime(block->m_fields.at(0)->fieldInfo());

    foreach(Field* field, block->m_fields)
    {
        assert(lastTime(field->fieldInfo()) == time);
    }

    return time;

}

int SolutionStore::lastAdaptiveStep(FieldInfo *fieldInfo, SolutionType solutionType, int timeStep)
{
    if(timeStep == -1)
        timeStep = lastTimeStep(fieldInfo, solutionType);

    int adaptiveStep = notFoundSoFar;
    foreach(FieldSolutionID sid, m_multiSolutions.keys())
    {
        if((sid.group == fieldInfo) && (sid.solutionType == solutionType) && (sid.timeStep == timeStep) && (sid.adaptivityStep > adaptiveStep))
            adaptiveStep = sid.adaptivityStep;
    }

    return adaptiveStep;
}

int SolutionStore::lastAdaptiveStep(Block *block, SolutionType solutionType, int timeStep)
{
    int adaptiveStep = lastAdaptiveStep(block->m_fields.at(0)->fieldInfo(), solutionType, timeStep);

    foreach(Field* field, block->m_fields)
    {
        assert(lastAdaptiveStep(field->fieldInfo(), solutionType, timeStep) == adaptiveStep);
    }

    return adaptiveStep;
}

FieldSolutionID SolutionStore::lastTimeAndAdaptiveSolution(FieldInfo *fieldInfo, SolutionType solutionType)
{
    FieldSolutionID solutionID;
    if(solutionType == SolutionType_Finer) {
        FieldSolutionID solutionIDNormal = lastTimeAndAdaptiveSolution(fieldInfo, SolutionType_Normal);
        FieldSolutionID solutionIDReference = lastTimeAndAdaptiveSolution(fieldInfo, SolutionType_Reference);
        if((solutionIDNormal.timeStep > solutionIDReference.timeStep) ||
                (solutionIDNormal.adaptivityStep > solutionIDReference.adaptivityStep))
        {
            solutionID = solutionIDNormal;
        }
        else
        {
            solutionID = solutionIDReference;
        }
    }
    else
    {
        solutionID.group = fieldInfo;
        solutionID.adaptivityStep = lastAdaptiveStep(fieldInfo, solutionType);
        solutionID.timeStep = lastTimeStep(fieldInfo, solutionType);
        solutionID.solutionType = solutionType;
    }

    return solutionID;
}

BlockSolutionID SolutionStore::lastTimeAndAdaptiveSolution(Block *block, SolutionType solutionType)
{
    FieldSolutionID fsid = lastTimeAndAdaptiveSolution(block->m_fields.at(0)->fieldInfo(), solutionType);
    BlockSolutionID bsid = fsid.blockSolutionID(block);


    foreach(Field* field, block->m_fields)
    {
        assert(bsid == lastTimeAndAdaptiveSolution(field->fieldInfo(), solutionType).blockSolutionID(block));
    }

    return bsid;
}

QList<double> SolutionStore::timeLevels(FieldInfo *fieldInfo)
{
    QList<double> list;

    foreach(FieldSolutionID fsid, m_multiSolutions.keys())
    {
        if(fsid.group == fieldInfo)
        {
            double time = m_multiSolutions[fsid].component(0).time;
            if(!list.contains(time))
                list.push_back(time);
        }
    }

    return list;
}
