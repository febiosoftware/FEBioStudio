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
#include "FERigidLoad.h"

//=============================================================================
// FSRigidLoad
//=============================================================================
FSRigidLoad::FSRigidLoad(int ntype, FSModel* ps, int nstep) : FSStepComponent(ps)
{ 
	m_type = ntype;
	m_nstepID = nstep;
	SetSuperClassID(FELOAD_ID);
}

void FSRigidLoad::Save(OArchive& ar)
{
	// save the name if there is one
	const string& name = GetName();
	if (name.empty() == false) ar.WriteChunk(CID_FEOBJ_NAME, name);

	const string& info = GetInfo();
	if (info.empty() == false) ar.WriteChunk(CID_FEOBJ_INFO, info);

	ar.WriteChunk(CID_COMPONENT_ACTIVE, m_bActive);

	ar.BeginChunk(CID_FEOBJ_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FSRigidLoad::Load(IArchive& ar)
{
	TRACE("FSRigidLoad::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_COMPONENT_ACTIVE: ar.read(m_bActive); break;
		case CID_FEOBJ_PARAMS: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FEBioRigidLoad::FEBioRigidLoad(FSModel* ps, int nstep) : FSRigidLoad(FE_FEBIO_RIGID_LOAD, ps, nstep) 
{

}

void FEBioRigidLoad::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSRigidLoad::Save(ar);
	}
	ar.EndChunk();
}

void FEBioRigidLoad::SetMaterialID(int n)
{
	// Not all rigid loads define the "rb" parameter. (e.g. rigid cables)
	if (GetParam("rb"))
	{
		SetParamInt("rb", n);
	}
}

int FEBioRigidLoad::GetMaterialID() const
{
	// Not all rigid loads define the "rb" parameter. (e.g. rigid cables)
	if (GetParam("rb"))
	{
		return GetParam("rb")->GetIntValue();
	}

	return -1;
}

void FEBioRigidLoad::Load(IArchive& ar)
{
	TRACE("FSRigidLoad::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSRigidLoad::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
// FSRigidBC
//=============================================================================
FSRigidBC::FSRigidBC(int ntype, FSModel* ps, int nstep) : FSStepComponent(ps)
{
	m_type = ntype;
	m_nstepID = nstep;
	SetSuperClassID(FEBC_ID);
}

void FSRigidBC::Save(OArchive& ar)
{
	// save the name if there is one
	const string& name = GetName();
	if (name.empty() == false) ar.WriteChunk(CID_FEOBJ_NAME, name);

	const string& info = GetInfo();
	if (info.empty() == false) ar.WriteChunk(CID_FEOBJ_INFO, info);

	ar.WriteChunk(CID_COMPONENT_ACTIVE, m_bActive);

	ar.BeginChunk(CID_FEOBJ_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FSRigidBC::Load(IArchive& ar)
{
	TRACE("FSRigidBC::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_COMPONENT_ACTIVE: ar.read(m_bActive); break;
		case CID_FEOBJ_PARAMS: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FEBioRigidBC::FEBioRigidBC(FSModel* ps, int nstep) : FSRigidBC(FE_FEBIO_RIGID_BC, ps, nstep)
{

}

void FEBioRigidBC::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSRigidBC::Save(ar);
	}
	ar.EndChunk();
}

void FEBioRigidBC::SetMaterialID(int n)
{
	SetParamInt("rb", n);
}

int FEBioRigidBC::GetMaterialID() const
{
	return GetParam("rb")->GetIntValue();
}

void FEBioRigidBC::Load(IArchive& ar)
{
	TRACE("FEBioRigidBC::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSRigidBC::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
// FSRigidIC
//=============================================================================
FSRigidIC::FSRigidIC(int ntype, FSModel* ps, int nstep) : FSStepComponent(ps)
{
	m_type = ntype;
	m_nstepID = nstep;
	SetSuperClassID(FEIC_ID);
}

void FSRigidIC::Save(OArchive& ar)
{
	// save the name if there is one
	const string& name = GetName();
	if (name.empty() == false) ar.WriteChunk(CID_FEOBJ_NAME, name);

	const string& info = GetInfo();
	if (info.empty() == false) ar.WriteChunk(CID_FEOBJ_INFO, info);

	ar.WriteChunk(CID_COMPONENT_ACTIVE, m_bActive);

	ar.BeginChunk(CID_FEOBJ_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FSRigidIC::Load(IArchive& ar)
{
	TRACE("FSRigidIC::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_COMPONENT_ACTIVE: ar.read(m_bActive); break;
		case CID_FEOBJ_PARAMS: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FEBioRigidIC::FEBioRigidIC(FSModel* ps, int nstep) : FSRigidIC(FE_FEBIO_RIGID_IC, ps, nstep)
{

}

void FEBioRigidIC::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSRigidIC::Save(ar);
	}
	ar.EndChunk();
}

void FEBioRigidIC::SetMaterialID(int n)
{
	SetParamInt("rb", n);
}

int FEBioRigidIC::GetMaterialID() const
{
	return GetParam("rb")->GetIntValue();
}

void FEBioRigidIC::Load(IArchive& ar)
{
	TRACE("FEBioRigidBC::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSRigidIC::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
