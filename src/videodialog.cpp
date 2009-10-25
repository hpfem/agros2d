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
    delete btnEncodeMencoder;
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
    // cmbCodec->addItem("H.263 (h263)", "h263");
    cmbCodec->addItem("H.263+ (h263p)", "h263p");
    cmbCodec->addItem("MPEG-4 (DivX 4/5) (mpeg4)", "mpeg4");
    cmbCodec->addItem("DivX 3 (msmpeg4)", "msmpeg4");
    cmbCodec->addItem("MS MPEG4v2 (msmpeg4v2)", "msmpeg4v2");
    cmbCodec->addItem("Windows Media Video, version 1 (AKA WMV7) (wmv1)", "wmv1");
    cmbCodec->addItem("Windows Media Video, version 2 (AKA WMV8) (wmv2)", "wmv2");
    cmbCodec->addItem("MPEG-1 video (mpeg1video)", "mpeg1video");
    cmbCodec->addItem("MPEG-2 video (mpeg2video)", "mpeg2video");
    cmbCodec->setCurrentIndex(cmbCodec->findData(codec));
    connect(cmbCodec, SIGNAL(currentIndexChanged(int)), this, SLOT(doCommandMencoder()));

    cmbFormat = new QComboBox();
    cmbFormat->addItem("Microsoft Audio/Video Interleaved (avi)", "avi");
    cmbFormat->addItem("MPEG-1/2 system stream format (mpeg)", "mpeg");
    // cmbFormat->addItem("FFmpeg libavformat muxers (lavf)", "lavf");
    cmbFormat->setCurrentIndex(cmbFormat->findData(format));
    connect(cmbFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(doCommandMencoder()));

    txtFPS = new QSpinBox();
    txtFPS->setMinimum(1);
    txtFPS->setValue(fps);
    connect(txtFPS, SIGNAL(valueChanged(int)), this, SLOT(doCommandMencoder()));

    QFormLayout *layoutControls = new QFormLayout();
    layoutControls->addRow(tr("Codec:"), cmbCodec);
    layoutControls->addRow(tr("Format:"), cmbFormat);
    layoutControls->addRow(tr("FPS:"), txtFPS);

    // dialog buttons
    QPushButton *btnCreateImages = new QPushButton(tr("Create images"));
    connect(btnCreateImages, SIGNAL(clicked()), this, SLOT(doCreateImages()));

    btnEncodeMencoder = new QPushButton(tr("Encode video"));
    btnEncodeMencoder->setEnabled(false);
    connect(btnEncodeMencoder, SIGNAL(clicked()), this, SLOT(doEncodeMencoder()));

    btnSaveVideo = new QPushButton(tr("Save video..."));
    btnSaveVideo->setEnabled(false);
    connect(btnSaveVideo, SIGNAL(clicked()), this, SLOT(doSaveVideo()));

    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addWidget(btnCreateImages);
    layoutButton->addWidget(btnEncodeMencoder);
    layoutButton->addWidget(btnSaveVideo);
    layoutButton->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutControls);
    // layout->addWidget(new QLabel(tr("Command:")));
    // layout->addWidget(txtCommand);
    layout->addStretch();
    layout->addLayout(layoutButton);

    setLayout(layout);

    doCommandMencoder();
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

    btnEncodeMencoder->setEnabled(true);
}

void VideoDialog::doEncodeMencoder()
{
    // exec mencoder
    QProcess *processMencoder = new QProcess();
    processMencoder->setStandardOutputFile(tempProblemDir() + "/video/output.txt");
    processMencoder->setStandardErrorFile(tempProblemDir() + "/video/error.txt");
    connect(processMencoder, SIGNAL(finished(int)), this, SLOT(doVideoCreated(int)));

    cout << commandMencoder.toStdString() << endl;
    processMencoder->start(commandMencoder);

    if (!processMencoder->waitForStarted())
    {
        processMencoder->kill();
        return;
    }

    while (!processMencoder->waitForFinished()) {}
}

void VideoDialog::doSaveVideo()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save video to file"), "", tr("AVI files (*.avi)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "avi") fileName += ".avi";
        QFile::remove(fileName);
        QFile::copy(outputFile, fileName);
    }
}

void VideoDialog::doCommandMencoder()
{
    outputFile = tempProblemDir() + "/video/output." + cmbFormat->itemData(cmbFormat->currentIndex()).toString();

    commandMencoder = QString("mencoder \"mf://%1\" -of %4 -mf fps=%5:type=png -ovc lavc -lavcopts vcodec=%3 -o %2").
                      arg(tempProblemDir() + "/video/*.png").
                      arg(outputFile).
                      arg(cmbCodec->itemData(cmbCodec->currentIndex()).toString()).
                      arg(cmbFormat->itemData(cmbFormat->currentIndex()).toString()).
                      arg(txtFPS->value());
}

void VideoDialog::doVideoCreated(int result)
{
    if (result == 0)
    {
        btnSaveVideo->setEnabled(true);
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
