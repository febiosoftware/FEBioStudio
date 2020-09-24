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
#include "FEBioModel.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/GModel.h>
#include <string.h>

//=============================================================================
FEBioMesh::FEBioMesh()
{

}

//-----------------------------------------------------------------------------
void FEBioMesh::UpdateMeshData()
{
	int nsize = 0, i, j, n, m, l;

	FEMesh& mesh = m_mesh;

	int nodes = mesh.Nodes();
	int faces = mesh.Faces();

	m_nFace.resize(nodes);
	for (i = 0; i<nodes; ++i) m_nFace[i] = 0;

	for (i = 0; i<faces; ++i)
	{
		FEFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j = 0; j<n; ++j) m_nFace[face.n[j]]++;
		nsize += n;
	}

	m_iFace.resize(nsize);
	m_pFace.resize(nodes);

	if (nsize > 0)
	{
		int *pi = &m_iFace[0];
		for (i = 0; i < nodes; ++i)
		{
			m_pFace[i] = pi;
			n = m_nFace[i];
			pi += n;
			m_nFace[i] = 0;
		}
	}

	for (i = 0; i<faces; ++i)
	{
		FEFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j = 0; j<n; ++j)
		{
			m = face.n[j];

			l = m_nFace[m];
			m_pFace[m][l] = i;
			m_nFace[m]++;
		}
	}

	// build the element lookup table
	m_minID = m_maxID = -1;
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		if (el.m_nid != -1)
		{
			if ((m_minID == -1) || (el.m_nid < m_minID)) m_minID = el.m_nid;
			if ((m_maxID == -1) || (el.m_nid > m_maxID)) m_maxID = el.m_nid;
		}
	}

	nsize = m_maxID - m_minID + 1;
	m_elem.resize(nsize, (FEElement*) 0);

	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		int nid = el.m_nid;
		if (nid != -1) m_elem[nid - m_minID] = &el;
	}
}

//-----------------------------------------------------------------------------
FEElement* FEBioMesh::FindElementFromID(int nid)
{
	if (nid == -1) return 0;
	if ((nid < m_minID) || (nid > m_maxID)) return 0;
	return m_elem[nid - m_minID];
}

//-----------------------------------------------------------------------------
int FEBioMesh::FindFace(const int* n, int nn, int noff)
{
	int* pf = m_pFace[n[0] - noff];
	int N = m_nFace[n[0] - noff];

	for (int i = 0; i<N; ++i)
	{
		FEFace& face = m_mesh.Face(pf[i]);
		bool bfound = true;
		for (int j = 0; j<nn; ++j)
		if (face.HasNode(n[j] - noff) == false)
		{
			bfound = false;
			break;
		}

		if (bfound) return pf[i];
	}

	return -1;
}

//-----------------------------------------------------------------------------
int FEBioMesh::FindFace(const vector<int>& n, int noff)
{
	return FindFace(&n[0], (int)n.size(), noff);
}

//=============================================================================
FEBioModel::NodeSet::NodeSet()
{
}

FEBioModel::NodeSet::NodeSet(const NodeSet& nodeSet)
{
	m_name = nodeSet.m_name;
	m_node = nodeSet.m_node;
}

FEBioModel::NodeSet::NodeSet(const std::string& name, const vector<int>& node)
{
	m_name = name;
	m_node = node;
}

void FEBioModel::NodeSet::operator = (const NodeSet& nodeSet)
{
	m_name = nodeSet.m_name;
	m_node = nodeSet.m_node;
}

//=============================================================================
FEBioModel::Surface::Surface()
{
	m_refs = 0;
}

FEBioModel::Surface::Surface(const FEBioModel::Surface& s)
{
	m_name = s.m_name;
	m_face = s.m_face;
	m_refs = s.m_refs;
}

void FEBioModel::Surface::operator = (const FEBioModel::Surface& s)
{
	m_name = s.m_name;
	m_face = s.m_face;
	m_refs = s.m_refs;
}

