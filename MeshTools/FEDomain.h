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
#include <FSCore/math3d.h>
#include <MeshLib/FSElement.h>
#include <MeshLib/FSMesh.h>

// This file contains helper classes for the FEBoundaryLayerMesher class
class FSDomain;

//-----------------------------------------------------------------------------
// Helper functions for treating linear and quadratic solid elements the same way.
// The boundary layer mesher works with the corner nodes of the elements. The
// midside nodes of quadratic elements are generated when the domain is meshed.
inline bool IsHexElement    (int ntype) { return (ntype == FE_HEX8  ) || (ntype == FE_HEX20  ); }
inline bool IsPentaElement  (int ntype) { return (ntype == FE_PENTA6) || (ntype == FE_PENTA15); }
inline bool IsTetElement    (int ntype) { return (ntype == FE_TET4  ) || (ntype == FE_TET10  ); }
inline bool IsPyramidElement(int ntype) { return (ntype == FE_PYRA5 ) || (ntype == FE_PYRA13 ); }

inline bool IsQuadraticElement(int ntype)
{
	return (ntype == FE_HEX20) || (ntype == FE_PENTA15) || (ntype == FE_TET10) || (ntype == FE_PYRA13);
}

// number of corner nodes of the supported solid elements (0 for all other element types)
inline int ElementCornerNodes(int ntype)
{
	if (IsHexElement(ntype))     return 8;
	if (IsPentaElement(ntype))   return 6;
	if (IsTetElement(ntype))     return 4;
	if (IsPyramidElement(ntype)) return 5;
	return 0;
}

// number of corner nodes of a face (3 for triangles, 4 for quads)
inline int FaceCornerNodes(const FSFace& face)
{
	switch (face.Type())
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10: return 3;
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9: return 4;
	}
	return face.Nodes();
}

class FEDVertex
{
public:
    //! constructor
    FEDVertex();
    FEDVertex(int m, const vec3d& x);

public:
    vec3d   r;          // vertex position
    int     m_ntag;     // tag number
    std::vector<int> m_edge; // list of edges connected to this vertex
    std::vector<int> m_quad; // list of quads connected to this vertex
    std::vector<int> m_tri;  // list of tris connected to this vertex
};

class FEDEdge
{
public:
    //! constructor
    FEDEdge();
    FEDEdge(int v0, int v1);

public:
    int  v[2];   // vertices of edge
    
public:
    // meshing functions
    void GenerateBias();
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
    // mesh data
    std::vector<int>     n;      // domain vertex list along edge
    int             nseg;   // number of segments along edge
    double          bias;   // mesh bias
    bool            dble;   // flag for bias (true = double, false = single)
    std::vector<double>  rbias;  // parametric coordinates of biased mesh
};

class FEDQuad
{
public:
    //! constructor
    FEDQuad();
    FEDQuad(int v0, int v1, int v2, int v3);
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
public:
    // local node numbering for edges
    static const int m_edge[4][2];
    
public:
    int             m_ntag; // tag number
    int             v[4];   // vertices of quad
    int             e[4];   // edges of quad
    bool            ep[4];  // edge sense (positive = true, negative = false)
    
    // mesh data
    std::vector< std::vector<int> >     n;      // domain vertex list in face
    std::vector< std::vector<double> >  eta1;   // parametric coordinates of vertices
    std::vector< std::vector<double> >  eta2;   // parametric coordinates of vertices
    
};

class FEDTri
{
public:
    //! constructor
    FEDTri();
    FEDTri(int v0, int v1, int v2, int a = -1);
    
    // find a vertex in the triangle
    int FindVertex(int vtx);
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
public:
    // local node numbering for edges
    static const int m_edge[3][2];

    // local edge numbering for apex
	static const int m_apex[3][3];
    
public:
    int             m_ntag; // tag number
    int             m_fne;
    int             v[3];   // vertices of tri
    int             e[3];   // edges of tri
    bool            ep[3];  // edge sense (positive = true, negative = false)
    
