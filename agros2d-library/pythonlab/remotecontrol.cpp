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

ScriptEngineRemote::ScriptEngineRemote()
{
    // server
    m_server = new QLocalServer();
    QLocalServer::removeServer("agros2d-server");
    if (!m_server->listen("agros2d-server"))
    {
        qWarning() << tr("Error: Unable to start the server (agros2d-server): %1.").arg(m_server->errorString());
        return;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(connected()));
}

ScriptEngineRemote::~ScriptEngineRemote()
{
    delete m_server;
}

void ScriptEngineRemote::connected()
{
    command = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readCommand()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void ScriptEngineRemote::readCommand()
{
    QTextStream in(m_server_socket);
    command = in.readAll();
}

void ScriptEngineRemote::disconnected()
{
    m_server_socket->deleteLater();

    ScriptResult result;
    if (!command.isEmpty())
    {
        result = currentPythonEngineAgros()->runScript(command);
    }

    m_client_socket = new QLocalSocket();
    connect(m_client_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

    m_client_socket->connectToServer("agros2d-client");
    if (m_client_socket->waitForConnected(1000))
    {
        QTextStream out(m_client_socket);
        out << result.text;
        out.flush();
        m_client_socket->waitForBytesWritten();
    }
    else
    {
        displayError(QLocalSocket::ConnectionRefusedError);
    }

    delete m_client_socket;
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
        qWarning() << tr("Server error: The following error occurred: %1.").arg(m_client_socket->errorString());
    }
}
