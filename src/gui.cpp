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

#include "gui.h"

#include "scene.h"
#include "scenesolution.h"
#include "progressdialog.h"

#include "hermes2d/hermes_field.h"

void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit)
{
    logMessage("addTreeWidgetItemValue()");

    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, unit + " ");
    item->setTextAlignment(2, Qt::AlignLeft);
}

void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    logMessage("fillComboBoxScalarVariable()");

    // store variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();

    // clear combo
    cmbFieldVariable->clear();
    Util::scene()->problemInfo()->hermes()->fillComboBoxScalarVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
}

void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    logMessage("fillComboBoxVectorVariable()");

    // store variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();

    // clear combo
    cmbFieldVariable->clear();
    Util::scene()->problemInfo()->hermes()->fillComboBoxVectorVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
}

void fillComboBoxTimeStep(QComboBox *cmbFieldVariable)
{
    logMessage("fillComboBoxTimeStep()");

    cmbFieldVariable->blockSignals(true);

    // store variable
    int timeStep = cmbFieldVariable->currentIndex();
    if (timeStep == -1) timeStep = Util::scene()->sceneSolution()->timeStepCount() - 1;

    // clear combo
    cmbFieldVariable->clear();
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        for (int i = 0; i < Util::scene()->sceneSolution()->timeStepCount(); i++)
        {
            cmbFieldVariable->addItem(QString::number(Util::scene()->sceneSolution()->solutionArrayList().value(i * Util::scene()->problemInfo()->hermes()->numberOfSolution())->time, 'e', 2), i);
        }
    }
    else
    {
        cmbFieldVariable->addItem(QString::number(0.0, 'e', 2), 0);
    }

    cmbFieldVariable->setCurrentIndex(timeStep);
    cmbFieldVariable->blockSignals(false);
}

// ***********************************************************************************************************

SLineEditValue::SLineEditValue(QWidget *parent) : QWidget(parent)
{
    logMessage("SLineEditValue::SLineEditValue()");

    m_minimum = -CONST_DOUBLE;
    m_minimumSharp = -CONST_DOUBLE;
    m_maximum =  CONST_DOUBLE;
    m_maximumSharp =  CONST_DOUBLE;

    // create controls
    txtLineEdit = new QLineEdit(this);
    txtLineEdit->setToolTip(tr("This textedit allows using variables."));
    txtLineEdit->setText("0");
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(evaluate()));
    connect(txtLineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    lblValue = new QLabel(this);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);

    setLayout(layout);

    evaluate();
}

void SLineEditValue::setNumber(double value)
{
    logMessage("SLineEditValue::setNumber()");

    txtLineEdit->setText(QString::number(value));
    evaluate();
}

double SLineEditValue::number()
{
    logMessage("SLineEditValue::number()");

    if (evaluate())
        return m_number;
    else
        return 0.0;
}

void SLineEditValue::setValue(Value value)
{
    logMessage("SLineEditValue::setValue()");

    txtLineEdit->setText(value.text);
    evaluate();
}

Value SLineEditValue::value()
{
    logMessage("SLineEditValue::value()");

    return Value(txtLineEdit->text());
}

bool SLineEditValue::evaluate(bool quiet)
{
    logMessage("SLineEditValue::evaluate()");

    bool isOk = false;

    Value val = value();
    if (val.evaluate(quiet))
    {
        if (val.number <= m_minimumSharp)
        {
            setLabel(QString("<= %1").arg(m_minimumSharp), QColor(Qt::blue), true);
        }
        else if (val.number >= m_maximumSharp)
        {
            setLabel(QString(">= %1").arg(m_maximumSharp), QColor(Qt::blue), true);
        }
        else if (val.number < m_minimum)
        {
            setLabel(QString("< %1").arg(m_minimum), QColor(Qt::blue), true);
        }
        else if (val.number > m_maximum)
        {
            setLabel(QString("> %1").arg(m_maximum), QColor(Qt::blue), true);
        }
        else
        {
            m_number = val.number;
            setLabel(QString("%1").arg(m_number, 0, 'g', 3), QApplication::palette().color(QPalette::WindowText), Util::config()->lineEditValueShowResult);
            isOk = true;
        }
    }
    else
    {
        setLabel(tr("error"), QColor(Qt::red), true);
        setFocus();
    }

    if (isOk)
    {
        emit evaluated(false);
        return true;
    }
    else
    {
        emit evaluated(true);
        return false;
    }
}

