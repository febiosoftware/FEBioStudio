#pragma once
#include <QTabBar>
#include <string>

class CMainWindow;
class CDocument;

class CMainTabBar : public QTabBar
{
	Q_OBJECT

public:
	CMainTabBar(CMainWindow* wnd, QWidget* parent = nullptr);

	int views();
	int getActiveView();
	void setActiveView(int n);
	void addView(const std::string& name, CDocument* doc = nullptr, bool makeActive = true);
	int findView(CDocument* doc);
	CDocument* getActiveDoc();
	CDocument* getDocument(int n);
	void closeView(int n);

private:
	std::vector<CDocument*>	m_docs;
};
