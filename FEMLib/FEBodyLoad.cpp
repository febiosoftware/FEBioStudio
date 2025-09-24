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
#include "FEBodyLoad.h"
#include <MeshLib/FSItemListBuilder.h>
#include <FECore/units.h>

FSBodyLoad::FSBodyLoad(int ntype, FSModel* ps, int nstep) : FSLoad(ntype, ps, 0, nstep)
{
	SetMeshItemType(FE_ELEM_FLAG);
}

//-----------------------------------------------------------------------------
FSConstBodyForce::FSConstBodyForce(FSModel* ps, int nstep) : FSBodyLoad(FE_CONST_BODY_FORCE, ps, nstep)
{
	SetTypeString("const");
	AddDoubleParam(0, "x");
	AddDoubleParam(0, "y");
	AddDoubleParam(0, "z");
}

double FSConstBodyForce::GetLoad(int n) { return GetFloatValue(FORCE_X + n); }

void FSConstBodyForce::SetLoad(int n, double v) { SetFloatValue(FORCE_X + n, v); }

//-----------------------------------------------------------------------------
FSNonConstBodyForce::FSNonConstBodyForce(FSModel* ps, int nstep) : FSBodyLoad(FE_NON_CONST_BODY_FORCE, ps, nstep)
{
	SetTypeString("non-const");
	AddMathParam("0", "x")->MakeVariable(true);
	AddMathParam("0", "y")->MakeVariable(true);
	AddMathParam("0", "z")->MakeVariable(true);
}

//-----------------------------------------------------------------------------
FSHeatSource::FSHeatSource(FSModel* ps, int nstep) : FSBodyLoad(FE_HEAT_SOURCE, ps, nstep)
{
	SetTypeString("heat_source");
	AddDoubleParam(0, "Q", "Q");
}

//-----------------------------------------------------------------------------
FSSBMPointSource::FSSBMPointSource(FSModel* ps, int nstep) : FSBodyLoad(FE_SBM_POINT_SOURCE, ps, nstep)
{
	SetTypeString("SBM Point Source");
	AddIntParam(1, "sbm", "sbm");
	AddDoubleParam(0, "value", "value");
	AddDoubleParam(0, "x", "x");
	AddDoubleParam(0, "y", "y");
	AddDoubleParam(0, "z", "z");
	AddBoolParam(true, "weigh_volume", "weigh volume");
}

//-----------------------------------------------------------------------------
FSCentrifugalBodyForce::FSCentrifugalBodyForce(FSModel* ps, int nstep) : FSBodyLoad(FE_CENTRIFUGAL_BODY_FORCE, ps, nstep)
{
    SetTypeString("centrifugal");
    AddScienceParam(0, UNIT_ANGULAR_VELOCITY, "angular_speed", "angular speed");
    AddVecParam(vec3d(0,0,1),"rotation_axis", "rotation axis");
    AddVecParam(vec3d(0,0,0),"rotation_center", "rotation center");
}

//-----------------------------------------------------------------------------
FSMassDamping::FSMassDamping(FSModel* ps, int nstep) : FSBodyLoad(FE_MASSDAMPING_LOAD, ps, nstep)
{
	SetTypeString("mass damping");
	AddDoubleParam(1, "C");
}

//-----------------------------------------------------------------------------
FEBioBodyLoad::FEBioBodyLoad(FSModel* ps, int nstep) : FSBodyLoad(FE_FEBIO_BODY_LOAD, ps, nstep)
{

}

void FEBioBodyLoad::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSBodyLoad::Save(ar);
	}
	ar.EndChunk();
}

void FEBioBodyLoad::Load(IArchive& ar)
{
	TRACE("FEBioBodyLoad::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSBodyLoad::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
