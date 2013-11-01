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

#include "remotecontrol.h"

#include "pythonengine_agros.h"

const QString OK_STRING = "\nOK\n";

ScriptEngineRemote::ScriptEngineRemote() : m_tcpSocket(NULL)
{  
    if (!listen())
    {
        qWarning() << tr("Error: Unable to start the server (agros2d-server): %1.").arg(errorString());
        return;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    qDebug() << QString("The server '%1' is running on IP: %2, port: %3").arg(serverName()).arg(ipAddress).arg(serverPort());

    connect(this, SIGNAL(newConnection()), this, SLOT(connected()));
}

ScriptEngineRemote::~ScriptEngineRemote()
{
}

void ScriptEngineRemote::connected()
{
    if (m_tcpSocket)
    {
        qDebug() << tr("Server is busy.");
        return;
    }

    qDebug() << tr("Client connected");

    m_tcpSocket = nextPendingConnection();
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readCommand()));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));
}

void ScriptEngineRemote::readCommand()
{
    if (m_tcpSocket->bytesAvailable() > 0)
    {
        m_command = QString(m_tcpSocket->readAll());
        qDebug() << tr("Command: %1").arg(m_command);

        bool successful = currentPythonEngineAgros()->runScript(m_command);

        if (successful)
        {
            if (!m_stdout.trimmed().isEmpty())
            {
                qDebug() << tr("Stdout: %1").arg(m_stdout.trimmed());
                m_tcpSocket->write((m_stdout.trimmed() + OK_STRING).toLatin1());
            }
            else
            {
                m_tcpSocket->write(OK_STRING.toLatin1());
            }
        }
        else
        {
            ErrorResult result = currentPythonEngineAgros()->parseError();
            qDebug() << tr("Error: %1").arg(result.error().trimmed());
            m_tcpSocket->write((result.error().trimmed() + OK_STRING).toLatin1());
        }
    }
    else
    {
        m_tcpSocket->write(OK_STRING.toLatin1());
    }
    m_tcpSocket->close();
}

void ScriptEngineRemote::disconnected()
{
    m_tcpSocket = NULL;
    m_stdout = "";

    disconnect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    disconnect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    qDebug() << tr("Client disconnected");
}

void ScriptEngineRemote::displayError(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        qWarning() << tr("Server error: The host was not found.");
        break;
    case QLocalSocket::ConnectionRefusedError:
        qWarning() << tr("Server error: The connection was refused by the peer. Make sure the agros2d-client server is running.");
        break;
    default:
        qWarning() << tr("Server error: The following error occurred: %1.").arg(m_tcpSocket->errorString());
    }
}

QString ScriptEngineRemote::serverName()
{
    return QString("agros2d-server-%1").arg(QString::number(QCoreApplication::applicationPid()));
}

void ScriptEngineRemote::stdOut(const QString &str)
{
    m_stdout += str;
}

void ScriptEngineRemote::stdHtml(const QString &str)
{
    m_stdout += str;
}
