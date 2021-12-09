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

#include "FEGMshImport.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>
#include <vector>
//using namespace std;

//! \todo PreView has trouble with reading surface elements and volume elements since
//! the surface elements are not shell elements.
FEGMshImport::FEGMshImport(FEProject& prj) : FEFileImport(prj)
{
	m_szline[0] = 0;
	m_pm = 0;
	m_pfem = 0;
}

bool FEGMshImport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// open the file
	if (!Open(szfile, "rt")) return false;

	// create a new mesh
	m_pm = new FEMesh();

	bool ret = true;
	while (fgets(m_szline, 255, m_fp))
	{
		if      (strncmp(m_szline, "$MeshFormat"   , 11) == 0) ret = ReadMeshFormat();
		else if (strncmp(m_szline, "$PhysicalNames", 14) == 0) ret = ReadPhysicalNames();
		else if (strncmp(m_szline, "$Nodes"        ,  6) == 0) ret = ReadNodes();
		else if (strncmp(m_szline, "$Elements"     ,  9) == 0) ret = ReadElements();
		else
		{
			// we didn't recognize the section
			return errf("Unknown section: %s", m_szline);
		}

		if (ret == false) return false;
	}

	// close the file
	Close();

	m_pm->RebuildMesh();

	// create a new object from this mesh
	GMeshObject* po = new GMeshObject(m_pm);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}

//-----------------------------------------------------------------------------

bool FEGMshImport::ReadMeshFormat()
{
	// read the format line
	fgets(m_szline, 255, m_fp);
	float fversion;
	int ntype;
	int ngsize;
	int nread = sscanf(m_szline, "%g %d %d", &fversion, &ntype, &ngsize);
	if (nread != 3) return errf("Syntax error on line 2");
//	if (fversion != 2.f) return errf("Invalid file version");
	if (ntype != 0) return errf("Invalid file type");
	if (ngsize != 8) return errf("data format must be 8");

	// read the end of the mesh format
	fgets(m_szline, 255, m_fp);
	if (strncmp(m_szline, "$EndMeshFormat", 14) != 0) return errf("Failed finding EndMeshFormat");

	return true;
}

//-----------------------------------------------------------------------------

bool FEGMshImport::ReadPhysicalNames()
{
	// read the number of physical names
	if (fgets(m_szline, 255, m_fp) == 0) return errf("Unexpected end of file");
	int names = 0;
	int nread = sscanf(m_szline, "%d", &names);
	if (nread != 1) return errf("Error while reading physical names");

	// read the names
	for (int i=0; i<names; ++i)
	{
		if (fgets(m_szline, 255, m_fp) == 0) return errf("Unexpected end of file");
		char szname[128] = {0};
		int n, nread;
		nread = sscanf(m_szline, "%*d%d%s", &n, szname);
		if (nread != 2) return false;
	}

	// read the end tag
	fgets(m_szline, 255, m_fp);
	if (strncmp(m_szline, "$EndPhysicalNames", 17) != 0) return errf("Failed reading $EndPhysicalNames");

	return true;
}

//-----------------------------------------------------------------------------

