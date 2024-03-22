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
#include "welcomePage.h"
#include "MainWindow.h"
#include "version.h"

const char* welcome = \
"<html>\
<head>\
<style>\
body { background-color: _BGCOLOR_; }\
h1 { color: gray; }\
a { font-size: 11pt; }\
ul { line-height: 150%; list-style-type: none; }\
</style>\
</head>\
<body>\
<p style=\"font-size: 36pt\"><img src=\":/icons/FEBioStudio.png\" style=\"float:left\"><b>FEBio Studio</b></p>\
<p style=\"font-size: 14pt\">_VERSION_</p>\
_SERVER_MESSAGE_\
_UPDATE_INFO_\
<h1>Start</h1>\
<table width=100%>\
<tr>\
<td>\
<ul>\
<li><a href=\"#new\">New Model ... </a></li>\
<li><a href=\"#open\">Open Model ...</a></li>\
<li><a href=\"#newproject\">New Project ... </a></li>\
<li><a href=\"#openproject\">Open Project ...</a></li>\
</ul>\
<h1>Recent Files</h1>\
<ul>\
_RECENT_FILES_\
</ul>\
<h1>Recent Projects</h1>\
<ul>\
_RECENT_PROJECTS_\
</ul>\
<h1>Online Resources</h1>\
<ul>\
<li><a href=\"#febio\">FEBio Website</li>\
<li><a href=\"#help\">FEBio Knowledgebase</li>\
<li><a href=\"#forum\">FEBio Forum</li>\
<li><a href=\"#bugreport\">Submit a Bug Report</li>\
</ul>\
</td>\
<td>\
<p style=\"font-size: 14pt\"><b>Welcome to FEBio Studio!</b></p>\
<p style=\"font-size: 12pt\">Click one of the links to the left to get started.</p>\
<p style=\"font-size: 12pt\">Select <b>New or Open Model</b> if you want to create or open a single model and run it in FEBio.</p>\
<p style=\"font-size: 12pt\">Select <b>New or Open Project</b> if you want to create and manage multiple FEBio models.</p>\
</td>\
</tr>\
</table>\
<p style=\"text-align:right; font-size:14pt\">Weiss Lab, University of Utah</p>\
<p style=\"text-align:right; font-size:14pt\">Ateshian Lab, Columbia University</p>\
</body>\
</html>"
;

QString elide(const QString& s, int l)
{
	if (s.length() <= l) return s;
//	int n1 = l / 2 - 2;
//	int n2 = l - 3 - n1;
//	return (s.left(n1) + "..." + s.right(n2));
	return s.left(3) + "..." + s.right(l - 6);
}

//=======================================================================================
CWelcomePage::CWelcomePage(CMainWindow* wnd) : CHTMLDocument(wnd)
{
	SetDocTitle("Welcome");
}

void CWelcomePage::Activate()
{
	// recent files
	QStringList files = m_wnd->GetRecentFileList();
	QString fileLinks;
	for (int i = 0; i < files.size(); ++i)
	{
		QString file = elide(files[i], 70);
		QString ref = QString("\"#recent_%1\"").arg(i);
		QString link = "<li><a href=" + ref + ">" + file + "</a></li>";
		fileLinks += link;
	}

	// recent projects
	QStringList projects = m_wnd->GetRecentProjectsList();
	QString prjLinks;
	for (int i = 0; i < projects.size(); ++i)
	{
		QString file = elide(projects[i], 70);
		QString ref = QString("\"#recentproject_%1\"").arg(i);
		QString link = "<li><a href=" + ref + ">" + file + "</a></li>";
		prjLinks += link;
	}

	QString version = QString("Version %1.%2.%3").arg(FBS_VERSION).arg(FBS_SUBVERSION).arg(FBS_SUBSUBVERSION);

#ifdef DEVCOMMIT
	version = QString("Dev Version %1.%2.%3.%4").arg(FBS_VERSION).arg(FBS_SUBVERSION).arg(FBS_SUBSUBVERSION).arg(DEVCOMMIT);
#endif

	QString page(welcome);
	page.replace("_VERSION_", version);
	page.replace("_RECENT_FILES_", fileLinks);
	page.replace("_RECENT_PROJECTS_", prjLinks);

	int theme = m_wnd->currentTheme();
	if (theme == 0)
		page.replace("_BGCOLOR_", "#fffae7");
	else
		page.replace("_BGCOLOR_", qApp->palette().color(QPalette::Base).name());

	QString updateText;
	if(m_wnd->updateAvailable())
	{
		updateText = "<p style=\"font-size:14pt\">";
		updateText += "A new update is available! Click <a style=\"font-size:14pt\" href=\"#update\">here</a> for more information.</p>"; 
	}
	page.replace("_UPDATE_INFO_", updateText);

    QString serverMessage;
    QString temp = m_wnd->GetServerMessage();
    temp = temp.replace("https://", "#http");
    if(!temp.isEmpty())
    {
        serverMessage = "<p style=\"font-size:14pt\">";
        serverMessage += temp;
        serverMessage += "</p>";
    }  

    page.replace("_SERVER_MESSAGE_", serverMessage);
    

	m_txt.setHtml(page);
}
