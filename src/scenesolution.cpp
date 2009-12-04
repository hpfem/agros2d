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

double SceneSolution::volumeIntegral(int labelIndex, PhysicFieldIntegralVolume physicFieldIntegralVolume)
{
    if (!isSolved()) return 0.0;

    double integral = 0.0;

    Quad2D* quad;
    Mesh *mesh;
    Element* e;

    if (sln1())
    {
        quad = &g_quad_2d_std;
        sln1()->set_quad_2d(quad);

        mesh = sln1()->get_mesh();

        for_all_active_elements(e, mesh)
        {
            if (e->marker == labelIndex)
            {
                update_limit_table(e->get_mode());

                sln1()->set_active_element(e);
                if (sln2())
                    sln2()->set_active_element(e);

                RefMap *ru = sln1()->get_refmap();

                int o;
                if (!sln2())
                    o = sln1()->get_fn_order() + ru->get_inv_ref_order();
                else
                    o = sln1()->get_fn_order() + sln2()->get_fn_order() + ru->get_inv_ref_order();

                limit_order(o);

                scalar *dudx1, *dudy1, *dudx2, *dudy2;
                scalar *value1, *value2;
                double *x, *y;

                // solution 1
                sln1()->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                // value
                value1 = sln1()->get_fn_values();
                // derivative
                sln1()->get_dx_dy_values(dudx1, dudy1);
                // coordinates
                x = ru->get_phys_x(o);
                y = ru->get_phys_y(o);

                // solution 2
                if (sln2())
                {
                    sln2()->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                    // value
                    value2 = sln2()->get_fn_values();
                    // derivative
                    sln2()->get_dx_dy_values(dudx2, dudy2);
                }

                update_limit_table(e->get_mode());

                double result = 0.0;
                switch (physicFieldIntegralVolume)
                {
                case PHYSICFIELDINTEGRAL_VOLUME_CROSSSECTION:
                    {
                        h1_integrate_expression(1);
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_VOLUME:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(1);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_X:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudx1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * dudx1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_Y:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudy1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * dudy1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_X:
                    {
                        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(EPS0 * marker->permittivity.number * dudx1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.number * dudx1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_Y:
                    {
                        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(EPS0 * marker->permittivity.number * dudy1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.number * dudy1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT:
                    {
                        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(EPS0 * marker->permittivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ENERGY_DENSITY:
                    {
                        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(0.5 * EPS0 * marker->permittivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * 0.5 * EPS0 * marker->permittivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_REAL:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        h1_integrate_expression(marker->current_density_real.number);
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_IMAG:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        h1_integrate_expression(marker->current_density_imag.number);
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(- 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(- 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                            h1_integrate_expression(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]));
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                            h1_integrate_expression(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                     (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]));
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_TRANSFORM:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                                    0.5 / marker->conductivity.number * (
                                                              sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i])
                                                            + sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]))
                                                    :
                                                    0.0);
                        }
                        else
                        {
                            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                                    2 * M_PI * x[i] * 0.5 / marker->conductivity.number * (
                                                            sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i])
                                                          + sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]))
                                                    :
                                                    0.0);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_VELOCITY:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                                        1.0 / marker->conductivity.number * (
                                                                  sqr(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                                  +
                                                                  sqr(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])))
                                                        :
                                                        0.0);
                            }
                            else
                            {
                                h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                                        1.0 / marker->conductivity.number * (
                                                                  sqr(- marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                       (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                                        :
                                                        0.0);

                            }
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_ENERGY_DENSITY:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(0.25 * (sqr(dudx1[i]) + sqr(dudy1[i]) + sqr(dudx2[i]) + sqr(dudy2[i])) / (marker->permeability.number * MU0));
                            }
                            else
                            {
                                h1_integrate_expression(0.25 * (sqr(dudx1[i]) + sqr(dudy1[i])) / (marker->permeability.number * MU0));
                            }
                        }
                        else
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression((2 * M_PI * x[i] * 0.25 * sqr(sqrt(sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0))
                                                      + (2 * M_PI * x[i] * 0.25 * sqr(sqrt(sqr(dudy2[i]) + sqr(dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0)));
                            }
                            else
                            {
                                h1_integrate_expression(2 * M_PI * x[i] * 0.25 * sqr(sqrt(sqr(dudy1[i]) + sqr(dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0));
                            }
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_X_REAL:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                                        + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i])
                                                        +
                                                        dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));
                            }
                            else
                            {
                                h1_integrate_expression(dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                        else
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(- (2 * M_PI * x[i] * (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))
                                                        + (2 * M_PI * x[i] * (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * (dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0)))
                                                        +
                                                        dudx1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));
                            }
                            else
                            {
                                h1_integrate_expression(dudx1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_Y_REAL:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                        + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i])
                                                        +
                                                        - dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));
                            }
                            else
                            {
                                h1_integrate_expression(- dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                        else
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(- (2 * M_PI * x[i] * (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                        + (2 * M_PI * x[i] * (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i])
                                        +
                                        - dudy1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));
                            }
                            else
                            {
                                h1_integrate_expression(- dudy1[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_X_IMAG:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(((marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx1[i])
                                                      + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx2[i])
                                                      +
                                                      - dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));
                            }
                            else
                            {
                                h1_integrate_expression(- dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));

                            }
                        }
                        else
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression((2 * M_PI * x[i] * (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * (dudx1[i] + ((x[i] > 0) ? value1[i] / x[i] : 0.0)))
                                                      + (2 * M_PI * x[i] * (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * (dudx2[i] + ((x[i] > 0) ? value2[i] / x[i] : 0.0)))
                                +
                                dudx2[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));
                            }
                            else
                            {
                                h1_integrate_expression(dudx2[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));

                            }
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_Y_IMAG:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(((marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy1[i])
                                                        + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy2[i])
                                                        +
                                                        - dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));
                            }
                            else
                            {
                                h1_integrate_expression(- dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));

                            }
                        }
                        else
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression((2 * M_PI * x[i] * (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy1[i])
                                                      + (2 * M_PI * x[i] * (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy2[i])
                                                      +
                                                      - dudy2[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));
                            }
                            else
                            {
                                h1_integrate_expression(- dudy2[i] * (marker->current_density_imag.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));

                            }
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_TORQUE_REAL:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(y[i] * (
                                                                - ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx1[i])
                                                                + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx2[i])
                                                                +
                                                                dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                                        +
                                                        x[i] * (
                                                                - ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy1[i])
                                                                + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy2[i])
                                                                +
                                                                - dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                                   (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])))
                                                        );
                            }
                            else
                            {
                                h1_integrate_expression(y[i] *
                                                        dudx1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                         (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i]))
                                                        +
                                                        x[i] *
                                                        - dudy1[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx1[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy1[i])));

                            }
                        }
                        else
                        {
                            h1_integrate_expression(0.0);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_TORQUE_IMAG:
                    {
                        SceneLabelMagneticMarker *marker = dynamic_cast<SceneLabelMagneticMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            if (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_HARMONIC)
                            {
                                h1_integrate_expression(y[i] * (
                                                                  ((marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudx1[i])
                                                                + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudx2[i])
                                                                +
                                                                - dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                                                                 (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])))
                                                        +
                                                        x[i] * (
                                                                ((marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]) * dudy1[i])
                                                                + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) * dudy2[i])
                                                                +
                                                                - dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                                   (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])))
                                                        );
                            }
                            else
                            {
                                h1_integrate_expression(y[i] *
                                                        - dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i]))
                                                        +
                                                        x[i] *
                                                        - dudy2[i] * (marker->current_density_real.number - marker->conductivity.number * ((marker->velocity_x.number - marker->velocity_angular.number * y[i]) * dudx2[i] +
                                                                                                                                           (marker->velocity_y.number + marker->velocity_angular.number * x[i]) * dudy2[i])));

                            }
                        }
                        else
                        {
                            h1_integrate_expression(0.0);
                        }
                    }
                    break;                    
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(value1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * value1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_X:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudx1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * dudx1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_Y:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudy1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * dudy1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_X:
                    {
                        SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->thermal_conductivity.number * dudx1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.number * dudy1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_Y:
                    {
                        SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->thermal_conductivity.number * dudy1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.number * dudy1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX:
                    {
                        SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->thermal_conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_X:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudx1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * dudx1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_Y:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudy1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * dudy1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD:
                    {
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_X:
                    {
                        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * dudx1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * dudx1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_Y:
                    {
                        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * dudy1[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * dudy1[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY:
                    {
                        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * sqrt(sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_LOSSES:
                    {
                        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[e->marker]->marker);
                        if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * (sqr(dudx1[i]) + sqr(dudy1[i])));
                        }
                    }
                    break;
                }

                integral += result;
            }
        }
    }

    return integral;
}

double SceneSolution::surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface)
{  
    if (!isSolved()) return 0.0;

    double integral = 0.0;
    Quad2D* quad = &g_quad_2d_std;
    sln1()->set_quad_2d(quad);

    Element* e;
    Mesh* mesh = sln1()->get_mesh();

    for_all_active_elements(e, mesh)
    {
        for (int edge = 0; edge < e->nvert; edge++)
        {
            if (e->en[edge]->bnd && e->en[edge]->marker-1 == edgeIndex)
                // if (e->marker == 1)
            {
                // if (edges[e->en[edge]->id] != 1) continue;

                update_limit_table(e->get_mode());

                sln1()->set_active_element(e);
                RefMap* ru = sln1()->get_refmap();

                Quad2D* quad2d = ru->get_quad_2d();
                int eo = quad2d->get_edge_points(edge);
                sln1()->set_quad_order(eo, FN_VAL | FN_DX | FN_DY);
                double3* pt = quad2d->get_points(eo);
                double3* tan = ru->get_tangent(edge);

                // value
                scalar* valueu = sln1()->get_fn_values();
                // derivative
                scalar *dudx, *dudy;
                sln1()->get_dx_dy_values(dudx, dudy);
                // x - coordinate
                double* x = ru->get_phys_x(eo);

                double result = 0;
                for (int i = 0; i < quad2d->get_num_points(eo); i++)
                {
                    switch (physicFieldIntegralSurface)
                    {
                    case PHYSICFIELDINTEGRAL_SURFACE_LENGTH:
                        {
                            result += pt[i][2] * tan[i][2];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_SURFACE:
                        {
                            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2];
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_ELECTROSTATIC_CHARGE_DENSITY:
                        {
                            SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[e->marker]->marker);
                            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE:
                        {
                            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * valueu[i];
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * valueu[i];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE_DIFFERENCE:
                        {
                            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_FLUX:
                        {
                            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[e->marker]->marker);
                            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * marker->thermal_conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->thermal_conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_CURRENT_CURRENT_DENSITY:
                        {
                            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[e->marker]->marker);
                            if (Util::scene()->problemInfo()->problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * marker->conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->conductivity.number * (- tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                        }
                        break;
                    default:
                        cerr << "Physical field boundary integral '" + QString::number(physicFieldIntegralSurface).toStdString() + "' is not implemented. SceneSolution::surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface)" << endl;
                        throw;
                        break;
                    }                   
                }

                integral += result;
            }
        }
    }

    return integral / 2.0;
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

PointValue SceneSolution::pointValue(const Point &point, Solution *sln)
{
    double value = 0;
    double dx = 0;
    double dy = 0;
    SceneLabelMarker *labelMarker = NULL;
    
    int index = -1;
    if (sln1() != NULL)
    {
        value = sln->get_pt_value(point.x, point.y, FN_VAL_0);
        if (Util::scene()->problemInfo()->physicField() != PHYSICFIELD_ELASTICITY)
        {
            dx =  sln->get_pt_value(point.x, point.y, FN_DX_0);
            dy =  sln->get_pt_value(point.x, point.y, FN_DY_0);
        }
    }
    
    // find marker
    index = findTriangleInMesh(m_mesh, point);    
    if (index > 0)
    {
        Element *element = m_mesh->get_element_fast(index);
        labelMarker = Util::scene()->labels[element->marker]->marker;
    }

    return PointValue(value, Point(dx, dy), labelMarker);
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
                    node->values[0][0][i] = 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];
                }
                else
                {
                    node->values[0][0][i] = 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i];
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
                    node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                                            0.5 / marker->conductivity.number * (
                                                    sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                                                    sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]))
                                            :
                                            0.0;
                }
                else
                {
                    node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                                            0.5 / marker->conductivity.number * (
                                                    sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value2[i]) +
                                                    sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo()->frequency * marker->conductivity.number * value1[i]))
                                            :
                                            0.0;
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
                                            marker->conductivity.number : 0.0;
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
