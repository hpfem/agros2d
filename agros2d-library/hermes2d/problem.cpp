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
#include "problem_config.h"

#include "util/global.h"
#include "util/constants.h"

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
#include "coupling.h"
#include "solver.h"
#include "meshgenerator_triangle.h"
#include "meshgenerator_gmsh.h"
#include "logview.h"

#include "pythonlab/pythonengine.h"

CalculationThread::CalculationThread() : QThread()
{
}

void CalculationThread::startCalculation(CalculationType type)
{
    m_calculationType = type;
    start(QThread::TimeCriticalPriority);
}

void CalculationThread::run()
{
    switch (m_calculationType)
    {
    case CalculationType_Mesh:
        Agros2D::problem()->mesh();
        break;
    case CalculationType_Solve:
        Agros2D::problem()->solve(false, false);
        break;
    case CalculationType_SolveAdaptiveStep:
        Agros2D::problem()->solve(true, false);
        break;
    case CalculationType_SolveTimeStep:
        assert(0);
        break;
    default:
        assert(0);
    }
}

Problem::Problem()
{
    // m_timeStep = 0;
    m_lastTimeElapsed = QTime(0, 0);
    m_isSolving = false;
    m_isMeshing = false;
    m_abort = false;

    m_config = new ProblemConfig();
    m_setting = new ProblemSetting();
    m_calculationThread = new CalculationThread();

    m_isNonlinear = false;

    actMesh = new QAction(icon("scene-meshgen"), tr("&Mesh area"), this);
    actMesh->setShortcut(QKeySequence(tr("Alt+W")));
    connect(actMesh, SIGNAL(triggered()), this, SLOT(doMeshWithGUI()));

    actSolve = new QAction(icon("run"), tr("&Solve"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+S")));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolveWithGUI()));

    actSolveAdaptiveStep = new QAction(icon("run-step"), tr("Adaptive step"), this);
    connect(actSolveAdaptiveStep, SIGNAL(triggered()), this, SLOT(doSolveAdaptiveStepWithGUI()));

    connect(m_config, SIGNAL(changed()), this, SLOT(clearSolution()));
}

Problem::~Problem()
{
    clearSolution();
    clearFieldsAndConfig();

    delete m_config;
    delete m_setting;
    delete m_calculationThread;
}

bool Problem::isMeshed() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (!fieldInfo->initialMesh())
            return false;

    return (m_fieldInfos.size() > 0);
}

bool Problem::isSolved() const
{
    return !Agros2D::solutionStore()->isEmpty();
}

int Problem::numAdaptiveFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->adaptivityType() != AdaptivityType_None)
            num++;
    return num;
}

int Problem::numTransientFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Transient)
            num++;
    return num;
}

bool Problem::isTransient() const
{
    return numTransientFields() > 0;
}

bool Problem::isHarmonic() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Harmonic)
            return true;

    return false;
}

bool Problem::determineIsNonlinear() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->linearityType() != LinearityType_Linear)
            return true;

    return false;
}

void Problem::clearSolution()
{
    m_abort = false;

    // m_timeStep = 0;
    m_lastTimeElapsed = QTime(0, 0);
    m_timeStepLengths.clear();
    m_timeHistory.clear();

    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        fieldInfo->clearInitialMesh();

    Agros2D::solutionStore()->clearAll();

    // remove cache
    removeDirectory(cacheProblemDir());

    emit clearedSolution();
}

void Problem::clearFieldsAndConfig()
{
    clearSolution();

    foreach (Block* block, m_blocks)
        delete block;
    m_blocks.clear();

    // clear couplings
    foreach (CouplingInfo* couplingInfo, m_couplingInfos)
        delete couplingInfo;
    m_couplingInfos.clear();

    QMapIterator<QString, FieldInfo *> i(m_fieldInfos);
    while (i.hasNext())
    {
        i.next();
        removeField(i.value());
        delete i.value();
    }
    m_fieldInfos.clear();

    // clear config
    m_config->clear();
    m_setting->clear();
}

