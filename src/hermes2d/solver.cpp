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
#include "solver.h"
#include "progressdialog.h"
#include "module.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "module_agros.h"
#include "weakform_parser.h"

using namespace Hermes::Hermes2D;

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray()
{
    logMessage("SolutionArray::SolutionArray()");

    sln.reset();
    space.reset();

    time = 0.0;
    adaptiveSteps = 0;
    adaptiveError = 100.0;
}

template <typename Scalar>
SolutionArray<Scalar>::~SolutionArray()
{
    logMessage("SolutionArray::~SolutionArray()");
}

template <typename Scalar>
void SolutionArray<Scalar>::load(QDomElement element)
{
    logMessage("SolutionArray::load()");

    QString fileNameSolution = tempProblemFileName() + ".sln";
    QString fileNameSpace = tempProblemFileName() + ".spc";

    // write content (saved solution)
    QByteArray contentSolution;
    contentSolution.append(element.elementsByTagName("sln").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSolution, QByteArray::fromBase64(contentSolution));

    // write content (saved space)
    QByteArray contentSpace;
    contentSpace.append(element.elementsByTagName("space").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSpace, QByteArray::fromBase64(contentSpace));

    sln = shared_ptr<Solution<Scalar> >(new Hermes::Hermes2D::Solution<Scalar>());
    // sln->load(fileNameSolution.toStdString().c_str());
    //space = new Hermes::Hermes2D::Space<Scalar>();
    //space->load(fileNameSpace.toStdString().c_str());
    adaptiveError = element.attribute("adaptiveerror").toDouble();
    adaptiveSteps = element.attribute("adaptivesteps").toInt();
    time = element.attribute("time").toDouble();

    // delete solution
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

template <typename Scalar>
void SolutionArray<Scalar>::save(QDomDocument *doc, QDomElement element)
{
    logMessage("SolutionArray::save()");

    // solution
    QString fileNameSolution = tempProblemFileName() + ".sln";
    // sln->save(fileNameSolution.toStdString().c_str(), false);
    QDomText textSolution = doc->createTextNode(readFileContentByteArray(fileNameSolution).toBase64());

    // space
    QString fileNameSpace = tempProblemFileName() + ".spc";
    //space->save_data(fileNameSpace.toStdString().c_str());
    QDomNode textSpace = doc->createTextNode(readFileContentByteArray(fileNameSpace).toBase64());

    QDomNode eleSolution = doc->createElement("sln");
    QDomNode eleSpace = doc->createElement("space");

    eleSolution.appendChild(textSolution);
    eleSpace.appendChild(textSpace);

    element.setAttribute("adaptiveerror", adaptiveError);
    element.setAttribute("adaptivesteps", adaptiveSteps);
    element.setAttribute("time", time);
    element.appendChild(eleSolution);
    element.appendChild(eleSpace);

    // delete
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

// *********************************************************************************************


template <typename Scalar>
void MultiSolutionArray<Scalar>::add(SolutionArray<Scalar> solutionArray, int component)
{
    assert(!m_solutionArrays.contains(component));
    m_solutionArrays.push_back(solutionArray);
}

// *********************************************************************************************

template <typename Scalar>
void Solver<Scalar>::init(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf, Block* block)
{
    m_block = block;
    m_progressItemSolve = progressItemSolve;
    m_wf = wf;
    isError = false;
}

template <typename Scalar>
void Solver<Scalar>::readMesh()
{
    // load the mesh file
    cout << "reading mesh in solver " << tempProblemFileName().toStdString() + ".xml" << endl;
    mesh = readMeshFromFile(tempProblemFileName() + ".xml");

    refineMesh(m_block->m_fields.at(0)->fieldInfo(), mesh, true, true);  //TODO multimesh
    Util::problem()->setMeshInitial(mesh);
}


template <typename Scalar>
void Solver<Scalar>::createSpace()
{
    cout << "---- createSpace()" << endl;
    // essential boundary conditions
    Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> *> bcs;
    for (int i = 0; i < m_block->numSolutions(); i++)
        bcs.push_back(new Hermes::Hermes2D::EssentialBCs<double>());

    foreach(Field* field, m_block->m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        int index = 0;
        foreach(SceneEdge* edge, Util::scene()->edges->items())
        {
            SceneBoundary *boundary = edge->getMarker(fieldInfo);

            if (boundary && (!boundary->isNone()))
            {
                //printf(" ---- chci typ %s\n", boundary->getType().data());
                Hermes::Module::BoundaryType *boundary_type = fieldInfo->module()->get_boundary_type(boundary->getType());

                cout << " ---- bdr type " << boundary_type->id.data() << ", " << boundary_type->name.data() << endl;
                for (Hermes::vector<ParserFormEssential *>::iterator it = boundary_type->essential.begin();
                     it < boundary_type->essential.end(); ++it)
                {
                    ParserFormEssential *form = ((ParserFormEssential *) *it);

                    Hermes::Hermes2D::EssentialBoundaryCondition<Scalar> *custom_form = NULL;

                    // compiled form
                    if (fieldInfo->weakFormsType == WeakFormsType_Compiled)
                    {
                        assert(0);
                        //                    string problemId = m_fieldInfo->module()->fieldid + "_" +
                        //                            analysisTypeToStringKey(m_fieldInfo->module()->get_analysis_type()).toStdString()  + "_" +
                        //                            coordinateTypeToStringKey(m_fieldInfo->module()->get_problem_type()).toStdString();

                        //                    Hermes::Hermes2D::ExactSolutionScalar<double> * function = factoryExactSolution<double>(problemId, form->i-1, mesh, boundary);
                        //                    custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(i + 1).toStdString(),
                        //                                                                                           function);
                    }

                    if (!custom_form && fieldInfo->weakFormsType == WeakFormsType_Compiled)
                        qDebug() << "Cannot find compiled VectorFormEssential().";

                    // interpreted form
                    if (!custom_form || fieldInfo->weakFormsType == WeakFormsType_Interpreted)
                    {
                        {
                            CustomExactSolution<double> *function = new CustomExactSolution<double>(mesh,
                                                                                                    form->expression,
                                                                                                    boundary);
                            custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(index + 1).toStdString(),
                                                                                                   function);
                        }
                    }

                    if (custom_form)
                    {
                        bcs[form->i - 1 + m_block->offset(field)]->add_boundary_condition(custom_form);
                        cout << "adding BC i: " << form->i - 1 + m_block->offset(field) << " ( form i " << form->i << ", " << m_block->offset(field) << "), expression: " << form->expression << endl;
                    }
                }
            }
            index++;
        }

        // create space
        for (int i = 0; i < fieldInfo->module()->number_of_solution(); i++)
        {
            space.push_back(shared_ptr<Space<Scalar> >(new Hermes::Hermes2D::H1Space<Scalar>(mesh, bcs[i + m_block->offset(field)], fieldInfo->polynomialOrder)));

            int j = 0;
            // set order by element
            foreach(SceneLabel* label, Util::scene()->labels->items()){
                if (!label->getMarker(fieldInfo)->isNone())
                {
                    cout << "setting order to " << (label->polynomialOrder > 0 ? label->polynomialOrder : fieldInfo->polynomialOrder) << endl;
                    space.at(i)->set_uniform_order(label->polynomialOrder > 0 ? label->polynomialOrder : fieldInfo->polynomialOrder,
                                                   QString::number(j).toStdString());
                }
                cout << "doooofs " << space.at(i)->get_num_dofs() << endl;
                j++;
            }
        }
    }
}

template <typename Scalar>
void Solver<Scalar>::createSolutions(bool copyPrevious)
{
    foreach(Field* field, m_block->m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        for (int i = 0; i < fieldInfo->module()->number_of_solution(); i++)
        {
            // solution agros array
            Solution<double> *sln = new Solution<double>();
            solution.push_back(shared_ptr<Solution<double> >(sln));

            // single adaptive step
//            if (copyPrevious)
//                sln->copy((listOfSolutionArrays.at(listOfSolutionArrays.size() - m_fieldInfo->module()->number_of_solution() + i)->sln).get());

            if (fieldInfo->adaptivityType != AdaptivityType_None)
            {
                // reference solution
                solutionReference.push_back(shared_ptr<Solution<Scalar> >(new Solution<Scalar>()));
            }
        }
    }
}


int DEBUG_COUNTER = 0;

template <typename Scalar>
bool Solver<Scalar>::solveOneProblem(Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
                                     Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam)
{
    // Initialize the FE problem.
    Hermes::Hermes2D::DiscreteProblem<Scalar> dp(m_wf, castConst(desmartize(spaceParam)));

    // Linear solver
    if (m_block->linearityType() == LinearityType_Linear)
    {
        // set up the solver, matrix, and rhs according to the solver selection.
        Hermes::Algebra::SparseMatrix<Scalar> *matrix = create_matrix<Scalar>(Hermes::SOLVER_UMFPACK);
        Hermes::Algebra::Vector<Scalar> *rhs = create_vector<Scalar>(Hermes::SOLVER_UMFPACK);
        Hermes::Algebra::LinearSolver<Scalar> *solver = create_linear_solver<Scalar>(Hermes::SOLVER_UMFPACK, matrix, rhs);

        // assemble the linear problem.
        dp.assemble(matrix, rhs);

        if (solver->solve())
        {
            Hermes::Hermes2D::Solution<Scalar>::vector_to_solutions(solver->get_sln_vector(), castConst(desmartize(spaceParam)), desmartize(solutionParam));

            for(int i = 0; i < solver->get_matrix_size(); i++)
                cout << solver->get_sln_vector()[i] << ", ";
            cout << "  <- reseni " << endl;
            FILE *f;
            char fileName[30];
            sprintf(fileName, "rhs%d.txt", DEBUG_COUNTER);
            f = fopen(fileName, "w");
            rhs->dump(f, "rhs");
            fclose(f);

            sprintf(fileName, "mat%d.txt", DEBUG_COUNTER);
            f = fopen(fileName, "w");
            matrix->dump(f, "matrix");
            fclose(f);
            DEBUG_COUNTER++;

            Hermes::Hermes2D::Views::Linearizer lin;
            bool mode_3D = true;
            lin.save_solution_vtk(solutionParam[0].get(), "sln.vtk", "SLN", mode_3D);
        }
        else
        {
            m_progressItemSolve->emitMessage(QObject::tr("Linear solver failed."), true);
            return false;
        }

        delete matrix;
        delete rhs;
        delete solver;
    }

    // Nonlinear solver
    if (m_block->linearityType() == LinearityType_Newton)
    {
        // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
        Hermes::Hermes2D::NewtonSolver<Scalar> newton(&dp, Hermes::SOLVER_UMFPACK);
        //newton.set_max_allowed_residual_norm(1e15);
        try
        {
            int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(spaceParam)));

            // Initial coefficient vector for the Newton's method.
            Scalar* coeff_vec = new Scalar[ndof];
            memset(coeff_vec, 0, ndof*sizeof(Scalar));

            cout << "solving with nonlinear tolerance " << m_block->nonlinearTolerance() << " and nonlin steps " << m_block->nonlinearSteps() << endl;
            newton.solve(coeff_vec, m_block->nonlinearTolerance(), m_block->nonlinearSteps());

            Hermes::Hermes2D::Solution<Scalar>::vector_to_solutions(newton.get_sln_vector(), castConst(desmartize(spaceParam)), desmartize(solutionParam));

//            m_progressItemSolve->emitMessage(QObject::tr("Newton's solver - assemble: %1 s").
//                                             arg(milisecondsToTime(newton.get_assemble_time() * 1000.0).toString("mm:ss.zzz")), false);
//            m_progressItemSolve->emitMessage(QObject::tr("Newton's solver - solve: %1 s").
//                                             arg(milisecondsToTime(newton.get_solve_time() * 1000.0).toString("mm:ss.zzz")), false);

            //delete coeff_vec; //TODO nebo se to dela v resici???
        }
        catch(Hermes::Exceptions::Exception e)
        {
            QString error = QString(e.getMsg());
            m_progressItemSolve->emitMessage(QObject::tr("Newton's iteration failed: ") + error, true);
            return false;
        }
    }

