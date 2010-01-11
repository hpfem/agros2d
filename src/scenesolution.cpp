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

#include "scenesolution.h"

SceneSolution::SceneSolution()
{
    m_timeStep = -1;

    m_mesh = NULL;
    m_solutionArrayList = NULL;
    m_slnContourView = NULL;
    m_slnScalarView = NULL;
    m_slnVectorXView = NULL;
    m_slnVectorYView = NULL;   
}

void SceneSolution::clear()
{
    m_timeStep = -1;

    // solution array
    if (m_solutionArrayList)
    {
        for (int i = 0; i < m_solutionArrayList->count(); i++)
        {
            delete m_solutionArrayList->value(i);
        }

        delete m_solutionArrayList;
        m_solutionArrayList = NULL;
    }

    // mesh
    if (m_mesh)
    {
        delete m_mesh;
        m_mesh = NULL;
    }

    // countour
    if (m_slnContourView)
    {
        delete m_slnContourView;
        m_slnContourView = NULL;
    }
    
    // scalar
    if (m_slnScalarView)
    {
        delete m_slnScalarView;
        m_slnScalarView = NULL;
    }
    
    // vector
    if (m_slnVectorXView)
    {
        delete m_slnVectorXView;
        m_slnVectorXView = NULL;
    }
    if (m_slnVectorYView)
    {
        delete m_slnVectorYView;
        m_slnVectorYView = NULL;
    }
}

void SceneSolution::loadMesh(QDomElement *element)
{
    QDomText text = element->childNodes().at(0).toText();

    // write content (saved mesh)
    QString fileName = tempProblemFileName() + ".mesh";
    QByteArray content;
    content.append(text.nodeValue());
    writeStringContentByteArray(fileName, QByteArray::fromBase64(content));

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    Mesh *mesh = new Mesh();
    mesh->load(fileName.toStdString().c_str());

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    setMesh(mesh);
}

void SceneSolution::saveMesh(QDomDocument *doc, QDomElement *element)
{
    if (isMeshed())
    {
        QString fileName = tempProblemFileName() + ".mesh";

        // save locale
        char *plocale = setlocale (LC_NUMERIC, "");
        setlocale (LC_NUMERIC, "C");

        m_mesh->save(fileName.toStdString().c_str());

        // set system locale
        setlocale(LC_NUMERIC, plocale);

        // read content
        QDomText text = doc->createTextNode(readFileContentByteArray(fileName).toBase64());
        element->appendChild(text);
    }
}

void SceneSolution::loadSolution(QDomElement *element)
{
    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();

    QDomNode n = element->firstChild();
    while(!n.isNull())
    {
        SolutionArray *solutionArray = new SolutionArray();
        solutionArray->load(&n.toElement());

        // add to the array
        solutionArrayList->append(solutionArray);

        n = n.nextSibling();
    }

    if (solutionArrayList->count() > 0)
        setSolutionArrayList(solutionArrayList);
}

void SceneSolution::saveSolution(QDomDocument *doc, QDomElement *element)
{
    if (isSolved())
    {
        for (int i = 0; i < timeStepCount(); i++)
        {
            QDomNode eleSolution = doc->createElement("solution");
            m_solutionArrayList->value(i)->save(doc, &eleSolution.toElement());
            element->appendChild(eleSolution);
        }
    }
}

Solution *SceneSolution::sln()
{
    return sln1();
}

Solution *SceneSolution::sln1()
{
    if (isSolved())
    {
        if (m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->sln)
            return m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->sln;
    }
    return NULL;
}

Solution *SceneSolution::sln2()
{
    if (isSolved())
        if ((Util::scene()->problemInfo()->hermes()->numberOfSolution() == 2) &&
            m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1)->sln)
            return m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution() + 1)->sln;
    return NULL;
}

Orderizer &SceneSolution::ordView()
{
    if (isSolved())
        return *m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->order;
}

double SceneSolution::adaptiveError()
{
    return (isSolved()) ? m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->adaptiveError : 100.0;
}

int SceneSolution::adaptiveSteps()
{
    return (isSolved()) ? m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->adaptiveSteps : 0.0;
}

