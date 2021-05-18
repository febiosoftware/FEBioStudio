/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "CreatePanel.h"
#include <vector>
//using namespace std;

namespace Ui {
	class CCreateSpringPane;
}

//-----------------------------------------------------------------------------
// create pane for creating spring groups
//
class CCreateSpringPane : public CCreatePane
{
	Q_OBJECT

public:
	CCreateSpringPane(CCreatePanel* parent);

	FSObject* Create();

	void showEvent(QShowEvent* ev);

	void hideEvent(QHideEvent* ev);

private:
	bool getNodeSelection(vector<int>& node, int n);

	bool updateTempObject();

protected slots:
	void on_node1_addButtonClicked();
	void on_node1_subButtonClicked();
	void on_node1_selButtonClicked();
	void on_node1_delButtonClicked();

	void on_node2_addButtonClicked();
	void on_node2_subButtonClicked();
	void on_node2_selButtonClicked();
	void on_node2_delButtonClicked();

	void on_newSet_clicked();

	void on_method_currentIndexChanged(int n);

private:
	Ui::CCreateSpringPane*	ui;
};
