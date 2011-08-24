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

#include "solver.h"
#include "scene.h"
#include "scenesolution.h"
#include "progressdialog.h"

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray()
{
    logMessage("SolutionArray::SolutionArray()");

    sln = NULL;
    space = NULL;

    time = 0.0;
    adaptiveSteps = 0;
    adaptiveError = 100.0;
}

template <typename Scalar>
SolutionArray<Scalar>::~SolutionArray()
{
    logMessage("SolutionArray::~SolutionArray()");

    if (sln)
    {
        delete sln;
        sln = NULL;
    }

    if (space)
    {
        delete space;
        space = NULL;
    }
}

template <typename Scalar>
void SolutionArray<Scalar>::load(QDomElement *element)
{
    logMessage("SolutionArray::load()");

    QString fileNameSolution = tempProblemFileName() + ".sln";
    QString fileNameSpace = tempProblemFileName() + ".spc";

    // write content (saved solution)
    QByteArray contentSolution;
    contentSolution.append(element->elementsByTagName("sln").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSolution, QByteArray::fromBase64(contentSolution));

    // write content (saved space)
    QByteArray contentSpace;
    contentSpace.append(element->elementsByTagName("space").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSpace, QByteArray::fromBase64(contentSpace));

    sln = new Hermes::Hermes2D::Solution<Scalar>();
    sln->load(fileNameSolution.toStdString().c_str());
    //space = new Hermes::Hermes2D::Space<Scalar>();
    //space->load(fileNameSpace.toStdString().c_str());
    adaptiveError = element->attribute("adaptiveerror").toDouble();
    adaptiveSteps = element->attribute("adaptivesteps").toInt();
    time = element->attribute("time").toDouble();

    // delete solution
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

template <typename Scalar>
void SolutionArray<Scalar>::save(QDomDocument *doc, QDomElement *element)
{
    logMessage("SolutionArray::save()");

    // solution
    QString fileNameSolution = tempProblemFileName() + ".sln";
    sln->save(fileNameSolution.toStdString().c_str(), false);
    QDomText textSolution = doc->createTextNode(readFileContentByteArray(fileNameSolution).toBase64());

    // space
    QString fileNameSpace = tempProblemFileName() + ".spc";
    //space->save_data(fileNameSpace.toStdString().c_str());
    QDomNode textSpace = doc->createTextNode(readFileContentByteArray(fileNameSpace).toBase64());

    QDomNode eleSolution = doc->createElement("sln");
    QDomNode eleSpace = doc->createElement("space");

    eleSolution.appendChild(textSolution);
    eleSpace.appendChild(textSpace);

    element->setAttribute("adaptiveerror", adaptiveError);
    element->setAttribute("adaptivesteps", adaptiveSteps);
    element->setAttribute("time", time);
    element->appendChild(eleSolution);
    element->appendChild(eleSpace);

    // delete
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

// *********************************************************************************************


template <typename Scalar>
SolverAgros<Scalar>::SolverAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf)
{
    analysisType = Util::scene()->problemInfo()->analysisType;
    polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;
    adaptivityMaxDOFs = Util::scene()->problemInfo()->adaptivityMaxDOFs;
    numberOfSolution = Util::scene()->problemInfo()->module()->number_of_solution();
    timeTotal = Util::scene()->problemInfo()->timeTotal.number();
    timeStep = Util::scene()->problemInfo()->timeStep.number();
    initialCondition = Util::scene()->problemInfo()->initialCondition.number();

    nonlinearTolerance = Util::scene()->problemInfo()->nonlinearTolerance;
    nonlinearSteps = Util::scene()->problemInfo()->nonlinearSteps;

    matrixSolver = Util::scene()->problemInfo()->matrixSolver;

    m_progressItemSolve = progressItemSolve;
    m_wf = wf;
}


//TODO PK why bcs has to be reference???

template <typename Scalar>
void SolverAgros<Scalar>::initCalculation(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> >& bcs)
{
    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    refineMesh(mesh, true, true);

    Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *select = NULL;
    switch (adaptivityType)
    {
    case AdaptivityType_H:
        select = new Hermes::Hermes2D::RefinementSelectors::HOnlySelector<Scalar>();
        break;
    case AdaptivityType_P:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_P_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    case AdaptivityType_HP:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        space.push_back(new Hermes::Hermes2D::H1Space<Scalar>(mesh, &bcs[i], polynomialOrder));

        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            if (Util::scene()->labels[j]->material != Util::scene()->materials[0])
                space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder,
                                               QString::number(j).toStdString());

        // solution agros array
        solution.push_back(new Hermes::Hermes2D::Solution<double>);

        if (adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
            // reference solution
            solutionReference.push_back(new Hermes::Hermes2D::Solution<Scalar>());
        }
    }
}

