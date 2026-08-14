#pragma once
#include "TextDocument.h"

class CFEBioStudyReportDoc : public CHTMLDocument
{
public:
	CFEBioStudyReportDoc(CMainWindow* wnd);

	bool OpenReportFile(const QString& fileName);

	void Activate() override;

private:
	QString htmlText;
};