void Problem::addField(FieldInfo *field)
{
    clearSolution();

    // remove field
    if (hasField(field->fieldId()))
        removeField(m_fieldInfos[field->fieldId()]);

    // add to the collection
    m_fieldInfos[field->fieldId()] = field;

    // couplings
    synchronizeCouplings();

    emit fieldsChanged();
}

void Problem::removeField(FieldInfo *field)
{
    clearSolution();

    // first remove references to markers of this field from all edges and labels
    Agros2D::scene()->edges->removeFieldMarkers(field);
    Agros2D::scene()->labels->removeFieldMarkers(field);

    // then remove them from lists of markers - here they are really deleted
    Agros2D::scene()->boundaries->removeFieldMarkers(field);
    Agros2D::scene()->materials->removeFieldMarkers(field);

    // remove from the collection
    m_fieldInfos.remove(field->fieldId());

    synchronizeCouplings();

    currentPythonEngine()->runExpression(QString("agros2d.__remove_field__(\"%1\")").arg(field->fieldId()));

    emit fieldsChanged();
}

void Problem::createStructure()
{
    foreach (Block* block, m_blocks)
        delete block;
    m_blocks.clear();

    m_isNonlinear = determineIsNonlinear();

    synchronizeCouplings();
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > ci = couplingInfos();

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

    foreach (Block* block, m_blocks)
    {
        // todo: is released?
        block->setWeakForm(new WeakFormAgros<double>(block));
        block->weakForm()->registerForms();
    }
}

bool Problem::mesh(bool emitMeshed)
{
    bool result = false;

    // TODO: make global check geometry before mesh() and solve()
    if (Agros2D::problem()->fieldInfos().count() == 0)
    {
        Agros2D::log()->printError(tr("Mesh"), tr("No fields defined"));
        return false;
    }

    m_isMeshing = true;

    try
    {
        result = meshAction(emitMeshed);
    }
    catch (AgrosGeometryException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        Agros2D::log()->printError(tr("Geometry"), QString("%1").arg(e.what()));
        m_isMeshing = false;
        return false;
    }
    catch (AgrosMeshException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        Agros2D::log()->printError(tr("Mesh"), QString("%1").arg(e.what()));
        m_isMeshing = false;
        return false;
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        Agros2D::log()->printWarning(tr("Mesh"), e.what());
        m_isMeshing = false;
        return false;
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        Agros2D::log()->printWarning(tr("Mesh"), tr("An unknown exception occured and has been ignored"));
        qDebug() << "Mesh: An unknown exception occured and has been ignored";
        m_isMeshing = false;
        return false;
    }

    m_isMeshing = false;

    return result;
}

bool Problem::meshAction(bool emitMeshed)
{
    clearSolution();

    Agros2D::log()->printMessage(QObject::tr("Problem"), QObject::tr("Mesh generation"));

    // check geometry
    Agros2D::scene()->checkGeometryResult();
    Agros2D::scene()->checkGeometryAssignement();

    QSharedPointer<MeshGenerator> meshGenerator;
    switch (config()->meshType())
    {
    case MeshType_Triangle:
    case MeshType_Triangle_QuadFineDivision:
    case MeshType_Triangle_QuadRoughDivision:
    case MeshType_Triangle_QuadJoin:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorTriangle());
        break;
    case MeshType_GMSH_Triangle:
    case MeshType_GMSH_Quad:
    case MeshType_GMSH_QuadDelaunay_Experimental:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorGMSH());
        break;
    default:
        QMessageBox::critical(QApplication::activeWindow(), "Mesh generator error", QString("Mesh generator '%1' is not supported.").arg(meshTypeString(config()->meshType())));
        break;
    }

    if (meshGenerator && meshGenerator->mesh())
    {
        // load mesh
        try
        {
            readInitialMeshesFromFile(emitMeshed, meshGenerator.data()->xmldomain());
            return true;
        }
        catch (AgrosException& e)
        {
            throw AgrosMeshException(e.what());
        }
        catch (Hermes::Exceptions::Exception& e)
        {
            throw AgrosMeshException(e.what());
        }
    }

    return false;
}

