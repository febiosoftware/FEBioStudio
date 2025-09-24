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
#include "ModelBuilder.h"
#include "AbaqusModel.h"
#include <FEMLib/FSModel.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/GDiscreteObject.h>
#include <FEBioLink/FEBioClass.h>
#include <sstream>
using std::stringstream;


ModelBuilder::ModelBuilder()
{

}

bool ModelBuilder::BuildGeometry(AbaqusModel& inp, GModel& mdl)
{
	FSMesh* pm = nullptr;
	m_inp = &inp;

	int nodes = 0, elems = 0;
	nodes += inp.GlobalPart().Nodes();
	elems += inp.GlobalPart().CountElements();

	AbaqusModel::ASSEMBLY* asmbly = inp.GetAssembly();
	if (asmbly == nullptr)
	{
		// create an assembly of all the parts
		asmbly = inp.CreateAssembly();
		if (asmbly == nullptr) return false;

		asmbly->m_name = inp.GetName();

		list<AbaqusModel::PART*>& Part = inp.PartList();
		for (auto pi : Part)
		{
			AbaqusModel::INSTANCE* pinst = asmbly->AddInstance();
			pinst->SetName(pi->m_name);
			pinst->SetPartName(pi->m_name);
		}
	}

	// instantiate all the parts
	list<AbaqusModel::INSTANCE*>& Inst = asmbly->InstanceList();
	for (auto it : Inst)
	{
		AbaqusModel::PART* pg = m_inp->FindPart(it->GetPartName());
		if (pg == nullptr) return false;
		if (pg->m_instance != nullptr)
		{
			pg = pg->Clone();
			inp.AddPart(pg);
		}
		it->SetPart(pg);
	}

	// loop over all instances
	list<AbaqusModel::INSTANCE*>::iterator pi;
	for (pi = Inst.begin(); pi != Inst.end(); ++pi)
	{
		AbaqusModel::INSTANCE* pinst = *pi;
		AbaqusModel::PART* part = pinst->GetPart();
		if (part == nullptr) return false;

		nodes += part->Nodes();
		elems += part->CountElements();
	}

	if ((nodes == 0) || (elems == 0)) return false;

	// create the mesh
	pm = new FSMesh();
	pm->Create(nodes, elems);

	// add the nodes from the global part
	int n = 0;
	Transform T;
	AbaqusModel::PART& gpart = inp.GlobalPart();
	build_nodes(&gpart, pm, T, n);

	// add the nodes from the parts
	for (pi = Inst.begin(); pi != Inst.end(); ++pi)
	{
		AbaqusModel::INSTANCE* pinst = *pi;
		AbaqusModel::PART* part = pinst->GetPart();

		// apply translation
		double t[3];
		pinst->GetTranslation(t);
		T.SetPosition(vec3d(t[0], t[1], t[2]));

		// apply rotation
		double R[7];
		pinst->GetRotation(R);
		if (R[6] != 0.0)
		{
			vec3d a = vec3d(R[0], R[1], R[2]);
			vec3d b = vec3d(R[3], R[4], R[5]);
			T.Rotate(a, b, R[6]);
		}

		// add the nodes
		build_nodes(part, pm, T, n);
	}

	// add the elements
	int m = 0;
	if (gpart.Elements() > 0) build_elems(&gpart, pm, m);
	for (pi = Inst.begin(); pi != Inst.end(); ++pi)
	{
		build_elems((*pi)->GetPart(), pm, m);
	}

	// partition the elements
	m_partitions.clear();
	if (gpart.Elements() > 0) partition_elems(&gpart, pm);
	for (pi = Inst.begin(); pi != Inst.end(); ++pi)
	{
		partition_elems((*pi)->GetPart(), pm);
	}

	pm->RebuildMesh(60.0, false);
	GMeshObject* po = new GMeshObject(pm);

	// add nodesets
	build_nodesets(gpart, pm);
	for (pi = Inst.begin(); pi != Inst.end(); ++pi) build_nodesets(*(*pi)->GetPart(), pm);

	// add elemsets
	build_elemsets(inp.GlobalPart(), pm);
	for (pi = Inst.begin(); pi != Inst.end(); ++pi) build_elemsets(*(*pi)->GetPart(), pm);

	// add surfaces
	build_surfaces(&inp.GlobalPart(), pm);
	for (pi = Inst.begin(); pi != Inst.end(); ++pi) build_surfaces((*pi)->GetPart(), pm);

	// assign part names
	assert(m_partitions.size() == po->Parts());
	if (m_partitions.size() == po->Parts())
	{
		for (int i = 0; i < m_partitions.size(); ++i)
		{
			po->Part(i)->SetName(m_partitions[i].first);
		}
	}

	// assign a name to the object
	po->SetName(asmbly->m_name);

	// add the object to the model
	mdl.AddObject(po);

	return true;
}

