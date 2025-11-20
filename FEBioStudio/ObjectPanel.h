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
#include <GLLib/GLMaterial.h>

class CMainWindow;
class CObjectPanelUI;

class CObjectPanel : public QWidget
{
	Q_OBJECT

public:
	enum CONVERT_FLAGS	{
		CONVERT_TO_EDITABLE_MESH,
		CONVERT_TO_EDITABLE_SURFACE,
		CONVERT_TO_MULTIBLOCK,
		CONVERT_TO_MULTIPATCH
	};

public:
	CObjectPanel(CMainWindow* wnd, QWidget* parent = 0);

	void Update();

protected slots:
	void onMaterialChanged(GLMaterial mat);

private:
	CObjectPanelUI*	ui;
};
