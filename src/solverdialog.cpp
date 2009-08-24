#include "solverdialog.h"

#include "scene.h"

SolverDialog::SolverDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(350, 260);
    setMaximumSize(minimumSize());

    setWindowIcon(icon("system-run"));
    setWindowTitle(tr("Solve problem..."));

    connect(this, SIGNAL(message(QString, bool)), this, SLOT(doShowMessage(QString, bool)));

    resize(minimumSize());

    createControls();}

SolverDialog::~SolverDialog()
{
    delete progressBar;
    delete lblMessage;
    delete lstMessage;
}

void SolverDialog::solve()
{
    lblMessage->setText(tr("Solver: solve problem..."));
    lstMessage->clear();
    progressBar->setValue(0);

    QApplication::processEvents();

    runMesh();

    setFileNameOrig("");
}

void SolverDialog::doShowMessage(const QString &message, bool isError)
{
    if (isError)
        lstMessage->setTextColor(QColor(Qt::red));
    else
        lstMessage->setTextColor(QColor(Qt::black));

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

    Util::scene()->sceneSolution()->mesh().free();
    QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".mesh");

    // create triangle files
    if (writeToTriangle())
    {
        emit message(tr("Triangle: poly file was created."), false);
        updateProgress(20);

        // exec triangle
        QProcess *processTriangle = new QProcess();
        processTriangle->setStandardOutputFile(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".triangle.out");
        processTriangle->setStandardErrorFile(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".triangle.err");
        connect(processTriangle, SIGNAL(finished(int)), this, SLOT(doMeshTriangleCreated(int)));

        processTriangle->start("triangle -p -P -q30.0 -e -A -a -z -Q -I -p \"" + QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + "\"");
        updateProgress(30);

        if (!processTriangle->waitForStarted())
        {
            emit message(tr("Triangle: could not start Triangle."), false);
            processTriangle->kill();

            updateProgress(100);
            emit solved();
            return;
        }

        // copy triangle files
        QSettings settings;
        bool deleteTriangleFiles = settings.value("Solver/DeleteTriangleMeshFiles", true).value<bool>();

        if ((!deleteTriangleFiles) && (!m_fileNameOrig.isEmpty()))
        {
            QFileInfo fileInfoOrig(m_fileNameOrig);

            QFile::copy(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".poly", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".poly");
            QFile::copy(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".node", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".node");
            QFile::copy(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".edge", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".edge");
            QFile::copy(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".ele", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".ele");
        }

        while (!processTriangle->waitForFinished()) {}
    }
    else
    {
        // error
        updateProgress(100);
        return;
    }
}

void SolverDialog::doMeshTriangleCreated(int exitCode)
{
    if (exitCode == 0)
    {
        // file info
        QFileInfo fileInfo(Util::scene()->problemInfo().fileName);

        emit message(tr("Triangle: mesh files was created."), false);
        updateProgress(40);

        // convert triangle mesh to hermes mesh
        if (triangleToHermes2D())
        {
            emit message(tr("Triangle: mesh was converted to Hermes2D mesh file."), false);

            // copy triangle files
            QSettings settings;
            bool deleteHermes2D = settings.value("Solver/DeleteHermes2DMeshFile", true).value<bool>();

            if ((!deleteHermes2D) && (!m_fileNameOrig.isEmpty()))
            {               
                QFileInfo fileInfoOrig(m_fileNameOrig);

                QFile::copy(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".mesh", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove triangle temp files
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".poly");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".node");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".edge");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".ele");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".triangle.out");
            QFile::remove(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".triangle.err");
            emit message(tr("Triangle: mesh files was deleted."), false);

            if (m_mode == SOLVER_MESH)
                updateProgress(100);
            else
                updateProgress(50);

            // load mesh
            // save locale
            char *plocale = setlocale (LC_NUMERIC, "");
            setlocale (LC_NUMERIC, "C");

            Util::scene()->sceneSolution()->mesh().load((QDir::temp().absolutePath() + "/agros2d/temp.mesh").toStdString().c_str());

            // set system locale
            setlocale(LC_NUMERIC, plocale);

            // check that all boundary edges have a marker assigned
            for (int i = 0; i < Util::scene()->sceneSolution()->mesh().get_max_node_id(); i++)
            {
                if (Node *node = Util::scene()->sceneSolution()->mesh().get_node(i))
                {
                    if (node->used)
                    {
                        if (node->ref < 2 && node->marker == 0)
                        {
                            emit message(tr("Hermes2D: boundary edge node does not have a boundary marker."), true);
                            Util::scene()->sceneSolution()->mesh().free();
                            return;
                        }
                    }
                }
            }
        }
        else
        {
            // error
            updateProgress(100);

            QFile::remove(Util::scene()->problemInfo().fileName + ".mesh");
            return;
        }

        if (m_mode == SOLVER_MESH_AND_SOLVE)
            runSolver();

        emit solved();
    }
    else
    {
        // error
        updateProgress(100);

        QFile file(Util::scene()->problemInfo().fileName + ".triangle.out");

        QTextStream standardOutput(&file);
        QString errorMessage = standardOutput.readAll();
        emit message(tr("Triangle: ") + errorMessage, true);

        if (!file.open(QIODevice::ReadOnly))
            return;
        file.close();
    }
}

