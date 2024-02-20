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
#include <GeomLib/GGroup.h>
#include <MeshLib/FENodeData.h>
#include <MeshLib/FEElementData.h>
#include <MeshLib/MeshTools.h>

//-----------------------------------------------------------------------------
// constructor
FEMeshValuator::FEMeshValuator(FSMesh& mesh) : m_mesh(mesh)
{
	m_curvature_levels = 1;
	m_curvature_maxiters = 10;
	m_curvature_extquad = false;
}

//-----------------------------------------------------------------------------
//! Returns the string names of the data fields.
//! When adding a new field, make sure to update the DataFields enum.
std::vector< std::string > FEMeshValuator::GetDataFieldNames()
{
	std::vector< std::string > names;
	names.push_back("Element Volume");
	names.push_back("Jacobian");
	names.push_back("Shell thickness");
	names.push_back("Shell area");
	names.push_back("Tet quality");
	names.push_back("Tet minimal dihedral angle");
	names.push_back("Tet maximal dihedral angle");
	names.push_back("Triangle quality");
	names.push_back("Triangle max dihedral angle");
	names.push_back("Tet10 midside node offset");
	names.push_back("Minimum element edge length");
	names.push_back("Maximum element edge length");
	names.push_back("1-Principal curvature");
	names.push_back("2-Principal curvature");
	return names;
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
		if ((nfield == PRINC_CURVE_1) || (nfield == PRINC_CURVE_2))
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
					FSElement& el = m_mesh.Element(i);
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
				FSElement& el = m_mesh.Element(i);
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
			case NODE_DATA:
			{
				FENodeData& nodeData = dynamic_cast<FENodeData&>(*meshData);
				if (nodeData.GetDataType() == DATA_SCALAR)
				{
					FSMesh* mesh = nodeData.GetMesh();
					for (int i = 0; i < mesh->Elements(); ++i)
					{
						FSElement& el = mesh->Element(i);
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j)
						{
							double val = nodeData.getScalar(el.m_node[j]);
							data.SetElementValue(i, j, val);
						}
						data.SetElementDataTag(i, 1);
					}
				}
			}
			break;
			case FACE_DATA:
				// TODO: Not sure what to do here. 
				break;
			case ELEM_DATA:
			{
				FEElementData& elemData = dynamic_cast<FEElementData&>(*meshData);
				const FSElemSet* pg = elemData.GetElementSet();
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
			case PART_DATA:
			{
				FEPartData& partData = dynamic_cast<FEPartData&>(*meshData);
				if (partData.GetDataType() == DATA_SCALAR)
				{
					FEElemList* pg = partData.BuildElemList();
					if (pg)
					{
						auto it = pg->First();
						int N = pg->Size();
						for (int i = 0; i < N; ++i, ++it)
						{
							int elemId = it->m_lid;
							data.SetElementDataTag(elemId, 1);

							if (partData.GetDataFormat() == DATA_ITEM)
							{
								double val = partData.GetValue(i, 0);
								data.SetElementValue(elemId, val);
							}
							else if (partData.GetDataFormat() == DATA_MULT)
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
				}
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
	const FSElement& el = m_mesh.Element(n);
	switch (nfield)
	{
	case ELEMENT_VOLUME:
		val = FEMeshMetrics::ElementVolume(m_mesh, el);
		break;
	case JACOBIAN:
		if (el.IsShell()) val = FEMeshMetrics::ShellJacobian(m_mesh, el, 1);
		else val = FEMeshMetrics::SolidJacobian(m_mesh, el);
		break;
	case SHELL_THICKNESS:
		if (el.IsShell())
		{
			int n = el.Nodes();
			val = el.m_h[0];
			for (int i = 1; i<n; ++i) if (el.m_h[i] < val) val = el.m_h[i];
		}
		break;
	case SHELL_AREA:
		if (el.IsShell()) val = FEMeshMetrics::ShellArea(m_mesh, el);
		break;
	case TET_QUALITY:
		val = FEMeshMetrics::TetQuality(m_mesh, el);
		break;
	case TET_MIN_DIHEDRAL_ANGLE:
		val = FEMeshMetrics::TetMinDihedralAngle(m_mesh, el);
		break;
	case TET_MAX_DIHEDRAL_ANGLE:
		val = FEMeshMetrics::TetMaxDihedralAngle(m_mesh, el);
		break;
	case TRIANGLE_QUALITY:
		val = FEMeshMetrics::TriQuality(m_mesh, el);
		break;
	case TRIANGLE_MAX_DIHEDRAL_ANGLE:
		val = FEMeshMetrics::TriMaxDihedralAngle(m_mesh, el);
		break;
	case TET10_MID_NODE_OFFSET:
		val = FEMeshMetrics::Tet10MidsideNodeOffset(m_mesh, el, true);
		break;
	case MIN_EDGE_LENGTH:
		val = FEMeshMetrics::MinEdgeLength(m_mesh, el);
		break;
	case MAX_EDGE_LENGTH:
		val = FEMeshMetrics::MaxEdgeLength(m_mesh, el);
		break;
	case PRINC_CURVE_1:
		if (el.IsShell())
		{
			val = 0.0;
		}
		break;
	case PRINC_CURVE_2:
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
	case PRINC_CURVE_1:
		val = FEMeshMetrics::Curvature(m_mesh, n, 2, m_curvature_levels, m_curvature_maxiters, m_curvature_extquad);
		break;
	case PRINC_CURVE_2:
		val = FEMeshMetrics::Curvature(m_mesh, n, 3, m_curvature_levels, m_curvature_maxiters, m_curvature_extquad);
		break;
	}
	return val;
}

//=======================================================================================================

//-----------------------------------------------------------------------------
// constructor
FESurfaceMeshValuator::FESurfaceMeshValuator(FSSurfaceMesh& mesh) : m_mesh(mesh)
{
	m_curvature_levels = 1;
	m_curvature_maxiters = 10;
	m_curvature_extquad = false;
}

//-----------------------------------------------------------------------------
//! Returns the string names of the data fields.
//! When adding a new field, make sure to update the DataFields enum.
std::vector< std::string > FESurfaceMeshValuator::GetDataFieldNames()
{
	std::vector< std::string > names;
	names.push_back("Face area");
	names.push_back("Triangle quality");
	names.push_back("Triangle max dihedral angle");
	names.push_back("Minimum element edge length");
	names.push_back("Maximum element edge length");
	return names;
}

//-----------------------------------------------------------------------------
void FESurfaceMeshValuator::SetCurvatureLevels(int levels)
{
	m_curvature_levels = levels;
}

//-----------------------------------------------------------------------------
void FESurfaceMeshValuator::SetCurvatureMaxIters(int maxIters)
{
	m_curvature_maxiters = maxIters;
}

//-----------------------------------------------------------------------------
void FESurfaceMeshValuator::SetCurvatureExtQuad(bool b)
{
	m_curvature_extquad = b;
}

//-----------------------------------------------------------------------------
// evaluate the particular data field
bool FESurfaceMeshValuator::Evaluate(int nfield, Mesh_Data& data)
{
	// evaluate mesh
	int n = 0;
	int NF = m_mesh.Faces();
	data.Init(&m_mesh, 0.0, 0);
	if (nfield < MAX_DEFAULT_FIELDS)
	{
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = m_mesh.Face(i);
			try {
				double val = EvaluateFace(i, nfield);
				data.SetElementValue(i, val);
				data.SetElementDataTag(i, 1);
			}
			catch (...)
			{
				data.SetElementDataTag(i, 0);
			}
		}
	}
	else
	{
		// Nothing to do here
	}

	// update stats
	data.UpdateValueRange();

	return true;
}

//-----------------------------------------------------------------------------
// Evaluate element data
double FESurfaceMeshValuator::EvaluateFace(int n, int nfield, int* err)
{
	if (err) *err = 0;
	double val = 0, sum = 0;
	FSFace& face = m_mesh.Face(n);

	// get the face nodal coordinates
	vec3d r[FSFace::MAX_NODES];
	for (int i = 0; i < face.Nodes(); ++i) r[i] = m_mesh.Node(face.n[i]).r;

	switch (nfield)
	{
	case FACE_AREA: // face area
		val = m_mesh.FaceArea(face);
		break;
	case TRIANGLE_QUALITY:
		if (face.Type() == FE_FACE_TRI3) val = TriangleQuality(r);
		break;
	case TRIANGLE_MAX_DIHEDRAL_ANGLE:
		if (face.Type() == FE_FACE_TRI3) val = TriMaxDihedralAngle(m_mesh, face);
		break;
	case MIN_EDGE_LENGTH:
		val = FEMeshMetrics::MinEdgeLength(m_mesh, face);
		break;
	case MAX_EDGE_LENGTH:
		val = FEMeshMetrics::MaxEdgeLength(m_mesh, face);
		break;
	default:
		val = 0.0;
	}

	return val;
}

//-----------------------------------------------------------------------------
// Evaluate element data
double FESurfaceMeshValuator::EvaluateNode(int n, int nfield, int* err)
{
	return 0.0;
}
