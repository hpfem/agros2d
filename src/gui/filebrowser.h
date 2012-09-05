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

#ifndef GUI_FILEBROWSER_H
#define GUI_FILEBROWSER_H

#include "util.h"

class FileBrowser : public QListWidget
{
    Q_OBJECT

public:
    FileBrowser(QWidget *parent = 0);
    void setDir(const QString &path);
    void setNameFilter(const QString &nameFilter);
    QString basePath();
    void refresh();

public slots:
    void createDir(const QString &dirName = "");
    void createFile(const QString &fileName = "");
    void deleteDir(const QString &dirName = "");
    void deleteFile(const QString &fileName = "");
    void deleteObject(const QString &name = "");
    void renameDir(const QString &dirName);
    void renameFile(const QString &fileName);
    void renameObject(const QString &name = "");

signals:
    void fileItemActivated(const QString &path);
    void fileItemDoubleClick(const QString &path);
    void directoryChanged(const QString &path);

private slots:
    void doFileItemActivated(QListWidgetItem *item);
    void doFileItemDoubleClick(QListWidgetItem *item);
    void doContextMenu(const QPoint &point);

private:
    QString m_nameFilter;
    QString m_basePath;

    QAction *actCreateDirectory;
    QAction *actCreateFile;
    QAction *actRename;
    QAction *actDelete;

    QMenu *mnuContext;
};

#endif // GUI_FILEBROWSER_H
