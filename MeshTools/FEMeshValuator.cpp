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
#include "FEMeshValuator.h"
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/triangulate.h>
#include <MeshTools/GGroup.h>
#include <MeshTools/FENodeData.h>
#include <MeshTools/FEElementData.h>

//-----------------------------------------------------------------------------
// constructor
FEMeshValuator::FEMeshValuator(FEMesh& mesh) : m_mesh(mesh)
{
	m_curvature_levels = 1;
	m_curvature_maxiters = 10;
	m_curvature_extquad = false;
}

//-----------------------------------------------------------------------------
void FEMeshValuator::SetCurvatureLevels(int levels)
{
	m_curvature_levels = levels;
}

//-----------------------------------------------------------------------------
void FEMeshValuator::SetCurvatureMaxIters(int maxIters)
{
	m_curvature_maxiters = maxIters;
}

//-----------------------------------------------------------------------------
void FEMeshValuator::SetCurvatureExtQuad(bool b)
{
	m_curvature_extquad = b;
}

//-----------------------------------------------------------------------------
// evaluate the particular data field
void FEMeshValuator::Evaluate(int nfield)
{
	// evaluate mesh
	int n = 0;
	int NE = m_mesh.Elements();
	Mesh_Data& data = m_mesh.GetMeshData();
	data.Init(&m_mesh, 0.0, 0);
	if (nfield < MAX_DEFAULT_FIELDS)
	{
		if ((nfield == 11) || (nfield == 12))
		{
			if (m_mesh.IsShell())
			{
				int NN = m_mesh.Nodes();
				vector<double> nodeData(NN, 0.0);
				for (int i = 0; i < NN; ++i)
				{
					try {
						double val = EvaluateNode(i, nfield);
						nodeData[i] = val;
					}
					catch (...)
					{

					}
				}

				for (int i = 0; i < NE; ++i)
				{
					FEElement& el = m_mesh.Element(i);
					if (el.IsVisible())
					{
						data.SetElementDataTag(i, 1);
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j)
						{
							double vj = nodeData[el.m_node[j]];
							data.SetElementValue(i, j, vj);
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < NE; ++i)
			{
				FEElement& el = m_mesh.Element(i);
				if (el.IsVisible())
				{
					try {
						double val = EvaluateElement(i, nfield);
						data.SetElementValue(i, val);
						data.SetElementDataTag(i, 1);
					}
					catch (...)
					{
						data.SetElementDataTag(i, 0);
					}
				}
				else data.SetElementDataTag(i, 0);
			}
		}
	}
	else
	{
		nfield -= MAX_DEFAULT_FIELDS;
		if ((nfield >= 0) && (nfield < m_mesh.MeshDataFields()))
		{
			FEMeshData* meshData = m_mesh.GetMeshDataField(nfield);
			switch (meshData->GetDataClass())
			{
			case FEMeshData::NODE_DATA:
			{
				FENodeData& nodeData = dynamic_cast<FENodeData&>(*meshData);
				FEMesh* mesh = nodeData.GetMesh();
				for (int i=0; i < mesh->Elements(); ++i)
				{ 
					FEElement& el = mesh->Element(i);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j)
					{
						double val = nodeData.get(el.m_node[j]);
						data.SetElementValue(i, j, val);
					}
					data.SetElementDataTag(i, 1);
				}
			}
			break;
			case FEMeshData::SURFACE_DATA:
				// TODO: Not sure what to do here. 
				break;
			case FEMeshData::ELEMENT_DATA:
			{
				FEElementData& elemData = dynamic_cast<FEElementData&>(*meshData);
				const FEPart* pg = elemData.GetPart();
				FEItemListBuilder::ConstIterator it = pg->begin();
				for (int i = 0; i < pg->size(); ++i, ++it)
				{
					int elemId = *it;
					double val = elemData.get(i);
					data.SetElementValue(elemId, val);
					data.SetElementDataTag(elemId, 1);
				}
			}
			break;
			case FEMeshData::PART_DATA:
			{
				FEPartData& partData = dynamic_cast<FEPartData&>(*meshData);

				FEElemList* pg = partData.BuildElemList();
				auto it = pg->First();
				int N = pg->Size();
				for (int i = 0; i < N; ++i, ++it)
				{
					int elemId = it->m_lid;
					data.SetElementDataTag(elemId, 1);

					if (partData.GetDataFormat() == FEMeshData::DATA_ITEM)
					{
						double val = partData.GetValue(i, 0);
						data.SetElementValue(elemId, val);
					}
					else if (partData.GetDataFormat() == FEMeshData::DATA_MULT)
					{
						FEElement_* pe = it->m_pi;
						int nn = pe->Nodes();
						for (int j = 0; j < nn; ++j)
						{
							double val = partData.GetValue(i, j);
							data.SetElementValue(elemId, j, val);
						}
					}
				}
				delete pg;
			}
			break;
			}
		}
	}

	// update stats
	data.UpdateValueRange();
}

//-----------------------------------------------------------------------------
// Evaluate element data
double FEMeshValuator::EvaluateElement(int n, int nfield, int* err)
{
	if (err) *err = 0;
	double val = 0, sum = 0;
	const FEElement& el = m_mesh.Element(n);
	switch (nfield)
	{
	case 0: // element volume
		val = FEMeshMetrics::ElementVolume(m_mesh, el);
		break;
	case 1: // jacobian
		if (el.IsShell()) val = FEMeshMetrics::ShellJacobian(m_mesh, el, 1);
		else val = FEMeshMetrics::SolidJacobian(m_mesh, el);
		break;
	case 2: // shell thickness
		if (el.IsShell())
		{
			int n = el.Nodes();
			val = el.m_h[0];
			for (int i = 1; i<n; ++i) if (el.m_h[i] < val) val = el.m_h[i];
		}
		break;
	case 3: // shell area
		if (el.IsShell()) val = FEMeshMetrics::ShellArea(m_mesh, el);
		break;
	case 4: // element quality
		val = FEMeshMetrics::TetQuality(m_mesh, el);
		break;
	case 5: // min dihedral angle
		val = FEMeshMetrics::TetMinDihedralAngle(m_mesh, el);
		break;
	case 6: // max dihedral angle
		val = FEMeshMetrics::TetMaxDihedralAngle(m_mesh, el);
		break;
	case 7:
		val = FEMeshMetrics::TriQuality(m_mesh, el);
		break;
	case 8:
		val = FEMeshMetrics::Tet10MidsideNodeOffset(m_mesh, el, true);
		break;
	case 9:
		val = FEMeshMetrics::MinEdgeLength(m_mesh, el);
		break;
	case 10:
		val = FEMeshMetrics::MaxEdgeLength(m_mesh, el);
		break;
	case 11:
		if (el.IsShell())
		{
			val = 0.0;
		}
		break;
	case 12:
		if (el.IsShell())
		{
			val = 0.0;
		}
		break;
	default:
		val = 0.0;
	}

	return val;
}

//-----------------------------------------------------------------------------
// Evaluate element data
double FEMeshValuator::EvaluateNode(int n, int nfield, int* err)
{
	if (err) *err = 0;
	double val = 0, sum = 0;
	switch (nfield)
	{
	case 11:
		val = FEMeshMetrics::Curvature(m_mesh, n, 2, m_curvature_levels, m_curvature_maxiters, m_curvature_extquad);
		break;
	case 12:
		val = FEMeshMetrics::Curvature(m_mesh, n, 3, m_curvature_levels, m_curvature_maxiters, m_curvature_extquad);
		break;
	}
	return val;
}
