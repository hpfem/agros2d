// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "imageloader.h"

#include "scene.h"
#include "util.h"

#include "gui/lineeditdouble.h"

ImageLoaderDialog::ImageLoaderDialog(QWidget *parent) : QDialog(parent)
{
    // setWindowIcon(icon("browser"));
    setWindowTitle(tr("Image"));
    setWindowFlags(Qt::Window);

    createControls();

    QSettings settings;
    restoreGeometry(settings.value("ImageLoaderDialog/Geometry", saveGeometry()).toByteArray());

    txtX->setValue(settings.value("ImageLoaderDialog/PositionX", 0.0).toDouble());
    txtY->setValue(settings.value("ImageLoaderDialog/PositionY", 0.0).toDouble());
    txtWidth->setValue(settings.value("ImageLoaderDialog/PositionWith", 1.0).toDouble());
    txtHeight->setValue(settings.value("ImageLoaderDialog/PositionHeight", 1.0).toDouble());
    doLoadFile(settings.value("ImageLoaderDialog/FileName").toString());
}

ImageLoaderDialog::~ImageLoaderDialog()
{
    QSettings settings;
    settings.setValue("ImageLoaderDialog/Geometry", saveGeometry());
}

void ImageLoaderDialog::createControls()
{
    lblImage = new QLabel();
    lblImage->setBackgroundRole(QPalette::Base);
    lblImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    lblImage->setScaledContents(true);
    lblImage->setMinimumSize(400, 300);

    lblImageFileName = new QLabel();

    QPushButton *btnLoadImage = new QPushButton(icon("three-dots"), "");
    btnLoadImage->setMaximumSize(btnLoadImage->sizeHint());
    connect(btnLoadImage, SIGNAL(clicked()), this, SLOT(doLoadFile()));

    QPushButton *btnRemoveImage = new QPushButton(icon("remove-item"), "");
    connect(btnRemoveImage, SIGNAL(clicked()), this, SLOT(doRemoveFile()));

    QHBoxLayout *layoutFileName = new QHBoxLayout();
    layoutFileName->addWidget(new QLabel(tr("Filename:")));
    layoutFileName->addWidget(lblImageFileName, 1);
    layoutFileName->addWidget(btnRemoveImage);
    layoutFileName->addWidget(btnLoadImage);

    txtX = new LineEditDouble(0.0);
    txtY = new LineEditDouble(0.0);
    txtWidth = new LineEditDouble(1.0);
    txtHeight = new LineEditDouble(1.0);

    QGridLayout *layoutPosition = new QGridLayout();
    layoutPosition->addWidget(new QLabel(tr("X:")), 0, 0);
    layoutPosition->addWidget(txtX, 0, 1);
    layoutPosition->addWidget(new QLabel(tr("Y:")), 1, 0);
    layoutPosition->addWidget(txtY, 1, 1);
    layoutPosition->addWidget(new QLabel(tr("Width:")), 0, 2);
    layoutPosition->addWidget(txtWidth, 0, 3);
    layoutPosition->addWidget(new QLabel(tr("Height:")), 1, 2);
    layoutPosition->addWidget(txtHeight, 1, 3);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutFileName);
    layout->addLayout(layoutPosition);
    layout->addWidget(lblImage, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ImageLoaderDialog::doRemoveFile()
{
    doLoadFile("");
}

void ImageLoaderDialog::doLoadFile()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), m_fileName, tr("Images (*.png *.bmp *.jpg)"));
    if (!fileName.isEmpty())
    {
        doLoadFile(fileName);
        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void ImageLoaderDialog::doLoadFile(const QString &fileName)
{
    m_fileName = fileName;
    lblImageFileName->setText(m_fileName);

    if (QFile::exists(m_fileName))
    {
        QPixmap pixmap(m_fileName);
        lblImage->setPixmap(pixmap);
        lblImage->resize(lblImage->pixmap()->size());
    }
    else
    {
        lblImage->setPixmap(QPixmap());
    }
}

void ImageLoaderDialog::doAccept()
{
    m_position.setX(txtX->value());
    m_position.setY(txtY->value());
    m_position.setWidth(txtWidth->value());
    m_position.setHeight(txtHeight->value());

    QSettings settings;
    settings.setValue("ImageLoaderDialog/PositionX", txtX->value());
    settings.setValue("ImageLoaderDialog/PositionY", txtY->value());
    settings.setValue("ImageLoaderDialog/PositionWith", txtWidth->value());
    settings.setValue("ImageLoaderDialog/PositionHeight", txtHeight->value());
    settings.setValue("ImageLoaderDialog/FileName", m_fileName);

    accept();
}

void ImageLoaderDialog::doReject()
{
    reject();
}

