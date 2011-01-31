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

#include "logdialog.h"

LogDialog::LogDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("LogDialog::LogDialog()");

    setWindowIcon(icon("log"));
    setWindowTitle(tr("Log dialog"));
    setWindowFlags(Qt::Window);

    createControls();

    QSettings settings;
    setMinimumSize(sizeHint());
    restoreGeometry(settings.value("LogDialog/Geometry", saveGeometry()).toByteArray());
}

LogDialog::~LogDialog()
{
    logMessage("LogDialog::~LogDialog()");

    QSettings settings;
    settings.setValue("LogDialog/Geometry", saveGeometry());

    delete btnSaveLog;
    delete btnClose;
}

void LogDialog::createControls()
{
    logMessage("LogDialog::createControls()");

    lstMessages = new QTextEdit(this);
    lstMessages->setReadOnly(true);

    btnSaveLog = new QPushButton(tr("Save log"));
    connect(btnSaveLog, SIGNAL(clicked()), this, SLOT(doSaveLog()));

    btnDeleteLog = new QPushButton(tr("Delete"));
    connect(btnDeleteLog, SIGNAL(clicked()), this, SLOT(doDeleteLog()));

    btnClose = new QPushButton(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    btnShowAdaptivityErrorChart = new QPushButton(tr("Adapt. error"));
    connect(btnShowAdaptivityErrorChart, SIGNAL(clicked()), this, SLOT(doShowAdaptivityErrorChart()));

    btnShowAdaptivityDOFChart = new QPushButton(tr("Adapt. DOFs"));
    connect(btnShowAdaptivityDOFChart, SIGNAL(clicked()), this, SLOT(doShowAdaptivityDOFChart()));

    btnShowAdaptivityErrorDOFChart = new QPushButton(tr("Adapt. conv."));
    connect(btnShowAdaptivityErrorDOFChart, SIGNAL(clicked()), this, SLOT(doShowAdaptivityErrorDOFChart()));

    btnShowNonlinearityChart = new QPushButton(tr("Adapt. conv."));
    connect(btnShowNonlinearityChart, SIGNAL(clicked()), this, SLOT(doShowNonlinearityChart()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnShowAdaptivityErrorChart);
    layoutButtons->addWidget(btnShowAdaptivityDOFChart);
    layoutButtons->addWidget(btnShowAdaptivityErrorDOFChart);
    layoutButtons->addWidget(btnShowNonlinearityChart);
    layoutButtons->addWidget(btnSaveLog);
    layoutButtons->addWidget(btnDeleteLog);
    layoutButtons->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(lstMessages);
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

void LogDialog::showDialog()
{
    logMessage("LogDialog::showDialog()");

    show();
    activateWindow();
    raise();
}

void LogDialog::loadProgressLog()
{
    logMessage("LogDialog::loadProgressLog()");

    showDialog();

    lstMessages->clear();

    QFile file(tempProblemDir() + "/messages.log");
    if (file.exists())
    {
        lstMessages->setTextColor(QColor(Qt::black));

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            lstMessages->setPlainText(trUtf8(file.readAll()));
            lstMessages->moveCursor(QTextCursor::End);
        }

        btnSaveLog->setEnabled(true);

        btnShowAdaptivityErrorChart->setEnabled(QFile::exists(tempProblemDir() + "/adaptivity_error.png"));
        btnShowAdaptivityDOFChart->setEnabled(QFile::exists(tempProblemDir() + "/adaptivity_dof.png"));
        btnShowAdaptivityErrorDOFChart->setEnabled(QFile::exists(tempProblemDir() + "/adaptivity_conv.png"));
    }
    else
    {
        lstMessages->setTextColor(QColor(Qt::gray));
        lstMessages->insertPlainText(tr("No messages..."));
        btnSaveLog->setEnabled(false);

        btnShowAdaptivityErrorChart->setEnabled(false);
        btnShowAdaptivityDOFChart->setEnabled(false);
        btnShowAdaptivityErrorDOFChart->setEnabled(false);
    }

    btnDeleteLog->setEnabled(false);
}

void LogDialog::loadApplicationLog()
{
    logMessage("LogDialog::loadApplicationLog()");

    showDialog();

    lstMessages->clear();

    QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    QFile file(location + "/app.log");
    if (file.exists())
    {
        lstMessages->setTextColor(QColor(Qt::black));

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            lstMessages->setPlainText(trUtf8(file.readAll()));
            lstMessages->moveCursor(QTextCursor::End);
        }

        btnSaveLog->setEnabled(true);
    }
    else
    {
        lstMessages->setTextColor(QColor(Qt::gray));
        lstMessages->insertPlainText(tr("No messages..."));
        btnSaveLog->setEnabled(false);
    }

    btnShowAdaptivityErrorChart->setEnabled(false);
    btnShowAdaptivityDOFChart->setEnabled(false);
    btnShowAdaptivityErrorDOFChart->setEnabled(false);
    btnDeleteLog->setEnabled(true);
}

void LogDialog::doClose()
{
    logMessage("LogDialog::doClose()");

    hide();
}

void LogDialog::doSaveLog()
{
    logMessage("LogDialog::doSaveLog()");

    QSettings settings;
    QString dir = settings.value("General/LastLogDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save log"), dir, tr("Log files (*.log)"));

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().toLower() != "log")
        fileName += ".log";

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) || !fileName.isEmpty())
    {
        QTextStream messages(&file);
        messages << lstMessages->toPlainText();

        QFileInfo fileInfo(fileName);
        settings.setValue("General/LastLogDir", fileInfo.absolutePath());
    }
}

