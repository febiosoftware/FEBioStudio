#pragma once
#include <QWidget>
#include <vector>
#include <list>
using namespace std;

class QListWidgetItem;

namespace Ui {
	class CSelectionBox;
};

class CSelectionBox : public QWidget
{
	Q_OBJECT

public:
	CSelectionBox(QWidget* parent = 0);

	void setName(const QString& name);
	void setType(const QString& name);

	void clearData();
	void addData(const QString& item, int data, int fmt = 0);
	void addData(const vector<int>& data);

	void removeData(int ndata);
	void removeData(const vector<int>& data);

	void getSelectedItems(vector<int>& sel);
	void getSelectedItems(list<int>& sel);

	void removeSelectedItems();

	void getAllItems(vector<int>& data);
	void getAllNames(QStringList& names);

	void showNameType(bool b);

	void enableAddButton(bool b);
	void enableRemoveButton(bool b);
	void enableDeleteButton(bool b);
	void enableSelectButton(bool b);
	void enableAllButtons(bool b);

signals:
	void addButtonClicked();
	void subButtonClicked();
	void delButtonClicked();
	void selButtonClicked();
	void nameChanged(const QString& t);

private slots:
	void on_addButton_clicked();
	void on_subButton_clicked();
	void on_delButton_clicked();
	void on_selButton_clicked();
	void on_name_textEdited(const QString& t);
	void on_list_itemDoubleClicked(QListWidgetItem *item);

private:
	Ui::CSelectionBox*	ui;
	vector<int>			m_data;
};