void SLineEditValue::setLabel(const QString &text, QColor color, bool isVisible)
{
    logMessage("SLineEditValue::setLabel()");

    lblValue->setText(text);
    QPalette palette = lblValue->palette();
    palette.setColor(QPalette::WindowText, color);
    lblValue->setPalette(palette);
    lblValue->setVisible(isVisible);
}

void SLineEditValue::focusInEvent(QFocusEvent *event)
{
    logMessage("SLineEditValue::focusInEvent()");

    txtLineEdit->setFocus(event->reason());
}

// ****************************************************************************************************************

Chart::Chart(QWidget *parent) : QwtPlot(parent)
{
    logMessage("Chart::Chart()");

    //  chart style
    clear();
    setAutoReplot(false);
    setMargin(5);
    setTitle("");
    setCanvasBackground(QColor(Qt::white));
    setMinimumSize(420, 260);

    // legend
    /*
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    insertLegend(legend, QwtPlot::BottomLegend);
    */

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::darkGray, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->enableX(true);
    grid->enableY(true);
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->attach(this);

    // axes
    setAxisTitle(QwtPlot::xBottom, " ");
    setAxisFont(QwtPlot::xBottom, QFont("Helvetica", 9, QFont::Normal));
    setAxisTitle(QwtPlot::yLeft, " ");
    setAxisFont(QwtPlot::yLeft, QFont("Helvetica", 9, QFont::Normal));

    // curve styles
    QwtSymbol sym;

    sym.setStyle(QwtSymbol::Cross);
    sym.setPen(QColor(Qt::black));
    sym.setSize(5);

    // curve
    m_curve = new QwtPlotCurve();
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curve->setPen(QPen(Qt::blue));
    m_curve->setCurveAttribute(QwtPlotCurve::Inverted);
    m_curve->setYAxis(QwtPlot::yLeft);
    m_curve->attach(this);
}

Chart::~Chart()
{
    logMessage("Chart::~Chart()");

    delete m_curve;
}

void Chart::saveImage(const QString &fileName)
{
    logMessage("Chart::saveImage()");

    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileNameTemp;
    if (fileName.isEmpty())
    {
        fileNameTemp = QFileDialog::getSaveFileName(this, tr("Export image to file"), dir, tr("PNG files (*.png)"));

        QFileInfo fileInfo(fileNameTemp);
        if (!fileNameTemp.isEmpty())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
    else
    {
        fileNameTemp = fileName;
    }

    if (!fileNameTemp.isEmpty())
    {
        QFileInfo fileInfo(fileNameTemp);
        if (fileInfo.suffix().toLower() != "png") fileNameTemp += ".png";

        QImage imageChart = image();
        imageChart.save(fileNameTemp, "PNG");
    }
}

QImage Chart::image() const
{
    logMessage("Chart::image()");

    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::white); // Qt::transparent

    QwtPlotPrintFilter filter;
    int options = QwtPlotPrintFilter::PrintAll;
    options &= ~QwtPlotPrintFilter::PrintBackground;
    options |= QwtPlotPrintFilter::PrintFrameWithScales;
    filter.setOptions(options);

    print(pixmap, filter);

    return pixmap.toImage();
}

void Chart::setData(double *xval, double *yval, int count)
{
    logMessage("Chart::setData()");

    const bool doReplot = autoReplot();
    setAutoReplot(false);

    m_curve->setData(xval, yval, count);

    setAutoReplot(doReplot);

    replot();
}

// ****************************************************************************************************

