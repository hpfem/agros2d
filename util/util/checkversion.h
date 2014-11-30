// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef UTIL_CHECKVERSION_H
#define UTIL_CHECKVERSION_H

#include "../util.h"

#include <QtGui>
#include <QtNetwork>

// check for new version
AGROS_UTIL_API void checkForNewVersion(bool quiet = false, bool isSolver = false);

class AGROS_UTIL_API CheckVersion : public QObject
{
    Q_OBJECT
public:
    CheckVersion(QUrl url, bool isSolver = false);
    ~CheckVersion();
    void run(bool quiet);

private:
    bool m_quiet;
    bool m_solver;
    QUrl m_url;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_networkReply;

private slots:
    void downloadFinished(QNetworkReply *networkReply);
    void handleError(QNetworkReply::NetworkError error);
};

#endif // UTIL_CHECKVERSION_H

