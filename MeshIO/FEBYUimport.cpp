#include "FEBYUimport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/GModel.h>

FEBYUimport::FEBYUimport(FEProject& prj) : FEFileImport(prj)
{
}

FEBYUimport::~FEBYUimport(void)
{
}

bool FEBYUimport::Load(const char* szfile)
{
	FEModel& fem = m_prj.GetFEModel();

	if (!Open(szfile, "rt")) return errf("Failed opening file %s.", szfile);

	char szline[256] = {0}, *ch;
	int nread, i;

	// read the first line
	int nparts = 0;
	int nodes = 0;
	int edges = 0;
	int elems = 0;
	ch = fgets(szline, 255, m_fp);
	if (ch == 0) return errf("An unexpected error occured while reading the file data.");
	nread = sscanf(szline, "%d%d%d%d", &nparts, &nodes, &elems, &edges);
	if (nread != 4) return errf("Error or first line. Is this a BYU file?");

	if (nparts <= 0) return errf("Invalid number of parts.");
	if (nodes <= 0) return errf("Invalid number of nodes.");
	if (edges <= 0) return errf("Invalid number of edges.");
	if (elems <= 0) return errf("Invalid number of polygons.");

	// read the parts
	m_Part.resize(nparts);
	for (i=0; i<nparts; ++i)
	{
		PART& p = m_Part[i];
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		nread = sscanf(szline, "%d%d", &p.n0, &p.n1);
	}

	// create a new mesh
	FESurfaceMesh* pm = new FESurfaceMesh();
	pm->Create(nodes, 0, elems);

	// read the nodes
	for (i=0; i<nodes; ++i)
	{
		FENode& n = pm->Node(i);
		vec3d& r = n.r;
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		nread = sscanf(szline, "%lg%lg%lg", &r.x, &r.y, &r.z);
		if (nread != 3) return errf("An error occured while reading the nodal coordinates.");
	}

	// read the elements
	int n[5];
	for (i=0; i<elems; ++i)
	{
		FEFace& f = pm->Face(i);
		f.m_gid = 0;
		ch = fgets(szline, 255, m_fp);
//		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		nread = sscanf(szline, "%d%d%d%d%d", &n[0], &n[1], &n[2], &n[3], &n[4]);
		switch (nread)
		{
		case 3: 
			f.SetType(FE_FACE_TRI3);
			f.n[0] = n[0] - 1;
			f.n[1] = n[1] - 1;
			f.n[2] = -n[2] - 1;
			f.n[3] = -n[2] - 1;
			break;
		case 4:
			f.SetType(FE_FACE_QUAD4);
			f.n[0] = n[0] - 1;
			f.n[1] = n[1] - 1;
			f.n[2] = n[2] - 1;
			f.n[3] = -n[3] - 1;
			break;
		default:
			delete pm;
			return errf("Only triangular and quadrilateral polygons are supported.");
		}
	}

	Close();

	pm->RebuildMesh();
	GSurfaceMeshObject* po = new GSurfaceMeshObject(pm);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
