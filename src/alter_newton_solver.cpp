#include "util.h"
#include "scene.h"
#include "scenesolution.h"
#include "alter_newton_solver.h"
#include "progressdialog.h"

void newton_log(char *str)
{
    Util::scene()->sceneSolution()->progressDialog()->showMessage(str, false, 0);

    // hack
    QString message = QString::fromAscii(str);
    int pos = message.indexOf("residual norm: ");
    if (pos != -1)
    {
        pos += 15;
        double error = message.right(message.length() - pos).toDouble();
        if (error > 0.0)
        {
            dynamic_cast<ProgressItemSolve *>(Util::scene()->sceneSolution()->progressDialog()->currentProgressItem())->
                    addNonlinearError(error);
        }
    }
}
