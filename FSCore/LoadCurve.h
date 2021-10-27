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

#pragma once
#include "Serializable.h"
#include <vector>

class BSpline;

//-----------------------------------------------------------------------------
// STRUCT: LOADPOINT
// structure describing a single datapoint of a load curve
//
class LOADPOINT
{
public:
	double	time;	// time value at this point
	double	load;	// load value at this point

public:
	LOADPOINT() { time = load = 0; }
	LOADPOINT(double t, double l) { time = t; load = l; }
};

//-----------------------------------------------------------------------------
// CLASS: FELoadCurve
// This class implements the concept of load curve
//

class FELoadCurve : public CSerializable
{
public:
	// curve types
	enum {
		LC_LINEAR,
		LC_STEP,
		LC_SMOOTH,
        LC_CSPLINE,
        LC_CPOINTS,
        LC_APPROX
	};

	// extend modes
	enum { 
		EXT_CONSTANT,
		EXT_EXTRAPOLATE,
		EXT_REPEAT,
		EXT_REPEAT_OFFSET
	};

public:
	FELoadCurve(double r);
	FELoadCurve();
	FELoadCurve(const FELoadCurve& lc);
	virtual ~FELoadCurve();

	FELoadCurve& operator = (const FELoadCurve& lc);

	void Add(double x, double y);
	int Add(const LOADPOINT& pt);
	void Delete(int n);
	void Delete(const std::vector<int>& indexList);
	void Clear() { m_Pt.clear(); }

	double Value(double time);

	int Flags() { return m_ntag; }
	void Flags(int n) { m_ntag = n; }

	void SetName(const char* sz);
	const char* GetName() { return m_szname; }

	void Load(IArchive& ar);
	void Save(OArchive& ar);

	int Size() const { return (int) m_Pt.size(); }

	LOADPOINT& operator [] (int n) { return m_Pt[n]; }
	const LOADPOINT& operator [] (int n) const { return m_Pt[n]; }

	LOADPOINT& Item(int n) { return m_Pt[n]; }

	void SetSize(int n) { m_Pt.resize(n); }

	int GetType() { return m_ntype; }
	void SetType(int ntype) { m_ntype = ntype; }

	void Scale(double s);

	void SetExtend(int nmode) { m_nextend = nmode; }
	int GetExtend() { return m_nextend; }

	double GetRefValue() { return m_ref; }
	void SetRefValue(double r) { m_ref = r; }

public:
	void SetID(int nid) { m_nID = nid; }
	int GetID() const { return m_nID; }
    void Update();  // initialize spline

protected:
	double ExtendValue(double t);

	double Val(int n) { return m_Pt[n].load; }

private:
	int		m_nID;		// load curve ID; is set and used only during export

protected:
	int		m_ntag;			// tags
	char	m_szname[256];	// name of load curve (i.e. expression that created it)
	int		m_ntype;		// interpolation type
	int		m_nextend;		// extend mode

	double	m_ref;		// reference value (for n==0)

	std::vector<LOADPOINT>	m_Pt;	// array of points
    
    BSpline*    m_spline;   // B-spline pointer
};
