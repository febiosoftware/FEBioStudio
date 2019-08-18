#include "stdafx.h"
#include "FERAWImageReader.h"
#include "FEMeshData_T.h"

using namespace Post;

//-----------------------------------------------------------------------------
FERAWImageReader::FERAWImageReader() : FEFileReader("RAW")
{
	m_ops.nx = 0;
	m_ops.ny = 0;
	m_ops.nz = 0;
	m_ops.nformat = -1;
}

//-----------------------------------------------------------------------------
bool FERAWImageReader::Load(FEModel& fem, const char* szfile)
{
	if ((m_ops.nx <= 0) || (m_ops.ny <= 0) || (m_ops.nz <= 0)) return errf("Invalid image dimensions");
	if (m_ops.nformat != 0) return errf("Invalid image format");

	// open the file
	if (Open(szfile, "rb") == false) return errf("Failed opening file.");

	// add one material to the scene
	FEMaterial mat;
	fem.AddMaterial(mat);

	// figure out the mesh dimensions
	int NN = m_ops.nx * m_ops.ny * m_ops.nz;
	int NE = 0;
	int dim = 0;
	if (m_ops.nz == 1) { dim = 2; NE = (m_ops.nx-1)*(m_ops.ny-1); }
	else { dim = 3; NE = (m_ops.nx-1)*(m_ops.ny-1)*(m_ops.nz - 1); }

	// create a new mesh
	FEMeshBase* pm = 0;
	if (dim == 2) pm = new FEQuadMesh;
	else pm = new FEMeshHex8;

	try
	{
		pm->Create(NN, NE);
	}
	catch (std::bad_alloc)
	{
		return errf("Insufficient memory for importing image data");
	}
	catch (...)
	{
		return errf("Unkwown exception in FERAWImageReader::Load");
	}
	fem.AddMesh(pm);

	// scale parameters
	double dx = m_ops.wx / (float) m_ops.nx;
	double dy = m_ops.wy / (float) m_ops.ny;
	double dz = m_ops.wz / (float) m_ops.nz;

	// position the nodes
	int n = 0;
	for (int k=0; k<m_ops.nz; ++k)
		for (int j=0; j<m_ops.ny; ++j)
			for (int i=0; i<m_ops.nx; ++i)
			{
				FENode& nd = pm->Node(n++);
				nd.m_r0.x = (float) i * dx;
				nd.m_r0.y = (float) j * dy;
				nd.m_r0.z = (float) k * dz;
				nd.m_rt = nd.m_r0;
			}

	if (dim == 2)
	{
		// create a 2D mesh
		n = 0;
		for (int j=0; j<m_ops.ny-1; ++j)
			for (int i=0; i<m_ops.nx-1; ++i)
			{
				FEElement& e = pm->Element(n++);
				e.m_node[0] = j*(m_ops.nx) + i;
				e.m_node[1] = j*(m_ops.nx) + i+1;
				e.m_node[2] = (j+1)*(m_ops.nx) + i+1;
				e.m_node[3] = (j+1)*(m_ops.nx) + i;
			}
	}
	else
	{
		// create a 3D mesh
		n = 0;
		for (int k=0; k<m_ops.nz-1; ++k)
			for (int j=0; j<m_ops.ny-1; ++j)
				for (int i=0; i<m_ops.nx-1; ++i)
				{
					FEElement& e = pm->Element(n++);
					e.m_node[0] = k*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i;
					e.m_node[1] = k*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i+1;
					e.m_node[2] = k*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i+1;
					e.m_node[3] = k*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i;
					e.m_node[4] = (k+1)*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i;
					e.m_node[5] = (k+1)*(m_ops.nx*m_ops.ny) + j*(m_ops.nx) + i+1;
					e.m_node[6] = (k+1)*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i+1;
					e.m_node[7] = (k+1)*(m_ops.nx*m_ops.ny) + (j+1)*(m_ops.nx) + i;
				}
	}

	// update the mesh
	pm->Update();
	fem.UpdateBoundingBox();

	// Add a data field
	fem.AddDataField(new FEDataField_T<FENodeData<float> >("Image", EXPORT_DATA));

	// we need a single state
	FEState* ps = new FEState(0.f, &fem, fem.GetFEMesh(0));
	fem.AddState(ps);

	// add the image data
	FENodeData<float>& d = dynamic_cast<FENodeData<float>&>(ps->m_Data[0]);
	unsigned char* pb = new unsigned char[ m_ops.nx*m_ops.ny ];
	n = 0;
	for (int k=0; k<m_ops.nz; ++k)
	{
		fread(pb, sizeof(unsigned char), m_ops.nx*m_ops.ny, m_fp);
		for (int j=0; j<m_ops.ny; ++j)
		{
			for (int i=0; i<m_ops.nx; ++i)
			{
				d[n++] = (float) pb[j*m_ops.nx + i];
			}
		}
	}
	delete [] pb;

	Close();

	return true;
}
