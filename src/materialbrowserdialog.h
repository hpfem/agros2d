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

#ifndef MATERIALBROWSERDIALOG_H
#define MATERIALBROWSERDIALOG_H

#include "util.h"

/*
class ChartBasic;

class MaterialBrowserDialog : public QDialog
{
    Q_OBJECT
public:
    MaterialBrowserDialog(QWidget *parent = 0);
    ~MaterialBrowserDialog();

    int showDialog(bool select = false);

    inline QList<double> x() const { return m_x; }
    inline QList<double> y() const { return m_y; }

protected:
    void createControls();
    void readMaterials();

private:
    QTreeWidget *trvMaterial;
    QTreeWidget *trvProperty;
    QLabel *lblMaterial;
    QLabel *lblProperty;
    QLabel *lblShortname;
    QLabel *lblDependenceShortname;
    QLabel *lblSource;
    QLabel *lblValue;
    ChartBasic *chartValue;

    QDialogButtonBox *buttonBox;

    QList<double> m_x;
    QList<double> m_y;

    bool m_select;

private slots:
    void doMaterialSelected(QTreeWidgetItem *item, int role);
    void doMaterialSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doPropertySelected(QTreeWidgetItem *item, int role);
    void doPropertySelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void doAccept();
};

*/
class MaterialBrowserDialog : public QDialog
{
    Q_OBJECT
public:
    MaterialBrowserDialog(QWidget *parent = 0);
    ~MaterialBrowserDialog();

    int showDialog(bool select = false);

    inline QList<double> x() const { return m_x; }
    inline QList<double> y() const { return m_y; }

protected:
    void readMaterials();
    void readMaterials(QDir dir, QTreeWidgetItem *parentItem);
    void materialInfo(const QString &fileName);

private:
    QWebView *webView;
    QTreeWidget *trvMaterial;
    QString m_selectedFilename;

    QDialogButtonBox *buttonBox;

    QList<double> m_x;
    QList<double> m_y;

    bool m_select;

private slots:
    void doAccept();
    void doReject();

    void doItemSelected(QTreeWidgetItem *item, int column);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);
};

#endif // MATERIALBROWSERDIALOG_H
