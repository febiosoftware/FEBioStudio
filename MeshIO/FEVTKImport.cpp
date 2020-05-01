#include "FEVTKImport.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>

FEVTKimport::FEVTKimport(FEProject& prj) : FEFileImport(prj)
{
}

FEVTKimport::~FEVTKimport(void)
{
}

bool FEVTKimport::Load(const char* szfile)
{
	FEModel& fem = m_prj.GetFEModel();

	if (!Open(szfile, "rt")) return errf("Failed opening file %s.", szfile);

	char szline[256] = {0}, *ch;
	int nread, i,j,k;
	char type[256];
	double temp[9];
	bool isASCII=false, isPOLYDATA = false, isUnstructuredGrid = false, isCellData = false,isScalar=false,isShellThickness=false;
	// read the header
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		if (strstr(ch, "ASCII") != 0) isASCII = true;
		if (strstr(ch, "POLYDATA") != 0) isPOLYDATA = true;
		if (strstr(ch, "UNSTRUCTURED_GRID") != 0) isUnstructuredGrid = true;
	}
	while(strstr(ch, "POINTS") == 0);
	if (!isASCII)
		return errf("Only ASCII files are read");

	//get number of nodes
	int nodes = atoi(ch+6);
	
	int size = 0;
	int nparts = 0;
	int edges = 0;
	int elems = 0;
	if (nodes <= 0) return errf("Invalid number of nodes.");
	

	// create a new mesh
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, 0);
	
	// read the nodes
	//Check how many nodes are there in each line
	int nodesRead = 0;
	while (nodesRead < nodes)
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");

		nread = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5], &temp[6], &temp[7], &temp[8]);
		if (nread % 3 != 0 && nread > 9)
			return errf("An error occured while reading the nodal coordinates.");

		int nodes_in_row = nread / 3;

		k = 0;
		for (j=0; j<nodes_in_row; ++j)
		{
			FENode& n = pm->Node(nodesRead++);
			vec3d& r = n.r;
			r.x = temp[k    ];
			r.y = temp[k + 1];
			r.z = temp[k + 2];
			k += 3;
		}
	}
	assert(nodesRead == nodes);

	//Reading element data
	while(1)
	{		
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		if (strstr(ch, "POLYGONS") != 0 || strstr(ch, "CELLS") != 0 ) 
		{
			sscanf(szline, "%s %d %d",type,&elems,&size);
			break;
		}
		ch = fgets(szline, 255, m_fp);
	}

	if(elems == 0||size == 0)
		return errf("Only POLYGON/CELL dataset format is supported.");

	pm->Create(0, elems);

	// read the elements
	int n[FEElement::MAX_NODES + 1];
	for (i=0; i<elems; ++i)
	{	
		FEElement& el = pm->Element(i);
		el.m_gid = 0;
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		nread = sscanf(szline, "%d%d%d%d%d%d%d%d%d%d%d", &n[0], &n[1], &n[2], &n[3], &n[4],&n[5],&n[6],&n[7],&n[8], &n[9], &n[10]);
		int min = 0;
		switch (n[0])
		{
		case 3: 
			el.SetType(FE_TRI3);
			el.m_node[0] = n[1]-min;
			el.m_node[1] = n[2]-min;
			el.m_node[2] = n[3]-min;
			break;
		case 4:			
			if(isPOLYDATA)
				el.SetType(FE_QUAD4);
			if(isUnstructuredGrid)
				el.SetType(FE_TET4);
			el.m_node[0] = n[1]-min;
			el.m_node[1] = n[2]-min;
			el.m_node[2] = n[3]-min;
			el.m_node[3] = n[4]-min;
			break;
		case 8:
			el.SetType(FE_HEX8);
			el.m_node[0] = n[1]-min;
			el.m_node[1] = n[2]-min;
			el.m_node[2] = n[3]-min;
			el.m_node[3] = n[4]-min;
			el.m_node[4] = n[5]-min;
			el.m_node[5] = n[6]-min;
			el.m_node[6] = n[7]-min;
			el.m_node[7] = n[8]-min;
			break;
		case 10:
			el.SetType(FE_TET10);
			el.m_node[0] = n[ 1] - min;
			el.m_node[1] = n[ 2] - min;
			el.m_node[2] = n[ 3] - min;
			el.m_node[3] = n[ 4] - min;
			el.m_node[4] = n[ 5] - min;
			el.m_node[5] = n[ 6] - min;
			el.m_node[6] = n[ 7] - min;
			el.m_node[7] = n[ 8] - min;
			el.m_node[8] = n[ 9] - min;
			el.m_node[9] = n[10] - min;
			break;
		default:
			delete pm;
			return errf("Only triangular, quadrilateral and hexahedron polygons are supported.");
		}
	}
	ch = fgets(szline, 255, m_fp);

	while (ch != NULL) //making sure the file doesn't ends here
	{
		//reading the point data
		do
		{	
			if (ch == NULL) break;
			if (ch == 0) return errf("An unexpected error occured while reading the file data.");
			if (strstr(ch, "POINT_DATA") != 0)
				size = atoi(ch+10);
			if(strstr(ch,"CELL_DATA")!=0)
			{
				size = atoi(ch + 9);
				isCellData = true;
			}
			if (strstr(ch, "SCALARS") != 0) isScalar = true;
			if (strstr(ch, "ShellThickness") != 0) isShellThickness = true;
			ch = fgets(szline, 255, m_fp);
		}
		while(ch == NULL || strstr(ch, "LOOKUP_TABLE") == 0);

		if(!isScalar && ch !=NULL)
			return errf("Only scalar data is supported.");	
		vector<double> data; 
		//reading shell thickness
		if(isShellThickness)
		{			
			data.reserve(size);
			//Check how many nodes are there in each line
			ch = fgets(szline, 255, m_fp);
			if (ch == 0) return errf("An unexpected error occured while reading the file data.");

			int nodes_each_row = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &temp[0],&temp[1],&temp[2], &temp[3],&temp[4],&temp[5], &temp[6],&temp[7],&temp[8]);
			if (nodes_each_row>9) 
				return errf("An error occured while reading the nodal coordinates.");
			double temp2 = double(size)/nodes_each_row;
			int rows = ceil(temp2);
			for (i=0; i<rows; ++i)
			{	
				for (j=0;j<nodes_each_row && i*nodes_each_row+j <size;j++)
				{
					data.push_back(temp[j]);
				}
				ch = fgets(szline, 255, m_fp);
				if (ch == 0 && i+1 != rows) 
					return errf("An unexpected error occured while reading the scalar data.");

				nread = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &temp[0],&temp[1],&temp[2], &temp[3],&temp[4],&temp[5], &temp[6],&temp[7],&temp[8]);
				if (nread > 9 && nread != -1)
				{ 			
					if (i+1 != rows)
						return errf("An error occured while reading the scalar.");
				}
			}
			//Assigning the scalar feild to the mesh
			//if scalar feild is shell thickness
			int nn[8];
			for (i=0; i<elems; ++i)
			{
				FEElement& el = pm->Element(i);
				for (int k=0; k<el.Nodes(); ++k) 
					nn[k] = el.m_node[k];
				double	h[4];
				for(int k =0;k<3;k++)
					h[k] = data[nn[k]];
				if(el.IsType(FE_QUAD4))
					h[3] = data[nn[3]];

				el.m_h[0] = h[0];
				el.m_h[1] = h[1];
				el.m_h[2] = h[2];
				if(el.IsType(FE_QUAD4))
					el.m_h[3] = h[3];				
			}
		}
		//reading cell data
		if(isCellData)
		{
			for (i = 0; i<size;)
			{
				ch = fgets(szline, 255, m_fp);
				if (ch == 0) return errf("An unexpected error occured while reading the file data.");
				nread = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5], &temp[6], &temp[7], &temp[8]);
				for (int j = 0; j<nread; ++j, ++i)
				{
					FEElement& el = pm->Element(i);
					el.m_gid = (int)temp[j];
				}
			}
		}
		break;
	}

	Close();

	pm->RebuildMesh(60.0, true, true);

	GMeshObject* po = new GMeshObject(pm);
	po->Update();

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
