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
#include "FEBoundaryLayerMesher.h"
#include "FEDomain.h"
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/FSMeshBuilder.h>
#include <map>
using namespace std;

FEBoundaryLayerMesher::FEBoundaryLayerMesher() : FEModifier("PostBL")
{
	AddDoubleParam(1, "bias", "bias");
	AddIntParam(1, "Segments", "Segments");
}

FSMesh* FEBoundaryLayerMesher::Apply(FSMesh* pm)
{
	FSMesh* pnm = new FSMesh(*pm);
	if (BoundaryLayer(pnm) == false)
	{
		delete pnm;
		pnm = nullptr;
	}
	return pnm;
}

bool FEBoundaryLayerMesher::BoundaryLayer(FSMesh* pm)
{
	if (pm == nullptr) return false;

	// get the modifier's parameters
	double bias = GetFloatValue(0);
	int nseg = GetIntValue(1);
	if (nseg < 1) return false;
	if (nseg == 1) return true;

	// store list of selected faces in fdata
	vector<int> fdata;
	for (int i = 0; i<pm->Faces(); ++i)
	{
		const FSFace& face = pm->Face(i);
		if (face.IsSelected())
			fdata.push_back(i);
	}

	// make sure we have work to do
	int ne1 = (int)fdata.size();
	if (ne1 == 0)
	{
		SetError("No faces selected!");
		return false;
	}

	// map faces to their element
    // and mark those elements
    pm->TagAllElements(-1);
	std::map<int, vector<int>> efm;
	for (int i = 0; i<ne1; ++i)
	{
		FSFace& face = pm->Face(fdata[i]);
		// get element to which this face belongs
		int iel = face.m_elem[0].eid;
		// store faces that share this element
		efm[iel].push_back(i);
		pm->Element(iel).m_ntag = 1;
	}

	// mark all nodes on the selected faces
	pm->TagAllNodes(-1);
	for (int i = 0; i < ne1; ++i)
	{
		const FSFace& face = pm->Face(fdata[i]);
		for (int j = 0; j < face.Nodes(); ++j)
			pm->Node(face.n[j]).m_ntag = 1;
	}

	// find all elements that share nodes with these faces
	// fne key = non-face element
	// fne mapped values = vector of entries into fdata faces
	std::map<int, vector<int>> fne;
	for (int i = 0; i<pm->Elements(); ++i) {
		const FSElement& el = pm->Element(i);
        if (pm->Element(i).m_ntag == -1) {
            vector<int> shared_nodes;
            shared_nodes.reserve(el.Nodes());
            for (int j = 0; j<el.Nodes(); ++j) {
                if (pm->Node(el.m_node[j]).m_ntag == 1)
                    shared_nodes.push_back(el.m_node[j]);
            }
            if (shared_nodes.size() > 0)
                fne[i] = shared_nodes;
        }
	}

	// tag elements for deletion
	int ne0 = pm->Elements();
	vector<bool> delem(ne0, false);

	// create a domain from all selected elements
	FSDomain dom(pm);
	std::map<int, vector<int>>::iterator it;
	for (it = efm.begin(); it != efm.end(); ++it) {
		if (it->second.size() == 1) {
			// only one face connected to this element
			int iel = (int)it->first;
			// add element to domain
			dom.AddElement(iel);
			// set meshing parameters
			const FSFace& face = pm->Face(fdata[it->second[0]]);
			FSElement_& el = pm->Element(iel);
			if (el.Type() == FE_HEX8) {
				// get the box from this domain
				int ibox = dom.Boxes() - 1;
				FEDBox& box = dom.Box(ibox);
				// find the local face number in that element
				int ifc = box.FindBoxFace(face);
				if (ifc == -1) return false;
				if (!box.SetMeshSingleFace(ifc, nseg, bias, false))
					return false;
				// tag the original element for deletion
				delem[iel] = true;
			}
			else if (el.Type() == FE_PENTA6) {
				// get the wedge from this domain
				int iwdg = dom.Wedges() - 1;
				FEDWedge& wdg = dom.Wedge(iwdg);
				// find the local face number in that element
				int ifc = wdg.FindWedgeFace(face);
				if (ifc == -1) return  false;
				if (!wdg.SetMeshSingleFace(ifc, nseg, bias, false))
					return false;
				// tag the original element for deletion
				delem[iel] = true;
			}
			else if (el.Type() == FE_TET4) {
				// get the tet from this domain
				int itet = dom.Tets() - 1;
				FEDTet& tet = dom.Tet(itet);
				// find the local face number in that element
				int ifc = tet.FindTetFace(face);
				if (ifc == -1) return false;
				if (!tet.SetMeshFromFace(ifc, nseg, bias, false))
					return false;
				// tag the original element for deletion
				delem[iel] = true;
			}
		}
		else if (it->second.size() == 2) {
			// two faces connected to this element
			const FSFace& face0 = pm->Face(fdata[it->second[0]]);
			const FSFace& face1 = pm->Face(fdata[it->second[1]]);
			// check if they share common nodes
			vector<int> cn;
			for (int i = 0; i<face0.Nodes(); ++i)
				for (int j = 0; j<face1.Nodes(); ++j)
					if (face0.n[i] == face1.n[j]) cn.push_back(face0.n[i]);
			// only allow two shared nodes
			if (cn.size() != 2) return false;
			int iel = (int)it->first;
			FSElement_& el = pm->Element(iel);
			if (el.Type() == FE_HEX8) {
				// we have an external corner
				// add element to domain
				dom.AddElement(iel);
				// get the box we just added to this domain
				int ibox = dom.Boxes() - 1;
				FEDBox& box = dom.Box(ibox);
				// find the shared edge in the box
				int iedge = box.FindBoxEdge(cn[0], cn[1]);
				// split the box into wedges
				int iwdg[2];
				dom.SplitBoxIntoWedges(ibox, iedge, 1, iwdg);
				// set the meshing of these wedges
				FEDWedge& wdg0 = dom.Wedge(iwdg[0]);
				FEDWedge& wdg1 = dom.Wedge(iwdg[1]);
				// find the local face numbers in those wedges
				int ifc0, ifc1;
				ifc0 = wdg0.FindWedgeFace(face0);
				ifc1 = wdg1.FindWedgeFace(face1);
				if (ifc0 == -1) ifc0 = wdg0.FindWedgeFace(face1);
				if (ifc1 == -1) ifc1 = wdg1.FindWedgeFace(face0);
				if ((ifc0 == -1) || (ifc1 == -1)) return false;
				if (!wdg0.SetMeshSingleFace(ifc0, nseg, bias, false))
					return false;
				if (!wdg1.SetMeshSingleFace(ifc1, nseg, bias, false))
					return false;
				// tag the original element for deletion
				delem[iel] = true;
			}
			else if (el.Type() == FE_PENTA6) {
				return false;
			}
			else if (el.Type() == FE_TET4) {
				// we have a tet with two faces on the selected surface
				// let's find the other two faces of that tet
				FSFace opface[2];
				int ifc0 = el.FindFace(face0);
				int ifc1 = el.FindFace(face1);
				int k = 0;
				for (int i = 0; i<el.Faces(); ++i) {
					if ((i != ifc0) && (i != ifc1))
						opface[k++] = el.GetFace(i);
				}
				// for each of these other faces mesh the opposite element
				for (int i = 0; i<2; ++i) {
					// find the neighboring element
					int jel = -1;
					for (int k = 0; k<pm->Elements(); ++k) {
						FSElement_& oel = pm->Element(k);
						if ((k != iel) && (oel.Type() == FE_TET4))
							if (oel.FindFace(opface[i]) != -1) {
								jel = k;
								break;
							}
					}
					if (jel == -1)
						break;
					// add element to domain
					dom.AddElement(jel);
					// get the tet we just added to this domain
					int jtet = dom.Tets() - 1;
					FEDTet& tet = dom.Tet(jtet);
					// find the local face number in that element
					int jfc = tet.FindTetFace(opface[i]);
					if (jfc == -1) return false;
					if (!tet.SetMeshFromFace(jfc, nseg, bias, false))
						return false;
					// tag the original element for deletion
					delem[jel] = true;
				}
			}
		}
		else if (it->second.size() > 2)
			// more than two faces share same element
			return false;
	}

	// add hex and penta elements that belong to internal corner edges
	// add tet elements that share one or two nodes with selected faces
	std::map<int, vector<int>>::iterator ie;
	for (ie = fne.begin(); ie != fne.end(); ++ie) {
		// we have an internal corner
		int iel = (int)ie->first;
		FSElement_& el = pm->Element(iel);
		if ((ie->second.size() == 2) && (el.Type() == FE_HEX8)) {
			// add element to domain
			dom.AddElement(iel);
			// get the box we just added to this domain
			int ibox = dom.Boxes() - 1;
			FEDBox& box = dom.Box(ibox);
			// find the shared edge in the box
			int iedge = box.FindBoxEdge(ie->second[0], ie->second[1]);
			// split the box into wedges
			int iwdg[2];
			dom.SplitBoxIntoWedges(ibox, iedge, 0, iwdg);
			// set the meshing of these wedges
			FEDWedge& wdg0 = dom.Wedge(iwdg[0]);
			FEDWedge& wdg1 = dom.Wedge(iwdg[1]);
			// find the local face numbers in those wedges
			int edg0, edg1;
			edg0 = wdg0.FindWedgeEdge(ie->second[0], ie->second[1]);
			edg1 = wdg1.FindWedgeEdge(ie->second[0], ie->second[1]);
			if ((edg0 == -1) || (edg1 == -1)) return false;
			if (!wdg0.SetMeshSingleEdge(edg0, nseg, bias, false))
				return false;
			if (!wdg1.SetMeshSingleEdge(edg1, nseg, bias, false))
				return false;
			// tag the original element for deletion
			delem[iel] = true;
		}
		else if ((ie->second.size() == 2) && (el.Type() == FE_PENTA6)) {
			// add element to domain
			dom.AddElement(iel);
			// get the wedge we just added to this domain
			int iwdg = dom.Wedges() - 1;
			FEDWedge& wdg = dom.Wedge(iwdg);
			// find the shared edge in the wedge
			int iedge = wdg.FindWedgeEdge(ie->second[0], ie->second[1]);
			if (!wdg.SetMeshSingleEdge(iedge, nseg, bias, false))
				return false;
			// tag the original element for deletion
			delem[iel] = true;
		}
		else if ((ie->second.size() == 1) && (el.Type() == FE_PENTA6)) {
			// add element to domain
			dom.AddElement(iel);
			// get the wedge we just added to this domain
			int iwdg = dom.Wedges() - 1;
			FEDWedge& wdg = dom.Wedge(iwdg);
			// find the shared vertex in the wedge
			int ivtx = wdg.FindWedgeVertex(ie->second[0]);
			if (ivtx == -1) return false;
			// split the wedge into tets
			int itet[3];
			dom.SplitWedgeIntoTets(iwdg, ivtx, itet);
			// set the meshing of the first of these tets
			FEDTet& tet = dom.Tet(itet[0]);
			// find the local vertex number in this tet
			ivtx = tet.FindTetVertex(ie->second[0]);
			if (ivtx == -1) return false;
			if (!tet.SetMeshFromVertex(ivtx, nseg, bias, false))
				return false;
			// tag the original element for deletion
			delem[iel] = true;
			return false;
		}
		else if ((ie->second.size() == 1) && (el.Type() == FE_TET4)) {
			// add element to domain
			dom.AddElement(iel);
			// get the tet we just added to this domain
			int itet = dom.Tets() - 1;
			FEDTet& tet = dom.Tet(itet);
			// find the shared node in the tet
			int ivtx = tet.FindTetVertex(ie->second[0]);
			if (!tet.SetMeshFromVertex(ivtx, nseg, bias, false))
				return false;
			// tag the original element for deletion
			delem[iel] = true;
		}
		else if ((ie->second.size() == 2) && (el.Type() == FE_TET4)) {
			// add element to domain
			dom.AddElement(iel);
			// get the tet we just added to this domain
			int itet = dom.Tets() - 1;
			FEDTet& tet = dom.Tet(itet);
			// find the shared edge in the tet
			int iedge = tet.FindTetEdge(ie->second[0], ie->second[1]);
			if (!tet.SetMeshFromEdge(iedge, nseg, bias, false))
				return false;
			// tag the original element for deletion
			delem[iel] = true;
		}
		else {
			bool weird = true;
			//            return;
		}
	}

	// create the mesh
	if (!dom.MeshDomain())
		return false;

	// delete all the tagged elements
	pm->TagAllElements(1);
	for (int i = 0; i<ne0; ++i)
		if (delem[i]) pm->Element(i).m_ntag = -1;

	FSMeshBuilder meshBuilder(*pm);
	meshBuilder.DeleteTaggedElements(-1);

	// rebuild the object
	meshBuilder.RebuildMesh();

	// check for inverted elements
	for (int i = 0; i<pm->Elements(); ++i) {
		double Ve = FEMeshMetrics::ElementVolume(*pm, pm->Element(i));
		if (Ve < 0)
			pm->Element(i).m_ntag = -1;
		else
			pm->Element(i).m_ntag = 1;
	}
	meshBuilder.InvertTaggedElements(-1);

	return true;
}
