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
#include "FECreateShells.h"
#include <GeomLib/GMeshObject.h>
using namespace std;

FECreateShells::FECreateShells() : FEModifier("Create Shells")
{
    AddDoubleParam(0, "h", "h");
}


FSMesh* FECreateShells::Apply(FSGroup* pg)
{
    if (pg->Type() != FE_SURFACE)
    {
        FEModifier::SetError("Invalid selection");
        return 0;
    }
    
    if (pg->size() == 0)
    {
        FEModifier::SetError("Empty selection");
        return 0;
    }
    
    vector<int> faceList;
    for (FSItemListBuilder::Iterator it = pg->begin(); it != pg->end(); ++it)
    {
        faceList.push_back(*it);
    }
    
    FSMesh* pm = pg->GetMesh();
    FSMesh* pnm = new FSMesh(*pm);
    CreateShells(pnm, faceList);
    
    return pnm;
}



FSMesh* FECreateShells::Apply(FSMesh* pm)
{
    vector<int> faceList;
    for (int i=0; i<pm->Faces(); ++i)
    {
        if (pm->Face(i).IsSelected()) faceList.push_back(i);
    }
    
    FSMesh* pnm = new FSMesh(*pm);
    CreateShells(pnm, faceList);
    
    return pnm;
}



void FECreateShells::CreateShells(FSMesh* pm, vector<int>& faceList){
    
    //get the user value
    double thick = GetFloatValue(0);
    
    //we count the nbr of selected faces and that they are tri or quad
    int faces = 0;
    for (int i=0; i < (int)faceList.size(); ++i)
    {
        FSFace& face = pm->Face(faceList[i]);
        int n = face.Nodes();
        // verification
        if ((n != 4) && (n != 3) && (n != 9) && (n != 8) && (n != 6)) return;
        
        for (int j = 0; j<n; ++j) pm->Node(face.n[j]).m_ntag = 1;
        ++faces;
    }
    
    //error message when no face is selected
    if (faces==0){
        FEModifier::SetError("Empty Selection");
        return ;
    }
    
    // get the largest element group number
    int nid = 0;
    for (int i = 0; i<pm->Elements(); ++i)
    {
        FSElement& el = pm->Element(i);
        if (el.m_gid > nid) nid = el.m_gid;
    }
    nid++;
    
    
    // create the new shell elements
    int nbrelem = pm->Elements();
    pm->Create(0, nbrelem + faces);
    
    int n=nbrelem;
    for (int i=0; i <(int)faceList.size(); ++i){
        FSElement& pe=pm->Element(n);
        FSFace& face = pm->Face(faceList[i]);
        int nf = face.Nodes();
        switch (nf)
        {
            case 3: pe.SetType(FE_TRI3 ); break;
            case 4: pe.SetType(FE_QUAD4); break;
            case 6: pe.SetType(FE_TRI6 ); break;
            case 7: pe.SetType(FE_TRI7 ); break;
            case 8: pe.SetType(FE_QUAD8); break;
            case 9: pe.SetType(FE_QUAD9); break;
            default:
                assert(false);
                FEModifier::SetError("Failure");
                return ;
        }
        for (int j=0; j<nf; ++j) pe.m_node[j] = face.n[j];
        pe.m_gid = nid;
        double* h = pe.m_h;
        for (int j=0; j<pe.Nodes(); ++j) h[j] = thick;
        n++;
    }
    pm->RebuildMesh();
}