void LogDialog::doDeleteLog()
{
    logMessage("LogDialog::doDeleteLog()");

    if (QMessageBox::question(this, tr("Delete"), tr("Are you sure that you want to permanently delete the application logfile?"), tr("&Yes"), tr("&No")) == 0)
    {
        QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
        QFile::remove(location + "/app.log");

        lstMessages->clear();
    }
}

void LogDialog::showImage()
{
    logMessage("showPicture()");

    if (QFile::exists(imageFileName))
    {
        // load
        imageLabel = new QLabel();
        QImage image(imageFileName);
        imageLabel->setPixmap(QPixmap::fromImage(image));

        scrollArea = new QScrollArea();
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setWidget(imageLabel);

        btnSaveData = new QPushButton(tr("Save data"));
        connect(btnSaveData, SIGNAL(clicked()), this, SLOT(doSaveData()));

        btnSaveImage = new QPushButton(tr("Save image"));
        connect(btnSaveImage, SIGNAL(clicked()), this, SLOT(doSaveImage()));

        btnCloseImage = new QPushButton(tr("Close"));
        connect(btnCloseImage, SIGNAL(clicked()), this, SLOT(doCloseImage()));

        QHBoxLayout *layoutButtons = new QHBoxLayout();
        layoutButtons->addStretch();
        layoutButtons->addWidget(btnSaveData);
        layoutButtons->addWidget(btnSaveImage);
        layoutButtons->addWidget(btnCloseImage);

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(scrollArea);
        layout->addLayout(layoutButtons);


        imageDialog = new QDialog(QApplication::activeWindow());
        imageDialog->setWindowTitle("Convergence chart");
        imageDialog->setLayout(layout);
        imageDialog->setMinimumSize(imageDialog->sizeHint());
        imageDialog->setMaximumSize(imageDialog->sizeHint());
        imageDialog->exec();

        delete imageDialog;
    }
}

void LogDialog::doSaveData()
{
    logMessage("LogDialog::doSaveData()");

    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), dir, tr("CSV files (*.csv)"), &selectedFilter);

    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);

        if (fileInfo.suffix().toLower() != "csv")
            fileName += ".csv";
        if (QFile::exists(fileName))
            QFile::remove(fileName);

        if (!QFile::copy(dataFileName, fileName))
            QMessageBox::critical(QApplication::activeWindow(), tr("Error"), tr("File '%1' could not be copied..").arg(fileName));
        else
            settings.setValue("General/LastDataDir", fileInfo.absolutePath());
    }
}

void LogDialog::doSaveImage()
    {
        logMessage("LogDialog::doSaveImage()");

        QSettings settings;
        QString dir = settings.value("General/LastImageDir").toString();

        QString fileName = QFileDialog::getSaveFileName(this, tr("Save image to file"), dir, tr("PNG files (*.png)"));

        if (!fileName.isEmpty())
        {
            QFileInfo fileInfo(fileName);

            if (fileInfo.suffix().toLower() != "png")
                fileName += ".png";
            if (QFile::exists(fileName))
                QFile::remove(fileName);

            if (!QFile::copy(imageFileName, fileName))
                QMessageBox::critical(QApplication::activeWindow(), tr("Error"), tr("File '%1' could not be copied..").arg(fileName));
            else
                settings.setValue("General/LastImageDir", fileInfo.absolutePath());
        }
    }

void LogDialog::doCloseImage()
{
    logMessage("LogDialog::doCloseImage()");

    imageDialog->close();
}

void LogDialog::doShowAdaptivityErrorChart()
{
    logMessage("LogDialog::doShowAdaptivityErrorChart()");

    dataFileName = tempProblemDir() + "/adaptivity_error.csv";
    imageFileName = tempProblemDir() + "/adaptivity_error.png";
    showImage();
}

void LogDialog::doShowAdaptivityDOFChart()
{
    logMessage("LogDialog::doShowAdaptivityDOFChart()");

    dataFileName = tempProblemDir() + "/adaptivity_dof.csv";
    imageFileName = tempProblemDir() + "/adaptivity_dof.png";
    showImage();
}

void LogDialog::doShowAdaptivityErrorDOFChart()
{
    logMessage("LogDialog::doShowAdaptivityErrorDOFChart()");

    dataFileName = tempProblemDir() + "/adaptivity_conv.csv";
    imageFileName = tempProblemDir() + "/adaptivity_conv.png";
    showImage();
}

void LogDialog::doShowNonlinearityChart()
{
    logMessage("LogDialog::doShowNonlinearityChart()");

    dataFileName = tempProblemDir() + "/nonlinearity_error.csv";
    imageFileName = tempProblemDir() + "/nonlinearity_error.png";
    showImage();
}