void SolverDialog::runSolver()
{
    // file info
    QFileInfo fileInfo(Util::scene()->problemInfo().fileName);
    QString fileName(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".mesh");

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Solver: solver was started: ") + physicFieldString(Util::scene()->problemInfo().physicField) + " (" + problemTypeString(Util::scene()->problemInfo().problemType) + ")", false);
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
                                                   electrostaticEdge, electrostaticLabel);

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
                                                   magnetostaticEdge, magnetostaticLabel);

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
                                                      harmonicMagneticEdge, harmonicMagneticLabel);

                delete [] harmonicMagneticEdge;
                delete [] harmonicMagneticLabel;
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
                                             currentEdge, currentLabel);

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
                                          heatEdge, heatLabel);

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
                                                elasticityEdge, elasticityLabel);

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

        emit message(tr("Solver: problem was solved."), false);
        Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
    }

    updateProgress(100);
}

bool SolverDialog::writeToTriangle()
{    
    // basic check
    if (Util::scene()->nodes.count() < 3)
    {
        emit message(tr("Triangle: invalid number of nodes (%1 < 3).").arg(Util::scene()->nodes.count()), true);
        return false;
    }
    if (Util::scene()->edges.count() < 3)
    {
        emit message(tr("Triangle: invalid number of edges (%1 < 3).").arg(Util::scene()->edges.count()), true);
        return false;
    }
    else
    {
        // at least one boundary condition has to be assigned
        int count = 0;
        for (int i = 0; i<Util::scene()->edges.count(); i++)
            if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) > 0)
                count++;

        if (count == 0)
        {
            emit message(tr("Triangle: at least one boundary condition has to be assigned."), true);
            return false;
        }
    }
    if (Util::scene()->labels.count() < 1)
    {
        emit message(tr("Triangle: invalid number of labels (%1 < 1).").arg(Util::scene()->labels.count()), true);
        return false;
    }
    else
    {
        // at least one material has to be assigned
        int count = 0;
        for (int i = 0; i<Util::scene()->labels.count(); i++)
            if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) > 0)
                count++;

        if (count == 0)
        {
            emit message(tr("Triangle: at least one material has to be assigned."), true);
            return false;
        }
    }
    if (Util::scene()->edgeMarkers.count() < 2) // + none marker
    {
        emit message(tr("Triangle: invalid number of boundary conditions (%1 < 1).").arg(Util::scene()->edgeMarkers.count()), true);
        return false;
    }
    if (Util::scene()->labelMarkers.count() < 2) // + none marker
    {
        emit message(tr("Triangle: invalid number of materials (%1 < 1).").arg(Util::scene()->labelMarkers.count()), true);
        return false;
    }

    // file info
    QFileInfo fileInfo(Util::scene()->problemInfo().fileName);

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDir dir;
    dir.mkdir(QDir::temp().absolutePath() + "/agros2d");
    QFile file(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".poly");

    if (!file.open(QIODevice::WriteOnly))
    {
        emit message(tr("Triangle: could not create triangle poly mesh file."), true);
        return false;
    }
    QTextStream out(&file);


    // Util::scene()->nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Util::scene()->nodes.count(); i++)
    {
        outNodes += QString("%1  %2  %3  %4\n").arg(i).arg(Util::scene()->nodes[i]->point.x, 0, 'f', 10).arg(Util::scene()->nodes[i]->point.y, 0, 'f', 10).arg(0);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->angle == 0)
        {
            // line
            outEdges += QString("%1  %2  %3  %4\n").arg(edgesCount).arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart)).arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd)).arg(i+1);
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudoUtil::scene()->nodes
            Point center = Util::scene()->edges[i]->center();
            double radius = Util::scene()->edges[i]->radius();
            double startAngle = atan2(center.y - Util::scene()->edges[i]->nodeStart->point.y, center.x - Util::scene()->edges[i]->nodeStart->point.x) / M_PI*180 - 180;
            int segments = Util::scene()->edges[i]->angle/5.0 + 1;
            if (segments < 5) segments = 5; // minimum segments

            double theta = Util::scene()->edges[i]->angle / float(segments - 1);

            int nodeStartIndex = 0;
            int nodeEndIndex = 0;
            for (int j = 0; j < segments; j++)
            {
                double arc = (startAngle + j*theta)/180.0*M_PI;
                double x = radius * cos(arc);
                double y = radius * sin(arc);

                nodeEndIndex = nodesCount+1;
                if (j == 0)
                {
                    nodeStartIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart);
                    nodeEndIndex = nodesCount;
                }
                if (j == segments-1)
                {
                    nodeEndIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd);
                }
                if ((j > 0) && (j < segments))
                {
                    outNodes += QString("%1  %2  %3  %4\n").arg(nodesCount).arg(center.x + x, 0, 'f', 10).arg(center.y + y, 0, 'f', 10).arg(0);
                    nodesCount++;
                }
                outEdges += QString("%1  %2  %3  %4\n").arg(edgesCount).arg(nodeStartIndex).arg(nodeEndIndex).arg(i+1);
                edgesCount++;
                nodeStartIndex = nodeEndIndex;
            }
        }
    }

    // holes
    int holesCount = 0;
    for (int i = 0; i<Util::scene()->labels.count(); i++) if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0) holesCount++;
    QString outHoles = QString("%1\n").arg(holesCount);
    for (int i = 0; i<Util::scene()->labels.count(); i++)
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
            outHoles += QString("%1  %2  %3\n").arg(i).arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).arg(Util::scene()->labels[i]->point.y, 0, 'f', 10);

    // labels
    QString outLabels;
    int labelsCount = 0;
    for(int i = 0; i<Util::scene()->labels.count(); i++)
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) > 0)
        {
        outLabels += QString("%1  %2  %3  %4  %5\n").arg(labelsCount).arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).arg(Util::scene()->labels[i]->point.y, 0, 'f', 10).arg(i).arg(Util::scene()->labels[i]->area);
        labelsCount++;
    }


    outNodes.insert(0, QString("%1 2 0 1\n").arg(nodesCount)); // + additional Util::scene()->nodes
    out << outNodes;
    outEdges.insert(0, QString("%1 1\n").arg(edgesCount)); // + additional edges
    out << outEdges;
    out << outHoles;
    outLabels.insert(0, QString("%1 1\n").arg(labelsCount)); // - holes
    out << outLabels;

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