//    if (m_fieldInfo->linearityType == LinearityType_Picard)
//    {
//    }

    return true;

}

template <typename Scalar>
void Solver<Scalar>::solve(SolverConfig config)
{
    QTime time;

    double error = 0.0;

    // read mesh from file
    readMesh();

    // create essential boundary conditions and space
    createSpace();

    qDebug() << "nodes: " << mesh->get_num_nodes();
    qDebug() << "elements: " << mesh->get_num_elements();
    qDebug() << "ndof: " << Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(desmartize(space)));

    // create solutions
    createSolutions();

    // init selectors
    //initSelectors();

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(space))) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
        //cleanup();
        return;
    }

    int actualTime = 0.0;
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(space), actualTime);
    m_wf->set_current_time(actualTime);

    m_wf->delete_all();
    m_wf->registerForms();

    if (!solveOneProblem(space, solution))
        isError = true;

    // output
    if (!isError)
    {

        //TODO dodelat ukladani do Problemu

//        for (int i = 0; i < m_block->numSolutions(); i++){
//            recordSolution(solution.at(i), space.at(i), error, actualAdaptivitySteps, 0.);
//        }
    }

    //cleanup();

    if (isError)
    {
        //clear();
    }

}


template class Solver<double>;
template class SolutionArray<double>;
