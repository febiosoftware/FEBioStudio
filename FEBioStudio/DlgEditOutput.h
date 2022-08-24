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
#include <QDialog>

class FSProject;
class CPlotVariable;
class QListWidgetItem;
class QTableWidgetItem;

namespace Ui {
	class CDlgAddSelection;
	class CDlgEditOutput;
}

class CDlgAddSelection : public QDialog
{
	Q_OBJECT

public:
	CDlgAddSelection(QWidget* parent);

	void setVariable(const CPlotVariable& var);
	void setSelections(const QStringList& l);

	int currentSelection();

private:
	Ui::CDlgAddSelection*	ui;
};

class CDlgEditOutput : public QDialog
{
	Q_OBJECT

public:
	CDlgEditOutput(FSProject& prj, QWidget* parent = 0, int tab = 0);

	void showEvent(QShowEvent* ev) override;

private:
	void UpdateVariables(const QString& flt);
	void UpdateLogTable();

protected slots:
	void OnAddSelection();
	void OnRemoveSelection();
	void OnNewVariable();
	void OnEditVariable();
	void OnDeleteVariable();
	void OnVariable(int nrow);
	void OnItemClicked(QListWidgetItem* item);
	void onFilterChanged(const QString& txt);
	void onLogAdd();
	void onLogRemove();
	void UpdateLogItemList();
	void onItemChanged(QTableWidgetItem* item);
	void accept();

private:
	FSProject&	m_prj;
	Ui::CDlgEditOutput*	ui;
};
