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
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "DlgLameConvertor.h"
#include "DlgUnitConverter.h"
#include "DlgKinemat.h"
#include "DlgPlotMix.h"
#include "DlgSettings.h"
#include "DlgMeshDiagnostics.h"
#include "DlgMaterialTest.h"
#include <QMessageBox>
#include <GeomLib/MeshLayer.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include "ModelDocument.h"
#include "PostDocument.h"
#include "DlgStartThread.h"
#include <PostLib/FEKinemat.h>
#include <PostLib/FELSDYNAimport.h>
#include <PostGL/GLModel.h>
#include "FEKinematFileReader.h"

void CMainWindow::on_actionCurveEditor_triggered()
{
	CModelDocument* doc = GetModelDocument();
	if (doc) ui->showCurveEditor();
}

void CMainWindow::on_actionMeshInspector_triggered()
{
	ui->showMeshInspector();
}

void CMainWindow::on_actionMeshDiagnostic_triggered()
{
	GObject* po = GetActiveObject();
	if (po == nullptr)
	{
		QMessageBox::warning(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgMeshDiagnostics dlg(this);
	dlg.SetObject(po);
	dlg.exec();
}

void CMainWindow::on_actionElasticityConvertor_triggered()
{
	CDlgLameConvertor dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionMaterialTest_triggered()
{
	CDlgMaterialTest dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionUnitConverter_triggered()
{
	CDlgUnitConverter dlg(this);
	dlg.exec();
}

class LoadKineFile : public CustomThread
{
public:
	LoadKineFile(CPostDocument* doc) : m_doc(doc)
	{
		m_kine = nullptr;
		m_task = 0;
		m_fileReader = nullptr;
		m_fem = doc->GetFSModel();
		m_currentState = 0;
		m_n0 = m_n1 = m_ni = -1;
	}

	void run() Q_DECL_OVERRIDE
	{
		m_task = 0;
		// load the file
		Post::FELSDYNAimport reader(m_fem);
		reader.read_displacements(true);
		m_fileReader = &reader;
		bool bret = reader.Load(m_modelFile.c_str());
		m_fileReader = nullptr;
		if (bret == false)
		{
			emit resultReady(false);
			return;
		}

		// apply the kine mat file
		m_task = 1;
		FEKinemat kine;
		kine.SetRange(m_n0, m_n1, m_ni);
		m_kine = &kine;
		if (kine.Apply(m_fem, m_kineFile.c_str()) == false)
		{
			emit resultReady(false);
			return;
		}

		// update post document
		m_doc->Initialize();

		// update displacements on all states
		m_task = 2;
		Post::CGLModel& mdl = *m_doc->GetGLModel();
		if (mdl.GetDisplacementMap() == nullptr)
		{
			mdl.AddDisplacementMap("Displacement");
		}
		int nstates = mdl.GetFSModel()->GetStates();
		for (m_currentState = 0; m_currentState < nstates; ++m_currentState) mdl.UpdateDisplacements(m_currentState, true);

		// all done
		m_kine = nullptr;
		emit resultReady(true);
	}

public:
	bool hasProgress() override { return true; }

	double progress() override
	{
		double pct = 0.0;
		if (m_task == 0)
		{
			if (m_fileReader) pct = m_fileReader->GetFileProgress();
		}
		else if (m_task == 1)
		{
			int nstates = (m_kine ? m_kine->States() : 1);
			int nread = m_fem->GetStates();
			pct = (100.0 * nread) / nstates;
		}
		else if (m_task == 2)
		{
			int nstates = m_fem->GetStates();
			pct = (100.0 * m_currentState) / nstates;
		}
		return pct;
	}

	const char* currentTask() override 
	{ 
		const char* sztask = "(unknown)";
		switch (m_task)
		{
		case 0: sztask = "Reading model file ..."; break;
		case 1: sztask = "Processing kine file ..."; break;
		case 2: sztask = "Updating states ..."; break;
		}
		return sztask;
	}

	void stop() override {}

public:
	int		m_n0, m_n1, m_ni;
	std::string	m_modelFile;
	std::string m_kineFile;

private:
	int	m_task;
	int	m_currentState;
	CPostDocument* m_doc;
	Post::FEPostModel* m_fem;
	FEKinemat* m_kine;
	FileReader* m_fileReader;
};

void CMainWindow::on_actionKinemat_triggered()
{
	CDlgKinemat dlg(this);
	if (dlg.exec())
	{
		// create a new document
		CPostDocument* postDoc = new CPostDocument(this);

		std::string modelFile = dlg.GetModelFile().toStdString();
		std::string kineFile  = dlg.GetKineFile().toStdString();

		Post::FEPostModel& fem = *postDoc->GetFSModel();
		LoadKineFile* kinethread = new LoadKineFile(postDoc);
		kinethread->m_n0 = dlg.StartIndex();
		kinethread->m_n1 = dlg.EndIndex();
		kinethread->m_ni = dlg.Increment();
		kinethread->m_modelFile = modelFile;
		kinethread->m_kineFile = kineFile;

		postDoc->SetDocFilePath(modelFile);

		CDlgStartThread dlg2(this, kinethread);
		dlg2.exec();

		if (dlg2.GetReturnCode() == false)
		{
			delete postDoc;
			QMessageBox::critical(this, "FEBio Studio", "Failed to apply kinemat tool.");
			return;
		}

		// prepare a file reader
		FEKinematFileReader* kine = new FEKinematFileReader(postDoc);
		kine->SetModelFile(modelFile);
		kine->SetKineFile(kineFile);
		kine->SetRange(dlg.StartIndex(), dlg.EndIndex(), dlg.Increment());
		postDoc->SetFileReader(kine);

		UpdateModel();
		Update();
		AddDocument(postDoc);
	}
}

void CMainWindow::on_actionPlotMix_triggered()
{
	CDlgPlotMix dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionOptions_triggered()
{
	CDlgSettings dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionLayerInfo_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CLogPanel* log = ui->logPanel;
	log->AddText("\nMesh Layer Info:\n===================\n");
	GModel* gm = doc->GetGModel();
	int nobjs = gm->Objects();
	int layers = gm->MeshLayers();
	MeshLayerManager* mlm = gm->GetMeshLayerManager();

	// get the longest layer name
	int l = 0;
	for (int i = 0; i < layers; ++i)
	{
		const std::string& s = gm->GetMeshLayerName(i);
		int sl = s.length();
		if (sl > l) l = sl;
	}

	int activeLayer = gm->GetActiveMeshLayer();
	for (int i = 0; i < layers; ++i)
	{
		log->AddText(QString::number(i) + ".");
		QString name = QString::fromStdString(gm->GetMeshLayerName(i));
		log->AddText(QString("%1:").arg(name, -l));

		int nc = nobjs;
		int meshes = mlm->FEMeshes(i);
		if (meshes != nobjs)
		{
			log->AddText(QString("(Incorrect number of meshes: %1 / %2)").arg(meshes).arg(nobjs));
		}
		else
		{
			for (int j = 0; j < nc; ++j)
			{
				const FEMesher* mesher = nullptr;
				const FSMesh*	mesh = nullptr;

				if (i == activeLayer)
				{
					mesher = gm->Object(j)->GetFEMesher();
					mesh = gm->Object(j)->GetFEMesh();
				}
				else
				{
					mesher = mlm->GetFEMesher(i, j);
					mesh = mlm->GetFEMesh(i, j);
				}

				ulong pmesher = (ulong)mesher;
				ulong pmesh = (ulong)mesh;

				QString s1; s1.setNum(pmesher, 16);
				QString s2; s2.setNum(pmesh, 16);

				log->AddText(QString("%1|%2").arg(s1, 8, '0').arg(s2, 8, '0'));

				if (j != nobjs - 1) log->AddText(",");
			}
		}

		if (i == activeLayer)
			log->AddText("***\n");
		else
			log->AddText("\n");
	}

	// print object info
	log->AddText("\nObject Info:\n===================\n");
	for (int i = 0; i < nobjs; ++i)
	{
		const int M = 20;
		GObject* po = gm->Object(i);
		log->AddText(QString("Object: %1 (ID = %2)\n").arg(QString::fromStdString(po->GetName())).arg(po->GetID()));
		log->AddText("_________________________________________________________________________________________\n");
		log->AddText("       Parts         |      Surfaces        |        Edges         |       Nodes\n");
		log->AddText("---------------------+----------------------+----------------------+---------------------\n");
		int m = 0;
		bool done = false;
		do
		{
			done = true;

			// print part info
			QString s, t;
			if (m < po->Parts())
			{
				done = false;
				GPart* pg = po->Part(m);
				s = QString("%1 (%2)").arg(QString::fromStdString(pg->GetName())).arg(pg->GetID());
				t = QString("%1").arg(s, -M);
			}
			else t = QString("%1").arg("", M, ' ');
			log->AddText(t);
			log->AddText(" | ");

			// print surface info
			if (m < po->Faces())
			{
				done = false;
				GFace* pg = po->Face(m);
				s = QString("%1 (%2)").arg(QString::fromStdString(pg->GetName())).arg(pg->GetID());
				t = QString("%1").arg(s, -M);
			}
			else t = QString("%1").arg("", M, ' ');
			log->AddText(t);
			log->AddText(" | ");

			// print edge info
			if (m < po->Edges())
			{
				done = false;
				GEdge* pg = po->Edge(m);
				s = QString("%1 (%2)").arg(QString::fromStdString(pg->GetName())).arg(pg->GetID());
				t = QString("%1").arg(s, -M);
			}
			else t = QString("%1").arg("", M, ' ');
			log->AddText(t);
			log->AddText(" | ");

			// print node info
			if (m < po->Nodes())
			{
				done = false;
				GNode* pg = po->Node(m);
				s = QString("%1 (%2)").arg(QString::fromStdString(pg->GetName())).arg(pg->GetID());
				t = QString("%1").arg(s, -M);
			}
			else t = QString("%1").arg("", M, ' ');
			log->AddText(t);
			log->AddText("\n");

			m++;
		}
		while (!done);
	}

	// model context
	log->AddText("\nModel Context:\n===================\n");
	log->AddText(QString("Object counter: %1\n").arg(GObject::GetCounter()));
	log->AddText(QString("Part counter: %1\n").arg(GPart::GetCounter()));
	log->AddText(QString("Face counter: %1\n").arg(GFace::GetCounter()));
	log->AddText(QString("Edge counter: %1\n").arg(GEdge::GetCounter()));
	log->AddText(QString("Node counter: %1\n").arg(GNode::GetCounter()));
}
