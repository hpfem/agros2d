#ifndef CLIENT_H
#define CLIENT_H

#include <QtNetwork>
#include <QLocalSocket>

#include <iostream>

using namespace std;

class QLocalSocket;

class Client : QObject
{
    Q_OBJECT

public:
    Client();
    ~Client();

    void run(const QString &command);

private slots:   
    void connected();
    void readResult();
    void disconnected();

    void displayError(QLocalSocket::LocalSocketError socketError);

private:
    QString result;

    QLocalServer *m_server;
    QLocalSocket *m_server_socket;
    QLocalSocket *m_client_socket;
};

#endif