//=============================================================================
FEBioModel::ElementSet::ElementSet()
{
}

FEBioModel::ElementSet::ElementSet(const string& name, const vector<int>& elem)
{
	m_name = name;
	m_elem = elem;
}

FEBioModel::ElementSet::ElementSet(const FEBioModel::ElementSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
}

void FEBioModel::ElementSet::operator = (const FEBioModel::ElementSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
}

//=============================================================================
FEBioModel::Part::Part()
{
	
}

void FEBioModel::Part::SetName(const char* szname)
{
	m_name = szname;
}

const char* FEBioModel::Part::GetName()
{
	return m_name.c_str();
}

FEBioModel::Part::~Part()
{
	
}

FEBioModel::Domain* FEBioModel::Part::AddDomain(const string& name, int matID)
{
	m_dom.push_back(Domain(this, name, matID));
	return &m_dom[m_dom.size() - 1];
}

FEBioModel::NodeSet* FEBioModel::Part::FindNodeSet(const std::string& name)
{
	for (int i = 0; i<(int)m_nset.size(); ++i)
	{
		NodeSet* ps = &m_nset[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

FEBioModel::Surface* FEBioModel::Part::FindSurface(const std::string& name)
{
	for (int i = 0; i<(int)m_surf.size(); ++i)
	{
		Surface* ps = &m_surf[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

int FEBioModel::Part::FindSurfaceIndex(const std::string& name)
{
	for (int i = 0; i<(int)m_surf.size(); ++i)
	{
		Surface* ps = &m_surf[i];
		if (ps->name() == name) return i;
	}
	return -1;
}

FEBioModel::ElementSet* FEBioModel::Part::FindElementSet(const std::string& name)
{
	for (int i = 0; i<(int)m_eset.size(); ++i)
	{
		ElementSet* ps = &m_eset[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

int FEBioModel::Part::FindElementSetIndex(const std::string& name)
{
	for (int i = 0; i<(int)m_eset.size(); ++i)
	{
		ElementSet* ps = &m_eset[i];
		if (ps->name() == name) return i;
	}
	return -1;
}


FEBioModel::DiscreteSet* FEBioModel::Part::FindDiscreteSet(const std::string& name)
{
	for (int i = 0; i<(int)m_des.size(); ++i)
	{
		DiscreteSet* ps = &m_des[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

FEBioModel::SurfacePair* FEBioModel::Part::FindSurfacePair(const std::string& name)
{
	for (int i = 0; i<(int)m_spr.size(); ++i)
	{
		SurfacePair* ps = &m_spr[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

FEBioModel::Domain* FEBioModel::Part::FindDomain(const std::string& name)
{
	for (int i = 0; i<(int)m_dom.size(); ++i)
	{
		Domain* pd = &m_dom[i];
		if (pd->name() == name) return pd;
	}
	return 0;
}

void FEBioModel::Part::Update()
{
	if (GetFEMesh() == 0) return;

	FEMesh& mesh = *GetFEMesh();

	// build node-index lookup table
	int noff = -1, maxID = 0;
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		int nid = mesh.Node(i).m_ntag;
		assert(nid > 0);
		if ((noff < 0) || (nid < noff)) noff = nid;
		if (nid > maxID) maxID = nid;
	}
	vector<int> NLT(maxID - noff + 1, -1);
	for (int i = 0; i<NN; ++i)
	{
		int nid = mesh.Node(i).m_ntag - noff;
		NLT[nid] = i;
	}

	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j) el.m_node[j] = NLT[ el.m_node[j] - noff ];
	}
}


//=============================================================================
FEBioModel::PartInstance::PartInstance(FEBioModel::Part* part) : m_part(part), m_po(nullptr)
{
	m_scl = vec3d(1,1,1);
}

FEBioModel::PartInstance::~PartInstance()
{
	if (m_po) delete m_po; m_po = nullptr;
}

FEMesh* FEBioModel::PartInstance::GetMesh() { return m_po->GetFEMesh(); }

FENodeSet* FEBioModel::PartInstance::BuildFENodeSet(const char* szname)
{
	NodeSet* nodeSet = m_part->FindNodeSet(szname);
	if (nodeSet == 0) return 0;

	// create the node set
	FENodeSet* pns = new FENodeSet(m_po, nodeSet->nodeList());

	// copy the name
	std::string name = nodeSet->name();
	pns->SetName(name.c_str());

	// all done
	return pns;
}

FESurface* FEBioModel::PartInstance::BuildFESurface(FEBioModel::Surface& surf)
{
	// create face list
	vector<int> faceList;
	int NF = surf.faces();
	for (int i = 0; i<NF; ++i)
	{
		const vector<int>& face = surf.face(i);
		int faceID = m_part->m_mesh.FindFace(face);
		assert(faceID >= 0);
		if (faceID >= 0) faceList.push_back(faceID);
	}

	// create the surface
	FESurface* ps = new FESurface(m_po, faceList);

	// copy the name
	std::string name = surf.name();
	ps->SetName(name.c_str());

	// increment surface reference counter
	surf.m_refs++;

	// all done
	return ps;
}

FENodeSet* FEBioModel::PartInstance::BuildFENodeSet(const FEBioModel::NodeSet& nset)
{
	// create the surface
	FENodeSet* pns = new FENodeSet(m_po, nset.nodeList());

	// copy the name
	std::string name = nset.name();
	pns->SetName(name.c_str());

	// all done
	return pns;
}

FESurface* FEBioModel::PartInstance::BuildFESurface(const char* szname)
{
	Surface* surface = m_part->FindSurface(szname);
	if (surface == 0) return 0;

	// create face list
	vector<int> faceList;
	int NF = surface->faces();
	for (int i = 0; i<NF; ++i)
	{
		const vector<int>& face = surface->face(i);
		int faceID = m_part->m_mesh.FindFace(face);
		assert(faceID >= 0);
		if (faceID >= 0) faceList.push_back(faceID);
	}

	// create the surface
	FESurface* ps = new FESurface(m_po, faceList);

	// copy the name
	std::string name = surface->name();
	ps->SetName(name.c_str());

	// increase ref counter on surface
	surface->m_refs++;

	// all done
	return ps;
}

FEPart* FEBioModel::PartInstance::BuildFEPart(const char* szname)
{
	ElementSet* set = m_part->FindElementSet(szname);
	if (set == 0) return 0;

	// get the element list
	vector<int> elemList = set->elemList();

	// these are element IDs. we need to convert them to indices
	// TODO: implement this!
	for (size_t i = 0; i < elemList.size(); ++i) elemList[i] -= 1;

	// create the part
	FEPart* pg = new FEPart(m_po, elemList);

	// copy the name
	std::string name = set->name();
	pg->SetName(name.c_str());

	// all done
	return pg;
}


//=============================================================================
FEBioModel::Domain::Domain(Part* part) : m_part(part)
{
}

FEBioModel::Domain::Domain(Part* part, const std::string& name, int matID) : m_part(part)
{
	m_name = name;
	m_matID = matID;
}

FEBioModel::Domain::Domain(const Domain& part)
{
	m_part = part.m_part;
	m_name = part.m_name;
	m_matID = part.m_matID;
	m_elem = part.m_elem;
}

void FEBioModel::Domain::operator = (const Domain& part)
{
	m_part = part.m_part;
	m_name = part.m_name;
	m_matID = part.m_matID;
	m_elem = part.m_elem;
}

//=============================================================================
FEBioModel::DiscreteSet::DiscreteSet()
{
	m_part = 0;
}

FEBioModel::DiscreteSet::DiscreteSet(const FEBioModel::DiscreteSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
	m_part = s.m_part;
}

//-----------------------------------------------------------------------------
void FEBioModel::DiscreteSet::operator = (const FEBioModel::DiscreteSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
	m_part = s.m_part;
}

//-----------------------------------------------------------------------------
void FEBioModel::DiscreteSet::SetName(const char* sz)
{
	m_name = sz;
}

//-----------------------------------------------------------------------------
void FEBioModel::DiscreteSet::Add(int n0, int n1)
{
	DISCRETE_ELEMENT el = { n0, n1 };
	m_elem.push_back(el);
}


//=============================================================================
FEBioModel::SurfacePair::SurfacePair() : m_part(0)
{
}

FEBioModel::SurfacePair::SurfacePair(const std::string& name, int masterID, int slaveID) : m_part(0)
{
	m_name = name;
	m_masterID = masterID;
	m_slaveID = slaveID;
}

FEBioModel::SurfacePair::SurfacePair(const SurfacePair& sp)
{
	m_part = sp.m_part;
	m_name = sp.m_name;
	m_masterID = sp.m_masterID;
	m_slaveID  = sp.m_slaveID;
}

void FEBioModel::SurfacePair::operator = (const SurfacePair& sp)
{
	m_part = sp.m_part;
	m_name = sp.m_name;
	m_masterID = sp.m_masterID;
	m_slaveID = sp.m_slaveID;
}

//=============================================================================
FEBioModel::PlotVariable::PlotVariable()
{
	m_type = DOMAIN_MESH;
}

FEBioModel::PlotVariable::PlotVariable(const std::string& name) : m_name(name)
{
	m_type = DOMAIN_MESH;
}

FEBioModel::PlotVariable::PlotVariable(const std::string& name, const std::string& domain, DOMAIN_TYPE domType) : m_name(name), m_domain(domain), m_type(domType)
{
}

FEBioModel::PlotVariable::PlotVariable(const FEBioModel::PlotVariable& s)
{
	m_name = s.m_name;
    m_domain = s.m_domain;
	m_type = s.m_type;
}

DOMAIN_TYPE FEBioModel::PlotVariable::domainType()
{
	return m_type;
}

//-----------------------------------------------------------------------------
void FEBioModel::PlotVariable::operator = (const FEBioModel::PlotVariable& s)
{
	m_name = s.m_name;
	m_domain = s.m_domain;
}

//=============================================================================
FEBioModel::LogVariable::LogVariable()
{
	m_type = -1;
	m_groupID = -1;
}

FEBioModel::LogVariable::LogVariable(const LogVariable& log)
{
	m_type = log.m_type;
	m_data = log.m_data;
	m_groupID = log.m_groupID;
}

void FEBioModel::LogVariable::operator=(const LogVariable& log)
{
	m_type = log.m_type;
	m_data = log.m_data;
	m_groupID = log.m_groupID;
}

FEBioModel::LogVariable::LogVariable(int ntype, const std::string& data)
{
	m_type = ntype;
	m_data = data;
	m_groupID = -1;
}

//=============================================================================
FEBioModel::FEBioModel(FEModel& fem) : m_fem(fem)
{
}

FEBioModel::~FEBioModel()
{
	for (int i = 0; i<Parts(); ++i) delete m_part[i];
	m_part.clear();

	for (int i = 0; i<Instances(); ++i) delete m_Inst[i];
	m_Inst.clear();
}

void FEBioModel::AddParamCurve(FELoadCurve* plc, int lc)
{
	PARAM_CURVE pc = { 0, plc, lc };
	m_PC.push_back(pc);
}

void FEBioModel::AddParamCurve(Param* p, int lc)
{
	PARAM_CURVE pc = {p, 0, lc};
	m_PC.push_back(pc);
}

FEBioModel::Part* FEBioModel::AddPart(const char* szname)
{
	Part* newPart = new Part;
	newPart->SetName(szname);
	m_part.push_back(newPart);
	return newPart;
}

FEBioModel::Part* FEBioModel::FindPart(const char* szname)
{
	for (int i=0; i<Parts(); ++i)
	{
		Part& part =  GetPart(i);
		if (strcmp(part.GetName(), szname) == 0) return &part;
	}
	return 0;
}

// Call this function after all geometry was processed.
void FEBioModel::UpdateGeometry()
{
	// update all the part mesh data
	for (int i=0; i<Parts(); ++i)
	{
		Part& part = GetPart(i);
		FEMesh* mesh = part.GetFEMesh();
		mesh->RebuildMesh();
	}

	// let's also create a new object for each instance
	int NP = Instances();
	for (int i = 0; i<NP; ++i)
	{
		PartInstance& instance = *GetInstance(i);
		Part* part = instance.GetPart();

		GMeshObject* po = instance.GetGObject();
		assert(po == nullptr);

		// clone the mesh
		FEMesh* newMesh = new FEMesh(*part->GetFEMesh());

		// create a new mesh object
		po = new GMeshObject(newMesh);
		instance.SetGObject(po);
		po->SetName(instance.GetName());

		// don't forget to update
		po->Update();

		// apply transform
		po->GetTransform().SetScale(instance.m_scl);
		po->GetTransform().SetPosition(instance.m_pos);
		po->GetTransform().SetRotation(instance.m_rot);

		// update the mesh index data
		part->GetFEBioMesh().UpdateMeshData();

		// assign materials to parts
		assert(part->Domains() == po->Parts());
		for (int i=0; i<po->Parts(); ++i)
		{
			GPart& gpart = *po->Part(i);
			Domain& elSet = part->GetDomain(i);

			GMaterial* mat = GetMaterial(elSet.MatID());
			if (mat) gpart.SetMaterialID(mat->GetID());

			std::string name = elSet.name();
			gpart.SetName(name.c_str());
		}
	}
}

FEBioModel::PartInstance* FEBioModel::FindInstance(const char* szname)
{
	int N = Instances();
	for (int i=0; i<N; ++i)
	{
		PartInstance* instance = GetInstance(i);
		if (strcmp(instance->GetName(), szname) == 0) return instance;
	}
	return 0;
}

FEItemListBuilder* FEBioModel::BuildItemList(const char* szname)
{
	if ((szname == 0) || (strlen(szname) == 0)) return nullptr;

	if (szname[0] == '@')
	{
		const char* ch = strchr(szname, ':');
		if (ch == 0) return nullptr;
		int n = ch - szname;
		if (strncmp(szname, "@surface", n) == 0)
		{
			return BuildFESurface(szname + n + 1);
		}
		else return nullptr;
	}
	else return BuildFENodeSet(szname);
}

FENodeSet* FEBioModel::BuildFENodeSet(const char* szname)
{
	// see if there is a dot
	const char* ch = 0;
	if (szname) ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.node_set"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset  = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset , ch + 1, l2); szset [l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);

		FENodeSet* set = 0;
		if (part) set = part->BuildFENodeSet(szset);

		delete [] szset;
		delete [] szpart;

		return set;
	}
	else
	{
		// if not, we assume that there is only one instance
		assert(Instances() == 1);
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		return part->BuildFENodeSet(szname);
	}
}

FEBioModel::Surface* FEBioModel::FindSurface(const char* szname)
{
	// the surface that will be returned
	FEBioModel::Surface* surf = 0;

	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);
		if (part)
		{
			surf = part->GetPart()->FindSurface(szset);
		}

		delete[] szset;
		delete[] szpart;
	}
	else
	{
		// if not, we assume that there is only one instance
		assert(Instances() == 1);
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		surf = part->GetPart()->FindSurface(szname);
	}

	return surf;
}

bool FEBioModel::BuildDiscreteSet(GDiscreteElementSet& set, const char* szname)
{
	// find the set
	FEBioModel::PartInstance* part = 0;
	FEBioModel::DiscreteSet* dset = 0;

	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.discrete_set_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		part = FindInstance(szpart);
		if (part)
		{
			dset = part->GetPart()->FindDiscreteSet(szset);
		}

		delete[] szset;
		delete[] szpart;
	}
	else
	{
		// if not, we assume that there is only one instance
		assert(Instances() == 1);
		if (Instances() != 1) return false;

		part = GetInstance(0);
		dset = part->GetPart()->FindDiscreteSet(szname);
	}

	if (part == 0) { assert(false); return false; }
	if (dset == 0) { assert(false); return false; }

	GMeshObject* po = part->GetGObject();
	if (po && po->GetFEMesh())
	{
		FEMesh& mesh = *po->GetFEMesh();
		int NN = mesh.Nodes();
		int ns = dset->size();
		for (int i = 0; i<ns; ++i)
		{
			const FEBioModel::DiscreteSet::DISCRETE_ELEMENT& el = dset->Element(i);

			int n0 = el.n0;
			int n1 = el.n1;
			if ((n0 >= 0) && (n0 < NN) && (n1 >= 0) && (n1 < NN))
			{
				n0 = po->MakeGNode(n0);
				n1 = po->MakeGNode(n1);
				set.AddElement(n0, n1);
			}
			else
			{
				assert(false);
			}
		}
	}

	return true;
}

FEBioModel::Domain* FEBioModel::FindDomain(const char* szname)
{
	// the domain that will be returned
	FEBioModel::Domain* dom = 0;

	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.domain_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);
		if (part)
		{
			dom = part->GetPart()->FindDomain(szset);
		}

		delete[] szset;
		delete[] szpart;
	}
	else
	{
		// if not, we assume that there is only one instance
		assert(Instances() == 1);
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		dom = part->GetPart()->FindDomain(szname);
	}

	return dom;
}


FESurface* FEBioModel::BuildFESurface(const char* szname)
{
	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);

		FESurface* surf = 0;
		if (part) surf = part->BuildFESurface(szset);

		delete[] szset;
		delete[] szpart;

		return surf;
	}
	else
	{
		// if not, we assume that there is only one instance
		assert(Instances() == 1);
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		return part->BuildFESurface(szname);
	}
}

FEPart* FEBioModel::BuildFEPart(const char* szname)
{
	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);

		FEPart* pg = 0;
		if (part) pg = part->BuildFEPart(szset);

		delete[] szset;
		delete[] szpart;

		return pg;
	}
	else
	{
		// if not, we assume that there is only one instance
		assert(Instances() == 1);
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		return part->BuildFEPart(szname);
	}
}

FEPart* FEBioModel::BuildFEPart(FEBioModel::Domain* dom)
{
	assert(Instances() == 1);
	PartInstance* part = GetInstance(0);

	FEPart* pg = new FEPart(part->GetGObject(), dom->GetElementIDList());

	pg->SetName(dom->name());

	return pg;
}

FEBioModel::SurfacePair* FEBioModel::FindSurfacePair(const char* szname)
{
	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* instance = FindInstance(szpart);

		SurfacePair* surf = 0;
		if (instance) surf = instance->GetPart()->FindSurfacePair(szset);

		delete[] szset;
		delete[] szpart;

		return surf;
	}
	else
	{
		// if not, we assume that there is only one instance
		assert(Instances() == 1);
		if (Instances() != 1) return 0;

		Part* part = &GetPart(0);
		return part->FindSurfacePair(szname);
	}
}

int FEBioModel::GetMaterialIndex(const char* szmat)
{
	for (int i=0; i<Materials(); ++i)
	{
		GMaterial* mat = GetMaterial(i);
		string name = mat->GetName();
		if (name == szmat) return i;
	}
	return -1;
}