bool SolverDialog::triangleToHermes2D()
{
    int i, n, k, l, count, marker, node_1, node_2, node_3;
    double x, y;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // file info
    QFileInfo fileInfo(Util::scene()->problemInfo().fileName);

    QFile fileMesh(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".mesh");
    if (!fileMesh.open(QIODevice::WriteOnly))
    {
        emit message(tr("Hermes2D: could not create hermes2d mesh file."), true);
        return false;
    }
    QTextStream outMesh(&fileMesh);

    QFile fileNode(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".node");
    if (!fileNode.open(QIODevice::ReadOnly))
    {
        emit message(tr("Hermes2D: could not read triangle node file."), true);
        return false;
    }
    QTextStream inNode(&fileNode);

    QFile fileEdge(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly))
    {
        emit message(tr("Hermes2D: could not read triangle edge file."), true);
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(QDir::temp().absolutePath() + "/agros2d/" + fileInfo.baseName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly))
    {
        emit message(tr("Hermes2D: could not read triangle ele file."), true);
        return false;
    }
    QTextStream inEle(&fileEle);

    // nodes
    QString outNodes;
    outNodes += "vertices = \n";
    outNodes += "{ \n";
    sscanf(inNode.readLine().toStdString().c_str(), "%i", &k);
    for (int i = 0; i<k; i++)
    {
        sscanf(inNode.readLine().toStdString().c_str(), "%i   %lf %lf %i", &n, &x, &y, &marker);
        outNodes += QString("\t{ %1,  %2 }, \n").arg(x, 0, 'f', 10).arg(y, 0, 'f', 10);
    }
    outNodes.truncate(outNodes.length()-3);
    outNodes += "\n} \n\n";

    // edges and curves
    QString outEdges;
    outEdges += "boundaries = \n";
    outEdges += "{ \n";
    sscanf(inEdge.readLine().toStdString().c_str(), "%i", &k);
    count = 0;
    for (int i = 0; i<k; i++)
    {
        sscanf(inEdge.readLine().toStdString().c_str(), "%i	%i	%i	%i", &n, &node_1, &node_2, &marker);
        if (marker != 0)
        {            
            if (Util::scene()->edges[marker-1]->marker->type != PHYSICFIELDBC_NONE)
            {
                count++;
                outEdges += QString("\t{ %1, %2, %3 }, \n").arg(node_1).arg(node_2).arg(abs(marker));
            }
        }
    }
    outEdges.truncate(outEdges.length()-3);
    outEdges += "\n} \n\n";
    if (count < 1)
    {
        emit message(tr("Hermes2D: invalid number of edge markers."), true);
        return false;
    }

    // elements
    QString outElements;
    outElements += "elements = \n";
    outElements += "{ \n";
    sscanf(inEle.readLine().toStdString().c_str(), "%i", &k);
    count = 0;
    for (int i = 0; i<k; i++)
    {
        count++;
        sscanf(inEle.readLine().toStdString().c_str(), "%i	%i	%i	%i	%i", &n, &node_1, &node_2, &node_3, &marker);
        outElements += QString("\t{ %1, %2, %3, %4  }, \n").arg(node_1).arg(node_2).arg(node_3).arg(abs(marker));
    }
    outElements.truncate(outElements.length()-3);
    outElements += "\n} \n\n";
    if (count < 1)
    {
        emit message(tr("Hermes2D: invalid number of label markers."), true);
        return false;
    }

    outMesh << outNodes;
    outMesh << outElements;
    outMesh << outEdges;

    fileNode.close();
    fileEdge.close();
    fileEle.close();

    fileMesh.waitForBytesWritten(0);
    fileMesh.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}
