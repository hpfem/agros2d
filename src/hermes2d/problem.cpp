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
#include "module_agros.h"
#include "coupling.h"
#include "solver.h"
#include "meshgenerator_triangle.h"
#include "meshgenerator_gmsh.h"
#include "logview.h"

Problem::Problem()
{
    // m_timeStep = 0;
    m_lastTimeElapsed = QTime(0, 0);
    m_isSolved = false;
    m_isSolving = false;

    m_config = new ProblemConfig();
    m_configView = new ProblemConfigView();

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
    delete m_configView;
}

bool Problem::isMeshed() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->initialMesh())
            return true;

    return false;
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
    // m_timeStep = 0;
    m_lastTimeElapsed = QTime(0, 0);
    m_timeStepLengths.clear();

    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        fieldInfo->clearInitialMesh();

    Agros2D::solutionStore()->clearAll();

    // remove cache
    removeDirectory(cacheProblemDir());
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
    m_configView->clear();
}

void Problem::addField(FieldInfo *field)
{
    // remove field
    if (hasField(field->fieldId()))
        removeField(m_fieldInfos[field->fieldId()]);

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
    Agros2D::scene()->edges->removeFieldMarkers(field);
    Agros2D::scene()->labels->removeFieldMarkers(field);

    // then remove them from lists of markers - here they are really deleted
    Agros2D::scene()->boundaries->removeFieldMarkers(field);
    Agros2D::scene()->materials->removeFieldMarkers(field);

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

    Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("mesh generation"));

    MeshGenerator *meshGenerator = NULL;
    switch (config()->meshType())
    {
    case MeshType_Triangle:
    case MeshType_Triangle_QuadFineDivision:
    case MeshType_Triangle_QuadRoughDivision:
    case MeshType_Triangle_QuadJoin:
        meshGenerator = new MeshGeneratorTriangle();
        break;
    case MeshType_GMSH_Triangle:
    case MeshType_GMSH_Quad:
    case MeshType_GMSH_QuadDelaunay_Experimental:
        meshGenerator = new MeshGeneratorGMSH();
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
            readInitialMeshesFromFile();

            delete meshGenerator;
            return true;
        }
        catch (Hermes::Exceptions::Exception& e)
        {
            Agros2D::log()->printError(tr("Mesh reader"), QString("%1").arg(e.what()));
            throw;
        }
    }
    delete meshGenerator;

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

void Problem::solveInit()
{
    m_isSolving = true;
    m_timeStepLengths.clear();

    // open indicator progress
    Indicator::openProgress();

    // control geometry
    ErrorResult result = Agros2D::scene()->checkGeometryResult();
    if (result.isError())
    {
        result.showDialog();
        m_isSolving = false;
        throw (AgrosSolverException("Geometry check failed"));
    }

    // save problem
    result = Agros2D::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

    createStructure();

    // todo: we should not mesh always, but we would need to refine signals to determine when is it neccesary (whether, e.g., parameters of the mesh have been changed)
    if (!mesh())
        throw AgrosSolverException("Could not create mesh");

    // check geometry
    if (!Agros2D::scene()->checkGeometryAssignement())
        throw AgrosSolverException("Geometry assignment failed");

    if (fieldInfos().count() == 0)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("no field defined."));
        throw AgrosSolverException("No field defined");
    }
}

void Problem::solve()
{
    solve(false, false);
}

void Problem::solveCommandLine()
{
    solve(false, true);
}

void Problem::solveAdaptiveStep()
{
    solve(true, false);
}

void Problem::solve(bool adaptiveStepOnly, bool commandLine)
{
    if (isSolving())
        return;

    if(numTransientFields() > 1)
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", "Coupling of more transient fields not possible at the moment.");
        return;
    }

    if((m_fieldInfos.size() > 1) && isTransient() && (numAdaptiveFields() >= 1))
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", "Space adaptivity for transient coupled problems not possible at the moment.");
        return;
    }

    if(isTransient() && config()->isTransientAdaptive() && (numAdaptiveFields() >= 1))
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", "Both space and transient adaptivity at the same time not possible at the moment.");
        return;
    }

    if(Agros2D::configComputer()->saveMatrixRHS)
        Agros2D::log()->printWarning(tr(""), tr("Warning: Matrix and RHS will be saved on the disk. This will slow down the calculation. You may disable it in Edit->Options->Solver menu."));

    // clear solution
    clearSolution();

    try
    {
        solveActionCatchExceptions(adaptiveStepOnly);

        // delete temp file
        if (config()->fileName() == tempProblemFileName() + ".a2d")
        {
            QFile::remove(config()->fileName());
            config()->setFileName("");
        }

        if (!commandLine)
        {
            Agros2D::scene()->blockSignals(true);

            Agros2D::scene()->setActiveTimeStep(Agros2D::solutionStore()->lastTimeStep(Agros2D::scene()->activeViewField(), SolutionMode_Normal));
            Agros2D::scene()->setActiveAdaptivityStep(Agros2D::solutionStore()->lastAdaptiveStep(Agros2D::scene()->activeViewField(), SolutionMode_Normal));
            Agros2D::scene()->setActiveSolutionType(SolutionMode_Normal);

            Agros2D::scene()->blockSignals(false);

            m_isSolving = false;
            m_isSolved = true;
            emit solved();

            // close indicator progress
            Indicator::closeProgress();
        }
        else
        {
            m_isSolved = true;
        }
    }
    catch (AgrosSolverException &e)
    {
        return;
    }
}

