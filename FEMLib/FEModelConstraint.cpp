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
#include <MeshTools/FEItemListBuilder.h>

FEModelConstraint::FEModelConstraint(int ntype, FEModel* fem, int nstep) : FEDomainComponent(ntype, fem, nstep)
{
	m_superClassID = FE_CONSTRAINT;
	SetMeshItemType(0);
}

FESurfaceConstraint::FESurfaceConstraint(int ntype, FEModel* fem, int nstep) : FEModelConstraint(ntype, fem, nstep)
{
	SetMeshItemType(FE_FACE_FLAG);
}

//=============================================================================
// FEVolumeConstraint
//-----------------------------------------------------------------------------

FEVolumeConstraint::FEVolumeConstraint(FEModel* ps, int nstep) : FESurfaceConstraint(FE_VOLUME_CONSTRAINT, ps, nstep)
{
	SetTypeString("volume");

	AddBoolParam(false, "laugon", "augmented lagrangian");
	AddDoubleParam(0.2, "augtol", "augmentation tolerance");
	AddDoubleParam(1, "penalty", "penalty factor");
}

//=============================================================================
// FEWarpingConstraint
//-----------------------------------------------------------------------------

FEWarpingConstraint::FEWarpingConstraint(FEModel* fem) : FEModelConstraint(FE_WARP_CONSTRAINT, fem)
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
// FENormalFlowSurface
//-----------------------------------------------------------------------------

FENormalFlowSurface::FENormalFlowSurface(FEModel* ps, int nstep) : FESurfaceConstraint(FE_NORMAL_FLUID_FLOW, ps, nstep)
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
// FESymmetryPlane
//-----------------------------------------------------------------------------

FESymmetryPlane::FESymmetryPlane(FEModel* ps, int nstep) : FESurfaceConstraint(FE_SYMMETRY_PLANE, ps, nstep)
{
	SetTypeString("symmetry plane");

	AddBoolParam(true, "laugon", "augmented lagrangian");
	AddDoubleParam(0.2, "tol", "augmentation tolerance");
	AddDoubleParam(1  , "penalty", "penalty factor");
	AddDoubleParam(0  , "minaug", "min. augmentations");
	AddDoubleParam(10 , "maxaug", "max. augmentations");
}

//=============================================================================
// FEFrictionlessFluidWall
//-----------------------------------------------------------------------------

FEFrictionlessFluidWall::FEFrictionlessFluidWall(FEModel* ps, int nstep) : FESurfaceConstraint(FE_FRICTIONLESS_FLUID_WALL, ps, nstep)
{
    SetTypeString("frictionless fluid wall");

    AddBoolParam(false, "laugon", "augmented lagrangian");
    AddDoubleParam(0.2, "tol", "augmentation tolerance");
    AddDoubleParam(1  , "penalty", "penalty factor");
    AddDoubleParam(0  , "minaug", "min. augmentations");
    AddDoubleParam(10 , "maxaug", "max. augmentations");
}

//=============================================================================
FEPrestrainConstraint::FEPrestrainConstraint(FEModel* ps, int nstep) : FEModelConstraint(FE_PRESTRAIN_CONSTRAINT, ps, nstep)
{
	SetTypeString("prestrain");

	AddBoolParam(true, "update");
	AddDoubleParam(0.0, "tolerance");
	AddIntParam(0, "min_iters");
	AddIntParam(-1, "max_iters");
}

//=============================================================================
FEInSituStretchConstraint::FEInSituStretchConstraint(FEModel* ps, int nstep) : FEModelConstraint(FE_INSITUSTRETCH_CONSTRAINT, ps, nstep)
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
FEBioNLConstraint::FEBioNLConstraint(FEModel* fem, int nstep) : FEModelConstraint(FE_FEBIO_NLCONSTRAINT, fem, nstep)
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
		FEModelConstraint::Save(ar);
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
		case CID_FEBIO_BASE_DATA: FEModelConstraint::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
