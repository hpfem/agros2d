#include "sceneviewdialog.h"

SceneViewDialog::SceneViewDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;
       
    setWindowIcon(icon("scene-properties"));
    setWindowTitle(tr("Scene View"));

    createControls();
    
    load();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

SceneViewDialog::~SceneViewDialog()
{
    // show
    delete chkShowGrid;
    delete chkShowGeometry;
    delete chkShowInitialMesh;

    delete butPostprocessorGroup;
    delete radPostprocessorNone;
    delete radPostprocessorScalarField;
    delete radPostprocessorScalarField3D;
    // delete radPostprocessorScalarField3DSolid;
    delete radPostprocessorOrder;

    delete chkShowContours;
    delete chkShowVectors;
    delete chkShowSolutionMesh;

    // scalar field
    delete cmbScalarFieldVariable;
    delete cmbScalarFieldVariableComp;
    delete chkScalarFieldRangeAuto;
    delete txtScalarFieldRangeMin;
    delete txtScalarFieldRangeMax;

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

    radPostprocessorNone->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_NONE);
    radPostprocessorScalarField->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW);
    radPostprocessorScalarField3D->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D);
    // radPostprocessorScalarField3DSolid->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID);
    radPostprocessorOrder->setChecked(m_sceneView->sceneViewSettings().postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_ORDER);

    chkShowContours->setChecked(m_sceneView->sceneViewSettings().showContours);
    chkShowVectors->setChecked(m_sceneView->sceneViewSettings().showVectors);
    chkShowSolutionMesh->setChecked(m_sceneView->sceneViewSettings().showSolutionMesh);

    // scalar field
    cmbScalarFieldVariable->setCurrentIndex(cmbScalarFieldVariable->findData(m_sceneView->sceneViewSettings().scalarPhysicFieldVariable));
    doScalarFieldVariable(cmbScalarFieldVariable->currentIndex());
    cmbScalarFieldVariableComp->setCurrentIndex(cmbScalarFieldVariableComp->findData(m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp));
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

    if (radPostprocessorNone->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_NONE;
    if (radPostprocessorScalarField->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW;
    if (radPostprocessorScalarField3D->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D;
    // if (radPostprocessorScalarField3DSolid->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID;
    if (radPostprocessorOrder->isChecked()) m_sceneView->sceneViewSettings().postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_ORDER;

    m_sceneView->sceneViewSettings().showContours = chkShowContours->isChecked();
    m_sceneView->sceneViewSettings().showVectors = chkShowVectors->isChecked();
    m_sceneView->sceneViewSettings().showSolutionMesh = chkShowSolutionMesh->isChecked();

    // scalar field
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariable = (PhysicFieldVariable) cmbScalarFieldVariable->itemData(cmbScalarFieldVariable->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().scalarPhysicFieldVariableComp = (PhysicFieldVariableComp) cmbScalarFieldVariableComp->itemData(cmbScalarFieldVariableComp->currentIndex()).toInt();
    m_sceneView->sceneViewSettings().scalarRangeAuto = chkScalarFieldRangeAuto->isChecked();
    m_sceneView->sceneViewSettings().scalarRangeMin = txtScalarFieldRangeMin->text().toDouble();
    m_sceneView->sceneViewSettings().scalarRangeMax = txtScalarFieldRangeMax->text().toDouble();

    // vector field
    m_sceneView->sceneViewSettings().vectorPhysicFieldVariable = (PhysicFieldVariable) cmbVectorFieldVariable->itemData(cmbVectorFieldVariable->currentIndex()).toInt();
}

void SceneViewDialog::createControls()
{
    // layout show
    chkShowGrid = new QCheckBox(tr("Grid"));
    chkShowGeometry = new QCheckBox(tr("Geometry"));
    chkShowInitialMesh = new QCheckBox(tr("Initial mesh"));

    QHBoxLayout *layoutShow = new QHBoxLayout();
    layoutShow->addWidget(chkShowGrid);
    layoutShow->addWidget(chkShowGeometry);
    layoutShow->addWidget(chkShowInitialMesh);

    QGroupBox *grpShow = new QGroupBox(tr("Show"));
    grpShow->setLayout(layoutShow);

    // postprocessor mode
    radPostprocessorNone = new QRadioButton("None", this);
    radPostprocessorScalarField = new QRadioButton("Scalar view", this);
    radPostprocessorScalarField3D = new QRadioButton("Scalar view 3D", this);
    // radPostprocessorScalarField3DSolid = new QRadioButton("Scalar view 3D solid", this);
    radPostprocessorOrder = new QRadioButton("Order", this);

    butPostprocessorGroup = new QButtonGroup(this);
    butPostprocessorGroup->addButton(radPostprocessorNone);
    butPostprocessorGroup->addButton(radPostprocessorScalarField);
    butPostprocessorGroup->addButton(radPostprocessorScalarField3D);
    // butPostprocessorGroup->addButton(radPostprocessorScalarField3DSolid);
    butPostprocessorGroup->addButton(radPostprocessorOrder);

    QVBoxLayout *layoutPostprocessorMode = new QVBoxLayout();
    layoutPostprocessorMode->addWidget(radPostprocessorNone);
    layoutPostprocessorMode->addWidget(radPostprocessorScalarField);
    layoutPostprocessorMode->addWidget(radPostprocessorScalarField3D);
    // layoutPostprocessorMode->addWidget(radPostprocessorScalarField3DSolid);
    layoutPostprocessorMode->addWidget(radPostprocessorOrder);

    // postprocessor show
    chkShowContours = new QCheckBox(tr("Contours"));
    chkShowVectors = new QCheckBox(tr("Vectors"));
    chkShowSolutionMesh = new QCheckBox(tr("Solution mesh"));

    QVBoxLayout *layoutPostprocessorShow = new QVBoxLayout();
    layoutPostprocessorShow->addWidget(chkShowContours);
    layoutPostprocessorShow->addWidget(chkShowVectors);
    layoutPostprocessorShow->addWidget(chkShowSolutionMesh);
    layoutPostprocessorShow->addStretch();

    // postprocessor
    QHBoxLayout *layoutPostprocessor = new QHBoxLayout();
    layoutPostprocessor->addLayout(layoutPostprocessorMode);
    layoutPostprocessor->addLayout(layoutPostprocessorShow);

    QGroupBox *grpPostprocessor = new QGroupBox(tr("Postprocessor"));
    grpPostprocessor->setLayout(layoutPostprocessor);

    // layout scalar field
    cmbScalarFieldVariable = new QComboBox();
    fillComboBoxVariable(cmbScalarFieldVariable, Util::scene()->problemInfo().physicField);
    connect(cmbScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));

    cmbScalarFieldVariableComp = new QComboBox();

    chkScalarFieldRangeAuto = new QCheckBox();
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    txtScalarFieldRangeMin = new QLineEdit("0.1");
    txtScalarFieldRangeMin->setValidator(new QDoubleValidator(txtScalarFieldRangeMin));
    txtScalarFieldRangeMax = new QLineEdit("0.1");
    txtScalarFieldRangeMax->setValidator(new QDoubleValidator(txtScalarFieldRangeMax));

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariable, 0, 1);

    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbScalarFieldVariableComp, 1, 1);

    layoutScalarField->addWidget(new QLabel(tr("Auto range:")), 2, 0);
    layoutScalarField->addWidget(chkScalarFieldRangeAuto, 2, 1);
    layoutScalarField->addWidget(new QLabel(tr("Minimum:")), 3, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeMin, 3, 1);
    layoutScalarField->addWidget(new QLabel(tr("Maximum:")), 4, 0);
    layoutScalarField->addWidget(txtScalarFieldRangeMax, 4, 1);

    QGroupBox *grpScalarField = new QGroupBox(tr("Scalar field"));
    grpScalarField->setLayout(layoutScalarField);

    // layout vector field
    cmbVectorFieldVariable = new QComboBox();
    switch (Util::scene()->problemInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        {
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD), PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD);
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT), PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT);
        }
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        {
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
        }
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        {
        }
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        {
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT);
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_FLUX), PHYSICFIELDVARIABLE_HEAT_FLUX);
        }
        break;
    case PHYSICFIELD_CURRENT:
        {
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD), PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD);
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY), PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY);
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            cmbVectorFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT);
        }
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(Util::scene()->problemInfo().physicField).toStdString() + "' is not implemented. SceneViewDialog::createControls()" << endl;
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

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpShow);
    layout->addWidget(grpPostprocessor);
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
        cmbScalarFieldVariableComp->addItem(Util::scene()->problemInfo().labelX(), PHYSICFIELDVARIABLECOMP_X);
        cmbScalarFieldVariableComp->addItem(Util::scene()->problemInfo().labelY(), PHYSICFIELDVARIABLECOMP_Y);
    }

    if (cmbScalarFieldVariableComp->currentIndex() == -1)
        cmbScalarFieldVariableComp->setCurrentIndex(0);
}

void SceneViewDialog::doScalarFieldRangeAuto(int state)
{
    txtScalarFieldRangeMin->setEnabled(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setEnabled(!chkScalarFieldRangeAuto->isChecked());       
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
