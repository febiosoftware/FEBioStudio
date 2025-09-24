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
#include <PostLib/VTKImport.h>
#include "FEDataManager.h"
#include "FEMeshData_T.h"
#include <filesystem>

using namespace Post;
namespace fs = std::filesystem;

//------------------------------------------------------------------------------
// FEPlotMix
//------------------------------------------------------------------------------

FEPlotMix::FEPlotMix(FEPostModel* fem) : m_fem(fem)
{

}

//------------------------------------------------------------------------------
FEPlotMix::~FEPlotMix(void)
{
}

//------------------------------------------------------------------------------
bool FEPlotMix::Load(const char **szfile, int n)
{
	if (n <= 0) return false;
	if (m_fem == nullptr) return false;

	m_fem->SetTitle("PlotMix");

	if      (m_op == 0) return StitchFinalStates(szfile, n);
	else if (m_op == 1) return MergeModels(szfile, n);
	else return false;
}

bool FEPlotMix::StitchFinalStates(const char** szfile, int n)
{
	// load the first model
	xpltFileReader* pfr = new xpltFileReader(m_fem);
	pfr->SetReadStateFlag(XPLT_READ_FIRST_AND_LAST);
	if (pfr->Load(szfile[0]) == false) { delete pfr; return false; }
	delete pfr;

	// the "time" value will be the state
	m_fem->GetState(0)->m_time = 0;
	m_fem->GetState(1)->m_time = 1;

	// get the mesh
	FSMesh& m1 = *m_fem->GetFEMesh(0);

	// get the datamanager
	FEDataManager* pdm1 = m_fem->GetDataManager();

	// set the time indicator of the first state
	FEState& s1 = *m_fem->GetState(0);
	s1.m_time = 0;

	// get the mesh of the new model
	FSMesh* mesh = m_fem->GetFEMesh(0);

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
			delete pfr;
			return false;
		}
		delete pfr;

		// make sure the mesh size is the same
		FSMesh& m2 = *fem2.GetFEMesh(0);
		if ((m1.Nodes   () != m2.Nodes()) ||
			(m1.Elements() != m2.Elements())) { return false; } 

		// see if the data size is the same
		FEDataManager* pdm2 = fem2.GetDataManager();

		if (pdm1->DataFields() != pdm2->DataFields()) { return false; }

		// need to modify the displacement field such that it is relative to the first file
		FEState* ps1 = m_fem->GetState(0);
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
		FEState* pstate = new FEState((float) i + 1.f, m_fem, fem2.GetState(0));

		// the state's mesh was set to the fem2 mesh, but we don't want that
		pstate->SetFEMesh(mesh);

		m_fem->AddState(pstate);
	}

	return true;
}

void FEPlotMix::ClearStates(FEPostModel &fem)
{
	while (fem.GetStates() > 1) fem.DeleteState(0);
}

bool FEPlotMix::LoadFile(const char* szfile, FEPostModel* fem)
{
	if (szfile == nullptr) return false;
	fs::path path(szfile);

	if (path.extension() == ".xplt")
	{
		xpltFileReader xplt(fem);
		if (xplt.Load(szfile) == false) return false;
	}
	else if (path.extension() == ".pvd")
	{
		PVDImport pvd(fem);
		if (pvd.Load(szfile) == false) return false;
	}
	else
	{
		return false;
	}

	return true;
}

