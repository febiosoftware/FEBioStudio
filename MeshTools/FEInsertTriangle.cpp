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
#include "FEInsertTriangle.h"
#include <MeshLib/FSSurfaceMesh.h>
#include <map>

// This routine fills triangles where there are reentrant corners along a boundary
// curve consisting of a chain of edges.  The user-specified "angle" determines the
// threshold of the deviation of the angle at the reentrant apex from 180 degrees
// (an angle of zero means that a nearly straight pair of consecutive edges could be
// filled with a triangle).

FEInsertTriangle::FEInsertTriangle() : FESurfaceModifier("Insert Triangle")
{
    m_n = vec3d(0,0,1);
    AddVecParam(m_n,"normal", "approximate triangle normal");
}

FSSurfaceMesh* FEInsertTriangle::Apply(FSSurfaceMesh* pm)
{
    // create a copy of this mesh
    FSSurfaceMesh* mesh = new FSSurfaceMesh(*pm);
    
    // get approximate surface normal
    m_n = GetVecValue(0);
    
    // get selected nodes
    std::vector<FSNode> node;
    node.reserve(mesh->Nodes());
    std::vector<int> ID;
    ID.reserve(mesh->Nodes());

    for (int i=0; i<mesh->Nodes(); ++i) {
        FSNode& ni = mesh->Node(i);
        if (ni.IsSelected()) {
            node.push_back(ni);
            ID.push_back(i);
        }
    }
    if (node.size() != 3) {
        error("Three nodes should be selected.");
        return nullptr;
    }
    
    // create triangle from three selected nodes
    // allocate room for the new face
    int NF = mesh->Faces();
    mesh->Create(0, 0, NF + 1);
    FSFace& face = mesh->Face(NF);
    vec3d fn = (node[1].r - node[0].r) ^ (node[2].r - node[0].r);
    face.SetType(FE_FACE_TRI3);
    if (fn*m_n > 0) {
        face.n[0] = ID[0];
        face.n[1] = ID[1];
        face.n[2] = ID[2];
    } else {
        face.n[0] = ID[0];
        face.n[1] = ID[2];
        face.n[2] = ID[1];
    }
    face.m_gid = 0;
    
    // rebuild the mesh
    mesh->RebuildMesh();
    
    // unselect all the nodes
    for (int i=0; i< mesh->Nodes(); ++i) {
        FSNode& ni = mesh->Node(i);
        ni.Unselect();
    }
    
    // determine which nodes should remain selected
    NF = mesh->Faces();
    face = mesh->Face(NF-1);
    for (int i=0; i< face.Edges(); ++i) {
        FSEdge edge = face.GetEdge(i);
        // check how many faces in the mesh have this edge
        int count = 0;
        for (int j=0; j< mesh->Faces(); ++j) {
            FSFace& oface = mesh->Face(j);
            if (oface.HasEdge(edge.n[0], edge.n[1])) ++count;
        }
        if (count == 1) {
            FSNode& n0 = mesh->Node(edge.n[0]);
            n0.Select();
            FSNode& n1 = mesh->Node(edge.n[1]);
            n1.Select();
        }
    }
    
    return mesh;
}
