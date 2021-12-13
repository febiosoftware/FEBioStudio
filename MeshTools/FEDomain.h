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
#include <FECore/vec3d.h>
#include <MeshLib/FEElement.h>
#include <MeshLib/FEMesh.h>

class FEDVertex;
class FEDEdge;
class FEDQuad;
class FEDTri;
class FEDBox;
class FEDWedge;
class FEDTet;

//-------------------------------------------------------------------------------
class FSDomain
{
public:
    //! constructor
    FSDomain() { m_pmesh = 0; };
    FSDomain(FSMesh* pm) { m_pmesh = pm; }
    
    //! destructor
    ~FSDomain();

public:
    int Vertices() const { return (int)VertexList.size();}
    int Edges   () const { return (int)EdgeList.size();  }
    int Quads   () const { return (int)QuadList.size();  }
    int Tris    () const { return (int)TriList.size();   }
    int Boxes   () const { return (int)BoxList.size();   }
    int Wedges  () const { return (int)WedgeList.size(); }
    int Tets    () const { return (int)TetList.size();   }

    FEDVertex&    Vertex (int n) { return VertexList[n]; }
    FEDEdge&	  Edge   (int n) { return EdgeList[n];   }
    FEDQuad&	  Quad   (int n) { return QuadList[n];   }
    FEDTri&       Tri    (int n) { return TriList[n];    }
    FEDBox&       Box    (int n) { return BoxList[n];    }
    FEDWedge&     Wedge  (int n) { return WedgeList[n];  }
    FEDTet&       Tet    (int n) { return TetList[n];    }
    
    FEDVertex*    VertexPtr (int n=0) { return ((n>=0) && (n<(int)VertexList.size())? &VertexList[n] : 0); }
    FEDEdge*      EdgePtr   (int n=0) { return ((n>=0) && (n<(int)EdgeList.size())? &EdgeList[n] : 0);     }
    FEDQuad*	  QuadPtr   (int n=0) { return ((n>=0) && (n<(int)QuadList.size())? &QuadList[n] : 0);     }
    FEDTri* 	  TriPtr    (int n=0) { return ((n>=0) && (n<(int)TriList.size())? &TriList[n] : 0);       }
    FEDBox*       BoxPtr    (int n=0) { return ((n>=0) && (n<(int)BoxList.size())? &BoxList[n] : 0);       }
    FEDWedge*     WedgePtr  (int n=0) { return ((n>=0) && (n<(int)WedgeList.size())? &WedgeList[n] : 0);       }
    FEDTet*       TetPtr    (int n=0) { return ((n>=0) && (n<(int)TetList.size())? &TetList[n] : 0);       }

public:
    vector<FEDVertex>   VertexList;
    vector<FEDEdge>     EdgeList;
    vector<FEDQuad>     QuadList;
    vector<FEDTri>      TriList;
    vector<FEDBox>      BoxList;
    vector<FEDWedge>    WedgeList;
    vector<FEDTet>      TetList;
    
public:
    // find vertex by tag number
    int FindVertexByTagNumber(int n);
    
    // find edge
    int FindEdge(FEDEdge edge);
    int FindEdge(FEDEdge edge, bool& pos);
    
    // find quad
    int FindQuad(FEDQuad quad);
    int FindQuad(FEDQuad quad, bool& pos, int& ist);
    int FindQuadFromFace(FSFace face);
    
    // find tri
    int FindTri(FEDTri tri);
    int FindTri(FEDTri tri, bool& pos, int& ist);
    int FindTriFromFace(FSFace face);
    
    // find box by tag number
    int FindBox(int n);
    
    // find wedge by tag number
    int FindWedge(int n);
    
    // find tet by tag number
    int FindTet(int n);
    
    // add a vertex to this domain
    int AddVertex(FEDVertex vtx);
    
    // add an edge to this domain
    int AddEdge(FEDEdge edge);
    
    // add a quad to this domain
    int AddQuad(FEDQuad quad);
    
    // add a tri to this domain
    int AddTri(FEDTri tri);
    
    // add a box from a list of vertices
    int AddBox(vector<int> vlist, int ntag = -1, int gid = -1);
    
    // add a wedge from a list of vertices
    int AddWedge(vector<int> vlist, int ntag = -1, int gid = -1);
    
    // add a tet from a list of vertices
    int AddTet(vector<int> vlist, int ntag = -1, int gid = -1);
    
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

//-------------------------------------------------------------------------------
class FEDVertex : public FSDomain
{
public:
    //! constructor
    FEDVertex();
    FEDVertex(int m, vec3d x);
    FEDVertex(const FEDVertex& vrtx);
    FEDVertex& operator=(const FEDVertex vrtx);
    