double Problem::timeStepToTime(int timeStepIndex) const
{
    if (timeStepIndex == 0 || timeStepIndex == NOT_FOUND_SO_FAR)
        return 0.0;
    else
        return m_timeStepLengths[timeStepIndex - 1];
}

double Problem::timeStepToTotalTime(int timeStepIndex) const
{
    double time = 0;
    for(int ts = 0; ts < timeStepIndex; ts++)
        time += m_timeStepLengths[ts];

    return time;
}

int Problem::timeToTimeStep(double time) const
{
    if (time == 0)
        return 0;

    double timeSum = 0;
    for(int ts = 0; ts < m_timeStepLengths.size(); ts++)
    {
        timeSum += m_timeStepLengths.at(ts);
        if(fabs(timeSum - time) < 1e-9* config()->value(ProblemConfig::TimeTotal).toDouble())
            return ts+1;
    }

    // todo: revise
    return 0;
    assert(0);
}

bool Problem::defineActualTimeStepLength(double ts)
{
    // todo: do properly
    const double eps = 1e-9 * config()->value(ProblemConfig::TimeTotal).toDouble();
    assert(actualTime() < config()->value(ProblemConfig::TimeTotal).toDouble() + eps);
    if(actualTime() > config()->value(ProblemConfig::TimeTotal).toDouble() - eps)
    {
        return false;
    }
    else
    {
        double alteredTS = min(ts, config()->value(ProblemConfig::TimeTotal).toDouble() - actualTime());
        m_timeStepLengths.push_back(alteredTS);
        return true;
    }
}

bool Problem::skipThisTimeStep(Block *block)
{
    if(actualTime() == 0)
        return false;
    double timeSkip = block->timeSkip();
    double lastTime = Agros2D::solutionStore()->lastTime(block);

    return lastTime + timeSkip > actualTime();
}

void Problem::refuseLastTimeStepLength()
{
    m_timeStepLengths.removeLast();
}

double Problem::actualTime() const
{
    return timeStepToTotalTime(m_timeStepLengths.size());
}

double Problem::actualTimeStepLength() const
{
    if(m_timeStepLengths.isEmpty())
        return config()->constantTimeStepLength();

    return m_timeStepLengths.last();
}

void Problem::solveInit(bool reCreateStructure)
{
    m_timeStepLengths.clear();
    m_timeHistory.clear();

    if (fieldInfos().isEmpty())
    {
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("No fields defined"));
        throw AgrosSolverException(tr("No field defined"));
    }

    // check problem settings
    if (Agros2D::problem()->isTransient())
    {
        if (!Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() > 0.0)
            throw AgrosSolverException(tr("Total time is zero"));
        if (!Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble() > 0.0)
            throw AgrosSolverException(tr("Time method tolerance is zero"));
    }

    // open indicator progress
    Indicator::openProgress();

    // control geometry
    Agros2D::scene()->checkGeometryResult();
    Agros2D::scene()->checkGeometryAssignement();

    // save problem
    try
    {
        Agros2D::scene()->writeToFile(tempProblemFileName() + ".a2d", false);
    }
    catch (AgrosException &e)
    {
        Agros2D::log()->printError(tr("Problem"), e.toString());
    }

    // todo: we should not mesh always, but we would need to refine signals to determine when is it neccesary
    // (whether, e.g., parameters of the mesh have been changed)
    if (!mesh(false))
        throw AgrosSolverException(tr("Could not create mesh"));

    if (reCreateStructure || m_blocks.isEmpty())
    {
        createStructure();
    }
}

void Problem::doAbortSolve()
{
    m_abort = true;
    Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Aborting calculation..."));
}

void Problem::solve()
{    
    m_calculationThread->startCalculation(CalculationThread::CalculationType_Solve);
}

void Problem::solveAdaptiveStep()
{
    m_calculationThread->startCalculation(CalculationThread::CalculationType_SolveAdaptiveStep);
}

