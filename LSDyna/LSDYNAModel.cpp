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

#include "LSDYNAModel.h"
#include <FEMLib/FSModel.h>
#include "GeomLib/GMeshObject.h"
#include "FEMLib/FEMultiMaterial.h"

LSDYNAModel::LSDYNAModel()
{
	m_po = 0;
}

void LSDYNAModel::clear()
{
	m_node.clear();
	m_shell.clear();
	m_solid.clear();
	m_part.clear();

	if (m_po)
	{
		delete m_po;
		m_po = 0;
	}
}

int LSDYNAModel::FindNode(int id, list<LSDYNAModel::NODE>::iterator& pn)
{
	int N = (int)m_node.size();
	int m = 0;
	do
	{
		if (id == pn->id) return pn->n;
		else if (id < pn->id)
		{
			if (pn->n > 0) --pn; else return -1;
		}
		else
		{
			if (pn->n < N - 1) ++pn; else return -1;
		}
		++m;
	} while (m <= N);

	return -1;
}

int LSDYNAModel::FindFace(int n[4])
{
	int* pf = m_pFace[n[0]];
	int N = m_nFace[n[0]];
	int i;

	FSMesh* pm = m_po->GetFEMesh();

	for (i = 0; i<N; ++i)
	{
		FSFace& face = pm->Face(pf[i]);
		if (face.HasNode(n[1]) &&
			face.HasNode(n[2]) &&
			face.HasNode(n[3])) return pf[i];
	}

	return -1;
}

int LSDYNAModel::FindShellDomain(int pid)
{
    int N = (int)m_dshell.size();
    for (int i=0; i<N; ++i)
        if (m_dshell[i].pid == pid) return i;
    return -1;
}

bool LSDYNAModel::BuildModel(FSModel& fem)
{
	// build the mesh and object
	if (BuildFEMesh(fem) == false) return false;

	// build and assign materials
	if (BuildMaterials(fem) == false) return false;

	// all good
	return true;
}