    // mesh data
    std::vector< std::vector<int> >     n;      // domain vertex list in face
    std::vector< std::vector<double> >  eta1;   // parametric coordinates of vertices
    std::vector< std::vector<double> >  eta2;   // parametric coordinates of vertices
    
};

class FEDBox
{
public:
    //! constructor
    FEDBox();

    // find a box face by node numbers
    int FindBoxFace(const FSFace& face);
    
    // find a box edge by node numbers
    int FindBoxEdge(int n0, int n1);
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
public:
    void SetDomain(FSDomain* pdom) { m_pDom = pdom; }
    FSDomain* GetDomain() { return m_pDom; }
    
public:
    // local node numbering for edges
    static const int m_edge[12][2];
    
    // local node numbering for quad faces
	static const int m_quad[6][4];
    
public:
    int             m_ntag;     // tag number
    int             m_gid;
    
public:
    int             v[8];       // list of vertices
    int             e[12];      // list of edges
    bool            ep[12];     // edge sense (positive = true, negative = false)
    int             q[6];       // list of faces
    bool            qp[6];      // quad sense (positive = true, negative = false)
    int             qst[6];     // quad starting node
    
    // mesh data
    std::vector< std::vector< std::vector<int> > >     n;      // domain node list along edge
    std::vector< std::vector<int> >               elem;   // elements
    
public:
    // set mesh parameters assuming faces 0,3,4 map identically to faces 2, 1, 5
    bool SetMeshFaces034(int nseg[3], double bias[3], bool dble[3]);
    
    // set mesh parameters along edges normal to a single face
    bool SetMeshSingleFace(int face, int nseg, double bias, bool dble);

protected:
    FSDomain*   m_pDom; // pointer to the domain this vertex belongs to
};

class FEDWedge
{
public:
    //! constructor
    FEDWedge();
    
    // find a wedge face by FSFace node numbers
    int FindWedgeFace(const FSFace& face);
    
    // find a wedge edge by node numbers
    int FindWedgeEdge(int n0, int n1);
    
    // find a wedge vertex by node number
    int FindWedgeVertex(int n0);
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
public:
    void SetDomain(FSDomain* pdom) { m_pDom = pdom; }
    FSDomain* GetDomain() { return m_pDom; }
    
public:
    // local node numbering for edges
    static const int m_edge[9][2];
    
    // local node numbering for faces
	static const int m_face[5][4];
    
public:
    int             m_ntag;     // tag number
    int             m_fne;      // face/node/edge number
    int             m_gid;

public:
    int             v[6];       // list of vertices
    int             e[9];       // list of edges
    bool            ep[9];      // edge sense (positive = true, negative = false)
    int             f[5];       // list of quad (0-2) and tri (3-4) faces
    bool            fp[5];      // face sense (positive = true, negative = false)
    int             fst[5];     // face starting node
    
    // mesh data
    std::vector< std::vector< std::vector<int> > >     n;      // domain node list
    std::vector< std::vector<int> >               elem;   // elements
    
public:
    // set mesh parameters
    bool SetMesh(int iedge, int nseg[3], double bias[3], bool dble[3]);
    
    // set mesh parameters along edges normal to a single face
    bool SetMeshSingleFace(int face, int nseg, double bias, bool dble);
    
    // set mesh parameters based on a single wedge edge
    bool SetMeshSingleEdge(int edge, int nseg, double bias, bool dble);
    
protected:
    FSDomain*   m_pDom; // pointer to the domain this vertex belongs to
};

class FEDTet
{
public:
    //! constructor
    FEDTet();
    
    // find a tet face by FSFace node numbers
    int FindTetFace(const FSFace& face);
    
    // find a tet edge by node numbers
    int FindTetEdge(int n0, int n1);
    
    // find the opposite tet edge by node numbers
    int FindTetOppositeEdge(int n0, int n1);
    
