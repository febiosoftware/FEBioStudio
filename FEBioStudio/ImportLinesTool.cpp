#include "stdafx.h"
#include "ImportLinesTool.h"
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
#include <PostGL/GLLinePlot.h>
#include "MainWindow.h"
#include "PostDoc.h"
#include <PostGL/GLModel.h>
using namespace Post;

class CImportLinesToolUI : public QWidget
{
public:
	QLineEdit*	name;
	QLineEdit*	fileName;
	int			m_ncount;

public:
	CImportLinesToolUI(CImportLinesTool* ptool)
	{
		m_ncount = 1;

		QPushButton* apply;
		QPushButton* browse;
		QVBoxLayout* pv = new QVBoxLayout;
		{
			QHBoxLayout* ph = new QHBoxLayout;
			name = new QLineEdit(QString("Lines%1").arg(m_ncount));
			QLabel* pl = new QLabel("Name"); pl->setBuddy(name);
			ph->addWidget(pl);
			ph->addWidget(name);
			pv->addLayout(ph);

			ph = new QHBoxLayout;
			fileName = new QLineEdit;
			browse = new QPushButton("Browse...");
			ph->addWidget(fileName);
			ph->addWidget(browse);
			pv->addLayout(ph);

			apply = new QPushButton("Apply");
			pv->addWidget(apply);
			pv->addStretch();
		}
		setLayout(pv);
		QObject::connect(apply , SIGNAL(clicked(bool)), ptool, SLOT(OnApply()));
		QObject::connect(browse, SIGNAL(clicked(bool)), ptool, SLOT(OnBrowse()));
	}
};

// constructor
CImportLinesTool::CImportLinesTool(CMainWindow* wnd) : CAbstractTool(wnd, "Import lines")
{
	ui = 0;
}

// get the property list
QWidget* CImportLinesTool::createUi()
{
	return ui = new CImportLinesToolUI(this);
}

void CImportLinesTool::OnApply()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		string fileName = ui->fileName->text().toStdString();
		string name = ui->name->text().toStdString();

		bool bsuccess = false;
		const char* szfile = fileName.c_str();
		const char* szext = strrchr(szfile, '.');
		if (szext  && (strcmp(szext, ".ang2") == 0))
		{
			// Read AngioFE2 format
			int nret = ReadAng2Format(szfile);
			bsuccess = (nret != 0);
			if (nret == 2)
			{
				QMessageBox::warning(0, "FEBio Studio", "End-of-file reached before all states were processed.");
			}
		}
		else
		{
			// read old format (this assumes this is a text file)
			bsuccess = ReadOldFormat(szfile);
		}

		if (bsuccess)
		{
			// add a line plot for visualizing the line data
			CGLLinePlot* pgl = new CGLLinePlot(doc->GetGLModel());
			doc->GetGLModel()->AddPlot(pgl);
			pgl->SetName(ui->name->text().toStdString());

			ui->m_ncount++;
			ui->name->setText(QString("Lines%1").arg(ui->m_ncount));

			GetMainWindow()->UpdatePostPanel(false, pgl);
			updateUi();
		}
		else
		{
			QMessageBox::critical(0, "FEBio Studio", "Failed reading line data file.");
		}
	}
}

void CImportLinesTool::OnBrowse()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "All files(*)");
	if (filename.isEmpty() == false)
	{
		ui->fileName->setText(filename);
	}
}

bool CImportLinesTool::ReadOldFormat(const char* szfile)
{
	CPostDoc* doc = GetPostDoc();
	if (doc == nullptr) return false;

	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	Post::FEPostModel& fem = *doc->GetFEModel();

	char szline[256] = { 0 };
	while (!feof(fp))
	{
		if (fgets(szline, 255, fp))
		{
			int nstate;
			float x0, y0, z0, x1, y1, z1;
			int n = sscanf(szline, "%d%*g%g%g%g%g%g%g", &nstate, &x0, &y0, &z0, &x1, &y1, &z1);
			if (n == 7)
			{
				if ((nstate >= 0) && (nstate < fem.GetStates()))
				{
					FEState& s = *fem.GetState(nstate);
					s.AddLine(vec3f(x0, y0, z0), vec3f(x1, y1, z1));
				}
			}
		}
	}

	fclose(fp);

	return true;
}

