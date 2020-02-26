#pragma once
#include <QTabBar>
#include <string>

class CMainWindow;
class CPostDoc;

class CMainTabBar : public QTabBar
{
	Q_OBJECT

public:
	CMainTabBar(CMainWindow* wnd, QWidget* parent = nullptr);

	int views();
	int getActiveView();
	void setActiveView(int n);
	void addView(const std::string& name, CPostDoc* postDoc = nullptr, bool makeActive = true);
	int findView(CPostDoc* doc);
	CPostDoc* getActiveDoc();
	void closeView(int n);
	CPostDoc* getPostDoc(int i);

private:
	std::vector<CPostDoc*>	m_docs;
};
