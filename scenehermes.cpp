#include "scenehermes.h"

ThreadSolver::ThreadSolver(Scene *scene)
{
    this->m_scene = scene;
    this->m_mode = SOLVER_MESH;
}

ThreadSolver::~ThreadSolver()
{
    delete processTriangle;
}

void ThreadSolver::run()
{
    runMesh();
}

void ThreadSolver::runMesh()
{
    m_scene->setMeshed(false);
    m_errorMessage = "";

    m_scene->sceneSolution()->mesh().free();
    QFile::remove(m_scene->projectInfo().fileName + ".mesh");

    // create triangle files
    m_scene->writeToTriangle();
    emit message(tr("Triangle files was created."));
    updateProgress(20);

    // exec triangle
    QProcess *processTriangle = new QProcess();
    processTriangle->setStandardOutputFile(m_scene->projectInfo().fileName + ".triangle.out");
    processTriangle->setStandardErrorFile(m_scene->projectInfo().fileName + ".triangle.err");
    connect(processTriangle, SIGNAL(finished(int)), this, SLOT(doMeshTriangleCreated(int)));

    QFileInfo fileInfo(m_scene->projectInfo().fileName);
    processTriangle->start("triangle -p -P -q30.000000 -e -A -a -z -Q -I -p \"" + fileInfo.absoluteFilePath() + "\"");
    updateProgress(30);

    if (!processTriangle->waitForStarted())
    {
        emit message(tr("Could not start triangle."));
        processTriangle->kill();
        return;
    }

    while (!processTriangle->waitForFinished())
        sleep(0.1);
}

void ThreadSolver::doMeshTriangleCreated(int exitCode)
{
    if (exitCode == 0)
    {
        emit message(tr("Triangle mesh was created."));
        updateProgress(40);

        // convert triangle mesh to hermes mesh
        if (m_scene->triangle2mesh(m_scene->projectInfo().fileName, m_scene->projectInfo().fileName))
        {
            emit message(tr("Triangle mesh was converted to Hermes mesh file."));

            QFile::remove(m_scene->projectInfo().fileName + ".poly");
            QFile::remove(m_scene->projectInfo().fileName + ".node");
            QFile::remove(m_scene->projectInfo().fileName + ".edge");
            QFile::remove(m_scene->projectInfo().fileName + ".ele");
            QFile::remove(m_scene->projectInfo().fileName + ".triangle.out");
            QFile::remove(m_scene->projectInfo().fileName + ".triangle.err");

            emit message(tr("Triangle files was deleted."));
            updateProgress(50);
        }
        else
        {
            QFile::remove(m_scene->projectInfo().fileName + ".mesh");
            QString msg(tr("Triangle mesh could not be converted to Hermes mesh file."));
            QMessageBox::warning(NULL, tr("Triangle to Hermes error"), msg);

            emit message(msg);
            return;
        }

        m_scene->setMeshed(true);

        if (m_mode == SOLVER_MESH_AND_SOLVE)
            runSolver();
    }
    else
    {
        QFile file(m_scene->projectInfo().fileName + ".triangle.out");

        if (!file.open(QIODevice::ReadOnly))
            return;

        QTextStream standardOutput(&file);
        m_errorMessage = standardOutput.readAll();

        QMessageBox::warning(NULL, tr("Triangle error"), m_errorMessage);

        file.close();
    }    
}

