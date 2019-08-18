#include "stdafx.h"
#include "FELSDYNAExport.h"
using namespace Post;

//-----------------------------------------------------------------------------
FELSDYNAExport::FELSDYNAExport()
{
	m_bsel = false;
	m_bnode = false;
}

//-----------------------------------------------------------------------------
bool FELSDYNAExport::Save(FEModel &fem, int ntime, const char *szfile)
{
	if (m_bsurf)
	{
		if (m_bsel) return ExportSelectedSurface(fem, ntime, szfile);
		else return ExportSurface(fem, ntime, szfile);
	}
	else return ExportMesh(fem, ntime, szfile);
}

//-----------------------------------------------------------------------------
bool FELSDYNAExport::ExportSurface(FEModel &fem, int ntime, const char *szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	int i, j, n;

	// get the mesh
	FEMeshBase& mesh = *fem.GetFEMesh(0);

	// count total nr of faces
	int faces = mesh.Faces();

	// count nr of nodes
	for (i=0; i<mesh.Nodes(); ++i) mesh.Node(i).m_ntag = 0;
	for (i=0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		n = f.Nodes();
		for (j=0; j<n; ++j) mesh.Node(f.node[j]).m_ntag = 1;
	}

	// count nr of nodes
	int nodes = 0;
	for (i=0; i<mesh.Nodes(); ++i) if (mesh.Node(i).m_ntag == 1) mesh.Node(i).m_ntag = ++nodes;

	// write the header
	fprintf(fp, "*KEYWORD\n");

	// write the nodes
	fprintf(fp, "*NODE\n");
	for (i=0; i<mesh.Nodes(); ++i)
	{
		FENode& node = mesh.Node(i);
		if (node.m_ntag > 0)
		{
			vec3f r = node.m_rt;
			fprintf(fp, "%8d%16.7e%16.7e%16.7e\n", node.m_ntag, r.x, r.y, r.z);
		}
	}

	FEState* ps = fem.GetState(ntime);

	// write the faces (as shells with thickness)
	fprintf(fp, "*ELEMENT_SHELL_THICKNESS\n");
	for (i=0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		int n[4];
		n[0] = mesh.Node(f.node[0]).m_ntag;
		n[1] = mesh.Node(f.node[1]).m_ntag;
		n[2] = mesh.Node(f.node[2]).m_ntag;
		n[3] = mesh.Node(f.node[3]).m_ntag;

		float v[4];
		v[0] = ps->m_NODE[f.node[0]].m_val;
		v[1] = ps->m_NODE[f.node[1]].m_val;
		v[2] = ps->m_NODE[f.node[2]].m_val;
		v[3] = ps->m_NODE[f.node[3]].m_val;
		fprintf(fp, "%8d%8d%8d%8d%8d%8d\n", i+1, f.m_mat+1, n[0], n[1], n[2], n[3]);
		fprintf(fp, "%16.7e%16.7e%16.7e%16.7e\n", v[0], v[1], v[2], v[3]);
	}

	fprintf(fp, "*END\n");

	fclose(fp);

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAExport::ExportSelectedSurface(FEModel &fem, int ntime, const char *szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;
	fprintf(fp, "*KEYWORD\n");

	FEMeshBase& m = *fem.GetFEMesh(0);
	int NN = m.Nodes();
	int NF = m.Faces();
	int i;
	for (i=0; i<NN; ++i) m.Node(i).m_ntag = -1;
	for (i=0; i<NF; ++i)
	{
		FEFace& f = m.Face(i);
		if (f.IsSelected())
		{
			int n = f.Nodes();
			m.Node(f.node[0]).m_ntag = 1;
			m.Node(f.node[1]).m_ntag = 1;
			m.Node(f.node[2]).m_ntag = 1;
			if (n==4) m.Node(f.node[3]).m_ntag = 1;
		}
	}

	fprintf(fp, "*NODE\n");
	int n = 1;
	for (i=0; i<NN; ++i)
	{
		FENode& node = m.Node(i);
		if (node.m_ntag == 1)
		{
			node.m_ntag = n++;
			fprintf(fp, "%8d%16g%16g%16g\n", node.m_ntag, node.m_rt.x, node.m_rt.y, node.m_rt.z);
		}
	}

	fprintf(fp, "*ELEMENT_SHELL\n");
	int l = 1;
	for (i=0; i<NF; ++i)
	{
		FEFace& f = m.Face(i);
		if (f.IsSelected())
		{
			int n[4], nf = f.Nodes();
			n[0] = m.Node(f.node[0]).m_ntag;
			n[1] = m.Node(f.node[1]).m_ntag;
			n[2] = m.Node(f.node[2]).m_ntag;
			if (nf == 4) n[3] = m.Node(f.node[3]).m_ntag; else n[3] = n[2];

			fprintf(fp, "%8d%8d%8d%8d%8d%8d\n", l, 1, n[0], n[1], n[2], n[3]);
			l++;
		}
	}

	if (m_bnode)
	{
		FEState* ps = fem.GetState(ntime);

		fprintf(fp, "*NODAL_RESULTS\n");
		for (int i=0; i<NN; ++i)
		{
			FENode& node = m.Node(i);
			if (node.m_ntag != -1)
			{
				double v = ps->m_NODE[i].m_val;
				fprintf(fp, "%8d%16lg\n", node.m_ntag, v);
			}
		}
	}

	fprintf(fp, "*END\n");
	fclose(fp);

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAExport::ExportMesh(FEModel& fem, int ntime, const char* szfile)
{
	int i, j;

	// open the file
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;
	fprintf(fp, "*KEYWORD\n");
	
	// tag all nodes that will be exported
	FEMeshBase& m = *fem.GetFEMesh(0);
	int NN = m.Nodes();
	int NE = m.Elements();
	for (i=0; i<NN; ++i) m.Node(i).m_ntag = -1;
	for (i=0; i<NE; ++i)
	{
		FEElement& e = m.Element(i);
		if ((m_bsel == false) || (e.IsSelected()))
		{
			int ne = e.Nodes();
			for (j=0; j<ne; ++j) m.Node(e.m_node[j]).m_ntag = 1;
		}
	}
	for (i=0; i<NN; ++i) if (m.Node(i).m_ntag != -1) m.Node(i).m_ntag = i+1;

	// export nodes
	fprintf(fp, "*NODE\n");
	for (i=0; i<NN; ++i)
	{
		FENode& node = m.Node(i);
		if (node.m_ntag != -1)
		{
			fprintf(fp, "%8d%16g%16g%16g\n", node.m_ntag, node.m_rt.x, node.m_rt.y, node.m_rt.z);
		}
	}

	// export nodal data (optional)
	if (m_bnode) NodalResults(fem, ntime, fp);

	// export solid elements
	if (m.SolidElements() > 0)
	{
		fprintf(fp, "*ELEMENT_SOLID\n");
		for (i=0; i<NE; ++i)
		{
			FEElement& e = m.Element(i);
			if (e.IsSolid())
			{
				if ((m_bsel == false) || (e.IsSelected()))
				{
					int ne = e.Nodes();
					int n[8];
					switch (e.Type())
					{
					case FE_TET4:
						n[0] = e.m_node[0];
						n[1] = e.m_node[1];
						n[2] = e.m_node[2];
						n[3] = e.m_node[3];
						n[4] = n[5] = n[6] = n[7] = n[3];
						break;
					case FE_PENTA6:
						n[0] = e.m_node[0];
						n[1] = e.m_node[1];
						n[2] = e.m_node[2];
						n[3] = e.m_node[3];
						n[4] = e.m_node[4];
						n[5] = e.m_node[5];
						n[6] = n[7] = n[5];
						break;
					case FE_HEX8:
						n[0] = e.m_node[0];
						n[1] = e.m_node[1];
						n[2] = e.m_node[2];
						n[3] = e.m_node[3];
						n[4] = e.m_node[4];
						n[5] = e.m_node[5];
						n[6] = e.m_node[6];
						n[7] = e.m_node[7];
						break;
					case FE_PYRA5:
						n[0] = e.m_node[0];
						n[1] = e.m_node[1];
						n[2] = e.m_node[2];
						n[3] = e.m_node[3];
						n[4] = e.m_node[4];
						break;
					}
					for (j=0; j<8; ++j) n[j] = m.Node(n[j]).m_ntag;
					fprintf(fp, "%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d\n", i+1, e.m_MatID+1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7]);
				}
			}
		}
	}

	// export shell elements
	if (m.ShellElements() > 0)
	{
		fprintf(fp, "*ELEMENT_SHELL\n");
		for (i=0; i<NE; ++i)
		{
			FEElement& e = m.Element(i);
			if (e.IsShell())
			{
				if ((m_bsel == false) || (e.IsSelected()))
				{
					int ne = e.Nodes();
					int n[4];
					switch (e.Type())
					{
					case FE_TRI3:
						n[0] = e.m_node[0];
						n[1] = e.m_node[1];
						n[2] = e.m_node[2];
						n[3] = n[2];
						break;
					case FE_QUAD4:
						n[0] = e.m_node[0];
						n[1] = e.m_node[1];
						n[2] = e.m_node[2];
						n[3] = e.m_node[3];
						break;
					}
					for (j=0; j<4; ++j) n[j] = m.Node(n[j]).m_ntag;
					fprintf(fp, "%8d%8d%8d%8d%8d%8d\n", i+1, e.m_MatID+1, n[0], n[1], n[2], n[3]);
				}
			}
		}
	}

	// finalize file
	fprintf(fp, "*END\n");
	fclose(fp);

	return true;
}

//-----------------------------------------------------------------------------
void FELSDYNAExport::NodalResults(FEModel &fem, int ntime, FILE* fp)
{
	FEMeshBase& m = *fem.GetFEMesh(0);
	int NN = m.Nodes();
	FEState* ps = fem.GetState(ntime);

	fprintf(fp, "*NODAL_RESULTS\n");
	for (int i=0; i<NN; ++i)
	{
		FENode& node = m.Node(i);
		if (node.m_ntag != -1)
		{
			double v = ps->m_NODE[i].m_val;
			fprintf(fp, "%8d%16lg\n", node.m_ntag, v);
		}
	}
}
