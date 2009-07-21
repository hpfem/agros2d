#include "solverdialog.h"

SolverDialog::SolverDialog(QWidget *parent) : QDialog(parent)
{
    setMinimumSize(350, 260);
    setMaximumSize(minimumSize());

    setWindowIcon(icon("logo"));
    setWindowTitle(tr("Solve problem..."));

    connect(this, SIGNAL(message(QString)), this, SLOT(doShowMessage(QString)));

    resize(minimumSize());

    createControls();}

SolverDialog::~SolverDialog()
{
    delete lblMessage;
    delete lstMessage;
    delete progressBar;
}

void SolverDialog::solve()
{
    lblMessage->setText(tr("Solve problem..."));
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
    QFileInfo fileInfo(Util::scene()->problemInfo().fileName);

    m_errorMessage = "";

    Util::scene()->sceneSolution()->mesh().free();
    QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".mesh");

    // create triangle files
    Util::scene()->writeToTriangle();
    emit message(tr("Triangle poly file was created."));
    updateProgress(20);

    // exec triangle
    QProcess *processTriangle = new QProcess();
    processTriangle->setStandardOutputFile(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".triangle.out");
    processTriangle->setStandardErrorFile(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".triangle.err");
    connect(processTriangle, SIGNAL(finished(int)), this, SLOT(doMeshTriangleCreated(int)));

    processTriangle->start("triangle -p -P -q30.0 -e -A -a -z -Q -I -p \"" + QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + "\"");
    updateProgress(30);

    if (!processTriangle->waitForStarted())
    {
        emit message(tr("Could not start Triangle."));
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
        QFileInfo fileInfo(Util::scene()->problemInfo().fileName);

        emit message(tr("Triangle mesh files was created."));
        updateProgress(40);

        // convert triangle mesh to hermes mesh
        if (Util::scene()->triangle2mesh(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName(), QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName()))
        {
            emit message(tr("Triangle mesh was converted to Hermes2D mesh file."));

            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".poly");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".node");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".edge");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".ele");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".triangle.out");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".triangle.err");
            emit message(tr("Triangle mesh files was deleted."));

            if (m_mode == SOLVER_MESH)
                updateProgress(100);
            else
                updateProgress(50);
        }
        else
        {
            QFile::remove(Util::scene()->problemInfo().fileName + ".mesh");
            QString msg(tr("Triangle mesh could not be converted to Hermes2D mesh file."));
            QMessageBox::warning(QApplication::activeWindow(), tr("Triangle to Hermes2D error."), msg);

            emit message(msg);
            emit solved();
            return;
        }

        if (m_mode == SOLVER_MESH_AND_SOLVE)
            runSolver();
    }
    else
    {
        QFile file(Util::scene()->problemInfo().fileName + ".triangle.out");

        if (!file.open(QIODevice::ReadOnly))
            return;

        QTextStream standardOutput(&file);
        m_errorMessage = standardOutput.readAll();

        QMessageBox::warning(NULL, tr("Triangle error."), m_errorMessage);

        file.close();
    }

    emit solved();
}

