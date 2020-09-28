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
#include "ModelDocument.h"
#include "DocTemplate.h"
#include "version.h"
#include <XML/XMLWriter.h>
#include <MeshIO/PRVObjectFormat.h>
#include <FEMLib/FEUserMaterial.h>
#include "Commands.h"
#include "MainWindow.h"
#include "ModelFileWriter.h"
#include <QMessageBox>
#include <MeshTools/GModel.h>
#include <PostGL/GLPlot.h>
#include <MeshLib/FENodeFaceList.h>

CModelDocument::CModelDocument(CMainWindow* wnd) : CDocument(wnd)
{
	m_psel = nullptr;

	SetFileWriter(new CModelFileWriter(this));
}

void CModelDocument::Clear()
{
	CDocument::Clear();

	// reset the project
	m_Project.Reset();

	// reset the counters
	GModel::Reset();
	GMaterial::ResetRefs();

	// clear all the jobs
	m_JobList.Clear();

	// reset selection
	if (m_psel) delete m_psel;
	m_psel = 0;
}

//-----------------------------------------------------------------------------
//! Get the project
FEProject& CModelDocument::GetProject()
{ 
	return m_Project; 
}

//-----------------------------------------------------------------------------
FEModel* CModelDocument::GetFEModel()
{ 
	return &m_Project.GetFEModel(); 
}

//-----------------------------------------------------------------------------
GModel* CModelDocument::GetGModel()
{
	return &m_Project.GetFEModel().GetModel();
}

//-----------------------------------------------------------------------------
// return the active object
GObject* CModelDocument::GetActiveObject()
{
	GObject* po = nullptr;
	GObjectSelection sel(GetFEModel());
	if (sel.Count() == 1) po = sel.Object(0);
	return po;
}

//-----------------------------------------------------------------------------
BOX CModelDocument::GetModelBox() { return m_Project.GetFEModel().GetModel().GetBoundingBox(); }

//-----------------------------------------------------------------------------
void CModelDocument::AddObject(GObject* po)
{
	DoCommand(new CCmdAddAndSelectObject(GetGModel(), po));
	GetMainWindow()->Update(0, true);
}

void CModelDocument::DeleteObject(FSObject* po)
{
	FEModel& fem = *GetFEModel();

	if (dynamic_cast<FEStep*>(po))
	{
		if (dynamic_cast<FEInitialStep*>(po))
		{
			QMessageBox::warning(m_wnd, "Delete step", "Cannot delete the initial step.");
			return;
		}
		else
		{
			DoCommand(new CCmdDeleteFSObject(po));
		}
	}
	else if (dynamic_cast<FEMaterial*>(po))
	{
		FEMaterial* pm = dynamic_cast<FEMaterial*>(po);
		FEMaterial* parent = const_cast<FEMaterial*>(pm->GetParentMaterial());
		FEMaterialProperty* pp = parent->FindProperty(pm);
		if (pp)// && (pp->maxSize() == 0))
		{
			pp->RemoveMaterial(pm);
		}
		else
		{
			QMessageBox::warning(m_wnd, "FEBio Studio", "Cannot delete this material property.");
			return;
		}
	}
	else if (dynamic_cast<GPart*>(po))
	{
		GPart* pg = dynamic_cast<GPart*>(po);
		GetGModel()->DeletePart(pg);

		// This cannot be undone (yet) so clear the undo stack
		ClearCommandStack();
	}
	else if (dynamic_cast<Post::CGLPlot*>(po))
	{
		// We don't push plane cut plots on the undo stack because
		// the plane cuts will remain active until the object is actually deleted. 
		assert(po->GetParent());
		delete po;
	}
	else if (po->GetParent())
	{
		if (dynamic_cast<CFEBioJob*>(po))
		{
			CFEBioJob* job = dynamic_cast<CFEBioJob*>(po);
/*			if (job->GetPostDoc())
			{
				m_wnd->CloseView(job->GetPostDoc());
			}
*/
		}
		DoCommand(new CCmdDeleteFSObject(po));
	}
	else if (dynamic_cast<FEMeshData*>(po))
	{
		FEMeshData* pd = dynamic_cast<FEMeshData*>(po);
		DoCommand(new CCmdRemoveMeshData(pd));
	}
	else
	{
		assert(false);
	}

	SetModifiedFlag(true);
}

//-----------------------------------------------------------------------------
int CModelDocument::FEBioJobs() const
{
	return (int)m_JobList.Size();
}