    //! destructor
    ~FEDVertex() { m_edge.clear(); m_quad.clear(); m_tri.clear(); }
    
public:
    vec3d   r;          // vertex position
    int     m_ntag;     // tag number
    vector<int> m_edge; // list of edges connected to this vertex
    vector<int> m_quad; // list of quads connected to this vertex
    vector<int> m_tri;  // list of tris connected to this vertex
};

//-------------------------------------------------------------------------------
class FEDEdge : public FSDomain
{
public:
    //! constructor
    FEDEdge();
    FEDEdge(int v0, int v1);
    FEDEdge(const FEDEdge& edge);
    FEDEdge& operator=(const FEDEdge edge);
    
    //! destructor
    ~FEDEdge() { n.clear(); rbias.clear(); }
    
public:
    int  v[2];   // vertices of edge
    
public:
    // meshing functions
    void GenerateBias();
    
    // create meshed vertices
    bool CreateMesh(FSDomain* pdom);
    
    // mesh data
    vector<int>     n;      // domain vertex list along edge
    int             nseg;   // number of segments along edge
    double          bias;   // mesh bias
    bool            dble;   // flag for bias (true = double, false = single)
    vector<double>  rbias;  // parametric coordinates of biased mesh
};

//-------------------------------------------------------------------------------
class FEDQuad : public FSDomain
{
public:
    //! constructor
    FEDQuad();
    FEDQuad(int v0, int v1, int v2, int v3);
    FEDQuad(const FEDQuad& quad);
    FEDQuad& operator=(const FEDQuad quad);
    
    //! destructor
    ~FEDQuad() { n.clear(); eta1.clear(); eta2.clear(); }
    
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
    vector< vector<int> >     n;      // domain vertex list in face
    vector< vector<double> >  eta1;   // parametric coordinates of vertices
    vector< vector<double> >  eta2;   // parametric coordinates of vertices
    
};

//-------------------------------------------------------------------------------
class FEDTri : public FSDomain
{
public:
    //! constructor
    FEDTri();
    FEDTri(int v0, int v1, int v2, int a = -1);
    FEDTri(const FEDTri& tri);
    FEDTri& operator=(const FEDTri tri);
    
    //! destructor
    ~FEDTri() { n.clear(); eta1.clear(); eta2.clear(); }
    
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
    vector< vector<int> >     n;      // domain vertex list in face
    vector< vector<double> >  eta1;   // parametric coordinates of vertices
    vector< vector<double> >  eta2;   // parametric coordinates of vertices
    
};

//-------------------------------------------------------------------------------
class FEDBox : public FSDomain
{
public:
    //! constructor
    FEDBox();
    FEDBox(const FEDBox& box);
    FEDBox& operator=(const FEDBox box);

    //! destructor
    ~FEDBox() { n.clear(); elem.clear(); }

    // find a box face by node numbers
    int FindBoxFace(FSFace face);
    
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
    vector< vector< vector<int> > >     n;      // domain node list along edge
    vector< vector<int> >               elem;   // elements
    
public:
    // set mesh parameters assuming faces 0,3,4 map identically to faces 2, 1, 5
    bool SetMeshFaces034(int nseg[3], double bias[3], bool dble[3]);
    
    // set mesh parameters along edges normal to a single face
    bool SetMeshSingleFace(int face, int nseg, double bias, bool dble);

protected:
    FSDomain*   m_pDom; // pointer to the domain this vertex belongs to
};

//-------------------------------------------------------------------------------
class FEDWedge : public FSDomain
{
public:
    //! constructor
    FEDWedge();
    FEDWedge(const FEDWedge& wdg);
    FEDWedge& operator=(const FEDWedge wdg);
    
    //! destructor
    ~FEDWedge() { n.clear(); elem.clear(); }
    
    // find a wedge face by FSFace node numbers
    int FindWedgeFace(FSFace face);
    
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
    vector< vector< vector<int> > >     n;      // domain node list
    vector< vector<int> >               elem;   // elements
    
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

//-------------------------------------------------------------------------------
class FEDTet : public FSDomain
{
public:
    //! constructor
    FEDTet();
    FEDTet(const FEDTet& tet);
    FEDTet& operator=(const FEDTet tet);
    
    //! destructor
    ~FEDTet() { n.clear(); elem.clear(); }
    
    // find a tet face by FSFace node numbers
    int FindTetFace(FSFace face);
    
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
    vector<int>             n;      // domain node list
    vector< vector<int> >   elem;   // elements
    
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