void Problem::solveActionCatchExceptions(bool adaptiveStepOnly)
{
    try
    {
        Agros2D::loadActivePlugins();
    }
    catch (AgrosException e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), /*QObject::tr(*/QString("%1").arg(e.what()));
        return;
    }

    m_lastTimeElapsed = QTime();
    QTime timeCounter = QTime();
    timeCounter.start();

    try
    {
        if (adaptiveStepOnly)
            solveAdaptiveStepAction();
        else
            solveAction();

        m_lastTimeElapsed = milisecondsToTime(timeCounter.elapsed());
    }
    // ToDo: Create better system of exceptions
    catch (Hermes::Exceptions::Exception& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), QString("%1").arg(e.what()));
        throw AgrosSolverException(e.what());
    }
    catch (Hermes::Exceptions::Exception* e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), QString("%1").arg(e->what()));
        throw;
    }
    catch (AgrosSolverException& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        throw;
    }
    catch (...)
    {
        throw;
    }

    // todo: somehow catch other exceptions - agros should not fail, but some message should be generated
    //                        catch (...)
    //                        {
    //                            // Agros2D::log()->printError(tr("Problem"), QString::fromStdString(e.what()));
    //                            return;
    //                        }



    // warning: in the case of exception, the program will not reach this place
    // therefore the cleanup and stop of timeElapsed is done in solve / solveAdaptiveStep by calling solveFinished
}

//adaptivity step: from 0, if no adaptivity, than 0
//time step: from 0 (initial condition), if block is not transient, calculate allways (todo: timeskipping)
//if no block transient, everything in timestep 0

void Problem::solveAction()
{
    Agros2D::scene()->blockSignals(true);

    Agros2D::scene()->setActiveAdaptivityStep(0);
    Agros2D::scene()->setActiveTimeStep(0);
    Agros2D::scene()->setActiveViewField(fieldInfos().values().at(0));

    solveInit();

    assert(isMeshed());

    QMap<Block*, Solver<double> *> solvers;

    Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("solving problem"));

    foreach (Block* block, m_blocks)
    {       
        Solver<double> *solver = block->prepareSolver();
        if (!solver)
        {
            qDeleteAll(solvers);
            throw AgrosSolverException(tr("Cannot create solver."));
        }

        solvers[block] = solver;
        solvers[block]->createInitialSpace();
    }

    NextTimeStep nextTimeStep(config()->initialTimeStepLength());
    bool doNextTimeStep = true;
    while (doNextTimeStep)
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
                    bool continueAdaptivity = true;
                    while (continueAdaptivity && (adaptStep <= block->adaptivitySteps()))
                    {
                        // solve problem
                        solvers[block]->solveReferenceAndProject(actualTimeStep(), adaptStep - 1, false);
                        // create adapted space
                        continueAdaptivity = solvers[block]->createAdaptedSpace(actualTimeStep(), adaptStep);

                        adaptStep++;
                    }
                }

                // TODO: it should be estimated in the first step as well
                // TODO: what if more blocks are transient? (take minimum? )

                // TODO: space + time adaptivity
                if (block->isTransient() && (actualTimeStep() >= 1))
                    nextTimeStep = solvers[block]->estimateTimeStepLength(actualTimeStep(), 0);
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

            doNextTimeStep = defineActualTimeStepLength(nextTimeStep.length);
        }
    }    

    // delete solvers
    qDeleteAll(solvers);
}

