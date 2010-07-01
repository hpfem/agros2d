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

void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, unit + " ");
    item->setTextAlignment(2, Qt::AlignLeft);
}

void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
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
            cmbFieldVariable->addItem(QString::number(Util::scene()->sceneSolution()->solutionArrayList()->value(i * Util::scene()->problemInfo()->hermes()->numberOfSolution())->time, 'e', 2), i);
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
    txtLineEdit->setText(QString::number(value));
    evaluate();
}

double SLineEditValue::number()
{
    if (evaluate())
        return m_number;
}

void SLineEditValue::setValue(Value value)
{
   txtLineEdit->setText(value.text);
   evaluate();
}

Value SLineEditValue::value()
{
    return Value(txtLineEdit->text());
}

bool SLineEditValue::evaluate(bool quiet)
{
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
    lblValue->setText(text);
    QPalette palette = lblValue->palette();
    palette.setColor(QPalette::WindowText, color);
    lblValue->setPalette(palette);
    lblValue->setVisible(isVisible);
}

void SLineEditValue::focusInEvent(QFocusEvent *event)
{
    txtLineEdit->setFocus(event->reason());
}

// ****************************************************************************************************************

Chart::Chart(QWidget *parent) : QwtPlot(parent)
{
    //  chart style
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
    delete m_curve;
}

void Chart::saveImage(const QString &fileName)
{
    QString fileNameTemp;
    if (fileName.isEmpty())
    {
        fileNameTemp = QFileDialog::getSaveFileName(this, tr("Export image to file"), "data", tr("PNG files (*.png)"));
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
    const bool doReplot = autoReplot();
    setAutoReplot(false);

    m_curve->setData(xval, yval, count);

    setAutoReplot(doReplot);

    replot();
}

// ****************************************************************************************************

FileBrowser::FileBrowser(QWidget *parent) : QListWidget(parent)
{
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
    m_nameFilter = nameFilter;
}

QString FileBrowser::basePath()
{
    return QString(m_basePath);
}

void FileBrowser::setDir(const QString &path)
{
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
    setDir(m_basePath);
}

void FileBrowser::doFileItemActivated(QListWidgetItem *item)
{

}

void FileBrowser::doFileItemDoubleClick(QListWidgetItem *item)
{
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
    QDir dir(dirName);
    if (dir.exists())
    {
        dir.rmdir(dirName);
        refresh();
    }
}

void FileBrowser::deleteFile(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
        refresh();
    }
}

void FileBrowser::deleteObject(const QString &name)
{
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
