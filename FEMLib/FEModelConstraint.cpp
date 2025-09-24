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
#include "FEModelConstraint.h"
#include <MeshLib/FSItemListBuilder.h>

FSModelConstraint::FSModelConstraint(int ntype, FSModel* fem, int nstep) : FSDomainComponent(ntype, fem, nstep)
{
	m_superClassID = FENLCONSTRAINT_ID;
	SetMeshItemType(0);
}

FSSurfaceConstraint::FSSurfaceConstraint(int ntype, FSModel* fem, int nstep) : FSModelConstraint(ntype, fem, nstep)
{
	SetMeshItemType(FE_FACE_FLAG);
}

FSBodyConstraint::FSBodyConstraint(int ntype, FSModel* fem, int nstep) : FSModelConstraint(ntype, fem, nstep)
{
	SetMeshItemType(FE_ELEM_FLAG);
}

//=============================================================================
// FSVolumeConstraint
//-----------------------------------------------------------------------------

FSVolumeConstraint::FSVolumeConstraint(FSModel* ps, int nstep) : FSSurfaceConstraint(FE_VOLUME_CONSTRAINT, ps, nstep)
{
	SetTypeString("volume");

	AddBoolParam(false, "laugon", "augmented lagrangian");
	AddDoubleParam(0.2, "augtol", "augmentation tolerance");
	AddDoubleParam(1, "penalty", "penalty factor");
}

//=============================================================================
// FSWarpingConstraint
//-----------------------------------------------------------------------------

FSWarpingConstraint::FSWarpingConstraint(FSModel* fem) : FSModelConstraint(FE_WARP_CONSTRAINT, fem)
{
	SetTypeString("warp-image");

	AddChoiceParam(0, "laugon", "Lagrange method")->SetEnumNames("penalty\0augmented_lagrangian\0");
	AddDoubleParam(0.1, "altol", "Augmentation tolerance");
	AddDoubleParam(0, "penalty", "Penalty");
	AddDoubleParam(0, "blur", "blur");
	AddVecParam(vec3d(0, 0, 0), "range_min", "Min range");
	AddVecParam(vec3d(1, 1, 1), "range_max", "Max range");
}

//=============================================================================
// FSNormalFlowSurface
//-----------------------------------------------------------------------------

FSNormalFlowSurface::FSNormalFlowSurface(FSModel* ps, int nstep) : FSSurfaceConstraint(FE_NORMAL_FLUID_FLOW, ps, nstep)
{
	SetTypeString("normal fluid flow");

	AddBoolParam(false, "laugon", "augmented lagrangian");
	AddDoubleParam(0.2, "tol", "augmentation tolerance");
	AddDoubleParam(1  , "penalty", "penalty factor");
	AddDoubleParam(0  , "minaug", "min. augmentations");
	AddDoubleParam(10 , "maxaug", "max. augmentations");
	AddDoubleParam(0  , "rhs", "rhs");
}

//=============================================================================
// FSSymmetryPlane
//-----------------------------------------------------------------------------

FSSymmetryPlane::FSSymmetryPlane(FSModel* ps, int nstep) : FSSurfaceConstraint(FE_SYMMETRY_PLANE, ps, nstep)
{
	SetTypeString("symmetry plane");

	AddBoolParam(true, "laugon", "augmented lagrangian");
	AddDoubleParam(0.2, "tol", "augmentation tolerance");
	AddDoubleParam(1  , "penalty", "penalty factor");
	AddDoubleParam(0  , "minaug", "min. augmentations");
	AddDoubleParam(10 , "maxaug", "max. augmentations");
}

//=============================================================================
// FSFrictionlessFluidWall
//-----------------------------------------------------------------------------

FSFrictionlessFluidWall::FSFrictionlessFluidWall(FSModel* ps, int nstep) : FSSurfaceConstraint(FE_FRICTIONLESS_FLUID_WALL, ps, nstep)
{
    SetTypeString("frictionless fluid wall");

    AddBoolParam(false, "laugon", "augmented lagrangian");
    AddDoubleParam(0.2, "tol", "augmentation tolerance");
    AddDoubleParam(1  , "penalty", "penalty factor");
    AddDoubleParam(0  , "minaug", "min. augmentations");
    AddDoubleParam(10 , "maxaug", "max. augmentations");
}

//=============================================================================
FSPrestrainConstraint::FSPrestrainConstraint(FSModel* ps, int nstep) : FSModelConstraint(FE_PRESTRAIN_CONSTRAINT, ps, nstep)
{
	SetTypeString("prestrain");

	AddBoolParam(true, "update");
	AddDoubleParam(0.0, "tolerance");
	AddIntParam(0, "min_iters");
	AddIntParam(-1, "max_iters");
}

//=============================================================================
FSInSituStretchConstraint::FSInSituStretchConstraint(FSModel* ps, int nstep) : FSModelConstraint(FE_INSITUSTRETCH_CONSTRAINT, ps, nstep)
{
	SetTypeString("in-situ stretch");

	AddBoolParam(true, "update");
	AddDoubleParam(0.0, "tolerance");
	AddIntParam(0, "min_iters");
	AddIntParam(-1, "max_iters");
	AddDoubleParam(0.0, "max_stretch");
	AddBoolParam(true, "isochoric");
}

//=============================================================================
FEBioNLConstraint::FEBioNLConstraint(FSModel* fem, int nstep) : FSModelConstraint(FE_FEBIO_NLCONSTRAINT, fem, nstep)
{

}

void FEBioNLConstraint::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSModelConstraint::Save(ar);
	}
	ar.EndChunk();
}

void FEBioNLConstraint::Load(IArchive& ar)
{
	TRACE("FEBioNLConstraint::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSModelConstraint::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FEBioSurfaceConstraint::FEBioSurfaceConstraint(FSModel* fem, int nstep) : FSSurfaceConstraint(FE_FEBIO_SURFACECONSTRAINT, fem, nstep)
{

}

void FEBioSurfaceConstraint::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSModelConstraint::Save(ar);
	}
	ar.EndChunk();
}

void FEBioSurfaceConstraint::Load(IArchive& ar)
{
	TRACE("FEBioSurfaceConstraint::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSSurfaceConstraint::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FEBioBodyConstraint::FEBioBodyConstraint(FSModel* fem, int nstep) : FSBodyConstraint(FE_FEBIO_BODYCONSTRAINT, fem, nstep)
{

}

void FEBioBodyConstraint::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSModelConstraint::Save(ar);
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

void FEBioBodyConstraint::Load(IArchive& ar)
{
	TRACE("FEBioBodyConstraint::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSBodyConstraint::Load(ar); break;
		case CID_PROPERTY_LIST  : LoadFEBioProperties(this, ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
// FEFixedNormalDisplacement
//-----------------------------------------------------------------------------

FSFixedNormalDisplacement::FSFixedNormalDisplacement(FSModel* ps, int nstep) : FSSurfaceConstraint(FE_FIXED_NORMAL_DISPLACEMENT, ps, nstep)
{
    SetTypeString("fixed normal displacement");
    
    AddBoolParam(true, "laugon", "augmented lagrangian");
    AddDoubleParam(0.2, "tol", "augmentation tolerance");
    AddDoubleParam(1  , "penalty", "penalty factor");
    AddDoubleParam(0  , "minaug", "min. augmentations");
    AddDoubleParam(10 , "maxaug", "max. augmentations");
    AddBoolParam(false , "shell_bottom", "shell bottom");
}

