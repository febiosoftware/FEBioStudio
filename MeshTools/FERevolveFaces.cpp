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
#include "FERevolveFaces.h"
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/FEMeshBuilder.h>
using namespace std;

FERevolveFaces::FERevolveFaces() : FEModifier("Revolve faces")
{
    AddVecParam(vec3d(0,0,1), "axis", "axis");
    AddVecParam(vec3d(0,0,0), "center", "center");
    AddDoubleParam(1.0, "angle", "angle");	// assumed in degrees
    AddDoubleParam(0.0, "pitch", "pitch");
    AddIntParam(1, "segments", "segments");
	AddBoolParam(true, "degenerate", "Allow degenerate elemens");
}

inline bool pointOnAxis(const vec3d& c, const vec3d& n, const vec3d& p)
{
    double tol = 1e-10;
    vec3d q = p - c;
    q = q - n*(q*n);
    double Lp = p.Length();
    double Lq = q.Length();
    double Lr = (Lp > tol) ? Lq/Lp : Lq;
    return (Lr < tol);
}

FSMesh* FERevolveFaces::Apply(FSMesh* pm)
{
    //if (pm->IsType(FE_QUAD4)) return RevolveShellMesh(pm);
    return RevolveSolidMesh(pm);
}

FSMesh* FERevolveFaces::RevolveSolidMesh(FSMesh* pm)
{
    bool isQuad = false;
    
    // check if mesh is quadratic
    int NE = pm->Elements();
    for (int i=0; i<NE; ++i)
    {
        const FEElement_& el = pm->ElementRef(i);
        if ((el.Type() == FE_HEX20) || (el.Type() == FE_PENTA15) || (el.Type() == FE_PYRA13) || (el.Type() == FE_QUAD8) || (el.Type()==FE_TRI6))
        {
            isQuad = true;
            break;
        }
    }
    
    // find all nodes that need to be copied.
    pm->TagAllNodes(-1);
    int NF = pm->Faces();
    int ne1 = 0;
    for (int i=0; i<NF; ++i)
    {
        FSFace& f = pm->Face(i);
        
        if (f.IsSelected())
        {
            int nf = f.Nodes();
            for (int j=0; j<nf; ++j){
                pm->Node(f.n[j]).m_ntag = 1;
            }
            ne1++;
            
        }
    }
    
    // get the parameters
    vec3d axis = GetVecValue(0); axis.Normalize();
    vec3d center = GetVecValue(1);
    double w = GetFloatValue(2) * DEG2RAD;
    double p = GetFloatValue(3);
    int nseg = GetIntValue(4);
    if (nseg < 1) return 0;

	bool allowDegenerateElements = GetBoolValue(5);

   	// count the tagged nodes
    vector<int> nodeList;
    int NN0 = pm->Nodes();
    int nn = 0;
    
    for (int i=0; i<NN0; ++i)
    {
        FSNode& node = pm->Node(i);
        if (node.m_ntag == 1)
        {
            // if a node lies on the axis of rotation we do not need to duplicate it
            vec3d r = pm->NodePosition(i);
            if (pointOnAxis(center, axis, r) == false)
            {
                double l = (r - center)*axis;
                node.m_ntag = nn++;
                nodeList.push_back(i);
                // add extra node for quadratic meshes
                if(isQuad){
                    nodeList.push_back(i);
                    nn++;
                }
            }
            else node.m_ntag = -2;
        }
        
    }
    assert(nn == (int) nodeList.size());
    if (nn == 0) return 0;
    
    // allocate new mesh
    int NN1 = NN0 + nn*nseg;
    FSMesh* pmnew = new FSMesh(*pm);
    pmnew->Create(NN1, 0);
    
    // loop over all tagged nodes
    for (int i = 0; i<nn; ++i)
    {
        // stores position of nodes on the same rotation arc (used for quadratic meshes)
        vector<vec3d> npos;
        
        // revolve the tagged nodes
        for (int l = 1; l <= nseg; ++l)
        {
            // setup rotation
            double wl = w * l / nseg;
            quatd Q(wl, axis);
            
            FSNode& node = pmnew->Node(nodeList[i]);
            FSNode& node2 = pmnew->Node(NN0 + (l - 1)*nn + node.m_ntag);
            
            vec3d r = pm->LocalToGlobal(node.r) - center;
            Q.RotateVector(r);
            node2.r = pm->GlobalToLocal(center + r) + axis*(wl/(2*PI)*p);
            node2.m_ntag = node.m_ntag;
            
            npos.push_back(node2.r);
            
            if(isQuad){
                // setup rotation
                double wl = w * (l - 0.5) / nseg;
                quatd Q(wl, axis);
                
                // create middle node for quadratic meshes
                FSNode& node3 = pmnew->Node(1 + NN0 + (l - 1)*nn + node.m_ntag);
                
                vec3d r = pm->LocalToGlobal(node.r) - center;
                Q.RotateVector(r);
                node3.r = pm->GlobalToLocal(center + r) + axis*(wl/(2*PI)*p);
                node3.m_ntag = node.m_ntag;
            }
        }
    }
    
    // create new elements
    int NE0 = pm->Elements();
    pmnew->Create(0, NE0 + nseg*ne1);
    
    // count element partitions
    int nid = pmnew->CountElementPartitions();
    
    int n = NE0;
    for (int l = 1; l <= nseg; ++l)
    {
        for (int i=0; i<pm->Faces(); ++i)
        {
            FSFace& face = pmnew->Face(i);
            if (pm->Face(i).IsSelected())
            {
                int nf = face.Nodes();
                FSElement& el = pmnew->Element(n);
                
                if (nf == 6)
                {
                    bool wedge = true;
                    int NOA = 0;
                    for (int j=0; j<nf; ++j)
                    {
                        if (pmnew->Node(face.n[j]).m_ntag == -2){
                            wedge = false;
                            NOA++;
                        }
                    }
                    
                    if (wedge == true) {
                        el.SetType(FE_PENTA15);
                        el.m_gid = nid;
                        
                        el.m_node[0] = face.n[0];
                        el.m_node[1] = face.n[1];
                        el.m_node[2] = face.n[2];
                        
                        el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                        el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                        el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                        
                        el.m_node[6] = face.n[3];
                        el.m_node[7] = face.n[4];
                        el.m_node[8] = face.n[5];
                        
                        el.m_node[ 9] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                        el.m_node[10] = NN0 + (l - 1)*nn + pmnew->Node(face.n[4]).m_ntag;
                        el.m_node[11] = NN0 + (l - 1)*nn + pmnew->Node(face.n[5]).m_ntag;
                        
                        el.m_node[12] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                        el.m_node[13] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                        el.m_node[14] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                        
                        // move the face
                        face.n[0] = el.m_node[ 3];
                        face.n[1] = el.m_node[ 4];
                        face.n[2] = el.m_node[ 5];
                        
                        face.n[3] = el.m_node[ 9];
                        face.n[4] = el.m_node[10];
                        face.n[5] = el.m_node[11];
                        face.m_elem[0].eid = n;
                        face.m_elem[1].eid = -1;
                        face.m_elem[2].eid = -1;
                        ++n;
                    }
                    else {
                        for(int j = 0; j < 3; j++){
                            int n0 = face.n[j];
                            int n1 = face.n[(j + 1) % 3];
                            int n2 = face.n[(j + 2) % 3];
                            int n3 = face.n[j + 3];
                            int n4 = face.n[((j + 1) % 3) + 3];
                            int n5 = face.n[((j + 2) % 3) + 3];

                            if (NOA==1) {
                                el.SetType(FE_PYRA13);
                                el.m_gid = nid;
                                
                                if (pmnew->Node(n0).m_ntag == -2) {
                                    el.m_node[0] = n1;
                                    el.m_node[1] = n2;
                                    el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(n1).m_ntag;
                                    el.m_node[4] = n0;
                                    el.m_node[5] = n4;
                                    el.m_node[6] = 1 + NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[7] = NN0 + (l - 1)*nn + pmnew->Node(n4).m_ntag;
                                    el.m_node[8] = 1 + NN0 + (l - 1)*nn + pmnew->Node(n1).m_ntag;
                                    el.m_node[9] = n3;
                                    el.m_node[10] = n5;
                                    el.m_node[11] = NN0 + (l - 1)*nn + pmnew->Node(n5).m_ntag;
                                    el.m_node[12] = NN0 + (l - 1)*nn + pmnew->Node(n3).m_ntag;
                                    
                                    // move the face
                                    face.n[(j + 1) % 3] = el.m_node[3];
                                    face.n[(j + 2) % 3] = el.m_node[2];
                                    face.n[j + 3] = el.m_node[12];
                                    face.n[((j + 1) % 3) + 3] = el.m_node[7];
                                    face.n[((j + 2) % 3) + 3] = el.m_node[11];
                                    
                                    break;
                                }
                                
                            }
                            else {
                                el.SetType(FE_TET10);
                                el.m_gid = nid;
                                
                                if ((pmnew->Node(n0).m_ntag == -2) && (pmnew->Node(n1).m_ntag == -2)){
                                    el.m_node[0] = n2;
                                    el.m_node[1] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[2] = n1;
                                    el.m_node[3] = n0;
                                    el.m_node[4] = 1 + NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(n4).m_ntag;
                                    el.m_node[6] = n4;
                                    el.m_node[7] = n5;
                                    el.m_node[8] = NN0 + (l - 1)*nn + pmnew->Node(n5).m_ntag;
                                    el.m_node[9] = n3;

                                    // move the face
                                    face.n[(j + 2) % 3] = el.m_node[1];
                                    face.n[((j + 1) % 3) + 3] = el.m_node[5];
                                    face.n[((j + 2) % 3) + 3] = el.m_node[8];
                                    break;
                                }
                            }
                        }
                        face.m_elem[0].eid = n;
                        face.m_elem[1].eid = -1;
                        face.m_elem[2].eid = -1;
                        ++n;
                    }
                }
                else if(nf == 3)
                {
                    bool wedge = true;
                    int NOA = 0;
                    for (int j=0; j<nf; ++j)
                    {
                        if (pmnew->Node(face.n[j]).m_ntag == -2){
                            wedge = false;
                            NOA++;
                        }
                    }
                    
                    if(wedge == true){
                        el.SetType(FE_PENTA6);
                        el.m_gid = nid;
                        
                        el.m_node[0] = face.n[0];
                        el.m_node[1] = face.n[1];
                        el.m_node[2] = face.n[2];
                        
                        el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                        el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                        el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                        
                        // move the face
                        face.n[0] = el.m_node[3];
                        face.n[1] = el.m_node[4];
                        face.n[2] = el.m_node[5];
                        face.m_elem[0].eid = n;
                        face.m_elem[1].eid = -1;
                        face.m_elem[2].eid = -1;
                        ++n;
                    }
                    else{
                        for(int j = 0; j < nf; j++){
                            int n0 = face.n[j];
                            int n1 = face.n[(j + 1) % nf];
                            int n2 = face.n[(j + 2) % nf];
                            if(NOA==1){
                                el.SetType(FE_PYRA5);
                                el.m_gid = nid;
                                
                                if (pmnew->Node(n0).m_ntag == -2){
                                    el.m_node[0] = n1;
                                    el.m_node[1] = n2;
                                    el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(n1).m_ntag;
                                    el.m_node[4] = n0;
                                    
                                    // move the face
                                    face.n[(j + 1) % nf] = el.m_node[3];
                                    face.n[(j + 2) % nf] = el.m_node[2];
                                    
                                    break;
                                }
                                
                            }
                            else{
                                el.SetType(FE_TET4);
                                el.m_gid = nid;
                                
                                if ((pmnew->Node(n0).m_ntag == -2) && (pmnew->Node(n1).m_ntag == -2)){
                                    el.m_node[0] = n2;
                                    el.m_node[1] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[2] = n1;
                                    el.m_node[3] = n0;
                                    
                                    // move the face
                                    face.n[(j + 2) % nf] = el.m_node[1];
                                    break;
                                }
                            }
                        }
                        ++n;
                    }
                }
                
                else if (nf)
                {
                    // see if we need to make a hex or a wedge
                    bool bhex = true;
                    for (int j=0; j<nf; ++j) if (pmnew->Node(face.n[j]).m_ntag == -2) bhex = false;
                    
                    if (bhex)
                    {
                        if(isQuad){
                            el.SetType(FE_HEX20);
                            el.m_gid = nid;
                            
                            el.m_node[0] = face.n[0];
                            el.m_node[1] = face.n[1];
                            el.m_node[2] = face.n[2];
                            el.m_node[3] = face.n[3];
                            
                            el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                            el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                            el.m_node[6] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                            el.m_node[7] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                            
                            el.m_node[ 8] = face.n[4];
                            el.m_node[ 9] = face.n[5];
                            el.m_node[10] = face.n[6];
                            el.m_node[11] = face.n[7];
                            
                            el.m_node[12] = NN0 + (l - 1)*nn + pmnew->Node(face.n[4]).m_ntag;
                            el.m_node[13] = NN0 + (l - 1)*nn + pmnew->Node(face.n[5]).m_ntag;
                            el.m_node[14] = NN0 + (l - 1)*nn + pmnew->Node(face.n[6]).m_ntag;
                            el.m_node[15] = NN0 + (l - 1)*nn + pmnew->Node(face.n[7]).m_ntag;
                            
                            el.m_node[16] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                            el.m_node[17] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                            el.m_node[18] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                            el.m_node[19] = 1 + NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                            
                            // move part of face
                            face.n[4] = el.m_node[12];
                            face.n[5] = el.m_node[13];
                            face.n[6] = el.m_node[14];
                            face.n[7] = el.m_node[15];
                            
                        }
                        else{
                            el.SetType(FE_HEX8);
                            el.m_gid = nid;
                            
                            el.m_node[0] = face.n[0];
                            el.m_node[1] = face.n[1];
                            el.m_node[2] = face.n[2];
                            el.m_node[3] = face.n[3];
                            
                            el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                            el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                            el.m_node[6] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                            el.m_node[7] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                            
                        }
                        // move the face
                        face.n[0] = el.m_node[4];
                        face.n[1] = el.m_node[5];
                        face.n[2] = el.m_node[6];
                        face.n[3] = el.m_node[7];
                        face.m_elem[0].eid = n;
                        face.m_elem[1].eid = -1;
                        face.m_elem[2].eid = -1;
                    }
                    else
                    {
                        // we need to figure out which edge is the rotation axis
						bool bfound = false;
                        for (int j=0; j<4; ++j)
                        {
                            int n0 = face.n[j];
                            int n1 = face.n[(j + 1) % 4];
                            int n2 = face.n[(j + 2) % 4];
                            int n3 = face.n[(j + 3) % 4];
                            int n4 = face.n[j + 4];
                            int n5 = face.n[((j + 1) % 4) + 4];
                            int n6 = face.n[((j + 2) % 4) + 4];
                            int n7 = face.n[((j + 3) % 4) + 4];
                            
                            
                            if ((pmnew->Node(n0).m_ntag == -2) && (pmnew->Node(n1).m_ntag == -2))
                            {
                                
                                if(isQuad){
                                    el.SetType(FE_PENTA15);
                                    el.m_gid = nid;
                                    
                                    el.m_node[0] = n0;
                                    el.m_node[1] = n3;
                                    el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(n3).m_ntag;
                                    
                                    el.m_node[3] = n1;
                                    el.m_node[4] = n2;
                                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    
                                    el.m_node[6] = n7;
                                    el.m_node[7] = 1 + NN0 + (l - 1)*nn + pmnew->Node(n3).m_ntag;
                                    el.m_node[8] = NN0 + (l - 1)*nn + pmnew->Node(n7).m_ntag;
                                    
                                    el.m_node[ 9] = n5;
                                    el.m_node[10] = 1 + NN0 + (l - 1)*nn + pmnew->Node(n2).m_ntag;
                                    el.m_node[11] = NN0 + (l - 1)*nn + pmnew->Node(n5).m_ntag;
                                    
                                    el.m_node[12] = n4;
                                    el.m_node[13] = n6;
                                    el.m_node[14] = NN0 + (l - 1)*nn + pmnew->Node(n6).m_ntag;
                                    
                                    // move the face
                                    face.n[(j + 2) % 4] = el.m_node[5];
                                    face.n[(j + 3) % 4] = el.m_node[2];
                                    face.n[((j + 1) % 4) + 4] = el.m_node[11];
                                    face.n[((j + 2) % 4) + 4] = el.m_node[14];
                                    face.n[((j + 3) % 4) + 4] = el.m_node[8];
                                    face.m_elem[0].eid = n;
                                    face.m_elem[1].eid = -1;
                                    face.m_elem[2].eid = -1;
									bfound = true;
                                    break;
                                }
                                else{
                                    el.SetType(FE_PENTA6);
                                    el.m_gid = nid;
                                    
                                    el.m_node[0] = n0;
                                    el.m_node[1] = n3;
                                    el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 3) % nf]).m_ntag;
                                    
                                    el.m_node[3] = n1;
                                    el.m_node[4] = n2;
                                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 2) % nf]).m_ntag;
                                    
                                    // move the face
                                    face.n[(j + 2) % nf] = el.m_node[5];
                                    face.n[(j + 3) % nf] = el.m_node[2];
                                    face.m_elem[0].eid = n;
                                    face.m_elem[1].eid = -1;
                                    face.m_elem[2].eid = -1;
									bfound = true;
                                    break;
                                }
                            }
                        }

						if (!bfound)
						{
							// It's not a hex but also not a wedge element. 
							// If degenerate elements are allowed, we'll treat is a hex,
							// otherwise we abort
							if (!allowDegenerateElements)
							{
								SetError("Degenerate elements detected but not allowed.");
								delete pmnew;
								return nullptr;
							}
							else
							{
								if (isQuad) {
									el.SetType(FE_HEX20);
									el.m_gid = nid;

									el.m_node[0] = face.n[0];
									el.m_node[1] = face.n[1];
									el.m_node[2] = face.n[2];
									el.m_node[3] = face.n[3];

									el.m_node[4] = NN0 + (l - 1) * nn + pmnew->Node(face.n[0]).m_ntag;
									el.m_node[5] = NN0 + (l - 1) * nn + pmnew->Node(face.n[1]).m_ntag;
									el.m_node[6] = NN0 + (l - 1) * nn + pmnew->Node(face.n[2]).m_ntag;
									el.m_node[7] = NN0 + (l - 1) * nn + pmnew->Node(face.n[3]).m_ntag;

									el.m_node[8] = face.n[4];
									el.m_node[9] = face.n[5];
									el.m_node[10] = face.n[6];
									el.m_node[11] = face.n[7];

									el.m_node[12] = NN0 + (l - 1) * nn + pmnew->Node(face.n[4]).m_ntag;
									el.m_node[13] = NN0 + (l - 1) * nn + pmnew->Node(face.n[5]).m_ntag;
									el.m_node[14] = NN0 + (l - 1) * nn + pmnew->Node(face.n[6]).m_ntag;
									el.m_node[15] = NN0 + (l - 1) * nn + pmnew->Node(face.n[7]).m_ntag;

									el.m_node[16] = 1 + NN0 + (l - 1) * nn + pmnew->Node(face.n[0]).m_ntag;
									el.m_node[17] = 1 + NN0 + (l - 1) * nn + pmnew->Node(face.n[1]).m_ntag;
									el.m_node[18] = 1 + NN0 + (l - 1) * nn + pmnew->Node(face.n[2]).m_ntag;
									el.m_node[19] = 1 + NN0 + (l - 1) * nn + pmnew->Node(face.n[3]).m_ntag;

									// move part of face
									face.n[4] = el.m_node[12];
									face.n[5] = el.m_node[13];
									face.n[6] = el.m_node[14];
									face.n[7] = el.m_node[15];

								}
								else {
									el.SetType(FE_HEX8);
									el.m_gid = nid;

									el.m_node[0] = face.n[0];
									el.m_node[1] = face.n[1];
									el.m_node[2] = face.n[2];
									el.m_node[3] = face.n[3];

									el.m_node[4] = NN0 + (l - 1) * nn + pmnew->Node(face.n[0]).m_ntag;
									el.m_node[5] = NN0 + (l - 1) * nn + pmnew->Node(face.n[1]).m_ntag;
									el.m_node[6] = NN0 + (l - 1) * nn + pmnew->Node(face.n[2]).m_ntag;
									el.m_node[7] = NN0 + (l - 1) * nn + pmnew->Node(face.n[3]).m_ntag;

								}
								// move the face
								face.n[0] = el.m_node[4];
								face.n[1] = el.m_node[5];
								face.n[2] = el.m_node[6];
								face.n[3] = el.m_node[7];
								face.m_elem[0].eid = n;
								face.m_elem[1].eid = -1;
								face.m_elem[2].eid = -1;
							}
						}
                    }
                    
                    ++n;
                }
            }
        }
    }
    
    // clear face selection
    // TODO: Ideally, the corresponding faces in the mesh should be selected
    pmnew->ClearFaceSelection();
    
    // gets rid of nodes that are not used in any elements (mainly for quadratic meshes)
	FEMeshBuilder meshBuilder(*pmnew);
	meshBuilder.RemoveIsolatedNodes();
    
    // rebuild the object
	meshBuilder.RebuildMesh();
    
    // ensures all element have positive volume
    for (int i = 0; i<pmnew->Elements(); ++i) {
        double Ve = FEMeshMetrics::ElementVolume(*pmnew, pmnew->Element(i));
        if (Ve < 0)
            pmnew->Element(i).m_ntag = -1;
        else
            pmnew->Element(i).m_ntag = 1;
    }
	meshBuilder.InvertTaggedElements(-1);
    
    return pmnew;
}

