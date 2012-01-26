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

#include "videodialog.h"

#include "gui.h"
#include "scene.h"
#include "scenesolution.h"
#include "sceneview.h"
#include "hermes2d/problem.h"

VideoDialog::VideoDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    logMessage("VideoDialog::VideoDialog()");

    m_sceneView = sceneView;

    setModal(true);
    setWindowIcon(icon("video"));
    setWindowTitle(tr("Video"));

    // store timestep
    m_timeStep = Util::problem()->timeStep();

    // timer animate
    timerAnimate = new QTimer(this);
    connect(timerAnimate, SIGNAL(timeout()), this, SLOT(doAnimateNextStep()));

    // timer create images
    timerFile = new QTimer(this);
    // connect(timerFile, SIGNAL(timeout()), this, SLOT(doAnimateNextStep()));

    createControls();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

VideoDialog::~VideoDialog()
{
    logMessage("VideoDialog::~VideoDialog()");

    // restore previous timestep
    //TODO odkomentovat
    //Util::scene()->activeSceneSolution()->setTimeStep(m_timeStep);

    delete timerAnimate;
}

void VideoDialog::showDialog()
{
    assert(0);
//    logMessage("VideoDialog::showDialog()");

//    doCommandFFmpeg();

//    txtAnimateFrom->setMaximum(Util::scene()->activeSceneSolution()->timeStepCount());
//    txtAnimateTo->setMaximum(Util::scene()->activeSceneSolution()->timeStepCount());
//    txtAnimateTo->setValue(Util::scene()->activeSceneSolution()->timeStepCount());

//    doValueFromChanged(txtAnimateFrom->value());
//    doValueToChanged(txtAnimateTo->value());

//    exec();
}