template <typename Scalar>
void SolverAgros<Scalar>::cleanup()
{

}

template <typename Scalar>
bool SolverAgros<Scalar>::solveOneProblem(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceParam,
                                            Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionParam)
{
    // Initialize the FE problem.
    Hermes::Hermes2D::DiscreteProblem<double> dp(m_wf, spaceParam);

    int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceParam);
    cout << "ndof " << ndof << endl;

    // Initial coefficient vector for the Newton's method.
    double* coeff_vec = new double[ndof];
    memset(coeff_vec, 0, ndof*sizeof(double));

    // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
    Hermes::Hermes2D::NewtonSolver<double> newton(&dp, Hermes::SOLVER_UMFPACK);
    if (!newton.solve(coeff_vec, nonlinearTolerance, 2*nonlinearSteps))  //TODO remove 2*
    {
        m_progressItemSolve->emitMessage(QObject::tr("Newton's iteration failed"), true);
        return false;
    }
    else{
        Hermes::Hermes2D::Solution<double>::vector_to_solutions(newton.get_sln_vector(), spaceParam, solutionParam);
//        m_progressItemSolve->emitMessage(QObject::tr("One problem solved"), false); //TODO temp
//        cout << "One problem solved" << endl;
    }

    return true;
}


template <typename Scalar>
Hermes::vector<SolutionArray<Scalar> *> SolverAgros<Scalar>::solveSolutionArray(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs)
{
    QTime time;

    // solution agros array
    Hermes::vector<SolutionArray<Scalar> *> solutionArrayList;

    // error marker
    bool isError = false;
    double error = 0.0;

    initCalculation(bcs);

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
        cleanup();
        return solutionArrayList;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        // nonlinear - initial solution
        // solution.at(i)->set_const(mesh, 0.0);

        // transient
        if (analysisType == AnalysisType_Transient)
        {
            // constant initial solution
            InitialCondition *initial = new InitialCondition(mesh, initialCondition);
            solutionArrayList.push_back(solutionArray(initial, space.at(i)));
        }
    }

    actualTime = 0.0;
    int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
    cout << "total " << timeTotal << ", step " << timeStep << ", timesteps " << timesteps << endl;
    for (int n = 0; n<timesteps; n++)
    {
        // set actual time
        actualTime = (n + 1) * timeStep;

        // update essential bc values
        Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(space, actualTime);
        // update timedep values
        Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

        m_wf->set_current_time(actualTime);
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            for (int i = 0; i < solution.size(); i++){
                cout << "push " << solution[i] << " sit " << solution[i]->get_mesh() <<  endl;
                m_wf->solution.push_back(solutionArrayList[i]->sln );
            }
        m_wf->delete_all();
        m_wf->registerForms();

        int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
        int actualAdaptivitySteps = -1;
        for (int i = 0; i<maxAdaptivitySteps; i++)
        {
            if (adaptivityType == AdaptivityType_None)
            {
                if(!solveOneProblem(space, solution))
                    isError = true;
            }
            else
            {
                Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceReference = *Hermes::Hermes2D::Space<Scalar>::construct_refined_spaces(space);

                if(!solveOneProblem(spaceReference, solutionReference)){
                    isError = true;
                    break;
                }
                // project the fine mesh solution onto the coarse mesh.
                Hermes::Hermes2D::OGProjection<Scalar>::project_global(space, solutionReference, solution, matrixSolver);

                // Calculate element errors and total error estimate.
                Hermes::Hermes2D::Adapt<Scalar> adaptivity(space, projNormType);

                // Calculate error estimate for each solution component and the total error estimate.
                error = adaptivity.calc_err_est(solution, solutionReference) * 100;

                // emit signal
                m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
                                                 arg(error, 0, 'f', 3).
                                                 arg(i + 1).
                                                 arg(maxAdaptivitySteps).
                                                 arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space)).
                                                 arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceReference)), false, 1);
                // add error to the list
                m_progressItemSolve->addAdaptivityError(error, Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space));

                if (error < adaptivityTolerance || Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) >= adaptivityMaxDOFs)
                {
                    break;
                }
                if (i != maxAdaptivitySteps-1) adaptivity.adapt(selector,
                                                                Util::config()->threshold,
                                                                Util::config()->strategy,
                                                                Util::config()->meshRegularity);
                actualAdaptivitySteps = i+1;

            }
        }


        // output
        if (!isError)
        {
            for (int i = 0; i < numberOfSolution; i++){
                cout << "solution" << i << endl;
                solutionArrayList.push_back(solutionArray(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*timeStep));
            }

            if (analysisType == AnalysisType_Transient)
                m_progressItemSolve->emitMessage(QObject::tr("Transient time step (%1/%2): %3 s").
                                                 arg(n+1).
                                                 arg(timesteps).
                                                 arg(actualTime, 0, 'e', 2), false, n+2);
        }
        else
            break;
    }


    cleanup();
    return solutionArrayList;
    //TODO je tu trochu zmatek v solutionArray, solutionAgros, atp...
}

