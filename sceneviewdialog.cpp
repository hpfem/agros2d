#include "sceneviewdialog.h"

SceneViewDialog::SceneViewDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    this->m_sceneView = sceneView;
    
    setMinimumSize(350, 450);
    setWindowIcon(getIcon("logo"));
    setWindowTitle(tr("Scene View"));
    setWindowModality(Qt::ApplicationModal);
    
    createControls();
    
    load();
}

SceneViewDialog::~SceneViewDialog()
{
    // show
    delete chkShowGrid;
    delete chkShowGeometry;
    delete chkShowInitialMesh;
    delete chkShowSolutionMesh;
    delete chkShowContours;
    delete chkShowScalarField;
    delete chkShowVectors;
    delete chkShowOrder;

    // grid
    delete txtGridStep;

    // contours
    delete txtContoursCount;

    // scalar field
    delete cmbScalarFieldVariable;
    delete cmbScalarFieldVariableComp;
    delete chkScalarFieldRangeAuto;
    delete txtScalarFieldRangeMin;
    delete txtScalarFieldRangeMax;
    delete cmbPalette;
    delete chkPaletteFilter;
    delete txtPaletteSteps;    

    // vector field
    delete cmbVectorFieldVariable;
}

int SceneViewDialog::showDialog()
{
    return exec();
}

void SceneViewDialog::load()
{
    // show
    chkShowGrid->setChecked(m_sceneView->sceneViewSettings().showGrid);
    chkShowGeometry->setChecked(m_sceneView->sceneViewSettings().showGeometry);
    chkShowInitialMesh->setChecked(m_sceneView->sceneViewSettings().showInitialMesh);
    chkShowSolutionMesh->setChecked(m_sceneView->sceneViewSettings().showSolutionMesh);
    chkShowOrder->setChecked(m_sceneView->sceneViewSettings().showOrder);
    chkShowContours->setChecked(m_sceneView->sceneViewSettings().showContours);
    chkShowScalarField->setChecked(m_sceneView->sceneViewSettings().showScalarField);
    chkShowVectors->setChecked(m_sceneView->sceneViewSettings().showVectors);

    // grid
    txtGridStep->setText(QString::number(m_sceneView->sceneViewSettings().gridStep));

    // contours
    txtContoursCount->setText(QString::number(m_sceneView->sceneViewSettings().contoursCount));

    // scalar field
    cmbScalarFieldVariable->setCurrentIndex(cmbScalarFieldVariable->findData(m_sceneView->sceneViewSettings().scalarPhysicFieldVariable));
    doScalarFieldVariable(cmbScalarFieldVariable->currentIndex());
    cmbScalarFieldVariableComp->setCurrentIndex(cmbScalarFieldVariableComp->findData(m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp));
    cmbPalette->setCurrentIndex(cmbPalette->findData(m_sceneView->sceneViewSettings().paletteType));
    chkPaletteFilter->setChecked(m_sceneView->sceneViewSettings().paletteFilter);
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setText(QString::number(m_sceneView->sceneViewSettings().paletteSteps));
    chkScalarFieldRangeAuto->setChecked(m_sceneView->sceneViewSettings().scalarRangeAuto);
    doScalarFieldRangeAuto(chkScalarFieldRangeAuto->checkState());
    txtScalarFieldRangeMin->setText(QString::number(m_sceneView->sceneViewSettings().scalarRangeMin));
    txtScalarFieldRangeMax->setText(QString::number(m_sceneView->sceneViewSettings().scalarRangeMax));

    // vector field
    cmbVectorFieldVariable->setCurrentIndex(cmbVectorFieldVariable->findData(m_sceneView->sceneViewSettings().vectorPhysicFieldVariable));
}

