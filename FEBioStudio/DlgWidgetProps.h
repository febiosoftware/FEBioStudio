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
#include <QDialog>

namespace Ui{
	class CDlgBoxProps;
	class CDlgLegendProps;
	class CDlgTriadProps;
	class CDlgCaptureFrameProps;
};

class GLWidget;
class QAbstractButton;
class CMainWindow;

class CDlgBoxProps : public QDialog
{
	Q_OBJECT

public:
	CDlgBoxProps(GLWidget* widget, QWidget* parent);

	void accept();
	void apply();

private slots:
	void onClicked(QAbstractButton* button);

private:
	Ui::CDlgBoxProps* ui;
	GLWidget* pw;
};

class CDlgLegendProps : public QDialog
{
	Q_OBJECT

public:
	CDlgLegendProps(GLWidget* widget, CMainWindow* parent);

	void accept();
	void apply();

private slots:
	void onClicked(QAbstractButton* button);

private:
	Ui::CDlgLegendProps* ui;
	GLWidget* pw;
	CMainWindow*	m_wnd;
};

class CDlgTriadProps : public QDialog
{
	Q_OBJECT

public:
	CDlgTriadProps(GLWidget* widget, QWidget* parent);

	void accept();
	void apply();

private slots:
	void onClicked(QAbstractButton* button);

private:
	Ui::CDlgTriadProps* ui;
	GLWidget* pw;
};

class CDlgCaptureFrameProps : public QDialog
{
	Q_OBJECT

public:
	CDlgCaptureFrameProps(GLWidget* widget, QWidget* parent);

	void accept();
	void apply();

private slots:
	void onClicked(QAbstractButton* button);

private slots:
	void onFormat(int nindex);

private:
	Ui::CDlgCaptureFrameProps* ui;
	GLWidget* pw;
};
