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

#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "DlgLameConvertor.h"
#include "DlgUnitConverter.h"
#include "DlgKinemat.h"
#include "DlgPlotMix.h"
#include "DlgSettings.h"
#include "DlgMeshDiagnostics.h"
#include <QMessageBox>
#include <QFileDialog>
#include <GeomLib/MeshLayer.h>
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>
#include "ModelDocument.h"
#include <PyLib/RunPython.h>

void CMainWindow::on_actionCurveEditor_triggered()
{
	ui->showCurveEditor();
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

void CMainWindow::on_actionUnitConverter_triggered()
{
	CDlgUnitConverter dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionKinemat_triggered()
{
	CDlgKinemat dlg(this);
	dlg.exec();
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

void CMainWindow::on_actionRunPythonScript_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Python Script", "", "Python scripts (*.py)");

	runPython(fileName.toStdString().c_str());
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
				const FEMesh*	mesh = nullptr;

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

void CMainWindow::onRunFinished(int exitCode, QProcess::ExitStatus es)
{
	CFEBioJob* job = CFEBioJob::GetActiveJob();
	if (job)
	{
		job->SetStatus(exitCode == 0 ? CFEBioJob::COMPLETED : CFEBioJob::FAILED);
		CFEBioJob::SetActiveJob(nullptr);

		QString sret = (exitCode == 0 ? "NORMAL TERMINATION" : "ERROR TERMINATION");
		QString jobName = QString::fromStdString(job->GetName());
		QString msg = QString("FEBio job \"%1 \" has finished:\n\n%2\n").arg(jobName).arg(sret);

		QString logmsg = QString("FEBio job \"%1 \" has finished: %2\n").arg(jobName).arg(sret);
		AddLogEntry(logmsg);

		if (exitCode == 0)
		{
			msg += "\nDo you wish to load the results?";
			if (QMessageBox::question(this, "Run FEBio", msg) == QMessageBox::Yes)
			{
				OpenFile(QString::fromStdString(job->GetPlotFileName()), false, false);
			}
		}
		else
		{
			QMessageBox::critical(this, "Run FEBio", msg);
		}
	}
	else
	{
		// Not sure if we should ever get here.
		QMessageBox::information(this, "FEBio Studio", "FEBio is done.");
	}
	CFEBioJob::SetActiveJob(nullptr);

	delete ui->m_process;
	ui->m_process = 0;
}

void CMainWindow::onReadyRead()
{
	if (ui->m_process == 0) return;

	QByteArray output = ui->m_process->readAll();
	QString s(output);
	AddOutputEntry(s);
}

void CMainWindow::onErrorOccurred(QProcess::ProcessError err)
{
	// make sure we don't have an active job since onRunFinished will not be called!
	CFEBioJob::SetActiveJob(nullptr);

	// suppress an error if user stopped FEBio job
	if (ui->m_bkillProcess && (err==QProcess::Crashed))
	{
		return;
	}

	// check for FailedToStart
	if (err == QProcess::FailedToStart)
	{
		QMessageBox::critical(this, "Run FEBio", "FEBio failed to start.\nCheck the launch configuration and make sure that the path to the FEBio executable is correct.");
	}
	else
	{
		QString errString;
		switch (err)
		{
		case QProcess::FailedToStart: errString = "Failed to start"; break;
		case QProcess::Crashed: errString = "Crashed"; break;
		case QProcess::Timedout: errString = "Timed out"; break;
		case QProcess::WriteError: errString = "Write error"; break;
		case QProcess::ReadError: errString = "Read error"; break;
		case QProcess::UnknownError: errString = "Unknown error"; break;
		default:
			errString = QString("Error code = %1").arg(err);
		}

		QString t = "An error has occurred.\nError = " + errString;
		AddLogEntry(t);
		QMessageBox::critical(this, "Run FEBio", t);
	}
}
