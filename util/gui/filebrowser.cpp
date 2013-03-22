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

#include "filebrowser.h"

#include "../util.h"

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
