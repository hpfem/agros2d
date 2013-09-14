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

#include "../resources_source/classes/material_xml.h"

class LineEditDouble;
class QCustomPlot;
class ScriptEditor;

class MaterialEditDialog : public QDialog
{
    Q_OBJECT
public:
    MaterialEditDialog(const QString &fileName, QWidget *parent = 0);
    ~MaterialEditDialog();

    int showDialog();
    inline QString fileName() { return m_fileName; }

protected:
    void createControls();
    void readMaterial();
    bool writeMaterial();

    enum NonlinearityType
    {
        Function = 1,
        Table = 2
    };

    inline QString nonlinearityTypeToStringKey(NonlinearityType type)
    {
        if (type == MaterialEditDialog::Table)
            return "table";
        else
            return "function";
    }

    inline NonlinearityType nonlinearityTypeFromStringKey(const QString &type)
    {
        if (type == "table")
            return MaterialEditDialog::Table;
        else
            return MaterialEditDialog::Function;
    }


private:
    QString m_fileName;

    QLineEdit *txtName;
    QLineEdit *txtDescription;

    QListWidget *lstProperties;
    QList<XMLMaterial::property> m_properties;

    // properties
    QWidget *propertyGUI;

    QLineEdit *txtPropertyName;
    QLineEdit *txtPropertyShortname;
    QLineEdit *txtPropertyUnit;
    QLineEdit *txtPropertySource;
    QLineEdit *txtPropertyIndependentVariableShortname;
    QLineEdit *txtPropertyIndependentVariableUnit;
    QComboBox *cmbPropertyNonlinearityType;
    QLabel *lblPropertyFunction;

    LineEditDouble *txtPropertyConstant;

    QPlainTextEdit *txtPropertyTableKeys;
    QPlainTextEdit *txtPropertyTableValues;

    ScriptEditor *txtPropertyFunction;
    LineEditDouble *txtPropertyFunctionFrom;
    LineEditDouble *txtPropertyFunctionTo;

    QGroupBox *widNonlinearTable;
    QGroupBox *widNonlinearFunction;
    QGroupBox *widChartNonlinear;
    QStackedLayout *layoutNonlinearType;
    QCustomPlot *chartNonlinear;

    QPushButton *btnDeleteProperty;

    QWidget *createPropertyGUI();

    void readProperty(XMLMaterial::property prop = XMLMaterial::property("", "", "", "", "none", "", ""));
    XMLMaterial::property writeProperty();

private slots:
    void doAccept();
    void addProperty(const QString &name = "", const QString &shortname = "", const QString &unit = "", const QString &nonlinearityKind = "",
                     const QString &indepedentShortname = "", const QString &indepedentUnit = "");
    // TODO: more general
    inline void addPropertyThermalConductivity() { addProperty("Thermal conductivity", "<i>&lambda;</i>", "W/m.K", "function", "<i>T</i>", "K"); }
    inline void addPropertySpecificHeat() { addProperty("Specific heat", "<i>c</i><sub>p</sub>", "J/kg.K", "function", "<i>T</i>", "K"); }
    inline void addPropertyDensity() { addProperty("Density", "<i>&rho;</i>", "kg/m<sup>3</sup>", "function", "<i>T</i>", "K"); }
    inline void addPropertyMagneticPermeability() { addProperty("Magnetic permeability", "<i>&mu;</i><sub>r</sub>", "-", "function", "<i>B</i>", "T"); }

    void deleteProperty();

    void doPropertyChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void doNonlinearDependenceChanged(int index);

    void setFunctionLabel();
    void drawChart();
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
    inline double constant() const { return m_selected_constant; }

protected:
    void readMaterials();
    void readMaterials(QDir dir, QTreeWidgetItem *parentItem);
    void materialInfo(const QString &fileName);

private:
    QWebView *webView;
    QTreeWidget *trvMaterial;
    QPushButton *btnNew;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QString m_selectedFilename;
    QString m_cascadeStyleSheet;

    QList<double> m_selected_x;
    QList<double> m_selected_y;
    double m_selected_constant;

    bool m_select;

private slots:
    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void linkClicked(const QUrl &url);
    void doNew();
    void doEdit();
    void doDelete();
};

#endif // MATERIALBROWSERDIALOG_H
