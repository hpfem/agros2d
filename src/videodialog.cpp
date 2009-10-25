#include "videodialog.h"

#include "sceneview.h"

VideoDialog::VideoDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;

    setModal(true);
    setWindowIcon(icon("video"));
    setWindowTitle(tr("Video"));

    createControls();

    resize(sizeHint());
    setMinimumSize(sizeHint());
}

VideoDialog::~VideoDialog()
{
    delete btnEncodeFFmpeg;
    delete btnSaveVideo;
    delete cmbCodec;
    delete cmbFormat;
    delete txtFPS;
}

void VideoDialog::showDialog()
{
    exec();
}

void VideoDialog::createControls()
{   
    QSettings settings;
    QString codec = settings.value("VideoDialog/Codec", "msmpeg4v2").value<QString>();
    QString format = settings.value("VideoDialog/Format", "avi").value<QString>();
    int fps = settings.value("VideoDialog/FPS", 15).value<int>();

    cmbCodec = new QComboBox();
    cmbCodec->addItem("Motion JPEG (mjpeg)", "mjpeg");
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

    QFormLayout *layoutControls = new QFormLayout();
    layoutControls->addRow(tr("Codec:"), cmbCodec);
    layoutControls->addRow(tr("Format:"), cmbFormat);
    layoutControls->addRow(tr("FPS:"), txtFPS);

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

    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addWidget(btnCreateImages);
    layoutButton->addWidget(btnEncodeFFmpeg);
    layoutButton->addWidget(btnSaveVideo);
    layoutButton->addWidget(btnOpenVideo);
    layoutButton->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutControls);
    // layout->addWidget(new QLabel(tr("Command:")));
    // layout->addWidget(txtCommand);
    layout->addStretch();
    layout->addLayout(layoutButton);

    setLayout(layout);

    doCommandFFmpeg();
}

void VideoDialog::doCreateImages()
{
    // create directory
    QDir(tempProblemDir()).mkdir("video");
    for (int i = 0; i < Util::scene()->sceneSolution()->timeStepCount(); i++)
    {
        Util::scene()->sceneSolution()->setSolutionArray(i);
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