bool LSDYNAModel::BuildFEMesh(FSModel& fem)
{
	int nodes = (int)m_node.size();
	int shells = (int)m_shell.size();
	int solids = (int)m_solid.size();
	int nparts = (int)m_part.size();

	int elems = shells + solids;

	if (nodes == 0) return false;
	if (elems == 0) return false;

	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// create nodes
	vector<NODE>::iterator in = m_node.begin();
	FSNode* pn = pm->NodePtr();
	int imin = 0, imax = 0;
	for (int i = 0; i<nodes; ++i, ++pn, ++in)
	{
		in->n = i;
		pn->r.x = in->x;
		pn->r.y = in->y;
		pn->r.z = in->z;

		if (i == 0 || (in->id < imin)) imin = in->id;
		if (i == 0 || (in->id > imax)) imax = in->id;
	}

	// build the node lookup table
	// this is used to convert node IDs into zero-based IDs
	int nsize = imax - imin + 1;
	vector<int> NLT(nsize, -1);
	in = m_node.begin();
	for (int i = 0; i<nodes; ++i, ++in) NLT[in->id - imin] = i;

	// get pointer to elements
	int eid = 0;

	// create solids
	if (solids > 0)
	{
		for (int i = 0; i<solids; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			ELEMENT_SOLID& ih = m_solid[i];
			ih.tag = i;
			int* n = ih.n;
			int pid = ih.pid;
			pe->m_gid = pid; // temporary assignment
			if ((n[7] == n[6]) && (n[7] == n[5]) && (n[7] == n[4]) && (n[7] == n[3])) pe->SetType(FE_TET4);
			else if ((n[7] == n[6]) && (n[7] == n[5]) && (n[7] == n[4]) && (n[3] == n[2]))
			{
				n[3] = n[7];
				pe->SetType(FE_TET4);
			}
			else if ((n[7] == n[6]) && (n[7] == n[5])) pe->SetType(FE_PENTA6);
			else if ((n[7] == n[6]) && (n[5] == n[4]))
			{
				int m[6] = { n[0], n[4], n[1], n[3], n[6], n[2] };
				n[0] = m[0]; n[1] = m[1]; n[2] = m[2];
				n[3] = m[3]; n[4] = m[4]; n[5] = m[5];
				n[6] = n[7] = n[5];
				pe->SetType(FE_PENTA6);
			}
			else pe->SetType(FE_HEX8);

			pe->m_node[0] = NLT[ih.n[0] - imin]; if (pe->m_node[0] < 0) return false;
			pe->m_node[1] = NLT[ih.n[1] - imin]; if (pe->m_node[1] < 0) return false;
			pe->m_node[2] = NLT[ih.n[2] - imin]; if (pe->m_node[2] < 0) return false;
			pe->m_node[3] = NLT[ih.n[3] - imin]; if (pe->m_node[3] < 0) return false;
			pe->m_node[4] = NLT[ih.n[4] - imin]; if (pe->m_node[4] < 0) return false;
			pe->m_node[5] = NLT[ih.n[5] - imin]; if (pe->m_node[5] < 0) return false;
			pe->m_node[6] = NLT[ih.n[6] - imin]; if (pe->m_node[6] < 0) return false;
			pe->m_node[7] = NLT[ih.n[7] - imin]; if (pe->m_node[7] < 0) return false;
		}
	}

	// create shells
	if (shells > 0)
	{
		for (int i = 0; i<shells; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			ELEMENT_SHELL& is = m_shell[i];
			is.tag = solids + i;

			pe->SetType(is.n[3] == is.n[2] ? FE_TRI3 : FE_QUAD4);
			pe->m_gid = is.pid;

			pe->m_node[0] = NLT[is.n[0] - imin]; if (pe->m_node[0] < 0) return false;
			pe->m_node[1] = NLT[is.n[1] - imin]; if (pe->m_node[1] < 0) return false;
			pe->m_node[2] = NLT[is.n[2] - imin]; if (pe->m_node[2] < 0) return false;
			pe->m_node[3] = NLT[is.n[3] - imin]; if (pe->m_node[3] < 0) return false;

            int n = FindShellDomain(is.pid);
            if (n != -1) {
                pe->m_h[0] = m_dshell[n].h[0];
                pe->m_h[1] = m_dshell[n].h[1];
                pe->m_h[2] = m_dshell[n].h[2];
                pe->m_h[3] = m_dshell[n].h[3];
            }
            else {
                pe->m_h[0] = is.h[0];
                pe->m_h[1] = is.h[1];
                pe->m_h[2] = is.h[2];
                pe->m_h[3] = is.h[3];
            }
		}
	}

	// partition the mesh
	if (nparts > 0)
	{
		int minpid = 0, maxpid = 0;
		for (int i=0; i<nparts; ++i)
		{
			PART& p = m_part[i];
			int pid = p.pid;
			if ((i==0) || (pid < minpid)) minpid = pid;
			if ((i==0) || (pid > maxpid)) maxpid = pid;
		}

		int nsize = maxpid - minpid + 1;
		vector<int> PLT(nsize, 0);

		int eid = 0;
		for (int i = 0; i < solids; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);
			PLT[pe->m_gid - minpid]++;
		}

		for (int i = 0; i < shells; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);
			PLT[pe->m_gid - minpid]++;
		}

		int n = 0;
		for (int i=0; i<nsize; ++i) 
		{
			if (PLT[i] > 0) PLT[i] = n++;
			else PLT[i] = -1;
		}

		eid = 0;
		for (int i = 0; i < solids; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);
			pe->m_gid = PLT[pe->m_gid - minpid];
		}
		for (int i = 0; i < shells; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);
			pe->m_gid = PLT[pe->m_gid - minpid];
		}

		for (int i=0; i<nparts; ++i)
		{
			PART& p = m_part[i];
			p.lid = PLT[p.pid - minpid];
		}
	}

	// update the mesh
	pm->RebuildMesh();

	m_po = new GMeshObject(pm);

	if (nparts > 0)
	{
		for (int i=0; i<nparts; ++i)
		{
			PART& p = m_part[i];

			int n = p.lid;
			if ((n>=0) && (n < m_po->Parts()))
			{
				GPart* pg = m_po->Part(n);
				pg->SetName(p.szname);
			}
		}
	}

	// create parts
