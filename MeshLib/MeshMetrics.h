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
#include "FEMesh.h"

namespace FEMeshMetrics {

// shortest edge on the mesh
double ShortestEdge(const FSMesh& mesh);

// longest edge of the element
double LongestEdge(const FSMesh& mesh, const FSElement& el);

// shortest edge of the element
double ShortestEdge(const FSMesh& mesh, const FSElement& el);

// jacobian for a solid element
double SolidJacobian(const FSMesh& mesh, const FSElement& el);

// calculate jacobian of a shell
double ShellJacobian(const FSMesh& mesh, const FSElement& el, int flag);

// area of shell
double ShellArea(const FSMesh& mesh, const FSElement& el);

// surface area of facet
double SurfaceArea(const FSMesh& mesh, const FSFace& f);

// volume of element
double ElementVolume(const FSMesh& mesh, const FSElement& e);

// quality if tet element
double TetQuality(const FSMesh& mesh, const FSElement& e);

// min dihedral angle of tet
double TetMinDihedralAngle(const FSMesh& mesh, const FSElement& e);

// max dihedral angle of tet
double TetMaxDihedralAngle(const FSMesh& mesh, const FSElement& e);

// quality if triangle element
double TriQuality(const FSMesh& mesh, const FSElement& e);

// max dihedral angle of a triangle
double TriMaxDihedralAngle(const FSMesh& mesh, const FSElement& e);

// max distance from tet10 mid edges nodes to edge line
double Tet10MidsideNodeOffset(const FSMesh& mesh, const FSElement& e, bool brel = false);

// evaluate gradient at element nodes
vec3d Gradient(const FSMesh& mesh, const FSElement& el, int node, double* v);

// evaluate gradient at element nodes (i.e. Grad{Na(x_b)})
vec3d ShapeGradient(const FSMesh& mesh, const FEElement_& el, int na, int nb);

// get the min edge length of an element
double MinEdgeLength(const FSMesh& mesh, const FSElement& e);

// get the max edge length of an element
double MaxEdgeLength(const FSMesh& mesh, const FSElement& e);

// get the min edge length of an face
double MinEdgeLength(const FSMeshBase& mesh, const FSFace& f);

// get the max edge length of an face
double MaxEdgeLength(const FSMeshBase& mesh, const FSFace& f);

float eval_curvature(const std::vector<vec3f>& x, const vec3f& r0, vec3f sn, int measure, bool useExtendedFit, int maxIter);

// curvature measures (see for values for measure in FEMeshData_T.h, in CurvatureField
double Curvature(FSMesh& mesh, int node, int measure, int levels = 1, int maxIters = 10, bool extQuad = false);

}

extern int FTHEX8[6][4];
extern int FTHEX20[6][8];
extern int FTHEX27[6][9];
extern int FTPENTA[5][4];
extern int FTTET[4][3];
extern int FTTET10[4][6];
extern int FTTET15[4][7];
extern int FTTET20[4][10];
extern int FTPYRA5[5][4];
extern int FTPENTA15[5][8];
extern int FTPYRA13[5][8];
