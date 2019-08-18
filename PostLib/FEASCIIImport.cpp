#include "stdafx.h"
#include "FEASCIIImport.h"
#include "FEDataManager.h"
#include "FEMeshData_T.h"
#include <cctype>
#include <stdlib.h>
using namespace Post;

#ifdef LINUX // same for Linux and Mac OS X
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

//-----------------------------------------------------------------------------
FEASCIIImport::ZONE::ZONE()
{
	szname[0] = 0;
}

//-----------------------------------------------------------------------------
FEASCIIImport::ZONE::ZONE(const ZONE& z)
{
	strcpy(szname, z.szname);
	m_npack = z.m_npack;
	m_nn = z.m_nn;
	m_ne = z.m_ne;
	m_ntype = z.m_ntype;
	m_ftime = z.m_ftime;

	m_Node = z.m_Node;
	m_Elem = z.m_Elem;
}

//-----------------------------------------------------------------------------
void FEASCIIImport::ZONE::operator = (const FEASCIIImport::ZONE& z)
{
	strcpy(szname, z.szname);
	m_npack = z.m_npack;
	m_nn = z.m_nn;
	m_ne = z.m_ne;
	m_ntype = z.m_ntype;
	m_ftime = z.m_ftime;

	m_Node = z.m_Node;
	m_Elem = z.m_Elem;
}

//-----------------------------------------------------------------------------
FEASCIIImport::FEASCIIImport(void) : FEFileReader("ASCII Data")
{
	m_sztitle[0] = 0;
	m_szline[0] = 0;
}

FEASCIIImport::~FEASCIIImport(void)
{
}

bool FEASCIIImport::Load(FEModel &fem, const char *szfile)
{
	// open the text file
	if (Open(szfile, "rt") == false) return false;

	// get the first line
	if (getline(m_szline, 255) == false) return errf("Error reading file.");

	// make sure we start with a clean slate
	m_Zone.clear();

	// read the zone info
	do
	{
		// see what we are dealing with
		if (strnicmp(m_szline, "TITLE", 5) == 0)
		{
			if (ReadTitle() == false) return errf("Error reading title.");
		}
		else if (strnicmp(m_szline, "VARIABLES", 9) == 0)
		{
			if (ReadVariables() == false) return errf("Error reading variables.");
		}
		else if (strnicmp(m_szline, "ZONE", 4) == 0)
		{
			if (ReadZone() == false) return errf("Error reading zone data.");
		}
		else return errf("Error reading file.");

		// get the next line
		getline(m_szline, 255);
	}
	while (!feof(m_fp)&&!ferror(m_fp));

	// close the file
	Close();

	// set the title
	const char* sztitle = m_sztitle;
	if (sztitle[0] == 0) sztitle = m_Zone[0].szname;
	if (sztitle[0] == 0)
	{
		const char* ch = strrchr(szfile, '/');
		if (ch == 0)
		{
			ch = strrchr(szfile, '\\'); 
			if (ch == 0) ch = szfile; else ch++;
		}
		else ch++;
		sztitle = ch;
	}
	fem.SetTitle(m_sztitle);

	// build the mesh
	return BuildMesh(fem);
}

//-----------------------------------------------------------------------------
// Reads a line from the file, skipping comment lines
bool FEASCIIImport::getline(char* szline, int nmax)
{
	do
	{
		fgets(szline, nmax, m_fp);
		if (ferror(m_fp) || feof(m_fp)) return false;
	}
	while (szline[0] == '#');

	// replace the eol with a zero
	char* eol = strrchr(szline, '\xA');
	if (eol) *eol = 0;
	eol = strrchr(szline, '\xD');
	if (eol) *eol = 0;

	return true;
}

//-----------------------------------------------------------------------------
char* next_item(char* ch)
{
	char* c = ch;
	while ((*c!=0)&&((*c==' ')||(*c==',')||(*c=='\t'))) ++c;
	return c;
}

//-----------------------------------------------------------------------------
// Reads a string, assuming the string is defined by double quotes (")
char* read_string(char* ch, char* sz)
{
	if (*ch!='"') return 0;
	ch++;
	while (*ch != '"')
	{
		if (*ch == 0) return 0;
		*sz++ = *ch++;
	}
	ch++;
	*sz = 0;
	return ch;
}

