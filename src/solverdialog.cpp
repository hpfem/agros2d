#include "solverdialog.h"

SolverDialog::SolverDialog(Scene *scene, QWidget *parent) : QDialog(parent)
{
    m_scene = scene;

    setMinimumSize(350, 260);
    setMaximumSize(minimumSize());
    setWindowIcon(icon("logo"));
    setWindowTitle(tr("Solve problem ..."));   

    connect(this, SIGNAL(message(QString)), this, SLOT(doShowMessage(QString)));

    resize(minimumSize());

    createControls();}

SolverDialog::~SolverDialog()
{
    delete lblMessage;
    delete progressBar;
    delete lstMessage;
}

void SolverDialog::solve()
{
    lblMessage->setText(tr("Solve problem ..."));
    lstMessage->clear();
    progressBar->setValue(0);

    QApplication::processEvents();

    runMesh();
}

void SolverDialog::doShowMessage(const QString &message)
{
    lstMessage->insertPlainText(message + "\n");
    lstMessage->ensureCursorVisible();
    lblMessage->setText(message);

    QApplication::processEvents();
}

void SolverDialog::createControls()
{
    lblMessage = new QLabel(this);

    progressBar = new QProgressBar(this);
    connect(this, SIGNAL(updateProgress(int)), progressBar, SLOT(setValue(int)));

    lstMessage = new QTextEdit(this);
    lstMessage->setReadOnly(true);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(lblMessage);
    layout->addWidget(progressBar);
    layout->addWidget(lstMessage);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SolverDialog::doAccept()
{
    hide();
}

void SolverDialog::runMesh()
{
    // file info
    QFileInfo fileInfo(m_scene->projectInfo().fileName);

    m_errorMessage = "";

    m_scene->sceneSolution()->mesh().free();
    QFile::remove(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".mesh");

    // create triangle files
    m_scene->writeToTriangle();
    emit message(tr("Triangle poly file was created."));
    updateProgress(20);

    // exec triangle
    QProcess *processTriangle = new QProcess();
    processTriangle->setStandardOutputFile(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".triangle.out");
    processTriangle->setStandardErrorFile(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".triangle.err");
    connect(processTriangle, SIGNAL(finished(int)), this, SLOT(doMeshTriangleCreated(int)));

    processTriangle->start("triangle -p -P -q30.000000 -e -A -a -z -Q -I -p \"" + QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + "\"");
    updateProgress(30);

    if (!processTriangle->waitForStarted())
    {
        emit message(tr("Could not start triangle."));
        processTriangle->kill();
        emit solved();
        return;
    }  

    while (!processTriangle->waitForFinished()) {}
}

void SolverDialog::doMeshTriangleCreated(int exitCode)
{
    if (exitCode == 0)
    {
        // file info
        QFileInfo fileInfo(m_scene->projectInfo().fileName);

        emit message(tr("Triangle mesh files was created."));
        updateProgress(40);

        // convert triangle mesh to hermes mesh
        if (m_scene->triangle2mesh(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName(), QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName()))
        {
            emit message(tr("Triangle mesh was converted to Hermes mesh file."));

            QFile::remove(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".poly");
            QFile::remove(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".node");
            QFile::remove(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".edge");
            QFile::remove(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".ele");
            QFile::remove(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".triangle.out");
            QFile::remove(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".triangle.err");
            emit message(tr("Triangle mesh files was deleted."));

            if (m_mode == SOLVER_MESH)
                updateProgress(100);
            else
                updateProgress(50);
        }
        else
        {
            QFile::remove(m_scene->projectInfo().fileName + ".mesh");
            QString msg(tr("Triangle mesh could not be converted to Hermes mesh file."));
            QMessageBox::warning(QApplication::activeWindow(), tr("Triangle to Hermes error"), msg);

            emit message(msg);
            emit solved();
            return;
        }

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

    emit solved();
}

void SolverDialog::runSolver()
{
    // file info
    QFileInfo fileInfo(m_scene->projectInfo().fileName);
    QString fileName(QDir::temp().absolutePath() + "/carbon2d/" + fileInfo.fileName() + ".mesh");

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Solver was started: ") + physicFieldString(m_scene->projectInfo().physicField) + " (" + problemTypeString(m_scene->projectInfo().problemType) + ")");
    updateProgress(60);

    SolutionArray *solutionArray;

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

                solutionArray = electrostatic_main(this,
                                                   fileName.toStdString().c_str(),
                                                   electrostaticEdge, electrostaticLabel,
                                                   m_scene->projectInfo().numberOfRefinements,
                                                   m_scene->projectInfo().polynomialOrder,
                                                   m_scene->projectInfo().adaptivitySteps,
                                                   m_scene->projectInfo().adaptivityTolerance,
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

                solutionArray = magnetostatic_main(this,
                                                   fileName.toStdString().c_str(),
                                                   magnetostaticEdge, magnetostaticLabel,
                                                   m_scene->projectInfo().numberOfRefinements,
                                                   m_scene->projectInfo().polynomialOrder,
                                                   m_scene->projectInfo().adaptivitySteps,
                                                   m_scene->projectInfo().adaptivityTolerance,
                                                   (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] magnetostaticEdge;
                delete [] magnetostaticLabel;
            }
            break;
        case PHYSICFIELD_CURRENT:
            {
                // edge markers
                CurrentEdge *currentEdge = new CurrentEdge[m_scene->edges.count()+1];
                currentEdge[0].type = PHYSICFIELDBC_NONE;
                currentEdge[0].value = 0;
                for (int i = 0; i<m_scene->edges.count(); i++)
                {
                    if (m_scene->edgeMarkers.indexOf(m_scene->edges[i]->marker) == 0)
                    {
                        currentEdge[i+1].type = PHYSICFIELDBC_NONE;
                        currentEdge[i+1].value = 0;
                    }
                    else
                    {
                        SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(m_scene->edges[i]->marker);
                        currentEdge[i+1].type = edgeCurrentMarker->type;
                        currentEdge[i+1].value = edgeCurrentMarker->value;
                    }
                }

                // label markers
                CurrentLabel *currentLabel = new CurrentLabel[m_scene->labels.count()];
                for (int i = 0; i<m_scene->labels.count(); i++)
                {
                    if (m_scene->labelMarkers.indexOf(m_scene->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[i]->marker);

                        currentLabel[i].conductivity = labelCurrentMarker->conductivity;
                    }
                }

                solutionArray = current_main(this,
                                             fileName.toStdString().c_str(),
                                             currentEdge, currentLabel,
                                             m_scene->projectInfo().numberOfRefinements,
                                             m_scene->projectInfo().polynomialOrder,
                                             m_scene->projectInfo().adaptivitySteps,
                                             m_scene->projectInfo().adaptivityTolerance,
                                             (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] currentEdge;
                delete [] currentLabel;
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

                solutionArray = heat_main(this,
                                          fileName.toStdString().c_str(),
                                          heatEdge, heatLabel,
                                          m_scene->projectInfo().numberOfRefinements,
                                          m_scene->projectInfo().polynomialOrder,
                                          m_scene->projectInfo().adaptivitySteps,
                                          m_scene->projectInfo().adaptivityTolerance,
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

                solutionArray = elasticity_main(fileName.toStdString().c_str(),
                                                elasticityEdge, elasticityLabel,
                                                m_scene->projectInfo().numberOfRefinements,
                                                m_scene->projectInfo().polynomialOrder,
                                                (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] elasticityEdge;
                delete [] elasticityLabel;
            }
            break;
        default:
            cout << "Physical field '" +  physicFieldStringKey(m_scene->projectInfo().physicField).toStdString() + "' is not implemented. SolverDialog::runSolver()" << endl;
            throw;
            break;
        }

        m_scene->sceneSolution()->setSolutionArray(solutionArray);

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