/*
template <typename Scalar>
Hermes::vector<SolutionArray<Scalar> *> SolutionAgros<Scalar>::solveSolutionArrayOld(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs)
{
    QTime time;

    // solution agros array
    Hermes::vector<SolutionArray<Scalar> *> solutionArrayList;

    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    refineMesh(mesh, true, true);

    // create an H1 space
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space;
    // create hermes solution array
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution;
    // create reference solution
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionReference;

    // projection norms
    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;

    // prepare selector
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;

    // error marker
    bool isError = false;

    Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *select = NULL;
    switch (adaptivityType)
    {
    case AdaptivityType_H:
        select = new Hermes::Hermes2D::RefinementSelectors::HOnlySelector<Scalar>();
        break;
    case AdaptivityType_P:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_P_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    case AdaptivityType_HP:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        space.push_back(new Hermes::Hermes2D::H1Space<Scalar>(mesh, &bcs[i], polynomialOrder));

        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            if (Util::scene()->labels[j]->material != Util::scene()->materials[0])
                space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder,
                                               QString::number(j).toStdString());

        // solution agros array
        solution.push_back(new Hermes::Hermes2D::Solution<Scalar>());

        if (adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
            // reference solution
            solutionReference.push_back(new Hermes::Hermes2D::Solution<Scalar>());
        }
    }

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
    }
    else
    {
        for (int i = 0; i < numberOfSolution; i++)
        {
            // transient
            if (analysisType == AnalysisType_Transient)
            {
                // constant initial solution
                // FIXME
                // solution.at(i)->set_const(mesh, initialCondition);
                solutionArrayList.push_back(solutionArray(solution.at(i)));
            }

            // nonlinear - initial solution
            // FIXME
            // solution.at(i)->set_const(mesh, 0.0);
        }

        actualTime = 0.0;

        // update time function

        Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

        m_wf->set_current_time(actualTime);
        m_wf->solution.clear();
        // FIXME
        if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
            for (int i = 0; i < solution.size(); i++)
                m_wf->solution.push_back((Hermes::Hermes2D::MeshFunction<Scalar> *) solution[i]);
        m_wf->delete_all();
        m_wf->registerForms();


        // emit message
        if (adaptivityType != AdaptivityType_None)
            m_progressItemSolve->emitMessage(QObject::tr("Adaptivity type: %1").arg(adaptivityTypeString(adaptivityType)), false);

        double error = 0.0;

        // solution

        int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
        int actualAdaptivitySteps = -1;
        for (int i = 0; i<maxAdaptivitySteps; i++)
        {
            // set up the solver, matrix, and rhs according to the solver selection.
            SparseMatrix<Scalar> *matrix = Hermes::Algebra::create_matrix<Scalar>(matrixSolver);
            Vector<Scalar> *rhs = create_vector<Scalar>(matrixSolver);
            Hermes::Solvers::LinearSolver<Scalar> *solver = create_linear_solver<Scalar>(matrixSolver, matrix, rhs); //TODO PK LinearSolver ??

            if (adaptivityType == AdaptivityType_None)
            {
                //if (analysisType != AnalysisType_Transient)
                   // solve(space, solution, solver, matrix, rhs); //TODO PK solver...
            }
            else
            {
                // construct globally refined reference mesh and setup reference space.
                Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceReference = *Hermes::Hermes2D::Space<Scalar>::construct_refined_spaces(space);

                // assemble reference problem.
                //solve(spaceReference, solutionReference, solver, matrix, rhs); //TODO PK solver...

                if (!isError)
                {
                    // project the fine mesh solution onto the coarse mesh.
                    Hermes::Hermes2D::OGProjection<Scalar>::project_global(space, solutionReference, solution, matrixSolver);

                    // Calculate element errors and total error estimate.
                    Hermes::Hermes2D::Adapt<Scalar> adaptivity(space, projNormType);

                    // Calculate error estimate for each solution component and the total error estimate.
                    error = adaptivity.calc_err_est(solution,
                                                    solutionReference) * 100;

                    // emit signal
                    m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
                                                     arg(error, 0, 'f', 3).
                                                     arg(i + 1).
                                                     arg(maxAdaptivitySteps).
                                                     arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space)).
                                                     arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceReference)), false, 1);
                    // add error to the list
                    m_progressItemSolve->addAdaptivityError(error, Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space));

                    if (error < adaptivityTolerance || Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) >= adaptivityMaxDOFs)
                    {
                        break;
                    }
                    if (i != maxAdaptivitySteps-1) adaptivity.adapt(selector,
                                                                    Util::config()->threshold,
                                                                    Util::config()->strategy,
                                                                    Util::config()->meshRegularity);
                    actualAdaptivitySteps = i+1;
                }

                if (m_progressItemSolve->isCanceled())
                {
                    isError = true;
                    break;
                }

                // delete reference space
                for (int i = 0; i < spaceReference.size(); i++)
                {
                    delete spaceReference.at(i)->get_mesh();
                    delete spaceReference.at(i);
                }
                spaceReference.clear();
            }

            // clean up.
            //delete solver; //TODO PK solver...
            delete matrix;
            delete rhs;
        }

        // delete reference solution
        for (int i = 0; i < solutionReference.size(); i++)
            delete solutionReference.at(i);
        solutionReference.clear();

        // delete selector
        if (select) delete select;
        selector.clear();


        // timesteps
        // if (!isError)
        {
            // set up the solver, matrix, and rhs according to the solver selection.
            SparseMatrix<Scalar> *matrix = create_matrix<Scalar>(matrixSolver);
            Vector<Scalar> *rhs = create_vector<Scalar>(matrixSolver);
            LinearSolver<Scalar> *solver = create_linear_solver<Scalar>(matrixSolver, matrix, rhs);

            // allocate dp for transient solution

            Hermes::Hermes2D::DiscreteProblem<Scalar> *dpTran = NULL;
            if (analysisType == AnalysisType_Transient)
            {
                dpTran = new Hermes::Hermes2D::DiscreteProblem<Scalar>(m_wf, space);
            }


            int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
            int actualAdaptivitySteps = -1;

            int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
            for (int n = 0; n<timesteps; n++)
            {
                // set actual time
                actualTime = (n+1)*timeStep;

                // update essential bc values
                Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(space, actualTime);
                // update timedep values
                Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

                m_wf->set_current_time(actualTime);
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                    for (int i = 0; i < solution.size(); i++)
                        m_wf->solution.push_back((Hermes::Hermes2D::MeshFunction<Scalar> *) solution[i]);
                m_wf->delete_all();
                m_wf->registerForms();

                // Initialize the FE problem.
                Hermes::Hermes2D::DiscreteProblem<double> dp(m_wf, space);

                int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space);

                // Initial coefficient vector for the Newton's method.
                double* coeff_vec = new double[ndof];
                memset(coeff_vec, 0, ndof*sizeof(double));

                // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
                Hermes::Hermes2D::NewtonSolver<double> newton(&dp, Hermes::SOLVER_UMFPACK);
                if (!newton.solve(coeff_vec, nonlinearTolerance, nonlinearSteps))
                {
                    m_progressItemSolve->emitMessage(QObject::tr("Newton's iteration failed"), true);
                    isError = true;
                }
                else
                    Hermes::Hermes2D::Solution<double>::vector_to_solutions(newton.get_sln_vector(), space, solution);

                // output
                if (!isError)
                {
                    for (int i = 0; i < numberOfSolution; i++)
                        solutionArrayList.push_back(solutionArray(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*timeStep));

                    if (analysisType == AnalysisType_Transient)
                        m_progressItemSolve->emitMessage(QObject::tr("Transient time step (%1/%2): %3 s").
                                                         arg(n+1).
                                                         arg(timesteps).
                                                         arg(actualTime, 0, 'e', 2), false, n+2);
                }
                else
                {
                    break;
                }

                if (m_progressItemSolve->isCanceled())
                {
                    isError = true;
                    break;
                }
            }

            // clean up
            if (solver) delete solver;
            if (matrix) delete matrix;
            if (rhs) delete rhs;

            // if (dpTran) delete dpTran;
        }
    }
    // delete mesh
    delete mesh;

    // delete space
    for (unsigned int i = 0; i < space.size(); i++)
    {
        // delete space.at(i)->get_mesh();
        delete space.at(i);
    }
    space.clear();

    // delete last solution
    for (unsigned int i = 0; i < solution.size(); i++)
        delete solution.at(i);
    solution.clear();

    if (isError)
    {
        for (int i = 0; i < solutionArrayList.size(); i++)
            delete solutionArrayList.at(i);
        solutionArrayList.clear();
    }
    return solutionArrayList;
}

template <typename Scalar>
bool SolutionAgros<Scalar>::solveLinear(Hermes::Hermes2D::DiscreteProblem<Scalar> *dp,
                                        Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space,
                                        Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution,
                                        Hermes::Solvers::LinearSolver<Scalar> *solver, Hermes::Algebra::SparseMatrix<Scalar> *matrix, Hermes::Algebra::Vector<Scalar> *rhs)
{
    // QTime time;
    // time.start();
    dp->assemble(matrix, rhs);
    // qDebug() << "assemble: " << time.elapsed();

    if(solver->solve())
    {
        //Hermes::Hermes2D::Solution<Scalar>::vector_to_solutions(solver->get_solution(), space, solution); //TODO PK
        return true;
    }
    else
    {
        m_progressItemSolve->emitMessage(QObject::tr("Matrix solver failed."), true, 1);
        return false;
    }
}
*/

template <typename Scalar>
SolutionArray<Scalar> *SolverAgros<Scalar>::solutionArray(Hermes::Hermes2D::Solution<Scalar> *sln, Hermes::Hermes2D::Space<Scalar> *space, double adaptiveError, double adaptiveSteps, double time)
{
    SolutionArray<Scalar> *solution = new SolutionArray<Scalar>();

    assert(sln);
    if (sln->get_type() == Hermes::Hermes2D::HERMES_EXACT)
    {
        solution->sln = sln;
    }
    else
    {
        solution->sln = new Hermes::Hermes2D::Solution<Scalar>();
        solution->sln->copy(sln);
    }

    assert(space);
    //solution->space = new Hermes::Hermes2D::Space<Scalar>();
    solution->space = space->dup(space->get_mesh());


    solution->adaptiveError = adaptiveError;
    solution->adaptiveSteps = adaptiveSteps;
    solution->time = time;

    return solution;
}

template class SolverAgros<double>;

template class SolutionArray<double>;

