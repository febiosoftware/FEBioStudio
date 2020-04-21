#include "FETetGenImport.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>

//-----------------------------------------------------------------------------
FETetGenImport::FETetGenImport()
{
	m_offset = 0;
}

//-----------------------------------------------------------------------------
FETetGenImport::~FETetGenImport()
{
}

//-----------------------------------------------------------------------------
bool FETetGenImport::Load(FEProject& prj, const char* szfile)
{
	m_Node.clear();
	m_Elem.clear();

	char sznode[256] = {0};
	strcpy(sznode, szfile);
	char* ch = strrchr(sznode, '.');
	if (ch == 0) return false;
	sprintf(ch, ".node");

	// open the node file
	if (Open(sznode, "rt") == false) return false;

	// read the first line
	char szline[256] = {0};
	fgets(szline, 255, m_fp);

	//checking for comments
	sscanf(szline, "%s", ch);
	while(*ch == '#')
	{
		fgets(szline, 255, m_fp);
		sscanf(szline, "%s", ch);
	}

	int nodes, ncoords;
	sscanf(szline, "%d%d", &nodes, &ncoords);
	if (ncoords != 3) return false;

	//taking care of empty lines
	fgets(szline, 255, m_fp);
	int nread = sscanf(szline, "%s", ch);
	while (nread == -1)
	{
		fgets(szline, 255, m_fp);
		nread = sscanf(szline, "%s", ch);
	}

	NODE nd;
	for (int i=0; i<nodes; ++i)
	{		
		sscanf(szline, "%*d%lg%lg%lg", &nd.x, &nd.y, &nd.z);
		m_Node.push_back(nd);
		fgets(szline, 255, m_fp);
	}

	Close();

	// open the element file
	if (Open(szfile, "rt") == false) return false;

	fgets(szline, 255, m_fp);

	//checking for comments
	sscanf(szline, "%s", ch);
	while(*ch == '#')
	{
		fgets(szline, 255, m_fp);
		sscanf(szline, "%s", ch);
	}

	int nelems, npe, natt;
	sscanf(szline, "%d%d%d", &nelems, &npe, &natt);
	if (npe != 4) return false;

	//taking care of empty lines
	// NOTE: Not sure what this is doing here. Commenting it out for now.
/*	fgets(szline, 255, m_fp);
	nread = sscanf(szline, "%s", ch);
	while (nread == -1)
	{
		fgets(szline, 255, m_fp);
		nread = sscanf(szline, "%s", ch);
	}
*/
	ELEM el;
	int att, nid;
	for (int i=0; i<nelems; ++i)
	{
		fgets(szline, 255, m_fp);
		int nread = sscanf(szline, "%d%d%d%d%d%d", &nid, &el.node[0], &el.node[1], &el.node[2], &el.node[3], &att);
		if ((natt>0) && (nread == 5)) el.att = att; else el.att = 0; 

		// we use the first ID to see if the file is zero or one based
		if (i == 0)
		{
			if ((nid != 0) && (nid != 1)) return false;
			m_offset = nid;
		}
		m_Elem.push_back(el);
	}

	Close();

	return BuildMesh(prj.GetFEModel());
}

//-----------------------------------------------------------------------------
bool FETetGenImport::BuildMesh(FEModel& fem)
{
	// counts
	int nodes = (int) m_Node.size();
	int elems = (int) m_Elem.size();

	// create a new mesh
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	// create the nodes
	for (int i=0; i<nodes; ++i)
	{
		NODE& nd = m_Node[i];
		pm->Node(i).r = vec3d(nd.x, nd.y, nd.z);
	}

	// create the elements
	for (int i=0; i<elems; ++i)
	{
		ELEM& tet = m_Elem[i];
		FEElement& el = pm->Element(i);

		el.SetType(FE_TET4);
		el.m_node[0] = tet.node[0]-m_offset;
		el.m_node[1] = tet.node[1]-m_offset;
		el.m_node[2] = tet.node[2]-m_offset;
		el.m_node[3] = tet.node[3]-m_offset;

		el.m_gid = tet.att;
	}

	// update the mesh
	pm->RebuildMesh();

	GObject* po = new GMeshObject(pm);

	// clean up
	m_Node.clear();
	m_Elem.clear();

	// if we get here we are good to go!
	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	// we're good!
	return true;
}
