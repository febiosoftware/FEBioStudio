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
#include "FEModifier.h"
#include "FELinearToQuadratic.h"

//-----------------------------------------------------------------------------
//! Convert a penta6 to a tet4 mesh by converting each penta to 3 tets
FSMesh* FEPenta6ToTet4::Apply(FSMesh* pm)
{
    // get the mesh metrics
    int NN0 = pm->Nodes();
    int NE0 = pm->Elements();
    int NF0 = pm->Faces();
    
    // Make sure this mesh only has pentas and elements that are not connected to hexes
    for (int i = 0; i < NE0; ++i)
    {
        FSElement& el = pm->Element(i);
        
        // make sure this is a solid
        if (el.IsSolid() == false) return nullptr;
        
        // make sure any non-hex element is not connected to a hex
        if (el.Type() != FE_PENTA6)
        {
            // make sure this is not connected to a hex
            for (int j = 0; j < el.Faces(); ++j)
            {
                FSElement_* pne = pm->ElementPtr(el.m_nbr[j]);
                if ((pne != nullptr) && (pne->Type() == FE_PENTA6)) return nullptr;
            }
        }
    }
    
    // create the new mesh
    FSMesh* mesh = new FSMesh;
    
    // allocate nodes and edges, since they are the same
    mesh->Create(NN0, 0);
    
    // copy nodes
    for (int i = 0; i < NN0; ++i)
    {
        mesh->Node(i) = pm->Node(i);
    }
    
    // tag all the faces to split
    pm->TagAllFaces(0);
    
    for (int i = 0; i < NE0; ++i)
    {
        FSElement& el = pm->Element(i);
        if (el.IsType(FE_PENTA6))
        {
            for (int j = 0; j < 5; ++j)
            {
                int fj = el.m_face[j];
                if ((fj >= 0) && (pm->Face(fj).Nodes() == 4)) pm->Face(fj).m_ntag = 1;
            }
        }
    }
    
    // each quad will be split in two
    int NF1 = 0;
    for (int i = 0; i < NF0; ++i)
    {
        FSFace& fs = pm->Face(i);
        if (fs.m_ntag == 1)
        {
            assert(fs.Type() == FE_FACE_QUAD4);
            NF1 += 2;
        }
        else NF1++;
    }
    
    int fc = 0;
    mesh->Create(0, 0, NF1);
    for (int i = 0; i < NF0; ++i)
    {
        FSFace& fs = pm->Face(i);
        
        if (fs.m_ntag == 0)
        {
            FSFace& f0 = mesh->Face(fc++);
            f0 = fs;
        }
        else
        {
            // find the lowest vertex index
            int l = 0;
            for (int j = 1; j < 4; ++j)
            {
                if (fs.n[j] < fs.n[l]) l = j;
            }
            
            // split at this lowest vertex
            FSFace& f0 = mesh->Face(fc++);
            f0.m_gid = fs.m_gid;
            f0.SetType(FE_FACE_TRI3);
            f0.n[0] = fs.n[(l + 0) % 4];
            f0.n[1] = fs.n[(l + 1) % 4];
            f0.n[2] = fs.n[(l + 2) % 4];
            
            FSFace& f1 = mesh->Face(fc++);
            f1.SetType(FE_FACE_TRI3);
            f1.m_gid = fs.m_gid;
            f1.n[0] = fs.n[(l + 2) % 4];
            f1.n[1] = fs.n[(l + 3) % 4];
            f1.n[2] = fs.n[(l + 0) % 4];
        }
    }
    
    // node look-up table after rotating cube to have lowest vertex at pivot
    int LT[6][6] = {
        { 0, 1, 2, 3, 4, 5 },
        { 1, 2, 0, 4, 5, 3 },
        { 2, 0, 1, 5, 3, 4 },
        { 3, 4, 5, 0, 1, 2 },
        { 4, 5, 3, 1, 2, 0 },
        { 5, 3, 4, 2, 0, 1 }
    };
    
    // The element faces
    // Note that the first two faces both start at node 0, and
    // these faces will always be split by the diagonal that goes through node 0
    int FT[5][4] = {
        { 0, 1, 4, 3 },
        { 0, 3, 5, 2 },
        { 2, 5, 4, 1 },
        {-1, 0, 2, 1 },
        {-1, 3, 4, 5 }
    };
    
    // the 3 tet elements in the penta
    int PT[2][3][4] = {
        {{ 0, 3, 4, 5}, { 0, 1, 2, 4}, { 0, 4, 2, 5}},
        {{ 0, 3, 4, 5}, { 0, 5, 4, 1}, { 0, 1, 2, 5}}
    };
    
    // Figure out how to split each element
    int NE1 = 0;
    std::vector<int> tag(NE0, -1);
    for (int i = 0; i < NE0; ++i)
    {
        FSElement& els = pm->Element(i);
        
        if (els.Type() == FE_PENTA6) {
            int* n = els.m_node;
            
            // find the lowest element vertex
            int l = 0;
            for (int j = 1; j < 6; ++j)
            {
                if (n[j] < n[l]) l = j;
            }
            int* L = LT[l];
            
            // calculate the case by inspecting the last quad face
            // (we already know how to split the first two quad faces)
            int ncase = 0;
            for (int j = 2; j < 3; ++j)
            {
                int* F = FT[j];
                
                // get the lowest index for this face
                int l0 = 0;
                for (int k = 1; k < 4; ++k)
                {
                    if (n[L[F[k]]] < n[L[F[l0]]]) l0 = k;
                }
                int l1 = (l0 + 2) % 4;
                
                // if the diagonal (l0, l1) constains node 1, we tag it
                if ((F[l0] == 1) || (F[l1] == 1)) ncase |= (1 << (j - 2));
            }
            
            // both cases give us 3 tets
            NE1 += 3;
            tag[i] = ncase;
        }
        else NE1++;
    }
    
    // allocate the new elements
    mesh->Create(0, NE1);
    int ec = 0;
    for (int i = 0; i < NE0; ++i)
    {
        FSElement& els = pm->Element(i);
        int* n = els.m_node;
        
        if (els.Type() != FE_PENTA6)
        {
            assert(tag[i] == -1);
            FSElement& eld = mesh->Element(ec++);
            eld = els;
        }
        else
        {
            // find the lowest element vertex
            int l = 0;
            for (int j = 0; j < 6; ++j)
            {
                if (n[j] < n[l]) l = j;
            }
            int* L = LT[l];
            // calculate the case by inspecting the last face
            // (we already know how to split the first two faces)
            int ncase = tag[i];

            // get the tets
            for (int k = 0; k < 1; ++k)
            {
                const int(*f)[4] = PT[ncase];
                int nt = 3;
                for (int j = 0; j < nt; ++j)
                {
                    FSElement& tet = mesh->Element(ec++);
                    tet.SetType(FE_TET4);
                    tet.m_gid = els.m_gid;
                    
                    tet.m_node[0] = n[L[f[j][0]]];
                    tet.m_node[1] = n[L[f[j][1]]];
                    tet.m_node[2] = n[L[f[j][2]]];
                    tet.m_node[3] = n[L[f[j][3]]];
                }
            }
        }
    }
    
    mesh->BuildMesh();
    
    return mesh;
}

//-----------------------------------------------------------------------------
//! Convert a penta6 to a penta15 mesh by converting each penta to 3 tets
FSMesh* FEPenta6ToPenta15::Apply(FSMesh* pm)
{
	FELinearToQuadratic mod;
	return mod.Apply(pm);
}
