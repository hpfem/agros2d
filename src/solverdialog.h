#ifndef SCENEHERMES_H
#define SCENEHERMES_H

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
    void message(const QString &message, bool isError);
    void updateProgress(int percent);
    void meshed();
    void solved();

public slots:
    void doShowMessage(const QString &message, bool isError);

public:
    SolverDialog(QWidget *parent);
    ~SolverDialog();

    void setFileNameOrig(const QString &fileNameOrig) { m_fileNameOrig = fileNameOrig; }
    void setMode(SolverMode mode) { m_mode = mode; }
    void solve();
    inline bool isCanceled() { return m_isCanceled; }

private slots:
    void doCancel();

private:
    bool m_isCanceled;
    SolverMode m_mode;
    QString m_fileNameOrig;

    QLabel *lblMessage;
    QProgressBar *progressBar;
    QTextEdit *lstMessage;
    QPushButton *btnCancel;

    void runMesh();
    void runSolver();

    bool writeToTriangle();
    bool triangleToHermes2D();

    void createControls();

private slots:
    void doMeshTriangleCreated(int exitCode);
};

#endif //SCENEHERMES_H
