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
