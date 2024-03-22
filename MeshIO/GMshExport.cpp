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
#include "GMshExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>

class GMeshOut {
public:
	enum ELEM_TYPE {
		INVALID = 0,
		TET4 = 4,
		HEX8 = 5
	};

public:
	struct NODE {
		int id = -1;
		vec3d r;
	};

	struct ELEM {
		int id;
		int nodes;
		int node[20];
	};

	class BLOCK {
	public:
		void Create(size_t elems, ELEM_TYPE etype) {
			m_Elem.resize(elems);
			m_elemType = etype;
		}

		int ElementType() { return m_elemType; }

		size_t Elems() { return m_Elem.size(); }

		ELEM& Elem(size_t i) { return m_Elem[i]; }

	private:
		int m_elemType = INVALID;
		std::vector<ELEM>	m_Elem;
	};

	class ENTITY {
	public:
		void CreateNodes(size_t nodes) { m_Node.resize(nodes); }

		size_t Nodes() { return m_Node.size(); }
		NODE& Node(size_t i) { return m_Node[i]; }

		void AddBlock(size_t elems, ELEM_TYPE etype)
		{
			BLOCK b; b.Create(elems, etype);
			m_block.push_back(b);
		}

		size_t Blocks() { return m_block.size(); }
		BLOCK& Block(size_t i) { return m_block[i]; }

		void SetExtent(vec3d r0, vec3d r1) {
			m_extent[0] = r0.x; m_extent[1] = r0.y; m_extent[2] = r0.z;
			m_extent[3] = r1.x; m_extent[4] = r1.y; m_extent[5] = r1.z;
		}

		double* GetExtent() { return m_extent; }

		size_t Elements() {
			size_t N = 0;
			for (BLOCK& b : m_block) N += b.Elems();
			return N;
		}

	public:
		void UpdateExtent()
		{
			vec3d r0 = Node(0).r, r1 = Node(0).r;
			for (size_t i = 0; i < Nodes(); ++i)
			{
				vec3d r = Node(i).r;

				if (r.x < r0.x) r0.x = r.x;
				if (r.y < r0.y) r0.y = r.y;
				if (r.z < r0.z) r0.z = r.z;

				if (r.x > r0.x) r1.x = r.x;
				if (r.y > r0.y) r1.y = r.y;
				if (r.z > r0.z) r1.z = r.z;
			}

			SetExtent(r0, r1);
		}

	private:
		double m_extent[6] = { 0 };
		std::vector<NODE>	m_Node;
		std::vector<BLOCK>	m_block;
	};

public:
	GMeshOut() {}

	size_t Entities() { return m_Ent.size(); }

	ENTITY& Entity(size_t i) { return m_Ent[i]; }

	void AddEntity(ENTITY& o) { m_Ent.push_back(o); }

	size_t Nodes()
	{
		size_t N = 0;
		for (ENTITY& o : m_Ent) N += o.Nodes();
		return N;
	}

	size_t Elements()
	{
		size_t N = 0;
		for (ENTITY& o : m_Ent) N += o.Elements();
		return N;
	}

	size_t Blocks()
	{
		size_t N = 0;
		for (ENTITY& o : m_Ent) N += o.Blocks();
		return N;
	}

private:
	std::vector<ENTITY>	m_Ent;
};

GMeshExport::GMeshExport(FSProject& prj) : FSFileExport(prj)
{

}

void BuildGMeshOut(GMeshOut& gmsh, GModel& mdl)
{
	int objs = mdl.Objects();
	int nn = 0;
	int ne = 0;
	for (int n = 0; n < objs; ++n)
	{
		GObject* po = mdl.Object(n);
		FSMesh* mesh = po->GetFEMesh();
		if (mesh)
		{
			int nodes = mesh->Nodes();

			GMeshOut::ENTITY o;
			o.CreateNodes(nodes);
			for (int i = 0; i < nodes; ++i)
			{
				FSNode& node = mesh->Node(i);
				node.m_ntag = nn + 1;
				vec3d r = mesh->LocalToGlobal(node.r);
				o.Node(i).id = nn + 1;
				o.Node(i).r = r;
				nn++;
			}

			// count elements per part
			int NE = mesh->Elements();
			int parts = po->Parts();
			std::vector<int> part(parts, 0);
			std::vector<GMeshOut::ELEM_TYPE> etype(parts);
			for (int i = 0; i < NE; ++i)
			{
				FSElement& el = mesh->Element(i);
				part[el.m_gid]++;
				switch (el.Type())
				{
				case FE_TET4: etype[el.m_gid] = GMeshOut::TET4; break;
				case FE_HEX8: etype[el.m_gid] = GMeshOut::HEX8; break;
				default:
					assert(false);
					break;
				}
			}

			// allocate parts
			for (int i = 0; i < parts; ++i) {
				o.AddBlock(part[i], etype[i]);
				part[i] = 0;
			}

			// assign elements
			GMeshOut::ELEM ge;
			int m[20] = { 0 };
			for (int j = 0; j < NE; ++j)
			{
				FSElement& el = mesh->Element(j);
				for (int n = 0; n < el.Nodes(); ++n) m[n] = mesh->Node(el.m_node[n]).m_ntag;

				el.m_ntag = ne + 1;
				ge.id = ne + 1;

				switch (el.Type())
				{
				case FE_TET4:
					ge.nodes = 4;
					ge.node[0] = m[0]; ge.node[1] = m[1]; ge.node[2] = m[2]; ge.node[3] = m[3];
					break;
				case FE_HEX8:
					ge.nodes = 8;
					ge.node[0] = m[0]; ge.node[1] = m[1]; ge.node[2] = m[2]; ge.node[3] = m[3];
					ge.node[4] = m[4]; ge.node[5] = m[5]; ge.node[6] = m[6]; ge.node[7] = m[7];
					break;
				default:
					assert(false);
					break;
				}

				GMeshOut::BLOCK& b = o.Block(el.m_gid);
				b.Elem(part[el.m_gid]++) = ge;
				ne++;
			}

			gmsh.AddEntity(o);
		}
	}

	// we also need to know the extent of each entity
	for (int i = 0; i < gmsh.Entities(); ++i)
	{
		GMeshOut::ENTITY& o = gmsh.Entity(i);
		o.UpdateExtent();
	}
}