void VideoDialog::createControls()
{
    logMessage("VideoDialog::createControls()");

    // tab
    QTabWidget *tabType = new QTabWidget();
    tabType->addTab(createControlsViewport(), icon(""), tr("Viewport"));
    tabType->addTab(createControlsFile(), icon(""), tr("File"));

    btnClose = new QPushButton(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addLayout(layoutButton);

    setLayout(layout);
}

QWidget *VideoDialog::createControlsFile()
{
    logMessage("VideoDialog::createControlsFile()");

    QSettings settings;
    QString codec = settings.value("VideoDialog/Codec", "msmpeg4v2").value<QString>();
    QString format = settings.value("VideoDialog/Format", "avi").value<QString>();
    int fps = settings.value("VideoDialog/FPS", 15).value<int>();

    // file
    cmbCodec = new QComboBox();
    cmbCodec->addItem("Motion JPEG (mjpeg)", "mjpeg");
    cmbCodec->addItem("Flash Video (flv)", "flv");
    cmbCodec->addItem("MPEG-4 part 2 (mpeg4)", "mpeg4");
    cmbCodec->addItem("MPEG-4 part 2 Microsoft variant version 2 (msmpeg4v2)", "msmpeg4v2");
    cmbCodec->addItem("Windows Media Video 7 (wmv1)", "wmv1");
    cmbCodec->addItem("Windows Media Video 8 (wmv2)", "wmv2");
    cmbCodec->addItem("H.263 (h263)", "h263");
    // cmbCodec->addItem("Graphics Interchange Format (gif)", "gif");
    // cmbCodec->addItem("Portable Network Graphic (png)", "png");

    cmbCodec->setCurrentIndex(cmbCodec->findData(codec));
    connect(cmbCodec, SIGNAL(currentIndexChanged(int)), this, SLOT(doCommandFFmpeg()));

    cmbFormat = new QComboBox();
    cmbFormat->addItem("Microsoft Audio/Video Interleaved (avi)", "avi");
    cmbFormat->addItem("MPEG system stream format (mpeg)", "mpeg");
    // cmbFormat->addItem("FLV format (flv)", "flv");
    cmbFormat->setCurrentIndex(cmbFormat->findData(format));
    connect(cmbFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(doCommandFFmpeg()));

    txtFPS = new QSpinBox();
    txtFPS->setMinimum(1);
    txtFPS->setValue(fps);
    connect(txtFPS, SIGNAL(valueChanged(int)), this, SLOT(doCommandFFmpeg()));

    progressBar = new QProgressBar(this);

    QFormLayout *layoutControlsFile = new QFormLayout();
    layoutControlsFile->addRow(tr("Codec:"), cmbCodec);
    layoutControlsFile->addRow(tr("Format:"), cmbFormat);
    layoutControlsFile->addRow(tr("FPS:"), txtFPS);
    layoutControlsFile->addRow(tr("Progress:"), progressBar);

    // dialog buttons
    QPushButton *btnCreateImages = new QPushButton(tr("Create images"));
    connect(btnCreateImages, SIGNAL(clicked()), this, SLOT(doCreateImages()));

    btnEncodeFFmpeg = new QPushButton(tr("Encode"));
    btnEncodeFFmpeg->setEnabled(false);
    connect(btnEncodeFFmpeg, SIGNAL(clicked()), this, SLOT(doEncodeFFmpeg()));

    btnSaveVideo = new QPushButton(tr("Save ..."));
    btnSaveVideo->setEnabled(false);
    connect(btnSaveVideo, SIGNAL(clicked()), this, SLOT(doSaveVideo()));

    btnOpenVideo = new QPushButton(tr("Open"));
    btnOpenVideo->setEnabled(false);
    connect(btnOpenVideo, SIGNAL(clicked()), this, SLOT(doOpenVideo()));

    QHBoxLayout *layoutButtonFile = new QHBoxLayout();
    layoutButtonFile->addWidget(btnCreateImages);
    layoutButtonFile->addWidget(btnEncodeFFmpeg);
    layoutButtonFile->addWidget(btnSaveVideo);
    layoutButtonFile->addWidget(btnOpenVideo);

    QVBoxLayout *layoutFile = new QVBoxLayout();
    layoutFile->addLayout(layoutControlsFile);
    layoutFile->addStretch();
    layoutFile->addLayout(layoutButtonFile);

    QWidget *widFile = new QWidget();
    widFile->setLayout(layoutFile);

    return widFile;
}

QWidget *VideoDialog::createControlsViewport()
{
    logMessage("VideoDialog::createControlsViewport()");

    // viewport
    lblAnimateTime = new QLabel("0.0");

    txtAnimateFrom = new QSpinBox();
    txtAnimateFrom->setMinimum(1);
    connect(txtAnimateFrom, SIGNAL(valueChanged(int)), this, SLOT(doValueFromChanged(int)));

    txtAnimateTo = new QSpinBox();
    txtAnimateTo->setMinimum(1);
    connect(txtAnimateTo, SIGNAL(valueChanged(int)), this, SLOT(doValueToChanged(int)));

    sldAnimate = new QSlider(Qt::Horizontal);
    sldAnimate->setTickPosition(QSlider::TicksBelow);
    connect(sldAnimate, SIGNAL(valueChanged(int)), this, SLOT(doSetTimeStep(int)));

    txtAnimateDelay = new SLineEditDouble(0.1);

    QGridLayout *layoutControlsViewport = new QGridLayout();
    layoutControlsViewport->addWidget(new QLabel(tr("From:")), 0, 0);
    layoutControlsViewport->addWidget(txtAnimateFrom, 0, 1);
    layoutControlsViewport->addWidget(new QLabel(tr("To:")), 1, 0);
    layoutControlsViewport->addWidget(txtAnimateTo, 1, 1);
    layoutControlsViewport->addWidget(new QLabel(tr("Delay:")), 0, 2);
    layoutControlsViewport->addWidget(txtAnimateDelay, 0, 3);
    layoutControlsViewport->addWidget(new QLabel(tr("Time:")), 1, 2);
    layoutControlsViewport->addWidget(lblAnimateTime, 1, 3, 1, 1, Qt::AlignLeft);
    layoutControlsViewport->addWidget(new QLabel(tr("Time step:")), 2, 0);

    // dialog buttons
    btnAnimate = new QPushButton(tr("Animate"));
    connect(btnAnimate, SIGNAL(clicked()), this, SLOT(doAnimate()));

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnAnimate);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addLayout(layoutControlsViewport);
    layoutViewport->addWidget(sldAnimate);
    layoutViewport->addStretch();
    layoutViewport->addLayout(layoutButtonViewport);

    QWidget *widViewport = new QWidget();
    widViewport->setLayout(layoutViewport);

    return widViewport;
}

void VideoDialog::doAnimate()
{
    logMessage("VideoDialog::doAnimate()");

    if (timerAnimate->isActive())
    {
        btnClose->setEnabled(true);

        timerAnimate->stop();
        btnAnimate->setText(tr("Animate"));
    }
    else
    {
        btnClose->setEnabled(false);

        btnAnimate->setText(tr("Stop"));
        doSetTimeStep(txtAnimateFrom->value());
        timerAnimate->start(txtAnimateDelay->value() * 1e3);
    }
}

void VideoDialog::doAnimateNextStep()
{
    assert(0);
//    logMessage("VideoDialog::doAnimateNextStep()");

//    if (Util::scene()->activeSceneSolution()->timeStep() + 1 < txtAnimateTo->value())
//    {
//        doSetTimeStep(Util::scene()->activeSceneSolution()->timeStep() + 2);
//    }
//    else
//    {
//        doAnimate();
//    }
}

