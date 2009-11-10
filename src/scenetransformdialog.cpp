#include "scenetransformdialog.h"

SceneTransformDialog::SceneTransformDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("scene-transform"));
    setWindowTitle(tr("Transform"));

    createControls();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

SceneTransformDialog::~SceneTransformDialog()
{    
    delete txtTranslateX;
    delete txtTranslateY;

    delete txtRotateBasePointX;
    delete txtRotateBasePointY;
    delete txtRotateAngle;

    delete txtScaleBasePointX;
    delete txtScaleBasePointY;
    delete txtScaleFactor;

    delete widTranslate;
    delete widRotate;
    delete widScale;

    delete tabWidget;
}

void SceneTransformDialog::createControls()
{
    // translate
    txtTranslateX = new SLineEditDouble();
    txtTranslateY = new SLineEditDouble();

    QGridLayout *layoutTranslate = new QGridLayout();
    layoutTranslate->addWidget(new QLabel(Util::scene()->problemInfo()->labelX() + ":"), 0, 0);
    layoutTranslate->addWidget(txtTranslateX, 0, 1);
    layoutTranslate->addWidget(new QLabel(Util::scene()->problemInfo()->labelY() + ":"), 1, 0);
    layoutTranslate->addWidget(txtTranslateY, 1, 1);
    layoutTranslate->addWidget(new QLabel(""), 2, 0);

    widTranslate = new QWidget();
    widTranslate->setLayout(layoutTranslate);

    // rotate
    txtRotateBasePointX = new SLineEditDouble();
    txtRotateBasePointY = new SLineEditDouble();
    txtRotateAngle = new SLineEditDouble();

    QGridLayout *layoutRotate = new QGridLayout();
    layoutRotate->addWidget(new QLabel(Util::scene()->problemInfo()->labelX() + ":"), 0, 0);
    layoutRotate->addWidget(txtRotateBasePointX, 0, 1);
    layoutRotate->addWidget(new QLabel(Util::scene()->problemInfo()->labelY() + ":"), 1, 0);
    layoutRotate->addWidget(txtRotateBasePointY, 1, 1);
    layoutRotate->addWidget(new QLabel(tr("Angle:")), 2, 0);
    layoutRotate->addWidget(txtRotateAngle, 2, 1);

    widRotate = new QWidget();
    widRotate->setLayout(layoutRotate);

    // scale
    txtScaleBasePointX = new SLineEditDouble();
    txtScaleBasePointY = new SLineEditDouble();
    txtScaleFactor = new SLineEditDouble();

    QGridLayout *layoutScale = new QGridLayout();
    layoutScale->addWidget(new QLabel(Util::scene()->problemInfo()->labelX() + ":"), 0, 0);
    layoutScale->addWidget(txtScaleBasePointX, 0, 1);
    layoutScale->addWidget(new QLabel(Util::scene()->problemInfo()->labelY() + ":"), 1, 0);
    layoutScale->addWidget(txtScaleBasePointY, 1, 1);
    layoutScale->addWidget(new QLabel(tr("Scaling Factor:")), 2, 0);
    layoutScale->addWidget(txtScaleFactor, 2, 1);

    widScale = new QWidget();
    widScale->setLayout(layoutScale);

    // copy
    chkCopy = new QCheckBox(tr("Copy objects"));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    // tab widget
    tabWidget = new QTabWidget(this);
    tabWidget->addTab(widTranslate, icon(""), tr("Translate"));
    tabWidget->addTab(widRotate, icon(""), tr("Rotate"));
    tabWidget->addTab(widScale, icon(""), tr("Scale"));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->addWidget(chkCopy);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SceneTransformDialog::doAccept()
{
    if (tabWidget->currentWidget() == widTranslate)
        Util::scene()->transformTranslate(Point(txtTranslateX->value(), txtTranslateY->value()), chkCopy->isChecked());

    if (tabWidget->currentWidget() == widRotate)
        Util::scene()->transformRotate(Point(txtRotateBasePointX->value(), txtRotateBasePointY->value()), txtRotateAngle->value(), chkCopy->isChecked());

    if (tabWidget->currentWidget() == widScale)
        Util::scene()->transformScale(Point(txtScaleBasePointX->value(), txtScaleBasePointY->value()), txtScaleFactor->value(), chkCopy->isChecked());

    accept();
}

void SceneTransformDialog::doReject()
{
    reject();
}