    // find a tet vertex by node number
    int FindTetVertex(int n0);
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
public:
    void SetDomain(FSDomain* pdom) { m_pDom = pdom; }
    FSDomain* GetDomain() { return m_pDom; }
    
public:
    // local node numbering for edges
    static const int m_edge[6][2];
    
    // local node numbering for faces
	static const int m_face[4][3];
    
public:
    int             m_ntag;     // tag number
    int             m_type;     // mesh type (0 = from face, 1 = from node, 2 = from edge)
    int             m_fne;      // face/node/edge number
    int             m_gid;

public:
    int             v[4];       // list of vertices
    int             e[6];       // list of edges
    bool            ep[6];      // edge sense (positive = true, negative = false)
    int             f[4];       // list of tri faces
    bool            fp[4];      // face sense (positive = true, negative = false)
    int             fst[4];     // face starting node
    
    // mesh data
    std::vector<int>             n;      // domain node list
    std::vector< std::vector<int> >   elem;   // elements
    
public:
    // set mesh parameters along edges emanating out of a face
    bool SetMeshFromFace(int iface, int nseg, double bias, bool dble);
    
    // set mesh parameters along edges emanating from a vertex
    bool SetMeshFromVertex(int ivtx, int nseg, double bias, bool dble);
    
    // set mesh parameters along edges emanating out of an edge
    bool SetMeshFromEdge(int iedge, int nseg, double bias, bool dble);
    
protected:
    FSDomain*   m_pDom; // pointer to the domain this vertex belongs to
};

class FEDPyramid
{
public:
    // mesh types
    enum { FROM_BASE = 0, FROM_TRI = 1, FROM_APEX = 2 };

    //! constructor
    FEDPyramid();
    
    // find a pyramid face by FSFace node numbers (0-3 = tri faces, 4 = quad base)
    int FindPyramidFace(const FSFace& face);
    
    // find a pyramid vertex by node number
    int FindPyramidVertex(int n0);
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
public:
    void SetDomain(FSDomain* pdom) { m_pDom = pdom; }
    FSDomain* GetDomain() { return m_pDom; }
    
public:
    // local node numbering for edges
    // edges 0-3 are the base edges, edges 4-7 connect base nodes 0-3 to the apex (node 4)
    static const int m_edge[8][2];
    
    // local node numbering for faces (triangular faces repeat the apex)
    static const int m_face[5][4];
    
public:
    int             m_ntag;     // tag number
    int             m_type;     // mesh type (see enum above)
    int             m_fne;      // face number for FROM_TRI
    int             m_gid;
    
public:
    int             v[5];       // list of vertices (0-3 = base, 4 = apex)
    int             e[8];       // list of edges
    bool            ep[8];      // edge sense (positive = true, negative = false)
    
    // mesh data
    std::vector< std::vector<int> >   elem;   // elements (8 = hex8, 5 = pyra5)
    
public:
    // set mesh parameters along the edges emanating out of the quadrilateral base
    // (the base is on the selected surface). The result is nseg-1 hexes and one pyramid.
    bool SetMeshFromBase(int nseg, double bias, bool dble);
    
    // set mesh parameters along the two base edges emanating out of
    // triangular face iface (0-3). The result is nseg pyramids, all sharing the apex.
    bool SetMeshFromTriFace(int iface, int nseg, double bias, bool dble);
    
    // set mesh parameters along the four edges emanating from the apex
    // (the apex is on the selected surface). The result is one pyramid and nseg-1 hexes.
    bool SetMeshFromApex(int nseg, double bias, bool dble);
    
protected:
    // set the meshing parameters of local edge ie, where the layers are stacked
    // in the direction of the edge definition (dp = true) or opposite to it (dp = false).
    // bias is the growth ratio along the stacking direction.
    bool SetEdge(int ie, bool dp, int nseg, double bias, bool dble);
    
    // get the stacked vertex list along local edge ie (in the stacking direction)
    bool GetStack(FSDomain* pdom, int ie, bool dp, std::vector<int>& stack);
    
protected:
    FSDomain*   m_pDom; // pointer to the domain this pyramid belongs to
};

