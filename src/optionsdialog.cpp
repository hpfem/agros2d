#include "optionsdialog.h"

OptionsDialog::OptionsDialog(SceneViewSettings *sceneViewSettings, QWidget *parent) : QDialog(parent)
{
    m_sceneViewSettings = sceneViewSettings;

    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));
    
    createControls();

    load();    

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

OptionsDialog::~OptionsDialog()
{
    // main
    delete cmbGUIStyle;
    delete cmbLanguage;

    // grid
    delete txtGridStep;

    // rulers
    delete chkRulers;

    // contours
    delete txtContoursCount;

    // scalar field
    delete cmbPalette;
    delete chkPaletteFilter;
    delete txtPaletteSteps;

    // 3d
    delete chkView3DLighting;

    // delete files
    delete chkDeleteTriangleMeshFiles;
    delete chkDeleteHermes2DMeshFile;

    // clear command history
    delete cmdClearCommandHistory;

    // show result in line edit value widget
    delete chkLineEditValueShowResult;

    // save with solution
    delete chkSaveWithSolution;

    delete lstView;
    delete panMain;
    delete panView;
    delete panColors;
    delete pages;
}

void OptionsDialog::load()
{
    QSettings settings;

    // gui style
    for (int i = 0; i < cmbGUIStyle->count(); i++) {
        if (cmbGUIStyle->itemText(i) == settings.value("General/GUIStyle"))
        {
            cmbGUIStyle->setCurrentIndex(i);
            break;
        }
    }

    // language
    for (int i = 0; i < cmbLanguage->count(); i++) {
        if (cmbLanguage->itemText(i) == settings.value("General/Language")) {
            cmbLanguage->setCurrentIndex(i);
            break;
        }
    }

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(settings.value("General/LineEditValueShowResult", false).value<bool>());

    // geometry
    txtGeometryAngleSegmentsCount->setValue(settings.value("Geometry/AngleSegmentsCount", 5).value<int>());
    txtGeometryNodeSize->setValue(m_sceneViewSettings->geometryNodeSize);
    txtGeometryEdgeWidth->setValue(m_sceneViewSettings->geometryEdgeWidth);
    txtGeometryLabelSize->setValue(m_sceneViewSettings->geometryLabelSize);

    // delete files
    chkDeleteTriangleMeshFiles->setChecked(settings.value("Solver/DeleteTriangleMeshFiles", true).value<bool>());
    chkDeleteHermes2DMeshFile->setChecked(settings.value("Solver/DeleteHermes2DMeshFile", true).value<bool>());

    // save problem with solution
    chkSaveWithSolution->setChecked(settings.value("Solver/SaveProblemWithSolution", false).value<bool>());

    // colors
    colorBackground->setColor(m_sceneViewSettings->colorBackground);
    colorGrid->setColor(m_sceneViewSettings->colorGrid);
    colorCross->setColor(m_sceneViewSettings->colorCross);
    colorNodes->setColor(m_sceneViewSettings->colorNodes);
    colorEdges->setColor(m_sceneViewSettings->colorEdges);
    colorLabels->setColor(m_sceneViewSettings->colorLabels);
    colorContours->setColor(m_sceneViewSettings->colorContours);
    colorVectors->setColor(m_sceneViewSettings->colorVectors);
    colorInitialMesh->setColor(m_sceneViewSettings->colorInitialMesh);
    colorSolutionMesh->setColor(m_sceneViewSettings->colorSolutionMesh);
    colorHighlighted->setColor(m_sceneViewSettings->colorHighlighted);
    colorSelected->setColor(m_sceneViewSettings->colorSelected);

    // grid
    txtGridStep->setText(QString::number(m_sceneViewSettings->gridStep));
    chkRulers->setChecked(m_sceneViewSettings->showRulers);

    // contours
    txtContoursCount->setValue(m_sceneViewSettings->contoursCount);

    // scalar field
    cmbPalette->setCurrentIndex(cmbPalette->findData(m_sceneViewSettings->paletteType));
    chkPaletteFilter->setChecked(m_sceneViewSettings->paletteFilter);
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(m_sceneViewSettings->paletteSteps);

    // 3d
    chkView3DLighting->setChecked(m_sceneViewSettings->scalarView3DLighting);
}