FileBrowser::FileBrowser(QWidget *parent) : QListWidget(parent)
{
    logMessage("FileBrowser::FileBrowser()");

    setNameFilter("");
    setDir(QDir::currentPath());
    setContextMenuPolicy(Qt::CustomContextMenu);

    actCreateDirectory = new QAction(icon("file-folder"), tr("&Create directory"), this);
    connect(actCreateDirectory, SIGNAL(triggered()), this, SLOT(createDir()));

    actCreateFile = new QAction(icon("file-text"), tr("Create &file"), this);
    connect(actCreateFile, SIGNAL(triggered()), this, SLOT(createFile()));

    actRename = new QAction(icon(""), tr("&Rename"), this);
    connect(actRename, SIGNAL(triggered()), this, SLOT(renameObject()));

    actDelete = new QAction(icon(""), tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteObject()));

    mnuContext = new QMenu(this);
    mnuContext->addAction(actCreateDirectory);
    mnuContext->addAction(actCreateFile);
    mnuContext->addSeparator();
    mnuContext->addAction(actRename);
    mnuContext->addAction(actDelete);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(this, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(doFileItemActivated(QListWidgetItem *)));
    connect(this, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(doFileItemActivated(QListWidgetItem *)));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(doFileItemDoubleClick(QListWidgetItem *)));
}

void FileBrowser::setNameFilter(const QString &nameFilter)
{
    logMessage("FileBrowser::setNameFilter()");

    m_nameFilter = nameFilter;
}

QString FileBrowser::basePath()
{
    logMessage("FileBrowser::basePath()");

    return QString(m_basePath);
}

void FileBrowser::setDir(const QString &path)
{
    logMessage("FileBrowser::setDir()");

    clear();

    QDir dir(path, m_nameFilter, QDir::DirsFirst);
    dir.setFilter(QDir::AllEntries | QDir::AllDirs);

    QStringList entries = dir.entryList();
    foreach(QString item, entries)
    {
        if (dir.isRoot() && (item == "..")) continue;
        if (item != ".")
        {
            QFileInfo fileInfo(path + QDir::separator() + item);
            if (QDir(path + QDir::separator() + item).exists())
            {
                addItem(new QListWidgetItem(icon("file-folder"), item));
            }
            else
            {
                if (fileInfo.suffix() == "py")
                    addItem(new QListWidgetItem(icon("file-python"), item));
                else
                    addItem(new QListWidgetItem(icon("file-text"), item));
            }
        }
    }

    m_basePath = dir.canonicalPath();
    emit directoryChanged(m_basePath);
}

void FileBrowser::refresh()
{
    logMessage("FileBrowser::refresh()");

    setDir(m_basePath);
}

void FileBrowser::doFileItemActivated(QListWidgetItem *item)
{
    logMessage("FileBrowser::doFileItemActivated()");
}

void FileBrowser::doFileItemDoubleClick(QListWidgetItem *item)
{
    logMessage("FileBrowser::doFileItemDoubleClick()");

    QString path = m_basePath + QDir::separator() + item->text();
    QDir dir(path);
    if (dir.exists())
    {
        setDir(dir.canonicalPath());
        emit fileItemDoubleClick(dir.canonicalPath());
    }
    else
    {
        emit fileItemDoubleClick(path);
    }
}

void FileBrowser::doContextMenu(const QPoint &point)
{
    logMessage("FileBrowser::doContextMenu()");

    actDelete->setEnabled(false);
    actRename->setEnabled(false);

    QListWidgetItem *item = itemAt(point);
    if (item)
    {
        if (item->text() != "..")
        {
            actDelete->setEnabled(true);
            actRename->setEnabled(true);
        }
    }
    mnuContext->exec(QCursor::pos());
}

void FileBrowser::createDir(const QString &dirName)
{
    logMessage("FileBrowser::createDir()");

    QString str;
    if (dirName.isEmpty())
        str = QInputDialog::getText(QApplication::activeWindow(), tr("Create directory"), tr("Enter directory name"));
    else
        str = dirName;

    if (str.isEmpty()) return;

    QDir(m_basePath).mkdir(str);
    setDir(m_basePath + QDir::separator() + str);
}

