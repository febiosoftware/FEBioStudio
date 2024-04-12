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
#include "FEModelComponent.h"
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>
#include "FSModel.h"
#include "FECoreMaterial.h"	// for FEElementRef
#include "FEMaterial.h" // for fiber generator defines
#include <MeshLib/FEElementData.h>
#include <exception>
#include <sstream>

FSModelComponent::FSModelComponent(FSModel* fem) : m_fem(fem)
{
	m_superClassID = -1;
	m_febClass = nullptr;
}

FSModelComponent::~FSModelComponent()
{
	if (m_febClass) FEBio::DeleteClass(m_febClass);
}

FSModel* FSModelComponent::GetFSModel()
{
	return m_fem;
}

int FSModelComponent::GetSuperClassID() const
{ 
	return m_superClassID; 
}

void FSModelComponent::SetSuperClassID(int superClassID)
{
	m_superClassID = superClassID;
}

// helper function for retrieving the load curve assigned to a parameter
FSLoadController* FSModelComponent::GetLoadController(int n)
{
	FSModel* fem = GetFSModel(); assert(fem);
	if (fem == nullptr) return nullptr;

	if ((n < 0) || (n >= Parameters()))
	{
		assert(false);
		return nullptr;
	}

	int lc = GetParam(n).GetLoadCurveID();
	if (lc < 0) return nullptr;
	
	FSLoadController* plc = fem->GetLoadControllerFromID(lc);
	if (plc == nullptr) { assert(false); return nullptr; }

	return plc;
}

void FSModelComponent::AssignLoadCurve(Param& p, LoadCurve& lc)
{
	FSModel* fem = GetFSModel(); assert(fem);
	if (fem == nullptr) return;

	FSLoadController* plc = fem->AddLoadCurve(lc);
	if (plc) p.SetLoadCurveID(plc->GetID());
}

void FSModelComponent::SetFEBioClass(void* pc)
{
	m_febClass = pc;
}

void* FSModelComponent::GetFEBioClass()
{
	return m_febClass;
}

bool FSModelComponent::UpdateData(bool bsave)
{
	if (bsave && m_febClass)
	{
		return FEBio::UpdateFEBioClass(this);
	}
	return false;
}

void FSModelComponent::SaveProperties(OArchive& ar)
{
	for (int i = 0; i < Properties(); ++i)
	{
		FSProperty& prop = GetProperty(i);
		ar.BeginChunk(CID_PROPERTY);
		{
			// store the property name
			ar.WriteChunk(CID_PROPERTY_NAME, prop.GetName());

			// store the property data
			for (int j = 0; j < prop.Size(); ++j)
			{
				FSCoreBase* pc = prop.GetComponent(j);
				if (pc)
				{
					string typeStr = pc->GetTypeString();
					ar.WriteChunk(CID_PROPERTY_TYPE, typeStr);
					ar.BeginChunk(CID_PROPERTY_DATA);
					{
						pc->Save(ar);
					}
					ar.EndChunk();
				}
			}
		}
		ar.EndChunk();
	}
}

void FSModelComponent::LoadProperties(IArchive& ar)
{
	FSModel* fem = GetFSModel();
	FSProperty* pc = nullptr;
	string typeString;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int cid = ar.GetChunkID();
		if (cid == CID_PROPERTY)
		{
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int cid = ar.GetChunkID();
				switch (cid)
				{
				case CID_PROPERTY_NAME:
				{
					std::string propName;
					ar.read(propName);
					pc = FindProperty(propName); assert(pc);
				}
				break;
				case CID_PROPERTY_TYPE: ar.read(typeString); break;
				case CID_PROPERTY_DATA:
				{
					FSModelComponent* pmc = FEBio::CreateFSClass(pc->GetSuperClassID(), -1, fem); assert(pmc);
					pmc->Load(ar);
					pc->AddComponent(pmc);
				}
				break;
				default:
					assert(false);
				}
				ar.CloseChunk();
			}
		}
		ar.CloseChunk();
	}
}

//==============================================================================
void SaveClassMetaData(FSModelComponent* pc, OArchive& ar)
{
	string typeStr(pc->GetTypeString());
	ar.WriteChunk(CID_FEBIO_TYPE_STRING, typeStr);
}

