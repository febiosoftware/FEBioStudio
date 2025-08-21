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
#include <QWizard>
#include "FEBioOpt.h"
#include "FEBioDiagnostic.h"

class FSModel;

class CMainWindow;
class QLineEdit;
class QPushButton;

namespace Ui {
	class CDlgFEBioOptimize;
	class CDlgSelectParam;
}

class CDlgFEBioOptimize : public QWizard
{
	Q_OBJECT

public:
	CDlgFEBioOptimize(CMainWindow* parent);

	void SetFEBioOpt(FEBioOpt ops);
	FEBioOpt GetFEBioOpt();

protected slots:
	void on_addParameter_clicked();
	void on_addData_clicked();
	void on_addElemData_clicked();
	void on_addNodeData_clicked();
	void on_pasteData_clicked();
	void on_pasteElemData_clicked();
	void on_pasteNodeData_clicked();
	void on_addvar_clicked();

private:
	Ui::CDlgFEBioOptimize*	ui;
};

class CSelectParam : public QWidget
{
	Q_OBJECT

private:
	QLineEdit*		m_edit;
	QPushButton*	m_push;
	FSModel*		m_fem;
	int				m_paramOption;

public:
	CSelectParam(FSModel* fem, int paramOption = 0, QWidget* parent = nullptr);

	void clear();
	QString text();
	void setText(const QString& txt);

private slots:
	void onSelectClicked();
};

class CDlgSelectParam : public QDialog
{
	Q_OBJECT

public:
	CDlgSelectParam(FSModel* fem, int paramOption, QWidget* parent = nullptr);

	void accept() override;

	QString text();

private:
	Ui::CDlgSelectParam*	ui;
};

//=================================================================================================
class CDlgFEBioTangentUI;

class CDlgFEBioTangent : public QDialog
{

public:
	CDlgFEBioTangent(CMainWindow* parent);

	FEBioTangentDiagnostic GetData();

private:
	void accept() override;

private:
	CDlgFEBioTangentUI* ui;
};
