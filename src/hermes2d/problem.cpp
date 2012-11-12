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

#include "field.h"
#include "block.h"
#include "problem.h"
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
    m_timeStepMethod = TimeStepMethod_BDF2AOrder;
    m_timeOrder = 2;
    m_timeMethodTolerance = Value("0.1", false);
    m_timeTotal = Value("1.0", false);
    m_numConstantTimeSteps = 10;
}

bool ProblemConfig::isTransientAdaptive() const
{
    if(m_timeStepMethod == TimeStepMethod_BDF2AOrder)
        return true;
    return false;
}

const double initialTimeStepRatio = 1000;
double ProblemConfig::initialTimeStepLength()
{
    if(isTransientAdaptive())
        return timeTotal().value() / initialTimeStepRatio;
    else
        return constantTimeStepLength();
}

Problem::Problem()
{
    m_timeStep = 0;
    m_lastTimeElapsed = QTime(0, 0);
    m_isSolved = false;
    m_isSolving = false;

    m_config = new ProblemConfig();

    connect(m_config, SIGNAL(changed()), this, SLOT(clearSolution()));

    actClearSolutions = new QAction(icon(""), tr("Clear solutions"), this);
    actClearSolutions->setStatusTip(tr("Clear solutions"));
    connect(actClearSolutions, SIGNAL(triggered()), this, SLOT(clearSolution()));       
}

Problem::~Problem()
{
    clearSolution();
    clearFieldsAndConfig();

    delete m_config;
}

QSharedPointer<Hermes::Hermes2D::Mesh> Problem::activeMeshInitial()
{
    return meshInitial(Util::scene()->activeViewField());
}

bool Problem::isTransient() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Transient)
            return true;

    return false;
}

bool Problem::isHarmonic() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Harmonic)
            return true;

    return false;
}

bool Problem::isNonlinear() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->linearityType() != LinearityType_Linear)
            return true;

    return false;
}

void Problem::clearSolution()
{
    m_isSolved = false;
    m_isSolving = false;
    m_timeStep = 0;
    m_lastTimeElapsed = QTime(0, 0);
    m_timeStepLengths.clear();

    m_meshesInitial.clear();

    Util::solutionStore()->clearAll();
}

