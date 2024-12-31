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

#include "GMshImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <vector>
//using namespace std;

static bool isKey(const char* sz1, const char* sz2)
{
	return (strncmp(sz1, sz2, strlen(sz2)) == 0);
}

class GMeshFormat
{
protected:
	struct ELEMENT
	{
		int ntype;
		int	gid;
		int node[20];
	};

	struct NODE
	{
		int id;
		vec3d	pos;
	};

public:
	GMeshFormat(GMshImport* _gmsh) : gmsh(_gmsh) {}
	virtual ~GMeshFormat() {}

	virtual bool Load() = 0;

	const char* nextLine() { return gmsh->nextLine(); }

	bool readLine(const char* szkey) 
	{ 
		const char* szline = nextLine();
		return isKey(szline, szkey);
	}

	FSMesh* BuildMesh();

protected:
	GMshImport* gmsh = nullptr;

	std::vector<NODE>		m_Node;	// nodal coordinates
	std::vector<ELEMENT>	m_Face;	// surface elements
	std::vector<ELEMENT>	m_Elem;	// volume elements
};

class GMeshLegacyFormat : public GMeshFormat
{
public:
	GMeshLegacyFormat(GMshImport* _gmsh) : GMeshFormat(_gmsh) {}
	bool Load() override;

private:
	bool ReadPhysicalNames();
	bool ReadNodes();
	bool ReadElements();
};

class GMeshNewFormat : public GMeshFormat
{
public:
	GMeshNewFormat(GMshImport* _gmsh) : GMeshFormat(_gmsh) {}
	bool Load() override;

private:
	bool ReadPhysicalNames();
	bool ReadParametrizations();
	bool ReadEntities();
	bool ReadNodes();
	bool ReadElements();
};