//-----------------------------------------------------------------------------
void CModelDocument::AddFEbioJob(CFEBioJob* job)
{
	m_JobList.Add(job);
	SetModifiedFlag();
}

CFEBioJob* CModelDocument::GetFEBioJob(int i)
{
	return m_JobList[i];
}

CFEBioJob* CModelDocument::FindFEBioJob(const std::string& s)
{
	for (int i = 0; i < FEBioJobs(); ++i)
	{
		CFEBioJob* job = m_JobList[i];
		if (job->GetName() == s) return job;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
void CModelDocument::Save(OArchive& ar)
{
	// save version info
	unsigned int version = SAVE_VERSION;
	ar.WriteChunk(CID_VERSION, version);

	// save model info
	ar.BeginChunk(CID_MODELINFO);
	{
		ar.WriteChunk(CID_MODELINFO_COMMENT, m_info);
		ar.WriteChunk(CID_MODELINFO_UNITS  , m_units);
	}
	ar.EndChunk();

	// save resources
	if (ImageModels() > 0)
	{
		ar.BeginChunk(CID_RESOURCE_SECTION);
		{
			SaveResources(ar);
		}
		ar.EndChunk();
	}

	// save the project
	ar.BeginChunk(CID_PROJECT);
	{
		m_Project.Save(ar);
	}
	ar.EndChunk();

	// save the job lists
	for (int i = 0; i < FEBioJobs(); ++i)
	{
		CFEBioJob* job = GetFEBioJob(i);
		ar.BeginChunk(CID_FEBIOJOB);
		{
			job->Save(ar);
		}
		ar.EndChunk();
	}	
}

//-----------------------------------------------------------------------------
void CModelDocument::Load(IArchive& ar)
{
	CCallStack::ClearStack();
	TRACE("CDocument::Load");
	unsigned int version;

	m_bValid = false;

	IArchive::IOResult nret = IArchive::IO_OK;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == CID_VERSION)
		{
			nret = ar.read(version);
			if (nret != IArchive::IO_OK) throw ReadError("Error occurred when parsing CID_VERSION (CDocument::Load)");
			if (version < MIN_PRV_VERSION)
			{
				throw InvalidVersion();
			}
			else if (version < SAVE_VERSION)
			{
//				int nret = flx_choice("This file was created with an older version of PreView.\nThe file data may not be read in correctly.\n\nWould you like to continue?", "Yes", "No", 0);
//				if (nret != 0) throw InvalidVersion();
			}
			else if (version > SAVE_VERSION)
			{
				throw InvalidVersion();
			}

			// set the version number of the file
			ar.SetVersion(version);
		}
		else if (nid == CID_MODELINFO)
		{
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				if (nid == CID_MODELINFO_COMMENT)
				{
					nret = ar.read(m_info);
				}
				else if (nid == CID_MODELINFO_UNITS)
				{
					nret = ar.read(m_units);
				}
				ar.CloseChunk();
			}
		}
		else if (nid == CID_RESOURCE_SECTION)
		{
			LoadResources(ar);
		}
		else if (nid == CID_PROJECT)
		{
			m_Project.Load(ar);
		}
		else if (nid == CID_FEBIOJOB)
		{
			CFEBioJob* job = new CFEBioJob(this);
			m_JobList.Add(job);
			job->Load(ar);
		}
		ar.CloseChunk();
	}

	m_bValid = true;
}


//-----------------------------------------------------------------------------
bool CModelDocument::LoadTemplate(int n)
{
	int N = TemplateManager::Templates();
	if ((n<0) || (n >= N)) return false;

	const DocTemplate& doc = TemplateManager::GetTemplate(n);

	/*	string fileName = TemplateManager::TemplatePath() + doc.fileName;
	const char* szfile = fileName.c_str();

	PRVArchive ar;
	if (ar.Load(szfile) == false) return false;
	*/
	m_Project.SetModule(doc.module);

	return true;
}