void Problem::clearFieldsAndConfig()
{
    clearSolution();

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

    m_isSolved = false;

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

void Problem::createStructure()
{
    foreach (Block* block, m_blocks)
        delete block;
    m_blocks.clear();

    synchronizeCouplings();

    //copy lists, items will be removed from them
    QList<FieldInfo *> fieldInfosCopy = fieldInfos().values();
    QList<CouplingInfo* > couplingInfosCopy = couplingInfos().values();

    while (!fieldInfosCopy.empty()){
        QList<FieldInfo*> blockFieldInfos;
        QList<CouplingInfo*> blockCouplingInfos;

        //first find one field, that is not weakly coupled and dependent on other fields
        bool dependent;
        foreach (FieldInfo* fieldInfo, fieldInfosCopy)
        {
            dependent = false;

            foreach (CouplingInfo* couplingInfo, couplingInfosCopy)
            {
                if (couplingInfo->isWeak() && (couplingInfo->targetField() == fieldInfo) && fieldInfosCopy.contains(couplingInfo->sourceField()))
                    dependent = true;
            }

            // this field is not weakly dependent, we can put it into this block
            if (!dependent){
                blockFieldInfos.push_back(fieldInfo);
                fieldInfosCopy.removeOne(fieldInfo);
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
            foreach (CouplingInfo* checkedCouplingInfo, couplingInfosCopy)
            {
                foreach (FieldInfo* checkedFieldInfo, blockFieldInfos)
                {
                    if(checkedCouplingInfo->isHard() && checkedCouplingInfo->isRelated(checkedFieldInfo))
                    {
                        //this coupling is related, add it to the block
                        added = true;
                        blockCouplingInfos.push_back(checkedCouplingInfo);
                        couplingInfosCopy.removeOne(checkedCouplingInfo);
                    }
                }
            }

            // check for fields related to allready included couplings
            foreach (FieldInfo* checkedFieldInfo, fieldInfosCopy)
            {
                foreach (CouplingInfo* checkedCouplingInfo, blockCouplingInfos)
                {
                    if(checkedCouplingInfo->isHard() && checkedCouplingInfo->isRelated(checkedFieldInfo))
                    {
                        //this field is related (by this coupling)
                        added = true;
                        blockFieldInfos.push_back(checkedFieldInfo);
                        fieldInfosCopy.removeOne(checkedFieldInfo);
                    }
                }
            }
        }

        // now all hard-coupled fields are here, create block
        m_blocks.append(new Block(blockFieldInfos, blockCouplingInfos));
    }

}

bool Problem::mesh()
{
    clearSolution();

    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("mesh generation"));

    MeshGenerator *pim = NULL;
    switch (config()->meshType())
    {
    case MeshType_Triangle:
    case MeshType_Triangle_QuadFineDivision:
    case MeshType_Triangle_QuadRoughDivision:
    case MeshType_Triangle_QuadJoin:
        pim = new MeshGeneratorTriangle();
        break;
    case MeshType_GMSH_Triangle:
    case MeshType_GMSH_Quad:
    case MeshType_GMSH_QuadDelaunay_Experimental:
        pim = new MeshGeneratorGMSH();
        break;
    default:
        QMessageBox::critical(QApplication::activeWindow(), "Mesh generator error", QString("Mesh generator '%1' is not supported.").arg(meshTypeString(config()->meshType())));
        break;
    }

    if (pim && pim->mesh())
    {
        emit meshed();
        delete pim;

        return true;
    }

    return false;
}

void Problem::solveInit()
{
    m_isSolving = true;
    m_timeStepLengths.clear();

    // open indicator progress
    Indicator::openProgress();

    // control geometry
    ErrorResult result = Util::scene()->checkGeometryResult();
    if (result.isError())
    {
        result.showDialog();
        m_isSolving = false;
        throw (AgrosSolverException("Geometry check failed"));
    }

    // save problem
    result = Util::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

    createStructure();

    if (!isMeshed())
    {
        if(!mesh())
            throw AgrosSolverException("Could not create mesh");
    }

    // check geometry
    if (!Util::scene()->checkGeometryAssignement())
        throw(AgrosSolverException("Geometry assignment failed"));

    if (fieldInfos().count() == 0)
    {
        Util::log()->printError(QObject::tr("Solver"), QObject::tr("no field defined."));
        throw AgrosSolverException("No field defined");
    }
}

double Problem::timeStepToTime(int timeStepIndex) const
{
    double time = 0;
    for(int ts = 0; ts < timeStepIndex; ts++)
        time += m_timeStepLengths[ts];

    return time;
}

int Problem::timeToTimeStep(double time) const
{
    if(time == 0)
        return 0;

    double timeSum = 0;
    for(int ts = 0; ts < m_timeStepLengths.size(); ts++)
    {
        timeSum += m_timeStepLengths.at(ts);
        if(fabs(timeSum - time) < 1e-9* config()->timeTotal().value())
            return ts+1;
    }

    // todo: revise
    return 0;
    assert(0);
}

bool Problem::defineActualTimeStepLength(double ts)
{
    // todo: do properly
    const double eps = 1e-9 * config()->timeTotal().value();
    assert(actualTime() < config()->timeTotal().value() + eps);
    if(actualTime() > config()->timeTotal().value() - eps)
        return false;
    else{
        double alteredTS = min(ts, config()->timeTotal().value() - actualTime());
        m_timeStepLengths.push_back(alteredTS);
        return true;
    }
}

bool Problem::skipThisTimeStep(Block *block)
{
    if(actualTime() == 0)
        return false;
    double timeSkip = block->timeSkip();
    double lastTime = Util::solutionStore()->lastTime(block);

    return lastTime + timeSkip > actualTime();
}

void Problem::refuseLastTimeStepLength()
{
    m_timeStepLengths.removeLast();
}

double Problem::actualTime() const
{
    return timeStepToTime(m_timeStepLengths.size());
}

double Problem::actualTimeStepLength() const
{
    if(m_timeStepLengths.isEmpty())
        return config()->constantTimeStepLength();

    return m_timeStepLengths.last();
}

void Problem::solveFinished()
{
    // delete temp file
    if (config()->fileName() == tempProblemFileName() + ".a2d")
    {
        QFile::remove(config()->fileName());
        config()->setFileName("");
    }

    m_isSolving = false;

    // close indicator progress
    Indicator::closeProgress();
}

void Problem::solve()
{
    if (isSolving())
        return;

    solveActionCatchExceptions(false);

    solveFinished();
}

//adaptivity step: from 0, if no adaptivity, than 0
//time step: from 0 (initial condition), if block is not transient, calculate allways (todo: timeskipping)
//if no block transient, everything in timestep 0

void Problem::solveAction()
{
    clearSolution();

    Util::scene()->blockSignals(true);

    Util::scene()->setActiveAdaptivityStep(0);
    Util::scene()->setActiveTimeStep(0);
    Util::scene()->setActiveViewField(fieldInfos().values().at(0));

    solveInit();

    assert(isMeshed());

    QMap<Block*, Solver<double>* > solvers;

    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("solving problem"));

    foreach (Block* block, m_blocks)
    {
        solvers[block] = block->prepareSolver();
        solvers[block]->createInitialSpace();
    }

    NextTimeStep nextTimeStep(config()->initialTimeStepLength());
    bool doNextTimeStep = true;
    while(doNextTimeStep)
    {
        foreach (Block* block, m_blocks)
        {
            Solver<double>* solver = solvers[block];
            if (block->isTransient() && (actualTimeStep() == 0))
            {
                solvers[block]->solveInitialTimeStep();
            }
            else if(!skipThisTimeStep(block))
            {
                if (block->adaptivityType() == AdaptivityType_None)
                {
                    // log analysis
                    QString fields;
                    foreach(Field *field, block->fields())
                        fields += field->fieldInfo()->fieldId() + ", ";
                    fields = fields.left(fields.length() - 2);

                    if (block->isTransient())
                    {
                        Util::log()->printMessage(QObject::tr("Solver (%1)").arg(fields),
                                                  QObject::tr("transient step %1/%2").
                                                  arg(actualTimeStep()).
                                                  arg(config()->numConstantTimeSteps()));
                    }
                    else
                    {
                        if (block->fields().count() == 1)
                            Util::log()->printMessage(QObject::tr("Solver (%1)").arg(fields), QObject::tr("single analysis"));
                        else
                            Util::log()->printMessage(QObject::tr("Solver (%1)").arg(fields), QObject::tr("coupled analysis"));
                    }

                    solver->solveSimple(actualTimeStep(), 0);
                }
                else
                {
                    //                    if(block->isTransient())
                    //                    {
                    //                        // pak vyuzit toho, ze mam vsechny adaptivni kroky z predchozi casove vrstvy
                    //                        // vezmu treba pred pred posledni adaptivni krok a tim budu mit derefinement
                    //                        QMessageBox::warning(QApplication::activeWindow(), "Solver Error", "Adaptivity not implemented for transient problems");
                    //                        return;
                    //                    }

                    int adaptStep = 1;
                    bool continueAdaptivity = true;
                    while (continueAdaptivity && (adaptStep <= block->adaptivitySteps()))
                    {
                        solver->solveReferenceAndProject(actualTimeStep(), adaptStep - 1, false);
                        continueAdaptivity = solver->createAdaptedSpace(actualTimeStep(), adaptStep);
                        adaptStep++;
                    }
                }

                // todo: it should be estimated in the first step as well
                // todo: what if more blocks are transient? (take minimum? )

                // todo: space + time adaptivity
                if (isTransient() && (actualTimeStep() >=1))
                    nextTimeStep = solver->estimateTimeStepLenght(actualTimeStep(), 0);

            }
        }

        doNextTimeStep = false;
        if(isTransient())
        {

//            nextTimeStep.refuse = (Util::problem()->actualTimeStep() > 2) && random() < RAND_MAX / 2;
//            nextTimeStep.length = Util::problem()->config()->constantTimeStepLength() / (1 + 10 * random() / double(RAND_MAX));

            if(nextTimeStep.refuse)
            {
                cout << "removing solutions on time step " << actualTimeStep() << endl;
                Util::solutionStore()->removeTimeStep(actualTimeStep());
                refuseLastTimeStepLength();
            }

            doNextTimeStep = defineActualTimeStepLength(nextTimeStep.length);
        }
    }

    Util::scene()->setActiveTimeStep(Util::solutionStore()->lastTimeStep(Util::scene()->activeViewField(), SolutionMode_Normal));
    Util::scene()->setActiveAdaptivityStep(Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal));
    Util::scene()->setActiveSolutionType(SolutionMode_Normal);

    Util::scene()->blockSignals(false);

    m_isSolved = true;
    emit solved();
}