void ModelBuilder::build_nodes(AbaqusModel::PART* pg, FSMesh* pm, const Transform& T, int& n)
{
	AbaqusModel::Tnode_itr pn = pg->m_Node.begin();
	for (int i = 0; i < pg->Nodes(); ++i, ++pn, ++n)
	{
		vec3d r0 = vec3d(pn->x, pn->y, pn->z);
		FSNode& node = pm->Node(n);
		node.r = T.LocalToGlobal(r0);
		pn->lid = n;
		node.SetID(n + 1);
	}
}

void ModelBuilder::build_elems(AbaqusModel::PART* part, FSMesh* pm, int& m)
{
	AbaqusModel::Telem_itr pe = part->m_Elem.begin();
	for (int i = 0; i < part->Elements(); ++i, ++pe)
	{
		if (pe->id != -1)
		{
			FSElement& el = pm->Element(m);
			int etype = pe->type;
			el.SetType(etype);
			el.m_gid = 0;
			pe->lid = m;
			int ne = el.Nodes();
			int* en = pe->n;
			auto pn = part->m_Node.begin();
			for (int j = 0; j < ne; ++j)
			{
				AbaqusModel::Tnode_itr pn = part->FindNode(en[j]);
				el.m_node[j] = pn->lid;
			}
			m++;
		}
	}
}

void ModelBuilder::partition_elems(AbaqusModel::PART* pg, FSMesh* pm)
{
	if (pg->m_Solid.empty() && pg->m_Shell.empty())
	{
		// assign all elements to the same part ID
		int gid = (int)m_partitions.size();
		if (!pg->m_Elem.empty())
		{
			for (auto& el : pg->m_Elem)
			{
				pm->Element(el.lid).m_gid = gid;
			}
			m_partitions.push_back({ pg->m_name, "" });
		}
	}
	else if (pg->m_Solid.empty() == false)
	{
		// partition based on solid sections
		for (auto& section : pg->m_Solid)
		{
			AbaqusModel::ELEMENT_SET* eset = pg->FindElementSet(section.elset);
			if (eset)
			{
				int gid = (int)m_partitions.size();
				vector<int> elem = get_element_indices(eset);
				for (int n : elem) pm->Element(n).m_gid = gid;
				m_partitions.push_back({ pg->m_name + "_" + eset->name, section.mat });
			}
		}
	}
	else if (pg->m_Solid.empty() == false)
	{
		// partition based on solid sections
		for (auto& section : pg->m_Shell)
		{
			AbaqusModel::ELEMENT_SET* eset = pg->FindElementSet(section.elset);
			if (eset)
			{
				int gid = (int)m_partitions.size();
				vector<int> elem = get_element_indices(eset);
				for (int n : elem) pm->Element(n).m_gid = gid;
				m_partitions.push_back({ pg->m_name + "_" + eset->name, section.mat });
			}
		}
	}
}

void ModelBuilder::build_nodesets(AbaqusModel::PART& part, FSMesh* pm)
{
	if (part.m_NSet.empty()) return;
	int nsets = (int)part.m_NSet.size();
	auto ns = part.m_NSet.begin();
	for (int i = 0; i < nsets; ++i, ++ns)
	{
		AbaqusModel::NODE_SET& nset = *ns;

		if (!nset.binternal)
		{
			FSNodeSet* pns = build_nodeset(nset, pm);
			if (pns) pm->AddFENodeSet(pns);
		}
	}
}

FSNodeSet* ModelBuilder::build_nodeset(AbaqusModel::NODE_SET& ns, FSMesh* pm)
{
	if (pm == nullptr) return nullptr;

	AbaqusModel::PART* pg = ns.part;
	if (ns.instance.empty() == false)
	{
		AbaqusModel::INSTANCE* inst = m_inp->FindInstance(ns.instance); assert(inst);
		if (inst == nullptr) pg = nullptr;
		else pg = inst->GetPart();
	}

	if (pg)
	{
		FSNodeSet* fsnset = new FSNodeSet(pm);

		string name = ns.name;
		if (ns.part && !ns.part->m_name.empty())
		{
			name = ns.part->m_name + "." + ns.name;
		}
		fsnset->SetName(name);

		auto it = ns.node.begin();
		int nn = (int)ns.node.size();
		for (int j = 0; j < nn; ++j, ++it)
		{
			int lid = pg->FindNode(*it)->lid;
			fsnset->add(lid);
		}

		m_nodeset_map[&ns] = fsnset;

		return fsnset;
	}
	return nullptr;
}

