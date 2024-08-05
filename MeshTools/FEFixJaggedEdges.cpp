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
#include "FEFixJaggedEdges.h"
#include <MeshLib/FESurfaceMesh.h>
#include <map>

// This routine fills triangles where there are reentrant corners along a boundary
// curve consisting of a chain of edges.  The user-specified "angle" determines the
// threshold of the deviation of the angle at the reentrant apex from 180 degrees
// (an angle of zero means that a nearly straight pair of consecutive edges could be
// filled with a triangle).

FEFixJaggedEdges::FEFixJaggedEdges() : FESurfaceModifier("Fix Jagged Edges")
{
	m_angle = 10;
	AddDoubleParam(m_angle, "angle", "angle (deg.)");
}

FSSurfaceMesh* FEFixJaggedEdges::Apply(FSSurfaceMesh* pm)
{
    // create a copy of this mesh
    FSSurfaceMesh* mesh = new FSSurfaceMesh(*pm);
    
    // convert angle from degrees to radians
    m_angle = GetFloatValue(0);
    m_angle *= PI/180;
    double sina = sin(m_angle);
    
    // get selected edges
    std::vector<FSEdge> edge;
    edge.reserve(mesh->Edges());
    
    for (int i=0; i<mesh->Edges(); ++i) {
        FSEdge& ei = mesh->Edge(i);
        if (ei.Nodes() > 2) {
            SetError("Can only fix 2-node edges.");
            return nullptr;
        }
        if (ei.IsSelected()) {
            // find the face to which this edge belongs
            for (int j=0; j<mesh->Faces(); ++j) {
                FSFace& face = mesh->Face(j);
                if (face.HasEdge(ei.n[0],ei.n[1])) {
                    if (ei.m_face[0] == -1) ei.m_face[0] = j;
                    else if (ei.m_face[0] == j) ei.m_face[0] = j;
                    else if (ei.m_face[0] > -1) ei.m_face[1] = j;
                }
            }
            // only include edges that belong to a single face (exclude interior edges)
            if ((ei.m_face[0] > -1) && (ei.m_face[1] == -1))
                edge.push_back(ei);
        }
    }
    
    // re-order edges consecutively
    int NE = edge.size();
    if (NE == 0) {
        SetError("No edges selected.");
        return nullptr;
    }
    // in case the edge chain is open, search for the two nodes that appear only once
    std::map<int,int> en;
    std::map<int,int>::iterator it;
    for (int i=0; i<NE; ++i) {
        for (int k=0; k<2; ++k) {
            it = en.find(edge[i].n[k]);
            if (it == en.end()) en.insert(it, std::pair<int,int>(edge[i].n[k],1));
            else it->second++;
        }
    }
    int first_node = -1;
    int last_node = -1;
    for (it=en.begin(); it!= en.end(); ++it) {
        if ((it->second == 1) && (first_node == -1)) first_node = it->first;
        else if ((it->second == 1) && (first_node > -1)) last_node = it->first;
        else if (it->second > 2) {
            SetError("Incorrect edge connectivity.");
            return nullptr;
        }
    }
    // find the edge that contains the first node and swap it with the first edge
    for (int i=1; i<NE; ++i) {
        if ((edge[i].n[0] == first_node) || (edge[i].n[1] == first_node)) {
            FSEdge etmp = edge[i];
            edge[i] = edge[0];
            edge[0] = etmp;
        }
    }
    
    // re-order the edges to have consecutive node numbers
    bool done = false;
    // by default assume that the first edge does not need to be flipped
    bool flip_first_edge = false;
    while (!done) {
        for (int i=0; i<NE-1; ++i) {
            // get end node number for edge i
            int root = edge[i].n[1];
            bool found = false;
            // find matching node in remaining edges
            for (int j=i+1; j<NE; ++j) {
                if (root == edge[j].n[0]) {
                    // if end node of edge i matches first node of edge j
                    // swap edge j with edge i+1
                    FSEdge etmp = edge[i+1];
                    edge[i+1] = edge[j];
                    edge[j] = etmp;
                    found = true;
                    break;
                }
                else if (root == edge[j].n[1]) {
                    // if end node of edge i matches end node of edge j
                    // swap first and end nodes of edge j
                    edge[j].n[1] = edge[j].n[0];
                    edge[j].n[0] = root;
                    // then swap edge j with edge i+1
                    FSEdge etmp = edge[i+1];
                    edge[i+1] = edge[j];
                    edge[j] = etmp;
                    found = true;
                    break;
                }
            }
            if (!found && !flip_first_edge) {
                // come here if no node matching root was found
                // flip first edge and restart
                int ntmp = edge[0].n[1];
                edge[0].n[1] = edge[0].n[0];
                edge[0].n[0] = ntmp;
                flip_first_edge = true;
                i=-1;
            }
            else if (!found && flip_first_edge) {
                // come here if no node matching root was found
                // and the first edge was already flipped once
                SetError("Edges have a gap.");
                return nullptr;
            }
            else {
                done = true;
            }
        }
    }
    
    // check edge chain for reentrant corners and fill with a triangle
    for (int i=0; i<NE-1; ++i) {
        FSNode& n0 = mesh->Node(edge[i].n[0]);
        FSNode& n1 = mesh->Node(edge[i].n[1]);
        FSNode& n2 = mesh->Node(edge[i+1].n[1]);
        // get vector connecting nodes of first edge
        vec3d v01 = n1.pos() - n0.pos(); v01.unit();
        // get vector connecting nodes of second edge
        vec3d v12 = n2.pos() - n1.pos(); v12.unit();
        // get node normal at n1
        FSFace& face = mesh->Face(edge[i].m_face[0]);
        vec3d vn1 = to_vec3d(face.m_nn[face.FindNode(edge[i].n[1])]);
        // get the angle between these vectors
        double sa = (v01 ^ v12)*vn1;
        double ca = v01*v12;
        double angle = atan2(sa,ca);
        // if corner is reentrant, create a triangular face
        if ((!flip_first_edge && (sa < -sina)) || (flip_first_edge && (sa > sina))) {
            // allocate room for the new face
            int NF = mesh->Faces();
            mesh->Create(0, 0, NF + 1);
            FSFace& face = mesh->Face(NF);
            face.SetType(FE_FACE_TRI3);
            face.n[0] = edge[i].n[0];
            if (!flip_first_edge) {
                face.n[2] = edge[i].n[1];
                face.n[1] = edge[i+1].n[1];
            }
            else {
                face.n[1] = edge[i].n[1];
                face.n[2] = edge[i+1].n[1];
            }
            face.m_gid = 0;
            // since the end node of edge i is the apex of the reentrant corner
            // skip the first node of edge i+1 when checking for next reentrant corners.
            ++i;
        }
    }
    
    // rebuild the mesh
    mesh->RebuildMesh();
    
    return mesh;
}
