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
#include "WindowPanel.h"
#include <QDialog>

class CPostDocument;
class CGLModelDocument;

namespace Ui{
	class CStatePanel;
	class CDlgAddState;
}

class CMainWindow;

class CStatePanel : public CWindowPanel
{
	Q_OBJECT

public:
	CStatePanel(CMainWindow* pwnd, QWidget* parent = 0);

	void Update(bool breset) override;

	CGLModelDocument* GetActiveDocument();

private slots:
	void on_stateList_doubleClicked(const QModelIndex& index);
	void on_addButton_clicked();
	void on_editButton_clicked();
	void on_deleteButton_clicked();
	void on_filterButton_clicked();

private:
	Ui::CStatePanel* ui;
};

class CDlgAddState : public QDialog
{
	Q_OBJECT

public:
	CDlgAddState(QWidget* parent);

private slots:
	void accept();

public:
	int		m_nstates;
	double	m_minTime;
	double	m_maxTime;
	int		m_status;
	bool	m_interpolate;

private:
	Ui::CDlgAddState* ui;
};