void Problem::solveAdaptiveStepAction()
{
    Agros2D::scene()->blockSignals(true);

    try
    {
        solveInit();

        assert(isMeshed());

        Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("solving problem"));

        assert(m_blocks.size() == 1);
        Block* block = m_blocks.at(0);
        Solver<double> *solver = block->prepareSolver();
        if (!solver)
            throw AgrosSolverException(tr("Cannot create solver."));

        int adaptStepNormal = Agros2D::solutionStore()->lastAdaptiveStep(block, SolutionMode_Normal, 0);
        int adaptStepNonExisting = Agros2D::solutionStore()->lastAdaptiveStep(block, SolutionMode_NonExisting, 0);
        int adaptStep = max(adaptStepNormal, adaptStepNonExisting);

        // it means that solution allready exists, but will be recalculated by adapt step
        bool solutionAlreadyExists = ((adaptStep >= 0) && (adaptStepNormal == adaptStep));

        // it does not exist, problem has not been solved yet
        if (adaptStep < 0)
        {
            Agros2D::scene()->setActiveAdaptivityStep(0);
            Agros2D::scene()->setActiveTimeStep(0);
            Agros2D::scene()->setActiveViewField(fieldInfos().values().at(0));

            solver->createInitialSpace();
            adaptStep = 0;
        }

        // standard adaptivity process may end by calculation of refference or by creating adapted space
        // (depends on which stopping criteria is fulfilled). To avoid unnecessary calculations:
        bool hasReference = (Agros2D::solutionStore()->lastAdaptiveStep(block, SolutionMode_Reference, 0) == adaptStep);
        if (!hasReference)
        {
            solver->solveReferenceAndProject(0, adaptStep, solutionAlreadyExists);
        }

        solver->createAdaptedSpace(0, adaptStep + 1);

        // only if solution in previous adapt step existed, solve new one (we would have two new adapt steps otherwise)
        if (solutionAlreadyExists || adaptStep == 0)
            solver->solveSimple(0, adaptStep + 1);

        // delete solver
        delete solver;
    }
    catch (AgrosSolverException& e)
    {
        throw;
    }
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
                                         QObject::tr("transient step %1 (%2%)").
                                         arg(actualTimeStep()).
                                         arg(int(100*actualTime()/config()->timeTotal().number())));
        }
        else
        {
            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(fields),
                                         QObject::tr("transient step %1/%2").
                                         arg(actualTimeStep()).
                                         arg(config()->numConstantTimeSteps()));
        }
    }
    else
    {
        if (block->fields().count() == 1)
            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(fields), QObject::tr("single analysis"));
        else
            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(fields), QObject::tr("coupled analysis"));
    }

}


void Problem::readInitialMeshesFromFile()
{
    Agros2D::log()->printMessage(tr("Problem"), tr("Loading initial mesh from disk"));

    // load initial mesh file
    // prepare mesh array
    Hermes::vector<Hermes::Hermes2D::Mesh*> meshesVector;
    QMap<FieldInfo *, Hermes::Hermes2D::Mesh *> meshes;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
    {
        Hermes::Hermes2D::Mesh *mesh = new Hermes::Hermes2D::Mesh();

        meshesVector.push_back(mesh);
        // cache
        meshes[fieldInfo] = mesh;
    }

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load mesh from file
    QString fileName = cacheProblemDir() + "/initial.mesh";
    Hermes::Hermes2D::MeshReaderH2DXML meshloader;
    meshloader.set_validation(false);
    meshloader.load(fileName.toStdString().c_str(), meshesVector);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    QSet<int> boundaries;
    foreach (FieldInfo *fieldInfo, m_fieldInfos)
    {
        Hermes::Hermes2D::Mesh *mesh = meshes[fieldInfo];

        // check that all boundary edges have a marker assigned
        for (int i = 0; i < mesh->get_max_node_id(); i++)
        {
            Hermes::Hermes2D::Node *node = mesh->get_node(i);

            if ((node->used == 1 && node->ref < 2 && node->type == 1))
            {
                int marker = atoi(mesh->get_boundary_markers_conversion().get_user_marker(node->marker).marker.c_str());

                assert(marker >= 0 || marker == -999);

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

            throw AgrosException(QObject::tr("Mesh reader (%1): boundary edges '%2' does not have a boundary marker").
                                 arg(fieldInfo->fieldId()).
                                 arg(markers));

            // delete meshes
            foreach (Hermes::Hermes2D::Mesh *mesh, meshes)
                delete mesh;
            meshes.clear();
            meshesVector.clear();

            return;
        }
        boundaries.clear();

        // refine mesh
        refineMesh(fieldInfo, mesh, true, true, true);

        // set initial mesh
        fieldInfo->setInitialMesh(mesh);
    }

    meshes.clear();
    meshesVector.clear();

    emit meshed();
}

void Problem::readSolutionsFromFile()
{
    Agros2D::log()->printMessage(tr("Problem"), tr("Loading spaces and solutions from disk"));

    if (QFile::exists(QString("%1/runtime.xml").arg(cacheProblemDir())))
    {
        // load active plugins
        try
        {
            Agros2D::loadActivePlugins();
        }
        catch (AgrosException e)
        {
            Agros2D::log()->printError(QObject::tr("Solver"), /*QObject::tr(*/QString("%1").arg(e.what()));
            return;
        }

        // load structure
        Agros2D::solutionStore()->loadRunTimeDetails();

        // set view
        Agros2D::scene()->blockSignals(true);

        Agros2D::scene()->setActiveTimeStep(Agros2D::solutionStore()->lastTimeStep(Agros2D::scene()->activeViewField(), SolutionMode_Normal));
        Agros2D::scene()->setActiveAdaptivityStep(Agros2D::solutionStore()->lastAdaptiveStep(Agros2D::scene()->activeViewField(), SolutionMode_Normal));
        Agros2D::scene()->setActiveSolutionType(SolutionMode_Normal);

        Agros2D::scene()->blockSignals(false);

        // emit solve
        m_isSolved = true;
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