/*	if (nparts > 0)
	{
		vector<PART>::iterator ip = m_part.begin();
		int nb0 = 0, nb1 = solids;
		int ns0 = 0, ns1 = shells;
		for (i = 0; i<nparts; ++i, ++ip)
		{
			FSPart* pg = new FSPart(m_po);

			pg->SetName(ip->szname);

			for (n = nb0; n<nb1; ++n)
			{
				ELEMENT_SOLID& ib = m_solid[n];
				if (ib.pid == ip->pid)
				{
					pg->add(ib.tag);
					ib.tag = -1;
				}

				if (ib.tag == -1)
				{
					if (n == nb0) nb0++;
					if (n == nb1 - 1) nb1--;
				}
			}

			for (n = ns0; n<ns1; ++n)
			{
				ELEMENT_SHELL& is = m_shell[n];
				if (is.pid == ip->pid)
				{
					pg->add(is.tag);
					is.tag = -1;
				}

				if (is.tag == -1)
				{
					if (n == ns0) ns0++;
					if (n == ns1 - 1) ns1--;
				}
			}

			m_po->AddFEPart(pg);
		}
	}
*/
	// create surfaces
	/*	if (m_set.size() > 0)
	{
	UpdateMesh(*pm);

	list<SET_SEGMENT_TITLE>::iterator pi;
	for (pi = m_set.begin(); pi != m_set.end(); ++pi)
	{
	SET_SEGMENT_TITLE& s = *pi;
	FSSurface* ps = new FSSurface(m_po);
	m_po->AddFESurface(ps);
	ps->SetName(s.m_szname);

	int F = s.m_face.size();
	for (int i=0; i<F; ++i)
	{
	int* fn = s.m_face[i].n;
	int n[4];
	n[0] = NLT[fn[0] - imin];
	n[1] = NLT[fn[1] - imin];
	n[2] = NLT[fn[2] - imin];
	n[3] = NLT[fn[3] - imin];
	int nf = FindFace(n);
	//                if (nf < 0) return false;
	if (nf < 0) break;
	ps->add(nf);
	}
	}
	}
	*/
	// clean up
	m_node.clear();
	m_shell.clear();
	m_solid.clear();

	// we're good!
	return true;
}

void LSDYNAModel::UpdateMesh(FSMesh& mesh)
{
	int nsize = 0, i, j, n, m, l;

	int nodes = mesh.Nodes();
	int faces = mesh.Faces();

	m_nFace.resize(nodes);
	for (i = 0; i<nodes; ++i) m_nFace[i] = 0;

	for (i = 0; i<faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j = 0; j<n; ++j) m_nFace[face.n[j]]++;
		nsize += n;
	}

	m_iFace.resize(nsize);
	m_pFace.resize(nodes);
	int *pi = &m_iFace[0];

	for (i = 0; i<nodes; ++i)
	{
		m_pFace[i] = pi;
		n = m_nFace[i];
		pi += n;
		m_nFace[i] = 0;
	}

	for (i = 0; i<faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j = 0; j<n; ++j)
		{
			m = face.n[j];

			l = m_nFace[m];
			m_pFace[m][l] = i;
			m_nFace[m]++;
		}
	}
}

