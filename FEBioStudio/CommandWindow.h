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
#include <initializer_list>
#include <FSCore/ParamBlock.h>

class CMainWindow;

namespace Ui {
	class CCommandWindow;
}

class CCommandLine
{
public:
	CCommandLine();
	CCommandLine(const char* szcmd);
	CCommandLine(const QString& cmd);
	CCommandLine(const QString& cmd, const QString& arg1);
	CCommandLine(const QString& cmd, const QString& arg1, const QString& arg2);
	CCommandLine(std::initializer_list<QString> initList);

	CCommandLine& operator << (const QString& s);
	CCommandLine& operator << (const QStringList& args);

	CCommandLine& AddArgument(const QString& arg);
	CCommandLine& AddCommand(const QString& cmd);

	QString GetCommandString() const { return m_cmd; }

private:
	QString	m_cmd;
};

class CCommandWindow;

class CCommandLogger
{
public:
	static void SetCommandWindow(CCommandWindow* wnd) { m_wnd = wnd; }

	static void Log(const CCommandLine& cmd);

private:
	CCommandLogger() {}
	static CCommandWindow* m_wnd;
};

QStringList Stringify(const ParamContainer& PL);

class CCommandWindow : public QWidget
{
	Q_OBJECT

public:
	CCommandWindow(CMainWindow* wnd, QWidget* parent = 0);
	~CCommandWindow();

	void Show();

	void showEvent(QShowEvent* ev) override;

	void LogCommand(const CCommandLine& cmd);

public slots:
	void OnEnter();
	void OnSave();
	void OnClear();

private:
	Ui::CCommandWindow* ui;
};