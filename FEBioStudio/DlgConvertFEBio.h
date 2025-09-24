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

class CMainWindow;

class CDlgConvertFEBioUI;

class CDlgConvertFEBio : public QDialog
{
	Q_OBJECT

public:
	enum FileFilters {
		FEB_FILES,
		FSM_FILES
	};

public:
	CDlgConvertFEBio(CMainWindow* wnd);

	QStringList getFileNames();

	QString getOutPath();

	int getOutputFormat();

	void SetFileFilter(int n);

public:
	unsigned int m_nsection;
	bool	m_bexportSelections;
	bool	m_compress;
	bool	m_writeNotes;

protected:
	void accept() override;

private slots:
	void on_addFiles();
	void on_addFolder();
	void on_selectOutPath();
	void on_removeFile();
	void on_clearFiles();
	void on_moreOptions();

private:
	CDlgConvertFEBioUI* ui;
};
