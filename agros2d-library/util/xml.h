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

#ifndef UTIL_XML_H
#define UTIL_XML_H

#include <QtXmlPatterns>

class MessageHandler : public QAbstractMessageHandler
 {
     public:
         MessageHandler()
             : QAbstractMessageHandler(0)
         {
         }

         QString statusMessage() const
         {
             return m_description;
         }

         int line() const
         {
             return m_sourceLocation.line();
         }

         int column() const
         {
             return m_sourceLocation.column();
         }

     protected:
         virtual void handleMessage(QtMsgType type, const QString &description,
                                    const QUrl &identifier, const QSourceLocation &sourceLocation)
         {
             Q_UNUSED(type);
             Q_UNUSED(identifier);

             m_messageType = type;
             m_description = description;
             m_sourceLocation = sourceLocation;
         }

     private:
         QtMsgType m_messageType;
         QString m_description;
         QSourceLocation m_sourceLocation;
 };

// xml transformation
QString transformXML(const QString &fileName, const QString &stylesheetFileName);

// xml validation
void validateXML(const QString &fileName, const QString &schemaFileName);


#endif // UTIL_XML_H

