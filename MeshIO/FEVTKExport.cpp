#include "FEVTKExport.h"
#include <GeomLib/GObject.h>

FEVTKExport::FEVTKExport(void)
{
}

FEVTKExport::~FEVTKExport(void)
{
}

bool FEVTKExport::Export(FEProject& prj, const char* szfile)
{
	int i, j, k;
	bool  isPOLYDATA = false, isUnstructuredGrid = false, isTriShell = false, isQuadShell = false, isHex8 = false;
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	FEModel* ps = &prj.GetFEModel();
	GModel& model = ps->GetModel();

	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		FEMesh& m = *pm;
		//tags all nodes as 0
		for (j=0; j<m.Nodes(); ++j) 
			m.Node(j).m_ntag = 0;
		// tags all the nodes in elements as 1
		for (j=0; j<m.Elements(); ++j)
		{
			FEElement &el = m.Element(j);
			for (k=0; k<el.Nodes(); ++k)
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
					break;
				case FE_QUAD4:
					isPOLYDATA = true;
					isQuadShell = true;
					break;
				case FE_HEX8:
					isHex8 = true;
					isUnstructuredGrid = true;
					break;
				default:
					delete pm;
					return errf("Only triangular, quadrilateral and hexahedron polygons are supported.");
				}
			}
		}
	}

	// tag the node as per the node number
	int nodes = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Nodes(); ++j) 
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
	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Nodes(); )
		{
			for (int k =0; k<3 && j+k<m.Nodes();k++)
			{
				FENode& n = m.Node(j+k);
				fprintf(fp, "%g %g %g ", n.r.x, n.r.y, n.r.z);
			}
			fprintf(fp, "\n");
			j = j + 3;				
		}
	}
	fprintf(fp, "%s\n" ,"");


	// --- E L E M E N T S ---	
	int nn[8];
	for (i=0; i<model.Objects(); ++i)
	{
		FEMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Elements(); ++j)
		{
			FEElement& el = m.Element(j);
			for (int k=0; k<el.Nodes(); ++k) 
				nn[k] = m.Node(el.m_node[k]).m_ntag;

			if(j==0)
			{
				if(isPOLYDATA)
					fprintf(fp, "%s %d %d\n" ,"POLYGONS", m.Elements(), m.Elements() * (el.Nodes()+1));
				if(isUnstructuredGrid)
					fprintf(fp, "%s %d %d\n" ,"CELLS", m.Elements(), m.Elements() * (el.Nodes()+1));
			}

			switch (el.Type())
			{
			case FE_TRI3:
				fprintf(fp, "%d %d %d %d\n", el.Nodes(), nn[0], nn[1], nn[2]);
				break;
			case FE_QUAD4:
				fprintf(fp, "%d %d %d %d %d\n", el.Nodes(),nn[0], nn[1], nn[2], nn[3]);
				break;
			case FE_HEX8:
				fprintf(fp, "%d %d %d %d %d %d %d %d %d\n", el.Nodes(),nn[0], nn[1], nn[2], nn[3], nn[4], nn[5], nn[6], nn[7]);
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
		for (i=0; i<model.Objects(); ++i)
		{
			FEMesh& m = *model.Object(i)->GetFEMesh();

			vector<double> nodeShellThickness; 
			nodeShellThickness.reserve(nodes);
			int nn[8];
			for (int k = 0 ; k< nodes;k++)
				nodeShellThickness.push_back(0);

			for (j=0; j<m.Elements(); ++j)
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
			for (i=0; i<model.Objects(); ++i)
			{
				FEMesh& m = *model.Object(i)->GetFEMesh();
				for (j=0; j<m.Nodes();)
				{
					for (int k =0; k<9 && j+k<m.Nodes();k++)
						fprintf(fp, "%15.10lg ", nodeShellThickness[j+k]);	
					fprintf(fp, "\n");
					j = j + 9;	
				}
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

	//celll type
	if(isHex8)
	{
		fprintf(fp, "%s\n" ,"");		
		for (i=0; i<model.Objects(); ++i)
		{
			FEMesh& m = *model.Object(i)->GetFEMesh();
			for (j=0; j<m.Elements(); ++j)
			{
				if(j == 0)
				{
					fprintf(fp, "%s %d\n" ,"CELL_TYPES", m.Elements());
				}
				fprintf(fp,"%s\n","12");
			}
		}	
	}

	fclose(fp);

	return true;
}
