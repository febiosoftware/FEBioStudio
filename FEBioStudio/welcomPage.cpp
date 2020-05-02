#include "welcomePage.h"
#include "MainWindow.h"

const char* welcome = \
"<html>\
<head>\
<style>\
h1{ color: gray; }\
a { color: DodgerBlue; font-size: 12pt; }\
ul { line-height: 200% }\
</style>\
</head>\
<body>\
<p style=\"font-size: 36pt\"><b>FEBio Studio</b></p>\
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

QString getWelcomePage(CMainWindow* wnd)
{
	QStringList files = wnd->GetRecentFileList();

	QString links;

	for (int i = 0; i < files.size(); ++i)
	{
		QString file = files[i];

		QString ref = QString("\"#recent_%1\"").arg(i);

		QString link = "<li><a href=" + ref + ">" + file + "</a></li>";

		links += link;
	}

	QString page(welcome);
	page.replace("RECENT_FILES", links);

	return page;
}