void Problem::solve(bool adaptiveStepOnly, bool commandLine)
{
    if (isMeshing() || isSolving())
        return;

    // clear solution
    clearSolution();

    if (numTransientFields() > 1)
    {
        Agros2D::log()->printError(tr("Solver"), tr("Coupling of more transient fields not possible at the moment."));
        return;
    }

    if ((m_fieldInfos.size() > 1) && isTransient() && (numAdaptiveFields() >= 1))
    {
        Agros2D::log()->printError(tr("Solver"), tr("Space adaptivity for transient coupled problems not possible at the moment."));
        return;
    }

    if (isTransient() && config()->isTransientAdaptive() && (numAdaptiveFields() >= 1))
    {
        Agros2D::log()->printError(tr("Solver"), tr("Both space and time adaptivity at the same time not possible at the moment."));
        return;
    }

    if( isTransient() && config()->isTransientAdaptive() && (config()->value(ProblemConfig::TimeOrder).toInt() == 1))
    {
        Agros2D::log()->printError(tr("Solver"), tr("Select higher order of time method to use adaptivity."));
        return;
    }

    if (Agros2D::problem()->fieldInfos().isEmpty())
    {
        Agros2D::log()->printError(tr("Solver"), tr("No fields defined"));
        return;
    }

    if (Agros2D::configComputer()->saveMatrixRHS)
        Agros2D::log()->printWarning(tr("Solver"), tr("Matrix and RHS will be saved on the disk and this will slow down the calculation. You may disable it in appllication settings."));

    try
    {
        Hermes::HermesCommonApi.set_integral_param_value(Hermes::numThreads, Agros2D::configComputer()->numberOfThreads);

        m_lastTimeElapsed = QTime();
        QTime timeCounter = QTime();
        timeCounter.start();

        m_isSolving = true;

        if (adaptiveStepOnly)
            solveAdaptiveStepAction();
        else
            solveAction();

        m_lastTimeElapsed = milisecondsToTime(timeCounter.elapsed());

        // elapsed time
        Agros2D::log()->printDebug(QObject::tr("Solver"), QObject::tr("Elapsed time: %1 s").arg(m_lastTimeElapsed.toString("mm:ss.zzz")));

        // delete temp file
        if (config()->fileName() == tempProblemFileName() + ".a2d")
        {
            QFile::remove(config()->fileName());
            config()->setFileName("");
        }

        m_abort = false;
        m_isSolving = false;

        if (!commandLine)
        {
            emit solved();

            // close indicator progress
            Indicator::closeProgress();
        }
    }
    catch (Hermes::Exceptions::NonlinearException &e)
    {
        Hermes::Solvers::NonlinearConvergenceState convergenceState = e.get_exception_state();
        switch(convergenceState)
        {
        case Hermes::Solvers::NotConverged:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Newton solver did not converge."));
            break;
        }
        case Hermes::Solvers::BelowMinDampingCoeff:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Damping coefficient below minimum."));
            break;
        }
        case Hermes::Solvers::AboveMaxAllowedResidualNorm:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Residual norm exceeded limit."));
            break;
        }
        case Hermes::Solvers::AboveMaxIterations:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Number of iterations exceeded limit."));
            break;
        }
        case Hermes::Solvers::Error:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("An error occured in Newton solver."));
            break;
        }
        default:
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Newton solver failed from unknown reason."));
        }

        m_isSolving = false;
        return;
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), QString("%1").arg(e.what()));
        m_isSolving = false;
        return;
    }
    catch (AgrosGeometryException& e)
    {
        Agros2D::log()->printError(QObject::tr("Geometry"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosSolverException& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosException& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not save at all
        m_isSolving = false;
        Agros2D::log()->printError(tr("Solver"), tr("An unknown exception occured in solver and has been ignored"));
        qDebug() << "Solver: An unknown exception occured and has been ignored";
        return;
    }
}

//adaptivity step: from 0, if no adaptivity, than 0
//time step: from 0 (initial condition), if block is not transient, calculate allways (todo: timeskipping)
//if no block transient, everything in timestep 0

void Problem::solveAction()
{
    // clear solution
    clearSolution();

    solveInit();

    assert(isMeshed());

    QMap<Block*, QSharedPointer<ProblemSolver<double> > > solvers;

    Agros2D::log()->printMessage(QObject::tr("Problem"), QObject::tr("Solving problem"));

    foreach (Block* block, m_blocks)
    {
        QSharedPointer<ProblemSolver<double> > solver = block->prepareSolver();
        if (solver.isNull())
            throw AgrosSolverException(tr("Cannot create solver."));

        solvers[block] = solver;
        solvers[block].data()->createInitialSpace();
    }

    TimeStepInfo nextTimeStep(config()->initialTimeStepLength());
    bool doNextTimeStep = true;
    do
    {
        foreach (Block* block, m_blocks)
        {
            if (block->isTransient() && (actualTimeStep() == 0))
            {
                solvers[block]->solveInitialTimeStep();
            }
            else if(!skipThisTimeStep(block))
            {
                stepMessage(block);
                if (block->adaptivityType() == AdaptivityType_None)
                {
                    // no adaptivity
                    solvers[block]->solveSimple(actualTimeStep(), 0);
                }
                else
                {
                    // adaptivity
                    int adaptStep = 1;
                    bool doContinueAdaptivity = true;
                    while (doContinueAdaptivity && (adaptStep <= block->adaptivitySteps()) && !m_abort)
                    {
                        // solve problem
                        solvers[block]->solveReferenceAndProject(actualTimeStep(), adaptStep - 1);
                        // create adapted space
                        doContinueAdaptivity = solvers[block]->createAdaptedSpace(actualTimeStep(), adaptStep);

                        // Python callback
                        foreach (Field *field, block->fields())
                        {
                            QString command = QString("(agros2d.field(\"%1\").adaptivity_callback(%2) if (agros2d.field(\"%1\").adaptivity_callback is not None and hasattr(agros2d.field(\"%1\").adaptivity_callback, '__call__')) else True)").
                                    arg(field->fieldInfo()->fieldId()).
                                    arg(adaptStep - 1);

                            double cont = 1.0;
                            bool successfulRun = currentPythonEngine()->runExpression(command, &cont);
                            if (!successfulRun)
                            {
                                ErrorResult result = currentPythonEngine()->parseError();
                                Agros2D::log()->printError(QObject::tr("Adaptivity callback"), result.error());
                            }

                            if (!cont)
                                doContinueAdaptivity = false;
                            break;
                        }

                        adaptStep++;
                    }
                }

                // TODO: it should be estimated in the first step as well
                // TODO: what if more blocks are transient? (take minimum? )

                // TODO: space + time adaptivity
                if (block->isTransient() && (actualTimeStep() >= 1))
                {
                    nextTimeStep = solvers[block]->estimateTimeStepLength(actualTimeStep(), 0);

                    //save actual time and indicator, whether calculation on this time was refused
                    m_timeHistory.push_back(QPair<double, bool>(actualTime(), nextTimeStep.refuse));
                    //qDebug() << nextTimeStep.length << ", " << actualTime() << ", " << nextTimeStep.refuse;
                }
            }
        }

        doNextTimeStep = false;
        if (isTransient())
        {
            if (nextTimeStep.refuse)
            {
                cout << "removing solutions on time step " << actualTimeStep() << endl;
                Agros2D::solutionStore()->removeTimeStep(actualTimeStep());
                refuseLastTimeStepLength();
            }
            else
            {
                // Python callback
                if (actualTimeStep() > 0)
                {
                    QString command = QString("(agros2d.problem().time_callback(%1) if (agros2d.problem().time_callback is not None and hasattr(agros2d.problem().time_callback, '__call__')) else True)").
                            arg(actualTimeStep());

                    double cont = 1.0;
                    bool successfulRun = currentPythonEngine()->runExpression(command, &cont);
                    if (!successfulRun)
                    {
                        ErrorResult result = currentPythonEngine()->parseError();
                        Agros2D::log()->printError(QObject::tr("Transient callback"), result.error());
                    }

                    if (!cont)
                    {
                        doNextTimeStep = false;
                        break;
                    }
                }
            }

            if (!doNextTimeStep)
                doNextTimeStep = defineActualTimeStepLength(nextTimeStep.length);
        }
    } while (doNextTimeStep && !m_abort);
}

void Problem::solveAdaptiveStepAction()
{
    solveInit(false);

    assert(isMeshed());

    Agros2D::log()->printMessage(QObject::tr("Problem"), QObject::tr("Solving problem"));

    assert(actualTimeStep() == 0);
    assert(m_blocks.size() == 1);
    Block* block = m_blocks.at(0);
    QSharedPointer<ProblemSolver<double> > solver = block->prepareSolver();
    if (solver.isNull())
        throw AgrosSolverException(tr("Cannot create solver."));

    int adaptStep = Agros2D::solutionStore()->lastAdaptiveStep(block, SolutionMode_Normal, 0);
    int adaptStepReference = Agros2D::solutionStore()->lastAdaptiveStep(block, SolutionMode_Reference, 0);

    if (adaptStep == NOT_FOUND_SO_FAR)
    {
        // it does not exist, problem has not been solved yet
        adaptStep = 0;
        solver.data()->createInitialSpace();
        solver.data()->solveReferenceAndProject(0, adaptStep);
    }
    else
    {
        solver.data()->resumeAdaptivityProcess(adaptStep);
        if(adaptStepReference == NOT_FOUND_SO_FAR)
        {
            // previously simple solve was used
            BlockSolutionID sidRemove(block, 0, 0, SolutionMode_Normal);
            Agros2D::solutionStore()->removeSolution(sidRemove);
            solver.data()->solveReferenceAndProject(0, adaptStep);
        }
        else
        {
            // previously solveAdaptiveStep was used
            assert(adaptStep == adaptStepReference);
        }
    }

    solver.data()->createAdaptedSpace(0, adaptStep + 1, true);
    solver.data()->solveReferenceAndProject(0, adaptStep + 1);
}

void Problem::stepMessage(Block* block)
{
    // log analysis
    QString fields;
    foreach(Field *field, block->fields())
        fields += field->fieldInfo()->fieldId() + ", ";
    fields = fields.left(fields.length() - 2);

    if (block->isTransient())
    {
        if(config()->isTransientAdaptive())
        {
            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(fields),
                                         QObject::tr("Transient step %1 (%2%)").
                                         arg(actualTimeStep()).
                                         arg(int(100*actualTime() / config()->value(ProblemConfig::TimeTotal).toDouble())));
        }
        else
        {
            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(fields),
                                         QObject::tr("Transient step %1/%2").
                                         arg(actualTimeStep()).
                                         arg(config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()));
        }
    }
    else
    {
        if (block->fields().count() == 1)
            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(fields), QObject::tr("Field solving (single analysis)"));
        else
            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(fields), QObject::tr("Fields solving (coupled analysis)"));
    }
}