int SceneSolution::findTriangleInVectorizer(const Vectorizer &vec, const Point &point)
{
    double4* vecVert = vec.get_vertices();
    int3* vecTris = vec.get_triangles();

    for (int i = 0; i < vec.get_num_triangles(); i++)
    {
        bool inTriangle = true;

        int k;
        double z;
        for (int l = 0; l < 3; l++)
        {
            k = l + 1;
            if (k == 3)
                k = 0;

            z = (vecVert[vecTris[i][k]][0] - vecVert[vecTris[i][l]][0]) * (point.y - vecVert[vecTris[i][l]][1]) -
                (vecVert[vecTris[i][k]][1] - vecVert[vecTris[i][l]][1]) * (point.x - vecVert[vecTris[i][l]][0]);

            if (z < 0)
            {
                inTriangle = false;
                break;
            }
        }

        if (inTriangle)
            return i;
    }

    return -1;
}

int SceneSolution::findTriangleInMesh(Mesh *mesh, const Point &point)
{
    for (int i = 0; i < mesh->get_num_elements(); i++)
    {
        bool inTriangle = true;
        
        Element *element = mesh->get_element_fast(i);
        
        int k;
        double z;
        for (int l = 0; l < 3; l++)
        {
            k = l + 1;
            if (k == 3)
                k = 0;
            
            z = (element->vn[k]->x - element->vn[l]->x) * (point.y - element->vn[l]->y) - (element->vn[k]->y - element->vn[l]->y) * (point.x - element->vn[l]->x);
            
            if (z < 0)
            {
                inTriangle = false;
                break;
            }
        }
        
        if (inTriangle)
            return i;
    }
    
    return -1;
}

void SceneSolution::setSolutionArrayList(QList<SolutionArray *> *solutionArrayList)
{
    m_solutionArrayList = solutionArrayList;
    setTimeStep(timeStepCount() / Util::scene()->problemInfo()->hermes()->numberOfSolution() - 1);
}

void SceneSolution::setTimeStep(int timeStep)
{
    m_timeStep = timeStep;
    if (!isSolved()) return;

    if (Util::scene()->problemInfo()->physicField() != PHYSICFIELD_ELASTICITY)
        m_vec.process_solution(sln1(), FN_DX_0, sln1(), FN_DY_0, EPS_NORMAL);

    Util::scene()->refresh();
}

double SceneSolution::time()
{
    if (isSolved())
    {
        if (m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->sln)
            return m_solutionArrayList->value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->time;
    }
    return 0.0;
}

void SceneSolution::setSlnContourView(ViewScalarFilter *slnScalarView)
{
    if (m_slnContourView)
    {
        delete m_slnContourView;
        m_slnContourView = NULL;
    }
    
    m_slnContourView = slnScalarView;
    m_linContourView.process_solution(m_slnContourView);
}

void SceneSolution::setSlnScalarView(ViewScalarFilter *slnScalarView)
{
    if (m_slnScalarView)
    {
        delete m_slnScalarView;
        m_slnScalarView = NULL;
    }
    
    m_slnScalarView = slnScalarView;
    m_linScalarView.process_solution(m_slnScalarView, FN_VAL_0);

    // deformed shape
    if (Util::scene()->problemInfo()->physicField() == PHYSICFIELD_ELASTICITY)
    {
        double3* linVert = m_linScalarView.get_vertices();

        double min =  1e100;
        double max = -1e100;
        for (int i = 0; i < m_linScalarView.get_num_vertices(); i++)
        {
            double x = linVert[i][0];
            double y = linVert[i][1];

            double dx = Util::scene()->sceneSolution()->sln1()->get_pt_value(x, y);
            double dy = Util::scene()->sceneSolution()->sln2()->get_pt_value(x, y);

            double dm = sqrt(sqr(dx) + sqr(dy));

            if (dm < min) min = dm;
            if (dm > max) max = dm;
        }

        RectPoint rect = Util::scene()->boundingBox();
        double k = qMax(rect.width(), rect.height()) / qMax(min, max) / 15.0;

        for (int i = 0; i < m_linScalarView.get_num_vertices(); i++)
        {
            double x = linVert[i][0];
            double y = linVert[i][1];

            double dx = Util::scene()->sceneSolution()->sln1()->get_pt_value(x, y);
            double dy = Util::scene()->sceneSolution()->sln2()->get_pt_value(x, y);

            linVert[i][0] += k*dx;
            linVert[i][1] += k*dy;
        }
    }
}

