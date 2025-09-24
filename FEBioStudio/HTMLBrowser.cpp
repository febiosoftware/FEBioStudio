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
#include "stdafx.h"
#include "HTMLBrowser.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include "TextDocument.h"
#include "MainWindow.h"

CHTMLBrowser::CHTMLBrowser(CMainWindow* wnd) : CDocumentView(wnd)
{
	QVBoxLayout* l = new QVBoxLayout;
	m_txt = new QTextBrowser;
	m_txt->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	m_txt->setObjectName("htmlview");
	l->addWidget(m_txt);
	setLayout(l);

	QObject::connect(m_txt, &QTextBrowser::anchorClicked, this, &CHTMLBrowser::on_htmlview_anchorClicked);
}

void CHTMLBrowser::setDocument(CDocument* doc)
{
	CHTMLDocument* htmlDoc = dynamic_cast<CHTMLDocument*>(doc);
	if (htmlDoc) m_txt->setDocument(htmlDoc->GetText());
	else
	{
		QString html("\
					<!DOCTYPE html>\
					<html> \
					<body style = \"background-color:#808080;\"> \
					</body> \
					</html>");
		m_txt->setDocument(nullptr);
		m_txt->setHtml(html);
	}
}

void CHTMLBrowser::on_htmlview_anchorClicked(const QUrl& link)
{
	CMainWindow* wnd = mainWindow();
	if (wnd == nullptr) return;

	QString ref = link.toString();
	if      (ref == "#new"        ) wnd->on_actionNewModel_triggered();
	else if (ref == "#newproject" ) wnd->on_actionNewProject_triggered();
	else if (ref == "#open"       ) wnd->on_actionOpen_triggered();
	else if (ref == "#openproject") wnd->on_actionOpenProject_triggered();
	else if (ref == "#febio"      ) wnd->on_actionFEBioURL_triggered();
	else if (ref == "#help"       ) wnd->on_actionFEBioResources_triggered();
	else if (ref == "#forum"      ) wnd->on_actionFEBioForum_triggered();
	else if (ref == "#update"     ) wnd->on_actionUpdate_triggered();
	else if (ref.contains("#http"))
	{
		QString temp = link.toString().replace("#http", "https://");
		QDesktopServices::openUrl(QUrl(temp));
	}
	else if (ref == "#bugreport") wnd->on_actionBugReport_triggered();
	else
	{
		std::string s = ref.toStdString();
		const char* sz = s.c_str();
		if (strncmp(sz, "#recent_", 8) == 0)
		{
			int n = atoi(sz + 8);

			QStringList recentFiles = wnd->GetRecentFileList();
			wnd->OpenFile(recentFiles.at(n));
		}
		if (strncmp(sz, "#recentproject_", 15) == 0)
		{
			int n = atoi(sz + 15);

			QStringList recentProjects = wnd->GetRecentProjectsList();
			wnd->OpenFile(recentProjects.at(n));
		}
	}
}