void Problem::solveAdaptiveStep()
{
    if (isSolving())
        return;

    // todo: should be number of blocks, but they are not created at this moment. Anyway, hard coupling does not work at this moment, rewrite....
    //if(m_blocks.count() > 1)
    if(m_fieldInfos.count() > 1)
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", "This action is possible for one field only, unless they are hard-coupled");
        return;
    }

    if(isTransient())
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", "This action is not possible for transient problems");
        return;
    }

    solveActionCatchExceptions(true);

    solveFinished();
}

void Problem::solveAdaptiveStepAction()
{
    Util::scene()->blockSignals(true);

    solveInit();

    assert(isMeshed());


    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("solving problem"));

    Block* block = m_blocks.at(0);
    Solver<double>* solver = block->prepareSolver();

    int adaptStepNormal = Util::solutionStore()->lastAdaptiveStep(block, SolutionMode_Normal, 0);
    int adaptStepNonExisting = Util::solutionStore()->lastAdaptiveStep(block, SolutionMode_NonExisting, 0);
    int adaptStep = max(adaptStepNormal, adaptStepNonExisting);

    // it means that solution allready exists, but will be recalculated by adapt step
    bool solutionAlreadyExists = ((adaptStep >= 0) && (adaptStepNormal == adaptStep));

    // it does not exist, problem has not been solved yet
    if(adaptStep < 0)
    {
        Util::scene()->setActiveAdaptivityStep(0);
        Util::scene()->setActiveTimeStep(0);
        Util::scene()->setActiveViewField(fieldInfos().values().at(0));

        solver->createInitialSpace();
        adaptStep = 0;
    }

    // standard adaptivity process may end by calculation of refference or by creating adapted space
    // (depends on which stopping criteria is fulfilled). To avoid unnecessary calculations:
    bool hasReference = (Util::solutionStore()->lastAdaptiveStep(block, SolutionMode_Reference, 0) == adaptStep);
    if(!hasReference)
    {
        solver->solveReferenceAndProject(0, adaptStep, solutionAlreadyExists);
    }

    solver->createAdaptedSpace(0, adaptStep + 1);

    // only if solution in previous adapt step existed, solve new one (we would have two new adapt steps otherwise)
    if(solutionAlreadyExists || adaptStep == 0)
        solver->solveSimple(0, adaptStep + 1);


    Util::scene()->setActiveTimeStep(Util::solutionStore()->lastTimeStep(Util::scene()->activeViewField(), SolutionMode_Normal));
    Util::scene()->setActiveAdaptivityStep(Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal));
    Util::scene()->setActiveSolutionType(SolutionMode_Normal);
    //cout << "setting active adapt step to " << Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal) << endl;

    Util::scene()->blockSignals(false);

    m_isSolved = true;
    emit solved();
}