// helper structure for finding position of vessel fragments
struct FRAG
{
	int		iel;	// element in which tip lies
	double	r[3];	// iso-coords of tip
	vec3f	r0;		// reference position of tip
	double	user_data;
};

vec3f GetCoordinatesFromFrag(Post::FEPostModel& fem, int nstate, FRAG& a)
{
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
	vec3f x[FEElement::MAX_NODES];

	vec3f r0 = a.r0;
	if (a.iel >= 0)
	{
		FEElement_& el = mesh.ElementRef(a.iel);
		for (int i=0; i<el.Nodes(); ++i) x[i] = fem.NodePosition(el.m_node[i], nstate);
		r0 = el.eval(x, a.r[0], a.r[1], a.r[2]);
	}

	return r0;
}

// return code:
// 0 = failed
// 1 = success
// 2 = EOF reached before all states were read in
int CImportLinesTool::ReadAng2Format(const char* szfile)
{
	CPostDoc* doc = GetPostDoc();
	if (doc == nullptr) return false;

	FILE* fp = fopen(szfile, "rb");
	if (fp == 0) return 0;

	Post::FEPostModel& fem = *doc->GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	// read the magic number
	unsigned int magic = 0;
	if (fread(&magic, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; };
	if (magic != 0xfdb97531) { fclose(fp); return 0; }

	// read the version number
	unsigned int version = 0;
	if (fread(&version, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }

	// the flags say if vessels can grow inside a material or not
	int mats = fem.Materials();
	vector<bool> flags(mats, true);

	// number of user-defined data fields in line file.
	int ndataFields = 0;

	switch (version)
	{
	case 0: break;	// nothing to do (all materials are candidates)
	case 1: 
		{
			// read masks
			int n = 0;
			unsigned int masks = 0;
			if (fread(&masks, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }
			for (int i=0; i<masks; ++i)
			{
				unsigned int mask = 0;
				if (fread(&mask, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }
				for (int j=0; j<32; ++j)
				{
					bool b = ((mask & (1 << j)) != 0);
					flags[n++] = b;
					if (n == mats) break;
				}
				if (n == mats) break;
			}
		}
		break;
	default:
		fclose(fp); return 0;
	}

	// store the raw data
	vector<pair<FRAG, FRAG> > raw;

	// we need to make sure that the mesh' coordinates
	// are the initial coordinates
	const int N = mesh.Nodes();
	vector<vec3d> tmp(N);

	// copy the initial positions to this mesh
	for (int i = 0; i < N; ++i)
	{
		tmp[i] = mesh.Node(i).r;
		mesh.Node(i).r = fem.NodePosition(i, 0);
	}

	// build search tool
	FEFindElement find(mesh);
	find.Init(flags);

	int nret = 1;

	int nstate = 0;
	while (!feof(fp) && !ferror(fp))
	{
		if (nstate >= fem.GetStates()) break;
		FEState& s = *fem.GetState(nstate);

		// this file format only stores incremental changes to the network
		// so we need to copy all the data from the previous state as well
		if (nstate > 0)
		{
			// copy line data
			for (int i=0; i<raw.size(); ++i)
			{
				FRAG& a = raw[i].first;
				FRAG& b = raw[i].second;
				vec3f r0 = GetCoordinatesFromFrag(fem, nstate, a);
				vec3f r1 = GetCoordinatesFromFrag(fem, nstate, b);

				// add the line
				s.AddLine(r0, r1, a.user_data, b.user_data, a.iel, b.iel);
			}
		}

		// read number of segments 
		unsigned int segs = 0;
		if (fread(&segs, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); nret = 2; break; }

		// read time stamp (is not used right now)
		float ftime = 0.0f;
		if (fread(&ftime, sizeof(float), 1, fp) != 1) { fclose(fp); nret = 2; break; }

		// read the segments
		int nd = 6 + 2 * ndataFields;
		vector<float> d(nd, 0.f);
		for (int i=0; i<segs; ++i)
		{
			if (fread(&d[0], sizeof(float), nd, fp) != nd) { fclose(fp); nret = 2; break; }

			// store the raw coordinates
			float* c = &d[0];
			vec3f a0 = vec3f(c[0], c[1], c[2]); c += 3 + ndataFields;
			vec3f b0 = vec3f(c[0], c[1], c[2]);

			float va = 0.f, vb = 0.f;
			if (ndataFields > 0)
			{
				va = d[3];
				vb = d[6 + ndataFields];
			}

			FRAG a, b;
			a.user_data = va;
			b.user_data = vb;
			if (find.FindElement(a0, a.iel, a.r) == false) a.iel = -1;
			if (find.FindElement(b0, b.iel, b.r) == false) b.iel = -1;
			raw.push_back(pair<FRAG, FRAG>(a, b));

			// convert them to global coordinates
			vec3f r0 = GetCoordinatesFromFrag(fem, nstate, a);
			vec3f r1 = GetCoordinatesFromFrag(fem, nstate, b);

			// add the line data
			s.AddLine(r0, r1, va, vb, a.iel, b.iel);
		}
		if (nret != 1) break;

		// next state
		nstate++;
	}
	fclose(fp);

	// restore mesh' nodal positions
	for (int i = 0; i < N; ++i) mesh.Node(i).r = tmp[i];

	return nret;
}


//=============================================================================

class CImportPointsToolUI : public QWidget
{
public:
	QLineEdit*	name;
	QLineEdit*	fileName;
	int			m_ncount;

public:
	CImportPointsToolUI(CImportPointsTool* ptool)
	{
		m_ncount = 1;

		QPushButton* apply;
		QPushButton* browse;
		QVBoxLayout* pv = new QVBoxLayout;
		{
			QGridLayout* pgrid = new QGridLayout;
			name = new QLineEdit(QString("Points%1").arg(m_ncount));
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

			apply = new QPushButton("Apply");
			pv->addWidget(apply);
			pv->addStretch();
		}
		setLayout(pv);
		QObject::connect(apply , SIGNAL(clicked(bool)), ptool, SLOT(OnApply()));
		QObject::connect(browse, SIGNAL(clicked(bool)), ptool, SLOT(OnBrowse()));
	}
};

// constructor
CImportPointsTool::CImportPointsTool(CMainWindow* wnd) : CAbstractTool(wnd, "Import points")
{
	ui = 0;
}

// get the property list
QWidget* CImportPointsTool::createUi()
{
	return ui = new CImportPointsToolUI(this);
}

void CImportPointsTool::OnBrowse()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "All files(*)");
	if (filename.isEmpty() == false)
	{
		ui->fileName->setText(filename);
	}
}

void CImportPointsTool::OnApply()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		string fileName = ui->fileName->text().toStdString();
		string name = ui->name->text().toStdString();

		FILE* fp = fopen(fileName.c_str(), "rt");
		if (fp == 0)
		{
			QMessageBox::critical(0, "FEBio Studio", "Failed opening data file.");
			return;
		}

		Post::FEPostModel& fem = *doc->GetFEModel();

		char szline[256] = {0};
		while (!feof(fp))
		{
			if (fgets(szline, 255, fp))
			{
				int nstate, id;
				float x, y, z;
				int n = sscanf(szline, "%d%d%g%g%g", &nstate, &id, &x, &y, &z);
				if (n == 5)
				{
					if ((nstate >= 0) && (nstate < fem.GetStates()))
					{
						FEState& s = *fem.GetState(nstate);
						s.AddPoint(vec3f(x, y, z), id);
					}
				}
			}
		}
		fclose(fp);

		// add a line plot
		CGLPointPlot* pgl = new CGLPointPlot(doc->GetGLModel());
		doc->GetGLModel()->AddPlot(pgl);
		pgl->SetName(name.c_str());

		ui->m_ncount++;
		ui->name->setText(QString("Points%1").arg(ui->m_ncount));

		updateUi();
	}
}
