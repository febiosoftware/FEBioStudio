#include "FEHyperSurfImport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/GModel.h>

FEHyperSurfImport::FEHyperSurfImport(FEProject& prj) : FEFileImport(prj)
{
}

FEHyperSurfImport::~FEHyperSurfImport(void)
{
}

bool FEHyperSurfImport::Load(const char* szfile)
{
	FEModel& fem = m_prj.GetFEModel();

	// open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s", szfile);

	int i;
	char szline[256];

	// find the vertices
	char* ch;
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("Error while reading file");
		if (strncmp(szline, "Vertices", 8) == 0) break;
	}
	while (true);

	int nodes = 0;
	sscanf(szline+8, "%d", &nodes);

	FESurfaceMesh* pm = new FESurfaceMesh();
	pm->Create(nodes, 0, 0);

	// read the nodes
	for (i=0; i<nodes; ++i)
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) { delete pm; Close(); return false; }

		FENode& node = pm->Node(i);
		sscanf(szline, "%lg%lg%lg", &node.r.x, &node.r.y, &node.r.z);
	}

	// find the triangles
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) { delete pm; Close(); return false; }
		if (strncmp(szline, "Triangles", 9) == 0) break;
	}
	while (true);

	int elems = 0;
	sscanf(szline+9, "%d", &elems);
	pm->Create(0, 0, elems);

	// read the elements
	int ne[3];
	for (i=0; i<elems; ++i)
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) { delete pm; Close(); return false; }

		FEFace& face = pm->Face(i);
		face.SetType(FE_FACE_TRI3);
		sscanf(szline, "%d%d%d", &ne[0], &ne[1], &ne[2]);

		face.m_gid = 0;
		face.n[0] = ne[0] - 1;
		face.n[1] = ne[1] - 1;
		face.n[2] = ne[2] - 1;
		face.n[3] = ne[2] - 1;
	}

	// close the file
	Close();

	// create a new object
	pm->Update();
	GObject* po = new GSurfaceMeshObject(pm);

	// add the object to the model
	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
