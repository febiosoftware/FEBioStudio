#include "welcomePage.h"
#include "MainWindow.h"
#include "version.h"

const char* welcome = \
"<html>\
<head>\
<style>\
h1{ color: gray; }\
a { color: DodgerBlue; font-size: 11pt; }\
ul { line-height: 150%; list-style-type: none; }\
</style>\
</head>\
<body>\
<p style=\"font-size: 36pt\"><img src=\":/icons/FEBioStudio.png\" style=\"float:left\"><b>FEBio Studio</b></p>\
<p style=\"font-size: 14pt\">version VERSION</p>\
<h1>Start</h1>\
<ul>\
<li><a href=\"#new\">New Model ... </a></li>\
<li><a href=\"#open\">Open Model ...</a></li>\
<li><a href=\"#openproject\">Open Project ...</a></li>\
</ul>\
<h1>Recent</h1>\
<ul>\
RECENT_FILES\
</ul>\
<h1>Online Resources</h1>\
<ul>\
<li><a href=\"#febio\">FEBio Website</li>\
<li><a href=\"#help\">FEBio Knowledgebase</li>\
</ul>\
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

CWelcomePage::CWelcomePage(CMainWindow* wnd) : QTextBrowser(wnd)
{
	m_wnd = wnd;
}

void CWelcomePage::Refresh()
{
	QStringList files = m_wnd->GetRecentFileList();

	QString links;

	for (int i = 0; i < files.size(); ++i)
	{
		QString file = elide(files[i], 70);

		QString ref = QString("\"#recent_%1\"").arg(i);

		QString link = "<li><a href=" + ref + ">" + file + "</a></li>";

		links += link;
	}

	QString version = QString("%1.%2.%3").arg(VERSION).arg(SUBVERSION).arg(SUBSUBVERSION);

	QString page(welcome);
	page.replace("VERSION", version);
	page.replace("RECENT_FILES", links);

	setHtml(page);
}