bool FEPlotMix::MergeModels(const char** szfile, int n)
{
	std::vector<FEPostModel*> models(n, nullptr);
	bool success = true;
	for (int i = 0; i < n; ++i)
	{
		const char* szfilei = szfile[i];
		models[i] = new FEPostModel();
		if (LoadFile(szfilei, models[i]) == false)
		{
			success = false;
			break;
		}

		// some sanity checks
		if (i > 0)
		{
			if (models[i]->GetStates() != models[i-1]->GetStates())
			{
				success = false;
				break;
			}
		}
	}

	if (success)
	{
		// merge materials
		for (int i = 0; i < n; ++i)
		{
			FEPostModel& fem = *models[i];
			int nmat = fem.Materials();
			for (int j = 0; j < nmat; ++j)
			{
				Post::Material mat = *fem.GetMaterial(j);

				fs::path filePath = szfile[i];

				string femName = filePath.stem().string();
				string matName = mat.GetName();
				if (!femName.empty()) matName = femName + "." + matName;
				mat.SetName(matName);

				m_fem->AddMaterial(mat);
			}
		}
		
		// merge the meshes
		int nodes = 0;
		int elems = 0;
		for (int i = 0; i < n; ++i)
		{
			FSMesh& mesh = *models[i]->GetFEMesh(0);
			nodes += mesh.Nodes();
			elems += mesh.Elements();
		}

		FSMesh* mesh = new FSMesh;
		mesh->Create(nodes, elems);

		nodes = 0;
		for (int i = 0; i < n; ++i)
		{
			FSMesh& meshi = *models[i]->GetFEMesh(0);
			int N = meshi.Nodes();
			for (int j = 0; j < N; ++j)
			{
				FSNode& n = meshi.Node(j);
				n.m_ntag = nodes;
				mesh->Node(nodes++) = n;
			}
		}

		elems = 0;
		int maxgid = 0;
		int gidoffset = 0;
		int matoffset = 0;
		for (int i = 0; i < n; ++i)
		{
			FSMesh& meshi = *models[i]->GetFEMesh(0);
			int N = meshi.Elements();
			for (int j = 0; j < N; ++j)
			{
				FSElement& elj = meshi.Element(j);
				FSElement& el = mesh->Element(elems++);
				el.m_MatID = elj.m_MatID + matoffset;
				el.m_gid = elj.m_gid + gidoffset;
				if (el.m_gid > maxgid) maxgid = el.m_gid;
				el.SetType(elj.Type());
				for (int k = 0; k < elj.Nodes(); ++k)
				{
					int nk = elj.m_node[k];
					el.m_node[k] = meshi.Node(nk).m_ntag;
				}
			}
			gidoffset = maxgid + 1;
			matoffset += models[i]->Materials();
		}
		mesh->RebuildMesh();

		m_fem->AddMesh(mesh);

		// merge data fields
		FEDataManager* pdm = m_fem->GetDataManager();
		for (int i = 0; i < n; ++i)
		{
			FEPostModel& fem = *models[i];
			FEDataManager* pdmi = fem.GetDataManager();
			int ndata = pdmi->DataFields();
			for (int j = 0; j < ndata; ++j)
			{
				FEDataFieldPtr pdf = pdmi->DataField(j);
				ModelDataField* ps = *pdf;

				if (pdm->FindDataField(ps->GetName()) == -1)
				{
					ModelDataField* newDataField = ps->Clone();
					pdm->AddDataField(newDataField);
				}
			}
		}

		// merge states
		for (int i = 0; i < models[0]->GetStates(); ++i)
		{
			Post::FEState* state_i = models[0]->GetState(i);

			Post::FEState* state = new Post::FEState(state_i->m_time, m_fem, mesh);
			m_fem->AddState(state);
		}

		for (int i = 0; i < m_fem->GetStates(); ++i)
		{
			Post::FEState* state = m_fem->GetState(i);
			for (int j = 0; j < state->m_Data.size(); ++j)
			{
				FEDataFieldPtr pdf = m_fem->GetDataManager()->DataField(j);
				Post::FEMeshData& pd = state->m_Data[j];

				if (dynamic_cast<FENodeItemData*>(&pd))
				{
					FENodeItemData& nd = dynamic_cast<FENodeItemData&>(pd);

					int nodeOffset = 0;
					for (int k = 0; k < n; ++k)
					{
						FEPostModel& fem = *models[k];
						FEDataManager* pdmi = fem.GetDataManager();
						Post::FEState* state_i = fem.GetState(i);
						FSMesh* mesh_i = state_i->GetFEMesh();

						int nfield = pdmi->FindDataField((*pdf)->GetName());
						if (nfield != -1)
						{
							if (state_i == nullptr) continue;
							Post::FENodeItemData& nd_i = dynamic_cast<Post::FENodeItemData&>(state_i->m_Data[nfield]);
							assert(nd_i.GetType() == nd.GetType());

							if (nd.GetType() == DATA_SCALAR)
							{
								Post::FENodeData<float>& src = dynamic_cast<Post::FENodeData<float>&>(nd_i);
								Post::FENodeData<float>& dst = dynamic_cast<Post::FENodeData<float>&>(nd);

								for (int k = 0; k < mesh_i->Nodes(); ++k)
								{
									float f = src[k];
									dst[k + nodeOffset] = f;
								}
							}
							else if (nd.GetType() == DATA_VEC3)
							{
								Post::FENodeData<vec3f>& src = dynamic_cast<Post::FENodeData<vec3f>&>(nd_i);
								Post::FENodeData<vec3f>& dst = dynamic_cast<Post::FENodeData<vec3f>&>(nd);
								for (int k = 0; k < mesh_i->Nodes(); ++k)
								{
									vec3f f = src[k];
									dst[k + nodeOffset] = f;
								}
							}
							else if (nd.GetType() == DATA_MAT3)
							{
								Post::FENodeData<mat3f>& src = dynamic_cast<Post::FENodeData<mat3f>&>(nd_i);
								Post::FENodeData<mat3f>& dst = dynamic_cast<Post::FENodeData<mat3f>&>(nd);
								for (int k = 0; k < mesh_i->Nodes(); ++k)
								{
									mat3f f = src[k];
									dst[k + nodeOffset] = f;
								}
							}
						}
						nodeOffset += mesh_i->Nodes();
					}
				}
			}
		}
	}

	// clean up
	for (auto fem : models) delete fem;
	models.clear();

	return true;
}
