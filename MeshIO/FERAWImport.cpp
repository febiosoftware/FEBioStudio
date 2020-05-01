#include "FERAWImport.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>

//-----------------------------------------------------------------------------
FERAWImport::FERAWImport(FEProject& prj) : FEFileImport(prj)
{
	m_nx = 0;
	m_ny = 0;
	m_nz = 0;

	m_x0 = m_y0 = m_z0 = 0.0;
	m_w = m_h = m_d = 1.0;
}

//-----------------------------------------------------------------------------
FERAWImport::~FERAWImport()
{
	m_nx = m_ny = m_nz = 0;
}

//-----------------------------------------------------------------------------
void FERAWImport::SetImageDimensions(int nx, int ny, int nz)
{
	m_nx = nx;
	m_ny = ny;
	m_nz = nz;
}

//-----------------------------------------------------------------------------
void FERAWImport::SetBoxSize(double x0, double y0, double z0, double w, double h, double d)
{
	m_x0 = x0;
	m_y0 = y0;
	m_z0 = z0;

	m_w = w;
	m_h = h;
	m_d = d;
}

//-----------------------------------------------------------------------------
bool FERAWImport::Load(const char* szfile)
{
	// open and read file
	if (Open(szfile, "rb") == false) return false;
	int N = m_nx*m_ny*m_nz;
	unsigned char* pb = new unsigned char[N];
	fread(pb, N, 1, m_fp);
	Close();

	// reindex image so that we know how many gray values are effectively used 
	vector<int> bin; bin.assign(256, 0);
	for (int i=0; i<N; ++i) bin[pb[i]]++;
	int n = 0;
	for (int i=0; i<256; ++i) if (bin[i] > 0) bin[i] = n++; else bin[i] = -1;
	for (int i=0; i<N; ++i) { pb[i] = bin[pb[i]]; assert(pb[i] >= 0); }

	// get the FE Model
	FEModel& fem = m_prj.GetFEModel();

	// create a new mesh
	int nodes = (m_nx+1)*(m_ny+1)*(m_nz+1);
	int elems = m_nx*m_ny*m_nz;
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	// define the nodal coordinates
	n = 0;
	for (int k=0; k<=m_nz; ++k)
		for (int j=0; j<=m_ny; ++j)
			for (int i=0; i<=m_nx; ++i, ++n)
			{
				vec3d& r = pm->Node(n).r;
				double x = (double) i / (double) m_nx;
				double y = (double) j / (double) m_ny;
				double z = (double) k / (double) m_nz;

				r.x = m_x0 + x*m_w;
				r.y = m_y0 + y*m_h;
				r.z = m_z0 + z*m_d;
			}

	// define the elements
	n = 0;
	for (int k=0; k<m_nz; ++k)
		for (int j=0; j<m_ny; ++j)
			for (int i=0; i<m_nx; ++i, ++n)
			{
				FEElement& elem = pm->Element(n);
				elem.SetType(FE_HEX8);
				elem.m_gid = pb[n];
				elem.m_node[0] = k*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i;
				elem.m_node[1] = k*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i+1;
				elem.m_node[2] = k*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i+1;
				elem.m_node[3] = k*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i;
				elem.m_node[4] = (k+1)*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i;
				elem.m_node[5] = (k+1)*(m_nx+1)*(m_ny+1) + j*(m_nx+1) + i+1;
				elem.m_node[6] = (k+1)*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i+1;
				elem.m_node[7] = (k+1)*(m_nx+1)*(m_ny+1) + (j+1)*(m_nx+1) + i;
			}
	delete [] pb;

	GMeshObject* po = new GMeshObject(pm);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
