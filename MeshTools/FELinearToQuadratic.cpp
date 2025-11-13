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
#include "FELinearToQuadratic.h"
#include <MeshLib/FSNodeNodeList.h>
#include <FECore/matrix.h>
using namespace std;

//-----------------------------------------------------------------------------
FELinearToQuadratic::FELinearToQuadratic() : FEModifier("Linear-to-Quadratic")
{
    m_bsmooth = false;
}

//-----------------------------------------------------------------------------
FSMesh* FELinearToQuadratic::Apply(FSMesh* pm)
{
    const int ELH8[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    const int ELP6[9][2] = {{0,1},{1,2},{2,0},{3,4},{4,5},{5,3},{0,3},{1,4},{2,5}};
    const int ELP5[8][2] = {{0,1},{1,2},{2,3},{3,0},{0,4},{1,4},{2,4},{3,4}};
    const int ELT4[6][2] = {{0,1},{1,2},{2,0},{0,3},{1,3},{2,3}};
    const int ELQ4[4][2] = {{0,1},{1,2},{2,3},{3,0}};
    const int ELT3[3][2] = {{0,1},{1,2},{2,0}};
    
    int NN = pm->Nodes();
    int NF = pm->Faces();
    int NT = pm->Elements();
    int NC = pm->Edges();
    
    // find all the edges
    vector< vector<int> > NEL; NEL.resize(NN);
    for (int i=0; i<NT; ++i)
    {
        FSElement& el = pm->Element(i);
        switch (el.Type()) {
            case FE_HEX8:
                for (int j=0; j<12; ++j)
                {
                    int n0 = el.m_node[ELH8[j][0]];
                    int n1 = el.m_node[ELH8[j][1]];
                    assert(n0 != n1);
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& nel = NEL[n0];
                    int nk = (int) nel.size();
                    int k = 0;
                    while ((k<nk)&&(nel[k] != n1)) k++;
                    if (k == nk) nel.push_back(n1);
                }
                break;
                
            case FE_PENTA6:
                for (int j=0; j<9; ++j)
                {
                    int n0 = el.m_node[ELP6[j][0]];
                    int n1 = el.m_node[ELP6[j][1]];
                    assert(n0 != n1);
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& nel = NEL[n0];
                    int nk = (int) nel.size();
                    int k = 0;
                    while ((k<nk)&&(nel[k] != n1)) k++;
                    if (k == nk) nel.push_back(n1);
                }
                break;
                
            case FE_PYRA5:
                for (int j=0; j<8; ++j)
                {
                    int n0 = el.m_node[ELP5[j][0]];
                    int n1 = el.m_node[ELP5[j][1]];
                    assert(n0 != n1);
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& nel = NEL[n0];
                    int nk = (int) nel.size();
                    int k = 0;
                    while ((k<nk)&&(nel[k] != n1)) k++;
                    if (k == nk) nel.push_back(n1);
                }
                break;
                
            case FE_TET4:
                for (int j=0; j<6; ++j)
                {
                    int n0 = el.m_node[ELT4[j][0]];
                    int n1 = el.m_node[ELT4[j][1]];
                    assert(n0 != n1);
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& nel = NEL[n0];
                    int nk = (int) nel.size();
                    int k = 0;
                    while ((k<nk)&&(nel[k] != n1)) k++;
                    if (k == nk) nel.push_back(n1);
                }
                break;
                
            case FE_QUAD4:
                for (int j=0; j<4; ++j)
                {
                    int n0 = el.m_node[ELQ4[j][0]];
                    int n1 = el.m_node[ELQ4[j][1]];
                    assert(n0 != n1);
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& nel = NEL[n0];
                    int nk = (int) nel.size();
                    int k = 0;
                    while ((k<nk)&&(nel[k] != n1)) k++;
                    if (k == nk) nel.push_back(n1);
                }
                break;
                
            case FE_TRI3:
                for (int j=0; j<3; ++j)
                {
                    int n0 = el.m_node[ELT3[j][0]];
                    int n1 = el.m_node[ELT3[j][1]];
                    assert(n0 != n1);
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& nel = NEL[n0];
                    int nk = (int) nel.size();
                    int k = 0;
                    while ((k<nk)&&(nel[k] != n1)) k++;
                    if (k == nk) nel.push_back(n1);
                }
                break;
                
            default:
                return 0;
        }
    }
    
    // count edges
    int NL = 0;
    for (int i=0; i<NN; ++i) NL += (int) NEL[i].size();
    
    // create the edge table
    vector<pair<int, int> > ET; ET.reserve(NL);
    int m = 0;
    for (int i=0; i<NN; ++i)
    {
        vector<int>& nel = NEL[i];
        int nk = (int) nel.size();
        for (int k=0; k<nk; ++k)
        {
            ET.push_back(pair<int, int>(i, nel[k]));
            nel[k] = m++;
        }
    }
    assert(NL == (int) ET.size());
    
    // create the element-edge table
    vector< vector<int> > EE; EE.assign(NT, vector<int>(12));
    for (int i=0; i<NT; ++i)
    {
        vector<int>& ee = EE[i];
        FSElement& e = pm->Element(i);
        switch (e.Type()) {
            case FE_HEX8:
                for (int j=0; j<12; ++j)
                {
                    int n0 = e.m_node[ELH8[j][0]];
                    int n1 = e.m_node[ELH8[j][1]];
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& ne = NEL[n0];
                    int nk = ne.size();
                    for (int k=0; k<nk; ++k)
                    {
                        pair<int, int>& ek = ET[ne[k]];
                        if ((ek.first == n0)&&(ek.second == n1))
                        {
                            ee[j] = ne[k];
                            break;
                        }
                    }
                }
                break;
                
            case FE_PENTA6:
                ee.resize(9);
                for (int j=0; j<9; ++j)
                {
                    int n0 = e.m_node[ELP6[j][0]];
                    int n1 = e.m_node[ELP6[j][1]];
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& ne = NEL[n0];
                    int nk = ne.size();
                    for (int k=0; k<nk; ++k)
                    {
                        pair<int, int>& ek = ET[ne[k]];
                        if ((ek.first == n0)&&(ek.second == n1))
                        {
                            ee[j] = ne[k];
                            break;
                        }
                    }
                }
                break;
                
            case FE_PYRA5:
                ee.resize(8);
                for (int j=0; j<8; ++j)
                {
                    int n0 = e.m_node[ELP5[j][0]];
                    int n1 = e.m_node[ELP5[j][1]];
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& ne = NEL[n0];
                    int nk = ne.size();
                    for (int k=0; k<nk; ++k)
                    {
                        pair<int, int>& ek = ET[ne[k]];
                        if ((ek.first == n0)&&(ek.second == n1))
                        {
                            ee[j] = ne[k];
                            break;
                        }
                    }
                }
                break;
                
            case FE_TET4:
                ee.resize(6);
                for (int j=0; j<6; ++j)
                {
                    int n0 = e.m_node[ELT4[j][0]];
                    int n1 = e.m_node[ELT4[j][1]];
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& ne = NEL[n0];
                    int nk = ne.size();
                    for (int k=0; k<nk; ++k)
                    {
                        pair<int, int>& ek = ET[ne[k]];
                        if ((ek.first == n0)&&(ek.second == n1))
                        {
                            ee[j] = ne[k];
                            break;
                        }
                    }
                }
                break;
                
            case FE_QUAD4:
                ee.resize(4);
                for (int j=0; j<4; ++j)
                {
                    int n0 = e.m_node[ELQ4[j][0]];
                    int n1 = e.m_node[ELQ4[j][1]];
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& ne = NEL[n0];
                    int nk = ne.size();
                    for (int k=0; k<nk; ++k)
                    {
                        pair<int, int>& ek = ET[ne[k]];
                        if ((ek.first == n0)&&(ek.second == n1))
                        {
                            ee[j] = ne[k];
                            break;
                        }
                    }
                }
                break;
                
            case FE_TRI3:
                ee.resize(3);
                for (int j=0; j<3; ++j)
                {
                    int n0 = e.m_node[ELT3[j][0]];
                    int n1 = e.m_node[ELT3[j][1]];
                    if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
                    
                    vector<int>& ne = NEL[n0];
                    int nk = ne.size();
                    for (int k=0; k<nk; ++k)
                    {
                        pair<int, int>& ek = ET[ne[k]];
                        if ((ek.first == n0)&&(ek.second == n1))
                        {
                            ee[j] = ne[k];
                            break;
                        }
                    }
                }
                break;
                
            default:
                break;
        }
    }
    
    // create the face-edge table
    vector< vector<int> > FE; FE.assign(NF, vector<int>(4));
    for (int i=0; i<NF; ++i)
    {
        FSFace& f = pm->Face(i);
        vector<int>& fe = FE[i];
        int nn = f.Nodes();
        fe.resize(nn);
        for (int j=0; j<nn; ++j)
        {
            int n0, n1;
            switch (nn) {
                case 3:
                    n0 = f.n[ELT4[j][0]];
                    n1 = f.n[ELT4[j][1]];
                    break;
                    
                case 4:
                    n0 = f.n[ELH8[j][0]];
                    n1 = f.n[ELH8[j][1]];
                    break;
                    
                default:
                    break;
            }
            
            if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
            
            vector<int>& ne = NEL[n0];
            int nk = ne.size();
            for (int k=0; k<nk; ++k)
            {
                pair<int, int>& ek = ET[ne[k]];
                if ((ek.first == n0)&&(ek.second == n1))
                {
                    fe[j] = ne[k];
                    break;
                }
            }
        }
    }
    
    // create the edge-edge table
    vector<int> CE; CE.assign(NC, -1);
    for (int i=0; i<NC; ++i)
    {
        FSEdge& e = pm->Edge(i);
        int n0 = e.n[0];
        int n1 = e.n[1];
        if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }
        
        vector<int>& ne = NEL[n0];
        int nk = ne.size();
        for (int k=0; k<nk; ++k)
        {
            pair<int, int>& ek = ET[ne[k]];
            if ((ek.first == n0)&&(ek.second == n1))
            {
                CE[i] = ne[k];
                break;
            }
        }
    }
    
    // the total number of new nodes is the number of old nodes plus the number of edges
    int NN1 = NN + NL;
    
    // allocate a new mesh
    FSMesh* pnew = new FSMesh;
    pnew->Create(NN1, NT, NF, NC);
    
    // copy the old nodes
    for (int i=0; i<NN; ++i)
    {
        FSNode& n0 = pm->Node(i);
        FSNode& n1 = pnew->Node(i);
        n1.r = n0.r;
        n1.m_gid = n0.m_gid;
    }
    
    // create the new edge nodes
    for (int i=0; i<(int) ET.size(); ++i)
    {
        FSNode& na = pm->Node(ET[i].first);
        FSNode& nb = pm->Node(ET[i].second);
        
        FSNode& n1 = pnew->Node(i + NN);
        n1.r = (na.r +nb.r)*0.5;
    }
    
    // create the elements
    for (int i=0; i<NT; ++i)
    {
        FSElement& e0 = pm->Element(i);
        FSElement& e1 = pnew->Element(i);
        e1 = e0;
        
        e1.m_gid = e0.m_gid;
        
        switch (e0.Type()) {
            case FE_HEX8:
            {
                e1.SetType(FE_HEX20);
                e1.m_node[0] = e0.m_node[0];
                e1.m_node[1] = e0.m_node[1];
                e1.m_node[2] = e0.m_node[2];
                e1.m_node[3] = e0.m_node[3];
                e1.m_node[4] = e0.m_node[4];
                e1.m_node[5] = e0.m_node[5];
                e1.m_node[6] = e0.m_node[6];
                e1.m_node[7] = e0.m_node[7];
                
                e1.m_node[ 8] = EE[i][ 0] + NN;
                e1.m_node[ 9] = EE[i][ 1] + NN;
                e1.m_node[10] = EE[i][ 2] + NN;
                e1.m_node[11] = EE[i][ 3] + NN;
                e1.m_node[12] = EE[i][ 4] + NN;
                e1.m_node[13] = EE[i][ 5] + NN;
                e1.m_node[14] = EE[i][ 6] + NN;
                e1.m_node[15] = EE[i][ 7] + NN;
                e1.m_node[16] = EE[i][ 8] + NN;
                e1.m_node[17] = EE[i][ 9] + NN;
                e1.m_node[18] = EE[i][10] + NN;
                e1.m_node[19] = EE[i][11] + NN;
            }
                break;
                
            case FE_PENTA6:
            {
                e1.SetType(FE_PENTA15);
                e1.m_node[0] = e0.m_node[0];
                e1.m_node[1] = e0.m_node[1];
                e1.m_node[2] = e0.m_node[2];
                e1.m_node[3] = e0.m_node[3];
                e1.m_node[4] = e0.m_node[4];
                e1.m_node[5] = e0.m_node[5];

                e1.m_node[ 6] = EE[i][ 0] + NN;
                e1.m_node[ 7] = EE[i][ 1] + NN;
                e1.m_node[ 8] = EE[i][ 2] + NN;
                e1.m_node[ 9] = EE[i][ 3] + NN;
                e1.m_node[10] = EE[i][ 4] + NN;
                e1.m_node[11] = EE[i][ 5] + NN;
                e1.m_node[12] = EE[i][ 6] + NN;
                e1.m_node[13] = EE[i][ 7] + NN;
                e1.m_node[14] = EE[i][ 8] + NN;
            }
                break;
                
            case FE_PYRA5:
            {
                e1.SetType(FE_PYRA13);
                e1.m_node[0] = e0.m_node[0];
                e1.m_node[1] = e0.m_node[1];
                e1.m_node[2] = e0.m_node[2];
                e1.m_node[3] = e0.m_node[3];
                e1.m_node[4] = e0.m_node[4];
                e1.m_node[5] = e0.m_node[5];
                
                e1.m_node[ 6] = EE[i][ 0] + NN;
                e1.m_node[ 7] = EE[i][ 1] + NN;
                e1.m_node[ 8] = EE[i][ 2] + NN;
                e1.m_node[ 9] = EE[i][ 3] + NN;
                e1.m_node[10] = EE[i][ 4] + NN;
                e1.m_node[11] = EE[i][ 5] + NN;
                e1.m_node[12] = EE[i][ 6] + NN;
            }
                break;
                
            case FE_TET4:
            {
                e1.SetType(FE_TET10);
                e1.m_node[0] = e0.m_node[0];
                e1.m_node[1] = e0.m_node[1];
                e1.m_node[2] = e0.m_node[2];
                e1.m_node[3] = e0.m_node[3];
                
                e1.m_node[4] = EE[i][0] + NN;
                e1.m_node[5] = EE[i][1] + NN;
                e1.m_node[6] = EE[i][2] + NN;
                e1.m_node[7] = EE[i][3] + NN;
                e1.m_node[8] = EE[i][4] + NN;
                e1.m_node[9] = EE[i][5] + NN;
            }
                break;
                
            case FE_QUAD4:
            {
                e1.SetType(FE_QUAD8);
                e1.m_node[0] = e0.m_node[0];
                e1.m_node[1] = e0.m_node[1];
                e1.m_node[2] = e0.m_node[2];
                e1.m_node[3] = e0.m_node[3];
                
                e1.m_node[4] = EE[i][0] + NN;
                e1.m_node[5] = EE[i][1] + NN;
                e1.m_node[6] = EE[i][2] + NN;
                e1.m_node[7] = EE[i][3] + NN;
            }
                break;
                
            case FE_TRI3:
            {
                e1.SetType(FE_TRI6);
                e1.m_node[0] = e0.m_node[0];
                e1.m_node[1] = e0.m_node[1];
                e1.m_node[2] = e0.m_node[2];
                
                e1.m_node[3] = EE[i][0] + NN;
                e1.m_node[4] = EE[i][1] + NN;
                e1.m_node[5] = EE[i][2] + NN;
            }
                break;
                
            default:
                break;
        }
        
    }
    
    // create the new faces
    for (int i=0; i<NF; ++i)
    {
        FSFace& f0 = pm->Face(i);
        FSFace& f1 = pnew->Face(i);
        
        switch (f0.Nodes()) {
            case 3:
                f1.SetType(FE_FACE_TRI6);
                f1.m_gid = f0.m_gid;
                f1.n[0] = f0.n[0];
                f1.n[1] = f0.n[1];
                f1.n[2] = f0.n[2];
                f1.n[3] = FE[i][0] + NN;
                f1.n[4] = FE[i][1] + NN;
                f1.n[5] = FE[i][2] + NN;
                f1.m_elem[0] = f0.m_elem[0];
                f1.m_elem[1] = f0.m_elem[1];
                f1.m_elem[2] = f0.m_elem[2];
                f1.m_nbr[0] = f0.m_nbr[0];
                f1.m_nbr[1] = f0.m_nbr[1];
                f1.m_nbr[2] = f0.m_nbr[2];
                break;
                
            case 4:
                f1.SetType(FE_FACE_QUAD8);
                f1.m_gid = f0.m_gid;
                f1.n[0] = f0.n[0];
                f1.n[1] = f0.n[1];
                f1.n[2] = f0.n[2];
                f1.n[3] = f0.n[3];
                f1.n[4] = FE[i][0] + NN;
                f1.n[5] = FE[i][1] + NN;
                f1.n[6] = FE[i][2] + NN;
                f1.n[7] = FE[i][3] + NN;
                f1.m_elem[0] = f0.m_elem[0];
                f1.m_elem[1] = f0.m_elem[1];
                f1.m_elem[2] = f0.m_elem[2];
                f1.m_nbr[0] = f0.m_nbr[0];
                f1.m_nbr[1] = f0.m_nbr[1];
                f1.m_nbr[2] = f0.m_nbr[2];
                f1.m_nbr[3] = f0.m_nbr[3];
                break;
                
            default:
                break;
        }
    }
    
    // create the new edges
    for (int i=0; i<NC; ++i)
    {
        FSEdge& e0 = pm->Edge(i);
        FSEdge& e1 = pnew->Edge(i);
        
		e1.SetType(FE_EDGE3);
        e1.n[0] = e0.n[0];
        e1.n[1] = e0.n[1];
        e1.n[2] = CE[i] + NN;
		e1.m_nid = e0.m_nid;
		e1.m_gid = e0.m_gid;
        e1.m_nbr[0] = e0.m_nbr[0];
        e1.m_nbr[1] = e0.m_nbr[1];
        e1.m_elem = e0.m_elem;
		e1.SetExterior(e0.IsExterior());
	}
    
    // apply surface smoothing
    if (m_bsmooth)
    {
        FESolidSmooth mod;
        mod.Apply(pnew);
    }
    
	pnew->UpdateMesh();
    
    return pnew;
}

