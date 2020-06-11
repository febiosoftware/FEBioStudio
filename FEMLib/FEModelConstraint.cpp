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
#include "FEModelConstraint.h"

FEModelConstraint::FEModelConstraint(int ntype, FEModel* fem, int nstep) : FEModelComponent(ntype, fem, nstep)
{

}

FESurfaceConstraint::FESurfaceConstraint(int ntype, FEModel* fem, int nstep) : FEModelConstraint(ntype, fem, nstep)
{

}

//=============================================================================
// FEVolumeConstraint
//-----------------------------------------------------------------------------

FEVolumeConstraint::FEVolumeConstraint(FEModel* ps, int nstep) : FESurfaceConstraint(FE_VOLUME_CONSTRAINT, ps, nstep)
{
	SetTypeString("Volume Constraint");

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
