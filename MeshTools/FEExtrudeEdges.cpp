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
#include "FEExtrudeEdges.h"
#include <MeshLib/FESurfaceMesh.h>
#include <map>

// This routine fills triangles where there are reentrant corners along a boundary
// curve consisting of a chain of edges.  The user-specified "angle" determines the
// threshold of the deviation of the angle at the reentrant apex from 180 degrees
// (an angle of zero means that a nearly straight pair of consecutive edges could be
// filled with a triangle).

FEExtrudeEdges::FEExtrudeEdges() : FESurfaceModifier("Extrude Edges")
{
	m_d = 0;
    m_bias = 1.0;
    m_nseg = 1;
	AddDoubleParam(m_d, "distance", "Distance");
    AddIntParam(m_nseg, "segments", "Segments");
    AddDoubleParam(m_bias, "bias", "Mesh bias");
}

FSSurfaceMesh* FEExtrudeEdges::Apply(FSSurfaceMesh* pm)
{
    // create a copy of this mesh
    FSSurfaceMesh* mesh = new FSSurfaceMesh(*pm);
    
    // get user-specified parameters
    m_d = GetFloatValue(0);
    m_nseg = GetIntValue(1);
    m_bias = GetFloatValue(2);
    
    // get the array of biased coordinates
    std::vector<double> rbias(m_nseg+1);
    rbias[0] = 0;
    rbias[m_nseg] = 1;
    // uniform mesh
    if (m_bias == 1) {
        double dr = 1./m_nseg;
        for (int i=1; i<m_nseg; ++i) rbias[i] = dr*i;
    }
    else
    {
        double dr = (m_bias - 1)/(pow(m_bias, m_nseg) - 1);
        rbias[1] = dr;
        for (int i=2; i<m_nseg; ++i) rbias[i] = rbias[i-1] + dr*pow(m_bias, i-1);
    }

    // get selected edges
    std::vector<FSEdge> edge;
    edge.reserve(mesh->Edges());
    
    for (int i=0; i<mesh->Edges(); ++i) {
        FSEdge& ei = mesh->Edge(i);
        if (ei.Nodes() > 2) {
            SetError("Can only extrude 2-node edges.");
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
    
    // extract number of selected edges
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
    
    // reorder the edges to have consecutive node numbers
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
    
    // evaluate the extrusion direction at each edge node
    // from the cross-product of the tangent and normal
    int NN = mesh->Nodes();
    for (int i=0; i<NE; ++i) {
        vec3d vt(0,0,0);
        vec3d vn1(0,0,0);
        vec3d p(0,0,0);
        // process middle edges
        if ((i > 0) && (i<NE-1)) {
            FSNode& n0 = mesh->Node(edge[i-1].n[0]);
            FSNode& n1 = mesh->Node(edge[i-1].n[1]);
            FSNode& n2 = mesh->Node(edge[i].n[1]);
            p = n1.pos();
            // get vector connecting nodes of first edge
            vec3d v01 = n1.pos() - n0.pos(); v01.unit();
            // get vector connecting nodes of second edge
            vec3d v12 = n2.pos() - n1.pos(); v12.unit();
            // evaluate the node_tangent
            vt = v01 + v12; vt.unit();
            // get node normal at n1
            FSFace& face = mesh->Face(edge[i].m_face[0]);
            vn1 = to_vec3d(face.m_nn[face.FindNode(edge[i-1].n[1])]);
        }
        // process first edge
        else if (i == 0) {
            // get node normal at n0
            FSFace& face = mesh->Face(edge[0].m_face[0]);
            vn1 = to_vec3d(face.m_nn[face.FindNode(edge[0].n[0])]);
            // if curve is closed
            if (first_node == -1) {
                FSNode& n0 = mesh->Node(edge[NE-1].n[0]);
                FSNode& n1 = mesh->Node(edge[0].n[0]);
                FSNode& n2 = mesh->Node(edge[0].n[1]);
                p = n1.pos();
                // get vector connecting nodes of first edge
                vec3d v01 = n1.pos() - n0.pos(); v01.unit();
                // get vector connecting nodes of second edge
                vec3d v12 = n2.pos() - n1.pos(); v12.unit();
                // evaluate the node_tangent
                vt = v01 + v12; vt.unit();
            }
            // if the curve is open
            else {
                FSNode& n0 = mesh->Node(edge[0].n[0]);
                FSNode& n1 = mesh->Node(edge[0].n[1]);
                p = n0.pos();
                // get vector connecting nodes of first edge
                vec3d v01 = n1.pos() - n0.pos(); v01.unit();
                // evaluate the node_tangent
                vt = v01; vt.unit();
            }
        }
        // process last edge
        else if (i == NE-1) {
            // if curve is closed
            if (first_node == -1) {
                FSNode& n0 = mesh->Node(edge[i-1].n[0]);
                FSNode& n1 = mesh->Node(edge[i].n[0]);
                FSNode& n2 = mesh->Node(edge[i].n[1]);
                p = n1.pos();
                // get vector connecting nodes of first edge
                vec3d v01 = n1.pos() - n0.pos(); v01.unit();
                // get vector connecting nodes of second edge
                vec3d v12 = n2.pos() - n1.pos(); v12.unit();
                // evaluate the node_tangent
                vt = v01 + v12; vt.unit();
                // get node normal at n0
                FSFace& face = mesh->Face(edge[i].m_face[0]);
                vn1 = to_vec3d(face.m_nn[face.FindNode(edge[i].n[0])]);
            }
            // if the curve is open
            else {
                FSNode& n0 = mesh->Node(edge[i].n[0]);
                FSNode& n1 = mesh->Node(edge[i].n[1]);
                p = n0.pos();
                // get vector connecting nodes of first edge
                vec3d v01 = n1.pos() - n0.pos(); v01.unit();
                // evaluate the node_tangent
                vt = v01; vt.unit();
                // get node normal at n0
                FSFace& face = mesh->Face(edge[i].m_face[0]);
                vn1 = to_vec3d(face.m_nn[face.FindNode(edge[i].n[0])]);
            }
        }
        // get the extrusion direction
        vec3d ve = vt ^ vn1; ve.unit();
        // allocate room for the new nodes
        int nn = mesh->Nodes();
        mesh->Create(nn + m_nseg, 0, 0);
        for (int j=0; j< m_nseg; ++j) {
            FSNode& node = mesh->Node(nn+j);
            node.pos(p+ve*(rbias[j+1]*m_d));
        }
    }
    // if the curve is open, add the extrusion for the last node
    if (first_node > -1) {
        FSNode& n0 = mesh->Node(edge[NE-1].n[0]);
        FSNode& n1 = mesh->Node(edge[NE-1].n[1]);
        vec3d p = n1.pos();
        // get vector connecting nodes of first edge
        vec3d v01 = n1.pos() - n0.pos(); v01.unit();
        // evaluate the node_tangent
        vec3d vt = v01; vt.unit();
        // get node normal at n0
        FSFace& face = mesh->Face(edge[NE-1].m_face[0]);
        vec3d vn1 = to_vec3d(face.m_nn[face.FindNode(edge[NE-1].n[1])]);
        // get the extrusion direction
        vec3d ve = vt ^ vn1; ve.unit();
        // allocate room for the new nodes
        int nn = mesh->Nodes();
        mesh->Create(nn + m_nseg, 0, 0);
        for (int j=0; j< m_nseg; ++j) {
            FSNode& node = mesh->Node(nn+j);
            node.pos(p+ve*(rbias[j+1]*m_d));
        }
    }
    
    // create the faces (quadrilateral first)
    int NF = mesh->Faces();
    mesh->Create(0, 0, NF + NE*m_nseg);
    for (int j=0; j<m_nseg; ++j) {
        for (int i=0; i<NE; ++i) {
            FSFace& face = mesh->Face(NF + i + j*NE);
            face.SetType(FE_FACE_QUAD4);
            face.m_gid = 0;
            if ((first_node == -1) && (i == NE-1)) {
                if (j == 0) {
                    face.n[0] = edge[i].n[0];
                    if (!flip_first_edge) {
                        face.n[3] = edge[0].n[0];
                        face.n[2] = NN;
                        face.n[1] = NN + i*m_nseg;
                    }
                    else {
                        face.n[1] = edge[0].n[0];
                        face.n[2] = NN;
                        face.n[3] = NN + i*m_nseg;
                    }
                }
                else {
                    face.n[0] = NN + i*m_nseg + j - 1;
                    if (!flip_first_edge) {
                        face.n[3] = NN + j - 1;
                        face.n[2] = NN + j;
                        face.n[1] = NN + i*m_nseg + j;
                    }
                    else {
                        face.n[1] = NN + j - 1;
                        face.n[2] = NN + j;
                        face.n[3] = NN + i*m_nseg + j;
                    }
                }
            }
            else {
                if (j == 0) {
                    face.n[0] = edge[i].n[0];
                    if (!flip_first_edge) {
                        face.n[3] = edge[i].n[1];
                        face.n[2] = NN + (i+1)*m_nseg;
                        face.n[1] = NN + i*m_nseg;
                    }
                    else {
                        face.n[1] = edge[i].n[1];
                        face.n[2] = NN + (i+1)*m_nseg;
                        face.n[3] = NN + i*m_nseg;
                    }
                }
                else {
                    face.n[0] = NN + i*m_nseg + j - 1;
                    if (!flip_first_edge) {
                        face.n[3] = NN + (i+1)*m_nseg + j - 1;
                        face.n[2] = NN + (i+1)*m_nseg + j;
                        face.n[1] = NN + i*m_nseg + j;
                    }
                    else {
                        face.n[1] = NN + (i+1)*m_nseg + j - 1;
                        face.n[2] = NN + (i+1)*m_nseg + j;
                        face.n[3] = NN + i*m_nseg + j;
                    }
                }
            }
        }
    }
    
    // rebuild the mesh
    mesh->RebuildMesh();
    
    if (pm->IsType(FE_FACE_TRI3)) {
        
    }
    
    return mesh;
}