bool LSDYNAModel::BuildMaterials(FSModel& fem)
{
	if (m_Mat.empty()) return true;

	// create materials
	list<MATERIAL*>::iterator im = m_Mat.begin();
	for (int i = 0; im != m_Mat.end(); ++i, ++im)
	{
		MATERIAL* glmat = *im;
		FSMaterial* gpmat = nullptr;
		if (dynamic_cast<MAT_ELASTIC*>(glmat)) {
			MAT_ELASTIC& lmat = *dynamic_cast<MAT_ELASTIC*>(glmat);
			FSIsotropicElastic* pmat = new FSIsotropicElastic(&fem);
			gpmat = pmat;
			pmat->SetFloatValue(FSIsotropicElastic::MP_DENSITY, lmat.ro);
			pmat->SetFloatValue(FSIsotropicElastic::MP_E, lmat.e);
			pmat->SetFloatValue(FSIsotropicElastic::MP_v, lmat.pr);
			pmat->SetName(lmat.szname);
		}
		else if (dynamic_cast<MAT_RIGID*>(glmat)) {
			MAT_RIGID& lmat = *dynamic_cast<MAT_RIGID*>(glmat);
			FSRigidMaterial* pmat = new FSRigidMaterial(&fem);
			gpmat = pmat;
			pmat->SetFloatValue(FSRigidMaterial::MP_DENSITY, lmat.ro);
			pmat->SetFloatValue(FSRigidMaterial::MP_E, lmat.e);
			pmat->SetFloatValue(FSRigidMaterial::MP_V, lmat.pr);
			pmat->SetName(lmat.szname);
		}
		else if (dynamic_cast<MAT_VISCOELASTIC*>(glmat)) {
			MAT_VISCOELASTIC& lmat = *dynamic_cast<MAT_VISCOELASTIC*>(glmat);
			FSUncoupledViscoElastic* pmat = new FSUncoupledViscoElastic(&fem);
			FSMooneyRivlin* emat = new FSMooneyRivlin(&fem);
			gpmat = pmat;
			emat->SetFloatValue(FSMooneyRivlin::MP_DENSITY, lmat.ro);
			emat->SetFloatValue(FSMooneyRivlin::MP_A, lmat.gi/2.);
			emat->SetFloatValue(FSMooneyRivlin::MP_B, 0);
			emat->SetFloatValue(FSMooneyRivlin::MP_K, lmat.bulk);
			pmat->SetElasticMaterial(emat);
			pmat->SetFloatValue(FSUncoupledViscoElastic::MP_G1, lmat.g0 / lmat.gi - 1);
			pmat->SetFloatValue(FSUncoupledViscoElastic::MP_T1, 1.0 / lmat.beta);
			pmat->SetName(lmat.szname);
		}
		else if (dynamic_cast<MAT_KELVIN_MAXWELL_VISCOELASTIC*>(glmat)) {
			MAT_KELVIN_MAXWELL_VISCOELASTIC& lmat = *dynamic_cast<MAT_KELVIN_MAXWELL_VISCOELASTIC*>(glmat);
			FSUncoupledViscoElastic* pmat = new FSUncoupledViscoElastic(&fem);
			FSMooneyRivlin* emat = new FSMooneyRivlin(&fem);
			gpmat = pmat;
			emat->SetFloatValue(FSMooneyRivlin::MP_DENSITY, lmat.ro);
			emat->SetFloatValue(FSMooneyRivlin::MP_A, lmat.gi/2.);
			emat->SetFloatValue(FSMooneyRivlin::MP_B, 0);
			emat->SetFloatValue(FSMooneyRivlin::MP_K, lmat.bulk);
			pmat->SetElasticMaterial(emat);
			pmat->SetFloatValue(FSUncoupledViscoElastic::MP_G1, lmat.g0 / lmat.gi - 1);
			pmat->SetFloatValue(FSUncoupledViscoElastic::MP_T1, 1.0 / lmat.dc);
			pmat->SetName(lmat.szname);
		}
		// For unknown materials, use MAT_ELASTIC
		else {
			FSIsotropicElastic* pmat = new FSIsotropicElastic(&fem);
			gpmat = pmat;
			pmat->SetName(glmat->szname);
		}

		GMaterial* pgm = new GMaterial(gpmat);
		fem.AddMaterial(pgm);

		// see if there is a part that has this material
		int nparts = (int)m_part.size();
		for (int j = 0; j<nparts; ++j)
		{
			PART& p = m_part[j];
			if (p.mid == glmat->mid)
			{
				int n = p.lid;
				if ((n>=0) && (m_po->Parts()))
				{
					GPart* pg = m_po->Part(n);
					pg->SetMaterialID(pgm->GetID());
				}
			}
		}
	}

	return true;
}