void Problem::readInitialMeshesFromFile(bool emitMeshed, std::auto_ptr<XMLSubdomains::domain> xmldomain)
{
    // load initial mesh file
    // prepare mesh array
    Hermes::vector<Hermes::Hermes2D::MeshSharedPtr> meshesVector;
    QMap<FieldInfo *, Hermes::Hermes2D::MeshSharedPtr> meshes;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
    {
        Hermes::Hermes2D::MeshSharedPtr mesh(new Hermes::Hermes2D::Mesh());

        meshesVector.push_back(mesh);
        // cache
        meshes[fieldInfo] = mesh;
    }

    Hermes::HermesCommonApi.set_integral_param_value(Hermes::checkMeshesOnLoad, false);

    Hermes::Hermes2D::MeshReaderH2DXML meshloader;
    if (!xmldomain.get())
    {
        Agros2D::log()->printMessage(tr("Problem"), tr("Loading initial mesh from disk"));

        // read from file
        // save locale
        char *plocale = setlocale (LC_NUMERIC, "");
        setlocale (LC_NUMERIC, "C");

        // load mesh from file
        QString fileName = QFileInfo(cacheProblemDir() + "/initial.msh").absoluteFilePath();
        meshloader.set_validation(false);
        meshloader.load(fileName.toStdString().c_str(), meshesVector);

        // set system locale
        setlocale(LC_NUMERIC, plocale);
    }
    else
    {
        Agros2D::log()->printMessage(tr("Problem"), tr("Reading initial mesh from memory"));

        // read from memory
        std::auto_ptr<XMLSubdomains::domain> xml(xmldomain);
        meshloader.load(xml, meshesVector);
    }

    QSet<int> boundaries;
    foreach (FieldInfo *fieldInfo, m_fieldInfos)
    {
        Hermes::Hermes2D::MeshSharedPtr mesh = meshes[fieldInfo];

        // check that all boundary edges have a marker assigned
        for (int i = 0; i < mesh->get_max_node_id(); i++)
        {
            Hermes::Hermes2D::Node *node = mesh->get_node(i);

            if ((node->used == 1 && node->ref < 2 && node->type == 1))
            {
                int marker = atoi(mesh->get_boundary_markers_conversion().get_user_marker(node->marker).marker.c_str());

                assert(marker >= 0 || marker == -999);

                if (marker >= Agros2D::scene()->edges->count())
                    throw AgrosException(tr("Marker index is out of range."));

                if (marker >= 0 && Agros2D::scene()->edges->at(marker)->marker(fieldInfo) == SceneBoundaryContainer::getNone(fieldInfo))
                    boundaries.insert(marker);
            }
        }

        if (boundaries.count() > 0)
        {
            QString markers;
            foreach (int marker, boundaries)
                markers += QString::number(marker) + ", ";
            markers = markers.left(markers.length() - 2);

            throw AgrosException(QObject::tr("Mesh reader (%1): boundary edges '%2' does not have a boundary condition").
                                 arg(fieldInfo->fieldId()).
                                 arg(markers));

            // delete meshes
            meshes.clear();
            meshesVector.clear();

            return;
        }
        boundaries.clear();

        // refine mesh
        fieldInfo->refineMesh(mesh, true, true, true);

        // set initial mesh
        fieldInfo->setInitialMesh(mesh);
    }

    meshes.clear();
    meshesVector.clear();

    if (emitMeshed)
        emit meshed();
}