//-----------------------------------------------------------------------------
void FESolidSmooth::Apply(FSMesh* pmesh)
{
    int NN = pmesh->Nodes();
    int NF = pmesh->Faces();
    vector<vec3d> rs; rs.assign(NN, vec3d(0,0,0));
    vector<int> tag; tag.assign(NN, 0);
    
    // tag all corner nodes
    // corner nodes = 1
    // edge nodes = 0
    // inside (non-surface) nodes = -1
    for (int i=0; i<NN; ++i) pmesh->Node(i).m_ntag = -1;
    for (int i=0; i<NF; ++i)
    {
        FSFace& f = pmesh->Face(i);
        for (int j=0; j<f.Nodes(); ++j) pmesh->Node(f.n[j]).m_ntag = 0;
    }
    for (int i=0; i<NF; ++i)
    {
        FSFace& f = pmesh->Face(i);
        for (int j=0; j<f.Nodes(); ++j) pmesh->Node(f.n[j]).m_ntag = 1;
    }
    
    // for now, ignore nodes that are no hard edges
    int NC = pmesh->Edges();
    for (int i=0; i<NC; ++i)
    {
        FSEdge& e = pmesh->Edge(i);
        pmesh->Node(e.n[0]).m_ntag = 2;
        pmesh->Node(e.n[1]).m_ntag = 2;
        pmesh->Node(e.n[2]).m_ntag = -1;
    }
    
    // calculate surface normals
	vector<vec3d> sn = pmesh->NodeNormals();
	assert(sn.size() == NN);

    // build the node-node list
	FSSurfaceNodeNodeList NNL(pmesh);

    // loop over all corner nodes
    for (int n=0; n<NN; ++n)
    {
        FSNode& ni = pmesh->Node(n);
        if (ni.m_ntag == 1)
        {
            vec3d r0 = ni.r;
            
            vector<int> n1;
            set<int>& l1 = NNL[n];
            set<int>::iterator it;
            for (it=l1.begin(); it != l1.end(); ++it)
            {
                FSNode& nj = pmesh->Node(*it);
                if (nj.m_ntag >= 1) n1.push_back(*it);
            }
            
            // get the nodal coordinates of all corner nodes, attached to this nodes
            int nn = (int) n1.size();
            vector<vec3d> x;
            for (int i=0; i<nn; ++i)
            {
                FSNode& nj = pmesh->Node(n1[i]);
                x.push_back(nj.r);
            }
            
            // construct local coordinate system
            vec3d e3 = sn[n];
            
            vec3d qx(1.0-e3.x*e3.x, -e3.y*e3.x, -e3.z*e3.x);
            if (qx.Length() < 1e-5) qx = vec3d(-e3.x*e3.y, 1.0-e3.y*e3.y, -e3.z*e3.y);
            qx.Normalize();
            vec3d e1 = qx;
            
            vec3d e2 = e3 ^ e1;
            
            mat3d Q;
            Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
            Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
            Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
            mat3d Qt = Q.transpose();
            
            // map coordinates
            vector<vec3d> y(nn);
            for (int i=0; i<nn; ++i)
            {
                vec3d tmp = x[i] - r0;
                y[i] = Q*tmp;
            }
            
            // polynomial coefficients
            double p[5] = {0};
            
            /*			if (nn >= 5)
             {
             // setup the linear system
             matrix R(nn, 5);
             vector<double> r(nn);
             for (int i=0; i<nn; ++i)
             {
             vec3d& p = y[i];
             R[i][0] = p.x*p.x;
             R[i][1] = p.x*p.y;
             R[i][2] = p.y*p.y;
             R[i][3] = p.x;
             R[i][4] = p.y;
             r[i] = p.z;
             }
             
             // solve for quadric coefficients
             vector<double> q(5);
             R.lsq_solve(q, r);
             p[0] = q[0]; p[1] = q[1]; p[2] = q[2];
             p[3] = q[3]; p[4] = q[4];
             }
             else if (nn >= 3)
             */			{
                 // setup the linear system
                 matrix R(nn, 3);
                 vector<double> r(nn);
                 for (int i=0; i<nn; ++i)
                 {
                     vec3d& p = y[i];
                     R[i][0] = p.x*p.x;
                     R[i][1] = p.x*p.y;
                     R[i][2] = p.y*p.y;
                     r[i] = p.z;
                 }
                 
                 // solve for quadric coefficients
                 vector<double> q(3);
                 R.lsq_solve(q, r);
                 p[0] = q[0]; p[1] = q[1]; p[2] = q[2];
                 p[3] = p[4] = 0.0;
             }
            
            // interpolate the quadratic for the center nodes
            for (it=l1.begin(); it != l1.end(); ++it)
            {
                FSNode& nj = pmesh->Node(*it);
                if (nj.m_ntag == 0)
                {
                    vec3d x = nj.r - r0;
                    vec3d y = Q*x;
                    y.z = p[0]*y.x*y.x + p[1]*y.x*y.y + p[2]*y.y*y.y + p[3]*y.x + p[4]*y.y;
                    x = Qt*y + r0;
                    rs[*it] += x;
                    tag[*it]++;
                }
            }
        }
    }
    
    // do all the edges nodes
    for (int i=0; i<NC; ++i)
    {
        FSEdge& edge1 = pmesh->Edge(i);
        for (int j=0; j<2; ++j)
        {
            int n0 = edge1.n[j];
            int n1 = edge1.n[(j+1)%2];
            if (edge1.m_nbr[j] >= 0)
            {
                FSEdge& edge2 = pmesh->Edge(edge1.m_nbr[j]);
                int n2 = edge2.n[0];
                if (n2 == n0) n2 = edge2.n[1];
                assert(n2 != n0);
                
                vec3d r0 = pmesh->Node(n0).r;
                vec3d r1 = pmesh->Node(n1).r;
                vec3d r2 = pmesh->Node(n2).r;
                
                vec3d a1 = r1 - r0;
                vec3d a2 = r2 - r0;
                
                vec3d e3 = a1^a2;
                if (e3.Length() > 1e-8)
                {
                    e3.Normalize();
                    vec3d e1 = r1 - r2; e1.Normalize();
                    vec3d e2 = e3 ^ e1;
                    
                    mat3d Q;
                    Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
                    Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
                    Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
                    mat3d Qt = Q.transpose();
                    
                    vec3d y1 = Q*a1;
                    vec3d y2 = Q*a2;
                    
                    matrix A(2,2);
                    A(0,0) = y1.x*y1.x; A(0,1) = y1.x;
                    A(1,0) = y2.x*y2.x; A(1,1) = y2.x;
                    
                    vector<double> R(2);
                    R[0] = y1.y;
                    R[1] = y2.y;
                    
                    vector<double> p(2);
                    A.solve(p, R);
                    
                    vec3d q1 = pmesh->Node(edge1.n[2]).r;
                    vec3d q2 = pmesh->Node(edge2.n[2]).r;
                    
                    vec3d s1 = Q*(q1 - r0);
                    vec3d s2 = Q*(q2 - r0);
                    
                    s1.y = p[0]*s1.x*s1.x + p[1]*s1.x;
                    s2.y = p[0]*s2.x*s2.x + p[1]*s2.x;
                    
                    q1 = Qt*s1 + r0;
                    q2 = Qt*s2 + r0;
                    rs[edge1.n[2]] += q1;
                    rs[edge2.n[2]] += q2;
                    tag[edge1.n[2]]++;
                    tag[edge2.n[2]]++;
                }
            }
        }
    }
    
    // apply the new coordinates of the center nodes
    for (int i=0; i<NN; ++i)
    {
        FSNode& ni = pmesh->Node(i);
        if (tag[i] > 0)
        {
            ni.r = rs[i] / (double) tag[i];
        }
    }
}

