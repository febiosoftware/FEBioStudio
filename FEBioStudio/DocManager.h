#pragma once
#include <vector>
#include <string>

class CDocument;
class CMainWindow;

// This class manages the currently open documents
class CDocManager
{
public:
	CDocManager(CMainWindow* wnd);

	~CDocManager();

	// return the number of documents
	int Documents() const;

	// add a document
	bool AddDocument(CDocument* doc);

	// remove a document
	void RemoveDocument(int i);

	// get a document
	CDocument* GetDocument(int i);

private:
	CMainWindow*	m_wnd;
	std::vector<CDocument*>	m_docList;
};