void SceneSolution::setSlnVectorView(ViewScalarFilter *slnVectorXView, ViewScalarFilter *slnVectorYView)
{
    if (m_slnVectorXView)
    {
        delete m_slnVectorXView;
        m_slnVectorXView = NULL;
    }
    if (m_slnVectorYView)
    {
        delete m_slnVectorYView;
        m_slnVectorYView = NULL;
    }
    
    m_slnVectorXView = slnVectorXView;
    m_slnVectorYView = slnVectorYView;
    
    m_vecVectorView.process_solution(m_slnVectorXView, FN_VAL_0, m_slnVectorYView, FN_VAL_0, EPS_LOW);
}

// **************************************************************************************************************************

ViewScalarFilter::ViewScalarFilter(MeshFunction* sln1, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) : Filter(sln1)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

ViewScalarFilter::ViewScalarFilter(MeshFunction* sln1, MeshFunction* sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) : Filter(sln1, sln2)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

double ViewScalarFilter::get_pt_value(double x, double y, int item)
{
    error("Not implemented");
}

void ViewScalarFilter::precalculate(int order, int mask)
{
    Quad2D* quad = quads[cur_quad];
    int np = quad->get_num_points(order);
    Node* node = new_node(FN_VAL_0, np);
    
    scalar *dudx1, *dudy1, *dudx2, *dudy2;
    scalar *value1, *value2;

    if (sln[0])
    {
        sln[0]->set_quad_order(order, FN_VAL | FN_DX | FN_DY);
        sln[0]->get_dx_dy_values(dudx1, dudy1);
        value1 = sln[0]->get_fn_values();
    }

    if (num == 2 && sln[1])
    {
        sln[1]->set_quad_order(order, FN_VAL | FN_DX | FN_DY);
        sln[1]->get_dx_dy_values(dudx2, dudy2);
        value2 = sln[1]->get_fn_values();
    }
    
    update_refmap();
    
    double *x = refmap->get_phys_x(order);
    double *y = refmap->get_phys_y(order);
    Element* e = refmap->get_active_element();
    
    SceneLabelMarker *labelMarker = Util::scene()->labels[e->marker]->marker;
    
    for (int i = 0; i < np; i++)
    {
        switch (m_physicFieldVariable)
        {
        case PHYSICFIELDVARIABLE_GENERAL_VARIABLE:
            {
                node->values[0][0][i] = value1[i];
            }
            break;
        case PHYSICFIELDVARIABLE_GENERAL_GRADIENT:
            {
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -dudx1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -dudy1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_GENERAL_CONSTANT:
            {
                SceneLabelGeneralMarker *marker = dynamic_cast<SceneLabelGeneralMarker *>(labelMarker);
                node->values[0][0][i] = marker->constant.number;
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
            {
                node->values[0][0][i] = value1[i];
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
            {
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -dudx1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -dudy1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -EPS0 * marker->permittivity.number * dudx1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -EPS0 * marker->permittivity.number * dudy1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = EPS0 * marker->permittivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                node->values[0][0][i] = 0.5 * EPS0 * marker->permittivity.number * (sqr(dudx1[i]) + sqr(dudy1[i]));
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                node->values[0][0][i] = marker->permittivity.number;
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
            {
                node->values[0][0][i] = value1[i];
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
            {
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = dudx1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = dudy1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_FLUX:
            {
                SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.number * dudx1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.number * dudy1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
            {
                SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);
                node->values[0][0][i] = marker->thermal_conductivity.number;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL:
            {
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i]));
                }
                else
                {
                    node->values[0][0][i] = sqrt(sqr(value1[i]) + sqr(value2[i])) * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL:
            {
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = value1[i];
                }
                else
                {
                    node->values[0][0][i] = value1[i] * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG:
            {
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = value2[i];
                }
                else
                {
                    node->values[0][0][i] = value2[i] * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY:
            {
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudx2[i]) + sqr(dudy1[i]) + sqr(dudy2[i]));
                }
                else
                {
                    node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudy2[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)) + sqr(dudx2[i] + ((x > 0) ? value2[i] / x[i] : 0.0)));
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL:
            {
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy1[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx1[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudx1[i]));
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy1[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx1[i] - ((x[i] > 0) ? value1[i] / x[i] : 0.0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)));
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG:
            {
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy2[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx2[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy2[i]) + sqr(dudx2[i]));
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy2[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx2[i] - ((x > 0) ? value2[i] / x[i] : 0.0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy2[i]) + sqr(dudx2[i] + ((x > 0) ? value2[i] / x[i] : 0.0)));
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudx2[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudx2[i]) + sqr(dudy1[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)) + sqr(dudx2[i] + ((x > 0) ? value2[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudy2[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)) + sqr(dudx2[i] + ((x > 0) ? value2[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy1[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx1[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudx1[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy1[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - (dudx1[i] - ((x[i] > 0) ? value1[i] / x[i] : 0.0)) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy2[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx2[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy2[i]) + sqr(dudx2[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy2[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - (dudx2[i] - ((x > 0) ? value2[i] / x[i] : 0.0)) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy2[i]) + sqr(dudx2[i] + ((x > 0) ? value2[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = sqrt(
                        sqr(marker->current_density_real.number) +
                        sqr(marker->current_density_imag.number));
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = marker->current_density_real.number;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = marker->current_density_imag.number;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                    {
                        node->values[0][0][i] = 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];
                    }
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
                    {

                    }
                }
                else
                {
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                    {
                        node->values[0][0][i] = 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];
                    }
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT)
                    {

                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i];
                }
                else
                {
                    node->values[0][0][i] = - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(
                            sqr(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                            sqr(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]));
                }
                else
                {
                    node->values[0][0][i] = sqrt(
                            sqr(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                            sqr(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]));
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]);
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * sqrt(sqr(dudx1[i]) + sqr(dudx2[i])) +
                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * sqrt(sqr(dudy1[i]) + sqr(dudy2[i])));
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = marker->current_density_real.number +
                                            marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        node->values[0][0][i] += 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];
                }
                else
                {
                    node->values[0][0][i] = marker->current_density_real.number +
                                            marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = marker->current_density_imag.number +
                                            marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        node->values[0][0][i] += 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i];
                }
                else
                {
                    node->values[0][0][i] = marker->current_density_imag.number +
                                            marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]);
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(
                            sqr(marker->current_density_real.number +
                                2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i] +
                                marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                            +
                            sqr(marker->current_density_imag.number +
                                2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i] +
                                marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]))
                            
                            );
                }
                else
                {
                    node->values[0][0][i] = sqrt(
                            sqr(marker->current_density_real.number +
                                2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i] +
                                marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                            +
                            sqr(marker->current_density_imag.number +
                                2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i] +
                                marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]))
                            );
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                    {
                        node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                                                0.5 / marker->conductivity.number * (
                                                        sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                                                        sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]))
                                                :
                                                0.0;
                    }
                }
                else
                {
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                    {
                        node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                                                0.5 / marker->conductivity.number * (
                                                        sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                                                        sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]))
                                                :
                                                0.0;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = (marker->conductivity.number > 0) ?
                                        sqr(marker->current_density_real.number -
                                            marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])) /
                                            marker->conductivity.number
                                            :
                                            0.0;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);

                node->values[0][0][i] = (marker->conductivity.number > 0) ?
                                        sqr(marker->current_density_real.number -
                                            marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])) /
                                            marker->conductivity.number : 0.0;

                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        node->values[0][0][i] += (marker->conductivity.number > 0.0) ?
                                                0.5 / marker->conductivity.number * (
                                                        sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                                                        sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]))
                                                :
                                                0.0;
                }
                else
                {
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        node->values[0][0][i] += (marker->conductivity.number > 0.0) ?
                                                0.5 / marker->conductivity.number * (
                                                        sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                                                        sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]))
                                                :
                                                0.0;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_LORENTZ_FORCE:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);

                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                node->values[0][0][i] = - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                                        +          ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i]))
                                                        +
                                                        dudx1[i] * (marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                   (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]));
                            }
                            else
                            {
                                node->values[0][0][i] = (dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                node->values[0][0][i] = - (0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                        +           ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                                        +
                                                        dudy1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));
                            }
                            else
                            {
                                node->values[0][0][i] = (dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                node->values[0][0][i] = sqrt(sqr(
                                                        0.5 * ( - ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                                                + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i]))
                                                        +
                                                        dudx1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                        ) + sqr(
                                                                node->values[0][0][i] = - (0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                                                        +           ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                                                                        +
                                                                                        dudy1[i] * (- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                                                ));
                            }
                            else
                            {
                                node->values[0][0][i] = sqrt(sqr(
                                                        (dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                                        ) + sqr(
                                                        (dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))

                                                                ));
                            }

                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                node->values[0][0][i] = - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))
                                                                 + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * (dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0))))
                                                        +
                                                        dudx1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]));
                            }
                            else
                            {
                                node->values[0][0][i] = (dudx1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                node->values[0][0][i] = - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                                 + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                                        +
                                                        - dudy1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]));
                            }
                            else
                            {
                                node->values[0][0][i] = (- dudy1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                  (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                node->values[0][0][i] = sqrt(sqr(
                                                            - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))
                                                                     + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * (dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0))))
                                                            +
                                                            dudx1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                             (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                    ) + sqr(
                                                            - 0.5 * (- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                                     + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i]))
                                                            +
                                                            - dudy1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                               (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                            ));
                            }
                            else
                            {
                                node->values[0][0][i] = sqrt(sqr(
                                                            (dudx1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                    (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                                        ) + sqr(
                                                            (- dudy1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                      (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                                            ));
                            }
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = 0.25 * (sqr(dudx1[i]) + sqr(dudy1[i])) / (marker->permeability.number * MU0);
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        node->values[0][0][i] += 0.25 * (sqr(dudx2[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0);
                }
                else
                {
                    node->values[0][0][i] = 0.25 * (sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                    if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                        node->values[0][0][i] += 0.25 * (sqr(dudy2[i]) + sqr(dudx2[i] + ((x > 0) ? value2[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                }
            }       
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = marker->permeability.number;
            }
            break;                
        case PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = marker->conductivity.number;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = marker->velocity_x.number - marker->velocity_angular.number * y[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = marker->velocity_y.number + marker->velocity_angular.number * x[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(marker->velocity_x.number - marker->velocity_angular.number * y[i]) +
                                                         sqr(marker->velocity_y.number + marker->velocity_angular.number * x[i]));
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = 0;
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = marker->velocity_y.number;
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = fabs(marker->velocity_y.number);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = marker->remanence.number;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE:
            {
                SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(labelMarker);
                node->values[0][0][i] = marker->remanence_angle.number;
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
            {
                node->values[0][0][i] = value1[i];
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
            {
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -dudx1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -dudy1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);

                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -marker->conductivity.number * dudx1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -marker->conductivity.number * dudy1[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = marker->conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
                node->values[0][0][i] = marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i]));
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
                node->values[0][0][i] = marker->conductivity.number;
            }
            break;
        case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
            {
                SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);

                // stress tensor
                double tz = marker->lambda() * (dudx1[i] + dudy2[i]);
                double tx = tz + 2*marker->mu() * dudx1[i];
                double ty = tz + 2*marker->mu() * dudy2[i];
                if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_AXISYMMETRIC)
                    tz += 2*marker->mu() * value1[i] / x[i];
                double txy = marker->mu() * (dudy1[i] + dudx2[i]);

                // Von Mises stress
                node->values[0][0][i] = 1.0/sqrt(2.0) * sqrt(sqr(tx - ty) + sqr(ty - tz) + sqr(tz - tx) + 6*sqr(txy));
            }
            break;
        default:
            cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilter::precalculate(int order, int mask)" << endl;
            throw;
            break;
        }
    }

    replace_cur_node(node);
}
