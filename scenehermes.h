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
    double adaptiveError;
    int adaptiveSteps;

    Solution *sln1;
    Solution *sln2;
    Orderizer *order1;
    Orderizer *order2;

    SolutionArray() {
        this->sln1 = NULL;
        this->sln2 = NULL;
        this->order1 = NULL;
        this->order2 = NULL;

        this->adaptiveError = 100.0;
        this->adaptiveSteps = 0;
    }

    void clear()
    {
        if (sln1 != NULL)
        {
            delete sln1;
            sln1 = NULL;
        }
        if (sln2 != NULL)
        {
            delete sln2;
            sln2 = NULL;
        }
        if (order1 != NULL)
        {
            delete order1;
            order1 = NULL;
        }
        if (order2 != NULL)
        {
            delete order2;
            order2 = NULL;
        }
    }
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
