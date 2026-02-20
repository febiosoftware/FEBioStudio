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

#include <QDesktopServices>
#include <QUrl>
#include "HelpFeature.h"
#include <FEBioLink/FEBioClass.h>

#define MANUAL_PATH "https://febiosoftware.github.io/febio-feature-manual/features/"

QString ClassIDToURL(int classID)
{
	QString url;
	if (classID != -1)
	{
		FEBio::FEBioClassInfo classInfo = FEBio::GetClassInfo(classID);

		QString superClass = classInfo.superClassName;
		superClass.remove(0, 2).remove("_ID");

		QString tmp = QString(classInfo.szmod) + "_" + superClass + "_" + QString(classInfo.sztype);
		url = tmp.replace(" ", "_").toLower();
	}
	return url;
}

void ShowHelp(const QString& url)
{
	QDesktopServices::openUrl(QUrl(QString(MANUAL_PATH) + url));
}
