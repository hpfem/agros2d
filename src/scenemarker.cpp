#include "scenemarker.h"
#include "scene.h"

SceneEdgeMarker::SceneEdgeMarker(const QString &name, PhysicFieldBC type)
{
    this->name = name;
    this->type = type;
}

QString SceneEdgeMarker::html()
{
    QString out;
    out += "<h4>" + physicFieldString(Util::scene()->problemInfo()->physicField()) + "</h4>";
    out += "<table>";
    QMap<QString, QString> data = this->data();
    for (int j = 0; j < data.keys().length(); j++)
    {
        out += "<tr>";
        out += "<td>" + data.keys()[j] + ":</td>";
        out += "<td>" + data.values()[j] + "</td>";
        out += "</tr>";
    }
    out += "</table>";

    return out;
}

QVariant SceneEdgeMarker::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

SceneEdgeMarkerNone::SceneEdgeMarkerNone() : SceneEdgeMarker("none", PHYSICFIELDBC_NONE)
{
}

// *************************************************************************************************************************************

SceneLabelMarker::SceneLabelMarker(const QString &name)
{
    this->name = name;
}

QString SceneLabelMarker::html()
{
    QString out;
    out += "<h4>" + physicFieldString(Util::scene()->problemInfo()->physicField()) + "</h4>";
    out += "<table>";
    QMap<QString, QString> data = this->data();
    for (int j = 0; j < data.keys().length(); j++)
    {
        out += "<tr>";
        out += "<td>" + data.keys()[j] + ":</td>";
        out += "<td>" + data.values()[j] + "</td>";
        out += "</tr>";
    }
    out += "</table>";

    return out;
}

QVariant SceneLabelMarker::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

SceneLabelMarkerNone::SceneLabelMarkerNone() : SceneLabelMarker("none")
{
}


// *************************************************************************************************************************************

DSceneEdgeMarker::DSceneEdgeMarker(QWidget *parent) : QDialog(parent)
{
    layout = new QVBoxLayout();
    txtName = new QLineEdit("");
}

DSceneEdgeMarker::~DSceneEdgeMarker()
{
    delete layout;
    delete txtName;
}

void DSceneEdgeMarker::createDialog()
{
    QHBoxLayout *layoutName = new QHBoxLayout();
    layoutName->addWidget(new QLabel(tr("Name:")));
    layoutName->addWidget(txtName);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addLayout(layoutName);
    layout->addLayout(createContent());
    layout->addStretch();
    layout->addWidget(buttonBox);

    txtName->setFocus();

    setLayout(layout);
}

void DSceneEdgeMarker::load()
{
    txtName->setText(m_edgeMarker->name);
}

bool DSceneEdgeMarker::save()
{
    // find name duplicities
    foreach (SceneEdgeMarker *edgeMarker, Util::scene()->edgeMarkers)
    {
        if (edgeMarker->name == txtName->text())
        {
            if (m_edgeMarker == edgeMarker)
                continue;

            QMessageBox::warning(this, tr("Boundary marker"), tr("Boundary marker name already exists."));
            return false;
        }
    }
    m_edgeMarker->name = txtName->text();
    return true;
}

void DSceneEdgeMarker::setSize()
{
    setWindowIcon(icon("scene-edgemarker"));
    setWindowTitle(tr("Boundary condition"));

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void DSceneEdgeMarker::doAccept()
{
    if (save())
        accept();    
}

void DSceneEdgeMarker::doReject()
{
    reject();
}

// *************************************************************************************************************************************

DSceneLabelMarker::DSceneLabelMarker(QWidget *parent) : QDialog(parent)
{
    layout = new QVBoxLayout();
    txtName = new QLineEdit("");
}

DSceneLabelMarker::~DSceneLabelMarker()
{
    delete layout;
    delete txtName;
}

void DSceneLabelMarker::createDialog()
{
    QHBoxLayout *layoutName = new QHBoxLayout();
    layoutName->addWidget(new QLabel(tr("Name:")));
    layoutName->addWidget(txtName);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addLayout(layoutName);
    layout->addLayout(createContent());
    layout->addStretch();
    layout->addWidget(buttonBox);

    txtName->setFocus();

    setLayout(layout);
}

void DSceneLabelMarker::setSize()
{
    setWindowIcon(icon("scene-labelmarker"));
    setWindowTitle(tr("Material"));

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void DSceneLabelMarker::load()
{
    txtName->setText(m_labelMarker->name);
}

bool DSceneLabelMarker::save()
{
    // find name duplicities
    foreach (SceneLabelMarker *labelMarker, Util::scene()->labelMarkers)
    {
        if (labelMarker->name == txtName->text())
        {
            if (m_labelMarker == labelMarker)
                continue;

            QMessageBox::warning(this, tr("Material marker"), tr("Material marker name already exists."));
            return false;
        }
    }
    m_labelMarker->name = txtName->text();
    return true;
}

void DSceneLabelMarker::doAccept()
{
    if (save())
        accept();
}

void DSceneLabelMarker::doReject()
{
    reject();
}

// ***********************************************************************************************************

EdgeMarkerDialog::EdgeMarkerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Edge marker"));
    setWindowIcon(icon("scene-edge"));
    setModal(true);

    // fill combo
    cmbMarker = new QComboBox(this);
    for (int i = 0; i<Util::scene()->edgeMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->edgeMarkers[i]->name, Util::scene()->edgeMarkers[i]->variant());
    }

    // select marker
    cmbMarker->setCurrentIndex(-1);
    SceneEdgeMarker *marker = NULL;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->edges[i]->marker;
            }
            if (marker != Util::scene()->edges[i]->marker)
            {
                marker = NULL;
                break;
            }
        }
    }
    if (marker)
        cmbMarker->setCurrentIndex(cmbMarker->findData(marker->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(new QLabel(tr("Edge marker:")));
    layoutMarker->addWidget(cmbMarker);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMarker);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

EdgeMarkerDialog::~EdgeMarkerDialog()
{
    delete cmbMarker;
}

void EdgeMarkerDialog::doAccept()
{
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->isSelected)
            Util::scene()->edges[i]->marker = marker();
    }

    accept();
}

// *************************************************************************************************************************************

LabelMarkerDialog::LabelMarkerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Edge marker"));
    setWindowIcon(icon("scene-label"));
    setModal(true);

    // fill combo
    cmbMarker = new QComboBox(this);
    for (int i = 0; i<Util::scene()->labelMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->labelMarkers[i]->name, Util::scene()->labelMarkers[i]->variant());
    }

    // select marker
    cmbMarker->setCurrentIndex(-1);
    SceneLabelMarker *marker = NULL;
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->labels[i]->marker;
            }
            if (marker != Util::scene()->labels[i]->marker)
            {
                marker = NULL;
                break;
            }
        }
    }
    if (marker)
        cmbMarker->setCurrentIndex(cmbMarker->findData(marker->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(new QLabel(tr("Label marker:")));
    layoutMarker->addWidget(cmbMarker);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMarker);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

LabelMarkerDialog::~LabelMarkerDialog()
{
    delete cmbMarker;
}

void LabelMarkerDialog::doAccept()
{
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
            Util::scene()->labels[i]->marker = marker();
    }

    accept();
}
