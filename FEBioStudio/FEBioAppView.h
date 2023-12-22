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
#include <FECore/FEParam.h>
#include "InputWidgets.h"

class CMainWindow;
class FEBioAppDocument;
class QCheckBox;

class FEBioAppView : public QWidget
{
	Q_OBJECT

public:
	FEBioAppView(CMainWindow* wnd);

	void setSource(QString filePath, FEBioAppDocument* app);

public slots:
	void onModelFinished(bool returnCode);

private:
	CMainWindow* m_wnd;
	QWidget* ui;
};

class CFEBioParamEdit : public QObject
{
	Q_OBJECT

public:
	enum class AlignOptions {
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_TOP,
		ALIGN_BOTTOM,
		ALIGN_TOP_LEFT,
		ALIGN_TOP_RIGHT,
		ALIGN_BOTTOM_LEFT,
		ALIGN_BOTTOM_RIGHT
	};

public:
	CFEBioParamEdit(QObject* parent);

	void SetParameter(FEParamValue p) { m_param = p; }
	void SetEditor(CFloatInput* w);
	void SetEditor(QCheckBox* w);

	QWidget* GetEditor() const;

public slots:
	void UpdateFloat(double newValue);
	void UpdateBool(bool newValue);

private:
	FEParamValue m_param;
	QWidget* m_editor;
};
