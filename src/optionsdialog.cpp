#include "optionsdialog.h"

OptionsDialog::OptionsDialog(SceneViewSettings *sceneViewSettings, QWidget *parent) : QDialog(parent)
{
    m_sceneViewSettings = sceneViewSettings;

    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));
    
    createControls();

    load();    

    setMinimumSize(350, 450);
    setMaximumSize(350, 450);
}

OptionsDialog::~OptionsDialog()
{
    // main
    delete cmbGUIStyle;
    delete cmbLanguage;

    delete lstView;
    delete panMain;
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

    // colors
    colorContours->setColor(m_sceneViewSettings->colorContours);
}

void OptionsDialog::save()
{
    QSettings settings;
    // gui style
    settings.setValue("General/GUIStyle", cmbGUIStyle->currentText());
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    settings.setValue("General/Language", cmbLanguage->currentText());
    setLanguage(cmbLanguage->currentText());

    // color
    m_sceneViewSettings->colorContours = colorContours->color();

    m_sceneViewSettings->save();
}

void OptionsDialog::createControls()
{    
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
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

    QListWidgetItem *itemColors = new QListWidgetItem(icon("options-colors"), tr("Colors"), lstView);
    itemColors->setTextAlignment(Qt::AlignHCenter);
    itemColors->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    pages->addWidget(panMain);
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

    cmbGUIStyle = new QComboBox(mainWidget);
    cmbGUIStyle->addItems(QStyleFactory::keys());

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    QGridLayout *layoutMain = new QGridLayout();
    layoutMain->addWidget(new QLabel(tr("UI:")), 0, 0);
    layoutMain->addWidget(cmbGUIStyle, 0, 1);
    layoutMain->addWidget(new QLabel(tr("Language:")), 1, 0);
    layoutMain->addWidget(cmbLanguage, 1, 1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMain);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *OptionsDialog::createColorsWidget()
{
    QWidget *colorsWidget = new QWidget(this);

    // colors
    colorContours = new ColorButton(this);

    QGridLayout *layoutColors = new QGridLayout();
    layoutColors->addWidget(new QLabel(tr("Contours:")), 0, 0);
    layoutColors->addWidget(colorContours, 0, 1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutColors);
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

// *******************************************************************************************************

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
    setAutoFillBackground(false);
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
    QColor color = QColorDialog::getColor();

    if (color.isValid())
    {
        setColor(color);
    }
}
