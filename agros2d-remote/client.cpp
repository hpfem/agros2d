#include "client.h"

Client::Client(int pid) : m_serverName(QString("agros2d-server-%1").arg(pid))
{
    // server
    m_server = new QLocalServer();
    QLocalServer::removeServer(clientName());
    if (!m_server->listen(clientName()))
    {
        cout << tr("Error: Unable to start the server (agros2d-client): %1.").arg(m_server->errorString()).toStdString() << endl;
        return;
    }
    connect(m_server, SIGNAL(newConnection()), this, SLOT(connected()));
}

Client::~Client()
{
    delete m_server;
    delete m_client_socket;
}

void Client::run(const QString &command)
{
    QByteArray block;
    if (!command.isEmpty())
    {
        m_client_socket = new QLocalSocket();
        connect(m_client_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

        m_client_socket->connectToServer(serverName());
        if (m_client_socket->waitForConnected(1000))
        {
            QTextStream out(m_client_socket);
            out << command;
            out.flush();
            m_client_socket->waitForBytesWritten();
        }
        else
        {
            displayError(QLocalSocket::ConnectionRefusedError);
        }

        delete m_client_socket;
    }
    else
    {
        exit(0);
    }
}

void Client::connected()
{
    result = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readResult()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void Client::readResult()
{
    QTextStream in(m_server_socket);
    result = in.readAll();
}

void Client::disconnected()
{
    m_server_socket->deleteLater();
    cout << result.toStdString() << endl;
    exit(0);
}

void Client::displayError(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        cout << tr("Client error: The host was not found.").toStdString() << endl;
        break;
    case QLocalSocket::ConnectionRefusedError:
        cout << tr("Client error: The connection was refused by the peer. Make sure the agros2d-server server is running.").toStdString() << endl;
        break;
    default:
        cout << tr("Client error: The following error occurred: %1.").arg(m_client_socket->errorString()).toStdString() << endl;
    }
    exit(0);
}

QString Client::clientName()
{
    return QString("agros2d-client-%1").arg(QString::number(QCoreApplication::applicationPid()));
}

QString Client::serverName()
{
    return m_serverName;
}
