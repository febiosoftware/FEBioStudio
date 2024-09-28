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

#include "stdafx.h"
#include "DlgImportLines.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include "Document.h"
#include <PostLib/FEPostModel.h>
#include <MeshTools/SphereFit.h>
#include "PropertyListView.h"
#include <PostLib/FEPointCongruency.h>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <PostGL/GLLinePlot.h>
#include "MainWindow.h"
#include "PostDocument.h"
#include <MeshLib/FEFindElement.h>
#include <PostLib/GLModel.h>
#include <PostGL/GLPointPlot.h>
#include <sstream>
using namespace std;

class CDlgImportLinesUI
{
public:
	CMainWindow*	m_wnd;

	QLineEdit*	name;
	QLineEdit*	fileName;

	static int m_ncount;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* pv = new QVBoxLayout;
		QHBoxLayout* ph = new QHBoxLayout;
		name = new QLineEdit(QString("Lines%1").arg(m_ncount++));
		QLabel* pl = new QLabel("Name"); pl->setBuddy(name);
		ph->addWidget(pl);
		ph->addWidget(name);
		pv->addLayout(ph);

		ph = new QHBoxLayout;
		fileName = new QLineEdit;
		QPushButton* browse = new QPushButton("Browse...");
		ph->addWidget(fileName);
		ph->addWidget(browse);
		pv->addLayout(ph);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(bb);
		dlg->setLayout(pv);

		QObject::connect(browse, SIGNAL(clicked(bool)), dlg, SLOT(OnBrowse()));
		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(OnApply()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

int CDlgImportLinesUI::m_ncount = 1;

// constructor
CDlgImportLines::CDlgImportLines(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgImportLinesUI)
{
	ui->m_wnd = wnd;
	setWindowTitle("Import Lines");
	ui->setup(this);
}

QString CDlgImportLines::GetFileName() { return ui->fileName->text(); }
QString CDlgImportLines::GetName() { return ui->name->text(); }

void CDlgImportLines::OnApply()
{
	if (ui->fileName->text().isEmpty())
	{
		QMessageBox::warning(this, "FEBio Studio", "Please select a valid file name.");
		return;
	}

	if (ui->name->text().isEmpty()) {
		QMessageBox::warning(this, "FEBio Studio", "Please enter a valid name.");
		return;
	}

	accept();
}

void CDlgImportLines::OnBrowse()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "All files(*)");
	if (filename.isEmpty() == false)
	{
		ui->fileName->setText(filename);
	}
}

//=============================================================================

class CDlgImportPointsUI
{
public:
	CMainWindow*	m_wnd;

	QLineEdit*	name;
	QLineEdit*	fileName;
	static int m_ncount;

public:
	void setup(QDialog* dlg)
	{
		QPushButton* browse;
		QVBoxLayout* pv = new QVBoxLayout;
		QGridLayout* pgrid = new QGridLayout;
		name = new QLineEdit(QString("Points%1").arg(m_ncount++));
		QLabel* pl = new QLabel("Name:"); pl->setBuddy(name);
		pgrid->addWidget(pl, 0, 0);
		pgrid->addWidget(name, 0, 1);

		fileName = new QLineEdit;
		pl = new QLabel("File:"); pl->setBuddy(fileName);
		browse = new QPushButton("..."); browse->setFixedWidth(30);
		pgrid->addWidget(pl, 1, 0);
		pgrid->addWidget(fileName, 1, 1);
		pgrid->addWidget(browse, 1, 2);

		pv->addLayout(pgrid);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(bb);
		dlg->setLayout(pv);
		QObject::connect(browse, SIGNAL(clicked(bool)), dlg, SLOT(OnBrowse()));
		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(OnApply()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

int CDlgImportPointsUI::m_ncount = 1;

// constructor
CDlgImportPoints::CDlgImportPoints(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgImportPointsUI)
{
	ui->m_wnd = wnd;
	setWindowTitle("Import points");
	ui->setup(this);
}

void CDlgImportPoints::OnBrowse()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "All files(*)");
	if (filename.isEmpty() == false)
	{
		ui->fileName->setText(filename);
	}
}

class PointDataFile : public Post::PointDataSource
{
public:
	PointDataFile(Post::PointDataModel* mdl) : Post::PointDataSource(mdl) {}
	bool Load(const char* szfile) override
	{
		m_fileName = szfile;
		bool b = Reload();
		if (b == false) m_fileName.clear();
		return b;
	}