void ThreadSolver::runSolver()
{
    // benchmark
    QTime time;
    time.start();

    emit message(tr("Solving ..."));
    updateProgress(60);

    SolutionArray slnArray;
    QFileInfo fileInfo(m_scene->projectInfo().fileName + ".mesh");

    if (fileInfo.exists())
    {
        switch (m_scene->projectInfo().physicField)
        {
        case PHYSICFIELD_ELECTROSTATIC:
            {
                // edge markers
                ElectrostaticEdge *electrostaticEdge = new ElectrostaticEdge[m_scene->edges.count()+1];
                electrostaticEdge[0].type = PHYSICFIELDBC_NONE;
                electrostaticEdge[0].value = 0;
                for (int i = 0; i<m_scene->edges.count(); i++)
                {
                    if (m_scene->edgeMarkers.indexOf(m_scene->edges[i]->marker) == 0)
                    {
                        electrostaticEdge[i+1].type = PHYSICFIELDBC_NONE;
                        electrostaticEdge[i+1].value = 0;
                    }
                    else
                    {
                        SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(m_scene->edges[i]->marker);
                        electrostaticEdge[i+1].type = edgeElectrostaticMarker->type;
                        electrostaticEdge[i+1].value = edgeElectrostaticMarker->value;
                    }
                }

                // label markers
                ElectrostaticLabel *electrostaticLabel = new ElectrostaticLabel[m_scene->labels.count()];
                for (int i = 0; i<m_scene->labels.count(); i++)
                {
                    if (m_scene->labelMarkers.indexOf(m_scene->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelElectrostaticMarker *labelElectrostaticMarker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[i]->marker);

                        electrostaticLabel[i].charge_density = labelElectrostaticMarker->charge_density;
                        electrostaticLabel[i].permittivity = labelElectrostaticMarker->permittivity;
                    }
                }

                slnArray = electrostatic_main((m_scene->projectInfo().fileName + ".mesh").toStdString().c_str(),
                                              electrostaticEdge, electrostaticLabel,
                                              m_scene->projectInfo().numberOfRefinements,
                                              m_scene->projectInfo().polynomialOrder,
                                              m_scene->projectInfo().adaptivitySteps,
                                              (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] electrostaticEdge;
                delete [] electrostaticLabel;
            }
            break;
        case PHYSICFIELD_MAGNETOSTATIC:
            {
                // edge markers
                MagnetostaticEdge *magnetostaticEdge = new MagnetostaticEdge[m_scene->edges.count()+1];
                magnetostaticEdge[0].type = PHYSICFIELDBC_NONE;
                magnetostaticEdge[0].value = 0;
                for (int i = 0; i<m_scene->edges.count(); i++)
                {
                    if (m_scene->edgeMarkers.indexOf(m_scene->edges[i]->marker) == 0)
                    {
                        magnetostaticEdge[i+1].type = PHYSICFIELDBC_NONE;
                        magnetostaticEdge[i+1].value = 0;
                    }
                    else
                    {
                        SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker = dynamic_cast<SceneEdgeMagnetostaticMarker *>(m_scene->edges[i]->marker);
                        magnetostaticEdge[i+1].type = edgeMagnetostaticMarker->type;
                        magnetostaticEdge[i+1].value = edgeMagnetostaticMarker->value;
                    }
                }

                // label markers
                MagnetostaticLabel *magnetostaticLabel = new MagnetostaticLabel[m_scene->labels.count()];
                for (int i = 0; i<m_scene->labels.count(); i++)
                {
                    if (m_scene->labelMarkers.indexOf(m_scene->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[i]->marker);

                        magnetostaticLabel[i].current_density = labelMagnetostaticMarker->current_density;
                        magnetostaticLabel[i].permeability = labelMagnetostaticMarker->permeability;
                    }
                }

                slnArray = magnetostatic_main((m_scene->projectInfo().fileName + ".mesh").toStdString().c_str(),
                                              magnetostaticEdge, magnetostaticLabel,
                                              m_scene->projectInfo().numberOfRefinements,
                                              m_scene->projectInfo().polynomialOrder,
                                              m_scene->projectInfo().adaptivitySteps,
                                              (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] magnetostaticEdge;
                delete [] magnetostaticLabel;
            }
            break;
        case PHYSICFIELD_HEAT_TRANSFER:
            {
                // edge markers
                HeatEdge *heatEdge = new HeatEdge[m_scene->edges.count()+1];
                heatEdge[0].type = PHYSICFIELDBC_NONE;
                heatEdge[0].temperature = 0;
                heatEdge[0].heatFlux = 0;
                heatEdge[0].h = 0;
                heatEdge[0].externalTemperature = 0;
                for (int i = 0; i<m_scene->edges.count(); i++)
                {
                    if (m_scene->edgeMarkers.indexOf(m_scene->edges[i]->marker) == 0)
                    {
                        heatEdge[i+1].type = PHYSICFIELDBC_NONE;
                    }
                    else
                    {
                        SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(m_scene->edges[i]->marker);
                        heatEdge[i+1].type = edgeHeatMarker->type;
                        switch (edgeHeatMarker->type)
                        {
                        case PHYSICFIELDBC_HEAT_TEMPERATURE:
                            {
                                heatEdge[i+1].temperature = edgeHeatMarker->temperature;
                            }
                            break;
                        case PHYSICFIELDBC_HEAT_HEAT_FLUX:
                            {
                                heatEdge[i+1].heatFlux = edgeHeatMarker->heatFlux;
                                heatEdge[i+1].h = edgeHeatMarker->h;
                                heatEdge[i+1].externalTemperature = edgeHeatMarker->externalTemperature;
                            }
                            break;
                        }
                    }
                }

                // label markers
                HeatLabel *heatLabel = new HeatLabel[m_scene->labels.count()];
                for (int i = 0; i<m_scene->labels.count(); i++)
                {
                    if (m_scene->labelMarkers.indexOf(m_scene->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[i]->marker);
                        heatLabel[i].thermal_conductivity = labelHeatMarker->thermal_conductivity;
                        heatLabel[i].volume_heat = labelHeatMarker->volume_heat;
                    }
                }

                slnArray = heat_main((m_scene->projectInfo().fileName + ".mesh").toStdString().c_str(),
                                     heatEdge, heatLabel,
                                     m_scene->projectInfo().numberOfRefinements,
                                     m_scene->projectInfo().polynomialOrder,
                                     m_scene->projectInfo().adaptivitySteps,
                                     (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] heatEdge;
                delete [] heatLabel;
            }
            break;
        case PHYSICFIELD_ELASTICITY:
            {
                // edge markers
                ElasticityEdge *elasticityEdge = new ElasticityEdge[m_scene->edges.count()+1];
                elasticityEdge[0].typeX = PHYSICFIELDBC_NONE;
                elasticityEdge[0].typeY = PHYSICFIELDBC_NONE;
                elasticityEdge[0].forceX = 0;
                elasticityEdge[0].forceY = 0;
                for (int i = 0; i<m_scene->edges.count(); i++)
                {
                    if (m_scene->edgeMarkers.indexOf(m_scene->edges[i]->marker) == 0)
                    {
                        elasticityEdge[i+1].typeX = PHYSICFIELDBC_NONE;
                        elasticityEdge[i+1].typeY = PHYSICFIELDBC_NONE;
                        elasticityEdge[i+1].forceX = 0;
                        elasticityEdge[i+1].forceY = 0;
                    }
                    else
                    {
                        SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(m_scene->edges[i]->marker);
                        elasticityEdge[i+1].typeX = edgeElasticityMarker->typeX;
                        elasticityEdge[i+1].typeY = edgeElasticityMarker->typeY;
                        elasticityEdge[i+1].forceX = edgeElasticityMarker->forceX;
                        elasticityEdge[i+1].forceY = edgeElasticityMarker->forceY;
                    }
                }

                // label markers
                ElasticityLabel *elasticityLabel = new ElasticityLabel[m_scene->labels.count()];
                for (int i = 0; i<m_scene->labels.count(); i++)
                {
                    if (m_scene->labelMarkers.indexOf(m_scene->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(m_scene->labels[i]->marker);

                        elasticityLabel[i].young_modulus = labelElasticityMarker->young_modulus;
                        elasticityLabel[i].poisson_ratio = labelElasticityMarker->poisson_ratio;
                    }
                }

                slnArray = elasticity_main((m_scene->projectInfo().fileName + ".mesh").toStdString().c_str(),
                                           elasticityEdge, elasticityLabel,
                                           m_scene->projectInfo().numberOfRefinements,
                                           m_scene->projectInfo().polynomialOrder,
                                           (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] elasticityEdge;
                delete [] elasticityLabel;
            }
            break;
        default:
            cout << "Physical field '" + m_scene->projectInfo().physicFieldString().toStdString() + "' is not implemented. ThreadSolver::runSolver()" << endl;
            throw;
            break;
        }

        m_scene->sceneSolution()->setSln(slnArray.sln1, slnArray.space1, slnArray.sln2, slnArray.space2, slnArray.order1);

        emit message(tr("Problem was solved."));
    }
    else
    {
        emit message(tr("Hermes mesh file doesn't exist."));
        return;
    }

    updateProgress(100);

    m_scene->sceneSolution()->setTimeElapsed(time.elapsed());
}

// *************************************************************************************************************************

SolverDialog::SolverDialog(Scene *scene, QWidget *parent) : QDialog(parent)
{
    this->m_scene = scene;

    setMinimumSize(300, 120);
    setMaximumSize(minimumSize());
    setWindowIcon(getIcon("logo"));
    setWindowTitle(tr("Solve problem ..."));
    setWindowModality(Qt::ApplicationModal);

    resize(minimumSize());

    connect(m_scene->solver(), SIGNAL(finished()), this, SLOT(doSolved()));

    createControls();
}

SolverDialog::~SolverDialog()
{
    delete lblMessage;
    delete progressBar;
}

int SolverDialog::showDialog()
{
    m_scene->solve();

    return exec();
}

void SolverDialog::doShowMessage(const QString &message)
{
    lblMessage->setText(message);
}

void SolverDialog::createControls()
{
    lblMessage = new QLabel(this);
    lblMessage->setText(tr("Solve problem ..."));
    connect(m_scene->solver(), SIGNAL(message(QString)), lblMessage, SLOT(setText(QString)));

    progressBar = new QProgressBar(this);
    connect(m_scene->solver(), SIGNAL(updateProgress(int)), progressBar, SLOT(setValue(int)));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(lblMessage);
    layout->addWidget(progressBar);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SolverDialog::doSolved()
{
    accept();
}

void SolverDialog::doReject()
{
    m_scene->solver()->terminate();
    reject();
}