void Problem::readSolutionsFromFile()
{
    Agros2D::log()->printMessage(tr("Problem"), tr("Loading spaces and solutions from disk"));

    if (QFile::exists(QString("%1/runtime.xml").arg(cacheProblemDir())))
    {
        // load structure
        Agros2D::solutionStore()->loadRunTimeDetails();
        createStructure();
        foreach(Block* block, m_blocks)
        {
            block->createBoundaryConditions();
        }

        // emit solve
        emit solved();
    }
}

void Problem::synchronizeCouplings()
{
    bool changed = false;

    // add missing
    foreach (FieldInfo* sourceField, m_fieldInfos)
    {
        foreach (FieldInfo* targetField, m_fieldInfos)
        {
            if (sourceField == targetField)
                continue;

            if (couplingList()->isCouplingAvailable(sourceField, targetField))
            {
                QPair<FieldInfo*, FieldInfo*> fieldInfosPair(sourceField, targetField);

                if (!m_couplingInfos.keys().contains(fieldInfosPair))
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
              couplingList()->isCouplingAvailable(couplingInfo->sourceField(), couplingInfo->targetField())))
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

void Problem::doMeshWithGUI()
{
    LogDialog *logDialog = new LogDialog(QApplication::activeWindow(), tr("Mesh"));
    logDialog->show();

    // create mesh
    mesh();
    if (isMeshed())
    {
        // successful run
        logDialog->close();
    }
}

void Problem::doSolveWithGUI()
{
    LogDialog *logDialog = new LogDialog(QApplication::activeWindow(), tr("Solver"));
    logDialog->show();

    // solve problem
    solve();
}

void Problem::doSolveAdaptiveStepWithGUI()
{
    LogDialog *logDialog = new LogDialog(QApplication::activeWindow(), tr("Adaptive step"));
    logDialog->show();

    // solve problem
    solveAdaptiveStep();
}

QString Problem::timeUnit()
{
    if(fieldInfos().size() == 1)
        return fieldInfos().begin().value()->value(FieldInfo::TimeUnit).toString();
    else
        return "s";
}