void LoadClassMetaData(FSModelComponent* pc, IArchive& ar)
{
	TRACE("LoadClassMetaData");
	int superClassId = pc->GetSuperClassID(); assert(superClassId > 0);

	string error;

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_TYPE_STRING:
		{
			string typeStr;
			ar.read(typeStr);
			assert(superClassId != -1);
			if (FEBio::BuildModelComponent(pc, typeStr, pc->Flags()) == false)
			{
				const char* szsuperclass = FEBio::GetSuperClassString(superClassId);
				if (szsuperclass == nullptr) szsuperclass = "(unknown)";
				std::stringstream ss;
				ss << "Failed to allocate model component:\n";
				ss << "super class : " << szsuperclass << std::endl;
				ss << "type string : \"" << typeStr << "\"";
//				ss << "\n\nIt is possible that this file requires an FEBio plugin before it can be loaded.";
				error = ss.str();
			}
		}
		break;
		}
		ar.CloseChunk();
	}

	if (!error.empty()) throw std::runtime_error(error);
}


void SaveFEBioProperties(FSModelComponent* pmc, OArchive& ar)
{
	for (int i = 0; i < pmc->Properties(); ++i)
	{
		FSProperty& pi = pmc->GetProperty(i);
		ar.BeginChunk(CID_PROPERTY);
		{
			ar.WriteChunk(CID_PROPERTY_NAME, pi.GetName());

			for (int j = 0; j < pi.Size(); ++j)
			{
				FSCoreBase* pc = pi.GetComponent(j);
				if (pc)
				{
					ar.BeginChunk(CID_PROPERTY_ITEM);
					{
						pc->Save(ar);
					}
					ar.EndChunk();
				}
			}
		}
		ar.EndChunk();
	}
}

void LoadFEBioProperties(FSModelComponent* pmc, IArchive& ar)
{
	TRACE("LoadFEBioProperties");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_PROPERTY)
		{
			int n = 0;
			string propName;
			FSProperty* prop = nullptr;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				switch (ar.GetChunkID())
				{
				case CID_PROPERTY_NAME:
				{
					assert(prop == nullptr);
					ar.read(propName);
					prop = pmc->FindProperty(propName); assert(prop);
					prop->Clear();
				}
				break;
				case CID_PROPERTY_ITEM:
				{
					FSModelComponent* pci = FEBio::CreateFSClass(prop->GetSuperClassID(), prop->GetPropertyType(), pmc->GetFSModel());
					assert(pci);
					pci->Load(ar);

					if (prop->maxSize() == FSProperty::NO_FIXED_SIZE)
						prop->AddComponent(pci);
					else prop->SetComponent(pci, n);
					n++;
				}
				break;
				}
				ar.CloseChunk();
			}
		}

		ar.CloseChunk();
	}
}

//===============================================================================

FSGenericClass::FSGenericClass(FSModel* fem) : FSModelComponent(fem) {}

void FSGenericClass::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSModelComponent::Save(ar);
	}
	ar.EndChunk();

	if (Properties() > 0)
	{
		ar.BeginChunk(CID_PROPERTY_LIST);
		{
			SaveFEBioProperties(this, ar);
		}
		ar.EndChunk();
	}
}

