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
        sln1 = NULL;
        sln2 = NULL;
        order1 = NULL;
        order2 = NULL;

        adaptiveError = 100.0;
        adaptiveSteps = 0;
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