bool CModelDocument::GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt)
{
	const char* szlog[] = { "LOG_DEFAULT", "LOG_NEVER", "LOG_FILE_ONLY", "LOG_SCREEN_ONLY", "LOG_FILE_AND_SCREEN" };
	const char* szprt[] = { "PRINT_ITERATIONS", "PRINT_VERBOSE" };

	XMLWriter xml;

	if (xml.open(fileName.c_str()) == false) return false;

	XMLElement root("febio_optimize");
	root.add_attribute("version", "2.0");
	xml.add_branch(root);

	// print options
	XMLElement ops("Options");
	switch (opt.method)
	{
	case 0: ops.add_attribute("type", "levmar"); break;
	case 1: ops.add_attribute("type", "constrained levmar"); break;
	}
	xml.add_branch(ops);
	{
		xml.add_leaf("obj_tol", opt.obj_tol);
		xml.add_leaf("f_diff_scale", opt.f_diff_scale);
		xml.add_leaf("log_level", szlog[opt.outLevel]);
		xml.add_leaf("print_level", szprt[opt.printLevel]);
	}
	xml.close_branch();

	// parameters
	XMLElement params("Parameters");
	xml.add_branch(params);
	for (int i = 0; i < (int)opt.m_params.size(); ++i)
	{
		FEBioOpt::Param& pi = opt.m_params[i];
		XMLElement par("param");
		par.add_attribute("name", pi.m_name);
		double v[3] = { pi.m_initVal, pi.m_minVal, pi.m_maxVal };
		par.value(v, 3);
		xml.add_leaf(par);
	}
	xml.close_branch();

	// objective
	XMLElement obj("Objective");
	obj.add_attribute("type", "data-fit");
	xml.add_branch(obj);
	{
		XMLElement fnc("fnc");
		fnc.add_attribute("type", "parameter");
		xml.add_branch(fnc);
		{
			XMLElement p("param");
			p.add_attribute("name", opt.m_objParam);
			xml.add_empty(p);
		}
		xml.close_branch();

		xml.add_branch("data");
		{
			for (int i = 0; i < (int)opt.m_data.size(); ++i)
			{
				FEBioOpt::Data& di = opt.m_data[i];
				double v[2] = { di.m_time, di.m_value };
				xml.add_leaf("pt", v, 2);
			}
		}
		xml.close_branch();
	}
	xml.close_branch();

	// closing tag
	xml.close_branch();

	xml.close();

	return true;
}

std::vector<MODEL_ERROR> CModelDocument::CheckModel()
{
	vector<MODEL_ERROR> errorList;
	checkModel(GetProject(), errorList);
	return errorList;
}

bool CModelDocument::ExportMaterials(const std::string& fileName, const vector<GMaterial*>& matList)
{
	if (matList.size() == 0) return false;

	OArchive ar;
	//									P V M 
	if (ar.Create(fileName.c_str(), 0x0050564D) == false) return false;

	// save version info
	unsigned int version = PVM_VERSION_NUMBER;
	ar.WriteChunk(PVM_VERSION, version);

	// write materials
	int mats = (int)matList.size();
	for (int i = 0; i<mats; ++i)
	{
		GMaterial* pmat = matList[i];
		int ntype = pmat->GetMaterialProperties()->Type();

		ar.BeginChunk(PVM_MATERIAL);
		{
			ar.WriteChunk(PVM_MATERIAL_TYPE, ntype);
			ar.BeginChunk(PVM_MATERIAL_DATA);
			{
				pmat->Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}
	ar.Close();

	return true;
}

bool CModelDocument::ImportMaterials(const std::string& fileName)
{
	IArchive ar;
	if (ar.Open(fileName.c_str(), 0x0050564D) == false) return false;

	FEModel& fem = *GetFEModel();
	IArchive::IOResult nret = IArchive::IO_OK;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVM_VERSION)
		{
			unsigned int version = 0;
			nret = ar.read(version);
			if (nret != IArchive::IO_OK) return false;

			if (version != PVM_VERSION_NUMBER) return false;

			ar.SetVersion(version);
		}
		else if (nid == PVM_MATERIAL)
		{
			GMaterial* gmat = 0;
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();

				if (nid == PVM_MATERIAL_TYPE)
				{
					int ntype = -1;
					ar.read(ntype);
					// allocate the material
					FEMaterial* pm = 0;
					if (ntype == FE_USER_MATERIAL) pm = new FEUserMaterial(FE_USER_MATERIAL);
					else pm = FEMaterialFactory::Create(ntype);

					if (pm == 0) return false;
					gmat = new GMaterial(pm);
				}
				else if (nid == PVM_MATERIAL_DATA)
				{
					if (gmat == 0) return false;
					gmat->Load(ar);
				}

				ar.CloseChunk();
			}

			if (gmat) fem.AddMaterial(gmat);
		}
		ar.CloseChunk();
	}

	return true;
}

