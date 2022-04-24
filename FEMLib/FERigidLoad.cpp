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
	SetParamInt("rb", n);
}

int FEBioRigidLoad::GetMaterialID() const
{
	return GetParam("rb")->GetIntValue();
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