//===========================================================================================
FSMesh* GMeshFormat::BuildMesh()
{
	size_t nodes = m_Node.size();
	size_t faces = m_Face.size();
	size_t elems = m_Elem.size();

	FSMesh* pm = new FSMesh;
	pm->Create(nodes, 0);

	// read the nodes
	for (int i = 0; i < nodes; ++i)
	{
		FSNode& node = pm->Node(i);
		node.r = m_Node[i].pos;
	}

	// build th node look-up table
	int minId = m_Node[0].id;
	int maxId = m_Node[0].id;
	for (int i = 1; i < nodes; ++i)
	{
		NODE& n = m_Node[i];
		if (n.id > maxId) maxId = n.id;
		if (n.id < minId) minId = n.id;
	}
	int nltsize = maxId - minId + 1;
	std::vector<int> NLT(nltsize, -1);
	for (int i = 0; i < nodes; ++i)
	{
		NODE& n = m_Node[i];
		NLT[n.id - minId] = i;
	}

	if (elems > 0)
	{
		pm->Create(0, elems, faces);
		for (int i = 0; i < elems; ++i)
		{
			FSElement& el = pm->Element(i);
			ELEMENT& e = m_Elem[i];

			el.m_gid = e.gid;
			el.SetType(e.ntype);
			switch (el.Type())
			{
			case FE_TET4:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				break;
			case FE_HEX8:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				el.m_node[4] = e.node[4];
				el.m_node[5] = e.node[5];
				el.m_node[6] = e.node[6];
				el.m_node[7] = e.node[7];
				break;
			case FE_PYRA5:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				el.m_node[4] = e.node[4];
				break;
			case FE_PENTA6:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				el.m_node[4] = e.node[4];
				el.m_node[5] = e.node[5];
				break;
			case FE_TET10:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				el.m_node[4] = e.node[4];
				el.m_node[5] = e.node[5];
				el.m_node[6] = e.node[6];
				el.m_node[7] = e.node[7];
				el.m_node[8] = e.node[8];
				el.m_node[9] = e.node[9];
				break;
			case FE_TET20:
			{
				for (int i = 0; i < 20; ++i) el.m_node[i] = e.node[i];
			}
			break;
			case FE_PYRA13:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				el.m_node[4] = e.node[4];
				el.m_node[5] = e.node[5];
				el.m_node[6] = e.node[6];
				el.m_node[7] = e.node[7];
				el.m_node[8] = e.node[8];
				el.m_node[9] = e.node[9];
				el.m_node[10] = e.node[10];
				el.m_node[11] = e.node[11];
				el.m_node[12] = e.node[12];
				break;
			default:
				delete pm;
				return nullptr;
			}

			for (int n = 0; n < el.Nodes(); ++n) el.m_node[n] = NLT[el.m_node[n] - minId];
		}

		for (int i = 0; i < faces; ++i)
		{
			FSFace& face = pm->Face(i);
			ELEMENT& e = m_Face[i];

			face.m_gid = e.gid;
			switch (e.ntype)
			{
			case FE_TRI3:
				face.SetType(FE_FACE_TRI3);
				face.n[0] = e.node[0];
				face.n[1] = e.node[1];
				face.n[2] = e.node[2];
				break;
			case FE_QUAD4:
				face.SetType(FE_FACE_QUAD4);
				face.n[0] = e.node[0];
				face.n[1] = e.node[1];
				face.n[2] = e.node[2];
				face.n[3] = e.node[3];
				break;
			case FE_TRI6:
				face.SetType(FE_FACE_TRI6);
				face.n[0] = e.node[0];
				face.n[1] = e.node[1];
				face.n[2] = e.node[2];
				face.n[3] = e.node[3];
				face.n[4] = e.node[4];
				face.n[5] = e.node[5];
				break;
			}

			for (int n = 0; n < face.Nodes(); ++n) face.n[n] = NLT[face.n[n] - minId];
		}
	}
	else if (faces > 0)
	{
		// If no volume elements are defined, we create a shell mesh
		pm->Create(0, faces);
		for (int i = 0; i < faces; ++i)
		{
			FSElement& el = pm->Element(i);
			ELEMENT& e = m_Face[i];

			el.m_gid = e.gid;
			el.SetType(e.ntype);
			switch (el.Type())
			{
			case FE_TRI3:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				break;
			case FE_QUAD4:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				break;
			case FE_TRI6:
				el.m_node[0] = e.node[0];
				el.m_node[1] = e.node[1];
				el.m_node[2] = e.node[2];
				el.m_node[3] = e.node[3];
				el.m_node[4] = e.node[4];
				el.m_node[5] = e.node[5];
				break;
			}

			for (int n = 0; n < el.Nodes(); ++n) el.m_node[n] = NLT[el.m_node[n] - minId];
		}
	}

	return pm;
}

//! \todo PreView has trouble with reading surface elements and volume elements since
//! the surface elements are not shell elements.
GMshImport::GMshImport(FSProject& prj) : FSFileImport(prj)
{
	m_szline[0] = 0;
	m_pfem = 0;
	m_bnewFormat = false;
}

bool GMshImport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// open the file
	if (!Open(szfile, "rt")) return false;

	// get the first line (should be $MeshFormat)
	fgets(m_szline, 255, m_fp);
	if (isKey(m_szline, "$MeshFormat"))
	{
		bool ret = ReadMeshFormat();
		if (ret == false) return false;
	}
	else return errf("This is not a valid GMsh file.");

	// allocate correct format parse
	GMeshFormat* fmt = nullptr;
	if (m_bnewFormat)
	{
		fmt = new GMeshNewFormat(this);
	}
	else
	{
		fmt = new GMeshLegacyFormat(this);
	}
	assert(fmt);

	// load the rest of the file
	if (fmt->Load() == false) return false;

	// close the file
	Close();

	// create a new mesh
	FSMesh* pm = fmt->BuildMesh();
	if (pm == nullptr) return errf("Failed building mesh.");

	// create a new object from this mesh
	pm->RebuildMesh();
	GMeshObject* po = new GMeshObject(pm);

	// give the object the name of the file
	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	// all done!
	return true;
}

