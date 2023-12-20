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

#pragma once
#include <QWidget>
#include <vector>
#include <list>
//using namespace std;

using std::vector;
using std::list;

class QListWidgetItem;

namespace Ui {
	class CSelectionBox;
};

class CSelectionBox : public QWidget
{
	Q_OBJECT

	class Item 
	{
	public:
		QString		m_label;
		int			m_data;

	public:
		Item() { m_data = 0; }
		Item(const QString& s, int data) : m_label(s), m_data(data) {}
		Item(const Item& it) { m_label = it.m_label; m_data = it.m_data; }
		void operator = (const Item& it) { m_label = it.m_label; m_data = it.m_data; }
	};

public:
	CSelectionBox(QWidget* parent = 0);

	void setName(const QString& name);
	void setType(const QString& name);

	void clearData();
	void addData(const QString& item, int data, int fmt = 0, bool checkForDuplicates = true);
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

	void setCollapsed(bool b);

signals:
	void addButtonClicked();
	void subButtonClicked();
	void delButtonClicked();
	void selButtonClicked();
	void clearButtonClicked();
	void nameChanged(const QString& t);
	void pickClicked();

private slots:
	void on_addButton_clicked();
	void on_subButton_clicked();
	void on_delButton_clicked();
	void on_selButton_clicked();
	void on_name_textEdited(const QString& t);
	void on_list_itemDoubleClicked(QListWidgetItem *item);
	void on_clearSelection_clicked();
	void on_toggleCollapse_toggled(bool b);
	void on_pick_clicked(bool b);

private:
	Ui::CSelectionBox*	ui;
	vector<Item>		m_items;
};

//-----------------------------------------------------------------------------
class FEItemListBuilder;

class CItemListSelectionBox : public CSelectionBox
{
public:
	CItemListSelectionBox(QWidget* parent = nullptr);

	void SetItemList(FEItemListBuilder* pItem);
};

class CMainWindow;
class FSMeshSelection;

class CMeshSelectionBox : public CItemListSelectionBox
{
	Q_OBJECT

public:
	CMeshSelectionBox(CMainWindow* wnd, QWidget* parent = nullptr);

	void SetSelection(FSMeshSelection* pms);

private slots:
	void onAddButtonClicked();
	void onSubButtonClicked();
	void onDelButtonClicked();
	void onSelButtonClicked();
	void onClearButtonClicked();
	void onPickButtonClicked();
	void onNameChanged(const QString& t);

signals:
	void selectionChanged();

private:
	CMainWindow* m_wnd;
	FSMeshSelection* m_pms;
};
