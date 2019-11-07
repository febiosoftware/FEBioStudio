#pragma once
#include"FEMesh.h"

namespace FEMeshMetrics {

// shortest edge on the mesh
double ShortestEdge(const FEMesh& mesh);

// longest edge of the element
double LongestEdge(const FEMesh& mesh, const FEElement& el);

// shortest edge of the element
double ShortestEdge(const FEMesh& mesh, const FEElement& el);

// jacobian for a solid element
double SolidJacobian(const FEMesh& mesh, const FEElement& el);

// calculate jacobian of a shell
double ShellJacobian(const FEMesh& mesh, const FEElement& el, int flag);

// area of shell
double ShellArea(const FEMesh& mesh, const FEElement& el);

// surface area of facet
double SurfaceArea(const FEMesh& mesh, const FEFace& f);

// volume of element
double ElementVolume(const FEMesh& mesh, const FEElement& e);

// quality if tet element
double TetQuality(const FEMesh& mesh, const FEElement& e);

// min dihedral angle of tet
double TetMinDihedralAngle(const FEMesh& mesh, const FEElement& e);

// max dihedral angle of tet
double TetMaxDihedralAngle(const FEMesh& mesh, const FEElement& e);

// quality if triangle element
double TriQuality(const FEMesh& mesh, const FEElement& e);

// max distance from tet10 mid edges nodes to edge line
double Tet10MidsideNodeOffset(const FEMesh& mesh, const FEElement& e, bool brel = false);

// evaluate gradient at element nodes
vec3d Gradient(const FEMesh& mesh, const FEElement& el, int node, double* v);

// evaluate gradient at element nodes (i.e. Grad{Na(x_b)})
vec3d ShapeGradient(const FEMesh& mesh, const FEElement_& el, int na, int nb);
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
