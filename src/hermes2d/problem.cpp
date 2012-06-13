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
#include "meshgenerator.h"
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
    m_timeStep = Value("1.0", false);
    m_timeTotal = Value("1.0", false);
}


Problem::Problem()
{
    m_timeStep = 0;
    m_timeElapsed = QTime(0, 0);
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

Hermes::Hermes2D::Mesh* Problem::activeMeshInitial()
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
    if (Util::problem()->isSolved())
        Util::solutionStore()->clearAll();
    Util::solutionStore()->clearAll();

    foreach (Hermes::Hermes2D::Mesh* mesh, m_meshesInitial)
        if (mesh)
            delete mesh;
    m_meshesInitial.clear();

    m_timeStep = 0;
    m_timeElapsed = QTime(0, 0);
    m_isSolved = false;
    m_isSolving = false;
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

    Util::problem()->synchronizeCouplings();

    //copy lists, items will be removed from them
    QList<FieldInfo *> fieldInfos = Util::problem()->fieldInfos().values();
    QList<CouplingInfo* > couplingInfos = Util::problem()->couplingInfos().values();

    while (!fieldInfos.empty()){
        QList<FieldInfo*> blockFieldInfos;
        QList<CouplingInfo*> blockCouplingInfos;

        //first find one field, that is not weakly coupled and dependent on other fields
        bool dependent;
        foreach (FieldInfo* fieldInfo, fieldInfos)
        {
            dependent = false;

            foreach (CouplingInfo* couplingInfo, couplingInfos)
            {
                if (couplingInfo->isWeak() && (couplingInfo->targetField() == fieldInfo) && fieldInfos.contains(couplingInfo->sourceField()))
                    dependent = true;
            }

            // this field is not weakly dependent, we can put it into this block
            if (!dependent){
                blockFieldInfos.push_back(fieldInfo);
                fieldInfos.removeOne(fieldInfo);
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
                        blockFieldInfos.push_back(checkedFieldInfo);
                        fieldInfos.removeOne(checkedFieldInfo);
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

    MeshGeneratorTriangle pim;
    if (pim.mesh())
    {
        emit meshed();
        return true;
    }

    return false;
}

void Problem::solveInit()
{
    m_isSolving = true;

    // open indicator progress
    Indicator::openProgress();


    // control geometry
    ErrorResult result = Util::scene()->checkGeometryResult();
    if (result.isError())
    {
        result.showDialog();
        m_isSolving = false;
        throw(AgrosSolverException("Geometry check failed"));
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

    if (Util::problem()->fieldInfos().count() == 0)
    {
        Util::log()->printError(QObject::tr("Solver"), QObject::tr("no field defined."));
        throw AgrosSolverException("No field defined");
    }
}

void Problem::solve()
{
    if (isSolving())
        return;

    QTime elapsedTime;
    elapsedTime.start();

    setActualTime(0.);

    try
    {
        solveAction();
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", QString::fromAscii(e.getMsg()));
        return;
    }
    catch (AgrosSolverException& e)
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", e.str);
        return;
    }
    // todo: somehow catch other exceptions - agros should not fail, but some message should be generated
//                        catch (...)
//                        {
//                            // Util::log()->printError(tr("Problem"), QString::fromStdString(e.what()));
//                            return;
//                        }

    // delete temp file
    if (Util::problem()->config()->fileName() == tempProblemFileName() + ".a2d")
    {
        QFile::remove(Util::problem()->config()->fileName());
        Util::problem()->config()->setFileName("");
    }

    m_isSolving = false;

    m_timeElapsed = milisecondsToTime(elapsedTime.elapsed());

    // close indicator progress
    Indicator::closeProgress();

}

void Problem::solveAction()
{
    clearSolution();

    Util::scene()->blockSignals(true);

    solveInit();

    assert(isMeshed());

    QMap<Block*, Solver<double>* > solvers;

    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("solving problem"));

    Util::scene()->setActiveAdaptivityStep(0);
    Util::scene()->setActiveTimeStep(0);
    Util::scene()->setActiveViewField(Util::problem()->fieldInfos().values().at(0));

    foreach (Block* block, m_blocks)
    {
        solvers[block] = block->prepareSolver();
        if (block->isTransient())
            solvers[block]->solveInitialTimeStep();
    }

    int timeStep = 0;
    bool doNextTimeStep = true;
    while(doNextTimeStep)
    {
        foreach (Block* block, m_blocks)
        {
            Solver<double>* solver = solvers[block];
            if (!(block->isTransient() && (timeStep == 0)) && (!block->skipThisTimeStep(timeStep)))
            {
                if (block->adaptivityType() == AdaptivityType_None)
                {
                    // todo: merge solveSimple and solveTimeStep
                    if(block->isTransient())
                    {
                        solver->solveTimeStep();
                    }
                    else
                    {
                        solver->createInitialSpace(timeStep);
                        solver->solveSimple(timeStep, 0, false);
                    }
                }
                else
                {
                    if(block->isTransient())
                    {
                        // pak vyuzit toho, ze mam vsechny adaptivni kroky z predchozi casove vrstvy
                        // vezmu treba pred pred posledni adaptivni krok a tim budu mit derefinement
                        QMessageBox::warning(QApplication::activeWindow(), "Solver Error", "Adaptivity not implemented for transient problems");
                        return;
                    }

                    solver->createInitialSpace(timeStep);
                    int adaptStep = 1;
                    bool continueAdaptivity = true;
                    while (continueAdaptivity && (adaptStep <= block->adaptivitySteps()))
                    {
                        solver->solveReferenceAndProject(timeStep, adaptStep, false);
                        continueAdaptivity = solver->createAdaptedSpace(timeStep, adaptStep);
                        adaptStep++;
                    }
                }

            }
        }
        timeStep++;
        doNextTimeStep = Util::problem()->isTransient() && (timeStep <= Util::problem()->config()->numTimeSteps());

        Util::scene()->setActiveTimeStep(Util::solutionStore()->lastTimeStep(Util::scene()->activeViewField(), SolutionMode_Normal));
        Util::scene()->setActiveAdaptivityStep(Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal));
        Util::scene()->setActiveSolutionType(SolutionMode_Normal);
        cout << "setting active adapt step to " << Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal) << endl;

        addToActualTime(Util::problem()->config()->timeStep().value());
    }


    Util::scene()->blockSignals(false);

    m_isSolved = true;
    emit solved();

}