void SolverDialog::runSolver()
{
    // file info
    QFileInfo fileInfo(Util::scene()->problemInfo().fileName);
    QString fileName(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.fileName() + ".mesh");

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Solver was started: ") + physicFieldString(Util::scene()->problemInfo().physicField) + " (" + problemTypeString(Util::scene()->problemInfo().problemType) + ")");
    updateProgress(60);

    SolutionArray *solutionArray;

    if (fileInfo.exists())
    {
        switch (Util::scene()->problemInfo().physicField)
        {
        case PHYSICFIELD_ELECTROSTATIC:
            {
                // edge markers
                ElectrostaticEdge *electrostaticEdge = new ElectrostaticEdge[Util::scene()->edges.count()+1];
                electrostaticEdge[0].type = PHYSICFIELDBC_NONE;
                electrostaticEdge[0].value = 0;
                for (int i = 0; i<Util::scene()->edges.count(); i++)
                {
                    if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
                    {
                        electrostaticEdge[i+1].type = PHYSICFIELDBC_NONE;
                        electrostaticEdge[i+1].value = 0;
                    }
                    else
                    {
                        SceneEdgeElectrostaticMarker *edgeElectrostaticMarker = dynamic_cast<SceneEdgeElectrostaticMarker *>(Util::scene()->edges[i]->marker);

                        // evaluate script
                        if (!edgeElectrostaticMarker->value.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        electrostaticEdge[i+1].type = edgeElectrostaticMarker->type;
                        electrostaticEdge[i+1].value = edgeElectrostaticMarker->value.number;
                    }
                }

                // label markers
                ElectrostaticLabel *electrostaticLabel = new ElectrostaticLabel[Util::scene()->labels.count()];
                for (int i = 0; i<Util::scene()->labels.count(); i++)
                {
                    if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelElectrostaticMarker *labelElectrostaticMarker = dynamic_cast<SceneLabelElectrostaticMarker *>(Util::scene()->labels[i]->marker);

                        // evaluate script
                        if (!labelElectrostaticMarker->charge_density.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                        if (!labelElectrostaticMarker->permittivity.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        electrostaticLabel[i].charge_density = labelElectrostaticMarker->charge_density.number;
                        electrostaticLabel[i].permittivity = labelElectrostaticMarker->permittivity.number;
                    }
                }

                solutionArray = electrostatic_main(this,
                                                   fileName.toStdString().c_str(),
                                                   electrostaticEdge, electrostaticLabel,
                                                   Util::scene()->problemInfo().numberOfRefinements,
                                                   Util::scene()->problemInfo().polynomialOrder,
                                                   Util::scene()->problemInfo().adaptivitySteps,
                                                   Util::scene()->problemInfo().adaptivityTolerance,
                                                   (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] electrostaticEdge;
                delete [] electrostaticLabel;
            }
            break;
        case PHYSICFIELD_MAGNETOSTATIC:
            {
                // edge markers
                MagnetostaticEdge *magnetostaticEdge = new MagnetostaticEdge[Util::scene()->edges.count()+1];
                magnetostaticEdge[0].type = PHYSICFIELDBC_NONE;
                magnetostaticEdge[0].value = 0;
                for (int i = 0; i<Util::scene()->edges.count(); i++)
                {
                    if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
                    {
                        magnetostaticEdge[i+1].type = PHYSICFIELDBC_NONE;
                        magnetostaticEdge[i+1].value = 0;
                    }
                    else
                    {
                        SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker = dynamic_cast<SceneEdgeMagnetostaticMarker *>(Util::scene()->edges[i]->marker);

                        // evaluate script
                        if (!edgeMagnetostaticMarker->value.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        magnetostaticEdge[i+1].type = edgeMagnetostaticMarker->type;
                        magnetostaticEdge[i+1].value = edgeMagnetostaticMarker->value.number;
                    }
                }

                // label markers
                MagnetostaticLabel *magnetostaticLabel = new MagnetostaticLabel[Util::scene()->labels.count()];
                for (int i = 0; i<Util::scene()->labels.count(); i++)
                {
                    if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelMagnetostaticMarker *labelMagnetostaticMarker = dynamic_cast<SceneLabelMagnetostaticMarker *>(Util::scene()->labels[i]->marker);

                        // evaluate script
                        if (!labelMagnetostaticMarker->current_density.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                        if (!labelMagnetostaticMarker->permeability.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        magnetostaticLabel[i].current_density = labelMagnetostaticMarker->current_density.number;
                        magnetostaticLabel[i].permeability = labelMagnetostaticMarker->permeability.number;
                    }
                }

                solutionArray = magnetostatic_main(this,
                                                   fileName.toStdString().c_str(),
                                                   magnetostaticEdge, magnetostaticLabel,
                                                   Util::scene()->problemInfo().numberOfRefinements,
                                                   Util::scene()->problemInfo().polynomialOrder,
                                                   Util::scene()->problemInfo().adaptivitySteps,
                                                   Util::scene()->problemInfo().adaptivityTolerance,
                                                   (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] magnetostaticEdge;
                delete [] magnetostaticLabel;
            }
            break;
        case PHYSICFIELD_HARMONIC_MAGNETIC:
            {
                // edge markers
                HarmonicMagneticEdge *harmonicMagneticEdge = new HarmonicMagneticEdge[Util::scene()->edges.count()+1];
                harmonicMagneticEdge[0].type = PHYSICFIELDBC_NONE;
                harmonicMagneticEdge[0].value = 0;
                for (int i = 0; i<Util::scene()->edges.count(); i++)
                {
                    if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
                    {
                        harmonicMagneticEdge[i+1].type = PHYSICFIELDBC_NONE;
                        harmonicMagneticEdge[i+1].value = 0;
                    }
                    else
                    {
                        SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker = dynamic_cast<SceneEdgeHarmonicMagneticMarker *>(Util::scene()->edges[i]->marker);

                        // evaluate script
                        if (!edgeHarmonicMagneticMarker->value.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        harmonicMagneticEdge[i+1].type = edgeHarmonicMagneticMarker->type;
                        harmonicMagneticEdge[i+1].value = edgeHarmonicMagneticMarker->value.number;
                    }
                }

                // label markers
                HarmonicMagneticLabel *harmonicMagneticLabel = new HarmonicMagneticLabel[Util::scene()->labels.count()];
                for (int i = 0; i<Util::scene()->labels.count(); i++)
                {
                    if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(Util::scene()->labels[i]->marker);

                        // evaluate script
                        if (!labelHarmonicMagneticMarker->current_density_real.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                        if (!labelHarmonicMagneticMarker->current_density_imag.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                        if (!labelHarmonicMagneticMarker->permeability.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                        if (!labelHarmonicMagneticMarker->conductivity.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        harmonicMagneticLabel[i].current_density_real = labelHarmonicMagneticMarker->current_density_real.number;
                        harmonicMagneticLabel[i].current_density_imag = labelHarmonicMagneticMarker->current_density_imag.number;
                        harmonicMagneticLabel[i].permeability = labelHarmonicMagneticMarker->permeability.number;
                        harmonicMagneticLabel[i].conductivity = labelHarmonicMagneticMarker->conductivity.number;
                    }
                }

                solutionArray = harmonicmagnetic_main(this,
                                                      fileName.toStdString().c_str(),
                                                      harmonicMagneticEdge, harmonicMagneticLabel,
                                                      Util::scene()->problemInfo().numberOfRefinements,
                                                      Util::scene()->problemInfo().polynomialOrder,
                                                      Util::scene()->problemInfo().adaptivitySteps,
                                                      Util::scene()->problemInfo().adaptivityTolerance,
                                                      (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] magnetostaticEdge;
                delete [] magnetostaticLabel;
            }
            break;
        case PHYSICFIELD_CURRENT:
            {
                // edge markers
                CurrentEdge *currentEdge = new CurrentEdge[Util::scene()->edges.count()+1];
                currentEdge[0].type = PHYSICFIELDBC_NONE;
                currentEdge[0].value = 0;
                for (int i = 0; i<Util::scene()->edges.count(); i++)
                {
                    if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
                    {
                        currentEdge[i+1].type = PHYSICFIELDBC_NONE;
                        currentEdge[i+1].value = 0;
                    }
                    else
                    {
                        SceneEdgeCurrentMarker *edgeCurrentMarker = dynamic_cast<SceneEdgeCurrentMarker *>(Util::scene()->edges[i]->marker);

                        // evaluate script
                        if (!edgeCurrentMarker->value.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        currentEdge[i+1].type = edgeCurrentMarker->type;
                        currentEdge[i+1].value = edgeCurrentMarker->value.number;
                    }
                }

                // label markers
                CurrentLabel *currentLabel = new CurrentLabel[Util::scene()->labels.count()];
                for (int i = 0; i<Util::scene()->labels.count(); i++)
                {
                    if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelCurrentMarker *labelCurrentMarker = dynamic_cast<SceneLabelCurrentMarker *>(Util::scene()->labels[i]->marker);

                        // evaluate script
                        if (!labelCurrentMarker->conductivity.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        currentLabel[i].conductivity = labelCurrentMarker->conductivity.number;
                    }
                }

                solutionArray = current_main(this,
                                             fileName.toStdString().c_str(),
                                             currentEdge, currentLabel,
                                             Util::scene()->problemInfo().numberOfRefinements,
                                             Util::scene()->problemInfo().polynomialOrder,
                                             Util::scene()->problemInfo().adaptivitySteps,
                                             Util::scene()->problemInfo().adaptivityTolerance,
                                             (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] currentEdge;
                delete [] currentLabel;
            }
            break;
        case PHYSICFIELD_HEAT_TRANSFER:
            {
                // edge markers
                HeatEdge *heatEdge = new HeatEdge[Util::scene()->edges.count()+1];
                heatEdge[0].type = PHYSICFIELDBC_NONE;
                heatEdge[0].temperature = 0;
                heatEdge[0].heatFlux = 0;
                heatEdge[0].h = 0;
                heatEdge[0].externalTemperature = 0;
                for (int i = 0; i<Util::scene()->edges.count(); i++)
                {
                    if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
                    {
                        heatEdge[i+1].type = PHYSICFIELDBC_NONE;
                    }
                    else
                    {
                        SceneEdgeHeatMarker *edgeHeatMarker = dynamic_cast<SceneEdgeHeatMarker *>(Util::scene()->edges[i]->marker);
                        heatEdge[i+1].type = edgeHeatMarker->type;
                        switch (edgeHeatMarker->type)
                        {
                        case PHYSICFIELDBC_HEAT_TEMPERATURE:
                            {
                                // evaluate script
                                if (!edgeHeatMarker->temperature.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                                heatEdge[i+1].temperature = edgeHeatMarker->temperature.number;
                            }
                            break;
                        case PHYSICFIELDBC_HEAT_HEAT_FLUX:
                            {
                                // evaluate script
                                if (!edgeHeatMarker->heatFlux.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                                if (!edgeHeatMarker->h.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                                if (!edgeHeatMarker->externalTemperature.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                                heatEdge[i+1].heatFlux = edgeHeatMarker->heatFlux.number;
                                heatEdge[i+1].h = edgeHeatMarker->h.number;
                                heatEdge[i+1].externalTemperature = edgeHeatMarker->externalTemperature.number;
                            }
                            break;
                        }
                    }
                }

                // label markers
                HeatLabel *heatLabel = new HeatLabel[Util::scene()->labels.count()];
                for (int i = 0; i<Util::scene()->labels.count(); i++)
                {
                    if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelHeatMarker *labelHeatMarker = dynamic_cast<SceneLabelHeatMarker *>(Util::scene()->labels[i]->marker);

                        // evaluate script
                        if (!labelHeatMarker->thermal_conductivity.evaluate(Util::scene()->problemInfo().scriptStartup)) return;
                        if (!labelHeatMarker->volume_heat.evaluate(Util::scene()->problemInfo().scriptStartup)) return;

                        heatLabel[i].thermal_conductivity = labelHeatMarker->thermal_conductivity.number;
                        heatLabel[i].volume_heat = labelHeatMarker->volume_heat.number;
                    }
                }

                solutionArray = heat_main(this,
                                          fileName.toStdString().c_str(),
                                          heatEdge, heatLabel,
                                          Util::scene()->problemInfo().numberOfRefinements,
                                          Util::scene()->problemInfo().polynomialOrder,
                                          Util::scene()->problemInfo().adaptivitySteps,
                                          Util::scene()->problemInfo().adaptivityTolerance,
                                          (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] heatEdge;
                delete [] heatLabel;
            }
            break;
        case PHYSICFIELD_ELASTICITY:
            {
                // edge markers
                ElasticityEdge *elasticityEdge = new ElasticityEdge[Util::scene()->edges.count()+1];
                elasticityEdge[0].typeX = PHYSICFIELDBC_NONE;
                elasticityEdge[0].typeY = PHYSICFIELDBC_NONE;
                elasticityEdge[0].forceX = 0;
                elasticityEdge[0].forceY = 0;
                for (int i = 0; i<Util::scene()->edges.count(); i++)
                {
                    if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) == 0)
                    {
                        elasticityEdge[i+1].typeX = PHYSICFIELDBC_NONE;
                        elasticityEdge[i+1].typeY = PHYSICFIELDBC_NONE;
                        elasticityEdge[i+1].forceX = 0;
                        elasticityEdge[i+1].forceY = 0;
                    }
                    else
                    {
                        SceneEdgeElasticityMarker *edgeElasticityMarker = dynamic_cast<SceneEdgeElasticityMarker *>(Util::scene()->edges[i]->marker);
                        elasticityEdge[i+1].typeX = edgeElasticityMarker->typeX;
                        elasticityEdge[i+1].typeY = edgeElasticityMarker->typeY;
                        elasticityEdge[i+1].forceX = edgeElasticityMarker->forceX;
                        elasticityEdge[i+1].forceY = edgeElasticityMarker->forceY;
                    }
                }

                // label markers
                ElasticityLabel *elasticityLabel = new ElasticityLabel[Util::scene()->labels.count()];
                for (int i = 0; i<Util::scene()->labels.count(); i++)
                {
                    if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
                    {
                    }
                    else
                    {
                        SceneLabelElasticityMarker *labelElasticityMarker = dynamic_cast<SceneLabelElasticityMarker *>(Util::scene()->labels[i]->marker);

                        elasticityLabel[i].young_modulus = labelElasticityMarker->young_modulus;
                        elasticityLabel[i].poisson_ratio = labelElasticityMarker->poisson_ratio;
                    }
                }

                solutionArray = elasticity_main(fileName.toStdString().c_str(),
                                                elasticityEdge, elasticityLabel,
                                                Util::scene()->problemInfo().numberOfRefinements,
                                                Util::scene()->problemInfo().polynomialOrder,
                                                (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR));

                delete [] elasticityEdge;
                delete [] elasticityLabel;
            }
            break;
        default:
            cerr << "Physical field '" +  physicFieldStringKey(Util::scene()->problemInfo().physicField).toStdString() + "' is not implemented. SolverDialog::runSolver()" << endl;
            throw;
            break;
        }

        Util::scene()->sceneSolution()->setSolutionArray(solutionArray);

        emit message(tr("Problem was solved."));
    }
    else
    {
        emit message(tr("Hermes2D mesh file doesn't exist."));
        return;
    }

    updateProgress(100);

    Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
}
