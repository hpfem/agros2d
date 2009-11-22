#include <QCoreApplication>

#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Client *client;
    QStringList args = QCoreApplication::arguments();

    if (args.count() == 2)
    {
        if (args[1] == "--help" || args[1] == "/help" || args[1] == "-help")
        {
        }
        else
        {
            // run command
            client = new Client;
            client->run(args[1]);
            return a.exec();
        }
    }

    if (args.count() == 3)
    {
        if (args[1] == "--script" || args[1] == "/script" || args[1] == "-script")
        {
            QFile file(args[2]);
            if (file.open(QFile::ReadOnly | QFile::Text))
            {
                // run script
                client = new Client;
                client->run(file.readAll());
            }
            file.close();
            return a.exec();
        }
    }

    // show help
    cout << "agros2d-remote [command | --script fileName]" << endl;
    a.exit(0);
    return 0;
}
