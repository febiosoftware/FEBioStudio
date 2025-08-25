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
#include "FEDomain.h"
using namespace std;

const int FEDQuad::m_edge[4][3] = {
    { 0, 1, 4 }, { 1, 2, 5 }, { 2, 3, 6 }, { 3, 0, 7 }
};

// local node numbering for edges
const int FEDTri::m_edge[3][3] = {
    { 0, 1, 3 }, { 1, 2, 4 }, { 2, 0, 5 }
};
// local edge numbering for apex
const int FEDTri::m_apex[3][3] = {
    { 0, 1, 2 }, { 1, 2, 0 }, { 2, 0, 1 }
};

// local node numbering for edges
const int FEDBox::m_edge[12][3] = {
    { 0, 1, 8 }, { 1, 2, 9 }, { 2, 3, 10 }, { 3, 0, 11 },
    { 4, 5, 12 }, { 5, 6, 13 }, { 6, 7, 14 }, { 7, 4, 15 },
    { 0, 4, 16 }, { 1, 5, 17 }, { 2, 6, 18 }, { 3, 7, 19 }
};

// local node numbering for quad faces
const int FEDBox::m_quad[6][4] = {
    { 0, 1, 5, 4 }, { 1, 2, 6, 5 }, { 2, 3, 7, 6 },
    { 3, 0, 4, 7 }, { 3, 2, 1, 0 }, { 4, 5, 6, 7 }
};

// local node numbering for edges
const int FEDWedge::m_edge[9][3] = {
    { 0, 1, 6}, { 1, 2, 7 }, { 2, 0, 8 },
    { 3, 4, 9 }, { 4, 5, 10 }, { 5, 3, 11 },
    { 0, 3, 12 }, { 1, 4, 13 }, { 2, 5, 14 }
};

// local node numbering for faces
const int FEDWedge::m_face[5][8] = {
    { 0, 1, 4, 3, 6, 17, 9, 12 }, { 1, 2, 5, 4, 7, 14, 10, 17 }, { 0, 3, 5, 2, 12, 11, 14, 8 },
    { 0, 2, 1, 1, 8, 7, 6, 6 }, { 3, 4, 5, 5, 9, 10, 11, 11 }
};

// local node numbering for edges
const int FEDTet::m_edge[6][3] = {
	{ 0, 1, 4 }, { 1, 2, 5 }, { 2, 0, 6 },
	{ 0, 3, 7 }, { 1, 3, 8 }, { 2, 3, 9 }
};

// local node numbering for faces
const int FEDTet::m_face[4][6] = {
	{ 0, 1, 3, 4, 8, 7 }, { 1, 2, 3, 5, 9, 8 }, { 0, 3, 2, 7, 9, 6 }, { 0, 2, 1, 6, 5, 4 }
};

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/////////////////////////////////////////////////////////////////////////////////
// FSDomain
/////////////////////////////////////////////////////////////////////////////////

FSDomain::FSDomain(FSMesh* pm)
{ 
	m_pmesh = pm; 
	m_NLT.assign(pm->Nodes(), -1);
}

//-------------------------------------------------------------------------------
// Find a vertex in the master VertexList by its tag number
int FSDomain::FindVertexByTagNumber(int n)
{
	if ((n < 0) || (n >= m_NLT.size())) return -1;
	int m = m_NLT[n];
	assert((m==-1)||(VertexList[m].m_ntag == n));
	return m;
}

