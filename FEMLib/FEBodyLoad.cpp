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
#include "FEBodyLoad.h"
#include <FSCore/paramunit.h>

//-----------------------------------------------------------------------------
FEConstBodyForce::FEConstBodyForce(FEModel* ps, int nstep) : FEBodyLoad(FE_CONST_BODY_FORCE, ps, nstep)
{
	SetTypeString("const");
	AddDoubleParam(0, "x")->SetLoadCurve();
	AddDoubleParam(0, "y")->SetLoadCurve();
	AddDoubleParam(0, "z")->SetLoadCurve();
}

FELoadCurve* FEConstBodyForce::GetLoadCurve(int n)
{
	return GetParamLC(FORCE_X + n);
}

double FEConstBodyForce::GetLoad(int n) { return GetFloatValue(FORCE_X + n); }

void FEConstBodyForce::SetLoad(int n, double v) { SetFloatValue(FORCE_X + n, v); }

//-----------------------------------------------------------------------------
FENonConstBodyForce::FENonConstBodyForce(FEModel* ps, int nstep) : FEBodyLoad(FE_NON_CONST_BODY_FORCE, ps, nstep)
{
	SetTypeString("non-const");
	AddMathParam("0", "x")->SetLoadCurve();
	AddMathParam("0", "y")->SetLoadCurve();
	AddMathParam("0", "z")->SetLoadCurve();
}

FELoadCurve* FENonConstBodyForce::GetLoadCurve(int n)
{
	return GetParamLC(FORCE_X + n);
}

//-----------------------------------------------------------------------------
FEHeatSource::FEHeatSource(FEModel* ps, int nstep) : FEBodyLoad(FE_HEAT_SOURCE, ps, nstep)
{
	SetTypeString("heat_source");
	AddDoubleParam(0, "Q", "Q")->SetLoadCurve();
}

//-----------------------------------------------------------------------------
FESBMPointSource::FESBMPointSource(FEModel* ps, int nstep) : FEBodyLoad(FE_SBM_POINT_SOURCE, ps, nstep)
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
FECentrifugalBodyForce::FECentrifugalBodyForce(FEModel* ps, int nstep) : FEBodyLoad(FE_CENTRIFUGAL_BODY_FORCE, ps, nstep)
{
    SetTypeString("centrifugal");
    AddScienceParam(0, UNIT_RADIAN, "angular_speed", "angular speed")->SetLoadCurve();
    AddVecParam(vec3d(0,0,1),"rotation_axis", "rotation axis");
    AddVecParam(vec3d(0,0,0),"rotation_center", "rotation center");
}
