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
#include <MeshLib/FEElement.h>
#include <MeshLib/FEMesh.h>

// This file contains helper classes for the FEBoundaryLayerMesher class
class FSDomain;

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
    FEDEdge(int v0, int v1, int v2 = -1);

public:
    int  v[3];   // vertices of edge
    
public:
    // meshing functions
    void GenerateBias();
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
    // mesh data
    bool            quadratic;
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

	FEDVertex& Vertex(int n) { return VertexList[n]; }
	FEDEdge& Edge(int n) { return EdgeList[n]; }
	FEDQuad& Quad(int n) { return QuadList[n]; }
	FEDTri& Tri(int n) { return TriList[n]; }
	FEDBox& Box(int n) { return BoxList[n]; }
	FEDWedge& Wedge(int n) { return WedgeList[n]; }
	FEDTet& Tet(int n) { return TetList[n]; }

public:
	std::vector<FEDVertex>   VertexList;
	std::vector<FEDEdge>     EdgeList;
	std::vector<FEDQuad>     QuadList;
	std::vector<FEDTri>      TriList;
	std::vector<FEDBox>      BoxList;
	std::vector<FEDWedge>    WedgeList;
	std::vector<FEDTet>      TetList;
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

	// mesh the domain
	bool MeshDomain();

public:
	FSMesh* m_pmesh;
};