void VideoDialog::doSetTimeStep(int index)
{
    assert(0);
//    logMessage("VideoDialog::doSetTimeStep()");

//    Util::scene()->sceneSolution()->setTimeStep(index - 1, false);
//    sldAnimate->setValue(index);

//    QString time = QString::number(Util::scene()->activeSceneSolution()->time(), 'g');
//    lblAnimateTime->setText(time + " s");

//    QApplication::processEvents();
}

void VideoDialog::doValueFromChanged(int index)
{
    logMessage("VideoDialog::doValueFromChanged()");

    sldAnimate->setMinimum(txtAnimateFrom->value());
}

void VideoDialog::doValueToChanged(int index)
{
    logMessage("VideoDialog::doValueToChanged()");

    sldAnimate->setMaximum(txtAnimateTo->value());
}

void VideoDialog::doCreateImages()
{
    assert(0);
//    logMessage("VideoDialog::doCreateImages()");

//    btnClose->setEnabled(false);
//    progressBar->setMaximum(Util::scene()->activeSceneSolution()->timeStepCount() - 1);
//    progressBar->setValue(0);

//    // create directory
//    QDir(tempProblemDir()).mkdir("video");
//    for (int i = 0; i < Util::scene()->activeSceneSolution()->timeStepCount(); i++)
//    {
//        progressBar->setValue(i);
//        Util::scene()->activeSceneSolution()->setTimeStep(i, false);
//        m_sceneView->saveImageToFile(tempProblemDir() + QString("/video/video_%1.png").arg(QString("0000000" + QString::number(i)).right(8)));
//    }

//    btnClose->setEnabled(true);
//    btnEncodeFFmpeg->setEnabled(true);
}

void VideoDialog::doEncodeFFmpeg()
{
    logMessage("VideoDialog::doEncodeFFmpeg()");

    btnClose->setEnabled(false);

    // exec mencoder
    QProcess *processFFmpeg = new QProcess();
    processFFmpeg->setStandardOutputFile(tempProblemDir() + "/video/output.txt");
    processFFmpeg->setStandardErrorFile(tempProblemDir() + "/video/error.txt");
    connect(processFFmpeg, SIGNAL(finished(int)), this, SLOT(doVideoCreated(int)));

    processFFmpeg->start(commandFFmpeg);

    if (!processFFmpeg->waitForStarted())
    {
        processFFmpeg->kill();
        btnClose->setEnabled(true);
        return;
    }

    while (!processFFmpeg->waitForFinished()) {}
}

void VideoDialog::doSaveVideo()
{
    logMessage("VideoDialog::doSaveVideo()");

    QSettings settings;
    QString dir = settings.value("General/LastVideoDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save video to file"), dir, QString("%1 (*.%2)").arg(cmbFormat->currentText()).arg(cmbFormat->itemData(cmbFormat->currentIndex()).toString()));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "avi") fileName += ".avi";
        QFile::remove(fileName);
        QFile::copy(outputFile, fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastVideoDir", fileInfo.absolutePath());
    }
}

void VideoDialog::doOpenVideo()
{
    logMessage("VideoDialog::doOpenVideo()");

    QDesktopServices::openUrl(QUrl::fromLocalFile(outputFile));
}

void VideoDialog::doCommandFFmpeg()
{
    logMessage("VideoDialog::doCommandFFmpeg()");

    outputFile = tempProblemDir() + "/video/output." + cmbFormat->itemData(cmbFormat->currentIndex()).toString();

    QString ffmpegBinary = "ffmpeg";
    if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "ffmpeg.exe"))
        ffmpegBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "ffmpeg.exe\"";
    if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "ffmpeg"))
        ffmpegBinary = QApplication::applicationDirPath() + QDir::separator() + "ffmpeg";

    commandFFmpeg = QString(Util::config()->commandFFmpeg).
                    arg(ffmpegBinary).
                    arg(txtFPS->value()).
                    arg(tempProblemDir() + "/video/").
                    arg(cmbCodec->itemData(cmbCodec->currentIndex()).toString()).
                    arg(outputFile);
}

void VideoDialog::doVideoCreated(int result)
{
    logMessage("VideoDialog::doVideoCreated()");

    btnClose->setEnabled(true);

    if (result == 0)
    {
        btnSaveVideo->setEnabled(true);
        btnOpenVideo->setEnabled(true);
    }
    else
    {
        QMessageBox::critical(this, tr("FFmpeg error"), readFileContent(tempProblemDir() + "/video/error.txt"));
    }

    // remove files
    QFile::remove(tempProblemDir() + QString("/video/video*.png"));
}

void VideoDialog::doClose()
{
    logMessage("VideoDialog::doClose()");

    QSettings settings;
    settings.setValue("VideoDialog/Codec", cmbCodec->itemData(cmbCodec->currentIndex()).toString());
    settings.setValue("VideoDialog/Format", cmbFormat->itemData(cmbFormat->currentIndex()).toString());
    settings.setValue("VideoDialog/FPS", txtFPS->value());

    hide();
}
