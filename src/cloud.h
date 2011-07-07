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

#ifndef CLOUD_H
#define CLOUD_H

#include "util.h"

class CloudDialogContent : public QDialog
{
    Q_OBJECT
public:
    CloudDialogContent(QWidget *parent = 0);
    ~CloudDialogContent();

    int showDialog();
    void createControls();
    void readFromCloudContent();
    void readFromCloud(int ID);

    inline QString fileName() { return m_fileName; }

private slots:
    void doAccept();
    void doReject();
    void httpContentFinished();
    void httpFileFinished();

private:
    QString m_fileName;
    QTreeWidget *trvProject;

    QNetworkAccessManager networkAccessManager;
    QNetworkReply *networkReply;
};

class CloudDialogSave : public QDialog
{
    Q_OBJECT
public:
    CloudDialogSave(QWidget *parent = 0);
    ~CloudDialogSave();

    int showDialog();
    void createControls();
    void readFromCloudContent();
    void saveToCloud();

private slots:
    void doAccept();
    void doReject();
    void httpContentFinished();
    void httpFileFinished();
    void doDocumentChanged(int index);

private:
    QNetworkAccessManager networkAccessManager;
    QNetworkReply *networkReply;

    QComboBox *cmbDocument;
    QLineEdit *txtName;
    QComboBox *cmbName;
    QLineEdit *txtAuthor;
    QLineEdit *txtAffiliation;
};

#endif // CLOUD_H
