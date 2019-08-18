#include "stdafx.h"
#include "FECurveIntersect.h"
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FECurveMesh.h>
#include <MeshLib/TriMesh.h>
#include <MeshLib/FESurfaceMesh.h>
#include <MeshLib/insertCurve.h>
#include <MeshLib/insertCurve2.h>

FECurveIntersect::FECurveIntersect() : FESurfaceModifier("Intersect Curve")
{
	m_method = 1;
	m_insertEdges = true;
	m_tol = 0.05;
}

void FECurveIntersect::AddCurve(GEdge* pc)
{
	m_curveList.push_back(pc);
}

// clear curve list
void FECurveIntersect::ClearCurveList()
{ 
	m_curveList.clear(); 
}

FESurfaceMesh* FECurveIntersect::Apply(FESurfaceMesh* pm)
{
	switch (m_method)
	{
	case 0:
		{
			InsertCurves ic;
			return ic.Apply(pm, m_curveList, m_insertEdges, m_tol);
		}
		break;
	case 1:
		{
			InsertCurves2 ic;
			return ic.Apply(pm, m_curveList, m_insertEdges);
		}
		break;
	break;
	}

	assert(false);
	return 0;
}

