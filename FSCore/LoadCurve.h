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
#include <FECore/PointCurve.h>

//-----------------------------------------------------------------------------
// This class implements the concept of load curve
//
class LoadCurve : public PointCurve
{
public:
	LoadCurve(double r);
	LoadCurve();
	LoadCurve(const LoadCurve& lc);
	virtual ~LoadCurve();

	LoadCurve& operator = (const LoadCurve& lc);

	double Value(double time);

	int Flags() { return m_ntag; }
	void Flags(int n) { m_ntag = n; }

	void SetName(const char* sz);
	const char* GetName() { return m_szname; }

	void Load(IArchive& ar);
	void Save(OArchive& ar);

	double GetRefValue() { return m_ref; }
	void SetRefValue(double r) { m_ref = r; }

public:
	void SetID(int nid) { m_nID = nid; }
	int GetID() const { return m_nID; }

private:
	int		m_nID;		// load curve ID; is set and used only during export

protected:
	int		m_ntag;			// tags
	char	m_szname[256];	// name of load curve (i.e. expression that created it)
	double	m_ref;		// reference value (for n==0)
};
