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
#include <MeshLib/FEMesh.h>
#include <MeshLib/FESurfaceMesh.h>

class FEMeshValuator
{
public:
	// When adding a new data field, do so before the MAX_DEFAULT_FIELDS value.
	enum DataFields {
		ELEMENT_VOLUME,
		JACOBIAN,
		SHELL_THICKNESS,
		SHELL_AREA,
		TET_QUALITY,
		TET_MIN_DIHEDRAL_ANGLE,
		TET_MAX_DIHEDRAL_ANGLE,
		TRIANGLE_QUALITY,
		TRIANGLE_MAX_DIHEDRAL_ANGLE,
		TET10_MID_NODE_OFFSET,
		MIN_EDGE_LENGTH,
		MAX_EDGE_LENGTH,
		PRINC_CURVE_1,
		PRINC_CURVE_2,
		// This last value is equal to the number of fields above.
		// This must remain the last value in this enum!
		MAX_DEFAULT_FIELDS
	};

public:
	// constructor
	FEMeshValuator(FSMesh& mesh);

	// evaluate the particular data field
	void Evaluate(int nfield);

	// evaluate just one element
	double EvaluateElement(int i, int nfield, int* err = 0);
	double EvaluateNode(int i, int nfield, int* err = 0);

	// get the list of all datafield names
	static std::vector< std::string > GetDataFieldNames();

public:
	void SetCurvatureLevels(int levels);
	void SetCurvatureMaxIters(int maxIters);
	void SetCurvatureExtQuad(bool b);

private:
	FSMesh& m_mesh;

	// properties for curvature
	int	m_curvature_levels;
	int	m_curvature_maxiters;
	bool m_curvature_extquad;
};

class FESurfaceMeshValuator
{
	// When adding a new data field, do so before the MAX_DEFAULT_FIELDS value.
	enum DataFields {
		FACE_AREA,
		TRIANGLE_QUALITY,
		TRIANGLE_MAX_DIHEDRAL_ANGLE,
		MIN_EDGE_LENGTH,
		MAX_EDGE_LENGTH,
		// This last value is equal to the number of fields above.
		// This must remain the last value in this enum!
		MAX_DEFAULT_FIELDS
	};

public:
	// constructor
	FESurfaceMeshValuator(FSSurfaceMesh& mesh);

	// get the list of all datafield names
	static std::vector< std::string > GetDataFieldNames();

	// evaluate the particular data field
	bool Evaluate(int nfield, Mesh_Data& data);

	// evaluate just one item
	double EvaluateFace(int i, int nfield, int* err = 0);
	double EvaluateNode(int i, int nfield, int* err = 0);

public:
	void SetCurvatureLevels(int levels);
	void SetCurvatureMaxIters(int maxIters);
	void SetCurvatureExtQuad(bool b);

private:
	FSSurfaceMesh& m_mesh;

	// properties for curvature
	int	m_curvature_levels;
	int	m_curvature_maxiters;
	bool m_curvature_extquad;
};
