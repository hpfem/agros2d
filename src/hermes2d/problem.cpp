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
        m_blocks.append(new Block(blockFieldInfos, blockCouplingInfos, this));
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

    QTime elapsedTime;
    elapsedTime.start();

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

    m_timeElapsed = milisecondsToTime(elapsedTime.elapsed());

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

