#ifndef SCENEHERMES_H
#define SCENEHERMES_H

#include <QtGui/QProgressBar>
#include <QThread>
#include <QProcess>
#include <QFileInfo>

#include "util.h"
#include "hermes2d.h"
#include "scene.h"
#include "sceneview.h"

class Solution;
class Linearizer;
class Vectorizer;
class Mesh;

class Scene;

struct SolutionArray
{
    Orderizer *order1;
    Space *space1;
    Space *space2;
    Solution *sln1;
    Solution *sln2;

    SolutionArray() { this->sln1 = NULL; this->sln2 = NULL; }
    SolutionArray(Solution *sln1, Space *space1) { this->sln1 = sln1; this->sln2 = NULL; this->space1 = space1; this->space2 = NULL; }
    SolutionArray(Solution *sln1, Orderizer *order1) { this->sln1 = sln1; this->sln2 = NULL; this->order1 = order1; this->space1 = NULL; this->space2 = NULL; }
    SolutionArray(Solution *sln1, Space *space1, Solution *sln2, Space *space2) { this->sln1 = sln1; this->sln2 = sln2; this->space1 = space1; this->space2 = space2;  }
};

class ThreadSolver : public QThread
{
    Q_OBJECT

signals:
    void message(QString);
    void updateProgress(int percent);

public:
    ThreadSolver(Scene *scene);
    ~ThreadSolver();

    inline ThreadSolverMode mode() { return m_mode; }
    void setMode(ThreadSolverMode mode) { this->m_mode = mode; }
    inline QString errorMessage() { return m_errorMessage; }

protected:
    QProcess *processTriangle;

    ThreadSolverMode m_mode;
    Scene *m_scene;
    QString m_errorMessage;

    void runMesh();
    void runSolver();

    void run();

private slots:
    void doMeshTriangleCreated(int exitCode);
};

class SolverDialog : public QDialog
{
    Q_OBJECT

public slots:
    void doShowMessage(const QString &message);

public:
    SolverDialog(Scene *scene, QWidget *parent);
    ~SolverDialog();

    int showDialog();

private slots:
    void doSolved();
    void doReject();

private:
    Scene *m_scene;

    QLabel *lblMessage;
    QProgressBar *progressBar;

    void createControls();
};

#endif //SCENEHERMES_H
