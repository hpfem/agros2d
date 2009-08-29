#include "client.h"

Client::Client()
{
    // client
    m_client_socket = new QLocalSocket();
    connect(m_client_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

    // server
    m_server = new QLocalServer();
    QLocalServer::removeServer("agros2d-client");
    if (!m_server->listen("agros2d-client"))
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
        QDataStream out(&block, QIODevice::ReadWrite);

        out.setVersion(QDataStream::Qt_4_5);
        out << (quint16) 0;
        out << command;
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));

        m_client_socket->abort();
        m_client_socket->connectToServer("agros2d-server");
        m_client_socket->write(block);
        m_client_socket->flush();
    }
    else
    {
        exit(0);
    }
}

void Client::connected()
{
    blockSize = 0;
    result = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readResult()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    while (m_server_socket->waitForReadyRead(10)) {}
    m_server_socket->disconnectFromServer();
}

void Client::readResult()
{
    QDataStream in(m_server_socket);
    in.setVersion(QDataStream::Qt_4_5);

    if (blockSize == 0)
    {
        if (m_server_socket->bytesAvailable() < (int) sizeof(quint16))
            return;
        in >> blockSize;
    }

    if (in.atEnd())
        return;

    in >> result;
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
        cout << tr("Error: The host was not found.").toStdString() << endl;
        break;
    case QLocalSocket::ConnectionRefusedError:
        cout << tr("Error: The connection was refused by the peer. Make sure the agros2d-server server is running.").toStdString() << endl;
        break;
    case QLocalSocket::PeerClosedError:
        break;
    default:
        cout << tr("Error: The following error occurred: %1.").arg(m_client_socket->errorString()).toStdString() << endl;
    }

    exit(0);
}