bool ExportGMeshOut(GMeshOut& gmsh, const char* szfile)
{
	// open the file
	FILE* fp = fopen(szfile, "wt");
	if (fp == nullptr) return false;

	// write the header
	fprintf(fp, "$MeshFormat\n");
	fprintf(fp, "4.1 0 %d\n", (int)sizeof(size_t));
	fprintf(fp, "$EndMeshFormat\n");

	// entities
	fprintf(fp, "$Entities\n");
	fprintf(fp, "0 0 0 %d\n", (int)gmsh.Entities());
	for (int i = 0; i < gmsh.Entities(); ++i)
	{
		GMeshOut::ENTITY& o = gmsh.Entity(i);
		double* e = o.GetExtent();
		fprintf(fp, "%d %lg %lg %lg %lg %lg %lg 1 %d 0\n", i + 1, e[0], e[1], e[2], e[3], e[4], e[5], i + 1);
	}
	fprintf(fp, "$EndEntities\n");

	// nodes
	size_t totalNodes = gmsh.Nodes();
	fprintf(fp, "$Nodes\n");
	fprintf(fp, "%d %d %d %d\n", (int)gmsh.Entities(), (int)totalNodes, 1, (int)totalNodes);
	for (int i = 0; i < (int)gmsh.Entities(); ++i)
	{
		GMeshOut::ENTITY& o = gmsh.Entity(i);
		size_t NN = o.Nodes();
		fprintf(fp, "3 %d 0 %d\n", i + 1, (int)NN);

		// nodal IDs
		for (size_t i = 0; i < NN; ++i)
		{
			GMeshOut::NODE& nd = o.Node(i);
			fprintf(fp, "%d\n", nd.id);
		}

		// nodal coordinates
		for (size_t i = 0; i < NN; ++i)
		{
			GMeshOut::NODE& nd = o.Node(i);
			fprintf(fp, "%lg %lg %lg\n", nd.r.x, nd.r.y, nd.r.z);
		}
	}
	fprintf(fp, "$EndNodes\n");

	// elements
	int totalElems = (int)gmsh.Elements();
	fprintf(fp, "$Elements\n");
	fprintf(fp, "%d %d %d %d\n", (int)gmsh.Blocks(), totalElems, 1, totalElems);
	for (int i = 0; i < gmsh.Entities(); ++i)
	{
		GMeshOut::ENTITY& o = gmsh.Entity(i);
		for (int j = 0; j < o.Blocks(); ++j)
		{
			GMeshOut::BLOCK& b = o.Block(j);
			int NE = (int)b.Elems();
			fprintf(fp, "3 %d %d %d\n", i + 1, b.ElementType(), NE);
			for (int l = 0; l < NE; ++l)
			{
				GMeshOut::ELEM& ge = b.Elem(l);
				fprintf(fp, "%d", ge.id);
				for (int n = 0; n < ge.nodes; ++n) fprintf(fp, " %d", ge.node[n]);
				fprintf(fp, "\n");
			}
		}
	}
	fprintf(fp, "$EndElements\n");

	fclose(fp);

	return true;
}

bool GMeshExport::Write(const char* szfile)
{
	// get the GModel
	GModel& mdl = m_prj.GetFSModel().GetModel();
	int objs = mdl.Objects();

	// build the gmesh output structure
	GMeshOut gmsh;
	BuildGMeshOut(gmsh, mdl);

	// export the gmsh structur
	return ExportGMeshOut(gmsh, szfile);
}