class FSDomain
{
public:
	//! constructor
	FSDomain(FSMesh* pm);

public:
	int Vertices() const { return (int)VertexList.size(); }
	int Edges() const { return (int)EdgeList.size(); }
	int Quads() const { return (int)QuadList.size(); }
	int Tris() const { return (int)TriList.size(); }
	int Boxes() const { return (int)BoxList.size(); }
	int Wedges() const { return (int)WedgeList.size(); }
	int Tets() const { return (int)TetList.size(); }
	int Pyramids() const { return (int)PyramidList.size(); }

	FEDVertex& Vertex(int n) { return VertexList[n]; }
	FEDEdge& Edge(int n) { return EdgeList[n]; }
	FEDQuad& Quad(int n) { return QuadList[n]; }
	FEDTri& Tri(int n) { return TriList[n]; }
	FEDBox& Box(int n) { return BoxList[n]; }
	FEDWedge& Wedge(int n) { return WedgeList[n]; }
	FEDTet& Tet(int n) { return TetList[n]; }
	FEDPyramid& Pyramid(int n) { return PyramidList[n]; }

public:
	std::vector<FEDVertex>   VertexList;
	std::vector<FEDEdge>     EdgeList;
	std::vector<FEDQuad>     QuadList;
	std::vector<FEDTri>      TriList;
	std::vector<FEDBox>      BoxList;
	std::vector<FEDWedge>    WedgeList;
	std::vector<FEDTet>      TetList;
	std::vector<FEDPyramid>  PyramidList;
	std::vector<int> m_NLT;

public:
	// find vertex by tag number
	int FindVertexByTagNumber(int n);

	// find edge
	int FindEdge(const FEDEdge& edge);
	int FindEdge(const FEDEdge& edge, bool& pos);

	// find quad
	int FindQuad(const FEDQuad& quad);
	int FindQuad(const FEDQuad& quad, bool& pos, int& ist);
	int FindQuadFromFace(const FSFace& face);

	// find tri
	int FindTri(const FEDTri& tri);
	int FindTri(const FEDTri& tri, bool& pos, int& ist);
	int FindTriFromFace(const FSFace& face);

	// find box by tag number
	int FindBox(int n);

	// find wedge by tag number
	int FindWedge(int n);

	// find tet by tag number
	int FindTet(int n);

	// find pyramid by tag number
	int FindPyramid(int n);

	// add a vertex to this domain
	int AddVertex(int n, const vec3d& r);

	// add an edge to this domain
	int AddEdge(FEDEdge edge);

	// add a quad to this domain
	int AddQuad(FEDQuad quad);

	// add a tri to this domain
	int AddTri(FEDTri tri);

	// add a box from a list of vertices
	int AddBox(std::vector<int> vlist, int ntag = -1, int gid = -1);

	// add a wedge from a list of vertices
	int AddWedge(std::vector<int> vlist, int ntag = -1, int gid = -1);

	// add a tet from a list of vertices
	int AddTet(std::vector<int> vlist, int ntag = -1, int gid = -1);

	// add a pyramid from a list of vertices
	int AddPyramid(std::vector<int> vlist, int ntag = -1, int gid = -1);

	// split a box into two wedges
	void SplitBoxIntoWedges(int ibox, int iedge, int iopt, int iwdg[2]);

	// split a wedge into three tets
	void SplitWedgeIntoTets(int iwdg, int ivtx, int itet[3]);

	// add an element to this domain
	bool AddElement(int iel);

public:
	// reset mesh parameters
	void ResetMeshParameters();

	// mesh the edges
	bool MeshEdges();

	// mesh the quads
	bool MeshQuads();

	// mesh the tris
	bool MeshTris();

	// mesh the boxes
	bool MeshBoxes();

	// mesh the wedges
	bool MeshWedges();

	// mesh the tets
	bool MeshTets();

	// mesh the pyramids
	bool MeshPyramids();

	// mesh the domain
	bool MeshDomain();

public:
	FSMesh* m_pmesh;
};
