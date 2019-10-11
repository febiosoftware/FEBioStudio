#pragma once
#include <QWidget>
#include <vector>
using namespace std;

class CDocument;
class FSObject;
class QListWidget;
class QListWidgetItem;
class QLineEdit;
class FEModel;
class CModelViewer;
class CModelTree;

class CModelSearch : public QWidget
{
	Q_OBJECT

public:	
	CModelSearch(CModelViewer* view, CModelTree* tree, QWidget* parent = 0);

	void Build(CDocument* doc);

	void contextMenuEvent(QContextMenuEvent* ev) override;

	void GetSelection(std::vector<FSObject*>& sel);

	void UpdateObject(FSObject* po);

private:
	void UpdateList();

protected slots:
	void onFilterChanged(const QString& t);
	void onItemDoubleClicked(QListWidgetItem* item);
	void onItemClicked(QListWidgetItem* item);

private:
	CModelViewer*	m_view;
	CModelTree*		m_tree;
	QLineEdit*		m_flt;
	QListWidget*	m_list;
};
