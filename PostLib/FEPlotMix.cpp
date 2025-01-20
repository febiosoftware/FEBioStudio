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
#include "FEPostModel.h"
#include "FEPlotMix.h"
#include "FELSDYNAPlot.h"
#include "XPLTLib/xpltFileReader.h"
#include "FEDataManager.h"
#include "FEMeshData_T.h"

using namespace Post;

//------------------------------------------------------------------------------
// FEPlotMix
//------------------------------------------------------------------------------

FEPlotMix::FEPlotMix(void)
{

}

//------------------------------------------------------------------------------
FEPlotMix::~FEPlotMix(void)
{
}

//------------------------------------------------------------------------------
FEPostModel* FEPlotMix::Load(const char **szfile, int n)
{
	if (n <= 0) return 0;

	// load the first model
	FEPostModel* pfem = new FEPostModel;
	pfem->SetTitle("PlotMix");
	xpltFileReader* pfr = new xpltFileReader(pfem);
	pfr->SetReadStateFlag(XPLT_READ_FIRST_AND_LAST);
	if (pfr->Load(szfile[0]) == false) { delete pfem; delete pfr; return 0; }
	delete pfr;

	// the "time" value will be the state
	pfem->GetState(0)->m_time = 0;
	pfem->GetState(1)->m_time = 1;

	// get the mesh
	FSMesh& m1 = *pfem->GetFEMesh(0);

	// get the datamanager
	FEDataManager* pdm1 = pfem->GetDataManager();

	// set the time indicator of the first state
	FEState& s1 = *pfem->GetState(0);
	s1.m_time = 0;

	// get the mesh of the new model
	FSMesh* mesh = pfem->GetFEMesh(0);

	// load the other models
	for (int i=1; i<n; ++i)
	{
		// create a new scene
		FEPostModel fem2;

		// create reader 
		pfr = new xpltFileReader(&fem2);

		// only read last states
		pfr->SetReadStateFlag(XPLT_READ_LAST_STATE_ONLY);

		// try to load the scene
		if (pfr->Load(szfile[i]) == false)
		{
			delete pfem;
			delete pfr;
			return 0;
		}
		delete pfr;

		// make sure the mesh size is the same
		FSMesh& m2 = *fem2.GetFEMesh(0);
		if ((m1.Nodes   () != m2.Nodes()) ||
			(m1.Elements() != m2.Elements())) { delete pfem; return 0; } 

		// see if the data size is the same
		FEDataManager* pdm2 = fem2.GetDataManager();

		if (pdm1->DataFields() != pdm2->DataFields()) { delete pfem; return 0; }

		// need to modify the displacement field such that it is relative to the first file
		FEState* ps1 = pfem->GetState(0);
		FEState* ps2 = fem2.GetState(0);
		FENodeData<vec3f>& dr2 = dynamic_cast<FENodeData<vec3f>&>(ps2->m_Data[0]);
		for (int j=0; j<m1.Nodes(); ++j)
		{
			FSNode& n1 = m1.Node(j);
			FSNode& n2 = m2.Node(j);
			vec3f r1 = to_vec3f(n1.r);
			vec3f r2 = to_vec3f(n2.r);
			dr2[j] = (r2 + dr2[j]) - r1;
		}

		// add a new state by copying it from fem2
		FEState* pstate = new FEState((float) i + 1.f, pfem, fem2.GetState(0));

		// the state's mesh was set to the fem2 mesh, but we don't want that
		pstate->SetFEMesh(mesh);

		pfem->AddState(pstate);
	}

	return pfem;
}

//------------------------------------------------------------------------------
void FEPlotMix::ClearStates(FEPostModel &fem)
{
	while (fem.GetStates() > 1) fem.DeleteState(0);
}
