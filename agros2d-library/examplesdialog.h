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

#ifndef EXAMPLESDIALOG_H
#define EXAMPLESDIALOG_H

#include "util.h"

class ExamplesDialog : public QDialog
{
    Q_OBJECT
public:
    ExamplesDialog(QWidget *parent);
    ~ExamplesDialog();

    int showDialog();
    inline QString selectedFilename() { return m_selectedFilename; }
    inline QString selectedFormFilename() { return m_selectedFormFilename; }

private slots:
    void doAccept();
    void doReject();

    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void linkClicked(const QUrl &url);

private:
    QWebView *webView;
    QString m_cascadeStyleSheet;

    QTreeWidget *lstProblems;
    QString m_selectedFilename;
    QString m_selectedFormFilename;

    QDialogButtonBox *buttonBox;

    void readProblems();
    int readProblems(QDir dir, QTreeWidgetItem *parentItem);
    void problemInfo(const QString &fileName);
    QList<QIcon> problemIcons(const QString &fileName);
};

#endif // EXAMPLESDIALOG_H
