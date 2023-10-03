/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include "ServerSettings.h"

bool ServerSettings::instantiated = false;
QString ServerSettings::scheme = "https";
QString ServerSettings::url = "repo.febio.org";
int ServerSettings::port = 443;

void ServerSettings::Instantiate()
{
    QString serverXML = QApplication::applicationDirPath() + "/server.xml";

    if(QFileInfo::exists(serverXML))
    {
        QXmlStreamReader reader;

        QFile serverXMLFile(serverXML);
        serverXMLFile.open(QIODevice::ReadOnly);

        reader.setDevice(&serverXMLFile);

        if (reader.readNextStartElement())
        {
            if(reader.name().toString() == "server")
            {
                while(reader.readNextStartElement())
                {
                    if(reader.name().toString() == "scheme")
                    {
                        scheme = reader.readElementText();
                    }
                    else if(reader.name().toString() == "url")
                    {
                        url = reader.readElementText();
                    }
                    else if(reader.name().toString() == "port")
                    {
                        port = reader.readElementText().toInt();
                    }
                    else
                    {
                        reader.skipCurrentElement();
                    }
                }
            }
        }
        
        serverXMLFile.close();
    }

    instantiated = true;
}

const QString& ServerSettings::Scheme()
{
    if(!instantiated) Instantiate();

    return scheme;
}

const QString& ServerSettings::URL()
{
    if(!instantiated) Instantiate();

    return url;
}

int ServerSettings::Port()
{
    if(!instantiated) Instantiate();

    return port;
}