void ModelBuilder::build_surfaces(AbaqusModel::PART* pg, FSMesh* pm)
{
	AbaqusModel::PART& part = *pg;
	int surfs = (int)part.m_Surf.size();
	if (surfs)
	{
		auto si = part.m_Surf.begin();
		for (int i = 0; i < surfs; ++i, ++si)
		{
			FSSurface* ps = build_surface(*si, pm);
			if (ps)
			{
				string name = si->name;
				if (si->part && !si->part->m_name.empty())
				{
					name = si->part->m_name + "." + si->name;
				}
				ps->SetName(name);
				pm->AddFESurface(ps);
			}
		}
	}
}

FSSurface* ModelBuilder::build_surface(AbaqusModel::SURFACE& surf, FSMesh* pm)
{
	if (pm == nullptr) return nullptr;

	AbaqusModel::PART* pg = surf.part;
	if (!surf.instance.empty())
	{
		pg = nullptr;
		AbaqusModel::INSTANCE* inst = m_inp->FindInstance(surf.instance); assert(inst);
		if (inst) pg = inst->GetPart();
	}
	if (pg == nullptr) return nullptr;

	FSSurface* ps = nullptr;

	if (surf.type == AbaqusModel::ST_ELEMENT)
	{
		ps = new FSSurface(pm);
		for (auto it : surf.set)
		{
			string ref = it.first;
			int sn = it.second;

			// assume ref is an element set name
			vector<AbaqusModel::ELEMENT_SET*> esets = pg->FindElementSets(ref);
			for (auto eset : esets)
			{
				vector<int> elem = get_element_indices(eset);
				for (int i = 0; i < elem.size(); ++i)
				{
					FSElement& el = pm->Element(elem[i]);

					if (el.IsType(FE_HEX8))
					{
						int n = -1;
						switch (sn)
						{
						case 1: n = el.m_face[4]; break;
						case 2: n = el.m_face[5]; break;
						case 3: n = el.m_face[0]; break;
						case 4: n = el.m_face[1]; break;
						case 5: n = el.m_face[2]; break;
						case 6: n = el.m_face[3]; break;
						}
						assert(n >= 0);
						ps->add(n);
					}
					else if (el.IsType(FE_PENTA6))
					{
						int n = -1;
						switch (sn)
						{
						case 1: n = el.m_face[3]; break;
						case 2: n = el.m_face[4]; break;
						case 3: n = el.m_face[0]; break;
						case 4: n = el.m_face[1]; break;
						case 5: n = el.m_face[2]; break;
						}
						assert(n >= 0);
						ps->add(n);
					}
					else if (el.IsType(FE_TET4))
					{
						int n = -1;
						switch (sn)
						{
						case 1: n = el.m_face[3]; break;
						case 2: n = el.m_face[0]; break;
						case 3: n = el.m_face[1]; break;
						case 4: n = el.m_face[2]; break;
						}
						assert(n >= 0);
						ps->add(n);
					}
					else if (el.IsType(FE_TET10))
					{
						int n = -1;
						switch (sn)
						{
						case 1: n = el.m_face[3]; break;
						case 2: n = el.m_face[0]; break;
						case 3: n = el.m_face[1]; break;
						case 4: n = el.m_face[2]; break;
						}
						assert(n >= 0);
						ps->add(n);
					}
					else if (el.IsType(FE_QUAD4))
					{
						int n = el.m_face[0];
						ps->add(n);
					}
					else if (el.IsType(FE_QUAD8))
					{
						int n = el.m_face[0];
						ps->add(n);
					}
					else if (el.IsType(FE_QUAD9))
					{
						int n = el.m_face[0];
						ps->add(n);
					}
					else if (el.IsType(FE_TRI3))
					{
						int n = el.m_face[0];
						ps->add(n);
					}
					else if (el.IsType(FE_TRI6))
					{
						int n = el.m_face[0];
						ps->add(n);
					}
				}
			}
		}
	}
	else if (surf.type == AbaqusModel::ST_NODE)
	{
		pm->TagAllNodes(0);
		for (auto it : surf.set)
		{
			string ref = it.first;
			AbaqusModel::NODE_SET* ps = m_inp->FindNodeSet(ref);
			if (ps)
			{
				vector<int> node = get_nodal_indices(ps);
				for (int n : node) pm->Node(n).m_ntag = 1;
			}
		}

		vector<int> face_list;
		for (int i = 0; i < pm->Faces(); ++i)
		{
			FSFace& face = pm->Face(i);
			int nf = face.Nodes();
			bool incl = true;
			for (int j = 0; j < nf; ++j)
			{
				if (pm->Node(face.n[j]).m_ntag == 0)
				{
					incl = false;
					break;
				}
			}

			if (incl) face_list.push_back(i);
		}

		if (!face_list.empty())
		{
			ps = new FSSurface(pm);
			ps->add(face_list);
		}
	}

	if (ps) m_surface_map[&surf] = ps;

	return ps;
}

