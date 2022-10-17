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
#include "FESurfaceModifier.h"
#include <vector>

class FECurveMesh;
class GEdge;
class TriMesh;

//-----------------------------------------------------------------------------
// This modifier inserts a curve into an existing mesh.
class FECurveIntersect : public FESurfaceModifier
{
public:
	// constructor
	FECurveIntersect();

	// apply the modifier
	FSSurfaceMesh* Apply(FSSurfaceMesh* pm);

public:
	// clear curve list
	void ClearCurveList();

	// sets the curve that will be inserted
	void AddCurve(GEdge* pc);

	// set the insert edges flag
	void SetInsertEdges(bool b) { m_insertEdges = b; }
	bool InsertEdges() const { return m_insertEdges; }

	// set the tolerance 
	double Tolerance() const { return m_tol; }
	void SetTolerance(double t) { m_tol = t; }

	// set the method
	void SetMethod(int n) { m_method = n; }

private:
	int				m_method;
	bool			m_insertEdges;
	double			m_tol;
	std::vector<GEdge*>	m_curveList;
};
