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
#include <MeshTools/FEModel.h>
#include <exception>
#include <sstream>

FSModelComponent::FSModelComponent(FSModel* fem) : m_fem(fem)
{
	m_superClassID = -1;
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
			if (FEBio::BuildModelComponent(pc, typeStr) == false)
			{
				const char* szsuperclass = FEBio::GetSuperClassString(superClassId);
				if (szsuperclass == nullptr) szsuperclass = "(unknown)";
				std::stringstream ss;
				ss << "Failed to allocate model component:\n";
				ss << "super class : " << szsuperclass << std::endl;
				ss << "type string : \"" << typeStr << "\"";
				ss << "\n\nIt is possible that this file requires an FEBio plugin before it can be loaded.";
				string s = ss.str();
                throw std::exception();
			}
		}
		break;
		}
		ar.CloseChunk();
	}
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

FSGenericClass::FSGenericClass() {}

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
}