vector<int> ModelBuilder::get_element_indices(AbaqusModel::ELEMENT_SET* pg)
{
	std::vector<int> elem;
	int n = (int)pg->elem.size();
	vector<int>::iterator pe = pg->elem.begin();

	AbaqusModel::PART* part = pg->part;
	if (!pg->instance.empty())
	{
		part = nullptr;
		AbaqusModel::INSTANCE* inst = m_inp->FindInstance(pg->instance); assert(inst);
		if (inst) part = inst->GetPart();
	}
	if (part == nullptr) return elem;

	for (int j = 0; j < n; ++j, ++pe)
	{
		AbaqusModel::Telem_itr it = part->FindElement(*pe);
		if (it != part->m_Elem.end())
			elem.push_back(it->lid);
	}
	return elem;
}

vector<int> ModelBuilder::get_nodal_indices(AbaqusModel::NODE_SET* pg)
{
	std::vector<int> node;
	int n = (int)pg->node.size();
	vector<int>::iterator pe = pg->node.begin();

	AbaqusModel::PART* part = pg->part;
	if (!pg->instance.empty())
	{
		part = nullptr;
		AbaqusModel::INSTANCE* inst = m_inp->FindInstance(pg->instance); assert(inst);
		if (inst) part = inst->GetPart();
	}
	if (part == nullptr) return node;

	for (int j = 0; j < n; ++j, ++pe)
	{
		AbaqusModel::Tnode_itr it = part->FindNode(*pe);
		if (it != part->m_Node.end())
			node.push_back(it->lid);
	}
	return node;
}

void ModelBuilder::build_elemsets(AbaqusModel::PART& part, FSMesh* pm)
{
	if (part.m_ESet.empty()) return;

	int elsets = (int)part.m_ESet.size();
	auto it = part.m_ESet.begin();
	for (int i = 0; i < elsets; ++i, ++it)
	{
		AbaqusModel::ELEMENT_SET& es = *it;

		if (!es.binternal)
		{
			AbaqusModel::PART* pg = &part;
			if (!es.instance.empty())
			{
				AbaqusModel::INSTANCE* inst = m_inp->FindInstance(es.instance); assert(inst);
				if (inst == nullptr) pg = nullptr;
				else pg = inst->GetPart();
			}

			if (pg)
			{
				FSElemSet* fseset = new FSElemSet(pm);

				string name = es.name;
				if (es.part && !es.part->m_name.empty())
				{
					name = es.part->m_name + "." + es.name;
				}
				fseset->SetName(name);

				int n = (int)es.elem.size();
				vector<int>::iterator pe = es.elem.begin();
				for (int j = 0; j < n; ++j, ++pe) fseset->add(pg->FindElement(*pe)->lid);

				pm->AddFEElemSet(fseset);
			}
		}
	}
}

bool ModelBuilder::BuildPhysics(AbaqusModel& inp, FSModel& fem)
{
	build_materials(inp, fem);

	GObject* po = fem.GetModel().Object(0);
	FSMesh* pm = po->GetFEMesh();

	// build springs
	build_springs(inp.GlobalPart(), fem);
	for (auto p : m_inp->PartList()) build_springs(*p, fem);

	// assign distributions
	assign_distribution(m_inp->GlobalPart(), pm);
	for (auto p : m_inp->PartList()) assign_distribution(*p, pm);

	// add the amplitude curves
	for (int n = 0; n < inp.Amplitudes(); ++n)
	{
		const AbaqusModel::Amplitude& amp = inp.GetAmplitude(n);
		LoadCurve lc;
		lc.Clear();
		for (int i = 0; i < amp.m_points.size(); ++i)
		{
			vec2d p = amp.m_points[i];
			lc.Add(p.x(), p.y());
		}
		if (amp.m_type == AbaqusModel::Amplitude::AMP_TABULAR) lc.SetInterpolator(LoadCurve::LINEAR);
		if (amp.m_type == AbaqusModel::Amplitude::AMP_SMOOTH_STEP) lc.SetInterpolator(LoadCurve::SMOOTH_STEP);
		FSLoadController* plc = fem.AddLoadCurve(lc);
		plc->SetName(amp.m_name);
	}

	// add the steps
	build_steps(inp, fem, pm);

	return true;
}

