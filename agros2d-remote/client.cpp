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

#include "client.h"

const QString OK_STRING = "\nOK\n";

Client::Client(const QString &IP, int port) : m_IP(IP), m_port(port)
{
    m_tcpSocket = new QTcpSocket(this);

    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readResult()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

Client::~Client()
{
    delete m_tcpSocket;
    delete m_networkSession;
}

bool Client::run(const QString &command)
{
    m_tcpSocket->abort();
    m_tcpSocket->connectToHost(m_IP, m_port);
    if (m_tcpSocket->waitForConnected(1000))
    {
        m_tcpSocket->write(QString(command).toLatin1());
        return true;
    }
    else
    {
        return false;
    }
}

void Client::readResult()
{
    if (m_tcpSocket->bytesAvailable() > 0)
    {
        QString out = QString(m_tcpSocket->readAll());
        m_result = out.left(out.indexOf(OK_STRING));
    }
    m_tcpSocket->close();
}

void Client::connected()
{
    m_result = "";
}

void Client::disconnected()
{
    if (!m_result.isEmpty())
        qDebug() << m_result;

    exit(0);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::HostNotFoundError:
        cout << tr("Client error: The host was not found.").toStdString() << endl;
        break;
    case QAbstractSocket::ConnectionRefusedError:
        cout << tr("Client error: The connection was refused by the peer. Make sure the agros2d-server server is running.").toStdString() << endl;
        break;
    default:
        cout << tr("Client error: %1").arg(m_tcpSocket->errorString()).toStdString() << endl;
    }
    exit(0);
}