//-----------------------------------------------------------------------------
// Reads a keyword
char* read_key(char* ch, char* sz)
{
	if (*ch=='"') return read_string(ch, sz);
	while (isalnum(*ch)) { *sz++ = *ch++; }
	*sz = 0;
	return ch;
}

//-----------------------------------------------------------------------------
// is this a valid character in a number string
bool isnum(char c)
{
	return (((c>='0')&&(c<='9'))||(c=='.')||(c=='e')||(c=='E')||(c=='+')||(c=='-'));
}

//-----------------------------------------------------------------------------
// reads a numeric value
char* read_num(char* ch, char* sz)
{
	while (isnum(*ch))
	{
		*sz++ = *ch++;
	}
	*sz = 0;
	return ch;
}

//-----------------------------------------------------------------------------
// Read the title
bool FEASCIIImport::ReadTitle()
{
	strcpy(m_sztitle, m_szline);
	return true;
}

//-----------------------------------------------------------------------------
// Read the VARIABLES section (Skips the TITLE as well)
bool FEASCIIImport::ReadVariables()
{
	// read the variables
	char* ch = strchr(m_szline, '=');
	if (ch == 0) return false;
	ch++;

	m_Var.clear();
	VARIABLE var;

	// parse the line
	ch = next_item(ch);
	while (*ch)
	{
		// get the next entry
		if (*ch != '"') return false;

		// parse the string value
		ch = read_string(ch, var.szname);
		if (ch == 0) return false;

		// add the variable
		m_Var.push_back(var);

		// get the next line
		ch = next_item(ch);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEASCIIImport::ReadZone()
{
	ZONE zone;

	// read the zone info
	if (ReadZoneInfo(zone) == false) return false;

	// read the nodes
	if (ReadNodes(zone) == false) return false;

	// read the elements
	if (ReadElements(zone) == false) return false;

	// add the zone
	m_Zone.push_back(zone);

	return true;
}

//-----------------------------------------------------------------------------
// Reads the ZONE section
bool FEASCIIImport::ReadZoneInfo(FEASCIIImport::ZONE& zone)
{
	// make sure it is the ZONE keyword
	if (strnicmp(m_szline, "ZONE", 4) != 0) return false;
	char* ch = m_szline;
	ch += 4;

	char szkey[256] = {0}, szval[256] = {0};

	// get the next item
	ch = next_item(ch);
	while (*ch)
	{
		// read the keyword
		ch = read_key(ch, szkey);

		// find the equal sign
		ch = next_item(ch);
		if (*ch != '=') return false;
		ch++;

		// find the value
		ch = next_item(ch);

		// read the value
		ch = read_key(ch, szval);

		// read the keyword
		if (stricmp(szkey, "T") == 0)
		{
			strcpy(zone.szname, szval);
		}
		else if (stricmp(szkey, "DATAPACKING") == 0)
		{
			if (stricmp(szval, "POINT") != 0) return false;
			zone.m_npack = 0;
		}
		else if (stricmp(szkey, "N") == 0)
		{
			zone.m_nn = atoi(szval);
		}
		else if (stricmp(szkey, "E") == 0)
		{
			zone.m_ne = atoi(szval);
		}
		else if (stricmp(szkey, "SOLUTIONTIME") == 0)
		{
			zone.m_ftime = atof(szval);
		}
		else if (stricmp(szkey, "ZONETYPE") == 0)
		{
			if      (strcmp(szval, "FETRIANGLE"     ) == 0) zone.m_ntype = ZONE_TRIANGLE;
			else if (strcmp(szval, "FEQUADRILATERAL") == 0) zone.m_ntype = ZONE_QUAD;
			else if (strcmp(szval, "FETETRAHEDRON"  ) == 0) zone.m_ntype = ZONE_TET;
			else if (strcmp(szval, "FEBRICK"        ) == 0) zone.m_ntype = ZONE_BRICK;
			else return false;
		}
		else return false;

		// get the next entry
		ch = next_item(ch);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read the nodal values
bool FEASCIIImport::ReadNodes(FEASCIIImport::ZONE& zone)
{
	char szval[256] = {0};

	// allocate nodes
	zone.m_Node.resize(zone.m_nn);

	// read the nodes
	for (int i=0; i<zone.m_nn; ++i)
	{
		if (getline(m_szline, 255) == false) return false;

		NODE& n = zone.m_Node[i];

		char* ch = m_szline;
		int nvar = (int) m_Var.size();
		for (int j=0; j<nvar; ++j)
		{
			ch = next_item(ch);
			if (*ch==0) return false;

			ch = read_num(ch, szval);

			double v = atof(szval);
			n.v[j] = (float) v;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FEASCIIImport::ReadElements(FEASCIIImport::ZONE& zone)
{
	char szval[256] = {0};

	// allocate nodes
	zone.m_Elem.resize(zone.m_ne);

	int nmust;
	switch (zone.m_ntype)
	{
	case ZONE_TRIANGLE: nmust = 3; break;
	case ZONE_QUAD    : nmust = 4; break;
	case ZONE_TET     : nmust = 4; break;
	case ZONE_BRICK   : nmust = 8; break;
	default:
		return false;
	}

	// read the elements
	int NN = zone.m_nn;
	for (int i=0; i<zone.m_ne; ++i)
	{
		if (getline(m_szline, 255) == false) return false;

		ELEM& e = zone.m_Elem[i];

		char* ch = m_szline;
		for (int j=0; j<nmust; ++j)
		{
			ch = next_item(ch);
			if (*ch==0) return false;

			ch = read_num(ch, szval);

			int n = atoi(szval);
			if ((n <= 0)||( n > NN)) return false;
			e.node[j] = n;
		}
	}
	return true;
}

bool FEASCIIImport::BuildMesh(FEModel &fem)
{
	// make sure we have at least one zone
	if (m_Zone.empty()) return false;

	// create the mesh
	ZONE& zone = m_Zone[0];
	FEMeshBase* pm = 0;
	switch (zone.m_ntype)
	{
	case ZONE_TRIANGLE: pm = new FETriMesh; break;
	case ZONE_QUAD    : pm = new FEQuadMesh; break;
	case ZONE_TET     : pm = new FEMeshTet4; break;
	case ZONE_BRICK   : pm = new FEMeshHex8; break;
	default:
		return false;
	}

	pm->Create(zone.m_nn, zone.m_ne);
	fem.AddMesh(pm);

	// assign nodes
	for (int i=0; i<zone.m_nn; ++i)
	{
		FENode& n = pm->Node(i);
		float* v = zone.m_Node[i].v;
		n.m_r0 = n.m_rt = vec3f(v[0], v[1], v[2]);
	}

	// assign elements
	for (int i=0; i<zone.m_ne; ++i)
	{
		FEElement& e = pm->Element(i);
		int* n = zone.m_Elem[i].node;
		for (int j=0; j<e.Nodes(); ++j) e.m_node[j] = n[j]-1;
	}
	pm->Update();
	fem.UpdateBoundingBox();

	// create a single material
	fem.ClearMaterials();
	FEMaterial m;
	m.diffuse = GLColor(192,192,192);
	m.ambient = GLColor(192,192,192);
	m.specular = GLColor(0,0,0);
	m.emission = GLColor(0,0,0);
	m.shininess = 1.0f;
	m.transparency = 1.f;
	m.benable = true;
	m.bvisible = true;
	m.bmesh = true;
	m.bcast_shadows = true;
	fem.AddMaterial(m);

	// create the variables
	FEDataManager* pdm = fem.GetDataManager();
	pdm->Clear();
	list<VARIABLE>::iterator pv = m_Var.begin();
	for (int i=0; i<(int) m_Var.size(); ++i, ++pv)
	{
		pdm->AddDataField(new FEDataField_T<FENodeData<float> >(pv->szname, EXPORT_DATA)); 
	}

	// create states
	for (int i=0; i<(int)m_Zone.size(); ++i)
	{
		// get the next zone
		ZONE& zone = m_Zone[i];

		// make sure we have the same number of nodes
		if (zone.m_nn != pm->Nodes()) break;

		// create a state for this zone
		FEState* ps = new FEState(zone.m_ftime, &fem, fem.GetFEMesh(0));
		fem.AddState(ps);

		// set the state data
		for (int j=0; j<(int) m_Var.size(); ++j)
		{
			FENodeData<float>& v = dynamic_cast<FENodeData<float>&>(ps->m_Data[j]);
			for (int k=0; k<zone.m_nn; ++k)
			{
				v[k] = zone.m_Node[k].v[j];
			}
		}
	}

	return true;
}