//-----------------------------------------------------------------------------
const char* GMshImport::nextLine()
{
	return fgets(m_szline, 255, m_fp);
}

//-----------------------------------------------------------------------------
bool GMshImport::ReadMeshFormat()
{
	// read the format line
	fgets(m_szline, 255, m_fp);
	float fversion;
	int ntype;
	int ngsize;
	int nread = sscanf(m_szline, "%g %d %d", &fversion, &ntype, &ngsize);
	if (nread != 3) return errf("Syntax error on line 2");
	if (ntype != 0) return errf("Invalid file type");
	if (ngsize != 8) return errf("data format must be 8");

	// read the end of the mesh format
	fgets(m_szline, 255, m_fp);
	if (isKey(m_szline, "$EndMeshFormat")==false) return errf("Failed finding EndMeshFormat");

	// chech the file version
	if (fversion >= 4.f) m_bnewFormat = true;
	else m_bnewFormat = false;

	return true;
}

//-----------------------------------------------------------------------------
bool GMeshLegacyFormat::Load()
{
	bool ret = true;
	const char* szline = nullptr;
	while (szline = nextLine())
	{
		if      (isKey(szline, "$PhysicalNames")) ret = ReadPhysicalNames();
		else if (isKey(szline, "$Nodes"        )) ret = ReadNodes();
		else if (isKey(szline, "$Elements"     )) ret = ReadElements();
		else
		{
			// we didn't recognize the section
			return gmsh->errf("Unknown section: %s", szline);
		}

		if (ret == false) return false;
	}

	return true;
}
 
//-----------------------------------------------------------------------------
bool GMeshLegacyFormat::ReadPhysicalNames()
{
	// read the number of physical names
	const char* szline = nullptr;
	if ((szline = nextLine()) == 0) return gmsh->errf("Unexpected end of file");
	int names = 0;
	int nread = sscanf(szline, "%d", &names);
	if (nread != 1) return gmsh->errf("Error while reading physical names");

	// read the names
	for (int i=0; i<names; ++i)
	{
		szline = nextLine();
		if (szline == 0) return gmsh->errf("Unexpected end of file");
		char szname[128] = {0};
		int n, nread;
		nread = sscanf(szline, "%*d%d%s", &n, szname);
		if (nread != 2) return false;
	}

	// read the end tag
	if (readLine("$EndPhysicalNames") == false) return gmsh->errf("Failed reading $EndPhysicalNames");

	return true;
}

//-----------------------------------------------------------------------------

bool GMeshLegacyFormat::ReadNodes()
{
	const char* szline = nextLine();
	int nodes = 0;
	int nread = sscanf(szline, "%d", &nodes);
	if (nread != 1) return gmsh->errf("Error while reading Nodes section");

	// allocate nodes
	m_Node.resize(nodes);

	// read the nodes
	for (int i=0; i<nodes; ++i)
	{
		NODE& n = m_Node[i];
		vec3d& r = n.pos;

		szline = nextLine();
		int nread = sscanf(szline, "%d %lg %lg %lg", &n.id, &r.x, &r.y, &r.z);
		if (nread != 4) return gmsh->errf("Error while reading Nodes section");
	}

	// read the end of the mesh format
	if (readLine("$EndNodes") == false) return gmsh->errf("Failed finding EndNodes");

	return true;
}

//-----------------------------------------------------------------------------

