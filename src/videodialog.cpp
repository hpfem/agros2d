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

#include "sceneview.h"

VideoDialog::VideoDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;

    setModal(true);
    setWindowIcon(icon("video"));
    setWindowTitle(tr("Video"));

    // store timestep
    m_timeStep = Util::scene()->sceneSolution()->timeStep();

    // timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(doAnimateNextStep()));

    createControls();

    resize(sizeHint());
    setMinimumSize(sizeHint());
}

VideoDialog::~VideoDialog()
{    
    // restore previous timestep
    Util::scene()->sceneSolution()->setTimeStep(m_timeStep);

    delete btnEncodeFFmpeg;
    delete btnSaveVideo;
    delete cmbCodec;
    delete cmbFormat;
    delete txtFPS;
    
    delete btnAnimate;
    delete txtAnimateFrom;
    delete txtAnimateTo;

    delete timer;
}

void VideoDialog::showDialog()
{   
    doCommandFFmpeg();

    txtAnimateFrom->setMaximum(Util::scene()->sceneSolution()->timeStepCount());
    txtAnimateTo->setMaximum(Util::scene()->sceneSolution()->timeStepCount());
    txtAnimateTo->setValue(Util::scene()->sceneSolution()->timeStepCount());

    doValueFromChanged(txtAnimateFrom->value());
    doValueToChanged(txtAnimateTo->value());

    exec();
}

void VideoDialog::createControls()
{   
    // tab
    QTabWidget *tabType = new QTabWidget();
    tabType->addTab(createControlsViewport(), icon(""), tr("Viewport"));
    tabType->addTab(createControlsFile(), icon(""), tr("File"));

    QPushButton *btnClose = new QPushButton(tr("Close"));
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

    QFormLayout *layoutControlsFile = new QFormLayout();
    layoutControlsFile->addRow(tr("Codec:"), cmbCodec);
    layoutControlsFile->addRow(tr("Format:"), cmbFormat);
    layoutControlsFile->addRow(tr("FPS:"), txtFPS);

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
    if (timer->isActive())
    {
        timer->stop();
        btnAnimate->setText(tr("Animate"));
    }
    else
    {
        btnAnimate->setText(tr("Stop"));
        doSetTimeStep(txtAnimateFrom->value());
        timer->start(txtAnimateDelay->value() * 1e3);
    }
}

void VideoDialog::doAnimateNextStep()
{
    if (Util::scene()->sceneSolution()->timeStep() + 1 < txtAnimateTo->value())
    {
        doSetTimeStep(Util::scene()->sceneSolution()->timeStep() + 2);
    }
    else
    {
        // stop timer
        doAnimate();
    }
}

void VideoDialog::doSetTimeStep(int index)
{
    Util::scene()->sceneSolution()->setTimeStep(index - 1);
    sldAnimate->setValue(index);

    QString time = QString::number(Util::scene()->sceneSolution()->time(), 'g');
    lblAnimateTime->setText(time + " s");

    QApplication::processEvents();
}

void VideoDialog::doValueFromChanged(int index)
{
    sldAnimate->setMinimum(txtAnimateFrom->value());
}

void VideoDialog::doValueToChanged(int index)
{
    sldAnimate->setMaximum(txtAnimateTo->value());
}

void VideoDialog::doCreateImages()
{
    // create directory
    QDir(tempProblemDir()).mkdir("video");
    for (int i = 0; i < Util::scene()->sceneSolution()->timeStepCount(); i++)
    {
        Util::scene()->sceneSolution()->setTimeStep(i);
        m_sceneView->saveImageToFile(tempProblemDir() + QString("/video/video_%1.png").arg(QString("0000" + QString::number(i)).right(5)));
    }

    btnEncodeFFmpeg->setEnabled(true);    
}

void VideoDialog::doEncodeFFmpeg()
{
    // exec mencoder
    QProcess *processFFmpeg = new QProcess();
    processFFmpeg->setStandardOutputFile(tempProblemDir() + "/video/output.txt");
    processFFmpeg->setStandardErrorFile(tempProblemDir() + "/video/error.txt");
    connect(processFFmpeg, SIGNAL(finished(int)), this, SLOT(doVideoCreated(int)));

    cout << commandFFmpeg.toStdString() << endl;
    processFFmpeg->start(commandFFmpeg);

    if (!processFFmpeg->waitForStarted())
    {
        processFFmpeg->kill();
        return;
    }

    while (!processFFmpeg->waitForFinished()) {}
}

void VideoDialog::doSaveVideo()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save video to file"), "", QString("%1 (*.%2)").arg(cmbFormat->currentText()).arg(cmbFormat->itemData(cmbFormat->currentIndex()).toString()));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "avi") fileName += ".avi";
        QFile::remove(fileName);
        QFile::copy(outputFile, fileName);
    }
}

void VideoDialog::doOpenVideo()
{
    QDesktopServices::openUrl(outputFile);
}

void VideoDialog::doCommandFFmpeg()
{
    outputFile = tempProblemDir() + "/video/output." + cmbFormat->itemData(cmbFormat->currentIndex()).toString();
    commandFFmpeg = QString("ffmpeg -r %4 -y -i \"%1video_%05d.png\" -vcodec %3 \"%2\"").
                    arg(tempProblemDir() + "/video/").
                    arg(outputFile).
                    arg(cmbCodec->itemData(cmbCodec->currentIndex()).toString()).
                    arg(txtFPS->value());
}

void VideoDialog::doVideoCreated(int result)
{
    if (result == 0)
    {
        btnSaveVideo->setEnabled(true);
        btnOpenVideo->setEnabled(true);
    }

    // remove files
    QFile::remove(tempProblemDir() + QString("/video/video*.png"));
}

void VideoDialog::doClose()
{
    QSettings settings;
    settings.setValue("VideoDialog/Codec", cmbCodec->itemData(cmbCodec->currentIndex()).toString());
    settings.setValue("VideoDialog/Format", cmbFormat->itemData(cmbFormat->currentIndex()).toString());
    settings.setValue("VideoDialog/FPS", txtFPS->value());

    hide();
}