FEDataMap* CModelDocument::CreateDataMap(FSObject* po, std::string& mapName, std::string& paramName, Param_Type type)
{
	FEModel& fem = *GetFEModel();
	FEComponent* pc = dynamic_cast<FEComponent*>(po);
	if (pc == 0) return 0;
	FEDataMap* dataMap = pc->CreateMap(paramName, type);
	dataMap->SetName(mapName);

	fem.AddDataMap(dataMap);

	return dataMap;
}

//-----------------------------------------------------------------------------
// SELECTION
//-----------------------------------------------------------------------------
void CModelDocument::UpdateSelection(bool report)
{
	// delete old selection
	if (m_psel) delete m_psel;
	m_psel = 0;

	FEModel* ps = GetFEModel();

	// figure out if there is a mesh selected
	GObject* po = GetActiveObject();
	FEMesh* pm = (po ? po->GetFEMesh() : 0);
	FEMeshBase* pmb = (po ? po->GetEditableMesh() : 0);
	FELineMesh* plm = (po ? po->GetEditableLineMesh() : 0);

	// get the mesh mode
	int meshMode = m_wnd->GetMeshMode();

	if (m_vs.nitem == ITEM_MESH)
	{
		switch (m_vs.nselect)
		{
		case SELECT_OBJECT: m_psel = new GObjectSelection(ps); break;
		case SELECT_PART: m_psel = new GPartSelection(ps); break;
		case SELECT_FACE: m_psel = new GFaceSelection(ps); break;
		case SELECT_EDGE: m_psel = new GEdgeSelection(ps); break;
		case SELECT_NODE: m_psel = new GNodeSelection(ps); break;
		case SELECT_DISCRETE: m_psel = new GDiscreteSelection(ps); break;
		}
	}
	else
	{
		if (meshMode == MESH_MODE_VOLUME)
		{
			switch (m_vs.nitem)
			{
			case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(ps, pm); break;
			case ITEM_FACE: if (pm) m_psel = new FEFaceSelection(ps, pm); break;
			case ITEM_EDGE: if (pm) m_psel = new FEEdgeSelection(ps, pm); break;
			case ITEM_NODE: if (pm) m_psel = new FENodeSelection(ps, pm); break;
			}
		}
		else
		{
			switch (m_vs.nitem)
			{
			case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(ps, pm); break;
			case ITEM_FACE: if (pmb) m_psel = new FEFaceSelection(ps, pmb); break;
			case ITEM_EDGE: if (plm) m_psel = new FEEdgeSelection(ps, plm); break;
			case ITEM_NODE: if (plm) m_psel = new FENodeSelection(ps, plm); break;
			}
		}
	}

	// update the window's toolbar to make sure it reflects the correct
	// selection tool
	if (report) m_wnd->ReportSelection();
}

//-----------------------------------------------------------------------------
FESelection* CModelDocument::GetCurrentSelection() { return m_psel; }

//-----------------------------------------------------------------------------
void CModelDocument::GrowNodeSelection(FEMeshBase* pm)
{
	FENodeFaceList NFT;
	NFT.Build(pm);

	int i;
	int NN = pm->Nodes();
	int NF = pm->Faces();
	for (i = 0; i<NN; ++i) pm->Node(i).m_ntag = 0;
	for (i = 0; i<NF; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i<NN; ++i)
	{
		if (pm->Node(i).IsSelected())
		{
			int nf = NFT.Valence(i);
			for (int j = 0; j<nf; ++j) NFT.Face(i, j)->m_ntag = 1;
		}
	}

	for (i = 0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.m_ntag == 1)
		{
			for (int j = 0; j<f.Nodes(); ++j) pm->Node(f.n[j]).m_ntag = 1;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pn = new int[n];
		n = 0;
		for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) pn[n++] = i;
		DoCommand(new CCmdSelectFENodes(pm, pn, n, false));
		delete[] pn;
	}
}

//-----------------------------------------------------------------------------
void CModelDocument::GrowFaceSelection(FEMeshBase* pm, bool respectPartitions)
{
	int N = pm->Faces(), i;
	for (i = 0; i<N; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i<N; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.IsSelected())
		{
			f.m_ntag = 1;
			int n = f.Nodes();
			if (n == 6) n = 3;
			if (n == 8) n = 4;
			for (int j = 0; j < n; ++j)
			{
				if (f.m_nbr[j] >= 0)
				{
					FEFace& fj = pm->Face(f.m_nbr[j]);
					if ((respectPartitions == false) || (f.m_gid == fj.m_gid))
					{
						fj.m_ntag = 1;
					}
				}
			}
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pf = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) pf[n++] = i;
		DoCommand(new CCmdSelectFaces(pm, pf, n, false));
		delete[] pf;
	}
}

