#include <QCoreApplication>

#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Client *client;
    QStringList args = QCoreApplication::arguments();

    if (args.count() == 4)
    {
        if (args[1] == "--help" || args[1] == "/help" || args[1] == "-help")
        {
        }
        else
        {
            // run command
            client = new Client(QString(args[1]), QString(args[2]).toInt());
            client->run(args[3]);
            return a.exec();
        }
    }

    if (args.count() == 5)
    {
        if (args[2] == "--script" || args[2] == "/script" || args[2] == "-script")
        {
            QFile file(args[3]);
            if (file.open(QFile::ReadOnly | QFile::Text))
            {
                // run script
                client = new Client(QString(args[1]), QString(args[2]).toInt());
                // set client name
                client->run(file.readAll());
            }
            file.close();
            return a.exec();
        }
    }

    // show help
    cout << "agros2d-remote IP port [command | --script fileName]" << endl;
    a.exit(0);
    return 0;
}