void Problem::solveAdaptiveStep()
{
    if (isSolving())
        return;

    QTime elapsedTime;
    elapsedTime.start();

    if(m_blocks.count() > 1)
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", "This action is possible for one field only, unless they are hard-coupled");
        return;
    }
    if(isTransient())
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", "This action is not possible for transient problems");
        return;
    }

    try
    {
        solveAdaptiveStepAction();
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", QString::fromAscii(e.getMsg()));
        return;
    }
    catch (AgrosSolverException& e)
    {
        QMessageBox::critical(QApplication::activeWindow(), "Solver Error", e.str);
        return;
    }

    // delete temp file
    if (Util::problem()->config()->fileName() == tempProblemFileName() + ".a2d")
    {
        QFile::remove(Util::problem()->config()->fileName());
        Util::problem()->config()->setFileName("");
    }

    m_isSolving = false;

    m_timeElapsed = milisecondsToTime(elapsedTime.elapsed());

    // close indicator progress
    Indicator::closeProgress();
}

void Problem::solveAdaptiveStepAction()
{
    Util::scene()->blockSignals(true);

    solveInit();

    assert(isMeshed());

    QMap<Block*, Solver<double>* > solvers;

    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("solving problem"));

    Util::scene()->setActiveViewField(Util::problem()->fieldInfos().values().at(0));

    foreach (Block* block, m_blocks)
    {
        solvers[block] = block->prepareSolver();
    }

    foreach (Block* block, m_blocks)
    {
        Solver<double>* solver = solvers[block];

        int adaptStepNormal = Util::solutionStore()->lastAdaptiveStep(block, SolutionMode_Normal, 0);
        int adaptStepNonExisting = Util::solutionStore()->lastAdaptiveStep(block, SolutionMode_NonExisting, 0);
        int adaptStep = max(adaptStepNormal, adaptStepNonExisting);

        // it means that solution allready exists, but will be recalculated by adapt step
        bool solutionAlreadyExists = ((adaptStep >= 0) && (adaptStepNormal == adaptStep));

        // it does not exist, problem has not been solved yet
        if(adaptStep < 0)
        {
            solver->createInitialSpace(0);
            adaptStep = 0;
        }

        // standard adaptivity process may end by calculation of refference or by creating adapted space
        // (depends on which stopping criteria is fulfilled). To avoid unnecessary calculations:
        bool hasReference = (Util::solutionStore()->lastAdaptiveStep(block, SolutionMode_Reference, 0) == adaptStep);
        if(!hasReference)
        {
            solver->solveReferenceAndProject(0, adaptStep + 1, solutionAlreadyExists);
        }

        solver->createAdaptedSpace(0, adaptStep + 1);
        solver->solveSimple(0, adaptStep + 1, false);


        Util::scene()->setActiveTimeStep(Util::solutionStore()->lastTimeStep(Util::scene()->activeViewField(), SolutionMode_Normal));
        Util::scene()->setActiveAdaptivityStep(Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal));
        Util::scene()->setActiveSolutionType(SolutionMode_Normal);
        cout << "setting active adapt step to " << Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal) << endl;
    }

    m_isSolved = true;
    emit solved();
}

void Problem::synchronizeCouplings()
{
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
        }
    }
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
