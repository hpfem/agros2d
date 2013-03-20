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

#include "xml.h"

#include "util.h"

QString transformXML(const QString &fileName, const QString &stylesheetFileName)
{
    QString out;

    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(QUrl(fileName));
    query.setQuery(QUrl(stylesheetFileName));
    query.evaluateTo(&out);

    return out;
}

void validateXML(const QString &fileName, const QString &schemaFileName)
{
    QXmlSchema schema;
    schema.load(QUrl(schemaFileName));

    MessageHandler schemaMessageHandler;
    schema.setMessageHandler(&schemaMessageHandler);

    if (!schema.isValid())
        throw AgrosException(QObject::tr("Schema '%1' is not valid. %2").
                             arg(schemaFileName).
                             arg(schemaMessageHandler.statusMessage()));

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    QXmlSchemaValidator validator(schema);
    MessageHandler validatorMessageHandler;
    validator.setMessageHandler(&validatorMessageHandler);

    // TODO: cannot read problem file containing two fields
    if (!validator.validate(&file, QUrl::fromLocalFile(file.fileName())))
        throw AgrosException(QObject::tr("File '%1' is not valid Agros2D problem file. Error (line %3, column %4): %2").
                             arg(fileName).
                             arg(validatorMessageHandler.statusMessage()).
                             arg(validatorMessageHandler.line()).
                             arg(validatorMessageHandler.column()));
}