void ModelBuilder::build_materials(AbaqusModel& inp, FSModel& fem)
{
	// add the materials
	list<AbaqusModel::MATERIAL>& Mat = inp.MaterialList();
	list<AbaqusModel::MATERIAL>::iterator pm = Mat.begin();
	for (int i = 0; i < (int)Mat.size(); ++i, ++pm)
	{
		FSMaterial* pmat = 0;
		switch (pm->mattype)
		{
		case AbaqusModel::ELASTIC:
			if (pm->ntype == 1)
			{
				pmat = new FSIsotropicElastic(&fem);
				pmat->SetFloatValue(FSIsotropicElastic::MP_DENSITY, pm->dens);
				pmat->SetFloatValue(FSIsotropicElastic::MP_E, pm->d[0]);
				pmat->SetFloatValue(FSIsotropicElastic::MP_v, pm->d[1]);
			}
			break;
		case AbaqusModel::ANI_HYPERELASTIC:
			pmat = new FSTransMooneyRivlin(&fem);
			pmat->SetFloatValue(FSTransMooneyRivlin::MP_DENSITY, pm->dens);
			break;
		case AbaqusModel::HYPERELASTIC:
			if (pm->ntype == 1)
			{
				pmat = new FSIncompNeoHookean(&fem);
				pmat->SetFloatValue(FSIncompNeoHookean::MP_DENSITY, pm->dens);
				pmat->SetFloatValue(FSIncompNeoHookean::MP_G, 2.0 * pm->d[0]);
				pmat->SetFloatValue(FSIncompNeoHookean::MP_K, 1.0 / pm->d[1]);
			}
			else if (pm->ntype == 2)
			{
				pmat = new FSOgdenMaterial(&fem);
				if (pm->nparam == 3)
				{
					pmat->SetFloatValue(FSOgdenMaterial::MP_C1, pm->d[0] * 2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M1, pm->d[1]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_K, 1.0 / pm->d[2]);
				}
				else if (pm->nparam == 6)
				{
					pmat->SetFloatValue(FSOgdenMaterial::MP_C1, pm->d[0] * 2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M1, pm->d[1]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_C2, pm->d[2] * 2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M2, pm->d[3]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_K, 1.0 / pm->d[4]);
				}
				else if (pm->nparam == 9)
				{
					pmat->SetFloatValue(FSOgdenMaterial::MP_C1, pm->d[0] * 2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M1, pm->d[1]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_C2, pm->d[2] * 2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M2, pm->d[3]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_C3, pm->d[4] * 2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M3, pm->d[5]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_K, 1.0 / pm->d[6]);
				}
			}
			break;
//		default:
//			assert(false);
		}

		if (pmat)
		{
			GMaterial* pgm = new GMaterial(pmat);
			pgm->SetName(pm->name);
			fem.AddMaterial(pgm);
		}
	}

	// assign materials to parts
	GObject* po = fem.GetModel().Object(0);
	if (m_partitions.size() == po->Parts())
	{
		for (int i = 0; i < po->Parts(); ++i)
		{
			string mat = m_partitions[i].second;
			GMaterial* pmat = fem.FindMaterial(mat);
			if (pmat) po->Part(i)->SetMaterialID(pmat->GetID());
		}
	}
}

