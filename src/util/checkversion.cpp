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

#include "util.h"

static CheckVersion *checkVersion = NULL;
void checkForNewVersion(bool quiet)
{
    // download version
#ifdef VERSION_BETA
    QUrl url("http://www.agros2d.org/version/version_beta.xml");
#else
    QUrl url("http://www.agros2d.org/version/version.xml");
#endif
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
    m_networkReply = m_manager->get(QNetworkRequest(m_url));

    qDebug() << m_url.toString();

    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(showProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    QString text = networkReply->readAll();

    qDebug() << text;

    if (!text.isEmpty())
    {
        QDomDocument doc;
        doc.setContent(text);

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problems
        QDomNode eleVersion = eleDoc.toElement().elementsByTagName("version").at(0);

        int beta = eleVersion.toElement().attribute("beta").toInt() == 1;
        int major = eleVersion.toElement().attribute("major").toInt();
        int minor = eleVersion.toElement().attribute("minor").toInt();
        int sub = eleVersion.toElement().attribute("sub").toInt();
        int git = eleVersion.toElement().attribute("git").toInt();
        int year = eleVersion.toElement().attribute("year").toInt();
        int month = eleVersion.toElement().attribute("month").toInt();
        int day = eleVersion.toElement().attribute("day").toInt();

        QDomNode eleUrl = eleDoc.toElement().elementsByTagName("url").at(0);

        if (!m_quiet && git == 0)
        {
            QMessageBox::critical(QApplication::activeWindow(), tr("New version"), tr("File is corrupted or network is disconnected."));
            return;
        }

        QString downloadUrl = eleUrl.toElement().text();
        if (git > VERSION_GIT)
        {
            QString str(tr("<b>New version available.</b><br/><br/>"
                           "Actual version: %1<br/>"
                           "New version: %2<br/><br/>"
                           "URL: <a href=\"%3\">%3</a>").
                        arg(QApplication::applicationVersion()).
                        arg(versionString(major, minor, sub, git, year, month, day, beta)).
                        arg(downloadUrl));

            QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
        }
        else if (!m_quiet)
        {
            QMessageBox::information(QApplication::activeWindow(), tr("New version"), tr("You are using actual version."));
        }
    }
}

void CheckVersion::showProgress(qint64 dl, qint64 all)
{
    // qDebug() << QString("\rDownloaded %1 bytes of %2).").arg(dl).arg(all);
}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    qDebug() << "An error ocurred (code #" << error << ").";
}