	bool Reload() override
	{
		Post::PointDataModel* pointData = GetPointDataModel();
		pointData->Clear();

		Post::FEPostModel& fem = *pointData->GetFEModel();

		const char* szfile = m_fileName.c_str();

		FILE* fp = fopen(szfile, "rt");
		if (fp == 0) return false;

		char szline[256] = { 0 };

		// get the first line
		fgets(szline, 255, fp);

		vector<string> header;

		// if the line starts with an asterisk, it is the header
		if (szline[0] == '*')
		{
			// remove trailing newline
			char* eol = strrchr(szline, '\n');
			if (eol) *eol = 0;
			else {
				eol = strrchr(szline, '\r');
				if (eol) *eol = 0;
			}

			// process header
			char tmp[256] = { 0 };
			char* sz = szline + 1;
			do {
				char* c = strchr(sz, ',');
				if (c)
				{
					int l = c - sz;
					strncpy(tmp, sz, l);
					tmp[l] = 0;
					header.push_back(tmp);
					c++;
				}
				else header.push_back(sz);
				sz = c;
			} while (sz);

			// read the next line
			fgets(szline, 255, fp);
		}

		// count the nr of fields in the line
		char* sz = szline;
		int nfields = 1;
		while (sz = strchr(sz, ',')) { nfields++; ++sz; }

		if (nfields < 5)
		{
			fclose(fp);
			return false;
		}

		int ndataFields = nfields - 1; assert(ndataFields >= 0);
		assert(ndataFields <= MAX_POINT_DATA_FIELDS);

		while (!feof(fp))
		{
			// process line
			int nstate, id;
			float x, y, z;
			vector<float> v;
			if (ndataFields > 0) v.assign(ndataFields, 0.f);

			sz = szline;
			int n = 0;
			do {
				switch (n)
				{
				case 0: nstate = atoi(sz); break;
				case 1: id = atoi(sz); v[n - 1] = (float)id; break;
				case 2: x = atof(sz); v[n - 1] = (float)x; break;
				case 3: y = atof(sz); v[n - 1] = (float)y; break;
				case 4: z = atof(sz); v[n - 1] = (float)z; break;
				default:
					if (n - 1 < MAX_POINT_DATA_FIELDS) v[n - 1] = atof(sz); else assert(false);
				}

				sz = strchr(sz, ',');
				if (sz) sz++;
				n++;
			} while (sz);

			if ((nstate >= 0) && (nstate < fem.GetStates()))
			{
				Post::PointData& pd = pointData->GetPointData(nstate);
				if (ndataFields > 0)
					pd.AddPoint(vec3f(x, y, z), v, id);
				else
					pd.AddPoint(vec3f(x, y, z), id);
			}

			// read the next line
			if (fgets(szline, 255, fp) == nullptr) break;
		}
		fclose(fp);

		for (int i = 0; i < ndataFields; ++i)
		{
			string dataName;
			int m = i + 1;
			if (m < header.size()) dataName = header[m];
			else
			{
				stringstream ss;
				ss << "Data" << i + 1;
				dataName = ss.str();
			}

			pointData->AddDataField(dataName);
		}
        return true;
	}

private:
	std::string	m_fileName;
};

void CDlgImportPoints::OnApply()
{
	CPostDocument* doc = ui->m_wnd->GetPostDocument();
	if (doc && doc->IsValid())
	{
		string fileName = ui->fileName->text().toStdString();
		string name = ui->name->text().toStdString();

		Post::FEPostModel& fem = *doc->GetFSModel();

		// create a data point
		Post::PointDataModel* pointData = new Post::PointDataModel(&fem);

		// create the point data source
		PointDataFile* src = new PointDataFile(pointData);

		bool b = src->Load(fileName.c_str());

		if (b)
		{
			// add a line plot
			Post::CGLPointPlot* pgl = new Post::CGLPointPlot();

			pgl->SetPointDataModel(pointData);

			doc->GetGLModel()->AddPlot(pgl);
			pgl->SetName(name.c_str());

			ui->name->setText(QString("Points%1").arg(ui->m_ncount));

			ui->m_wnd->UpdatePostPanel(false, pgl);
		}
		else
		{
			QMessageBox::critical(this, "FEBio Studio", "Hmm, that does not look the correct file format.");
		}

		accept();
	}
}
