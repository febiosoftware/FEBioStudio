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
#include "FELoad.h"
#include <FECore/units.h>

//=============================================================================
// NODAL LOAD
//=============================================================================
FSNodalDOFLoad::FSNodalDOFLoad(FSModel* ps) : FSNodalLoad(FE_NODAL_DOF_LOAD, ps)
{
	SetTypeString("Nodal Load");
	AddIntParam(0, "bc", "bc")->SetEnumNames("x-force\0y-force\0z-force\0");
	AddScienceParam(1.0, UNIT_FORCE, "scale", "scale")->MakeVariable(true);
}

//-----------------------------------------------------------------------------
FSNodalDOFLoad::FSNodalDOFLoad(FSModel* ps, FSItemListBuilder* pi, int bc, double f, int nstep) : FSNodalLoad(FE_NODAL_DOF_LOAD, ps, pi, nstep)
{
	SetTypeString("Nodal Load");
	AddIntParam(bc, "bc", "bc")->SetEnumNames("x-force\0y-force\0z-force\0");
	AddScienceParam(f, UNIT_FORCE, "scale", "scale")->MakeVariable(true);
}

//=============================================================================
FEBioNodalLoad::FEBioNodalLoad(FSModel* ps) : FSNodalLoad(FE_FEBIO_NODAL_LOAD, ps)
{

}

void FEBioNodalLoad::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSNodalLoad::Save(ar);
	}
	ar.EndChunk();
}

void FEBioNodalLoad::Load(IArchive& ar)
{
	TRACE("FEBioBoundaryCondition::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSNodalLoad::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