FSMesh* FERevolveFaces::RevolveShellMesh(FSMesh* pm)
{
    // for now, only quad4 meshes
    //if (pm->IsType(FE_QUAD4) == false) return 0;
    
    // get the parameters
    vec3d axis = GetVecValue(0); axis.Normalize();
    vec3d center = GetVecValue(1);
    double w = GetFloatValue(2) * DEG2RAD;
    int nseg = GetIntValue(3);
    if (nseg < 1) return 0;
    
    // count the tagged nodes
    vector<int> nodeList;
    int NN0 = pm->Nodes();
    int nn = 0;
    for (int i = 0; i<NN0; ++i)
    {
        FSNode& node = pm->Node(i);
        
        // if a node lies on the axis of rotation we do not need to duplicate it
        vec3d r = pm->NodePosition(i);
        if (pointOnAxis(center, axis, r) == false)
        {
            double l = (r - center)*axis;
            node.m_ntag = nn++;
            nodeList.push_back(i);
        }
        else node.m_ntag = -2;
    }
    assert(nn == (int)nodeList.size());
    if (nn == 0) return 0;
    
    // allocate new mesh
    int NN1 = NN0 + nn*nseg;
    FSMesh* pmnew = new FSMesh(*pm);
    pmnew->Create(NN1, 0);
    
    // revolve the tagged nodes
    for (int l = 1; l <= nseg; ++l)
    {
        // setup rotation
        double wl = w * l / nseg;
        quatd Q(wl, axis);
        
        // loop over all tagged nodes
        for (int i = 0; i<nn; ++i)
        {
            FSNode& node = pmnew->Node(nodeList[i]);
            
            FSNode& node2 = pmnew->Node(NN0 + (l - 1)*nn + node.m_ntag);
            
            vec3d r = pm->LocalToGlobal(node.r) - center;
            Q.RotateVector(r);
            node2.r = pm->GlobalToLocal(center + r);
            node2.m_ntag = node.m_ntag;
        }
    }
    
    // create new elements
    int NE0 = pm->Elements();
    pmnew->Create(0, nseg*NE0);
    
    int n = 0;
    for (int l = 1; l <= nseg; ++l)
    {
        for (int i = 0; i<pm->Faces(); ++i)
        {
            FSFace& face = pmnew->Face(i);
            
            int nf = face.Nodes();
            FSElement& el = pmnew->Element(n);
            
            if (nf == 3)
            {
                el.SetType(FE_PENTA6);
                
                el.m_node[0] = face.n[0];
                el.m_node[1] = face.n[1];
                el.m_node[2] = face.n[2];
                
                el.m_node[3] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                
                // move the face
                face.n[0] = el.m_node[3];
                face.n[1] = el.m_node[4];
                face.n[2] = el.m_node[5];
                face.m_elem[0].eid = n;
                face.m_elem[1].eid = -1;
                face.m_elem[2].eid = -1;
                
                ++n;
            }
            else if (nf)
            {
                // see if we need to make a hex or a wedge
                bool bhex = true;
                for (int j = 0; j<nf; ++j) if (pmnew->Node(face.n[j]).m_ntag == -2) bhex = false;
                
                if (bhex)
                {
                    el.SetType(FE_HEX8);
                    
                    el.m_node[0] = face.n[0];
                    el.m_node[1] = face.n[1];
                    el.m_node[2] = face.n[2];
                    el.m_node[3] = face.n[3];
                    
                    el.m_node[4] = NN0 + (l - 1)*nn + pmnew->Node(face.n[0]).m_ntag;
                    el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[1]).m_ntag;
                    el.m_node[6] = NN0 + (l - 1)*nn + pmnew->Node(face.n[2]).m_ntag;
                    el.m_node[7] = NN0 + (l - 1)*nn + pmnew->Node(face.n[3]).m_ntag;
                    
                    // move the face
                    face.n[0] = el.m_node[4];
                    face.n[1] = el.m_node[5];
                    face.n[2] = el.m_node[6];
                    face.n[3] = el.m_node[7];
                    face.m_elem[0].eid = n;
                    face.m_elem[1].eid = -1;
                    face.m_elem[2].eid = -1;
                }
                else
                {
                    // we need to figure out which edge is the rotation axis
                    for (int j = 0; j<nf; ++j)
                    {
                        int n0 = face.n[j];
                        int n1 = face.n[(j + 1) % nf];
                        int n2 = face.n[(j + 2) % nf];
                        int n3 = face.n[(j + 3) % nf];
                        if ((pmnew->Node(n0).m_ntag == -2) && (pmnew->Node(n1).m_ntag == -2))
                        {
                            el.SetType(FE_PENTA6);
                            
                            el.m_node[0] = n0;
                            el.m_node[1] = n3;
                            el.m_node[2] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 3) % nf]).m_ntag;
                            
                            el.m_node[3] = n1;
                            el.m_node[4] = n2;
                            el.m_node[5] = NN0 + (l - 1)*nn + pmnew->Node(face.n[(j + 2) % nf]).m_ntag;
                            
                            // move the face
                            face.n[(j + 2) % nf] = el.m_node[5];
                            face.n[(j + 3) % nf] = el.m_node[2];
                            face.m_elem[0].eid = n;
                            face.m_elem[1].eid = -1;
                            face.m_elem[2].eid = -1;
                            
                            break;
                        }
                    }
                }
                
                ++n;
            }
        }
    }
    
    // rebuild the object
	FEMeshBuilder meshBuilder(*pmnew);
	meshBuilder.RebuildMesh();
	meshBuilder.RemoveIsolatedNodes();
    
    return pmnew;
}