int scan_line(const char* sz, int* data, int nmax)
{
	if ((sz == 0) || sz[0] == 0) return 0;
	if (nmax == 0) return 0;
	int n = 0;
	while (*sz)
	{
		while (*sz && isspace(*sz)) sz++;
		if (*sz)
		{
			data[n++] = atoi(sz);
			while (*sz && (isspace(*sz) == 0)) sz++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
bool GMeshLegacyFormat::ReadElements()
{
	const char* szline = nextLine();
	int elems = 0;
	int nread = sscanf(szline, "%d", &elems);
	if (nread != 1) return gmsh->errf("Error while reading Element section");

	m_Face.reserve(elems);
	m_Elem.reserve(elems);

	// read the elements
	ELEMENT el;
	const int NMAX = 30;
	int n[NMAX];

	for (int i=0; i<elems; ++i)
	{
		szline = nextLine();
		int nread = scan_line(szline, n, NMAX);

		// element type
		int etype = n[1];

		// number of tags
		int ntags = n[2];
		if (ntags == 0) ntags = 2;	// According to the documentation GLMesh files should have at least two tags

		// TODO: The first tag stores the physical entity, which I think is a user-partition
		// and requires the physical entities to be defined in the file. The second tag
		// is the model entity and I think is more similar to the partitioning we use. 
		// Perhaps in the future we can use the physical entities to create user-partitions
		el.gid = (ntags > 0? n[4] : 0);

		// store pointer to node numbers (depends on tags)
		int* m = n + ntags + 3;

		// surface elements
		switch (etype)
		{
		case 2: // triangle
			if (nread != 3 + ntags + 3) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_TRI3;
			el.node[0] = m[0];
			el.node[1] = m[1];
			el.node[2] = m[2];
			m_Face.push_back(el);
			break;
		case 3: // quad
			if (nread != 3 + ntags + 4) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_QUAD4;
			el.node[0] = m[0];
			el.node[1] = m[1];
			el.node[2] = m[2];
			el.node[3] = m[3];
			m_Face.push_back(el);
			break;
		}

		// volume elements
		switch (etype)
		{
		case 4: // tetrahedron
			if (nread != 3 + ntags + 4) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_TET4;
			el.node[0] = m[0];
			el.node[1] = m[1];
			el.node[2] = m[2];
			el.node[3] = m[3];
			m_Elem.push_back(el);
			break;
		case 5: // hexahedrons
			if (nread != 3 + ntags + 8) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_HEX8;
			el.node[0] = m[0];
			el.node[1] = m[1];
			el.node[2] = m[2];
			el.node[3] = m[3];
			el.node[4] = m[4];
			el.node[5] = m[5];
			el.node[6] = m[6];
			el.node[7] = m[7];
			m_Elem.push_back(el);
			break;
		case 6: // pentahedron
			if (nread != 3 + ntags + 6) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_PENTA6;
			el.node[0] = m[0];
			el.node[1] = m[1];
			el.node[2] = m[2];
			el.node[3] = m[3];
			el.node[4] = m[4];
			el.node[5] = m[5];
			m_Elem.push_back(el);
			break;
		case 7: // 5-node pyramid
			if (nread != 3 + ntags + 5) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_PYRA5;
			el.node[0] = m[0];
			el.node[1] = m[1];
			el.node[2] = m[2];
			el.node[3] = m[3];
			el.node[4] = m[4];
			m_Elem.push_back(el);
			break;
        case 11: // 10-node tetrahedron
            if (nread != 3 + ntags + 10) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
            el.ntype = FE_TET10;
            el.node[0] = m[0];
            el.node[1] = m[1];
            el.node[2] = m[2];
            el.node[3] = m[3];
            el.node[4] = m[4];
            el.node[5] = m[5];
            el.node[6] = m[6];
            el.node[7] = m[7];
            el.node[8] = m[9];
            el.node[9] = m[8];
			m_Elem.push_back(el);
            break;
        case 14: // 14-node pyramid
            {
                if (nread != 3 + ntags + 14) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
                el.ntype = FE_PYRA13;
                el.node[0] = m[0];
                el.node[1] = m[1];
                el.node[2] = m[2];
                el.node[3] = m[3];
                el.node[4] = m[4];
                el.node[ 5] = m[ 5];
                el.node[ 6] = m[ 8];
                el.node[ 7] = m[10];
                el.node[ 8] = m[ 6];
                el.node[ 9] = m[ 7];
                el.node[10] = m[ 9];
                el.node[11] = m[11];
                el.node[12] = m[12];
				m_Elem.push_back(el);
            }
                break;
        case 19: // 13-node pyramid
            {
                if (nread != 3 + ntags + 13) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
                el.ntype = FE_PYRA13;
                el.node[0] = m[0];
                el.node[1] = m[1];
                el.node[2] = m[2];
                el.node[3] = m[3];
                el.node[4] = m[4];
                el.node[ 5] = m[ 5];
                el.node[ 6] = m[ 8];
                el.node[ 7] = m[10];
                el.node[ 8] = m[ 6];
                el.node[ 9] = m[ 7];
                el.node[10] = m[ 9];
                el.node[11] = m[11];
                el.node[12] = m[12];
				m_Elem.push_back(el);
            }
            break;
		case 29: // 20-tet tehrahedron
			{
				if (nread != 3 + ntags + 20) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_TET20;
				el.node[ 0] = m[ 0];
				el.node[ 1] = m[ 1];
				el.node[ 2] = m[ 2];
				el.node[ 3] = m[ 3];
				el.node[ 4] = m[ 4];
				el.node[ 5] = m[ 5];
				el.node[ 6] = m[ 6];
				el.node[ 7] = m[ 7];
				el.node[ 8] = m[ 9];
				el.node[ 9] = m[ 8];
				el.node[10] = m[11];
				el.node[11] = m[10];
				el.node[12] = m[15];
				el.node[13] = m[14];
				el.node[14] = m[13];
				el.node[15] = m[12];
				el.node[16] = m[17];
				el.node[17] = m[19];
				el.node[18] = m[18];
				el.node[19] = m[16];

				m_Elem.push_back(el);
			}
			break;
		}
	}

	int nfaces = (int) m_Face.size();
	int nelems = (int) m_Elem.size();

	// make sure all gids start from 0
	if (nelems > 0)
	{
		int minid = m_Elem[0].gid;
		for (int i=0; i<nelems; ++i)
			if (m_Elem[i].gid < minid) minid = m_Elem[i].gid;

		if (minid != 0)
		{
			for (int i=0; i<nelems; ++i) m_Elem[i].gid -= minid;
		}
	}

	// make sure all gids start from 0
	if (nfaces > 0)
	{
		int minid = m_Face[0].gid;
		for (int i=0; i<nfaces; ++i)
			if (m_Face[i].gid < minid) minid = m_Face[i].gid;

		if (minid != 0)
		{
			for (int i=0; i<nfaces; ++i) m_Face[i].gid -= minid;
		}
	}
	
	// read the end of the mesh format
	if (readLine("$EndElements") == false) return gmsh->errf("Failed finding EndElements");

	return true;
}

//===========================================================================================
bool GMeshNewFormat::Load()
{
	bool ret = true;
	const char* szline = nullptr;
	while (szline = nextLine())
	{
		if      (isKey(szline, "$PhysicalNames"   )) ret = ReadEntities();
		else if (isKey(szline, "$Entities"        )) ret = ReadEntities();
		else if (isKey(szline, "$Nodes"           )) ret = ReadNodes();
		else if (isKey(szline, "$Elements"        )) ret = ReadElements();
		else if (isKey(szline, "$Parametrizations")) ret = ReadParametrizations();
		else
		{
			// we didn't recognize the section
			return gmsh->errf("Unknown section: %s", szline);
		}

		if (ret == false) return false;
	}

	return true;
}

bool GMeshNewFormat::ReadPhysicalNames()
{
	while (readLine("$EndPhysicalNames") == false);
	return true;
}

bool GMeshNewFormat::ReadParametrizations()
{
	while (readLine("$EndParametrizations") == false);
	return true;
}

bool GMeshNewFormat::ReadEntities()
{
	// get the next line
	const char* szline = nextLine();

	int npts, ncrvs, nsrfs, nvols;
	int nread = sscanf(szline, "%d%d%d%d", &npts, &ncrvs, &nsrfs, &nvols);

	while (readLine("$EndEntities") == false);

	return true;
}

bool GMeshNewFormat::ReadNodes()
{
	// get the next line
	const char* szline = nextLine();

	// scan it for #entities, #nodes, minNodeTag, maxNodeTag
	int entities, nodes, minNodeTag, maxNodeTag;
	int nread = sscanf(szline, "%d%d%d%d", &entities, &nodes, &minNodeTag, &maxNodeTag);
	if (nread != 4) return false;

	// allocate nodes
	m_Node.resize(nodes);
	int nodeIndex = 0;

	// loop over entities
	for (int n = 0; n < entities; ++n)
	{
		// read the next line
		int ndim, ntag, nparam, numNodes;
		szline = nextLine();
		nread = sscanf(szline, "%d%d%d%d", &ndim, &ntag, &nparam, &numNodes);
		if (nread != 4) return false;

		// read the node IDs
		for (int i = 0; i < numNodes; ++i)
		{
			NODE& n = m_Node[nodeIndex + i];
			szline = nextLine();
			sscanf(szline, "%d", &n.id);
		}

		// read the nodal coordinates
		for (int i = 0; i < numNodes; ++i)
		{
			vec3d& r = m_Node[nodeIndex++].pos;
			szline = nextLine();
			sscanf(szline, "%lg%lg%lg", &r.x, &r.y, &r.z);
		}
	}

	// read the end of the nodes section
	if (readLine("$EndNodes") == false) return gmsh->errf("Failed finding $EndNodes");

	return true;
}

bool GMeshNewFormat::ReadElements()
{
	// scan it for #entities, #elements, minElemTag, maxElemTag
	const char* szline = nextLine();
	int entities, elems, minElemTag, maxElemTag;
	int nread = sscanf(szline, "%d%d%d%d", &entities, &elems, &minElemTag, &maxElemTag);
	if (nread != 4) return gmsh->errf("Error while reading Element section");;

	m_Face.reserve(elems);
	m_Elem.reserve(elems);

	// read the elements
	ELEMENT el;
	const int NMAX = 30;
	int n[NMAX];

	// loop over entities
	for (int entity = 0; entity < entities; ++entity)
	{
		// read the next line
		szline = nextLine();
		int entityDim, entityTag, elemType, numElemsInBlock;
		nread = sscanf(szline, "%d%d%d%d", &entityDim, &entityTag, &elemType, &numElemsInBlock);
		if (nread != 4) return gmsh->errf("Error while reading Element section");;

		for (int i = 0; i < numElemsInBlock; ++i)
		{
			szline = nextLine();
			int nread = scan_line(szline, n, NMAX);

			// store the entity number
			el.gid = entityTag;

			// surface elements
			int* m = n + 1;
			switch (elemType)
			{
			case 2: // triangle
				if (nread != 4) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_TRI3;
				el.node[0] = m[0];
				el.node[1] = m[1];
				el.node[2] = m[2];
				m_Face.push_back(el);
				break;
			case 3: // quad
				if (nread != 5) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_QUAD4;
				el.node[0] = m[0];
				el.node[1] = m[1];
				el.node[2] = m[2];
				el.node[3] = m[3];
				m_Face.push_back(el);
				break;
			}

			// volume elements
			switch (elemType)
			{
			case 4: // tetrahedron
				if (nread != 5) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_TET4;
				el.node[0] = m[0];
				el.node[1] = m[1];
				el.node[2] = m[2];
				el.node[3] = m[3];
				m_Elem.push_back(el);
				break;
			case 5: // hexahedrons
				if (nread != 9) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_HEX8;
				el.node[0] = m[0];
				el.node[1] = m[1];
				el.node[2] = m[2];
				el.node[3] = m[3];
				el.node[4] = m[4];
				el.node[5] = m[5];
				el.node[6] = m[6];
				el.node[7] = m[7];
				m_Elem.push_back(el);
				break;
			case 6: // pentahedron
				if (nread != 7) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_PENTA6;
				el.node[0] = m[0];
				el.node[1] = m[1];
				el.node[2] = m[2];
				el.node[3] = m[3];
				el.node[4] = m[4];
				el.node[5] = m[5];
				m_Elem.push_back(el);
				break;
			case 7: // 5-node pyramid
				if (nread != 6) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_PYRA5;
				el.node[0] = m[0];
				el.node[1] = m[1];
				el.node[2] = m[2];
				el.node[3] = m[3];
				el.node[4] = m[4];
				m_Elem.push_back(el);
				break;
			case 11: // 10-node tetrahedron
				if (nread != 11) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_TET10;
				el.node[0] = m[0];
				el.node[1] = m[1];
				el.node[2] = m[2];
				el.node[3] = m[3];
				el.node[4] = m[4];
				el.node[5] = m[5];
				el.node[6] = m[6];
				el.node[7] = m[7];
				el.node[8] = m[9];
				el.node[9] = m[8];
				m_Elem.push_back(el);
				break;
			case 14: // 14-node pyramid
			{
				if (nread != 14) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_PYRA13;
				el.node[ 0] = m[ 0];
				el.node[ 1] = m[ 1];
				el.node[ 2] = m[ 2];
				el.node[ 3] = m[ 3];
				el.node[ 4] = m[ 4];
				el.node[ 5] = m[ 5];
				el.node[ 6] = m[ 8];
				el.node[ 7] = m[10];
				el.node[ 8] = m[ 6];
				el.node[ 9] = m[ 7];
				el.node[10] = m[ 9];
				el.node[11] = m[11];
				el.node[12] = m[12];
				m_Elem.push_back(el);
			}
			break;
			case 19: // 13-node pyramid
			{
				if (nread != 14) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_PYRA13;
				el.node[ 0] = m[ 0];
				el.node[ 1] = m[ 1];
				el.node[ 2] = m[ 2];
				el.node[ 3] = m[ 3];
				el.node[ 4] = m[ 4];
				el.node[ 5] = m[ 5];
				el.node[ 6] = m[ 8];
				el.node[ 7] = m[10];
				el.node[ 8] = m[ 6];
				el.node[ 9] = m[ 7];
				el.node[10] = m[ 9];
				el.node[11] = m[11];
				el.node[12] = m[12];
				m_Elem.push_back(el);
			}
			break;
			case 29: // 20-tet tehrahedron
			{
				if (nread != 21) return gmsh->errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_TET20;
				el.node[ 0] = m[ 0];
				el.node[ 1] = m[ 1];
				el.node[ 2] = m[ 2];
				el.node[ 3] = m[ 3];
				el.node[ 4] = m[ 4];
				el.node[ 5] = m[ 5];
				el.node[ 6] = m[ 6];
				el.node[ 7] = m[ 7];
				el.node[ 8] = m[ 9];
				el.node[ 9] = m[ 8];
				el.node[10] = m[11];
				el.node[11] = m[10];
				el.node[12] = m[15];
				el.node[13] = m[14];
				el.node[14] = m[13];
				el.node[15] = m[12];
				el.node[16] = m[17];
				el.node[17] = m[19];
				el.node[18] = m[18];
				el.node[19] = m[16];

				m_Elem.push_back(el);
			}
			break;
			}
		}
	}

	int nfaces = (int)m_Face.size();
	int nelems = (int)m_Elem.size();

	// make sure all gids start from 0
	if (nelems > 0)
	{
		int minid = m_Elem[0].gid;
		for (int i = 0; i < nelems; ++i)
			if (m_Elem[i].gid < minid) minid = m_Elem[i].gid;

		if (minid != 0)
		{
			for (int i = 0; i < nelems; ++i) m_Elem[i].gid -= minid;
		}
	}

	// make sure all gids start from 0
	if (nfaces > 0)
	{
		int minid = m_Face[0].gid;
		for (int i = 0; i < nfaces; ++i)
			if (m_Face[i].gid < minid) minid = m_Face[i].gid;

		if (minid != 0)
		{
			for (int i = 0; i < nfaces; ++i) m_Face[i].gid -= minid;
		}
	}

	// read the end of the mesh format
	if (readLine("$EndElements") == false) return gmsh->errf("Failed finding EndElements");

	return true;
}