//-----------------------------------------------------------------------------
void CModelDocument::GrowElementSelection(FEMesh* pm, bool respectPartitions)
{
	int N = pm->Elements(), i;
	for (i = 0; i<N; ++i) pm->Element(i).m_ntag = 0;
	for (i = 0; i<N; ++i)
	{
		FEElement& e = pm->Element(i);
		if (e.IsSelected())
		{
			e.m_ntag = 1;
			int ne = 0;
			if (e.IsSolid()) ne = e.Faces();
			if (e.IsShell()) ne = e.Edges();
			
			for (int j = 0; j < ne; ++j) 
				if (e.m_nbr[j] >= 0)
				{
					FEElement& ej = pm->Element(e.m_nbr[j]);
					if ((respectPartitions == false) || (e.m_gid == ej.m_gid))
					{
						ej.m_ntag = 1;
					}
				}
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pe = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) pe[n++] = i;
		DoCommand(new CCmdSelectElements(pm, pe, n, false));
		delete[] pe;
	}
}

//-----------------------------------------------------------------------------
void CModelDocument::GrowEdgeSelection(FEMeshBase* pm)
{
	// TODO: implement this
}


//-----------------------------------------------------------------------------
void CModelDocument::ShrinkNodeSelection(FEMeshBase* pm)
{
	FENodeFaceList NFT;
	NFT.Build(pm);

	int i;
	int NN = pm->Nodes();
	int NF = pm->Faces();
	for (i = 0; i<NN; ++i) pm->Node(i).m_ntag = (pm->Node(i).IsSelected() ? 1 : 0);
	for (i = 0; i<NF; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i<NN; ++i)
	{
		if (pm->Node(i).IsSelected() == false)
		{
			int nf = NFT.Valence(i);
			for (int j = 0; j<nf; ++j) NFT.Face(i, j)->m_ntag = 1;
		}
	}

	for (i = 0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.m_ntag == 1)
		{
			for (int j = 0; j<f.Nodes(); ++j) pm->Node(f.n[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pn = new int[n];
		n = 0;
		for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) pn[n++] = i;
		DoCommand(new CCmdSelectFENodes(pm, pn, n, false));
		delete[] pn;
	}
}

//-----------------------------------------------------------------------------
void CModelDocument::ShrinkFaceSelection(FEMeshBase* pm)
{
	int N = pm->Faces(), i;
	for (i = 0; i<N; ++i) pm->Face(i).m_ntag = 1;
	for (i = 0; i<N; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.IsSelected() == false)
		{
			f.m_ntag = 0;
			int n = f.Nodes();
			if (n == 6) n = 3;
			if (n == 8) n = 4;
			for (int j = 0; j<n; ++j) if (f.m_nbr[j] >= 0) pm->Face(f.m_nbr[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pf = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) pf[n++] = i;
		DoCommand(new CCmdSelectFaces(pm, pf, n, false));
		delete[] pf;
	}
}

//-----------------------------------------------------------------------------
void CModelDocument::ShrinkEdgeSelection(FEMeshBase* pm)
{
	// TODO: implement this
}

//-----------------------------------------------------------------------------
void CModelDocument::ShrinkElementSelection(FEMesh* pm)
{
	int N = pm->Elements(), i;
	for (i = 0; i<N; ++i) pm->Element(i).m_ntag = 1;
	for (i = 0; i<N; ++i)
	{
		FEElement& e = pm->Element(i);
		if (e.IsSelected() == false)
		{
			e.m_ntag = 0;
			if (e.IsSolid()) for (int j = 0; j<e.Faces(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 0;
			if (e.IsShell()) for (int j = 0; j<e.Edges(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pe = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) pe[n++] = i;
		DoCommand(new CCmdSelectElements(pm, pe, n, false));
		delete[] pe;
	}
}

void CModelDocument::HideCurrentSelection()
{
	if (GetItemMode() == ITEM_MESH)
	{
		// Hide object level items
		int nmode = GetSelectionMode();
		if ((nmode == SELECT_OBJECT) || (nmode == SELECT_PART))
		{
			FESelection* ps = GetCurrentSelection();
			if (ps && ps->Size())
			{
				DoCommand(new CCmdHideSelection(this));
			}
		}
	}
	else
	{
		// Hide sub-object (i.e. mesh) level items
		FESelection* ps = GetCurrentSelection();
		if (ps && ps->Size())
		{
			DoCommand(new CCmdHideSelection(this));
		}
	}
}

void CModelDocument::HideUnselected()
{
	int itemMode = GetItemMode();
	if (itemMode == ITEM_MESH)
	{
		int selMode = GetSelectionMode();
		if (selMode == SELECT_OBJECT)
		{
			GModel* mdl = GetGModel();
			vector<GObject*> po;
			for (int i = 0; i<mdl->Objects(); ++i)
				if (mdl->Object(i)->IsSelected() == false) po.push_back(mdl->Object(i));

			DoCommand(new CCmdHideObject(po, true));
		}
		else if (selMode == SELECT_PART)
		{
			GModel* mdl = GetGModel();
			list<GPart*> partList;
			for (int i = 0; i<mdl->Objects(); ++i)
			{
				GObject* po = mdl->Object(i);
				for (int j = 0; j<po->Parts(); ++j)
				{
					GPart* part = po->Part(j);
					if (part->IsSelected() == false) partList.push_back(part);
				}
			}

			DoCommand(new CCmdHideParts(mdl, partList));
		}
	}
	else
	{
		GObject* po = GetActiveObject();
		if (po == 0) return;

		if (itemMode == ITEM_ELEM)
		{
			FEMesh* pm = po->GetFEMesh();
			if (pm == 0) return;

			vector<int> elemList;
			for (int i = 0; i<pm->Elements(); ++i)
			{
				FEElement& el = pm->Element(i);
				if (el.IsSelected() == false) elemList.push_back(i);
			}
			DoCommand(new CCmdHideElements(pm, elemList));
		}
		else if (itemMode == ITEM_FACE)
		{
			FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
			if (pm == 0) return;

			vector<int> faceList;
			for (int i = 0; i<pm->Faces(); ++i)
			{
				FEFace& face = pm->Face(i);
				if (face.IsSelected() == false) faceList.push_back(i);
			}
			DoCommand(new CCmdHideFaces(pm, faceList));
		}
	}
}

//-----------------------------------------------------------------------------
bool CModelDocument::ImportGeometry(FEFileImport* preader, const char *szfile)
{
	ClearCommandStack();

	// try to load the file
	if (preader->Load(szfile) == false) return false;

	// set the modified flag
	m_bModified = true;

	return true;
}

// helper function for applying a modifier
bool CModelDocument::ApplyFEModifier(FEModifier& modifier, GObject* po, FEGroup* sel, bool clearSel)
{
	// get the mesh
	FEMesh* pm = po->GetFEMesh();
	if (pm == 0) return false;

	// apply modifier and create new mesh
	FEMesh* newMesh = 0;
	if (sel)
		newMesh = modifier.Apply(sel);
	else
		newMesh = modifier.Apply(pm);

	// make sure the modifier succeeded
	if (newMesh == 0) return false;

	// clear the selection
	if (clearSel) newMesh->ClearFaceSelection();

	// swap the meshes
	string ss = modifier.GetName();
	return DoCommand(new CCmdChangeFEMesh(po, newMesh), ss.c_str());
}

bool CModelDocument::ApplyFESurfaceModifier(FESurfaceModifier& modifier, GSurfaceMeshObject* po, FEGroup* sel)
{
	// get the surface mesh
	FESurfaceMesh* mesh = po->GetSurfaceMesh();
	if (mesh == 0) return false;

	// create a new mesh
	FESurfaceMesh* newMesh = modifier.Apply(mesh, sel);
	if (newMesh == 0) return false;

	// if the object has an FE mesh, we need to delete it
	CCommand* cmd = nullptr;
	if (po->GetFEMesh())
	{
		CCmdGroup* cmdg = new CCmdGroup("Apply surface modifier");
		cmdg->AddCommand(new CCmdChangeFEMesh(po, nullptr));
		cmdg->AddCommand(new CCmdChangeFESurfaceMesh(po, newMesh));
		cmd = cmdg;
	}
	else cmd = new CCmdChangeFESurfaceMesh(po, newMesh);

	// swap the meshes
	return DoCommand(cmd);
}
