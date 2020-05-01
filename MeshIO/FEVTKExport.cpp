#include "FEVTKExport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>

FEVTKExport::FEVTKExport(FEProject& prj) : FEFileExport(prj)
{
}

FEVTKExport::~FEVTKExport(void)
{
}

bool FEVTKExport::Write(const char* szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	bool isPOLYDATA = false;
	bool isUnstructuredGrid = false;
	bool isTriShell = false;
	bool isQuadShell = false;
	bool isHex8 = false;
	bool isTet4 = false;
	bool isTet10 = false;

	FEModel* ps = &m_prj.GetFEModel();
	GModel& model = ps->GetModel();

	int totElems = 0;
	int nodesPerElem = 0;

	for (int i=0; i<model.Objects(); ++i)
	{
		FEMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		FEMesh& m = *pm;

		//tags all nodes as -1
		m.TagAllNodes(-1);

		totElems += m.Elements();

		// tags all the nodes in elements as 1
		for (int j=0; j<m.Elements(); ++j)
		{
			FEElement &el = m.Element(j);
			for (int k=0; k<el.Nodes(); ++k)
				m.Node(el.m_node[k]).m_ntag = 1;

			//check element type
			if (j==0)
			{
				int elType = el.Type();
				switch(elType)
				{
				case FE_TRI3:
					isPOLYDATA = true;
					isTriShell = true;
					nodesPerElem = 3;
					break;
				case FE_QUAD4:
					isPOLYDATA = true;
					isQuadShell = true;
					nodesPerElem = 4;
					break;
				case FE_HEX8:
					isHex8 = true;
					isUnstructuredGrid = true;
					nodesPerElem = 8;
					break;
				case FE_TET4:
					isTet4 = true;
					isUnstructuredGrid = true;
					nodesPerElem = 4;
					break;
				case FE_TET10:
					isTet10 = true;
					isUnstructuredGrid = true;
					nodesPerElem = 10;
					break;
				default:
					return errf("Only triangular, quadrilateral, tetrahedral, and hexahedron polygons are supported.");
				}
			}
		}
	}

	// tag the node as per the node number
	int nodes = 0;
	for (int i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (int j=0; j<m.Nodes(); ++j)
		{
			if (m.Node(j).m_ntag == 1) 
			{
				m.Node(j).m_ntag = nodes;
				nodes++;
			}
		}
	}

	// --- H E A D E R ---
	fprintf(fp, "%s\n" , "# vtk DataFile Version 3.0");
	fprintf(fp, "%s\n" ,"vtk output");
	fprintf(fp, "%s\n" ,"ASCII");
	if(isPOLYDATA)
		fprintf(fp, "%s\n" ,"DATASET POLYDATA");
	if(isUnstructuredGrid)
		fprintf(fp, "%s\n" ,"DATASET UNSTRUCTURED_GRID");
	fprintf(fp, "%s %d %s\n" ,"POINTS", nodes ,"float");

	//fprintf(fp, "%d %d %d %d\n", parts, nodes, faces, edges);

	// --- N O D E S ---
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh& m = *po->GetFEMesh();
		for (int j=0; j<m.Nodes(); )
		{
			for (int k =0; k<3 && j+k<m.Nodes();k++)
			{
				FENode& n = m.Node(j+k);
				vec3d r = m.LocalToGlobal(n.r);
				fprintf(fp, "%g %g %g ", r.x, r.y, r.z);
			}
			fprintf(fp, "\n");
			j = j + 3;				
		}
	}
	fprintf(fp, "%s\n" ,"");


	// --- E L E M E N T S ---


	if (isPOLYDATA)
		fprintf(fp, "%s %d %d\n", "POLYGONS", totElems, totElems * (nodesPerElem + 1));
	if (isUnstructuredGrid)
		fprintf(fp, "%s %d %d\n", "CELLS", totElems, totElems * (nodesPerElem + 1));

	int nn[FEElement::MAX_NODES];
	for (int i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (int j=0; j<m.Elements(); ++j)
		{
			FEElement& el = m.Element(j);
			for (int k=0; k<el.Nodes(); ++k) 
				nn[k] = m.Node(el.m_node[k]).m_ntag;

			switch (el.Type())
			{
			case FE_TRI3:
				fprintf(fp, "%d %d %d %d\n", el.Nodes(), nn[0], nn[1], nn[2]);
				break;
			case FE_QUAD4:
				fprintf(fp, "%d %d %d %d %d\n", el.Nodes(),nn[0], nn[1], nn[2], nn[3]);
				break;
			case FE_TET4:
				fprintf(fp, "%d %d %d %d %d\n", el.Nodes(), nn[0], nn[1], nn[2], nn[3]);
				break;
			case FE_HEX8:
				fprintf(fp, "%d %d %d %d %d %d %d %d %d\n", el.Nodes(),nn[0], nn[1], nn[2], nn[3], nn[4], nn[5], nn[6], nn[7]);
				break;
			case FE_TET10:
				fprintf(fp, "%d %d %d %d %d %d %d %d %d %d %d\n", el.Nodes(), nn[0], nn[1], nn[2], nn[3], nn[4], nn[5], nn[6], nn[7], nn[8], nn[9]);
				break;
			default:
				return false;
			}
		}
	}
	//----Shell Thickness ----
	if (m_ops.bshellthick)
	{
		fprintf(fp, "%s\n" ,"");
		fprintf(fp, "%s %d\n" ,"POINT_DATA", nodes);
		fprintf(fp, "%s %s %s\n" ,"SCALARS", "ShellThickness", "float");
		fprintf(fp,"%s\n","LOOKUP_TABLE default");
		for (int i=0; i<model.Objects(); ++i)
		{
			FEMesh& m = *model.Object(i)->GetFEMesh();

			vector<double> nodeShellThickness; 
			nodeShellThickness.reserve(nodes);
			int nn[8];
			for (int k = 0 ; k< nodes;k++)
				nodeShellThickness.push_back(0);

			for (int j=0; j<m.Elements(); ++j)
			{
				FEElement& el = m.Element(j);
				if (!el.IsType(FE_TRI3) && !el.IsType(FE_QUAD4))
					break;

				for (int k=0; k<el.Nodes(); ++k) 
						nn[k] = el.m_node[k];

				double* h = el.m_h;
			
				for(int k =0;k<el.Nodes();k++)
					nodeShellThickness[nn[k]] = h[k];

			}

			for (int j=0; j<m.Nodes();)
			{
				for (int k =0; k<9 && j+k<m.Nodes();k++)
					fprintf(fp, "%15.10lg ", nodeShellThickness[j+k]);	
				fprintf(fp, "\n");
				j = j + 9;	
			}
		}
	}

	//-----Nodal Data-----------
	if (m_ops.bscalar_data)
	{
/*		fprintf(fp, "%s\n" ,"");
		fprintf(fp, "%s %d\n" ,"POINT_DATA", nodes);
		fprintf(fp, "%s %s %s\n" ,"SCALARS", "ScalarData", "float");
		fprintf(fp,"%s\n","LOOKUP_TABLE default");
		for (i=0; i<model.Objects(); ++i)
		{
			FEMesh& m = *model.Object(i)->GetFEMesh();
			for (j=0; j<m.Nodes();)
			{
				for (int k =0; k<9 && j+k<m.Nodes();k++)
				{
					FENode& n = m.Node(j+k);
						fprintf(fp, "%15.10lg ", n.m_ndata);
				}
				fprintf(fp, "\n");
				j = j + 9;	
			}
		}
*/
	}

	//cell type
	if(isHex8 || isTet4 || isTet10)
	{
		fprintf(fp, "%s\n" ,"");		
		fprintf(fp, "%s %d\n", "CELL_TYPES", totElems);
		for (int i=0; i<model.Objects(); ++i)
		{
			FEMesh& m = *model.Object(i)->GetFEMesh();
			for (int j=0; j<m.Elements(); ++j)
			{
				if (isHex8) fprintf(fp, "%s\n", "12");
				if (isTet4) fprintf(fp, "%s\n", "10");
				if (isTet10) fprintf(fp, "%s\n", "24");
			}
		}	
	}

	fclose(fp);

	return true;
}
