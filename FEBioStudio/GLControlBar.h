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

class CMainWindow;
class CGLControlBar_UI;

class CGLControlBar : public QWidget
{
	Q_OBJECT

public:
	CGLControlBar(CMainWindow* wnd, QWidget* parent = 0);

	void Update();

	void SetMeshItem(int n);

	void toggleSelectConnected();

private slots:
	void onPivotChanged();
	void onPivotClicked(bool b);
	void onSnapToGridClicked(bool b);
	void onSnapToNodeClicked(bool b);
	void onToggleVisibleClicked(bool b);
	void onMeshButtonClicked(int n);
	void onMeshToolClicked(int n);
	void onMaxAngleChanged(double v);
	void onSelectBackfacing(bool b);
	void onIgnoreInterior(bool b);
	void onHideSelection(bool b);
	void onShowAll(bool b);
	void onSelectAndHide(bool b);
	void onZoomSelectClicked(bool b);
	void onZoomAllClicked(bool b);
	void onToggleMesh(bool b);
	void onToggleLight(bool b);

private:
	CGLControlBar_UI*	ui;
};