void OptionsDialog::save()
{
    QSettings settings;
    // gui style
    settings.setValue("General/GUIStyle", cmbGUIStyle->currentText());
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    if (settings.value("General/Language", QLocale::system().name()) != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                                 tr("Language change"),
                                 tr("Interface language has been changed. You must restart the application."));
    settings.setValue("General/Language", cmbLanguage->currentText());

    // show result in line edit value widget
    settings.setValue("General/LineEditValueShowResult", chkLineEditValueShowResult->isChecked());

    // geometry
    settings.setValue("Geometry/AngleSegmentsCount", txtGeometryAngleSegmentsCount->value());
    m_sceneViewSettings->geometryNodeSize = txtGeometryNodeSize->value();
    m_sceneViewSettings->geometryEdgeWidth = txtGeometryEdgeWidth->value();
    m_sceneViewSettings->geometryLabelSize = txtGeometryLabelSize->value();

    // delete files
    settings.setValue("Solver/DeleteTriangleMeshFiles", chkDeleteTriangleMeshFiles->isChecked());
    settings.setValue("Solver/DeleteHermes2DMeshFile", chkDeleteHermes2DMeshFile->isChecked());

    // save problem with solution
    settings.setValue("Solver/SaveProblemWithSolution", chkSaveWithSolution->isChecked());

    // color
    m_sceneViewSettings->colorBackground = colorBackground->color();
    m_sceneViewSettings->colorGrid = colorGrid->color();
    m_sceneViewSettings->colorCross = colorCross->color();
    m_sceneViewSettings->colorNodes = colorNodes->color();
    m_sceneViewSettings->colorEdges = colorEdges->color();
    m_sceneViewSettings->colorLabels = colorLabels->color();
    m_sceneViewSettings->colorContours = colorContours->color();
    m_sceneViewSettings->colorVectors = colorVectors->color();
    m_sceneViewSettings->colorInitialMesh = colorInitialMesh->color();
    m_sceneViewSettings->colorSolutionMesh = colorSolutionMesh->color();
    m_sceneViewSettings->colorHighlighted = colorHighlighted->color();
    m_sceneViewSettings->colorSelected = colorSelected->color();

    // grid
    m_sceneViewSettings->gridStep = txtGridStep->text().toDouble();
    m_sceneViewSettings->showRulers = chkRulers->isChecked();

    // contours
    m_sceneViewSettings->contoursCount = txtContoursCount->value();

    // scalar field
    m_sceneViewSettings->paletteType = (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt();
    m_sceneViewSettings->paletteFilter = chkPaletteFilter->isChecked();
    m_sceneViewSettings->paletteSteps = txtPaletteSteps->value();

    // 3d
    m_sceneViewSettings->scalarView3DLighting = chkView3DLighting->isChecked();

    // save
    m_sceneViewSettings->save();
}

void OptionsDialog::createControls()
{    
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panView = createViewWidget();
    panColors = createColorsWidget();

    // List View
    lstView->setCurrentRow(0);
    lstView->setViewMode(QListView::IconMode);
    lstView->setMovement(QListView::Static);
    lstView->setIconSize(QSize(64, 64));
    lstView->setMinimumWidth(100);
    lstView->setMaximumWidth(100);
    lstView->setSpacing(12);
    connect(lstView, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
               this, SLOT(doCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));

    // listView items
    QListWidgetItem *itemMain = new QListWidgetItem(icon("options-main"), tr("Main"), lstView);
    itemMain->setTextAlignment(Qt::AlignHCenter);
    itemMain->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemView = new QListWidgetItem(icon("options-view"), tr("View"), lstView);
    itemView->setTextAlignment(Qt::AlignHCenter);
    itemView->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *itemColors = new QListWidgetItem(icon("options-colors"), tr("Colors"), lstView);
    itemColors->setTextAlignment(Qt::AlignHCenter);
    itemColors->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    pages->addWidget(panMain);
    pages->addWidget(panView);
    pages->addWidget(panColors);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(lstView);
    layoutHorizontal->addWidget(pages);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    // layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *OptionsDialog::createMainWidget()
{
    QWidget *mainWidget = new QWidget(this);

    // general
    cmbGUIStyle = new QComboBox(mainWidget);
    cmbGUIStyle->addItems(QStyleFactory::keys());

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("UI:")), 0, 0);
    layoutGeneral->addWidget(cmbGUIStyle, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Language:")), 1, 0);
    layoutGeneral->addWidget(cmbLanguage, 1, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // solver layout
    chkDeleteTriangleMeshFiles = new QCheckBox(tr("Delete files with initial mesh (Triangle)"));
    chkDeleteHermes2DMeshFile = new QCheckBox(tr("Delete files with solution mesh (Hermes2D)"));
    chkSaveWithSolution = new QCheckBox("Save problem with solution");

    QVBoxLayout *layoutSolver = new QVBoxLayout();
    layoutSolver->addWidget(chkDeleteTriangleMeshFiles);
    layoutSolver->addWidget(chkDeleteHermes2DMeshFile);
    layoutSolver->addWidget(chkSaveWithSolution);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    // other layout
    cmdClearCommandHistory = new QPushButton(mainWidget);
    cmdClearCommandHistory->setText(tr("Clear command history"));
    connect(cmdClearCommandHistory, SIGNAL(clicked()), this, SLOT(doClearCommandHistory()));

    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));

    QHBoxLayout *layoutClearCommandHistory = new QHBoxLayout();
    layoutClearCommandHistory->addWidget(cmdClearCommandHistory);    
    layoutClearCommandHistory->addStretch();

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addLayout(layoutClearCommandHistory);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addWidget(grpSolver);
    layout->addWidget(grpOther);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *OptionsDialog::createViewWidget()
{
    QWidget *viewWidget = new QWidget(this);

    // geometry
    txtGeometryAngleSegmentsCount = new QSpinBox(this);
    txtGeometryAngleSegmentsCount->setMinimum(1);
    txtGeometryAngleSegmentsCount->setMaximum(100);
    txtGeometryNodeSize = new SLineEditDouble();
    txtGeometryEdgeWidth = new SLineEditDouble();
    txtGeometryLabelSize = new SLineEditDouble();

    QGridLayout *layoutGeometry = new QGridLayout();
    layoutGeometry->addWidget(new QLabel(tr("Angle segments count:")), 0, 0);
    layoutGeometry->addWidget(txtGeometryAngleSegmentsCount, 0, 1);
    layoutGeometry->addWidget(new QLabel(tr("Node size:")), 1, 0);
    layoutGeometry->addWidget(txtGeometryNodeSize, 1, 1);
    layoutGeometry->addWidget(new QLabel(tr("Edge width:")), 2, 0);
    layoutGeometry->addWidget(txtGeometryEdgeWidth, 2, 1);
    layoutGeometry->addWidget(new QLabel(tr("Label size:")), 3, 0);
    layoutGeometry->addWidget(txtGeometryLabelSize, 3, 1);

    QGroupBox *grpGeometry = new QGroupBox(tr("Geometry"));
    grpGeometry->setLayout(layoutGeometry);

    // layout grid
    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkRulers = new QCheckBox(tr("Show rulers"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(new QLabel(tr("Grid step:")), 0, 0);
    layoutGrid->addWidget(txtGridStep, 0, 1);
    layoutGrid->addWidget(chkRulers, 1, 0);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    // layout palette
    cmbPalette = new QComboBox();
    cmbPalette->addItem(tr("Jet"), PALETTE_JET);
    cmbPalette->addItem(tr("Autumn"), PALETTE_AUTUMN);
    cmbPalette->addItem(tr("Hot"), PALETTE_HOT);
    cmbPalette->addItem(tr("Copper"), PALETTE_COPPER);
    cmbPalette->addItem(tr("Cool"), PALETTE_COOL);
    cmbPalette->addItem(tr("B/W ascending"), PALETTE_BW_ASC);
    cmbPalette->addItem(tr("B/W descending"), PALETTE_BW_DESC);

    chkPaletteFilter = new QCheckBox();
    connect(chkPaletteFilter, SIGNAL(stateChanged(int)), this, SLOT(doPaletteFilter(int)));

    txtPaletteSteps = new QSpinBox(this);
    txtPaletteSteps->setMinimum(5);
    txtPaletteSteps->setMaximum(100);

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->addWidget(new QLabel(tr("Palette:")), 0, 0);
    layoutScalarField->addWidget(cmbPalette, 0, 1, 1, 3);

    layoutScalarField->addWidget(new QLabel(tr("Filter:")), 1, 0);
    layoutScalarField->addWidget(chkPaletteFilter, 1, 1);
    layoutScalarField->addWidget(new QLabel(tr("Steps:")), 1, 2);
    layoutScalarField->addWidget(txtPaletteSteps, 1, 3);

    QGroupBox *grpScalarView = new QGroupBox(tr("Scalar view"));
    grpScalarView->setLayout(layoutScalarField);

    // layout contours
    txtContoursCount = new QSpinBox(this);
    txtContoursCount->setMinimum(1);
    txtContoursCount->setMaximum(100);

    QGridLayout *layoutContours = new QGridLayout();
    layoutContours->addWidget(new QLabel(tr("Contours count:")), 0, 0);
    layoutContours->addWidget(txtContoursCount, 0, 1);

    QGroupBox *grpContours = new QGroupBox(tr("Contours"));
    grpContours->setLayout(layoutContours);

    // layout 3d
    chkView3DLighting = new QCheckBox(tr("Ligthing"), this);

    QHBoxLayout *layout3D = new QHBoxLayout();
    layout3D->addWidget(chkView3DLighting);

    QGroupBox *grp3D = new QGroupBox(tr("3D"));
    grp3D->setLayout(layout3D);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeometry);
    layout->addWidget(grpGrid);
    layout->addWidget(grpContours);
    layout->addWidget(grpScalarView);
    layout->addWidget(grp3D);
    layout->addStretch();

    viewWidget->setLayout(layout);

    return viewWidget;
}

QWidget *OptionsDialog::createColorsWidget()
{
    QWidget *colorsWidget = new QWidget(this);

    // colors
    colorBackground = new ColorButton(this);
    colorGrid = new ColorButton(this);
    colorCross = new ColorButton(this);

    colorNodes = new ColorButton(this);
    colorEdges = new ColorButton(this);
    colorLabels = new ColorButton(this);
    colorContours = new ColorButton(this);
    colorVectors = new ColorButton(this);
    colorInitialMesh = new ColorButton(this);
    colorSolutionMesh = new ColorButton(this);

    colorHighlighted = new ColorButton(this);
    colorSelected = new ColorButton(this);

    QGridLayout *layoutColors = new QGridLayout();
    layoutColors->addWidget(new QLabel(tr("Background:")), 0, 0);
    layoutColors->addWidget(new QLabel(tr("Grid:")), 1, 0);
    layoutColors->addWidget(new QLabel(tr("Cross:")), 2, 0);
    layoutColors->addWidget(new QLabel(tr("Nodes:")), 3, 0);
    layoutColors->addWidget(new QLabel(tr("Edges:")), 4, 0);
    layoutColors->addWidget(new QLabel(tr("Labels:")), 5, 0);
    layoutColors->addWidget(new QLabel(tr("Contours:")), 6, 0);
    layoutColors->addWidget(new QLabel(tr("Vectors:")), 7, 0);
    layoutColors->addWidget(new QLabel(tr("Initial mesh:")), 8, 0);
    layoutColors->addWidget(new QLabel(tr("Solution mesh:")), 9, 0);
    layoutColors->addWidget(new QLabel(tr("Highlighted elements:")), 10, 0);
    layoutColors->addWidget(new QLabel(tr("Selected elements:")), 11, 0);

    layoutColors->addWidget(colorBackground, 0, 1);
    layoutColors->addWidget(colorGrid, 1, 1);
    layoutColors->addWidget(colorCross, 2, 1);
    layoutColors->addWidget(colorNodes, 3, 1);
    layoutColors->addWidget(colorEdges, 4, 1);
    layoutColors->addWidget(colorLabels, 5, 1);
    layoutColors->addWidget(colorContours, 6, 1);
    layoutColors->addWidget(colorVectors, 7, 1);
    layoutColors->addWidget(colorInitialMesh, 8, 1);
    layoutColors->addWidget(colorSolutionMesh, 9, 1);
    layoutColors->addWidget(colorHighlighted, 10, 1);
    layoutColors->addWidget(colorSelected, 11, 1);

    QGroupBox *grpColor = new QGroupBox(tr("Colors"));
    grpColor->setLayout(layoutColors);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpColor);
    layout->addStretch();

    colorsWidget->setLayout(layout);

    return colorsWidget;
}

void OptionsDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    pages->setCurrentIndex(lstView->row(current));
}

void OptionsDialog::doAccept()
{
    save();

    accept();
}

void OptionsDialog::doReject()
{
    reject();
}

void OptionsDialog::doPaletteFilter(int state)
{
    txtPaletteSteps->setEnabled(!chkPaletteFilter->isChecked());
}

void OptionsDialog::doClearCommandHistory()
{
    QSettings settings;
    settings.setValue("CommandDialog/RecentCommands", QStringList());

    QMessageBox::information(QApplication::activeWindow(), tr("Information"), tr("Command history was cleared succesfully."));
}

// *******************************************************************************************************

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
    setAutoFillBackground(false);
    setCursor(Qt::PointingHandCursor);    
    connect(this, SIGNAL(clicked()), this, SLOT(doClicked()));
}

ColorButton::~ColorButton()
{
}

void ColorButton::setColor(const QColor &color)
{
    m_color = color;
    repaint();
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setPen(m_color);
    painter.setBrush(m_color);
    painter.drawRect(rect());
}

void ColorButton::doClicked()
{
    QColor color = QColorDialog::getColor(m_color);

    if (color.isValid())
    {
        setColor(color);
    }
}

