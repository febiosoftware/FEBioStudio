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
#include "LoadCurve.h"
#include <algorithm>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LoadCurve::LoadCurve()
{
	// add two points
	Add(0, 0);
	Add(1, 1);

	m_ntag = 0;
	m_nID = -1;
	m_ref = 0.0;

	m_szname[0] = 0;
}

LoadCurve::LoadCurve(double r)
{
	m_ntag = 0;
	m_nID = -1;
	m_ref = r;
	m_szname[0] = 0;
}

LoadCurve::LoadCurve(const LoadCurve& lc) : PointCurve(lc)
{
	m_ntag = lc.m_ntag;
	strcpy(m_szname, lc.m_szname);
	m_ref = lc.m_ref;
}

LoadCurve::~LoadCurve()
{

}

LoadCurve& LoadCurve::operator =(const LoadCurve& lc)
{
	PointCurve& This = *this;
	This = lc;

	m_ntag = lc.m_ntag;
	strcpy(m_szname, lc.m_szname);
	m_ref = lc.m_ref;

	return (*this);
}

void LoadCurve::SetName(const char* sz)
{ 
	strcpy(m_szname, sz); 
}

double LoadCurve::Value(double time)
{
	return value(time);
}

void LoadCurve::Load(IArchive& ar)
{
	TRACE("LoadCurve::Load");

	int N = 0;
	bool bact;

	int ntype = 0;
	int next = 0;
	Clear();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_LC_ACTIVE: 
			if (ar.Version() < 0x00010003)
			{
				// The active flag is obsolote, but we still
				// read it for backward compatibility
				ar.read(bact); 
				m_ntag = (bact?1:0);
			}
			else throw ReadError("error parsing CID_LC_ACTIVE (LoadCurve::Load)");
			break;
		case CID_LC_REF   : ar.read(m_ref); break;
		case CID_LC_FLAGS : ar.read(m_ntag); break;
		case CID_LC_TYPE  : ar.read(ntype); SetInterpolator(ntype);  break;
		case CID_LC_EXTEND: ar.read(next); SetExtendMode(next); break;
		case CID_LC_POINTS: ar.read(N); break;
		case CID_LC_POINT:
			{
				double x(0), y(0);
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					switch (nid)
					{
					case CID_LC_TIME: ar.read(x); break;
					case CID_LC_LOAD: ar.read(y); break;
					}
					ar.CloseChunk();
				}
				Add(x, y);
			}
			break;
		}
		ar.CloseChunk();
	}
	assert(Points() == N);

    Update();
}

void LoadCurve::Save(OArchive& ar)
{
	int N = Points();
	
	ar.WriteChunk(CID_LC_FLAGS , m_ntag);
	ar.WriteChunk(CID_LC_TYPE  , GetInterpolator());
	ar.WriteChunk(CID_LC_EXTEND, GetExtendMode());
	ar.WriteChunk(CID_LC_REF   , m_ref);
	ar.WriteChunk(CID_LC_POINTS, N);

	for (int n=0; n<N; ++n)
	{
		vec2d pt = Point(n);
		ar.BeginChunk(CID_LC_POINT);
		{
			ar.WriteChunk(CID_LC_TIME, pt.x());
			ar.WriteChunk(CID_LC_LOAD, pt.y());
		}
		ar.EndChunk();
	}
}
