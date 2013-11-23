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

#ifndef PYTHON_UNITTESTS_H
#define PYTHON_UNITTESTS_H

#include "pythonlab/pythonconsole.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"

#include "util.h"
#include "util/global.h"

#include "../../resources_source/classes/test_xml.h"

class LogWidget;

class AGROS_LIBRARY_API UnitTestsWidget : public QDialog
{
    Q_OBJECT
public:
    UnitTestsWidget(QWidget *parent = 0);
    ~UnitTestsWidget();

protected:

private:
    QTreeWidget *trvTests;
    QSplitter *splitter;
    QPushButton *btnScenarios;
    QPushButton *btnRunTests;
    QPushButton *btnStopTest;
    QPushButton *btnUncheckTests;
    QDialogButtonBox *buttonBox;

    LogWidget *logWidget;

    XMLTest::test m_test;

    QWebView *webView;
    QString m_cascadeStyleSheet;

    bool m_isAborted;

    QList<QTreeWidgetItem *> availableTests();

    void readTestFromDisk(const QString& fileName);

    void readTestsFromSuite();
    void readScenariosFromSuite();
    void runTestFromSuite(const QString &module, const QString &cls);

    void showInfoTests(const QString &testID = "");

    void saveTestsSettings();

private slots:
    void doAccept();
    void doReject();

    void doContextMenu(const QPoint &pos);
    void showInfoTest(QAction *act);

    void uncheckTests();
    void runTestsFromSuite();   
    void stopTest();
    void setEnabledControls(bool state);
    void readTestsSettingsFromScenario(QAction *action);
};



#endif // PYTHON_UNITTESTS_H
