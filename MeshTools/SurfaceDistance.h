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
#include <vector>

class GObject;

//-----------------------------------------------------------------------------
// This class measures the distance between two (shell) meshes and assigns
// the distance to the element thickness.
class CSurfaceDistance
{
public:
	enum {
		NORMAL,
		CLOSEST_POINT,
	};

public:
	// constructor
	CSurfaceDistance();

	// set the max range of the distances
	void SetRange(double gmin, double gmax);

	// calculate signed or unsigned distance
	void SignedDistance(bool b) { m_bsigned = b; }
	bool SignedDistance() const { return m_bsigned; }

	// set the projection method
	void SetProjectionMethod(int n) { m_ntype = n; }

	// set the multiplier
	void SetMultiplier(double s) { m_scale = s; }

	void SetClamp(bool b) { m_bclamp = b; }

	bool Apply(GObject* pso, GObject* pmo);

protected:
	bool	m_bclamp;
	double	m_min, m_max;
	double	m_scale;
	bool	m_bsigned;
	int		m_ntype;

protected:
	bool NormalProject(GObject* pso, GObject* pmo, std::vector<double>& dist);
	bool ClosestPoint (GObject* pso, GObject* pmo, std::vector<double>& dist);

public:
	double	m_mean;
	double	m_stddev;
};
