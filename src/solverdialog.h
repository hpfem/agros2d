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

class SolverThread : public QThread
{
    Q_OBJECT
public:
    SolverThread(QObject *parent = 0);
    ~SolverThread();

    void run();    
    inline void setFileNameOrig(const QString &fileNameOrig) { m_fileNameOrig = fileNameOrig; }
    inline void setMode(SolverMode mode) { m_mode = mode; }

    void showMessage(const QString &msg, bool isError);
    inline void showProgress(int percent) { emit updateProgress(percent); }
    inline bool isCanceled() { return m_isCanceled; }
    void cancel();

public slots:
    void doMeshTriangleCreated(int exitCode);
    void doStarted();

signals:
    void updateProgress(int percent);
    void meshed();
    void solved();
    void message(const QString &message, bool isError);

private:
    QString m_fileNameOrig;
    SolverMode m_mode;
    bool m_isCanceled;
    QMutex mutex;
    QWaitCondition condition;

    void runMesh();
    void runSolver();

    bool writeToTriangle();
    bool triangleToHermes2D();
};

class SolverDialog : public QDialog
{
    Q_OBJECT

    public slots:
    void doShowMessage(const QString &message, bool isError);
    void doSolved();

signals:
    void solved();

public:
    SolverDialog(QWidget *parent);
    ~SolverDialog();

    inline void setFileNameOrig(const QString &fileNameOrig) { thread->setFileNameOrig(fileNameOrig); }
    inline void setMode(SolverMode mode) { thread->setMode(mode); }
    int solve();

private slots:
    void doCancel();
    void doClose();

private:
    SolverThread *thread;

    QLabel *lblMessage;
    QProgressBar *progressBar;
    QTextEdit *lstMessage;
    QPushButton *btnCancel;
    QPushButton *btnClose;

    void createControls();
};

#endif //SCENEHERMES_H
