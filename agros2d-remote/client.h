// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

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
    Client(const QString &IP, int port);
    ~Client();

    bool run(const QString &command);

private slots:   
    void connected();
    void readResult();
    void disconnected();

    void displayError(QAbstractSocket::SocketError socketError);

private:
    QString m_IP;
    int m_port;

    QString m_result;

    QTcpSocket *m_tcpSocket;
    QNetworkSession *m_networkSession;
};

#endif