void ModelBuilder::assign_distribution(AbaqusModel::PART& part, FSMesh* pm)
{
	for (auto& it : part.m_Solid)
	{
		AbaqusModel::SOLID_SECTION& solidSection = it;
		AbaqusModel::Orientation* orient = part.FindOrientation(solidSection.orient);
		if (orient)
		{
			AbaqusModel::Distribution* dist = part.FindDistribution(orient->dist);
			if (dist)
			{
				// skip the first entry
				for (int j = 1; j < dist->m_data.size(); ++j)
				{
					AbaqusModel::Distribution::ENTRY& e = dist->m_data[j];
					int eid = part.FindElement(e.elem)->lid;

					vec3d a = vec3d(e.val[0], e.val[1], e.val[2]);
					vec3d b = vec3d(e.val[3], e.val[4], e.val[5]);
					FSElement& el = pm->Element(eid);

					vec3d e1 = a; e1.Normalize();
					vec3d e3 = a ^ b; e3.Normalize();
					vec3d e2 = e3 ^ e1;

					mat3d Q;
					Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
					Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
					Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

					el.m_Qactive = true;
					el.m_Q = Q;
				}
			}
		}
	}

	for (auto& it : part.m_Shell)
	{
		AbaqusModel::SHELL_SECTION& shellSection = it;
		AbaqusModel::Orientation* orient = part.FindOrientation(shellSection.orient);
		if (orient)
		{
			AbaqusModel::Distribution* dist = part.FindDistribution(orient->dist);
			if (dist)
			{
				// skip the first entry
				for (int j = 1; j < dist->m_data.size(); ++j)
				{
					AbaqusModel::Distribution::ENTRY& e = dist->m_data[j];
					int eid = part.FindElement(e.elem)->lid;

					vec3d a = vec3d(e.val[0], e.val[1], e.val[2]);
					vec3d b = vec3d(e.val[3], e.val[4], e.val[5]);
					FSElement& el = pm->Element(eid);

					vec3d e1 = a; e1.Normalize();
					vec3d e3 = a ^ b; e3.Normalize();
					vec3d e2 = e3 ^ e1;

					mat3d Q;
					Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
					Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
					Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

					el.m_Qactive = true;
					el.m_Q = Q;
				}
			}
		}

	}
}

void ModelBuilder::build_springs(AbaqusModel::PART& part, FSModel& fem)
{
	GModel& mdl = fem.GetModel();

	GMeshObject* po = dynamic_cast<GMeshObject*>(mdl.Object(0));
	if (po == nullptr) return;

	if (!part.m_Spring.empty())
	{
		int NS = (int)part.m_Spring.size();
		for (auto& s : part.m_Spring)
		{
			AbaqusModel::SpringSet* set = part.FindSpringSet(s.elset.c_str());
			if (set)
			{
				int nsize = (int)set->m_Elem.size();
				GDiscreteSpringSet* dis = new GDiscreteSpringSet(&mdl);
				dis->SetName(s.elset);

				auto& springs = set->m_Elem;
				for (int i = 0; i < springs.size(); ++i)
				{
					AbaqusModel::SPRING_ELEMENT& el = springs[i];

					int n0 = get_nodal_id(part, el.n[0]);
					int n1 = get_nodal_id(part, el.n[1]);
					if ((n0 >= 0) && (n1 >= 0))
					{
						n0 = po->MakeGNode(n0);
						n1 = po->MakeGNode(n1);

						GNode* gn0 = po->FindNode(n0);
						GNode* gn1 = po->FindNode(n1);

						dis->AddElement(gn0, gn1);
					}
				}

				if (s.nonlinear)
				{
					FSDiscreteMaterial* dmat = FEBio::CreateDiscreteMaterial("nonlinear spring", &fem);
					dis->SetMaterial(dmat);

					FSProperty* prop = dmat->FindProperty("force");
					if (prop)
					{
						FSFunction1D* plc = FEBio::CreateFunction1D("point", &fem);
						prop->SetComponent(plc);

						Param* p = plc->GetParam("points");
						if (p)
						{
							LoadCurve& lc = s.m_lc;
							std::vector<vec2d> v = lc.GetPoints();
							p->SetVectorVec2dValue(v);
						}
					}
				}
				else
				{
					FSDiscreteMaterial* dmat = FEBio::CreateDiscreteMaterial("linear spring", &fem);
					dis->SetMaterial(dmat);

					Param* p = dmat->GetParam("E");
					p->SetFloatValue(s.k);
				}

				fem.GetModel().AddDiscreteObject(dis);
			}
		}
	}
}

void ModelBuilder::build_steps(AbaqusModel& inp, FSModel& fem, FSMesh* pm)
{
	build_step(inp.GetInitStep(), fem, pm, fem.GetStep(0));

	list<AbaqusModel::STEP>& Step = inp.StepList();
	for (list<AbaqusModel::STEP>::iterator it = Step.begin(); it != Step.end(); ++it)
	{
		AbaqusModel::STEP& stepi = *it;
		FSNonLinearMechanics* festep = new FSNonLinearMechanics(&fem);
		STEP_SETTINGS& set = festep->GetSettings();
		set.dt = stepi.dt0;
		set.ntime = (int)(stepi.time / stepi.dt0 + 0.5);
		festep->SetName(stepi.name);
		fem.AddStep(festep);

		build_step(stepi, fem, pm, festep);
	}
}

