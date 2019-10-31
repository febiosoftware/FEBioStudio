#include "stdafx.h"
#include "FESelectElementsFromFaces.h"
#include "FEDomain.h"
#include <MeshLib/MeshMetrics.h>
#include <map>

FESelectElementsFromFaces::FESelectElementsFromFaces() : FEModifier("Select Elements from Faces")
{
}

FEMesh* FESelectElementsFromFaces::Apply(FEMesh* pm)
{
    FEMesh* pnm = new FEMesh(*pm);
    SelectElementsFromFaces(pnm);
    return pnm;
}

void FESelectElementsFromFaces::SelectElementsFromFaces(FEMesh* pm)
{
    // tag elements for selection
    int ne0 = pm->Elements();
    vector<bool> selem(ne0, false);
    
    // store list of selected faces in fdata
    vector<FEFace> fdata;
    
    for (int i = 0; i<pm->Faces(); ++i)
    {
        FEFace& face = pm->Face(i);
        if (face.IsSelected()) {
            fdata.push_back(face);
            face.Unselect();
        }
    }
    
    int ne1 = (int)fdata.size();
    
    // map faces to their element
    std::map<int, vector<int>> fel;
    std::map<int, vector<int>>::iterator it;
    for (int i = 0; i<ne1; ++i)
    {
        FEFace face = fdata[i];
        // get element to which this face belongs
        int iel = face.m_elem[0].eid;
        // store faces that share this element
        fel[iel].push_back(i);
    }
    
    // mark all nodes on the selected faces
    for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = -1;
    for (int i = 0; i<ne1; ++i)
        for (int j = 0; j<fdata[i].Nodes(); ++j)
            pm->Node(fdata[i].n[j]).m_ntag = 1;
    
    // find all elements that share nodes with these faces
    // fne key = non-face element
    // fne mapped values = vector of entries into fdata faces
    std::map<int, vector<int>> fne;
    std::map<int, vector<int>>::iterator ie;
    for (int i = 0; i<pm->Elements(); ++i) {
        FEElement el = pm->Element(i);
        vector<int> shared_nodes;
        shared_nodes.reserve(el.Nodes());
        for (int j = 0; j<el.Nodes(); ++j) {
            if (pm->Node(el.m_node[j]).m_ntag == 1)
                shared_nodes.push_back(el.m_node[j]);
        }
        if (shared_nodes.size() > 0)
            fne[i] = shared_nodes;
    }
    
    for (it = fel.begin(); it != fel.end(); ++it) {
        if (it->second.size() == 1) {
            // only one face connected to this element
            int iel = (int)it->first;
            selem[iel] = true;
        }
        else if (it->second.size() == 2) {
            // two faces connected to this element
            FEFace face0 = fdata[it->second[0]];
            FEFace face1 = fdata[it->second[1]];
            // check if they share common nodes
            vector<int> cn;
            for (int i = 0; i<face0.Nodes(); ++i)
                for (int j = 0; j<face1.Nodes(); ++j)
                    if (face0.n[i] == face1.n[j]) cn.push_back(face0.n[i]);
            // only allow two shared nodes
            if (cn.size() != 2) return;
            int iel = (int)it->first;
            selem[iel] = true;
        }
        else if (it->second.size() > 2)
            // more than two faces share same element
            return;
    }
    
    // add hex and penta elements that belong to internal corner edges
    // add tet elements that share one or two nodes with selected faces
    for (ie = fne.begin(); ie != fne.end(); ++ie) {
        // we have an internal corner
        int iel = (int)ie->first;
        selem[iel] = true;
    }
    
    // select all the tagged elements
    for (int i = 0; i<ne0; ++i)
        if (selem[i]) pm->Element(i).Select();
}

