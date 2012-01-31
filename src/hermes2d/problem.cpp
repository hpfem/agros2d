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
#include "scenesolution.h"
#include "module.h"
#include "module_agros.h"
#include "coupling.h"
#include "solver.h"
#include "progressdialog.h"


Field::Field(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{

}

bool Field::solveInitVariables()
{
    //TODO transient
    //
    //
    //    // transient
    //    if (Util::scene()->problemInfo()->analysisType() == AnalysisType_Transient)
    //    {
    //        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return false;
    //        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return false;
    //        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return false;
    //    }

    if(!Util::scene()->boundaries->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    if(!Util::scene()->materials->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    return true;
}

Block::Block(QList<FieldInfo *> fieldInfos, ProgressItemSolve* progressItemSolve) : m_progressItemSolve(progressItemSolve)
{
    foreach(FieldInfo* fi, fieldInfos)
    {
        m_fields.append(new Field(fi));
    }

    m_solutionList = new SolutionArrayList<double>;
}

bool Block::solveInit(Coupling *coupling, Hermes::Hermes2D::Solution<double> *sourceSolution)
{
    foreach(Field* field, m_fields)
    {
        if(! field->solveInitVariables())
            assert(0); //TODO co to znamena?
        m_wf = new WeakFormAgros<double>(field->fieldInfo(), coupling, sourceSolution);

        m_solutionList->init(m_progressItemSolve, m_wf, m_fields.at(0)->m_fieldInfo);
        m_solutionList->clear();
    }
}

void Block::solve()
{
    m_solutionList->solve();
    cout << "num elem pri prirazeni do scene solution " <<  Util::problem()->meshInitial()->get_num_active_elements() << endl;
    Util::scene()->sceneSolution(m_fields[0]->fieldInfo())->setMeshInitial(Util::problem()->meshInitial());

    QList<SolutionArray<double>* > solutionArrays;
    for(int i = 0; i < m_fields[0]->fieldInfo()->module()->number_of_solution(); i++)
        solutionArrays.push_back(m_solutionList->at(i));
    Util::scene()->sceneSolution(m_fields[0]->fieldInfo())->setSolutionArray(solutionArrays);

}

Problem::Problem()
{
    m_timeStep = 0;
    m_isSolved = false;
    m_isSolving = false;
//    m_progressDialog = new ProgressDialog();
//    m_progressItemMesh = new ProgressItemMesh();
//    m_progressItemSolve = new ProgressItemSolve();
//    m_progressItemSolveAdaptiveStep = new ProgressItemSolveAdaptiveStep();
//    m_progressItemProcessView = new ProgressItemProcessView();

}

void Problem::createStructure()
{
    QMap<QString, FieldInfo *> fieldInfos = Util::scene()->fieldInfos();

    //TODO only weak coupling so far
    foreach(FieldInfo* fi, fieldInfos)
    {
        QList<FieldInfo*> tmp;
        tmp.append(fi);
        m_blocks.append(new Block(tmp, m_progressItemSolve));
    }

    //TODO temporary
    Coupling *heatElastCoup = new Coupling(CoordinateType_Planar);
    heatElastCoup->read("resources/couplings/heat-elasticity.xml");
    m_couplings.push_back(heatElastCoup);
}


void Problem::mesh()
{
    ProgressItemMesh* pim = new ProgressItemMesh();
    pim->mesh();
}

void Problem::postprocess()
{
    ProgressItemProcessView* pipv = new ProgressItemProcessView();
    pipv->run();
}

void Problem::solve(SolverMode solverMode)
{
    logMessage("SceneSolution::solve()");

    setVerbose(true);

    if (isSolving()) return;

    m_isSolving = true;

    // clear problem
    //clear(solverMode == SolverMode_Mesh || solverMode == SolverMode_MeshAndSolve);

    // open indicator progress
    Indicator::openProgress();

    // save problem
    ErrorResult result = Util::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

//    if (solverMode == SolverMode_Mesh || solverMode == SolverMode_MeshAndSolve)
//    {
//        m_progressDialog->clear();
//        m_progressDialog->appendProgressItem(m_progressItemMesh);
//        if (solverMode == SolverMode_MeshAndSolve)
//        {
//            m_progressDialog->appendProgressItem(m_progressItemSolve);
//            m_progressDialog->appendProgressItem(m_progressItemProcessView);
//        }
//    }
//    else if (solverMode == SolverMode_SolveAdaptiveStep)
//    {
//        m_progressDialog->appendProgressItem(m_progressItemSolveAdaptiveStep);
//        m_progressDialog->appendProgressItem(m_progressItemProcessView);
//    }

//    if (m_progressDialog->run())
//    {
//        setTimeStep(timeStepCount() - 1);
//        //emit meshed();
//        //emit solved();
//    }

    createStructure();

    Util::scene()->setActiveViewField(m_blocks[0]->m_fields[0]->fieldInfo());

    mesh();
    emit meshed();

    Util::scene()->createSolutions();

    assert(isMeshed());
    if (isMeshed())
    {
        InitialCondition<double> initial(m_meshInitial, 0.0);
        Util::scene()->activeSceneSolution()->linInitialMeshView().process_solution(&initial);
    }


//    foreach(Block* block, m_blocks)
//    {
//        block->solveInit();
//        block->solve();
//    }

    const int elastTODO = 0;  //TODO temp
    const int heatTODO = 1;   //TODO temp

    m_blocks[heatTODO]->solveInit();
    m_blocks[heatTODO]->solve();

    m_blocks[elastTODO]->solveInit(m_couplings[0], m_blocks[heatTODO]->m_solutionList->at(0)->sln.get());
    m_blocks[elastTODO]->solve();

    // delete temp file
    if (Util::scene()->problemInfo()->fileName == tempProblemFileName() + ".a2d")
    {
        QFile::remove(Util::scene()->problemInfo()->fileName);
        Util::scene()->problemInfo()->fileName = "";
    }


    // close indicator progress
    Indicator::closeProgress();

    m_isSolving = false;
    m_isSolved = true;
    emit solved();
    emit timeStepChanged(false);

    postprocess();

    //    if (!solve_init_variables())
    //        return Hermes::vector<SolutionArray<double> *>(); //TODO PK <double>

    //    WeakFormAgros<double> wf(number_of_solution()); //TODO PK <double>

    //    SolverAgros<double> solutionAgros(progressItemSolve, &wf);

    //    Hermes::vector<SolutionArray<double> *> solutionArrayList = solutionAgros.solve(); //TODO PK <double>
    //    return solutionArrayList;


//    foreach(Block* block, m_blocks)
//    {
//        block->solveInit();
//        block->solve();
//    }

}

ProgressDialog* Problem::progressDialog()
{
    return m_progressDialog;
}


