#include "util.h"
#include "scene.h"
#include "scenesolution.h"
#include "alter_newton_solver.h"
#include "hermes2d/problem.h"
#include "progressdialog.h"

void newton_log(char *str)
{
    cout << "newton log TODO uncoment " << str << endl;
//    Util::problem()->progressDialog()->showMessage(str, false, 0);

//    // hack
//    QString message = QString::fromAscii(str);
//    int pos = message.indexOf("residual norm: ");
//    if (pos != -1)
//    {
//        pos += 15;
//        double error = message.right(message.length() - pos).toDouble();
//        if (error > 0.0)
//        {
//            dynamic_cast<ProgressItemSolve *>(Util::problem()->progressDialog()->currentProgressItem())->
//                    addNonlinearError(error);
//        }
//    }
}
