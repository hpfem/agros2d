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
    if (timeStep == -1) timeStep = 0;

    // clear combo
    cmbFieldVariable->clear();
    for (int i = 0; i < Util::scene()->sceneSolution()->timeStepCount(); i++)
    {
        cmbFieldVariable->addItem(QString::number(Util::scene()->sceneSolution()->solutionArrayList()->value(i)->time, 'e', 2), i);
    }

    cmbFieldVariable->setCurrentIndex(timeStep);
    cmbFieldVariable->blockSignals(false);
}

// ***********************************************************************************************************

SLineEditValue::SLineEditValue(QWidget *parent) : QWidget(parent)
{
    // create controls
    txtLineEdit = new QLineEdit(this);
    txtLineEdit->setToolTip(tr("This textedit allows using variables."));
    txtLineEdit->setText("0");
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(evaluate()));

    lblValue = new QLabel(this);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);

    setLayout(layout);

    QSettings settings;
    m_showResult = settings.value("General/LineEditValueShowResult", false).value<bool>();

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
    Value val = value();
    QPalette palette = lblValue->palette();
    if (val.evaluate(quiet))
    {
        m_number = val.number;
        lblValue->setText(QString("%1").arg(m_number, 0, 'g', 3));
        palette.setColor(QPalette::WindowText, QApplication::palette().color(QPalette::WindowText));
        lblValue->setPalette(palette);
        lblValue->setVisible(m_showResult);
        return true;
    }
    else
    {
        lblValue->setText(tr("error"));
        palette.setColor(QPalette::WindowText, QColor(Qt::red));
        lblValue->setPalette(palette);
        lblValue->setVisible(true);
        setFocus();
        return false;
    }
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
    setMinimumSize(700, 450);

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

        QImage image(1024, 768, QImage::Format_ARGB32);
        print(image);
        image.save(fileNameTemp, "PNG");
    }
}

void Chart::setData(double *xval, double *yval, int count)
{
    const bool doReplot = autoReplot();
    setAutoReplot(false);

    m_curve->setData(xval, yval, count);

    setAutoReplot(doReplot);

    replot();
}

// ***********************************************************************************************************

CommandDialog::CommandDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Command"));
    setWindowIcon(icon("system-run"));   

    cmbCommand = new QComboBox(this);
    cmbCommand->setEditable(true);
    cmbCommand->setMinimumWidth(350);

    // completer
    QSettings settings;
    QStringList list;
    list = settings.value("CommandDialog/RecentCommands").value<QStringList>();
    /*
    QStringListModel *model = new QStringListModel();
    model->setStringList(list);

    completer = new QCompleter(model, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    cmbCommand->setCompleter(completer);
    */
    cmbCommand->addItem("", "");
    cmbCommand->addItems(list);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutCommand = new QHBoxLayout();
    layoutCommand->addWidget(new QLabel(tr("Enter command:")));
    layoutCommand->addWidget(cmbCommand);
    layoutCommand->addStretch();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutCommand);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

CommandDialog::~CommandDialog()
{
    delete cmbCommand;
    // delete completer;
}

void CommandDialog::doAccept()
{
    if (!command().isEmpty())
    {
        QSettings settings;
        QStringList list;
        list = settings.value("CommandDialog/RecentCommands").value<QStringList>();
        list.insert(0, command());

        // remove last item (over 30), empty strings and duplicates
        list.removeAll("");
        list.removeDuplicates();
        while (list.count() > 30)
            list.removeAt(0);

        settings.setValue("CommandDialog/RecentCommands", list);
    }

    accept();
}


// ****************************************************************************************************

FileBrowser::FileBrowser(QWidget *parent) : QListWidget(parent)
{
    setNameFilter("");
    setDir(QDir::currentPath());
    setContextMenuPolicy(Qt::CustomContextMenu);

    actCreateDirectory = new QAction(icon("file-folder"), tr("&Create directory"), this);
    connect(actCreateDirectory, SIGNAL(triggered()), this, SLOT(createDir()));

    actCreateFile = new QAction(icon("file-text"), tr("&Create file"), this);
    connect(actCreateFile, SIGNAL(triggered()), this, SLOT(createFile()));

    actDelete = new QAction(icon(""), tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteObject()));

    mnuContext = new QMenu(this);
    mnuContext->addAction(actCreateDirectory);
    mnuContext->addAction(actCreateFile);
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
            QFileInfo fileInfo(path + "/" + item);
            if (QDir(path + "/" + item).exists())
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
    // emit fileItemActivated(m_basePath + "/" + item->text());
}

void FileBrowser::doFileItemDoubleClick(QListWidgetItem *item)
{
    QString path = m_basePath + "/" + item->text();
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

    QListWidgetItem *item = itemAt(point);
    if (item)
    {
        if (item->text() != "..")
            actDelete->setEnabled(item);
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
    setDir(m_basePath + "/" + str);
}

void FileBrowser::createFile(const QString &fileName)
{
    QString str;
    if (fileName.isEmpty())
       str = QInputDialog::getText(QApplication::activeWindow(), tr("Create file"), tr("Enter file name"));
    else
       str = fileName;

    if (str.isEmpty()) return;

    QFile file(m_basePath + "/" + str);
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

void FileBrowser::deleteObject(const QString &name)
{
    QString str;
    if (name.isEmpty())
        str = m_basePath + "/" + currentItem()->text();
    else
        str = name;

    cout << str.toStdString() << endl;

    QDir dir(str);
    if (dir.exists())
    {
        deleteDir(str);
    }
    else
    {
        deleteFile(str);
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