bool FEGMshImport::ReadNodes()
{
	fgets(m_szline, 255, m_fp);
	int nodes = 0;
	int nread = sscanf(m_szline, "%d", &nodes);
	if (nread != 1) return errf("Error while reading Nodes section");

	m_pm->Create(nodes, 0);

	// read the nodes
	for (int i=0; i<nodes; ++i)
	{
		FENode& node = m_pm->Node(i);
		vec3d& r = node.r;

		fgets(m_szline, 255, m_fp);
		int nread = sscanf(m_szline, "%*d %lg %lg %lg", &r.x, &r.y, &r.z);
		if (nread != 3) return errf("Error while reading Nodes section");
	}

	// read the end of the mesh format
	fgets(m_szline, 255, m_fp);
	if (strncmp(m_szline, "$EndNodes", 9) != 0) return errf("Failed finding EndNodes");

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
bool FEGMshImport::ReadElements()
{
	fgets(m_szline, 255, m_fp);
	int elems = 0;
	int nread = sscanf(m_szline, "%d", &elems);
	if (nread != 1) return errf("Error while reading Element section");

	vector<ELEMENT> Face;
	vector<ELEMENT> Elem;
	Face.reserve(elems);
	Elem.reserve(elems);

	// read the elements
	ELEMENT el;
	const int NMAX = 30;
	int n[NMAX];

	for (int i=0; i<elems; ++i)
	{
		fgets(m_szline, 255, m_fp);
		int nread = scan_line(m_szline, n, NMAX);

		// element type
		int etype = n[1];

		// number of tags
		int ntags = n[2];
		if (ntags == 0) ntags = 2;	// According to the documentation GMesh files should have at least two tags

		// store the entity number (last tag)
		el.gid = (ntags > 0? n[2 + ntags] : 0);

		// store pointer to node numbers (depends on tags)
		int* m = n + ntags + 3;

		// surface elements
		switch (etype)
		{
		case 2: // triangle
			if (nread != 3 + ntags + 3) return errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_TRI3;
			el.node[0] = m[0] - 1;
			el.node[1] = m[1] - 1;
			el.node[2] = m[2] - 1;
			Face.push_back(el);
			break;
		case 3: // quad
			if (nread != 3 + ntags + 4) return errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_QUAD4;
			el.node[0] = m[0] - 1;
			el.node[1] = m[1] - 1;
			el.node[2] = m[2] - 1;
			el.node[3] = m[3] - 1;
			Face.push_back(el);
			break;
		}

		// volume elements
		switch (etype)
		{
		case 4: // tetrahedron
			if (nread != 3 + ntags + 4) return errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_TET4;
			el.node[0] = m[0] - 1;
			el.node[1] = m[1] - 1;
			el.node[2] = m[2] - 1;
			el.node[3] = m[3] - 1;
			Elem.push_back(el);
			break;
		case 5: // hexahedrons
			if (nread != 3 + ntags + 8) return errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_HEX8;
			el.node[0] = m[0] - 1;
			el.node[1] = m[1] - 1;
			el.node[2] = m[2] - 1;
			el.node[3] = m[3] - 1;
			el.node[4] = m[4] - 1;
			el.node[5] = m[5] - 1;
			el.node[6] = m[6] - 1;
			el.node[7] = m[7] - 1;
			Elem.push_back(el);
			break;
		case 6: // pentahedron
			if (nread != 3 + ntags + 6) return errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_PENTA6;
			el.node[0] = m[0] - 1;
			el.node[1] = m[1] - 1;
			el.node[2] = m[2] - 1;
			el.node[3] = m[3] - 1;
			el.node[4] = m[4] - 1;
			el.node[5] = m[5] - 1;
			Elem.push_back(el);
			break;
		case 7: // 5-node pyramid
			if (nread != 3 + ntags + 5) return errf("Invalid number of entries when reading element %d", n[0]);
			el.ntype = FE_PYRA5;
			el.node[0] = m[0] - 1;
			el.node[1] = m[1] - 1;
			el.node[2] = m[2] - 1;
			el.node[3] = m[3] - 1;
			el.node[4] = m[4] - 1;
			Elem.push_back(el);
			break;
        case 11: // 10-node tetrahedron
            if (nread != 3 + ntags + 10) return errf("Invalid number of entries when reading element %d", n[0]);
            el.ntype = FE_TET10;
            el.node[0] = m[0] - 1;
            el.node[1] = m[1] - 1;
            el.node[2] = m[2] - 1;
            el.node[3] = m[3] - 1;
            el.node[4] = m[4] - 1;
            el.node[5] = m[5] - 1;
            el.node[6] = m[6] - 1;
            el.node[7] = m[7] - 1;
            el.node[8] = m[9] - 1;
            el.node[9] = m[8] - 1;
            Elem.push_back(el);
            break;
        case 19: // 13-node pyramid
            {
                if (nread != 3 + ntags + 13) return errf("Invalid number of entries when reading element %d", n[0]);
                el.ntype = FE_PYRA13;
                el.node[0] = m[0] - 1;
                el.node[1] = m[1] - 1;
                el.node[2] = m[2] - 1;
                el.node[3] = m[3] - 1;
                el.node[4] = m[4] - 1;
                el.node[ 5] = m[ 5] - 1;
                el.node[ 6] = m[ 8] - 1;
                el.node[ 7] = m[10] - 1;
                el.node[ 8] = m[ 6] - 1;
                el.node[ 9] = m[ 7] - 1;
                el.node[10] = m[ 9] - 1;
                el.node[11] = m[11] - 1;
                el.node[12] = m[12] - 1;
                Elem.push_back(el);
            }
            break;
		case 29: // 20-tet tehrahedron
			{
				if (nread != 3 + ntags + 20) return errf("Invalid number of entries when reading element %d", n[0]);
				el.ntype = FE_TET20;
				el.node[ 0] = m[ 0] - 1;
				el.node[ 1] = m[ 1] - 1;
				el.node[ 2] = m[ 2] - 1;
				el.node[ 3] = m[ 3] - 1;
				el.node[ 4] = m[ 4] - 1;
				el.node[ 5] = m[ 5] - 1;
				el.node[ 6] = m[ 6] - 1;
				el.node[ 7] = m[ 7] - 1;
				el.node[ 8] = m[ 9] - 1;
				el.node[ 9] = m[ 8] - 1;
				el.node[10] = m[11] - 1;
				el.node[11] = m[10] - 1;
				el.node[12] = m[15] - 1;
				el.node[13] = m[14] - 1;
				el.node[14] = m[13] - 1;
				el.node[15] = m[12] - 1;
				el.node[16] = m[17] - 1;
				el.node[17] = m[19] - 1;
				el.node[18] = m[18] - 1;
				el.node[19] = m[16] - 1;

				Elem.push_back(el);
			}
			break;
		}
	}

	int nfaces = (int) Face.size();
	int nelems = (int) Elem.size();

	// make sure all gids start from 0
	if (nelems > 0)
	{
		int minid = Elem[0].gid;
		for (int i=0; i<nelems; ++i)
			if (Elem[i].gid < minid) minid = Elem[i].gid;

		if (minid != 0)
		{
			for (int i=0; i<nelems; ++i) Elem[i].gid -= minid;
		}
	}

	// make sure all gids start from 0
	if (nfaces > 0)
	{
		int minid = Face[0].gid;
		for (int i=0; i<nfaces; ++i)
			if (Face[i].gid < minid) minid = Face[i].gid;

		if (minid != 0)
		{
			for (int i=0; i<nfaces; ++i) Face[i].gid -= minid;
		}
	}
	
	if (nelems > 0)
	{
		m_pm->Create(0, nelems, nfaces);
		for (int i=0; i<nelems; ++i)
		{
			FEElement& el = m_pm->Element(i);
			ELEMENT& e = Elem[i];

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
				return false;
			}
		}

		for (int i=0; i<nfaces; ++i)
		{
			FEFace& face = m_pm->Face(i);
			ELEMENT& e = Face[i];

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
		}
	}
	else if (nfaces > 0)
	{
		// If no volume elements are defined, we create a shell mesh
		m_pm->Create(0, nfaces);
		for (int i=0; i<nfaces; ++i)
		{
			FEElement& el = m_pm->Element(i);
			ELEMENT& e = Face[i];

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
		}
	}

	// read the end of the mesh format
	fgets(m_szline, 255, m_fp);
	if (strncmp(m_szline, "$EndElements", 12) != 0) return errf("Failed finding EndElements");

	return true;
}
