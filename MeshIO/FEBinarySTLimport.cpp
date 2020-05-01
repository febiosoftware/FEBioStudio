#include "FEBinarySTLimport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/GModel.h>

//-----------------------------------------------------------------------------
FEBinarySTLimport::FEBinarySTLimport(FEProject& prj) : FEFileImport(prj)
{
}

//-----------------------------------------------------------------------------
FEBinarySTLimport::~FEBinarySTLimport(void)
{
}

//-----------------------------------------------------------------------------
// read a line from the input file
bool FEBinarySTLimport::read_facet(FEBinarySTLimport::FACET& f)
{
	unsigned short att;
	if (fread(f.norm, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v1, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v2, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v3, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(&att, sizeof(att), 1, m_fp) != 1) return false;
	return true;
}

//-----------------------------------------------------------------------------
// Load an STL model
bool FEBinarySTLimport::Load(const char* szfile)
{
	FEModel& fem = m_prj.GetFEModel();
	m_pfem = &fem;

	// try to open the file
	if (Open(szfile, "rb") == false) return errf("Failed opening file %s.", szfile);

	// read the header
	char szbuf[80] = {0};
	if (fread(szbuf, 80, 1, m_fp) != 1) return errf("Failed reading header.");

	// clear the list
	m_Face.clear();

	// read the number of triangles
	int numtri = 0;
	fread(&numtri, sizeof(int), 1, m_fp);
	if (numtri <= 0) return errf("Invalid number of triangles.");

	// read all the triangles
	FACET face;
	for (int i=0; i<numtri; ++i)
	{
		// read the facet line
		if (read_facet(face) == false) return errf("Error encountered reading triangle data.");

		// add the facet to the list
		m_Face.push_back(face);
	}

	// close the file
	Close();

	// build the nodes
	build_mesh();

	return true;
}

//-----------------------------------------------------------------------------
// Build the FE model
void FEBinarySTLimport::build_mesh()
{
	int i;

	// number of facets
	int NF = m_Face.size();

	// find the bounding box of the model
	BOX& b = m_box = BoundingBox();
	double h = 0.01*b.GetMaxExtent();
	b.Inflate(h,h,h);
	vec3d r0(b.x0, b.y0, b.z0);
	vec3d r1(b.x1, b.y1, b.z1);

	// create the box partition
	int NB = m_NB = (int) pow((double) (NF*3), 0.17) + 1;
	m_BL.resize(NB*NB*NB);
	int n = 0;
	for (i=0; i<NB; ++i)
		for (int j=0; j<NB; ++j)
			for (int k=0; k<NB; ++k)
			{
				OBOX& bn = m_BL[n++];
				bn.r0.x = r0.x + i*(r1.x - r0.x)/NB;
				bn.r0.y = r0.x + j*(r1.x - r0.x)/NB;
				bn.r0.z = r0.x + k*(r1.x - r0.x)/NB;

				bn.r1.x = r0.x + (i+1)*(r1.x - r0.x)/NB;
				bn.r1.y = r0.x + (j+1)*(r1.x - r0.x)/NB;
				bn.r1.z = r0.x + (k+1)*(r1.x - r0.x)/NB;
			}

	// reserve space for nodes
	m_Node.reserve(NF*3);
	
	// create the nodes
	list<FACET>::iterator pf = m_Face.begin();
	int nid = 0;
	for (i=0; i<NF; ++i, ++pf)
	{
		FACET& f = *pf;
        vec3d v1(f.v1[0], f.v1[1], f.v1[2]);
        vec3d v2(f.v2[0], f.v2[1], f.v2[2]);
        vec3d v3(f.v3[0], f.v3[1], f.v3[2]);
		f.n[0] = find_node(v1);
		f.n[1] = find_node(v2);
		f.n[2] = find_node(v3);

		// make sure all three nodes are distinct
		int* n = f.n;
		if ((n[0] == n[1]) || (n[0]==n[2]) || (n[1]==n[2])) f.nid = -1;
		else f.nid = nid++;
	}
	int NN = m_Node.size();
	int NE = nid;

	// create the mesh
	FESurfaceMesh* pm = new FESurfaceMesh;
	pm->Create(NN, 0, NE);

	// create nodes
	for (i=0; i<NN; ++i)
	{
		vec3d& ri = m_Node[i].r;
		pm->Node(i).pos(ri);
	}

	// create elements
	list<FACET>::iterator is = m_Face.begin();
	for (i=0; i<NF; ++i, ++is)
	{
		FACET& facet = *is;
		int n = facet.nid;
		if (n >= 0)
		{
			FEFace& face = pm->Face(n);
			face.SetType(FE_FACE_TRI3);
			face.m_gid = 0;
			face.n[0] = facet.n[0];
			face.n[1] = facet.n[1];
			face.n[2] = facet.n[2];
		}
		else assert(false);
	}

	// update the mesh
	pm->RebuildMesh();
	GSurfaceMeshObject* po = new GSurfaceMeshObject(pm);

	static int nc = 1;
	char sz[256];
	sprintf(sz, "STL-Object%02d", nc++);
	po->SetName(sz);

	// add the object to the model
	m_pfem->GetModel().AddObject(po);
}

//-----------------------------------------------------------------------------
int FEBinarySTLimport::FindBox(vec3d& r)
{
	BOX& b = m_box;
	int i = (int)(m_NB*(r.x - b.x0)/(b.x1 - b.x0));
	int j = (int)(m_NB*(r.y - b.y0)/(b.y1 - b.y0));
	int k = (int)(m_NB*(r.z - b.z0)/(b.z1 - b.z0));
	return k + j*(m_NB + i*m_NB);
}

//-----------------------------------------------------------------------------
int FEBinarySTLimport::find_node(vec3d& r, const double eps)
{
	// get the box in which this node lies
	int nb = FindBox(r);
	assert((nb >= 0)&&(nb<(int)m_BL.size()));
	OBOX& b = m_BL[nb];

	// see if this node is already in this box
	int N = b.NL.size();
	for (int i=0; i<N; ++i)
	{
		NODE& ni = m_Node[b.NL[i]];
		vec3d& ri = ni.r;
		if ((ri - r)*(ri - r) < eps) return ni.n;
	}

	N = m_Node.size();
	NODE nd;
	nd.r = r;
	nd.n = N;
	m_Node.push_back(nd);
	b.NL.push_back(nd.n);
	return nd.n;
}

//-----------------------------------------------------------------------------
BOX FEBinarySTLimport::BoundingBox()
{
	list<FACET>::iterator pf = m_Face.begin();
	vec3d r = vec3d(pf->v1[0], pf->v1[1], pf->v1[2]);
	BOX b(r, r);
	for (pf = m_Face.begin(); pf != m_Face.end(); ++pf)
	{
		b += vec3d(pf->v1[0], pf->v1[1], pf->v1[2]);
		b += vec3d(pf->v2[0], pf->v2[1], pf->v2[2]);
		b += vec3d(pf->v3[0], pf->v3[1], pf->v3[2]);
	}
	return b;
}
