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
#include "WindowPanel.h"
#include <QDialog>
#include <string>
#include <vector>
#include <FSCore/math3d.h>

class CMainWindow;
class QModelIndex;
class CPostDocument;

namespace Post {
	class ModelDataField;
	class CGLModel;
}

namespace Ui {
	class CPostDataPanel;
	class CDlgAddDataFile;
	class CDlgFilter;
	class CDlgExportData;
}

class CDlgExportData : public QDialog
{
public:
	CDlgExportData(QWidget* parent);
	~CDlgExportData();

public:
	bool selectionOnly() const;
	bool writeConnectivity() const;
	int stateOutputOption() const;
	QString stateList() const;

private:
	Ui::CDlgExportData* ui;
};

class CPostDataPanel : public CWindowPanel
{
	Q_OBJECT

public:
	CPostDataPanel(CMainWindow* pwnd, QWidget* parent = 0);

	void Update(bool breset) override;

	private slots:
	void on_AddStandard_triggered();
	void on_AddFromFile_triggered();
	void on_AddEquation_triggered();
	void on_AddFilter_triggered();
	void on_CopyButton_clicked();
	void on_DeleteButton_clicked();
	void on_ExportButton_clicked();
	void on_dataList_clicked(const QModelIndex&);
	void on_fieldName_editingFinished();
	void on_props_dataChanged(bool b);

private:
	Post::CGLModel* GetActiveModel();

private:
	Ui::CPostDataPanel* ui;
};

class CDlgAddDataFile : public QDialog
{
	Q_OBJECT

public:
	CDlgAddDataFile(QWidget* parent);

	void accept();

	private slots:
	void onBrowse();

public:
	int	m_nclass;
	int	m_ntype;
	std::string	m_file;
	std::string m_name;

private:
	Ui::CDlgAddDataFile* ui;
};

class CDlgFilter : public QDialog
{
public:
	CDlgFilter(QWidget* parent);

	void setDataOperands(const std::vector<QString>& opNames);
	void setDataField(Post::ModelDataField* pdf);

	int getArrayComponent();

	void accept();

	void setDefaultName(const QString& name);

	QString getNewName();

	int getNewDataFormat();
	int getNewDataClass();

	double GetScaleFactor();
	vec3d  GetVecScaleFactor();

	int GetGradientConfiguration();

public:
	int	m_nflt;

	double	m_theta;
	int		m_iters;

	int		m_nop;
	int		m_ndata;

private:
	double	m_scale[9];	// scale factors
	int		m_nsc;		// scale components

private:
	Ui::CDlgFilter* ui;
};