void FileBrowser::createFile(const QString &fileName)
{
    logMessage("FileBrowser::createFile()");

    QString str;
    if (fileName.isEmpty())
        str = QInputDialog::getText(QApplication::activeWindow(), tr("Create file"), tr("Enter file name"));
    else
        str = fileName;

    if (str.isEmpty()) return;

    // add extension
    QFileInfo fileInfo(str);
    if (fileInfo.suffix() != "py") str += ".py";

    QFile file(m_basePath + QDir::separator() + str);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        QMessageBox::critical(this, tr("Create file"), file.errorString());
    else
    {
        QTextStream out(&file);
        out << "";
        file.close();

        refresh();
    }
}

void FileBrowser::deleteDir(const QString &dirName)
{
    logMessage("FileBrowser::deleteDir()");

    QDir dir(dirName);
    if (dir.exists())
    {
        dir.rmdir(dirName);
        refresh();
    }
}

void FileBrowser::deleteFile(const QString &fileName)
{
    logMessage("FileBrowser::deleteFile()");

    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
        refresh();
    }
}

void FileBrowser::deleteObject(const QString &name)
{
    logMessage("FileBrowser::deleteObject()");

    QString str;
    if (name.isEmpty())
        str = m_basePath + QDir::separator() + currentItem()->text();
    else
        str = name;

    QDir dir(str);
    if (dir.exists())
        deleteDir(str);
    else
        deleteFile(str);
}

void FileBrowser::renameDir(const QString &dirName)
{
    logMessage("FileBrowser::renameDir()");

    QString str = QInputDialog::getText(QApplication::activeWindow(), tr("Rename directory"), tr("Enter directory name"),
                                        QLineEdit::Normal, QDir(dirName).dirName());

    if (str.isEmpty()) return;

    if (!QDir(m_basePath).rename(dirName, str))
        QMessageBox::warning(this, tr("Rename directory"), tr("Directory could not be renamed."));
    else
        refresh();
}

void FileBrowser::renameFile(const QString &fileName)
{
    logMessage("FileBrowser::renameFile()");

    QString str = QInputDialog::getText(QApplication::activeWindow(), tr("Rename file"), tr("Enter file name"),
                                        QLineEdit::Normal, QFileInfo(fileName).baseName());

    if (str.isEmpty()) return;

    // add extension
    QFileInfo fileInfo(str);
    if (fileInfo.suffix() != "py") str += ".py";

    if (!QFile::rename(fileName, QFileInfo(fileName).absolutePath() + QDir::separator() + str))
        QMessageBox::warning(this, tr("Rename file"), tr("File could not be renamed."));
    else
        refresh();
}

void FileBrowser::renameObject(const QString &name)
{
    logMessage("FileBrowser::renameObject()");

    QString str;
    if (name.isEmpty())
        str = m_basePath + QDir::separator() + currentItem()->text();
    else
        str = name;

    QDir dir(str);
    if (dir.exists())
        renameDir(str);
    else
        renameFile(str);
}

// *******************************************************************************************************************************

ImageLoaderDialog::ImageLoaderDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ImageLoaderDialog::ImageLoaderDialog()");

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
    logMessage("ImageLoaderDialog::~ImageLoaderDialog()");

    QSettings settings;
    settings.setValue("ImageLoaderDialog/Geometry", saveGeometry());
}

void ImageLoaderDialog::createControls()
{
    logMessage("ImageLoaderDialog::createControls()");

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

    txtX = new SLineEditDouble(0.0);
    txtY = new SLineEditDouble(0.0);
    txtWidth = new SLineEditDouble(1.0);
    txtHeight = new SLineEditDouble(1.0);

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
    logMessage("ImageLoaderDialog::doLoadFile()");

    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), m_fileName, tr("Images (*.png *.bmp *.jpg)"));
    if (!fileName.isEmpty())
    {
        doLoadFile(fileName);
        QFileInfo fileInfo(fileName);
        settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void ImageLoaderDialog::doLoadFile(const QString &fileName)
{
    logMessage("ImageLoaderDialog::doLoadFile(const QString &fileName)");

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
    logMessage("ImageLoaderDialog::doAccept()");

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
    logMessage("ImageLoaderDialog::doReject()");

    reject();
}
