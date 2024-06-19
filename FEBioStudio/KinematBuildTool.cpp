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
#include "KinematBuildTool.h"
#include "ModelDocument.h"
#include "MainWindow.h"
#include <LSDyna/LSDYNAimport.h>
#include <GeomLib/GObject.h>
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/FERigidLoad.h>
#include <PostLib/FEKinemat.h>
#include <sstream>

CKinematBuildTool::CKinematBuildTool(CMainWindow* w) : CBasicTool(w, "Kinemat", 1)
{
	m_btransformFirstState = true;

	addResourceProperty(&m_modelFile, "Model file:");
	addResourceProperty(&m_kineFile, "Kine file:");
	addBoolProperty(&m_btransformFirstState, "Transform initial state");
}

bool CKinematBuildTool::OnApply()
{
	if (m_modelFile.isEmpty()) { SetErrorString("Please select a model file."); return false; }
	if (m_kineFile.isEmpty ()) { SetErrorString("Please select a kine file." ); return false; }

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	FSProject& prj = doc->GetProject();

	std::string smodelfile = m_modelFile.toStdString();

	LSDYNAimport lsd(prj);
	if (lsd.Load(smodelfile.c_str()) == false) { SetErrorString("Failed to read model file."); return false; }

	FSModel& fem = prj.GetFSModel();
	GModel& mdl = fem.GetModel();
	if (mdl.Objects() != 1) { SetErrorString("Hmm, something went wrong."); return false; }

	// create the same nr of rigid bodies as parts
	GObject* po = mdl.Object(0);
	int n = po->Parts();
	for (int i = 0; i < n; ++i)
	{
		GPart* pg = po->Part(i);
		FSMaterial* pm = FEBio::CreateMaterial("rigid body", &fem);
		pm->SetParamBool("override_com", true);
		pm->SetParamVec3d("center_of_mass", vec3d(0, 0, 0));
		GMaterial* gm = new GMaterial(pm);
		gm->SetName(pg->GetName());
		fem.AddMaterial(gm);
		po->AssignMaterial(pg, gm->GetID());
	}

	// read the kine file
	std::string kineFile = m_kineFile.toStdString();
	FEKinemat kine;
	if (kine.ReadKine(kineFile.c_str()) == false) { SetErrorString("Failed reading kine file."); return false; }

	// get the initial step. We'll apply the rigid constraints to it
	FSStep* step = fem.GetStep(0);

	// get the initial transform
	if (m_btransformFirstState)
	{
		FEKinemat::STATE& s0 = kine.GetState(0);

		FSMesh* pm = po->GetFEMesh();
		pm->TagAllNodes(-1);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement& el = pm->Element(i);
			int ne = el.Nodes();
			for (int j = 0; j < ne; ++j)
			{
				FSNode& nj = pm->Node(el.m_node[j]);
				assert((nj.m_ntag == -1) || (nj.m_ntag == el.m_gid));
				nj.m_ntag = el.m_gid;
			}
		}
		
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& nd = pm->Node(i); assert(nd.m_ntag != -1);
			vec3d r = s0.D[nd.m_ntag].apply(nd.r);
			nd.r = r;
		}
		po->Update();
	}

	// Add an analysis step (remember the type of the analysis is the same as the module)
	FSStep* step1 = FEBio::CreateStep("solid", &fem);
	step1->SetName("Step1");
	FEBio::InitDefaultProps(step1);

	step1->SetParamInt("time_steps", kine.States() - 1);
	step1->SetParamFloat("step_size", 1);
	step1->FindProperty("time_stepper")->SetComponent(nullptr);
	fem.AddStep(step1);

	// generate load curves from the kine data
	int nstates = kine.States();
	int nmat = fem.Materials();
	vector<LoadCurve> LC(nmat * 6);
	for (LoadCurve& lc : LC) lc.Clear();
	for (int i = 0; i < nstates; ++i)
	{
		FEKinemat::STATE& s = kine.GetState(i);
		for (int j = 0; j < nmat; ++j)
		{
			FEKinemat::KINE& D = s.D[j];

			vec3d t, R;

			if (m_btransformFirstState)
			{
				FEKinemat::KINE& D0 = kine.GetState(0).D[j];

				vec3d t0 = D0.translate();
				mat3d Q0 = D0.rotate();
				mat3d Q0i = Q0.transpose();

				vec3d t1 = D.translate();
				mat3d Q1 = D.rotate();

				mat3d Q = Q1 * Q0i;
				quatd q(Q);
				R = q.GetRotationVector();

				t = t1 - Q * t0;
			}
			else
			{
				t = D.translate();
				mat3d Q = D.rotate();
				quatd q(Q);
				R = q.GetRotationVector();
			}

			LC[j * 6    ].Add((double)i, t.x);
			LC[j * 6 + 1].Add((double)i, t.y);
			LC[j * 6 + 2].Add((double)i, t.z);
			LC[j * 6 + 3].Add((double)i, R.x);
			LC[j * 6 + 4].Add((double)i, R.y);
			LC[j * 6 + 5].Add((double)i, R.z);
		}
	}

	// for each rigid body, create 6 prescribed constraints
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* gm = fem.GetMaterial(i);

		std::string d[6] = { "_x", "_y", "_z", "_Ru", "_Rv", "_Rw"};
		for (int j = 0; j < 3; ++j)
		{
			FSRigidBC* rbc = FEBio::CreateRigidBC("rigid_displacement", &fem);

			rbc->SetParamInt("rb", gm->GetID());
			rbc->SetParamInt("dof", j);
			rbc->SetParamFloat("value", 1.0);
			std::stringstream ss;
			ss << gm->GetName() << d[j];
			rbc->SetName(ss.str());
			step->AddRigidBC(rbc);

			// create a load controller for this
			FSLoadController* plc = fem.AddLoadCurve(LC[i*6 + j]);
			Param* p = rbc->GetParam("value"); assert(p);
			p->SetLoadCurveID(plc->GetID());
		}

		for (int j = 0; j < 3; ++j)
		{
			FSRigidBC* rbc = FEBio::CreateRigidBC("rigid_rotation", &fem);
			rbc->SetParamInt("rb", gm->GetID());
			rbc->SetParamInt("dof", j);
			rbc->SetParamFloat("value", 1.0);
			std::stringstream ss;
			ss << gm->GetName() << d[j + 3];
			rbc->SetName(ss.str());
			step->AddRigidBC(rbc);

			// create a load controller for this
			FSLoadController* plc = fem.AddLoadCurve(LC[i * 6 + j + 3]);
			Param* p = rbc->GetParam("value"); assert(p);
			p->SetLoadCurveID(plc->GetID());
		}
	}
	fem.UpdateLoadControllerReferenceCounts();

	GetMainWindow()->on_actionZoomExtents_triggered();

	return true;
}
