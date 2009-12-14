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

#ifndef SCENEHERMES_H
#define SCENEHERMES_H

#include <QDomDocument>

#include "util.h"

class SolverDialog;

class Solution;
class Linearizer;
class Vectorizer;
class Orderizer;
class Mesh;

struct SolutionArray
{
    double time;
    double adaptiveError;
    int adaptiveSteps;

    Solution *sln;
    Orderizer *order;

    SolutionArray();
    ~SolutionArray();

    void load(QDomElement *element);
    void save(QDomDocument *doc, QDomElement *element);
};

class SolverDialog : public QDialog
{
    Q_OBJECT

public slots:
    void showMessage(const QString &message, bool isError);
    void showProgress(int index);

signals:
    void meshed();
    void solved();

public:
    SolverDialog(QWidget *parent);
    ~SolverDialog();

    inline void setFileNameOrig(const QString &fileNameOrig) { m_fileNameOrig = fileNameOrig; }
    inline void setMode(SolverMode mode) { m_mode = mode; }

    inline bool isCanceled() { return m_isCanceled; }

    int solve();
    void cancel();

private slots:
    void doCancel();
    void doClose();
    void doStart();
    void doMeshTriangleCreated(int exitCode);
    void doFinished();

private:
    QString m_fileNameOrig;
    SolverMode m_mode;
    bool m_isCanceled;

    QLabel *lblMessage;
    QProgressBar *progressBar;
    QTextEdit *lstMessage;
    QPushButton *btnCancel;
    QPushButton *btnClose;

    void createControls();

    void runMesh();
    void runSolver();

    bool writeToTriangle();
    bool triangleToHermes2D();
};

#endif //SCENEHERMES_H
