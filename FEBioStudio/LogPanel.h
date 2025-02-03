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
#include <QTextCharFormat>

namespace Ui {
	class CLogPanel;
};

class CLogPanel : public QWidget
{
	Q_OBJECT

public:
	enum LogTarget {
		FBS_LOG,
		FEBIO_LOG,
		BUILD_LOG,
		PYTHON_LOG
	};

public:
	CLogPanel(QWidget* parent = 0);

	void Clear(LogTarget trg);

	void ShowLog(LogTarget trg);

	// Changed to allow for parsing of ANSI escape codes that might be
	// returned from an ssh session
	// Code taken from https://stackoverflow.com/questions/26500429/qtextedit-and-colored-bash-like-output-emulation
	// Modifed to work with QRegularExpression instead of QRegEx
	void AddText(const QString& txt, LogTarget trg = LogTarget::FBS_LOG);

private slots:
	void on_logSave_clicked(bool b);
	void on_logClear_clicked(bool b);
	void on_logScroll_clicked(bool b);
	void on_combo_currentIndexChanged(int i);

private:
	void parseEscapeSequence(int attribute, QListIterator< QString > & i, QTextCharFormat & textCharFormat, QTextCharFormat const & defaultTextCharFormat);

private:
	Ui::CLogPanel*	ui;
};
