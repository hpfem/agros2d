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

#ifndef UTIL_CHECKVERSION_H
#define UTIL_CHECKVERSION_H

#include <QtGui>
#include <QtNetwork>

// check for new version
void checkForNewVersion(bool quiet = false);

class CheckVersion : public QObject
{
    Q_OBJECT
public:
    CheckVersion(QUrl url);
    ~CheckVersion();
    void run(bool quiet);

private:
    bool m_quiet;
    QUrl m_url;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_networkReply;

private slots:
    void downloadFinished(QNetworkReply *networkReply);
    void showProgress(qint64, qint64);
    void handleError(QNetworkReply::NetworkError error);
};

#endif // UTIL_CHECKVERSION_H

