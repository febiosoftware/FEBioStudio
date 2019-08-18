#pragma once
#include "FESurfaceModifier.h"
#include <vector>
using namespace std;

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
	FESurfaceMesh* Apply(FESurfaceMesh* pm);

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
	vector<GEdge*>	m_curveList;
};
