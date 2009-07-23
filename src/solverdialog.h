#ifndef SCENEHERMES_H
#define SCENEHERMES_H

#include <QtGui/QProgressBar>
#include <QtGui/QListWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QLabel>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QVBoxLayout>

#include "util.h"

class Solution;
class Linearizer;
class Vectorizer;
class Orderizer;
class Mesh;

struct SolutionArray
{
    double adaptiveError;
    int adaptiveSteps;

    Solution *sln1;
    Solution *sln2;
    Orderizer *order1;
    Orderizer *order2;

    SolutionArray()
    {
        this->sln1 = NULL;
        this->sln2 = NULL;
        this->order1 = NULL;
        this->order2 = NULL;

        this->adaptiveError = 100.0;
        this->adaptiveSteps = 0;
    }

    ~SolutionArray()
    {
        clear();
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

class SolverDialog : public QDialog
{
    Q_OBJECT

signals:
    void message(const QString &message);
    void updateProgress(int percent);
    void solved();

public slots:
    void doShowMessage(const QString &message);

public:
    SolverDialog(QWidget *parent);
    ~SolverDialog();

    void setMode(SolverMode mode) { this->m_mode = mode; }    
    void solve();

private slots:
    void doAccept();

private:
    SolverMode m_mode;
    QString m_errorMessage;

    QLabel *lblMessage;
    QProgressBar *progressBar;
    QTextEdit *lstMessage;

    void runMesh();
    void runSolver();

    void createControls();

private slots:
    void doMeshTriangleCreated(int exitCode);
};

#endif //SCENEHERMES_H