void ModelBuilder::build_step(AbaqusModel::STEP& step, FSModel& fem, FSMesh* pm, FSStep* festep)
{
	if (festep == nullptr) return;

	// add all boundary conditions
	list<AbaqusModel::BOUNDARY>& BCs = step.BoundaryConditionList();
	list<AbaqusModel::BOUNDARY>::iterator bci;
	int n = 0;
	for (bci = BCs.begin(); bci != BCs.end(); ++bci)
	{
		AbaqusModel::BOUNDARY& bc = *bci;

		int ns = (int)bc.m_nodeSet.size();
		for (int i = 0; i < ns; ++i, ++n)
		{
			AbaqusModel::BOUNDARY::NSET& bns = bc.m_nodeSet[i];

			FSNodeSet* nset = nullptr;
			AbaqusModel::NODE_SET* ns = m_inp->FindNodeSet(bns.nset);
			if (ns)
			{
				if (m_nodeset_map.find(ns) != m_nodeset_map.end())
				{
					nset = m_nodeset_map[ns];
				}
				else
				{
					nset = build_nodeset(*ns, pm);
					if (nset) pm->AddFENodeSet(nset);
				}
			}

			if (bc.m_ampl.empty() && (bns.ndof[0] <= 3))
			{
				if (bns.val == 0.0)
				{
					int dof = (1 << (bns.ndof[0] - 1));
					FSFixedDisplacement* pbc = new FSFixedDisplacement(&fem, nset, dof);
					char szname[256] = { 0 };
					sprintf(szname, "bc_%d", n);
					pbc->SetName(szname);
					festep->AddComponent(pbc);
				}
				else
				{
					FSPrescribedDisplacement* pbc = new FSPrescribedDisplacement(&fem, nset, bns.ndof[0] - 1, bns.val);
					char szname[256] = { 0 };
					sprintf(szname, "bc_%d", n);
					pbc->SetName(szname);
					festep->AddComponent(pbc);
				}
			}
			else if (!bc.m_ampl.empty())
			{
				FSPrescribedDisplacement* pbc = new FSPrescribedDisplacement(&fem, nset, bns.ndof[0] - 1, bns.val);
				char szname[256] = { 0 };
				sprintf(szname, "bc_%d", n);
				pbc->SetName(szname);

				int lc = m_inp->FindAmplitude(bc.m_ampl);
				if (lc >= 0)
				{
					FSLoadController* plc = fem.GetLoadController(lc);
					if (plc)
					{
						Param& p = pbc->GetParam(FSPrescribedDOF::SCALE);
						p.SetLoadCurveID(plc->GetID());
					}
				}
				festep->AddComponent(pbc);
			}
		}
	}

	// add concentrated loads
	list<AbaqusModel::CLOAD>& CLoads = step.CLoadList();
	for (auto& it : CLoads)
	{
		int lcid = -1;
		int lc = m_inp->FindAmplitude(it.ampl);
		if (lc >= 0)
		{
			FSLoadController* plc = fem.GetLoadController(lc);
			if (plc) lcid = plc->GetID();
		}

		for (auto& cns : it.nset)
		{
			FSNodeSet* nset = nullptr;
			AbaqusModel::NODE_SET* ns = m_inp->FindNodeSet(cns.nset);
			if (ns)
			{
				if (m_nodeset_map.find(ns) != m_nodeset_map.end())
				{
					nset = m_nodeset_map[ns];
				}
				else
				{
					nset = build_nodeset(*ns, pm);
					if (nset) pm->AddFENodeSet(nset);
				}
			}

			FSNodalDOFLoad* pfc = new FSNodalDOFLoad(&fem, nset, cns.ndof - 1, cns.val);
			char szname[256] = { 0 };
			sprintf(szname, "cload_%d", n);
			pfc->SetName(szname);

			if (lcid >= 0)
			{
				Param& p = pfc->GetParam(FSNodalDOFLoad::LOAD);
				p.SetLoadCurveID(lcid);
			}

			festep->AddComponent(pfc);
		}
	}

	// add all surface loads
	list<AbaqusModel::DSLOAD>& SLoads = step.SurfaceLoadList();
	list<AbaqusModel::DSLOAD>::iterator sl;
	n = 0;
	for (sl = SLoads.begin(); sl != SLoads.end(); ++sl)
	{
		AbaqusModel::DSLOAD& p = *sl;

		int ns = (int)p.m_surf.size();
		for (int i = 0; i < ns; ++i, ++n)
		{
			AbaqusModel::SURFACE* ps = m_inp->FindSurface(sl->m_surf[i].surf);
			FSSurface* surface = nullptr;
			if (ps && (m_surface_map.find(ps) != m_surface_map.end())) surface = m_surface_map[ps];

			FSPressureLoad* pl = new FSPressureLoad(&fem, surface);
			char szname[256] = { 0 };
			sprintf(szname, "dsload_%d", n);
			pl->SetName(szname);
			pl->SetLoad(p.m_surf[i].load);

			if (p.m_ampl >= 0)
			{
				FSLoadController* plc = fem.GetLoadController(p.m_ampl);
				if (plc)
				{
					Param& p = pl->GetParam(FSPressureLoad::LOAD);
					p.SetLoadCurveID(plc->GetID());
				}
			}

			festep->AddComponent(pl);
		}
	}

	// add all contacts
	n = 0;
	for (int i = 0; i < step.ContactPairs(); ++i, ++n)
	{
		const AbaqusModel::CONTACT_PAIR& cp = step.GetContactPair(i);
		FSTensionCompressionInterface* ci = new FSTensionCompressionInterface(&fem);

		stringstream ss; ss << "contact_pair" << n;
		ci->SetName(ss.str());

		if (cp.surf1.empty() == false)
		{
			FSSurface* surf1 = nullptr;
			AbaqusModel::SURFACE* s1 = m_inp->FindSurface(cp.surf1);
			if (s1)
			{
				auto it = m_surface_map.find(s1);
				if (it != m_surface_map.end()) surf1 = it->second;
			}
			if (surf1) ci->SetPrimarySurface(surf1);
		}

		if (cp.surf2.empty() == false)
		{
			FSSurface* surf2 = nullptr;
			AbaqusModel::SURFACE* s2 = m_inp->FindSurface(cp.surf2);
			if (s2)
			{
				auto it = m_surface_map.find(s2);
				if (it != m_surface_map.end()) surf2 = it->second;
			}
			if (surf2) ci->SetSecondarySurface(surf2);
		}

		ci->SetFloatValue(FSTensionCompressionInterface::FRICCOEFF, cp.friction);

		festep->AddInterface(ci);
	}

	// add all tied contacts
	n = 0;
	for (int i = 0; i < step.Ties(); ++i, ++n)
	{
		const AbaqusModel::TIE& tie = step.GetTie(i);

		FSTiedElasticInterface* ti = new FSTiedElasticInterface(&fem);

		stringstream ss; ss << "tie" << n;
		ti->SetName(ss.str());

		if (tie.surf1.empty() == false)
		{
			FSSurface* surf1 = nullptr;
			AbaqusModel::SURFACE* s1 = m_inp->FindSurface(tie.surf1);
			if (s1)
			{
				auto it = m_surface_map.find(s1);
				if (it != m_surface_map.end()) surf1 = it->second;
			}
			if (surf1) ti->SetPrimarySurface(surf1);
		}

		if (tie.surf2.empty() == false)
		{
			FSSurface* surf2 = nullptr;
			AbaqusModel::SURFACE* s2 = m_inp->FindSurface(tie.surf2);
			if (s2)
			{
				auto it = m_surface_map.find(s2);
				if (it != m_surface_map.end()) surf2 = it->second;
			}
			if (surf2) ti->SetSecondarySurface(surf2);
		}

		festep->AddInterface(ti);
	}

}

int ModelBuilder::get_nodal_id(AbaqusModel::PART& part, std::string& ref)
{
	if (ref.empty()) return -1;

	AbaqusModel::PART* pg = nullptr;
	int node = -1;
	if (isdigit(ref[0]))
	{
		pg = &part;
		node = atoi(ref.c_str());
	}
	else
	{
		size_t l = ref.find('.');
		if (l != string::npos)
		{
			string partName = ref.substr(0, l);
			string nodeRef = ref.substr(l + 1);
			node = atoi(nodeRef.c_str());

			AbaqusModel::INSTANCE* inst = m_inp->FindInstance(partName);
			if (inst) pg = inst->GetPart();
		}
	}

	if (pg)
	{
		AbaqusModel::Tnode_itr pn = pg->FindNode(node);
		if (pn != pg->m_Node.end()) return pn->lid;
	}

	return -1;
}