void FSGenericClass::Load(IArchive& ar)
{
	TRACE("FSGenericClass::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSModelComponent::Load(ar); break;
		case CID_PROPERTY_LIST: LoadFEBioProperties(this, ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}

	// mape parameters to FEBio class
	UpdateData(true);
}

//===============================================================================
FSVec3dValuator::FSVec3dValuator(FSModel* fem) : FSGenericClass(fem)
{
	SetSuperClassID(FEVEC3DVALUATOR_ID);
	m_naopt = -1;
	m_n[0] = 1;
	m_n[1] = 2;
}

bool FSVec3dValuator::UpdateData(bool bsave)
{
	const char* sztype = GetTypeString();
	if (sztype && (strcmp(sztype, "user" ) == 0)) m_naopt = FE_FIBER_USER;
	if (sztype && (strcmp(sztype, "map") == 0))
	{
		m_naopt = FE_FIBER_MAP;
		m_map = GetParam("map")->GetStringValue();
	}
	if (sztype && (strcmp(sztype, "local") == 0))
	{
		m_naopt = FE_FIBER_LOCAL;
		Param* p = GetParam("local"); assert(p);
		if (p)
		{
			std::vector<int> n = p->GetArrayIntValue(); assert(n.size() == 2);
			if (n.size() == 2)
			{
				m_n[0] = n[0];
				m_n[1] = n[1];
			}
		}
	}
	return FSGenericClass::UpdateData(bsave);
}

vec3d FSVec3dValuator::GetFiberVector(const FEElementRef& el)
{
	switch (m_naopt)
	{
	case FE_FIBER_USER: return el->m_fiber; break;
	case FE_FIBER_LOCAL:
	{
		vec3d v(0, 0, 0);
		FSCoreMesh* pm = el.m_pmesh;
		if (pm)
		{
			int n0 = m_n[0] - 1;
			int n1 = m_n[1] - 1;
			if (n0 < 0) n0 = 0;
			if (n1 < 0) n1 = 1;
			vec3d r0 = pm->Node(el->m_node[n0]).pos();
			vec3d r1 = pm->Node(el->m_node[n1]).pos();
			v = r1 - r0;
			v.unit();
		}
		return v;
	}
	break;
	case FE_FIBER_MAP:
	{
		FSMesh* pm = dynamic_cast<FSMesh*>(el.m_pmesh);
		if (pm)
		{
			FEMeshData* pmd = pm->FindMeshDataField(m_map);
			if (pmd)
			{
				FEElementData* ped = dynamic_cast<FEElementData*>(pmd);
				FEPartData* ppd = dynamic_cast<FEPartData*>(pmd);
				if (ped)
				{
					double d[3];
					ped->get(el.m_nelem, d);
					return vec3d(d[0], d[1], d[2]);
				}
				if (ppd)
				{
					int m = ppd->GetElementIndex(el.m_nelem);
					if (m >= 0)
					{
						double d[3];
						d[0] = ppd->get(3 * m);
						d[1] = ppd->get(3 * m + 1);
						d[2] = ppd->get(3 * m + 2);
						return vec3d(d[0], d[1], d[2]);
					}
				}
			}
		}
		return vec3d(0, 0, 0);
	}
	break;
	}
	return FEBio::GetMaterialFiber(GetFEBioClass(), el.center());
}


//===============================================================================
FSMat3dValuator::FSMat3dValuator(FSModel* fem) : FSGenericClass(fem)
{
	SetSuperClassID(FEMAT3DVALUATOR_ID);
	m_naopt = -1;
}

bool FSMat3dValuator::UpdateData(bool bsave)
{
	const char* sztype = GetTypeString();
	if (sztype && (strcmp(sztype, "local") == 0))
	{
		m_naopt = FE_AXES_LOCAL;
		Param* p = GetParam("local"); assert(p);
		if (p)
		{
			std::vector<int> n = p->GetArrayIntValue(); assert(n.size() == 3);
			if (n.size() == 3)
			{
				m_n[0] = n[0];
				m_n[1] = n[1];
				m_n[2] = n[2];
			}
		}
	}
	else m_naopt = -1;

	return FSGenericClass::UpdateData(bsave);
}

mat3d FSMat3dValuator::GetMatAxis(const FEElementRef& el) const
{
	FSCoreMesh* mesh = el.m_pmesh;
	switch (m_naopt)
	{
	case FE_AXES_LOCAL:
	{
		vec3d r0[FSElement::MAX_NODES];
		for (int i = 0; i < el->Nodes(); ++i) r0[i] = mesh->Node(el->m_node[i]).pos();

		vec3d a, b, c, d;
		mat3d Q;

		a = r0[m_n[1] - 1] - r0[m_n[0] - 1];
		a.unit();

		if (m_n[2] != m_n[1])
		{
			d = r0[m_n[2] - 1] - r0[m_n[0] - 1];
		}
		else
		{
			d = vec3d(0, 1, 0);
			if (fabs(d * a) > 0.999) d = vec3d(1, 0, 0);
		}

		c = a ^ d;
		b = c ^ a;

		b.unit();
		c.unit();

		Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
		Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
		Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;

		return Q;
	}
	break;
	}

	// we have to cast away the const
	FSMat3dValuator* This = const_cast<FSMat3dValuator*>(this);
	return FEBio::GetMaterialAxis(This->GetFEBioClass(), el.center());
}
