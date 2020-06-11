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

//-----------------------------------------------------------------------------
FEBodyForce::FEBodyForce(FEModel* ps, int nstep) : FEBodyLoad(FE_BODY_FORCE, ps, nstep)
{
	SetTypeString("Body Force");
	AddDoubleParam(0, "x", "x")->SetLoadCurve();
	AddDoubleParam(0, "y", "y")->SetLoadCurve();
	AddDoubleParam(0, "z", "z")->SetLoadCurve();
}

FELoadCurve* FEBodyForce::GetLoadCurve(int n)
{
	return GetParamLC(LOAD1 + n);
}

//-----------------------------------------------------------------------------
FEHeatSource::FEHeatSource(FEModel* ps, int nstep) : FEBodyLoad(FE_HEAT_SOURCE, ps, nstep)
{
	SetTypeString("Heat Source");
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
