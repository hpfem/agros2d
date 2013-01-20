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

class LineEditDouble;

class MaterialEditDialog : public QDialog
{
    Q_OBJECT
public:
    MaterialEditDialog(const QString &fileName, QWidget *parent = 0);
    ~MaterialEditDialog();

    int showDialog();

protected:
    void createControls();
    void readMaterial();
    bool writeMaterial();

private:
    struct Property
    {
        QLineEdit *txtName;
        QLineEdit *txtShortname;
        QLineEdit *txtUnit;
        QLineEdit *txtSource;
        QLineEdit *txtDependenceShortname;
        QLineEdit *txtDependenceUnit;

        LineEditDouble *txtConstant;

        QTextEdit *txtTableKeys;
        QTextEdit *txtTableValues;

        QTextEdit *txtFunction;
        LineEditDouble *txtFunctionFrom;
        LineEditDouble *txtFunctionTo;

        QTabWidget *tabTableAndFunction;
    };

    QString m_fileName;

    QLineEdit *txtName;
    QLineEdit *txtDescription;
    QList<Property> propertiesUI;

    QTabWidget *tabProperties;

    MaterialEditDialog::Property addPropertyUI(const QString &name);

private slots:
    void doAccept();
    void addProperty();
    void closeProperty(int index);
};

class MaterialBrowserDialog : public QDialog
{
    Q_OBJECT
public:
    MaterialBrowserDialog(QWidget *parent = 0);
    ~MaterialBrowserDialog();

    int showDialog(bool select = false);

    inline QList<double> x() const { return m_selected_x; }
    inline QList<double> y() const { return m_selected_y; }

protected:
    void readMaterials();
    void readMaterials(QDir dir, QTreeWidgetItem *parentItem);
    void materialInfo(const QString &fileName);

private:
    QWebView *webView;
    QTreeWidget *trvMaterial;
    QPushButton *btnEdit;
    QString m_selectedFilename;

    QList<double> m_selected_x;
    QList<double> m_selected_y;

    bool m_select;

    void functionValues(const QString &function, double from, double to, int count, QList<double> *keys, QList<double> *values);

private slots:
    void doItemSelected(QTreeWidgetItem *item, int column);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void linkClicked(const QUrl &url);
    void doEdit();
};

#endif // MATERIALBROWSERDIALOG_H