void Problem::solveActionCatchExceptions(bool adaptiveStepOnly)
{
    // load plugins
    QStringList modules = fieldInfos().keys();
    QStringList couplings;
    foreach (CouplingInfo *info, couplingInfos().values())
        couplings.append(info->couplingId());

    QStringList plugins;
    plugins.append(modules);
    plugins.append(couplings);

    try
    {
        Util::loadPlugins(plugins);
    }
    catch (AgrosException e)
    {
        Util::log()->printError(QObject::tr("Solver"), /*QObject::tr(*/QString("%1").arg(e.what()));
        return;
    }

    m_lastTimeElapsed = QTime(0, 0);
    QTime timeCounter = QTime(0, 0);
    timeCounter.start();

    try
    {
        if(adaptiveStepOnly)
            solveAdaptiveStepAction();
        else
            solveAction();
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        Util::log()->printError(QObject::tr("Solver"), /*QObject::tr(*/QString("%1").arg(e.what()));
        return;
    }
    catch (Hermes::Exceptions::Exception* e)
    {
        Util::log()->printError(QObject::tr("Solver"), /*QObject::tr(*/QString("%1").arg(e->what()));
        return;
    }
    catch (AgrosSolverException& e)
    {
        Util::log()->printError(QObject::tr("Solver"), /*QObject::tr(*/e.what());
        return;
    }
    // todo: somehow catch other exceptions - agros should not fail, but some message should be generated
    //                        catch (...)
    //                        {
    //                            // Util::log()->printError(tr("Problem"), QString::fromStdString(e.what()));
    //                            return;
    //                        }

    m_lastTimeElapsed = milisecondsToTime(timeCounter.elapsed());

    // warning: in the case of exception, the program will not reach this place
    // therefore the cleanup and stop of timeElapsed is done in solve / solveAdaptiveStep by calling solveFinished
}

void Problem::synchronizeCouplings()
{
    bool changed = false;

    // add missing
    foreach (FieldInfo* sourceField, m_fieldInfos)
    {
        foreach (FieldInfo* targetField, m_fieldInfos)
        {
            if(sourceField == targetField)
                continue;
            QPair<FieldInfo*, FieldInfo*> fieldInfosPair(sourceField, targetField);
            if (isCouplingAvailable(sourceField, targetField)){
                if (!m_couplingInfos.contains(fieldInfosPair))
                {
                    m_couplingInfos[fieldInfosPair] = new CouplingInfo(sourceField, targetField);

                    changed = true;
                }
            }
        }
    }

    // remove extra
    foreach (CouplingInfo* couplingInfo, m_couplingInfos)
    {
        if (!(m_fieldInfos.contains(couplingInfo->sourceField()->fieldId()) &&
              m_fieldInfos.contains(couplingInfo->targetField()->fieldId()) &&
              isCouplingAvailable(couplingInfo->sourceField(), couplingInfo->targetField())))
        {
            m_couplingInfos.remove(QPair<FieldInfo*, FieldInfo*>(couplingInfo->sourceField(), couplingInfo->targetField()));

            changed = true;
        }
    }

    if (changed)
        emit couplingsChanged();
}

Block* Problem::blockOfField(FieldInfo *fieldInfo) const
{
    foreach(Block* block, m_blocks)
    {
        if(block->contains(fieldInfo))
            return block;
    }
    return NULL;
}
