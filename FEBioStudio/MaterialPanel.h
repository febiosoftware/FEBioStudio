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
#include "CommandPanel.h"
#include <FSCore/color.h>
class CPostDocument;

namespace Ui {
	class CMaterialPanel;
}

class MaterialProps;
class QListWidgetItem;

class CMaterialPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CMaterialPanel(CMainWindow* wnd, QWidget* parent = 0);

	void Update(bool breset) override;
	void UpdateStates();

private:
	CPostDocument* GetActiveDocument();

private:
	void SetItemColor(int item, GLColor c);

private slots:
	void on_materialList_currentRowChanged(int nrow);
	void on_showButton_toggled(bool b);
	void on_enableButton_toggled(bool b);
	void on_editName_editingFinished();
	void on_matprops_dataChanged(int nprop);

private:
	Ui::CMaterialPanel*	ui;
	MaterialProps*	m_pmat;
};
