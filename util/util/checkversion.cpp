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

#include "checkversion.h"

#include "../util.h"
#include "../config.h"
#include "../util/system_utils.h"

QString esc(const QString &str)
{
#if QT_VERSION < 0x050000
    return Qt::escape(str);
#else
    return QString(str).toHtmlEscaped();
#endif
}

static CheckVersion *checkVersion = NULL;
void checkForNewVersion(bool quiet)
{
    // download version
    QUrl url("http://www.agros2d.org/version/log/version.php");

    if (checkVersion == NULL)
        checkVersion = new CheckVersion(url);

    checkVersion->run(quiet);
}

CheckVersion::CheckVersion(QUrl url) : QObject(), m_url(url)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

CheckVersion::~CheckVersion()
{
    delete m_manager;
}

void CheckVersion::run(bool quiet)
{
    m_quiet = quiet;

    QByteArray postData;
    postData.append("OS=" +  esc(SystemUtils::operatingSystem()) + "&");    
    postData.append("PROCESSOR=" +  esc(SystemUtils::cpuType()) + "&");
    postData.append("THREADS=" +  QString::number(SystemUtils::numberOfThreads()) + "&");
    postData.append("MEMORY=" +  QString::number(SystemUtils::totalMemorySize()) + "&");
    postData.append("RESOLUTION=" +  esc(QString("%1 x %2").
                                         arg(QApplication::desktop()->screenGeometry().width()).
                                         arg(QApplication::desktop()->screenGeometry().height())) + "&");
    postData.append("AGROS2D_VERSION=" +  esc(QApplication::applicationVersion()) + "&");
    postData.append("AGROS2D_ARCH=" +  esc(version64bit() ? "64 bit" : "32 bit") + "&");

    QNetworkRequest req(m_url);
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));

    m_networkReply = m_manager->post(req, postData);

    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    QString text = networkReply->readAll().trimmed();

    if (!text.isEmpty())
    {
        if (text > versionString())
        {
            QString str(tr("<b>New version available.</b><br/><br/>"
                           "Actual version: %1<br/>"
                           "New version: %2<br/><br/>"
                           "URL: <a href=\"http://www.agros2d.org/down/\">http://www.agros2d.org/down/</a>").
                        arg(versionString()).
                        arg(text));

            QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
        }
        else if (!m_quiet)
        {
            QMessageBox::information(QApplication::activeWindow(), tr("New version"), tr("You are using actual version."));
        }
    }
    else if (!m_quiet)
    {
        QMessageBox::critical(QApplication::activeWindow(), tr("New version"), tr("File is corrupted or network is disconnected."));
        return;
    }

}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    qDebug() << "An error ocurred (code #" << error << ").";
}