//-------------------------------------------------------------------------------
// Find an edge in the master EdgeList
int FSDomain::FindEdge(const FEDEdge& edge)
{
    // only search the list of edges connected to the first vertex of this edge
    const FEDVertex& v = Vertex(edge.v[0]);
    for (int i=0; i<v.m_edge.size(); ++i) {
        const FEDEdge& e = Edge(v.m_edge[i]);
        if (((e.v[0] == edge.v[0]) && (e.v[1] == edge.v[1])) ||
            ((e.v[0] == edge.v[1]) && (e.v[1] == edge.v[0]))) {
            return v.m_edge[i];
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------
// Find an edge in the master EdgeList
// If the edge is found, pos = true means that the sense of edge is the same as
// the entry in EdgeList, pos = false means that the sense is opposite.
int FSDomain::FindEdge(const FEDEdge& edge, bool& pos)
{
    // only search the list of edges connected to the first vertex of this edge
    const FEDVertex& v = Vertex(edge.v[0]);
    for (int i=0; i<v.m_edge.size(); ++i) {
        const FEDEdge& e = Edge(v.m_edge[i]);
        if ((e.v[0] == edge.v[0]) && (e.v[1] == edge.v[1])) {
            pos = true;
            return v.m_edge[i];
        }
        else if ((e.v[0] == edge.v[1]) && (e.v[1] == edge.v[0])) {
            pos = false;
            return v.m_edge[i];
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------
// Find a quad in the master QuadList
int FSDomain::FindQuad(const FEDQuad& quad)
{
    // only search the list of quads connected to the first vertex of this quad
    bool found_quad, found_node;
    const FEDVertex& v = Vertex(quad.v[0]);
    for (int i=0; i<v.m_quad.size(); ++i) {
        found_quad = true;
        const FEDQuad& q = Quad(v.m_quad[i]);
        for (int j=0; j<4; ++j) {
            found_node = false;
            for (int k=0; k<4; ++k) {
                found_node =  (quad.v[j] == q.v[k]);
                if (found_node) break;
            }
            found_quad = (found_quad && found_node);
            if (!found_quad) break;
        }
        if (found_quad) return v.m_quad[i];
    }
    return -1;
}

//-------------------------------------------------------------------------------
// Find a quad in the master QuadList
int FSDomain::FindQuad(const FEDQuad& quad, bool& pos, int& ist)
{
    // only search the list of quads connected to the first vertex of this quad
    const FEDVertex& v = Vertex(quad.v[0]);
    for (int i=0; i<v.m_quad.size(); ++i) {
        const FEDQuad& q = Quad(v.m_quad[i]);
        if ((q.v[0] == quad.v[0]) && (q.v[1] == quad.v[1]) &&
            (q.v[2] == quad.v[2]) && (q.v[3] == quad.v[3])) {
            pos = true; ist = 0; return v.m_quad[i];
        }
        else if ((q.v[0] == quad.v[1]) && (q.v[1] == quad.v[2]) &&
                 (q.v[2] == quad.v[3]) && (q.v[3] == quad.v[0])) {
            pos = true; ist = 1; return v.m_quad[i];
        }
        else if ((q.v[0] == quad.v[2]) && (q.v[1] == quad.v[3]) &&
                 (q.v[2] == quad.v[0]) && (q.v[3] == quad.v[1])) {
            pos = true; ist = 2; return v.m_quad[i];
        }
        else if ((q.v[0] == quad.v[3]) && (q.v[1] == quad.v[0]) &&
                 (q.v[2] == quad.v[1]) && (q.v[3] == quad.v[2])) {
            pos = true; ist = 3; return v.m_quad[i];
        }
        else if ((q.v[0] == quad.v[3]) && (q.v[1] == quad.v[2]) &&
                 (q.v[2] == quad.v[1]) && (q.v[3] == quad.v[0])) {
            pos = false; ist = 3; return v.m_quad[i];
        }
        else if ((q.v[0] == quad.v[0]) && (q.v[1] == quad.v[3]) &&
                 (q.v[2] == quad.v[2]) && (q.v[3] == quad.v[1])) {
            pos = false; ist = 0; return v.m_quad[i];
        }
        else if ((q.v[0] == quad.v[1]) && (q.v[1] == quad.v[0]) &&
                 (q.v[2] == quad.v[3]) && (q.v[3] == quad.v[2])) {
            pos = false; ist = 1; return v.m_quad[i];
        }
        else if ((q.v[0] == quad.v[2]) && (q.v[1] == quad.v[1]) &&
                 (q.v[2] == quad.v[0]) && (q.v[3] == quad.v[3])) {
            pos = false; ist = 2; return v.m_quad[i];
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------
// Find a quad in the master QuadList from a FSFace
int FSDomain::FindQuadFromFace(const FSFace& face)
{
    if (face.Nodes() != 4) return -1;
    int v[4];
    for (int i=0; i<4; ++i) {
        v[i] = FindVertexByTagNumber(face.n[i]);
        if (v[i] == -1) return -1;
    }
    FEDQuad quad(v[0], v[1], v[2], v[3]);
    return FindQuad(quad);
}

//-------------------------------------------------------------------------------
// Find a tri in the master TriList
int FSDomain::FindTri(const FEDTri& tri)
{
    // only search the list of tris connected to the first vertex of this tri
    bool found_tri, found_node;
    const FEDVertex& v = Vertex(tri.v[0]);
    for (int i=0; i<v.m_tri.size(); ++i) {
        found_tri = true;
        const FEDTri& t = Tri(v.m_tri[i]);
        for (int j=0; j<3; ++j) {
            found_node = false;
            for (int k=0; k<3; ++k) {
                found_node =  (tri.v[j] == t.v[k]);
                if (found_node) break;
            }
            found_tri = (found_tri && found_node);
            if (!found_tri) break;
        }
        if (found_tri) return v.m_tri[i];
    }
    return -1;
}

//-------------------------------------------------------------------------------
// Find a tri in the master TriList
int FSDomain::FindTri(const FEDTri& tri, bool& pos, int& ist)
{
    // only search the list of quads connected to the first vertex of this quad
    const FEDVertex& v = Vertex(tri.v[0]);
    for (int i=0; i<v.m_tri.size(); ++i) {
        FEDTri t = Tri(v.m_tri[i]);
        if ((t.v[0] == tri.v[0]) && (t.v[1] == tri.v[1]) &&
            (t.v[2] == tri.v[2])) {
            pos = true; ist = 0; return v.m_tri[i];
        }
        else if ((t.v[0] == tri.v[1]) && (t.v[1] == tri.v[2]) &&
                 (t.v[2] == tri.v[0])) {
            pos = true; ist = 1; return v.m_tri[i];
        }
        else if ((t.v[0] == tri.v[2]) && (t.v[1] == tri.v[0]) &&
                 (t.v[2] == tri.v[1])) {
            pos = true; ist = 2; return v.m_tri[i];
        }
        else if ((t.v[0] == tri.v[0]) && (t.v[1] == tri.v[2]) &&
                 (t.v[2] == tri.v[1])) {
            pos = false; ist = 0; return v.m_tri[i];
        }
        else if ((t.v[0] == tri.v[1]) && (t.v[1] == tri.v[0]) &&
                 (t.v[2] == tri.v[2])) {
            pos = false; ist = 1; return v.m_tri[i];
        }
        else if ((t.v[0] == tri.v[2]) && (t.v[1] == tri.v[1]) &&
                 (t.v[2] == tri.v[0])) {
            pos = false; ist = 2; return v.m_tri[i];
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------
// Find a tri in the master TriList from a FSFace
int FSDomain::FindTriFromFace(const FSFace& face)
{
    if (face.Nodes() != 3) return -1;
    int v[3];
    for (int i=0; i<3; ++i) {
        v[i] = FindVertexByTagNumber(face.n[i]);
        if (v[i] == -1) return -1;
    }
    FEDTri tri(v[0], v[1], v[2]);
    return FindTri(tri);
}

//-------------------------------------------------------------------------------
// Find a box in the master BoxList by its tag number
int FSDomain::FindBox(int n)
{
    for (int i=0; i<Boxes(); ++i)
        if (Box(i).m_ntag == n) return i;
    
    return -1;
}

//-------------------------------------------------------------------------------
// Find a wedge in the master WedgeList by its tag number
int FSDomain::FindWedge(int n)
{
    for (int i=0; i<Wedges(); ++i)
        if (Wedge(i).m_ntag == n) return i;
    
    return -1;
}

//-------------------------------------------------------------------------------
// Find a tet in the master TetList by its tag number
int FSDomain::FindTet(int n)
{
    for (int i=0; i<Tets(); ++i)
        if (Tet(i).m_ntag == n) return i;
    
    return -1;
}

//-------------------------------------------------------------------------------
// Add a vertex to this domain
int FSDomain::AddVertex(int n, const vec3d& r)
{
    // get new vertex number
    int vn = Vertices();
    // store vertex in VertexList
    VertexList.push_back(FEDVertex(n, r));

	if (n >= 0) m_NLT[n] = vn;
    
    return vn;
}

//-------------------------------------------------------------------------------
// Add an edge to this domain
int FSDomain::AddEdge(FEDEdge edge)
{
    // get new edge number
    int en = Edges();
    // store edge in EdgeList
    EdgeList.push_back(edge);

    // store edge number in each vertex
    Vertex(edge.v[0]).m_edge.push_back(en);
    Vertex(edge.v[1]).m_edge.push_back(en);
    
    return en;
}

//-------------------------------------------------------------------------------
// Add a quad to this domain
int FSDomain::AddQuad(FEDQuad quad)
{
    // get new quad number
    int qn = Quads();
    // store quad in QuadList
    quad.m_ntag = qn;
    QuadList.push_back(quad);
    
    // store quad number in each vertex
    Vertex(quad.v[0]).m_quad.push_back(qn);
    Vertex(quad.v[1]).m_quad.push_back(qn);
    Vertex(quad.v[2]).m_quad.push_back(qn);
    Vertex(quad.v[3]).m_quad.push_back(qn);
    
    return qn;
}

//-------------------------------------------------------------------------------
// Add a tri to this domain
int FSDomain::AddTri(FEDTri tri)
{
    // get new tri number
    int tn = Tris();
    // store tri in TriList
    tri.m_ntag = tn;
    TriList.push_back(tri);
    
    // store tri number in each vertex
    Vertex(tri.v[0]).m_tri.push_back(tn);
    Vertex(tri.v[1]).m_tri.push_back(tn);
    Vertex(tri.v[2]).m_tri.push_back(tn);
    
    return tn;
}

//-------------------------------------------------------------------------------
// Add a box from a list of vertices in this domain
int FSDomain::AddBox(vector<int> vlist, int ntag, int gid)
{
    // check vlist
    if (vlist.size() < 8)
        return -1;
    
    // create a box
    FEDBox box;
    box.m_ntag = ntag;
    box.m_gid = gid;
    box.SetDomain(this);
    
    // store the supplied vertices into the box vertex list
    for (int i=0; i<8; ++i)
        box.v[i] = vlist[i];
    
    // store the edges into the edge list
    for (int i=0; i<12; ++i) {
        bool ep;
        FEDEdge edge(box.v[box.m_edge[i][0]],box.v[box.m_edge[i][1]]);
        int en = FindEdge(edge, ep);
        if (en == -1) {
            box.e[i] = AddEdge(edge);
            box.ep[i] = true;
        }
        else {
            box.e[i] = en;
            box.ep[i] = ep;
        }
    }
    
    // store the quad faces into the quad list
    bool qp;
    int qst;
    for (int i=0; i<6; ++i) {
        FEDQuad quad(box.v[box.m_quad[i][0]],
                     box.v[box.m_quad[i][1]],
                     box.v[box.m_quad[i][2]],
                     box.v[box.m_quad[i][3]]);
        int qn = FindQuad(quad, qp, qst);
        if (qn == -1) {
            bool ep;
            for (int j=0; j<4; ++j) {
                FEDEdge edge(quad.v[quad.m_edge[j][0]],
                             quad.v[quad.m_edge[j][1]]);
                quad.e[j] = FindEdge(edge, ep);
                quad.ep[j] = ep;
            }
            box.q[i] = AddQuad(quad);
            box.qp[i] = true;
            box.qst[i] = 0;
        }
        else {
            box.q[i] = qn;
            box.qp[i] = qp;
            box.qst[i] = qst;
        }
    }
    
    int ibox = Boxes();
    BoxList.push_back(box);
    
    return ibox;
}

//-------------------------------------------------------------------------------
// Add a wedge from a list of vertices in this domain
int FSDomain::AddWedge(vector<int> vlist, int ntag, int gid)
{
    // check vlist
    if (vlist.size() < 6)
        return -1;
    
    // create a wedge
    FEDWedge wdg;
    wdg.m_ntag = ntag;
    wdg.m_gid = gid;
    wdg.SetDomain(this);
    
    // store the supplied vertices into the wedge vertex list
    for (int i=0; i<6; ++i)
        wdg.v[i] = vlist[i];
    
    // store the edges into the edge list
    for (int i=0; i<9; ++i) {
        bool ep;
        FEDEdge edge(wdg.v[wdg.m_edge[i][0]],wdg.v[wdg.m_edge[i][1]]);
        int en = FindEdge(edge, ep);
        if (en == -1) {
            wdg.e[i] = AddEdge(edge);
            wdg.ep[i] = true;
        }
        else {
            wdg.e[i] = en;
            wdg.ep[i] = ep;
        }
    }
    
    bool fp;
    int fst;
    // store the quad faces into the quad list
    for (int i=0; i<3; ++i) {
        FEDQuad quad(wdg.v[wdg.m_face[i][0]],
                     wdg.v[wdg.m_face[i][1]],
                     wdg.v[wdg.m_face[i][2]],
                     wdg.v[wdg.m_face[i][3]]);
        int qn = FindQuad(quad, fp, fst);
        if (qn == -1) {
            bool ep;
            for (int j=0; j<4; ++j) {
                FEDEdge edge(quad.v[quad.m_edge[j][0]],
                             quad.v[quad.m_edge[j][1]]);
                quad.e[j] = FindEdge(edge, ep);
                quad.ep[j] = ep;
            }
            wdg.f[i] = AddQuad(quad);
            wdg.fp[i] = true;
            wdg.fst[i] = 0;
        }
        else {
            wdg.f[i] = qn;
            wdg.fp[i] = fp;
            wdg.fst[i] = fst;
        }
    }
    // store the tri faces into the tri list
    for (int i=3; i<5; ++i) {
        FEDTri tri(wdg.v[wdg.m_face[i][0]],
                   wdg.v[wdg.m_face[i][1]],
                   wdg.v[wdg.m_face[i][2]]);
        int tn = FindTri(tri, fp, fst);
        if (tn == -1) {
            bool ep;
            for (int j=0; j<3; ++j) {
                FEDEdge edge(tri.v[tri.m_edge[j][0]],
                             tri.v[tri.m_edge[j][1]]);
                tri.e[j] = FindEdge(edge, ep);
                tri.ep[j] = ep;
            }
            wdg.f[i] = AddTri(tri);
            wdg.fp[i] = true;
            wdg.fst[i] = 0;
        }
        else {
            wdg.f[i] = tn;
            wdg.fp[i] = fp;
            wdg.fst[i] = fst;
        }
    }
    
    int iwdg = Wedges();
    WedgeList.push_back(wdg);
    
    return iwdg;
}

//-------------------------------------------------------------------------------
// Add a tet from a list of vertices in this domain
int FSDomain::AddTet(vector<int> vlist, int ntag, int gid)
{
    // check vlist
    if (vlist.size() < 4)
        return -1;
    
    // create a tet
    FEDTet tet;
    tet.m_ntag = ntag;
    tet.m_gid = gid;
    tet.SetDomain(this);
    
    // store the supplied vertices into the wedge vertex list
    for (int i=0; i<4; ++i)
        tet.v[i] = vlist[i];
    
    // store the edges into the edge list
    for (int i=0; i<6; ++i) {
        bool ep;
        FEDEdge edge(tet.v[tet.m_edge[i][0]],tet.v[tet.m_edge[i][1]]);
        int en = FindEdge(edge, ep);
        if (en == -1) {
            tet.e[i] = AddEdge(edge);
            tet.ep[i] = true;
        }
        else {
            tet.e[i] = en;
            tet.ep[i] = ep;
        }
    }
    
    bool fp;
    int fst;
    // store the tri faces into the tri list
    for (int i=0; i<4; ++i) {
        FEDTri tri(tet.v[tet.m_face[i][0]],
                   tet.v[tet.m_face[i][1]],
                   tet.v[tet.m_face[i][2]]);
        int tn = FindTri(tri, fp, fst);
        if (tn == -1) {
            bool ep;
            for (int j=0; j<3; ++j) {
                FEDEdge edge(tri.v[tri.m_edge[j][0]],
                             tri.v[tri.m_edge[j][1]]);
                tri.e[j] = FindEdge(edge, ep);
                tri.ep[j] = ep;
            }
            tet.f[i] = AddTri(tri);
            tet.fp[i] = true;
            tet.fst[i] = 0;
        }
        else {
            tet.f[i] = tn;
            tet.fp[i] = fp;
            tet.fst[i] = fst;
        }
    }
    
    int itet = Tets();
    TetList.push_back(tet);
    
    return itet;
}

//-------------------------------------------------------------------------------
// Split a box into two wedges
// Use before setting mesh parameters and meshing
void FSDomain::SplitBoxIntoWedges(int ibox, int iedge, int iopt, int iwdg[2])
{
    int wdg_opt0[2][12][6] = {
        {
            {0,3,7,1,2,6},{1,0,4,2,3,7},{3,7,4,2,6,5},{0,4,5,3,7,6},
            {4,0,3,5,1,2},{5,1,0,6,2,3},{7,4,0,6,5,1},{4,5,1,7,6,2},
            {0,1,2,4,5,6},{1,2,3,5,6,7},{2,3,0,6,7,4},{3,0,1,7,4,5}
        },
        {
            {0,7,4,1,6,5},{1,4,5,2,7,6},{3,4,0,2,5,1},{0,5,1,3,6,2},
            {4,3,7,5,2,6},{5,0,4,6,3,7},{7,0,3,6,1,2},{4,1,0,7,2,3},
            {0,2,3,4,6,7},{1,3,0,5,7,4},{2,0,1,6,4,5},{3,1,2,7,5,6}
        }
    };
    
    int wdg_opt1[2][12][6] = {
        {
            {7,4,0,6,5,1},{4,5,1,7,6,2},{4,0,3,5,1,2},{5,1,0,6,2,3},
            {3,7,4,2,6,5},{0,4,5,3,7,6},{0,3,7,1,2,6},{1,0,4,2,3,7},
            {2,3,0,6,7,4},{3,0,1,7,4,5},{0,1,2,4,5,6},{1,2,3,5,6,7}
        },
        {
            {7,0,3,6,1,2},{4,1,0,7,2,3},{4,3,7,5,2,6},{5,0,4,6,3,7},
            {3,4,0,2,5,1},{0,5,1,3,6,2},{0,7,4,1,6,5},{1,4,5,2,7,6},
            {2,0,1,6,4,5},{3,1,2,7,5,6},{0,2,3,4,6,7},{1,3,0,5,7,4}
        }
    };
    
    int split_quads[12][2] = {
        {1,3},{0,2},{1,3},{0,2},{1,3},{0,2},{1,3},{0,2},{4,5},{4,5},{4,5},{4,5}
    };
    
    FEDBox& box = Box(ibox);
    vector<int> vlist(6);

    // for each of two wedges
    for (int i=0; i<2; ++i) {
        // get the list of vertices from the box
        if (iopt == 0) {
            for (int j=0; j<6; ++j)
                vlist[j] = box.v[wdg_opt0[i][iedge][j]];
        }
        else {
            for (int j=0; j<6; ++j)
                vlist[j] = box.v[wdg_opt1[i][iedge][j]];
        }
        // create the wedge
        iwdg[i] = AddWedge(vlist,box.m_ntag, box.m_gid);
    }
    
    // tag the box to be excluded from the domain
    box.m_ntag = -1;
    
    // tag the split quads to be excluded from the domain
    Quad(box.q[split_quads[iedge][0]]).m_ntag = -1;
    Quad(box.q[split_quads[iedge][1]]).m_ntag = -1;
    
}

//-------------------------------------------------------------------------------
// Split a wedge into three tets
// Use before setting mesh parameters and meshing
void FSDomain::SplitWedgeIntoTets(int iwdg, int ivtx, int itet[3])
{
    int tet[6][3][4] = {
        {{0,1,2,3},{1,2,3,4},{2,3,4,5}},
        {{1,2,0,4},{2,0,4,5},{0,4,5,3}},
        {{2,0,1,5},{0,1,5,3},{1,5,3,4}},
        {{3,5,4,0},{5,4,0,2},{4,0,2,1}},
        {{4,3,5,1},{3,5,1,0},{5,1,0,2}},
        {{5,4,3,2},{4,3,2,1},{3,2,1,0}}
    };
    
    int split_quad[6] = {1,2,0,1,2,0};
    
    FEDWedge& wdg = Wedge(iwdg);
    vector<int> vlist(4);
    
    // for each of three tets
    for (int i=0; i<3; ++i) {
        // get the list of vertices from the wedge
        for (int j=0; j<4; ++j)
            vlist[j] = wdg.v[tet[ivtx][i][j]];
        // create the tet
        itet[i] = AddTet(vlist,wdg.m_ntag,wdg.m_gid);
    }
    
    // tag the wedge to be excluded from the domain
    wdg.m_ntag = -1;
    
    // tag the split quad to be excluded from the domain
    Quad(wdg.f[split_quad[ivtx]]).m_ntag = -1;
    
}

//-------------------------------------------------------------------------------
// Add an element as a domain in FSDomain
bool FSDomain::AddElement(int iel)
{
    const FSElement& el = m_pmesh->Element(iel);
    int gid = el.m_gid;
    
    vector<int> vlist(el.Nodes());
    
    // store the element nodes into the vertex list
    for (int i=0; i<el.Nodes(); ++i) {
        int n = el.m_node[i];
        int vn = FindVertexByTagNumber(n);
        if (vn == -1) {
            FSNode& node = m_pmesh->Node(el.m_node[i]);
            vlist[i] = AddVertex(el.m_node[i], node.r);
        }
        else
            vlist[i] = vn;
    }
    
    if (el.Type() == FE_HEX8) {
        
        // check if this element has been added already
        if (FindBox(iel) != -1) return false;
        
        // otherwise, add it as a box to this domain
        AddBox(vlist, iel, gid);
        return true;
    }
    else if (el.Type() == FE_PENTA6) {
        
        // check if this element has been added already
        if (FindWedge(iel) != -1) return false;
        
        // otherwise, add it as a wedge to this domain
        AddWedge(vlist, iel, gid);
        return true;
    }
    else if (el.Type() == FE_TET4) {
        
        // check if this element has been added already
        if (FindTet(iel) != -1) return false;
        
        // otherwise, add it as a tet to this domain
        AddTet(vlist, iel, gid);
        return true;
    }
    
    return false;
}

//-------------------------------------------------------------------------------
// Reset mesh parameters
void FSDomain::ResetMeshParameters()
{
    for (int i=0; i<Edges(); ++i) {
        Edge(i).nseg = -1;
        Edge(i).bias = 1;
        Edge(i).dble = false;
    }
}

//-------------------------------------------------------------------------------
bool FSDomain::MeshEdges()
{
    for (int i=0; i<Edges(); ++i) {
        FEDEdge& edge = Edge(i);
        bool meshed = edge.CreateMesh(this);
        if (!meshed) return false;
    }
    return true;
}

//-------------------------------------------------------------------------------
bool FSDomain::MeshQuads()
{
    for (int i=0; i<Quads(); ++i) {
        FEDQuad& quad = Quad(i);
        // mesh only if tag != -1
        if (quad.m_ntag != -1)
            if (!quad.CreateMesh(this))
                return false;
    }
    return true;
}

//-------------------------------------------------------------------------------
bool FSDomain::MeshTris()
{
    for (int i=0; i<Tris(); ++i) {
        FEDTri& tri = Tri(i);
        // mesh only if tag != -1
        if (tri.m_ntag != -1)
            if (!tri.CreateMesh(this))
                return false;
    }
    return true;
}

//-------------------------------------------------------------------------------
bool FSDomain::MeshBoxes()
{
    for (int i=0; i<Boxes(); ++i) {
        FEDBox& box = Box(i);
        // mesh only if tag != -1
        if (box.m_ntag != -1)
            if (!box.CreateMesh(this))
                return false;
    }
    return true;
}

//-------------------------------------------------------------------------------
bool FSDomain::MeshWedges()
{
    for (int i=0; i<Wedges(); ++i) {
        FEDWedge& wdg = Wedge(i);
        if (wdg.m_ntag != -1)
            if (!wdg.CreateMesh(this))
                return false;
    }
    return true;
}

//-------------------------------------------------------------------------------
bool FSDomain::MeshTets()
{
    for (int i=0; i<Tets(); ++i) {
        FEDTet& tet = Tet(i);
        if (tet.m_ntag != -1)
            if (!tet.CreateMesh(this))
                return false;
    }
    return true;
}

//-------------------------------------------------------------------------------
bool FSDomain::MeshDomain()
{
    // mesh edges, quads and boxes
    if (!MeshEdges() ) return false;
    if (!MeshQuads() ) return false;
    if (!MeshTris()  ) return false;
    if (!MeshBoxes() ) return false;
    if (!MeshWedges()) return false;
    if (!MeshTets()  ) return false;
    
    // count all the newly created vertices
    int n0 = m_pmesh->Nodes();
    int n1 = 0;
    for (int i=0; i<Vertices(); ++i)
        if (Vertex(i).m_ntag == -1) ++n1;

    // allocate room for new nodes
    m_pmesh->Create(n0 + n1, 0);

    // create new nodes
    n1 = 0;
    for (int i=0; i<Vertices(); ++i) {
        FEDVertex& vtx = Vertex(i);
        if (vtx.m_ntag == -1) {
            FSNode& node = m_pmesh->Node(n0 + n1);
            node.r = vtx.r;
            // store node number in vertex tag
            node.m_ntag = vtx.m_ntag = n0 + n1;
            ++n1;
        }
    }
    
    // create new elements
    int ne0 = m_pmesh->Elements();
    int ne1 = 0;
    // count boxes
    for (int i=0; i<Boxes(); ++i)
        ne1 += Box(i).elem.size();
    // count wedges
    for (int i=0; i<Wedges(); ++i)
        ne1 += Wedge(i).elem.size();
    // count tets
    for (int i=0; i<Tets(); ++i)
        ne1 += Tet(i).elem.size();
    m_pmesh->Create(0, ne0 + ne1);
    ne1 = 0;
    // create box elements
    for (int i=0; i<Boxes(); ++i) {
        FEDBox& box = Box(i);
        for (int j=0; j<box.elem.size(); ++j) {
            FSElement& el = m_pmesh->Element(ne0 + ne1);
            el.SetType(FE_HEX8);
            el.m_gid = box.m_gid; assert(el.m_gid >= 0);
            for (int k=0; k<8; ++k)
                el.m_node[k] = Vertex(box.elem[j][k]).m_ntag;
            ++ne1;
        }
    }
    // create wedge elements
    for (int i=0; i<Wedges(); ++i) {
        FEDWedge& wdg = Wedge(i);
        for (int j=0; j<wdg.elem.size(); ++j) {
            FSElement& el = m_pmesh->Element(ne0 + ne1);
            el.m_gid = wdg.m_gid; assert(el.m_gid >= 0);
            if (wdg.elem[j].size() == 8) {
                el.SetType(FE_HEX8);
                for (int k=0; k<8; ++k)
                    el.m_node[k] = Vertex(wdg.elem[j][k]).m_ntag;
                ++ne1;
            }
            else if (wdg.elem[j].size() == 6) {
                el.SetType(FE_PENTA6);
                for (int k=0; k<6; ++k)
                    el.m_node[k] = Vertex(wdg.elem[j][k]).m_ntag;
                ++ne1;
            }
        }
    }
    // create tet elements
    for (int i=0; i<Tets(); ++i) {
        FEDTet& tet = Tet(i);
        for (int j=0; j<tet.elem.size(); ++j) {
            FSElement& el = m_pmesh->Element(ne0 + ne1);
            el.m_gid = tet.m_gid; assert(el.m_gid >= 0);
            if (tet.elem[j].size() == 4) {
                el.SetType(FE_TET4);
                for (int k=0; k<4; ++k)
                    el.m_node[k] = Vertex(tet.elem[j][k]).m_ntag;
                ++ne1;
            }
            else if (tet.elem[j].size() == 6) {
                el.SetType(FE_PENTA6);
                for (int k=0; k<6; ++k)
                    el.m_node[k] = Vertex(tet.elem[j][k]).m_ntag;
                ++ne1;
            }
            else if (tet.elem[j].size() == 8) {
                el.SetType(FE_HEX8);
                for (int k=0; k<8; ++k)
                    el.m_node[k] = Vertex(tet.elem[j][k]).m_ntag;
                ++ne1;
            }
        }
    }

    return true;
}

//-------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////
// FEDVertex
/////////////////////////////////////////////////////////////////////////////////

FEDVertex::FEDVertex()
{
    m_ntag = -1;
    r = vec3d(0,0,0);
}

//-------------------------------------------------------------------------------
FEDVertex::FEDVertex(int m, const vec3d& x)
{
    m_ntag = m;
    r = x;
}

/////////////////////////////////////////////////////////////////////////////////
// FEDEdge
/////////////////////////////////////////////////////////////////////////////////

FEDEdge::FEDEdge()
{
    quadratic = false;
    nseg = -1;
    bias = 1;
    dble = false;
    v[0] = v[1] = v[2] = -1;
}

//-------------------------------------------------------------------------------
FEDEdge::FEDEdge(int v0, int v1, int v2)
{
    if (v2 == -1) quadratic = false;
    nseg = -1;
    bias = 1;
    dble = false;
    v[0] = v0; v[1] = v1; v[2] = v2;
}

//-------------------------------------------------------------------------------
void FEDEdge::GenerateBias()
{
    if (nseg < 1) return;

    // find the normalized extrusion distances
    int NSEG = nseg;
    if (quadratic) NSEG = 2*nseg;
    rbias.resize(NSEG+1);
    rbias[0] = 0;
    rbias[NSEG] = 1;
    // uniform mesh
    if (bias == 1) {
        double dr = 1./nseg;
        for (int i=1; i<nseg; ++i) rbias[i] = dr*i;
    }
    else if (dble)
    {
        double dr = (bias - 1)/(pow(bias, nseg) - 1);
        rbias[1] = dr;
        for (int i=2; i<nseg; ++i) rbias[i] = rbias[i-1] + dr*pow(bias, i-1);
    }
    else
    {
        double dr = (bias - 1)/(pow(bias, nseg) - 1);
        rbias[1] = dr;
        for (int i=2; i<nseg; ++i) rbias[i] = rbias[i-1] + dr*pow(bias, i-1);
    }
    if (quadratic)
        for (int i=1; i<=nseg; ++i) rbias[nseg+i] = (rbias[i-1]+rbias[i])/2;

}

//-------------------------------------------------------------------------------
bool FEDEdge::CreateMesh(FSDomain* pdom)
{
    // check if we can mesh
    if (nseg < 1) return false;

    // generate the biased parametric coordinates
    GenerateBias();
    
    // store the start and end vertices in the vertex list
    int NSEG = nseg;
    if (quadratic) NSEG = 2*nseg;
    n.resize(NSEG+1,-1);
    
    // create new vertices by interpolation
    vec3d r0 = pdom->Vertex(v[0]).r;
    vec3d r1 = pdom->Vertex(v[1]).r;

    if (quadratic) {
        vec3d r2 = pdom->Vertex(v[2]).r;
        for (int i=0; i<=NSEG+1; ++i) {
            double eta = rbias[i];
            vec3d r = r0*N(0,eta) + r1*N(1,eta) + r2*N(2,eta);
            n[nseg+i] = pdom->AddVertex(-1, r);
        }
    }
    else {
        for (int i=0; i<=nseg; ++i) {
            double eta = rbias[i];
            vec3d r = r0*N(0,eta) + r1*N(1,eta);
            n[i] = pdom->AddVertex(-1, r);
        }
    }
    
    return true;
}

// Edge shape functions when 0 ≤ eta ≤ 1
double FEDEdge::N(int a, double eta)
{
    if (quadratic) {
        switch (a) {
            case 0:
                return 2*(1-eta)*(0.5-eta);
                break;
                
            case 1:
                return 2*eta*(eta-0.5);
                break;
                
            case 2:
                return 4*eta*(1-eta);
                break;
                
            default:
                return -1;
                break;
        }
    }
    else {
        switch (a) {
            case 0:
                return 1-eta;
                break;
                
            case 1:
                return eta;
                break;
                
            default:
                return -1;
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////
// FEDQuad
/////////////////////////////////////////////////////////////////////////////////

FEDQuad::FEDQuad()
{
    quadratic = false;
    m_ntag = -1;
    v[0] = v[1] = v[2] = v[3] = -1;
    e[0] = e[1] = e[2] = e[3] = -1;
}

//-------------------------------------------------------------------------------
FEDQuad::FEDQuad(int v0, int v1, int v2, int v3,
                 int v4, int v5, int v6, int v7)
{
    if (v4 == -1) quadratic = false;
    m_ntag = -1;
    v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
    v[4] = v4; v[5] = v5; v[6] = v6; v[7] = v7;
    e[0] = e[1] = e[2] = e[3] = -1;
}

//-------------------------------------------------------------------------------
bool FEDQuad::CreateMesh(FSDomain* pdom)
{
    // in 2D
    // edge 0 of quad = eta1 axis
    // edge 3 of quad = eta2 axis (-ve)
    int mseg = pdom->Edge(e[0]).nseg;   // number of segments along eta1
    int nseg = pdom->Edge(e[3]).nseg;   // number of segments along eta2

    int MSEG = mseg;
    int NSEG = nseg;
    if ((pdom->Edge(e[0]).quadratic && pdom->Edge(e[3]).quadratic) != quadratic)
        return false;
    if (quadratic) { MSEG = 2*mseg; NSEG = 2*nseg; }

    // check if we can mesh
    if ((mseg < 1) || (nseg < 1)) return false;
    
    // allocate memory for vertex list
    // eta1 and eta2 double arrays sized as [MSEG+1][NSEG+1]
    n.resize(MSEG+1);
    eta1.resize(MSEG+1);
    eta2.resize(MSEG+1);
    for (int i=0; i<= MSEG; ++i) {
        n[i].resize(NSEG+1);
        eta1[i].resize(NSEG+1);
        eta2[i].resize(NSEG+1);
    }
    
    // eta values are evaluated from the bias and increase from 0 to 1
    // store the first (eta2=0) and last (eta2=1) set of eta1's in vertex list
    int i,j,k,l;
    for (i=0; i<=mseg; ++i) {
        k = (ep[0] ? i : (mseg - i));
        n[i][0] = pdom->Edge(e[0]).n[k];
        eta1[i][0] = pdom->Edge(e[0]).rbias[k];
        eta2[i][0] = 0;
        k = ep[2] ? (mseg - i) : i;
        n[i][nseg] = pdom->Edge(e[2]).n[k];
        eta1[i][nseg] = pdom->Edge(e[2]).rbias[k];
        eta2[i][nseg] = 1;
    }
    
    // store the first (eta1=0) and last (eta1=1) set of eta2's in vertex list
    for (j=0; j<=nseg; ++j) {
        l = ep[3] ? (nseg - j) : j;
        n[0][j] = pdom->Edge(e[3]).n[l];
        eta1[0][j] = 0;
        eta2[0][j] = pdom->Edge(e[3]).rbias[l];
        l = ep[1] ? j : (nseg - j);
        n[mseg][j] = pdom->Edge(e[1]).n[l];
        eta1[mseg][j] = 1;
        eta2[mseg][j] = pdom->Edge(e[1]).rbias[l];
    }
    
    for (i=0; i<MSEG+1; ++i) {
        k = (ep[0] ? i : (MSEG - i));
        double h1 = pdom->Edge(e[0]).rbias[k];
        for (j=0; j<NSEG+1; ++j) {
            l = ep[3] ? (NSEG - j) : j;
            eta1[i][j] = h1;
        }
    }
    for (j=0; j<NSEG+1; ++j) {
        l = ep[3] ? (NSEG - j) : j;
        double h2 = pdom->Edge(e[3]).rbias[l];
        for (i=0; i<MSEG+1; ++i) {
            eta2[i][j] = h2;
        }
    }
    
    vec3d r0 = pdom->Vertex(v[0]).r;
    vec3d r1 = pdom->Vertex(v[1]).r;
    vec3d r2 = pdom->Vertex(v[2]).r;
    vec3d r3 = pdom->Vertex(v[3]).r;
    if (quadratic) {
        vec3d r4 = pdom->Vertex(v[4]).r;
        vec3d r5 = pdom->Vertex(v[5]).r;
        vec3d r6 = pdom->Vertex(v[6]).r;
        vec3d r7 = pdom->Vertex(v[7]).r;
        // create new vertices by bilinear interpolation
        for (i=0; i<=MSEG+1; ++i) {
            for (j=0; j<=NSEG+1; ++j) {
                double h1 = eta1[i][j];
                double h2 = eta2[i][j];
                vec3d r = r0*N(0,0,h1,h2)+r1*N(1,0,h1,h2)+r2*N(1,1,h1,h2)+r3*N(0,1,h1,h2)
                +r4*N(2,0,h1,h2)+r5*N(1,2,h1,h2)+r6*N(2,1,h1,h2)+r7*N(0,2,h1,h2);
                n[i][j] = pdom->AddVertex(-1, r);
            }
        }
    }
    else {
        // create new vertices by bilinear interpolation
        for (i=1; i<mseg; ++i) {
            for (j=1; j<nseg; ++j) {
                double h1 = eta1[i][j];
                double h2 = eta2[i][j];
                vec3d r = r0*N(0,0,h1,h2)+r1*N(1,0,h1,h2)+r2*N(1,1,h1,h2)+r3*N(0,1,h1,h2);
                n[i][j] = pdom->AddVertex(-1, r);
            }
        }
    }
    
    return true;
}

//-------------------------------------------------------------------------------
// evaluate shape function
double FEDQuad::N(int a, int b, double eta1, double eta2) {
    FEDEdge edge;
    double Na = edge.N(a, eta1);
    double Nb = edge.N(b, eta2);
    return Na*Nb;
}


/////////////////////////////////////////////////////////////////////////////////
// FEDTri
/////////////////////////////////////////////////////////////////////////////////

FEDTri::FEDTri()
{
    m_ntag = -1;
    m_fne = -1;
    v[0] = v[1] = v[2] = -1;
    e[0] = e[1] = e[2] = -1;
}

//-------------------------------------------------------------------------------
FEDTri::FEDTri(int v0, int v1, int v2, int a)
{
    m_ntag = -1;
    m_fne = a;
    v[0] = v0; v[1] = v1; v[2] = v2;
    e[0] = e[1] = e[2] = -1;
}

//-------------------------------------------------------------------------------
// find a vertex in the triangle
int FEDTri::FindVertex(int vtx)
{
    if (v[0] == vtx) return 0;
    else if (v[1] == vtx) return 1;
    else if (v[2] == vtx) return 2;
    else return -1;
}

//-------------------------------------------------------------------------------
bool FEDTri::CreateMesh(FSDomain* pdom)
{
    // in 2D
    // edge 0 of tri = eta1 axis
    // edge 2 of tri = eta2 axis (+ve)
    int e0, e1, e2;
    switch (m_fne) {
        case 0 : e0 = 0; e1 = 1; e2 = 2; break;
        case 1 : e0 = 1; e1 = 2; e2 = 0; break;
        case 2 : e0 = 2; e1 = 0; e2 = 1; break;
        default: e0 = 0; e1 = 1; e2 = 2; break;
    }
    
    const FEDEdge& edg0 = pdom->Edge(e[e0]);
    const FEDEdge& edg1 = pdom->Edge(e[e1]);
    const FEDEdge& edg2 = pdom->Edge(e[e2]);
    int mseg = edg0.nseg;   // number of segments along eta1
    int nseg = edg1.nseg;   // number of segments along eta2
    
    // check if we can mesh
    if ((mseg < 1) || (nseg < 1)) return false;
    
    // allocate memory for vertex list
    // eta1 and eta2 double arrays sized as [mseg+1][nseg+1]
    n.resize(mseg+1);
    eta1.resize(mseg+1);
    eta2.resize(mseg+1);
    for (int i=0; i<= mseg; ++i) {
        n[i].resize(nseg+1);
        eta1[i].resize(nseg+1);
        eta2[i].resize(nseg+1);
    }
    
    // eta values are evaluated from the bias and increase from 0 to 1
    // store the first (eta2=0) and last (eta2=eta1) set of eta1's in vertex list
    int i,j,k,l;
    for (i=0; i<=mseg; ++i) {
        k = (ep[e0] ? i : (mseg - i));
        n[i][0] = edg0.n[k];
        eta1[i][0] = edg0.rbias[k];
        eta2[i][0] = 0;
        k = ep[e2] ? (mseg - i) : i;
        n[i][nseg] = edg2.n[k];
        eta1[i][nseg] = edg2.rbias[k];
        eta2[i][nseg] = eta1[i][nseg];
    }
    
    // store the first (eta1=0) and last (eta1=1) set of eta2's in vertex list
    for (j=0; j<=nseg; ++j) {
        n[0][j] = ep[e0] ? edg0.v[0] : edg0.v[1];
        eta1[0][j] = 0;
        eta2[0][j] = 0;
        l = ep[e1] ? j : (nseg - j);
        n[mseg][j] = edg1.n[l];
        eta1[mseg][j] = 1;
        eta2[mseg][j] = edg1.rbias[l];
    }
    
    // create new vertices by bilinear interpolation
    vec3d r0 = pdom->Vertex(n[0][0]).r;
    vec3d r1 = pdom->Vertex(n[mseg][0]).r;
    vec3d r2 = pdom->Vertex(n[mseg][nseg]).r;
    vec3d r3 = pdom->Vertex(n[0][nseg]).r;
    for (i=1; i<mseg; ++i) {
        double eta10 = eta1[i][0];
        double eta12 = eta1[i][nseg];
        for (j=1; j<nseg; ++j) {
            double eta23 = eta2[0][j];
            double eta21 = eta2[mseg][j];
            
            double den = (1 + eta10*eta21 - eta12*eta21 - eta10*eta23 + eta12*eta23);
            eta1[i][j] = (eta10 - eta10*eta23 + eta12*eta23)/den;
            eta2[i][j] = (eta10*eta21 + eta23 - eta10*eta23)/den;
            // TODO: use a parametric vector function to evaluate surface points
            // for any desired surface shape, using this bilinear interpolation
            // in parametric space
            double h1 = eta1[i][j];
            double h2 = eta2[i][j];
            vec3d r = r0*(1-h1)*(1-h2) +r1*h1*(1-h2) + r2*h1*h2 + r3*(1-h1)*h2;
            n[i][j] = pdom->AddVertex(-1, r);
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
// FEDBox
/////////////////////////////////////////////////////////////////////////////////

FEDBox::FEDBox()
{
    m_ntag = -1;
    m_gid = -1;
    for (int i=0; i< 8; ++i) v[i] = -1;
    for (int i=0; i<12; ++i) {
        e[i] = -1;
        ep[i] = true;
    }
    for (int i=0; i< 6; ++i) {
        q[i] = -1;
        qp[i] = true;
        qst[i] = -1;
    }
    m_pDom = nullptr;
}

//-------------------------------------------------------------------------------
// Find a box box face by node numbers
int FEDBox::FindBoxFace(const FSFace& face)
{
	int v[4] = { 0 };
    // find the vertices from the node list
    for (int i=0; i<face.Nodes(); ++i) {
        v[i] = m_pDom->FindVertexByTagNumber(face.n[i]);
        if (v[i] == -1)
            return -1;
    }
    
    for (int i=0; i<6; ++i) {
        const FEDQuad& quad = m_pDom->Quad(q[i]);
        if ((v[0] == quad.v[0]) && (v[1] == quad.v[1]) &&
            (v[2] == quad.v[2]) && (v[3] == quad.v[3])) {
            return i;
        }
        else if ((v[0] == quad.v[1]) && (v[1] == quad.v[2]) &&
                 (v[2] == quad.v[3]) && (v[3] == quad.v[0])) {
            return i;
        }
        else if ((v[0] == quad.v[2]) && (v[1] == quad.v[3]) &&
                 (v[2] == quad.v[0]) && (v[3] == quad.v[1])) {
            return i;
        }
        else if ((v[0] == quad.v[3]) && (v[1] == quad.v[0]) &&
                 (v[2] == quad.v[1]) && (v[3] == quad.v[2])) {
            return i;
        }
        else if ((v[0] == quad.v[3]) && (v[1] == quad.v[2]) &&
                 (v[2] == quad.v[1]) && (v[3] == quad.v[0])) {
            return i;
        }
        else if ((v[0] == quad.v[0]) && (v[1] == quad.v[3]) &&
                 (v[2] == quad.v[2]) && (v[3] == quad.v[1])) {
            return i;
        }
        else if ((v[0] == quad.v[1]) && (v[1] == quad.v[0]) &&
                 (v[2] == quad.v[3]) && (v[3] == quad.v[2])) {
            return i;
        }
        else if ((v[0] == quad.v[2]) && (v[1] == quad.v[1]) &&
                 (v[2] == quad.v[0]) && (v[3] == quad.v[3])) {
            return i;
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------
// find a box edge by node numbers
int FEDBox::FindBoxEdge(int n0, int n1)
{
    // find the vertices from the node list
    int v0 = m_pDom->FindVertexByTagNumber(n0);
    int v1 = m_pDom->FindVertexByTagNumber(n1);
    
    for (int i=0; i<12; ++i) {
        if (((v0 == v[m_edge[i][0]]) && (v1 == v[m_edge[i][1]])) ||
            ((v0 == v[m_edge[i][1]]) && (v1 == v[m_edge[i][0]]))) {
            return i;
        }
    }
    return -1;
}


//-------------------------------------------------------------------------------
// set mesh parameters assuming faces 0,3,4 map identically to faces 2, 1, 5
// nseg[0] = number of elements along the edges normal to face 0
// nseg[1] = number of elements along the edges normal to face 3
// nseg[2] = number of elements along the edges normal to face 4
bool FEDBox::SetMeshFaces034(int nseg[3], double bias[3], bool dble[3])
{
    // list of edges normal to faces 0, 3 and 4
    const int enf[3][4] = {
        { 3, 1, 5, 7},
        { 2, 0, 4, 6},
        {11,10, 9, 8}
    };
    // edge normal direction is positive?
    const bool enfp[3][4] = {
        { false, true, true, false},
        { false, true, true, false},
        { true , true, true, true}
    };
    double b, bi;

    // for each face
    for (int i=0; i<3; ++i) {
        // for each edge normal to this face
        for (int j=0; j<4; ++j) {
            // check if edge normal direction is positive
            if (enfp[i][j]) {
                b = bias[i];
                bi = 1./b;
            }
            else {
                bi = bias[i];
                b = 1./bi;
            }
            // copy the edge
            int ie = e[enf[i][j]];

			FEDEdge& edge = m_pDom->Edge(ie);
			// check if this edge was previously meshed
			if (m_pDom->Edge(ie).nseg == -1) 
			{
				// set bias based on edge direction in this box
				edge.bias = ep[enf[i][j]] ? b : bi;
				edge.nseg = nseg[i];
				edge.dble = dble[i];
			}
			else
			{
				// else, if previously meshed, check consistency
				if ((m_pDom->Edge(ie).nseg != edge.nseg) ||
					(m_pDom->Edge(ie).bias != edge.bias) ||
					(m_pDom->Edge(ie).dble != edge.dble)) {
					assert(false);
					return false;
				}
			}
        }
    }
    
    return true;
}

//-------------------------------------------------------------------------------
bool FEDBox::SetMeshSingleFace(int face, int n, double b, bool d)
{
    int nseg[3];
    double bias[3];
    bool dble[3] = {false, false, false};
    if (face == 0) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = 1;
        bias[0] = b; bias[1] = 1; bias[2] = 1;
    }
    else if (face == 3) {
        nseg[0] = 1; nseg[1] = n; nseg[2] = 1;
        bias[0] = 1; bias[1] = b; bias[2] = 1;
    }
    else if (face == 4) {
        nseg[0] = 1; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1; bias[1] = 1; bias[2] = b;
    }
    else if (face == 2) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = 1;
        bias[0] = 1./b; bias[1] = 1; bias[2] = 1;
    }
    else if (face == 1) {
        nseg[0] = 1; nseg[1] = n; nseg[2] = 1;
        bias[0] = 1; bias[1] = 1./b; bias[2] = 1;
    }
    else if (face == 5) {
        nseg[0] = 1; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1; bias[1] = 1; bias[2] = 1./b;
    }
    return SetMeshFaces034(nseg, bias, dble);
}

//-------------------------------------------------------------------------------
bool FEDBox::CreateMesh(FSDomain* pdom)
{
    // create the box mesh by stacking quad meshes from box face 4 to face 5
    // assume that mesh bias is identical in (faces 4 and 5)
    // and in (edges 8,9,10,11)
    
    int mseg = pdom->Edge(pdom->Quad(q[4]).e[0]).nseg;   // number of segments along eta1
    int nseg = pdom->Edge(pdom->Quad(q[4]).e[1]).nseg;   // number of segments along eta2
    int pseg = pdom->Edge(e[8]).nseg;   // number of segments along eta3
    
    // allocate memory for node list
    n.resize(mseg+1);
    for (int i=0; i<= mseg; ++i) {
        n[i].resize(nseg+1);
        for (int j=0; j<= nseg; ++j) {
            n[i][j].resize(pseg+1);
        }
    }

    // allocate memory for stack list
    vector<int> qstack(pseg+1,-1);
    
    // we already have the first (face 4) and last (face 5) stack entries
    // and they have been meshed already
    qstack[0] = q[4];
    qstack[pseg] = q[5];
    int le[4];  // local edge numbering for q[4]
    if (qp[4])
        for (int i=0; i<4; ++i)
            le[i] = 11 - mod(qst[4] + i,4);
    else
        for (int i=0; i<4; ++i)
            le[i] = 11 - mod(qst[4] - i,4);
    
    // create the intermediate stacks
    int enl;
    bool epl;
    for (int k=1; k<pseg; ++k) {
        int l = pseg - k;
        // create a quad for this stack as a copy of face 4
        FEDQuad quad = FEDQuad(pdom->Quad(q[4]));
        // reset vertices of this quad
        quad.v[0] = ep[le[0]] ? pdom->Edge(e[le[0]]).n[k] : pdom->Edge(e[le[0]]).n[l];
        quad.v[1] = ep[le[1]] ? pdom->Edge(e[le[1]]).n[k] : pdom->Edge(e[le[1]]).n[l];
        quad.v[2] = ep[le[2]] ? pdom->Edge(e[le[2]]).n[k] : pdom->Edge(e[le[2]]).n[l];
        quad.v[3] = ep[le[3]] ? pdom->Edge(e[le[3]]).n[k] : pdom->Edge(e[le[3]]).n[l];
        // create edges for this quad
        FEDEdge edge;
        for (int i=0; i<4; ++i) {
            // create copy of edges of face 4 (includes meshing parameters)
            edge = FEDEdge(pdom->Edge(quad.e[i]));
            // substitute vertices of current stack
            if (pdom->Quad(q[4]).ep[i]) {
                edge.v[0] = quad.v[quad.m_edge[i][0]];
                edge.v[1] = quad.v[quad.m_edge[i][1]];
            }
            else {
                edge.v[0] = quad.v[quad.m_edge[i][1]];
                edge.v[1] = quad.v[quad.m_edge[i][0]];
            }
            // check if this edge exists
            enl = pdom->FindEdge(edge,epl);
            if (enl == -1) {
                // mesh this edge
                edge.CreateMesh(pdom);
                // add it to list of edges for this quad and add edge to domain
                quad.e[i] = pdom->AddEdge(edge);
            }
            else {
                quad.e[i] = enl;
            }
        }
        // create a mesh for this quad
        quad.CreateMesh(pdom);
        // add it to list of quads in stack and add this quad to the domain
        qstack[k] = pdom->AddQuad(quad);
    }
    
    // store vertices in node list
    for (int k=0; k< pseg; ++k) {
        // get a pointer to this stack entry
        const FEDQuad& quad = pdom->Quad(qstack[k]);
        // copy the node list
        for (int j=0; j<= nseg; ++j) {
            for (int i=0; i<= mseg; ++i) {
                n[i][j][k] = quad.n[i][j];
            }
        }
    }
    
    // last stack requires different sequence
    const FEDQuad& quad = pdom->Quad(qstack[pseg]);
    int m = (qst[4] + qst[5]) % 4;
    if (qp[4] && qp[5]) {
        for (int j=0; j<= nseg; ++j) {
            int l = nseg - j;
            for (int i=0; i<= mseg; ++i) {
                int k = mseg - i;
                switch (m) {
                    case 0: n[i][j][pseg] = quad.n[i][l]; break;
                    case 1: n[i][j][pseg] = quad.n[l][k]; break;
                    case 2: n[i][j][pseg] = quad.n[k][j]; break;
                    case 3: n[i][j][pseg] = quad.n[j][i]; break;
                    default: break;
                }
            }
        }
    }
    else if (qp[4] && !qp[5]) {
        for (int j=0; j<= nseg; ++j) {
            int l = nseg - j;
            for (int i=0; i<= mseg; ++i) {
                int k = mseg - i;
                switch (m) {
                    case 0: n[i][j][pseg] = quad.n[l][i]; break;
                    case 1: n[i][j][pseg] = quad.n[k][l]; break;
                    case 2: n[i][j][pseg] = quad.n[j][k]; break;
                    case 3: n[i][j][pseg] = quad.n[i][j]; break;
                    default: break;
                }
            }
        }
    }
    else if (!qp[4] && qp[5]) {
        for (int j=0; j<= nseg; ++j) {
            int l = nseg - j;
            for (int i=0; i<= mseg; ++i) {
                int k = mseg - i;
                switch (m) {
                    case 0: n[i][j][pseg] = quad.n[j][k]; break;
                    case 1: n[i][j][pseg] = quad.n[k][l]; break;
                    case 2: n[i][j][pseg] = quad.n[l][i]; break;
                    case 3: n[i][j][pseg] = quad.n[i][j]; break;
                    default: break;
                }
            }
        }
    }
    else if (!qp[4] && !qp[5]) {
        for (int j=0; j<= nseg; ++j) {
            int l = nseg - j;
            for (int i=0; i<= mseg; ++i) {
                int k = mseg - i;
                switch (m) {
                    case 0: n[i][j][pseg] = quad.n[k][j]; break;
                    case 1: n[i][j][pseg] = quad.n[l][k]; break;
                    case 2: n[i][j][pseg] = quad.n[i][l]; break;
                    case 3: n[i][j][pseg] = quad.n[j][i]; break;
                    default: break;
                }
            }
        }
    }
    else
        return false;

    // allocate size and generate element list
    elem.resize(mseg*nseg*pseg);
    int iel = 0;
    if (qp[4]) {
        for (int k=0; k< pseg; ++k) {
            for (int j=0; j< nseg; ++j) {
                for (int i=0; i< mseg; ++i) {
                    elem[iel].resize(8);
                    elem[iel][0] = n[i  ][j  ][k  ];
                    elem[iel][1] = n[i  ][j+1][k  ];
                    elem[iel][2] = n[i+1][j+1][k  ];
                    elem[iel][3] = n[i+1][j  ][k  ];
                    elem[iel][4] = n[i  ][j  ][k+1];
                    elem[iel][5] = n[i  ][j+1][k+1];
                    elem[iel][6] = n[i+1][j+1][k+1];
                    elem[iel][7] = n[i+1][j  ][k+1];
                    ++iel;
                }
            }
        }
    }
    else {
        for (int k=0; k< pseg; ++k) {
            for (int j=0; j< nseg; ++j) {
                for (int i=0; i< mseg; ++i) {
                    elem[iel].resize(8);
                    elem[iel][0] = n[i  ][j  ][k  ];
                    elem[iel][1] = n[i+1][j  ][k  ];
                    elem[iel][2] = n[i+1][j+1][k  ];
                    elem[iel][3] = n[i  ][j+1][k  ];
                    elem[iel][4] = n[i  ][j  ][k+1];
                    elem[iel][5] = n[i+1][j  ][k+1];
                    elem[iel][6] = n[i+1][j+1][k+1];
                    elem[iel][7] = n[i  ][j+1][k+1];
                    ++iel;
                }
            }
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
// FEDWedge
/////////////////////////////////////////////////////////////////////////////////

FEDWedge::FEDWedge()
{
    m_ntag = -1;
    m_gid = -1;
    m_fne = -1;
    for (int i=0; i< 6; ++i) v[i] = -1;
    for (int i=0; i<9; ++i) {
        e[i] = -1;
        ep[i] = true;
    }
    for (int i=0; i< 5; ++i) {
        f[i] = -1;
        fp[i] = true;
        fst[i] = -1;
    }
    m_pDom = 0;
}

//-------------------------------------------------------------------------------
// Find a wedge face by node numbers
int FEDWedge::FindWedgeFace(const FSFace& face)
{
	int v[4] = { 0 };
    // find the vertices from the node list
    for (int i=0; i<face.Nodes(); ++i) {
        v[i] = m_pDom->FindVertexByTagNumber(face.n[i]);
        if (v[i] == -1)
            return -1;
    }
    
    if (face.Nodes() == 4) {
        for (int i=0; i<3; ++i) {
            const FEDQuad& quad = m_pDom->Quad(f[i]);
            if ((v[0] == quad.v[0]) && (v[1] == quad.v[1]) &&
                (v[2] == quad.v[2]) && (v[3] == quad.v[3])) {
                return i;
            }
            else if ((v[0] == quad.v[1]) && (v[1] == quad.v[2]) &&
                     (v[2] == quad.v[3]) && (v[3] == quad.v[0])) {
                return i;
            }
            else if ((v[0] == quad.v[2]) && (v[1] == quad.v[3]) &&
                     (v[2] == quad.v[0]) && (v[3] == quad.v[1])) {
                return i;
            }
            else if ((v[0] == quad.v[3]) && (v[1] == quad.v[0]) &&
                     (v[2] == quad.v[1]) && (v[3] == quad.v[2])) {
                return i;
            }
            else if ((v[0] == quad.v[3]) && (v[1] == quad.v[2]) &&
                     (v[2] == quad.v[1]) && (v[3] == quad.v[0])) {
                return i;
            }
            else if ((v[0] == quad.v[0]) && (v[1] == quad.v[3]) &&
                     (v[2] == quad.v[2]) && (v[3] == quad.v[1])) {
                return i;
            }
            else if ((v[0] == quad.v[1]) && (v[1] == quad.v[0]) &&
                     (v[2] == quad.v[3]) && (v[3] == quad.v[2])) {
                return i;
            }
            else if ((v[0] == quad.v[2]) && (v[1] == quad.v[1]) &&
                     (v[2] == quad.v[0]) && (v[3] == quad.v[3])) {
                return i;
            }
        }
    }
    else if (face.Nodes() == 3) {
        for (int i=3; i<5; ++i) {
            const FEDTri& tri = m_pDom->Tri(f[i]);
            if ((v[0] == tri.v[0]) && (v[1] == tri.v[1]) &&
                (v[2] == tri.v[2])) {
                return i;
            }
            else if ((v[0] == tri.v[1]) && (v[1] == tri.v[2]) &&
                     (v[2] == tri.v[0])) {
                return i;
            }
            else if ((v[0] == tri.v[2]) &&
                     (v[1] == tri.v[0]) && (v[2] == tri.v[1])) {
                return i;
            }
            else if ((v[0] == tri.v[2]) && (v[1] == tri.v[1]) &&
                     (v[2] == tri.v[0])) {
                return i;
            }
            else if ((v[0] == tri.v[0]) && (v[1] == tri.v[2]) &&
                     (v[2] == tri.v[1])) {
                return i;
            }
            else if ((v[0] == tri.v[1]) && (v[1] == tri.v[0]) &&
                     (v[2] == tri.v[2])) {
                return i;
            }
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------
// find a wedge edge by node numbers
int FEDWedge::FindWedgeEdge(int n0, int n1)
{
    int v0 = m_pDom->FindVertexByTagNumber(n0);
    int v1 = m_pDom->FindVertexByTagNumber(n1);
    if ((v0 == -1) || (v1 == -1)) return -1;

    for (int i=0; i<9; ++i)
        if (((v[m_edge[i][0]] == v0) && (v[m_edge[i][1]] == v1)) ||
            ((v[m_edge[i][1]] == v0) && (v[m_edge[i][0]] == v1)))
            return i;

    return -1;
}

//-------------------------------------------------------------------------------
// find a wedge vertex by node number
int FEDWedge::FindWedgeVertex(int n0)
{
    int v0 = m_pDom->FindVertexByTagNumber(n0);
    if (v0 == -1) return -1;
    
    for (int i=0; i<6; ++i)
        if (v[i] == v0) return i;
    
    return -1;
}

//-------------------------------------------------------------------------------
// set mesh parameters assuming two quad faces map identically,
// and the two tri faces map identically
// nseg[0] = mseg = number of elements on edges emanating from iedge
// nseg[1] = nseg = number of elements along complementary triangle edges
// nseg[2] = pseg = number of elements between triangle faces
bool FEDWedge::SetMesh(int iedge, int nseg[3], double bias[3], bool dble[3])
{
    m_fne = iedge;

    // edges enf[i] are meshed identically with nseg[i], bias[i], dble[i]
    vector< vector<int> > enf;
    vector< vector<bool> > enp;
    enf.resize(3);
    enf[0].resize(4); enf[1].resize(2); enf[2].resize(3);
    enp.resize(3);
    enp[0].resize(4,true); enp[1].resize(2,true); enp[2].resize(3,true);
    
    if (iedge == 6) {
        // face 0 maps to 2 (mseg), face 1 (nseg), face 3 maps to 4 (pseg)
        enf[0][0] = 0; enf[0][1] = 2; enf[0][2] = 3; enf[0][3] = 5;
        enf[1][0] = 1; enf[1][1] = 4;
        enf[2][0] = 6; enf[2][1] = 7; enf[2][2] = 8;
    }
    else if (iedge == 7) {
        // face 1 maps to 0 (mseg), face 2 (nseg), face 3 maps to 4 (pseg)
        enf[0][0] = 1; enf[0][1] = 0; enf[0][2] = 4; enf[0][3] = 3;
        enf[1][0] = 2; enf[1][1] = 5;
        enf[2][0] = 6; enf[2][1] = 7; enf[2][2] = 8;
    }
    else if (iedge == 8) {
        // face 2 maps to 1 (mseg), face 0 (nseg), face 3 maps to 4 (pseg)
        enf[0][0] = 2; enf[0][1] = 1; enf[0][2] = 5; enf[0][3] = 4;
        enf[1][0] = 0; enf[1][1] = 3;
        enf[2][0] = 6; enf[2][1] = 7; enf[2][2] = 8;
    }
    else
        return false;
    // common setting for all cases
    enp[0][1] = false; enp[0][3] = false;
    
    // set apex of triangle faces of wedge
    int vtx0, vtx1;
    if (ep[iedge]) {
        vtx0 = m_pDom->Edge(e[iedge]).v[0];
        vtx1 = m_pDom->Edge(e[iedge]).v[1];
    }
    else {
        vtx1 = m_pDom->Edge(e[iedge]).v[0];
        vtx0 = m_pDom->Edge(e[iedge]).v[1];
    }
    FEDTri& tri0 = m_pDom->Tri(f[3]); tri0.m_fne = tri0.FindVertex(vtx0);
    FEDTri& tri1 = m_pDom->Tri(f[4]); tri1.m_fne = tri1.FindVertex(vtx1);
    
    double b, bi;
    FEDEdge edge;
    
    // for each face
    for (int i=0; i<3; ++i) {
        // for each edge in this face
        for (int j=0; j<enf[i].size(); ++j) {
            // check if edge direction is positive
            if (enp[i][j]) {
                b = bias[i];
                bi = 1./b;
            }
            else {
                bi = bias[i];
                b = 1./bi;
            }
            // copy the edge
            int ie = e[enf[i][j]];
            edge = FEDEdge(m_pDom->Edge(ie));
            // set bias based on edge direction in this wedge
            edge.bias = ep[enf[i][j]] ? b : bi;
            edge.nseg = nseg[i];
            edge.dble = dble[i];
            // check if this edge was previously meshed
            if (m_pDom->Edge(ie).nseg == -1) {
                m_pDom->Edge(ie) = edge;
            }
            // if previously meshed, check consistency
            else if ((m_pDom->EdgeList[ie].nseg != edge.nseg) ||
                     (m_pDom->EdgeList[ie].bias != edge.bias) ||
                     (m_pDom->EdgeList[ie].dble != edge.dble)) {
                return false;
            }
        }
    }
    
    return true;
}

//-------------------------------------------------------------------------------
bool FEDWedge::SetMeshSingleFace(int face, int n, double b, bool d)
{
    int nseg[3];
    double bias[3];
    bool dble[3] = {false, false, false};
    int iedge = 6;
    switch (face) {
        case 0:
            nseg[0] = n; nseg[1] = 1; nseg[2] = 1;
            bias[0] = 1./b; bias[1] = 1; bias[2] = 1;
            iedge = 8;
            break;
            
        case 1:
            nseg[0] = n; nseg[1] = 1; nseg[2] = 1;
            bias[0] = 1./b; bias[1] = 1; bias[2] = 1;
            iedge = 6;
            break;
            
        case 2:
            nseg[0] = n; nseg[1] = 1; nseg[2] = 1;
            bias[0] = 1./b; bias[1] = 1; bias[2] = 1;
            iedge = 7;
            break;
            
        case 3:
            nseg[0] = 1; nseg[1] = 1; nseg[2] = n;
            bias[0] = 1; bias[1] = 1; bias[2] = b;
            break;
            
        case 4:
            nseg[0] = 1; nseg[1] = 1; nseg[2] = n;
            bias[0] = 1; bias[1] = 1; bias[2] = 1./b;
            break;
            
        default:
            return false;
            break;
    }

    return SetMesh(iedge, nseg, bias, dble);
}

//-------------------------------------------------------------------------------
bool FEDWedge::SetMeshSingleEdge(int edge, int n, double b, bool d)
{
    int nseg[3];
    double bias[3];
    bool dble[3] = {false, false, false};
    int iedge = -1;
    
    if (edge == 1) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1./b; bias[1] = 1; bias[2] = b;
        iedge = 6;
    }
    else if (edge == 4) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1./b; bias[1] = 1; bias[2] = 1./b;
        iedge = 6;
    }
    else if (edge == 2) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1./b; bias[1] = 1; bias[2] = b;
        iedge = 7;
    }
    else if (edge == 5) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1./b; bias[1] = 1; bias[2] = 1./b;
        iedge = 7;
    }
    else if (edge == 0) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1./b; bias[1] = 1; bias[2] = b;
        iedge = 8;
    }
    else if (edge == 3) {
        nseg[0] = n; nseg[1] = 1; nseg[2] = n;
        bias[0] = 1./b; bias[1] = 1; bias[2] = 1./b;
        iedge = 8;
    }
    else {
        nseg[0] = n; nseg[1] = 1; nseg[2] = 1;
        bias[0] = b; bias[1] = 1; bias[2] = 1;
        iedge = edge;
    }

    return SetMesh(iedge, nseg, bias, dble);
}

//-------------------------------------------------------------------------------
bool FEDWedge::CreateMesh(FSDomain* pdom)
{
    // create the wedge mesh by stacking tri meshes from wedge face 3 to face 4
    // assume that mesh bias is identical in (faces 3 and 4)
    // and in (edges 6,7,8)
    
    int mseg = -1, nseg = -1, pseg;
    int ifst = pdom->Tri(f[3]).m_fne;

    if (ifst == -1)  return false;
    mseg = pdom->Edge(pdom->Tri(f[3]).e[ifst]).nseg;   // number of segments along eta1
    nseg = pdom->Edge(pdom->Tri(f[3]).e[(ifst+1) % 3]).nseg;   // number of segments along eta2
    pseg = pdom->Edge(e[6]).nseg;   // number of segments along eta3

    assert((mseg == pdom->Tri(f[3]).n.size() - 1) &&
           (nseg == pdom->Tri(f[3]).n[0].size() - 1));

    // check compatibility of starting vertices of triangular faces
    int m = (fst[3] + fst[4]) % 3;
    if (m != 0) return false;

    // allocate memory for node list
    n.resize(mseg+1);
    for (int i=0; i<= mseg; ++i) {
        n[i].resize(nseg+1);
        for (int j=0; j<= nseg; ++j) {
            n[i][j].resize(pseg+1);
        }
    }
    
    // allocate memory for stack list
    vector<int> fstack(pseg+1,-1);
    
    // we already have the first (face 3) and last (face 4) stack entries
    // and they have been meshed already
    fstack[0] = f[3];
    fstack[pseg] = f[4];
    int le[3];  // local edge numbering for f[3]
    if (fp[3]) {
        for (int i=0; i<3; ++i)
            le[i] = 8 - mod(fst[3] + i + 2,3);
    }
    else
        for (int i=0; i<3; ++i)
            le[i] = 8 - mod(fst[3] - i + 2,3);
    
    // create the intermediate stacks
    int enl;
    bool epl;
    for (int k=1; k<pseg; ++k) {
        int l = pseg - k;
        // create a tri for this stack as a copy of face 3
        FEDTri tri = FEDTri(pdom->Tri(f[3]));
        // reset vertices of this tri
        tri.v[0] = ep[le[0]] ? pdom->Edge(e[le[0]]).n[k] : pdom->Edge(e[le[0]]).n[l];
        tri.v[1] = ep[le[1]] ? pdom->Edge(e[le[1]]).n[k] : pdom->Edge(e[le[1]]).n[l];
        tri.v[2] = ep[le[2]] ? pdom->Edge(e[le[2]]).n[k] : pdom->Edge(e[le[2]]).n[l];
        // create edges for this tri
        FEDEdge edge;
        for (int i=0; i<3; ++i) {
            // create copy of edges of face 3 (includes meshing parameters)
            edge = FEDEdge(pdom->Edge(tri.e[i]));
            // substitute vertices of current stack
            if (tri.ep[i]) {
                edge.v[0] = tri.v[tri.m_edge[i][0]];
                edge.v[1] = tri.v[tri.m_edge[i][1]];
            }
            else {
                edge.v[0] = tri.v[tri.m_edge[i][1]];
                edge.v[1] = tri.v[tri.m_edge[i][0]];
            }
            // check if this edge exists
            enl = pdom->FindEdge(edge,epl);
            if (enl == -1) {
                // mesh this edge
                edge.CreateMesh(pdom);
                // add it to list of edges for this quad and add edge to domain
                tri.e[i] = pdom->AddEdge(edge);
            }
            else {
                tri.e[i] = enl;
            }
        }
        // create a mesh for this tri
        tri.CreateMesh(pdom);
        // add it to list of tris in stack and add this tri to the domain
        fstack[k] = pdom->AddTri(tri);
    }
    
    // store vertices in node list
    for (int k=0; k< pseg; ++k) {
        // get a pointer to this stack entry
        const FEDTri& tri = pdom->Tri(fstack[k]);
        // copy the node list
        for (int j=0; j<= nseg; ++j) {
            for (int i=0; i<= mseg; ++i) {
                n[i][j][k] = tri.n[i][j];
            }
        }
    }
    
    // last stack requires different sequence
    const FEDTri& tri = pdom->Tri(fstack[pseg]);
    // check compatibility
    if (fp[3] == fp[4]) {
        for (int j=0; j<= nseg; ++j) {
            int l = nseg - j;
            for (int i=0; i<= mseg; ++i) {
                n[i][j][pseg] = tri.n[i][l];
            }
        }
    }
    else {
        for (int j=0; j<= nseg; ++j) {
            for (int i=0; i<= mseg; ++i) {
                n[i][j][pseg] = tri.n[i][j];
            }
        }
    }
    
    // allocate size and generate element list
    elem.resize(mseg*nseg*pseg);
    int iel = 0;
    if (fp[3]) {
        for (int k=0; k< pseg; ++k) {
            for (int j=0; j< nseg; ++j) {
                // first one is a penta6
                {
                    int i = 0;
                    elem[iel].resize(6);
                    elem[iel][0] = n[i  ][j  ][k  ];
                    elem[iel][1] = n[i+1][j+1][k  ];
                    elem[iel][2] = n[i+1][j  ][k  ];
                    elem[iel][3] = n[i  ][j  ][k+1];
                    elem[iel][4] = n[i+1][j+1][k+1];
                    elem[iel][5] = n[i+1][j  ][k+1];
                    ++iel;
                }
                // rest are hex8
                for (int i=1; i< mseg; ++i) {
                    elem[iel].resize(8);
                    elem[iel][0] = n[i  ][j  ][k  ];
                    elem[iel][1] = n[i  ][j+1][k  ];
                    elem[iel][2] = n[i+1][j+1][k  ];
                    elem[iel][3] = n[i+1][j  ][k  ];
                    elem[iel][4] = n[i  ][j  ][k+1];
                    elem[iel][5] = n[i  ][j+1][k+1];
                    elem[iel][6] = n[i+1][j+1][k+1];
                    elem[iel][7] = n[i+1][j  ][k+1];
                    ++iel;
                }
            }
        }
    }
    else {
        for (int k=0; k< pseg; ++k) {
            for (int j=0; j< nseg; ++j) {
                // first one is a penta6
                {
                    int i = 0;
                    elem[iel].resize(6);
                    elem[iel][0] = n[i  ][j  ][k  ];
                    elem[iel][1] = n[i+1][j  ][k  ];
                    elem[iel][2] = n[i+1][j+1][k  ];
                    elem[iel][3] = n[i  ][j  ][k+1];
                    elem[iel][4] = n[i+1][j  ][k+1];
                    elem[iel][5] = n[i+1][j+1][k+1];
                    ++iel;
                }
                // rest are hex8
                for (int i=1; i< mseg; ++i) {
                    elem[iel].resize(8);
                    elem[iel][0] = n[i  ][j  ][k  ];
                    elem[iel][1] = n[i+1][j  ][k  ];
                    elem[iel][2] = n[i+1][j+1][k  ];
                    elem[iel][3] = n[i  ][j+1][k  ];
                    elem[iel][4] = n[i  ][j  ][k+1];
                    elem[iel][5] = n[i+1][j  ][k+1];
                    elem[iel][6] = n[i+1][j+1][k+1];
                    elem[iel][7] = n[i  ][j+1][k+1];
                    ++iel;
                }
            }
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
// FEDTet
/////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------
// tet data arrays

// list of edges emanating from each of four faces
const int tet_fe[4][3] = {
    {2, 1, 5},
    {0, 2, 3},
    {0, 4, 1},
    {3, 5, 4}
};
const bool tet_fep[4][3] = {
    { false, true, false },
    { false, true, false },
    { true, false, false },
    { true, true, true }
};
// list of remaining edges
const int tet_fer[4][3] = {
    {0, 4, 3},
    {1, 5, 4},
    {3, 5, 2},
    {2, 1, 0}
};

// list of edges emanating from each of four nodes
const int tet_ne[4][3] = {
    {0, 3, 2},
    {1, 4, 0},
    {2, 5, 1},
    {3, 4, 5}
};
const bool tet_nep[4][3] = {
    { true, true, false },
    { true, true, false },
    { true, true, false },
    { false, false, false }
};
// list of remaining edges
const int tet_ner[4][3] = {
    {1, 4, 5},
    {2, 3, 5},
    {0, 3, 4},
    {0, 1, 2}
};
// faces opposing nodes
const int tet_fon[4] = { 1, 2, 0, 3};
// nodes opposing faces
const int tet_nof[4] = { 2, 0, 1, 3};

// list of edges emanating from each of six edges
const int tet_ee[6][4] = {
    {2, 1, 4, 3},
    {0, 2, 5, 4},
    {1, 0, 3, 5},
    {0, 4, 5, 2},
    {1, 5, 3, 0},
    {2, 3, 4, 1}
};
const bool tet_eep[6][4] = {
    { false, true, true, true },
    { false, true, true, true },
    { false, true, true, true },
    { true, false, false, false },
    { true, false, false, false },
    { true, false, false, false }
};
const int tet_eer[6][2] = {
    {0, 5},
    {1, 3},
    {2, 4},
    {3, 1},
    {4, 2},
    {5, 0}
};
// edges opposing edges
const int tet_eop[6] = { 5, 3, 4, 1, 2, 0 };

// faces connected to each node
const int tet_nfc[4][3] = {
    { 0, 2, 3 },
    { 1, 3, 0 },
    { 2, 3, 1 },
    { 0, 1, 2 }
};

// faces connected to each face
const int tet_ffc[4][3] = {
    { 1, 2, 3},
    { 2, 3, 0},
    { 3, 0, 1},
    { 0, 1, 2}
};

// faces connected to each edge in specific order
const int tet_efc[6][4] = {
    { 3, 0, 2, 1 },
    { 3, 1, 0, 2 },
    { 3, 2, 1, 0 },
    { 0, 2, 3, 1 },
    { 1, 0, 3, 2 },
    { 2, 1, 3, 0 }
};
// vertices of tet which form apex for meshing thoses faces
const int tet_efa[6][4] = {
    { 2, 3, 0, 1 },
    { 0, 3, 1, 2 },
    { 1, 3, 2, 0 },
    { 1, 2, 0, 3 },
    { 2, 0, 1, 3 },
    { 0, 1, 2, 3 }
};

//-------------------------------------------------------------------------------
FEDTet::FEDTet()
{
    m_ntag = -1;
    m_type = -1;
    m_fne = -1;
    m_gid = -1;
    for (int i=0; i< 4; ++i) v[i] = -1;
    for (int i=0; i<6; ++i) {
        e[i] = -1;
        ep[i] = true;
    }
    for (int i=0; i< 4; ++i) {
        f[i] = -1;
        fp[i] = true;
        fst[i] = -1;
    }
    m_pDom = 0;
}

//-------------------------------------------------------------------------------
// Find a tet face by node numbers
int FEDTet::FindTetFace(const FSFace& face)
{
    if (face.Nodes() != 3) return-1;
	int v[3] = { 0 };
    // find the vertices from the node list
    for (int i=0; i<face.Nodes(); ++i) {
        v[i] = m_pDom->FindVertexByTagNumber(face.n[i]);
        if (v[i] == -1)
            return -1;
    }
    
    for (int i=0; i<4; ++i) {
        const FEDTri& tri = m_pDom->Tri(f[i]);
        if ((v[0] == tri.v[0]) && (v[1] == tri.v[1]) &&
            (v[2] == tri.v[2])) {
            return i;
        }
        else if ((v[0] == tri.v[1]) && (v[1] == tri.v[2]) &&
                 (v[2] == tri.v[0])) {
            return i;
        }
        else if ((v[0] == tri.v[2]) &&
                 (v[1] == tri.v[0]) && (v[2] == tri.v[1])) {
            return i;
        }
        else if ((v[0] == tri.v[2]) && (v[1] == tri.v[1]) &&
                 (v[2] == tri.v[0])) {
            return i;
        }
        else if ((v[0] == tri.v[0]) && (v[1] == tri.v[2]) &&
                 (v[2] == tri.v[1])) {
            return i;
        }
        else if ((v[0] == tri.v[1]) && (v[1] == tri.v[0]) &&
                 (v[2] == tri.v[2])) {
            return i;
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------
// find a tet edge by node numbers
int FEDTet::FindTetEdge(int n0, int n1)
{
    int v0 = m_pDom->FindVertexByTagNumber(n0);
    int v1 = m_pDom->FindVertexByTagNumber(n1);
    if ((v0 == -1) || (v1 == -1)) return -1;
    
    for (int i=0; i<6; ++i)
        if (((v[m_edge[i][0]] == v0) && (v[m_edge[i][1]] == v1)) ||
            ((v[m_edge[i][1]] == v0) && (v[m_edge[i][0]] == v1)))
            return i;
    
    return -1;
}

//-------------------------------------------------------------------------------
// find the opposite tet edge by node numbers
int FEDTet::FindTetOppositeEdge(int n0, int n1)
{
    int iedge = FindTetEdge(n0, n1);
    if (iedge != -1) return tet_eop[iedge];
    
    return -1;
}

//-------------------------------------------------------------------------------
// find a tet vertex by node number
int FEDTet::FindTetVertex(int n0)
{
    int v0 = m_pDom->FindVertexByTagNumber(n0);
    if (v0 == -1) return -1;
    
    for (int i=0; i<4; ++i)
        if (v[i] == v0)
            return i;
    
    return -1;
}

//-------------------------------------------------------------------------------
// set mesh parameters along edges emanating out of a face
bool FEDTet::SetMeshFromFace(int iface, int nseg, double bias, bool dble)
{
    m_type = 0;
    m_fne = iface;

    // set m_fne for each triangle face of this tet
    // to the vertex opposite iface
    int vtx = v[tet_nof[iface]];
    for (int i=0; i<3; ++i) {
        FEDTri& tri = m_pDom->Tri(f[tet_ffc[iface][i]]);
        tri.m_fne = tri.FindVertex(vtx);
    }

    double b, bi;
    FEDEdge edge;
    
    // for each edge emanating from this face
    for (int j=0; j<3; ++j) {
        // check if edge direction is positive
        if (tet_fep[iface][j]) {
            b = bias;
            bi = 1./b;
        }
        else {
            bi = bias;
            b = 1./bi;
        }
        // copy the edge
        int ie = e[tet_fe[iface][j]];
        edge = FEDEdge(m_pDom->Edge(ie));
        // set bias based on edge direction in this wedge
        edge.bias = ep[tet_fe[iface][j]] ? b : bi;
        edge.nseg = nseg;
        edge.dble = dble;
        // check if this edge was previously meshed
        if (m_pDom->Edge(ie).nseg == -1) {
            m_pDom->Edge(ie) = edge;
        }
        // if previously meshed, check consistency
        else if ((m_pDom->EdgeList[ie].nseg != edge.nseg) ||
                 (m_pDom->EdgeList[ie].bias != edge.bias) ||
                 (m_pDom->EdgeList[ie].dble != edge.dble)) {
            return false;
        }
    }

    // for remaining edges
    for (int j=0; j<3; ++j) {
        // copy the edge
        int ie = e[tet_fer[iface][j]];
        edge = FEDEdge(m_pDom->Edge(ie));
        // set meshing parameters
        edge.bias = 1.0;
        edge.nseg = 1;
        edge.dble = false;
        // check if this edge was previously meshed
        if (m_pDom->Edge(ie).nseg == -1) {
            m_pDom->Edge(ie) = edge;
        }
        // if previously meshed, check consistency
        else if ((m_pDom->EdgeList[ie].nseg != edge.nseg) ||
                 (m_pDom->EdgeList[ie].bias != edge.bias) ||
                 (m_pDom->EdgeList[ie].dble != edge.dble)) {
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------------
// set mesh parameters along edges emanating from a vertex
bool FEDTet::SetMeshFromVertex(int ivtx, int nseg, double bias, bool dble)
{
    m_type = 1;
    m_fne = ivtx;
    
    // set m_fne for each triangle face of this tet connected to ivtx
    // to the vertex ivtx
    int vtx = v[ivtx];
    for (int i=0; i<3; ++i) {
        FEDTri& tri = m_pDom->Tri(f[tet_nfc[ivtx][i]]);
        tri.m_fne = tri.FindVertex(vtx);
    }
    
    double b, bi;
    FEDEdge edge;
    
    // for each edge emanating from this node
    for (int j=0; j<3; ++j) {
        // check if edge direction is positive
        if (tet_nep[ivtx][j]) {
            b = bias;
            bi = 1./b;
        }
        else {
            bi = bias;
            b = 1./bi;
        }
        // copy the edge
        int ie = e[tet_ne[ivtx][j]];
        edge = FEDEdge(m_pDom->Edge(ie));
        // set bias based on edge direction in this wedge
        edge.bias = ep[tet_ne[ivtx][j]] ? b : bi;
        edge.nseg = nseg;
        edge.dble = dble;
        // check if this edge was previously meshed
        if (m_pDom->Edge(ie).nseg == -1) {
            m_pDom->Edge(ie) = edge;
        }
        // if previously meshed, check consistency
        else if ((m_pDom->EdgeList[ie].nseg != edge.nseg) ||
                 (m_pDom->EdgeList[ie].bias != edge.bias) ||
                 (m_pDom->EdgeList[ie].dble != edge.dble)) {
            return false;
        }
    }
    
    // for remaining edges
    for (int j=0; j<3; ++j) {
        // copy the edge
        int ie = e[tet_ner[ivtx][j]];
        edge = FEDEdge(m_pDom->Edge(ie));
        // set meshing parameters
        edge.bias = 1.0;
        edge.nseg = 1;
        edge.dble = false;
        // check if this edge was previously meshed
        if (m_pDom->Edge(ie).nseg == -1) {
            m_pDom->Edge(ie) = edge;
        }
        // if previously meshed, check consistency
        else if ((m_pDom->EdgeList[ie].nseg != edge.nseg) ||
                 (m_pDom->EdgeList[ie].bias != edge.bias) ||
                 (m_pDom->EdgeList[ie].dble != edge.dble)) {
            return false;
        }
    }
    
    return true;
}

//-------------------------------------------------------------------------------
// set mesh parameters along edges emanating out of an edge
bool FEDTet::SetMeshFromEdge(int iedge, int nseg, double bias, bool dble)
{
    m_type = 2;
    m_fne = iedge;
    
    // set m_fne for each triangle face of this tet connected to iedge
    for (int i=0; i<4; ++i) {
        int vtx = v[tet_efa[iedge][i]];
        FEDTri& tri = m_pDom->Tri(f[tet_efc[iedge][i]]);
        tri.m_fne = tri.FindVertex(vtx);
    }

    double b, bi;
    FEDEdge edge;
    
    // for each edge emanating from this edge
    for (int j=0; j<4; ++j) {
        // check if edge direction is positive
        if (tet_eep[iedge][j]) {
            b = bias;
            bi = 1./b;
        }
        else {
            bi = bias;
            b = 1./bi;
        }
        // copy the edge
        int ie = e[tet_ee[iedge][j]];
        edge = FEDEdge(m_pDom->Edge(ie));
        // set bias based on edge direction in this wedge
        edge.bias = ep[tet_ee[iedge][j]] ? b : bi;
        edge.nseg = nseg;
        edge.dble = dble;
        // check if this edge was previously meshed
        if (m_pDom->Edge(ie).nseg == -1) {
            m_pDom->Edge(ie) = edge;
        }
        // if previously meshed, check consistency
        else if ((m_pDom->EdgeList[ie].nseg != edge.nseg) ||
                 (m_pDom->EdgeList[ie].bias != edge.bias) ||
                 (m_pDom->EdgeList[ie].dble != edge.dble)) {
            return false;
        }
    }
    
    // for remaining edges
    for (int j=0; j<2; ++j) {
        // copy the edge
        int ie = e[tet_eer[iedge][j]];
        edge = FEDEdge(m_pDom->Edge(ie));
        // set meshing parameters
        edge.bias = 1.0;
        edge.nseg = 1;
        edge.dble = false;
        // check if this edge was previously meshed
        if (m_pDom->Edge(ie).nseg == -1) {
            m_pDom->Edge(ie) = edge;
        }
        // if previously meshed, check consistency
        else if ((m_pDom->EdgeList[ie].nseg != edge.nseg) ||
                 (m_pDom->EdgeList[ie].bias != edge.bias) ||
                 (m_pDom->EdgeList[ie].dble != edge.dble)) {
            return false;
        }
    }
    
    return true;
}

//-------------------------------------------------------------------------------
bool FEDTet::CreateMesh(FSDomain* pdom)
{
    switch (m_type) {
        case 0:
        {
            // elements stacked from a face
            int iface = m_fne;
            FEDEdge edg0, edg1, edg2;
            bool ep0, ep1, ep2;
            if (fp[iface]) {
                edg0 = pdom->Edge(e[tet_fe[iface][0]]);
                edg1 = pdom->Edge(e[tet_fe[iface][1]]);
                edg2 = pdom->Edge(e[tet_fe[iface][2]]);
                ep0 = (ep[tet_fe[iface][0]] == tet_fep[iface][0]) ? true : false;
                ep1 = (ep[tet_fe[iface][1]] == tet_fep[iface][1]) ? true : false;
                ep2 = (ep[tet_fe[iface][2]] == tet_fep[iface][2]) ? true : false;
            }
            else {
                edg0 = pdom->Edge(e[tet_fe[iface][0]]);
                edg2 = pdom->Edge(e[tet_fe[iface][1]]);
                edg1 = pdom->Edge(e[tet_fe[iface][2]]);
                ep0 = (ep[tet_fe[iface][0]] == tet_fep[iface][0]) ? true : false;
                ep2 = (ep[tet_fe[iface][1]] == tet_fep[iface][1]) ? true : false;
                ep1 = (ep[tet_fe[iface][2]] == tet_fep[iface][2]) ? true : false;
            }
            int nseg = edg0.nseg;
            elem.resize(nseg);
            // create wedges
            for (int i=0; i<nseg-1; ++i) {
                elem[i].resize(6);
                int j = nseg - i;
                elem[i][0] = ep0 ? edg0.n[i] : edg0.n[j];
                elem[i][2] = ep1 ? edg1.n[i] : edg1.n[j];
                elem[i][1] = ep2 ? edg2.n[i] : edg2.n[j];
                elem[i][3] = ep0 ? edg0.n[i+1] : edg0.n[j-1];
                elem[i][5] = ep1 ? edg1.n[i+1] : edg1.n[j-1];
                elem[i][4] = ep2 ? edg2.n[i+1] : edg2.n[j-1];
            }
            // create final tet
            {
                int i = nseg-1;
                int j = 1;
                elem[i].resize(4);
                elem[i][0] = ep0 ? edg0.n[i] : edg0.n[j];
                elem[i][2] = ep1 ? edg1.n[i] : edg1.n[j];
                elem[i][1] = ep2 ? edg2.n[i] : edg2.n[j];
                elem[i][3] = ep0 ? edg0.n[i+1] : edg0.n[j-1];
            }
        }
            break;
            
        case 1:
        {
            // elements stacked from a node
            int inode = m_fne;
            FEDEdge edg0, edg1, edg2;
            bool ep0, ep1, ep2;
            if (fp[tet_fon[inode]]) {
                edg0 = pdom->Edge(e[tet_ne[inode][0]]);
                edg1 = pdom->Edge(e[tet_ne[inode][1]]);
                edg2 = pdom->Edge(e[tet_ne[inode][2]]);
                ep0 = (ep[tet_ne[inode][0]] == tet_nep[inode][0]) ? true: false;
                ep1 = (ep[tet_ne[inode][1]] == tet_nep[inode][1]) ? true: false;
                ep2 = (ep[tet_ne[inode][2]] == tet_nep[inode][2]) ? true: false;
            }
            else {
                edg0 = pdom->Edge(e[tet_ne[inode][0]]);
                edg2 = pdom->Edge(e[tet_ne[inode][1]]);
                edg1 = pdom->Edge(e[tet_ne[inode][2]]);
                ep0 = (ep[tet_ne[inode][0]] == tet_nep[inode][0]) ? true: false;
                ep2 = (ep[tet_ne[inode][1]] == tet_nep[inode][1]) ? true: false;
                ep1 = (ep[tet_ne[inode][2]] == tet_nep[inode][2]) ? true: false;
            }
            int nseg = edg0.nseg;
            elem.resize(nseg);
            // create first tet
            {
                int i = 0;
                int j = nseg;
                elem[i].resize(4);
                elem[i][0] = ep0 ? edg0.n[i+1] : edg0.n[j-1];
                elem[i][1] = ep1 ? edg1.n[i+1] : edg1.n[j-1];
                elem[i][2] = ep2 ? edg2.n[i+1] : edg2.n[j-1];
                elem[i][3] = ep0 ? edg0.n[i  ] : edg0.n[j  ];
            }
            // create wedges
            for (int i=1; i<nseg; ++i) {
                elem[i].resize(6);
                int j = nseg - i;
                elem[i][0] = ep0 ? edg0.n[i] : edg0.n[j];
                elem[i][2] = ep1 ? edg1.n[i] : edg1.n[j];
                elem[i][1] = ep2 ? edg2.n[i] : edg2.n[j];
                elem[i][3] = ep0 ? edg0.n[i+1] : edg0.n[j-1];
                elem[i][5] = ep1 ? edg1.n[i+1] : edg1.n[j-1];
                elem[i][4] = ep2 ? edg2.n[i+1] : edg2.n[j-1];
            }
        }
            break;
            
        case 2:
        {
            // elements stacked from an edge
            int iedge = m_fne;
            FEDEdge edg0, edg1, edg2, edg3;
            bool ep0, ep1, ep2, ep3;
            if (ep[iedge]) {
                edg0 = pdom->Edge(e[tet_ee[iedge][0]]);
                edg1 = pdom->Edge(e[tet_ee[iedge][1]]);
                edg2 = pdom->Edge(e[tet_ee[iedge][2]]);
                edg3 = pdom->Edge(e[tet_ee[iedge][3]]);
                ep0 = (ep[tet_ee[iedge][0]] == tet_eep[iedge][0]) ? true : false;
                ep1 = (ep[tet_ee[iedge][1]] == tet_eep[iedge][1]) ? true : false;
                ep2 = (ep[tet_ee[iedge][2]] == tet_eep[iedge][2]) ? true : false;
                ep3 = (ep[tet_ee[iedge][3]] == tet_eep[iedge][3]) ? true : false;
            }
            else {
                edg2 = pdom->Edge(e[tet_ee[iedge][0]]);
                edg3 = pdom->Edge(e[tet_ee[iedge][1]]);
                edg0 = pdom->Edge(e[tet_ee[iedge][2]]);
                edg1 = pdom->Edge(e[tet_ee[iedge][3]]);
                ep2 = (ep[tet_ee[iedge][0]] == tet_eep[iedge][0]) ? true : false;
                ep3 = (ep[tet_ee[iedge][1]] == tet_eep[iedge][1]) ? true : false;
                ep0 = (ep[tet_ee[iedge][2]] == tet_eep[iedge][2]) ? true : false;
                ep1 = (ep[tet_ee[iedge][3]] == tet_eep[iedge][3]) ? true : false;
            }
            int nseg = edg0.nseg;
            elem.resize(nseg);
            if (nseg == 1) {
                // create just one tet
                int i = 0;
                int j = nseg;
                elem[i].resize(4);
                elem[i][0] = ep0 ? edg0.n[i] : edg0.n[j];
                elem[i][1] = ep1 ? edg1.n[i] : edg1.n[j];
                elem[i][2] = ep1 ? edg1.n[i+1] : edg1.n[j-1];
                elem[i][3] = ep2 ? edg2.n[i+1] : edg2.n[j-1];
            }
            else {
                // create first wedge
                {
                    int i = 0;
                    int j = nseg;
                    elem[i].resize(6);
                    elem[i][0] = ep0 ? edg0.n[i  ] : edg0.n[j  ];
                    elem[i][1] = ep0 ? edg0.n[i+1] : edg0.n[j-1];
                    elem[i][2] = ep3 ? edg3.n[i+1] : edg3.n[j-1];
                    elem[i][3] = ep1 ? edg1.n[i  ] : edg1.n[j  ];
                    elem[i][4] = ep1 ? edg1.n[i+1] : edg1.n[j-1];
                    elem[i][5] = ep2 ? edg2.n[i+1] : edg2.n[j-1];
                }
                // create hexes
                for (int i=1; i<nseg-1; ++i) {
                    elem[i].resize(8);
                    int j = nseg - i;
                    elem[i][0] = ep0 ? edg0.n[i] : edg0.n[j];
                    elem[i][1] = ep3 ? edg3.n[i] : edg3.n[j];
                    elem[i][2] = ep2 ? edg2.n[i] : edg2.n[j];
                    elem[i][3] = ep1 ? edg1.n[i] : edg1.n[j];
                    elem[i][4] = ep0 ? edg0.n[i+1] : edg0.n[j-1];
                    elem[i][5] = ep3 ? edg3.n[i+1] : edg3.n[j-1];
                    elem[i][6] = ep2 ? edg2.n[i+1] : edg2.n[j-1];
                    elem[i][7] = ep1 ? edg1.n[i+1] : edg1.n[j-1];
                }
                // create final wedge
                {
                    int i = nseg-1;
                    int j = 1;
                    elem[i].resize(6);
                    elem[i][0] = ep0 ? edg0.n[i  ] : edg0.n[j  ];
                    elem[i][1] = ep1 ? edg1.n[i  ] : edg1.n[j  ];
                    elem[i][2] = ep1 ? edg1.n[i+1] : edg1.n[j-1];
                    elem[i][3] = ep3 ? edg3.n[i  ] : edg3.n[j  ];
                    elem[i][4] = ep2 ? edg2.n[i  ] : edg2.n[j  ];
                    elem[i][5] = ep2 ? edg2.n[i+1] : edg2.n[j-1];
                }
            }
        }
            break;
            
        default:
            break;
    }
    return true;
}