void SceneViewDialog::save()
{
    // show
    m_sceneView->sceneViewSettings().showGrid = chkShowGrid->isChecked();
    m_sceneView->sceneViewSettings().showGeometry = chkShowGeometry->isChecked();
    m_sceneView->sceneViewSettings().showInitialMesh = chkShowInitialMesh->isChecked();
    m_sceneView->sceneViewSettings().showSolutionMesh = chkShowSolutionMesh->isChecked();
    m_sceneView->sceneViewSettings().showOrder = chkShowOrder->isChecked();
    m_sceneView->sceneViewSettings().showContours = chkShowContours->isChecked();
    m_sceneView->sceneViewSettings().showScalarField = chkShowScalarField->isChecked();
    m_sceneView->sceneViewSettings().showVectors = chkShowVectors->isChecked();

    // grid
    m_sceneView->sceneViewSettings().gridStep = txtGridStep->text().toDouble();

    // contours
    m_sceneView->sceneViewSettings().contoursCount = txtContoursCount->text().toInt();

    // scalar field
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = (PhysicFieldVariable) cmbScalarFieldVariable->itemData(cmbScalarFieldVariable->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = (PhysicFieldVariableComp) cmbScalarFieldVariableComp->itemData(cmbScalarFieldVariableComp->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().paletteType = (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().paletteFilter = chkPaletteFilter->isChecked();
    m_sceneView->sceneViewSettings().paletteSteps = txtPaletteSteps->text().toInt();
    m_sceneView->sceneViewSettings().scalarRangeAuto = chkScalarFieldRangeAuto->isChecked();
    m_sceneView->sceneViewSettings().scalarRangeMin = txtScalarFieldRangeMin->text().toDouble();
    m_sceneView->sceneViewSettings().scalarRangeMax = txtScalarFieldRangeMax->text().toDouble();

    // vector field
    m_sceneView->sceneViewSettings().vectorPhysicFieldVariable = (PhysicFieldVariable) cmbVectorFieldVariable->itemData(cmbVectorFieldVariable->currentIndex()).toInt();
}

void SceneViewDialog::createControls()
{
    // layoutShow
    chkShowGrid = new QCheckBox(tr("Grid"));
    chkShowGeometry = new QCheckBox(tr("Geometry"));
    chkShowInitialMesh = new QCheckBox(tr("Initial mesh"));
    chkShowSolutionMesh = new QCheckBox(tr("Solution mesh"));
    chkShowOrder = new QCheckBox(tr("Order"));
    chkShowContours = new QCheckBox(tr("Contours"));
    chkShowScalarField = new QCheckBox(tr("Scalar field"));
    chkShowVectors = new QCheckBox(tr("Vectors"));

    QGridLayout *layoutShow = new QGridLayout();
    layoutShow->addWidget(chkShowGrid, 0, 0);
    layoutShow->addWidget(chkShowGeometry, 1, 0);
    layoutShow->addWidget(chkShowInitialMesh, 2, 0);
    layoutShow->addWidget(chkShowSolutionMesh, 3, 0);
    layoutShow->addWidget(chkShowContours, 0, 1);
    layoutShow->addWidget(chkShowScalarField, 1, 1);
    layoutShow->addWidget(chkShowVectors, 2, 1);
    layoutShow->addWidget(chkShowOrder, 3, 1);

    QGroupBox *grpShow = new QGroupBox(tr("Show"));
    grpShow->setLayout(layoutShow);

    // layout scalar field
    cmbScalarFieldVariable = new QComboBox();
    switch (m_sceneView->scene()->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        {
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL), PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD), PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT), PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY);
        }
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        {
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY);
        }
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        {
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_FLUX), PHYSICFIELDVARIABLE_HEAT_FLUX);
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY), PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY);
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            cmbScalarFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS), PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS);
        }
        break;
    default:
        cerr << "Physical field '" + m_sceneView->scene()->projectInfo().physicFieldString().toStdString() + "' is not implemented. SceneViewDialog::createControls()" << endl;
        throw;
        break;
    }
    connect(cmbScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));

    cmbPalette = new QComboBox();
    cmbPalette->addItem(tr("Jet"), PALETTE_JET);
    cmbPalette->addItem(tr("Autumn"), PALETTE_AUTUMN);
    cmbPalette->addItem(tr("Hot"), PALETTE_HOT);
    cmbPalette->addItem(tr("Copper"), PALETTE_COPPER);
    cmbPalette->addItem(tr("Cool"), PALETTE_COOL);
    cmbPalette->addItem(tr("B/W ascending"), PALETTE_BW_ASC);
    cmbPalette->addItem(tr("B/W descending"), PALETTE_BW_DESC);

    cmbScalarFieldVariableComp = new QComboBox();

    chkScalarFieldRangeAuto = new QCheckBox();
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    txtScalarFieldRangeMin = new QLineEdit("0.1");
    txtScalarFieldRangeMin->setValidator(new QDoubleValidator(txtScalarFieldRangeMin));
    txtScalarFieldRangeMax = new QLineEdit("0.1");
    txtScalarFieldRangeMax->setValidator(new QDoubleValidator(txtScalarFieldRangeMax));

    chkPaletteFilter = new QCheckBox();
    connect(chkPaletteFilter, SIGNAL(stateChanged(int)), this, SLOT(doPaletteFilter(int)));

    txtPaletteSteps = new QLineEdit("0");
    txtPaletteSteps->setValidator(new QIntValidator(txtPaletteSteps));

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariable, 0, 1);
    layoutScalarField->addWidget(new QLabel(tr("Palette:")), 0, 2);
    layoutScalarField->addWidget(cmbPalette, 0, 3);

    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariableComp, 1, 1);

    layoutScalarField->addWidget(new QLabel(tr("Filter:")), 2, 0);
    layoutScalarField->addWidget(chkPaletteFilter, 2, 1);
    layoutScalarField->addWidget(new QLabel(tr("Steps:")), 2, 2);
    layoutScalarField->addWidget(txtPaletteSteps, 2, 3);

    layoutScalarField->addWidget(new QLabel(tr("Auto range:")), 3, 0);
    layoutScalarField->addWidget(chkScalarFieldRangeAuto, 3, 1);
    layoutScalarField->addWidget(new QLabel(tr("Minimum:")), 3, 2);
    layoutScalarField->addWidget(txtScalarFieldRangeMin, 3, 3);

    layoutScalarField->addWidget(new QLabel(tr("Maximum:")), 4, 2);
    layoutScalarField->addWidget(txtScalarFieldRangeMax, 4, 3);

    QGroupBox *grpScalarField = new QGroupBox(tr("Scalar field"));
    grpScalarField->setLayout(layoutScalarField);

    // layout vector field
    cmbVectorFieldVariable = new QComboBox();
    switch (m_sceneView->scene()->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        {
            cmbVectorFieldVariable->addItem(tr("Electric field"), PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD);
            cmbVectorFieldVariable->addItem(tr("Displacement"), PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT);
        }
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        {
            cmbVectorFieldVariable->addItem(tr("Flux density"), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
            cmbVectorFieldVariable->addItem(tr("Field intensity"), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
        }
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        {
            cmbVectorFieldVariable->addItem(tr("Temperature gradient"), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT);
            cmbVectorFieldVariable->addItem(tr("Heat flux"), PHYSICFIELDVARIABLE_HEAT_FLUX);
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            cmbVectorFieldVariable->addItem(tr("Temperature gradient"), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT);
        }
        break;
    default:
        cerr << "Physical field '" + m_sceneView->scene()->projectInfo().physicFieldString().toStdString() + "' is not implemented. SceneViewDialog::createControls()" << endl;
        throw;
        break;
    }

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutVectorField->addWidget(cmbVectorFieldVariable, 0, 1);
    layoutVectorField->addWidget(new QLabel(""), 0, 2);
    layoutVectorField->addWidget(new QLabel(""), 0, 3);

    QGroupBox *grpVectorField = new QGroupBox(tr("Vector field"));
    grpVectorField->setLayout(layoutVectorField);

    // layout grid
    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(new QLabel(tr("Step:")), 0, 0);
    layoutGrid->addWidget(txtGridStep, 0, 1);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    // layout contours
    txtContoursCount = new QLineEdit("0");
    txtContoursCount->setValidator(new QIntValidator(txtContoursCount));

    QGridLayout *layoutContours = new QGridLayout();
    layoutContours->addWidget(new QLabel(tr("Count:")), 0, 0);
    layoutContours->addWidget(txtContoursCount, 0, 1);

    QGroupBox *grpContours = new QGroupBox(tr("Contours"));
    grpContours->setLayout(layoutContours);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpShow);
    layout->addWidget(grpGrid);
    layout->addWidget(grpContours);
    layout->addWidget(grpScalarField);
    layout->addWidget(grpVectorField);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SceneViewDialog::doScalarFieldVariable(int index)
{
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbScalarFieldVariable->itemData(index).toInt();

    cmbScalarFieldVariableComp->clear();
    if (isPhysicFieldVariableScalar(physicFieldVariable))
    {
        cmbScalarFieldVariableComp->addItem(tr("Scalar"), PHYSICFIELDVARIABLECOMP_SCALAR);
    }
    else
    {
        cmbScalarFieldVariableComp->addItem(tr("Magnitude"), PHYSICFIELDVARIABLECOMP_MAGNITUDE);
        cmbScalarFieldVariableComp->addItem(tr("X"), PHYSICFIELDVARIABLECOMP_X);
        cmbScalarFieldVariableComp->addItem(tr("Y"), PHYSICFIELDVARIABLECOMP_Y);
    }

    if (cmbScalarFieldVariableComp->currentIndex() == -1)
        cmbScalarFieldVariableComp->setCurrentIndex(0);

}

void SceneViewDialog::doScalarFieldRangeAuto(int state)
{
    txtScalarFieldRangeMin->setEnabled(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setEnabled(!chkScalarFieldRangeAuto->isChecked());       
}

void SceneViewDialog::doPaletteFilter(int state)
{
    txtPaletteSteps->setEnabled(!chkPaletteFilter->isChecked());
}


void SceneViewDialog::doAccept()
{
    save();

    accept();
}

void SceneViewDialog::doReject()
{
    reject();
}