//-----------------------------------------------------------------------------
//! Convert a quadratic solid mesh to a linear solid mesh by eliminating all the center edge nodes
FSMesh* FEQuadraticToLinear::Apply(FSMesh* pm)
{
    // get the number of items
    int NN = pm->Nodes();
    int NE = pm->Elements();
    int NF = pm->Faces();
    int NC = pm->Edges();
    
    // count the number of corner nodes
    for (int i=0; i<NN; ++i) pm->Node(i).m_ntag = -1;
    for (int i=0; i<NE; ++i)
    {
        FSElement& el = pm->Element(i);
        int neln = 0;
        switch (el.Type()) {
            case FE_HEX20: neln = 8; break;
            case FE_PENTA15: neln = 6; break;
            case FE_PYRA13: neln = 5; break;
            case FE_TET10: neln = 4; break;
            case FE_QUAD8: neln = 4; break;
            case FE_TRI6: neln = 3; break;
            default: return 0; break;
        }
        for (int j=0; j<neln; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
    }
    int nn = 0;
    for (int i=0; i<NN; ++i)
    {
        FSNode& ni = pm->Node(i);
        if (ni.m_ntag == 1) 
        {
            ni.m_ntag = nn++;
        }
    }
    
    // allocate a new mesh
    FSMesh* pnew = new FSMesh;
    pnew->Create(nn, NE, NF, NC);
    
    // create the nodes
    nn = 0;
    for (int i=0; i<NN; ++i)
    {
        FSNode& n0 = pm->Node(i);
        if (n0.m_ntag >= 0)
        {
            FSNode& n1 = pnew->Node(nn++);
            n1.r = n0.r;
            n1.m_gid = n0.m_gid;
        }
    }
    
    // create the elements
    for (int i=0; i<NE; ++i)
    {
        FSElement& e0 = pm->Element(i);
        FSElement& e1 = pnew->Element(i);
        e1 = e0;
        
        e1.m_gid = e0.m_gid;
        
        switch (e0.Type()) {
            case FE_HEX20: e1.SetType(FE_HEX8); break;
            case FE_PENTA15: e1.SetType(FE_PENTA6); break;
            case FE_PYRA13: e1.SetType(FE_PYRA5); break;
            case FE_TET10: e1.SetType(FE_TET4); break;
            case FE_QUAD8: e1.SetType(FE_QUAD4); break;
            case FE_TRI6: e1.SetType(FE_TRI3); break;
            default: break;
        }
        
        for (int j=0; j<e1.Nodes(); ++j)
            e1.m_node[j] = pm->Node(e0.m_node[j]).m_ntag;
    }
    
    // create the new faces
    for (int i=0; i<NF; ++i)
    {
        FSFace& f0 = pm->Face(i);
        FSFace& f1 = pnew->Face(i);
        
        switch (f0.Nodes()) {
            case 6: f1.SetType(FE_FACE_TRI3); break;
            case 8: f1.SetType(FE_FACE_QUAD4); break;
            default: break;
        }
        f1.m_gid = f0.m_gid;
        for (int j=0; j<f1.Nodes(); ++j) {
            f1.n[j] = pm->Node(f0.n[j]).m_ntag;
            f1.m_nbr[j] = f0.m_nbr[j];
        }
        f1.m_elem[0] = f0.m_elem[0];
        f1.m_elem[1] = f0.m_elem[1];
        f1.m_elem[2] = f0.m_elem[2];
    }
    
    // create the new edges
    for (int i=0; i<NC; ++i)
    {
        FSEdge& e0 = pm->Edge(i);
        FSEdge& e1 = pnew->Edge(i);

		e1.SetType(FE_EDGE2);        
        e1.n[0] = pm->Node(e0.n[0]).m_ntag;
        e1.n[1] = pm->Node(e0.n[1]).m_ntag;
        e1.n[2] = -1;
        e1.m_nid = e0.m_nid;
        e1.m_gid = e0.m_gid;
        e1.m_nbr[0] = e0.m_nbr[0];
        e1.m_nbr[1] = e0.m_nbr[1];
        e1.m_elem = e0.m_elem;
		e1.SetExterior(e0.IsExterior());
	}
    
	pnew->UpdateMesh();
    
    return pnew;
}
