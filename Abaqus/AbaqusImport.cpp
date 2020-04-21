// AbaqusImport.cpp: implementation of the AbaqusImport class.
//
//////////////////////////////////////////////////////////////////////

#include "AbaqusImport.h"
#include <stdlib.h>
#include <GeomLib/GMeshObject.h>
#include <vector>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FESurfaceLoad.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/GModel.h>
using namespace std;

//-----------------------------------------------------------------------------
AbaqusImport::AbaqusImport()
{
	// default options
	m_bnodesets = true;
	m_belemsets = true;
	m_bfacesets = true;
	m_bautopart = false;
	m_bautosurf = true;
	m_breadPhysics = false;
}

//-----------------------------------------------------------------------------
AbaqusImport::~AbaqusImport()
{
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_line(char* szline, FILE* fp)
{
	// read a line but skip over comments (i.e.lines that start with **)
	do
	{
		fgets(szline, 255, fp);
		++m_nline;
		if (feof(fp)) return false;
	}
	while ((szline[0] == '\n') || (szline[0] == '\r') || (strncmp(szline,"**", 2) == 0));

	// remove the eof line charachter
	char* ch = strrchr(szline, '\n');
	if (ch) *ch = 0;

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::skip_keyword(char* szline, FILE* fp)
{
	do
	{
		if (read_line(szline, fp) == false) return false;
	}
	while (szline[0] != '*');
	return true;
}

//-----------------------------------------------------------------------------
// compare two strings, not considering case
bool szicmp(const char* sz1, const char* sz2)
{
	int l1 = (int)strlen(sz1);
	int l2 = (int)strlen(sz2);
	if (l1 < l2) return false;
	int n1 = 0, n2 = 0;

	char c1, c2;

	do 
	{
		c1 = sz1[n1++];
		c2 = sz2[n2++];

		if ((c1 >= 'A') && (c1 <= 'Z')) c1 = 'a' + (c1 - 'A');
		if ((c2 >= 'A') && (c2 <= 'Z')) c2 = 'a' + (c2 - 'A');
		if (c1 != c2) return false;
	}
	while ((n1 < l1) && (n2 < l2));

	return true;
}

//-----------------------------------------------------------------------------
//! Load an Abaqus model file
bool AbaqusImport::Load(FEProject& prj, const char* szfile)
{
	FEModel& fem = prj.GetFEModel();
	m_pprj = &prj;
	m_pfem = &fem;

	m_nline = 0;

	// try to open the file
#ifdef LINUX
	fprintf(stderr, "Reading file %s\n", szfile);
#endif

	// try to open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s", szfile);

	// parse the file
	try
	{
		if (parse_file(m_fp) == false) return false;
	}
	catch (...)
	{
		return false;
	}

	// build the model
	if (build_model() == false) return false;

	// set the name of the mesh
	GModel& model = fem.GetModel();
	if (model.Objects())
	{
		char szname[256] = {0};
		strcpy(szname, szfile);
		char* ch = strrchr(szname, '.');
		if (ch) *ch = 0;
		ch = strrchr(szname, '/');
		if (ch == 0) ch = strrchr(szname, '\\'); else ++ch;
		if (ch == 0) ch = szname; else ++ch;
		GObject* po = model.Object(model.Objects()-1);
		po->SetName(ch);
	}

	// we're good!
	return true;
}

//-----------------------------------------------------------------------------
//! Parse an abaqus model file
bool AbaqusImport::parse_file(FILE* fp)
{
	// get the first line
	char szline[256];
	if (!read_line(szline, fp)) return errf("Error while reading file");

	// parse the keywords
	while (!feof(fp))
	{
		// find what keyword this is
		if (szicmp(szline, "*HEADING"))	// read the heading
		{
			if (!read_heading(szline, fp)) return errf("Error while reading keyword HEADING (line %d)", m_nline);
		}
		else if (szicmp(szline, "*NODE PRINT"))
		{
			// we need to read this otherwise, the NODE reader gets messed up
			read_line(szline, fp);
		}
		else if (szicmp(szline, "*NODE OUTPUT"))
		{
			// we need to read this otherwise, the NODE reader gets messed up
			read_line(szline, fp);
		}
		else if (szicmp(szline, "*NODE")) // read nodes
		{
			if (!read_nodes(szline, fp)) return errf("Error while reading keyword NODE (line %d)", m_nline);
		}
		else if (szicmp(szline, "*NGEN")) // read node generation
		{
			if (!read_ngen(szline, fp)) return errf("Error while reading keyword NGEN (line %d)", m_nline);
		}
		else if (szicmp(szline, "*NFILL")) // read nfill
		{
			if (!read_nfill(szline, fp)) return errf("Error while reading keyword NFILL (line %d)", m_nline);
		}
		else if (szicmp(szline, "*ELEMENT OUTPUT"))
		{
			// we need to read this otherwise, the ELEMENT reader gets messed up
			read_line(szline, fp);
		}
		else if (szicmp(szline, "*SOLID SECTION"))
		{
			if (!read_solid_section(szline, fp)) return errf("Error while reading keyword SOLID SECTION (line %d)", m_nline);
		}
		else if (szicmp(szline, "*ELEMENTSET")) // read element sets
		{
			if (!read_element_sets(szline, fp)) return errf("Error while reading keyword ELEMENTSET (line %d)", m_nline);			
		}
		else if (szicmp(szline, "*ELEMENT")) // read elements
		{
			if (!read_elements(szline, fp)) return errf("Error while reading keyword ELEMENT (line %d)", m_nline);
		}
		else if (szicmp(szline, "*ELSET")) // read element sets
		{
			if (!read_element_sets(szline, fp)) return errf("Error while reading keyword ELSET (line %d)", m_nline);			
		}
		else if (szicmp(szline, "*NSET")) // read element sets
		{
			if (!read_node_sets(szline, fp)) return errf("Error while reading keyword NSET (line %d)", m_nline);			
		}
		else if (szicmp(szline, "*SURFACE BEHAVIOR"))
		{
			// read the next line
			read_line(szline, fp);
		}
		else if (szicmp(szline, "*SURFACE INTERACTION"))
		{
			if (!read_surface_interaction(szline, fp)) return errf("Error while reading keyword SURFACE INTERACTION (line %d)", m_nline);
		}
		else if (szicmp(szline, "*SURFACE")) // read surfaces
		{
			if (!read_surface(szline, fp)) return errf("Error while reading keyword SURFACE (line %d)", m_nline);
		}
		else if (szicmp(szline, "*MATERIAL")) // read materials
		{
			if (!read_materials(szline, fp)) return errf("Error while reading keyword MATERIAL (line %d)", m_nline);
		}
		else if (szicmp(szline, "*PART")) // read parts
		{
			if (!read_part(szline, fp)) return errf("Error while reading keyword PART (line %d)", m_nline);
		}
		else if (szicmp(szline,"*END PART") || szicmp(szline, "*ENDPART"))
		{
			if (!read_end_part(szline, fp)) return errf("Error while reading keyword END PART (line %d)", m_nline);
		}
		else if (szicmp(szline, "*INSTANCE"))
		{
			if (!read_instance(szline, fp)) return errf("Error while reading keyword INSTANCE (line %d)", m_nline);
		}
		else if (szicmp(szline, "*END INSTANCE"))
		{
			if (!read_end_instance(szline, fp)) return errf("Error while reading keyword END INSTANCE (line %d)", m_nline);
		}
		else if (szicmp(szline, "*STEP"))
		{
			if (!read_step(szline, fp)) 
			{
				return errf("Error while reading keyword STEP (line %d)", m_nline);
			}
		}
/*		else if (szicmp(szline, "*BOUNDARY"))
		{
			if (!read_boundary(szline, fp)) return errf("Error while reading keyword BOUNDARY (line %d)", m_nline);
		}
*/		else if (szicmp(szline, "*DSLOAD"))
		{
			if (!read_dsload(szline, fp)) return errf("Error while reading keyword DSLOAD (line %d)", m_nline);
		}
		else if (szicmp(szline, "*ORIENTATION"))
		{
			if (!read_orientation(szline, fp)) return errf("Error while reading keyword ORIENTATION (line %d)", m_nline);
		}
		else if (szicmp(szline, "*DISTRIBUTION TABLE"))
		{
			read_line(szline, fp);
		}
		else if (szicmp(szline, "*DISTRIBUTION"))
		{
			if (!read_distribution(szline, fp)) return errf("Error while reading keyword DISTRIBUTION (line %d)", m_nline);
		}
		else if (szicmp(szline, "*INCLUDE")) // include another file
		{
			// get the filename
			char* szfile = strrchr(szline, '=');
			if (szfile == 0) return false; else ++szfile;

#ifdef LINUX
			fprintf(stderr, "Reading file %s\n", szfile);
#endif
			// try to open the file
			FILE* fpi = fopen(szfile, "rt");
			if (fpi == 0) return errf("Failed including %s\n", szfile);

			// parse the file
			bool bret = parse_file(fpi);

			// close the file
			fclose(fpi);

			if (bret == false) return false;

			// read the next line
			read_line(szline, fp);
		}
		else
		{
			// read the next line
			read_line(szline, fp);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

int AbaqusImport::parse_line(const char* szline, ATTRIBUTE* pa)
{
	char c;
	int n = 0;
	int l = (int)strlen(szline);
	int m = 0;
	int k = 0;
	char* sz = pa[n].szatt;
	pa[0].szval[0] = 0;
	while (m<l)
	{
		c = szline[m];
		switch (c)
		{
		case ',':	// go to the next attribute
			++n;
			sz[k] = 0;
			k = 0;
			sz = pa[n].szatt;
			pa[n].szval[0] = 0;
			break;
		case '"':
			{
				++m;
				while (m<l && ((c=szline[m])!='"')) { sz[k++] = c; ++m; }
			}
			break;
		case '=':	// read the value
			sz[k] = 0;
			k = 0;
			sz = pa[n].szval;
			break;
		case ' ': // skip spaces
			break;
		default:	// copy text
			sz[k++] = c;
		}

		++m;							
	}
	sz[k] = 0;
	++n;

	return n;
}

//-----------------------------------------------------------------------------

const char* AbaqusImport::find_attribute(AbaqusImport::ATTRIBUTE* pa, int nmax, const char* szatt)
{
	for (int i=0; i<nmax; ++i)
	{
		if (szicmp(pa[i].szatt, szatt)) return pa[i].szval;
	}
	return 0;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_heading(char* szline, FILE* fp)
{
	int n = 0;
	do
	{
		fgets(szline, 255, fp);
		if (feof(fp)) return false;

		if (n == 0) strncpy(m_szTitle, szline, AbaqusModel::Max_Title);
	}
	while (szline[0] != '*');

	return true;
}


//-----------------------------------------------------------------------------

bool AbaqusImport::read_nodes(char* szline, FILE* fp)
{
	// parse the szline for optional parameters
	ATTRIBUTE att[4];
	parse_line(szline, att);

	// get the active part
	AbaqusModel::PART& part = *m_inp.GetActivePart(true);

	// read the nodes
	AbaqusModel::NODE n;
	n.x = n.y = n.z = 0;
	read_line(szline, fp);
	char* ch;
	int nc = 0;
	while (!feof(fp) && (szline[0] != '*'))
	{
		// parse the line
		ch = szline;
		sscanf(ch, "%d", &n.id);

		ch = strchr(ch, ',');
		if (ch == 0) return false;
		++ch;
		sscanf(ch, "%lg", &n.x);

		ch = strchr(ch, ','); 
		if (ch==0) return false;
		++ch;
		sscanf(ch, "%lg", &n.y);

		ch = strchr(ch, ','); 
		if (ch==0) return false;
		++ch;
		sscanf(ch, "%lg", &n.z);

		// add the node to the list
		part.AddNode(n);

		// read the next line
		read_line(szline, fp);
	}

	// build the node-look up table
	part.BuildNLT();

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_ngen(char* szline, FILE* fp)
{
	int i;

	// get the active part
	AbaqusModel::PART& part = *m_inp.GetActivePart();

	// scan the element line for optional parameters
	ATTRIBUTE att[7];
	int natt = parse_line(szline, att);

	// check the parameters
	int nline = 0;
	for (i=1; i<natt; ++i)
	{
		if (szicmp(att[i].szatt, "LINE"))
		{
			const char* sz = att[i].szval;
			if      (szicmp(sz, "C")) nline = 1;
			else return errf("line type %s not supported", sz);
		}
	}

	// read the next line
	read_line(szline, fp);

	int l1, l2, lc, linc;
	while (!feof(fp) && (szline[0] != '*'))
	{
		// parse the line
		sscanf(szline, "%d,%d,%d,%d", &l1, &l2, &linc, &lc);
		AbaqusModel::Tnode_itr pn1 = part.FindNode(l1);
		AbaqusModel::Tnode_itr pn2 = part.FindNode(l2);

		// generate the nodes
		AbaqusModel::NODE n;
		double t;
		if (nline==0)
		{
			for (i=l1+linc; i<l2; i += linc)
			{
				t = (double) (i-l1)/(double) (l2 - l1);

				n.x = (1.0-t)*pn1->x + t*(pn2->x);
				n.y = (1.0-t)*pn1->y + t*(pn2->y);
				n.z = (1.0-t)*pn1->z + t*(pn2->z);

				n.id = i;
				part.AddNode(n);
			}
		}
		else if (nline==1)
		{
			AbaqusModel::Tnode_itr pc = part.FindNode(lc);

			vec3d m1(pn1->x - pc->x, pn1->y - pc->y, pn1->z - pc->z);
			vec3d m2(pn2->x - pc->x, pn2->y - pc->y, pn2->z - pc->z);
			double L1 = m1.Length();
			double L2 = m2.Length();
			m1.Normalize();
			m2.Normalize();
			quatd qt(m1, m2), q;
			vec3d p = qt.GetVector(), r;
			double w = qt.GetAngle(), L;

			for (i=l1+linc; i<l2; i += linc)
			{
				t = (double) (i-l1)/(double) (l2 - l1);
				L = (1.0 -t)*L1 + t*L2;
				q = quatd(t*w, p);
				r = m1;
				q.RotateVector(r);

				n.x = pc->x + L*r.x;
				n.y = pc->y + L*r.y;
				n.z = pc->z + L*r.z;
	
				n.id = i;
				part.AddNode(n);
			}
		}

		read_line(szline, fp);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_nfill(char* szline, FILE* fp)
{
	// get the active part
	AbaqusModel::PART& part = *m_inp.GetActivePart();

	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		char* ch1 = strchr(szline, ',');
		if (ch1) *ch1 = 0; else return false;
		map<string, AbaqusModel::NODE_SET>::iterator ns1 = part.FindNodeSet(szline);
		if (ns1 == part.m_NSet.end()) return false;
		char* ch2 = strchr(ch1+1, ',');
		if (ch2) *ch2 = 0; else return false;
		map<string, AbaqusModel::NODE_SET>::iterator ns2 = part.FindNodeSet(ch1 + 1);
		if (ns2 == part.m_NSet.end()) return false;

		int nl, ni;
		sscanf(ch2+1, "%d,%d", &nl, &ni);

		if (ns1->second.node.size() != ns2->second.node.size()) return false;

		int N = (int)ns1->second.node.size();
		AbaqusModel::NODE n;
		double t;
		for (int l=1; l<nl; ++l)
		{
			list<AbaqusModel::Tnode_itr>::iterator n1 = ns1->second.node.begin();
			list<AbaqusModel::Tnode_itr>::iterator n2 = ns2->second.node.begin();

			t = (double) l / (double) nl;

			for (int i=0; i<N; ++i, ++n1, ++n2)
			{
				n.id = (*n1)->id + l*ni + i;

				n.x = (*n1)->x*(1.0 - t) + (*n2)->x;
				n.y = (*n1)->y*(1.0 - t) + (*n2)->y;
				n.z = (*n1)->z*(1.0 - t) + (*n2)->z;

				part.AddNode(n);
			}
		}

		read_line(szline, fp);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_elements(char* szline, FILE* fp)
{
	// scan the element line for optional parameters
	ATTRIBUTE att[7];
	int natt = parse_line(szline, att);

	// check the parameters
	int ntype = -1;
	bool bsprings = false;
	const char* szset = 0;
	for (int i=1; i<natt; ++i)
	{
		if (szicmp(att[i].szatt, "TYPE"))
		{
			const char* sz = att[i].szval;
			if      (szicmp(sz, "C3D8"  )) ntype = FE_HEX8;
            else if (szicmp(sz, "C3D8H" )) ntype = FE_HEX8;
			else if (szicmp(sz, "C3D8I" )) ntype = FE_HEX8;
			else if (szicmp(sz, "C3D6"  )) ntype = FE_PENTA6;
			else if (szicmp(sz, "C3D4"  )) ntype = FE_TET4;
			else if (szicmp(sz, "C3D10" )) ntype = FE_TET10;
			else if (szicmp(sz, "C3D20R")) ntype = FE_HEX20;
			else if (szicmp(sz, "C3D20" )) ntype = FE_HEX20;
			else if (szicmp(sz, "R3D4"  )) ntype = FE_QUAD4;
			else if (szicmp(sz, "S4"    )) ntype = FE_QUAD4;
			else if (szicmp(sz, "S4R"   )) ntype = FE_QUAD4;
			else if (szicmp(sz, "S3"    )) ntype = FE_TRI3;
			else if (szicmp(sz, "S3R"   )) ntype = FE_TRI3;
			else if (szicmp(sz, "R3D3"  )) ntype = FE_TRI3;
			else if (szicmp(sz, "CPE3"  )) ntype = FE_TRI3;
            else if (szicmp(sz, "STRI3" )) ntype = FE_TRI3;
            else if (szicmp(sz, "STRI65")) ntype = FE_TRI6;
            else if (szicmp(sz, "S8R"   )) ntype = FE_QUAD8;
            else if (szicmp(sz, "S9R5"  )) ntype = FE_QUAD9;
			else if (szicmp(sz, "SPRINGA"))
			{
				ntype = -1;
				bsprings = true;
			}
			else return errf("Element type %s not supported", sz);
		}
		else if (szicmp(att[i].szatt, "ELSET"))
		{
			// set the set name (except if it's a spring)
			if (bsprings == false)
			{
				szset = att[i].szval;
			}
		}
	}

	// spring elements will be handled differently.
	if (bsprings) return read_spring_elements(szline, fp);

	// get the active part
	AbaqusModel::PART* pg = m_inp.GetActivePart();
	if (pg == 0) { skip_keyword(szline, m_fp); return true; }
	AbaqusModel::PART& part = *pg;

	// find the element set
	list<AbaqusModel::ELEMENT_SET>::iterator ps = part.m_ElSet.end();
	if (szset)
	{
		ps = part.FindElementSet(szset);
		if (ps == part.m_ElSet.end()) ps = part.AddElementSet(szset);
	}

	// read the elements
	read_line(szline, fp);
	AbaqusModel::ELEMENT el;
	AbaqusModel::Telem_itr pe;

	int N = 0;
	switch (ntype)
	{
	case FE_HEX8  : N = 8; break;
	case FE_PENTA6: N = 6; break;
	case FE_TET4  : N = 4; break;
	case FE_TET10 : N = 10; break;
	case FE_HEX20 : N = 20; break;
	case FE_QUAD4 : N = 4; break;
	case FE_TRI3  : N = 3; break;
    case FE_TRI6  : N = 6; break;
    case FE_QUAD8 : N = 8; break;
    case FE_QUAD9 : N = 9; break;
	default:
		assert(false);
		return false;
	};

	int nc = 0;
	while (!feof(fp) && (szline[0] != '*'))
	{
		// set the element type
		el.type = ntype;

		// parse the line
		char* ch = szline;

		// get the element id
		sscanf(ch, "%d", &el.id);
		ch = strchr(ch, ',');
		if (ch == 0) return false; else ++ch;

		// read the node numbers
		for (int i=0; i<N; ++i)
		{
			sscanf(ch, "%d", &el.n[i]);

			if (i!=N-1)
			{
				// find the next comma
				ch = strchr(ch, ',');
				if (ch == 0) return false;

				// if we've reached at the end of the line
				// then we load the next line
				if (strlen(ch) == 1)
				{
					read_line(szline, fp);
					ch = szline;
				}
				else ++ch;
			}
		}

		// make sure to copy the last node for triangles
		if (ntype == FE_TRI3) el.n[3] = el.n[2];

		// check for pyramid elements
		if (ntype == FE_HEX8)
		{
			if ((el.n[7] == el.n[4]) &&
				(el.n[6] == el.n[4]) && 
				(el.n[5] == el.n[4])) el.type = FE_PYRA5;
		}

		// add the element to the list
		part.AddElement(el);

		// add the element to the elementset
		if (ps != part.m_ElSet.end()) ps->elem.push_back(el.id);

		// read the next line
		read_line(szline, fp);
	}
	
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_spring_elements(char* szline, FILE* fp)
{
	// get the active part
	AbaqusModel::PART* pg = m_inp.GetActivePart();
	if (pg)
	{
		AbaqusModel::PART& part = *m_inp.GetActivePart();
		read_line(szline, fp);
		AbaqusModel::SPRING el;

		int nc = 0;
		while (!feof(fp) && (szline[0] != '*'))
		{
			// parse the line
			char* ch = szline;

			// get the element id
			sscanf(ch, "%d", &el.id);
			ch = strchr(ch, ',');
			if (ch == 0) return false; else ++ch;

			// read the node numbers
			const int N = 2;
			for (int i=0; i<N; ++i)
			{
				sscanf(ch, "%d", &el.n[i]);

				if (i!=N-1)
				{
					// find the next comma
					ch = strchr(ch, ',');
					if (ch == 0) return false;

					// if we've reached at the end of the line
					// then we load the next line
					if (strlen(ch) == 1)
					{
						read_line(szline, fp);
						ch = szline;
					}
					else ++ch;
				}
			}

			// add the spring element to the list
			part.AddSpring(el);

			// read the next line
			read_line(szline, fp);
		}
	}
	else
	{
		// springs can be defined outside of a part. I suspect that this is a mechanism
		// for connecting two parts with springs. This is currently somewhat of a problem
		// so for now we only support springs that are connected in the same part.
		// the format of the nodes is somewhat different in this case. It is:
		// spring number, part1_name.node_number, part2_name.node_number
		read_line(szline, fp);
		AbaqusModel::SPRING el;

		int nc = 0;
		while (!feof(fp) && (szline[0] != '*'))
		{
			ATTRIBUTE att[3];
			int natt = parse_line(szline, att);
			if (natt != 3) return false;

			// get the element id
			sscanf(att[0].szatt, "%d", &el.id);

			// the part
			AbaqusModel::PART* pg = 0;

			// read the node numbers
			const int N = 2;
			for (int i=0; i<N; ++i)
			{
				char* szatt = att[i+1].szatt;
				char* ch = strchr(szatt, '.');
				if (ch == 0) return false;
				*ch++ = 0;

				// find the part (and make sure both nodes are attached to the same part)
				if (pg == 0)
				{
					AbaqusModel::INSTANCE* pi = m_inp.FindInstance(szatt);
					if (pi == 0) return false;
					pg = pi->GetPart();
					if (pg == 0) return false;
				}
				else
				{
					AbaqusModel::INSTANCE* pi = m_inp.FindInstance(szatt);
					if (pi == 0) return false;
					if (pi->GetPart() != pg) return false;
				}

				// get the node number
				el.n[i] = atoi(ch);
			}

			// make sure we have a part
			if (pg == 0) return false;

			// add the spring element to the list
			pg->AddSpring(el);

			// read the next line
			read_line(szline, fp);
		}
	}
	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_element_sets(char* szline, FILE* fp)
{
	// read the attributes
	ATTRIBUTE att[5];
	int natt = parse_line(szline, att);

	char szname[256];

	// find the instance part (if any)
	AbaqusModel::PART* pg = 0;
	const char* szinst = find_attribute(att, 7, "instance");
	if (szinst)
	{
		AbaqusModel::INSTANCE* pinst = m_inp.FindInstance(szinst);
		if (pinst) pg = pinst->GetPart();
	}
	else pg = m_inp.GetActivePart();

	// get the part
	if (pg == 0) return false;
	AbaqusModel::PART& part = *pg;

	// check the attributes
	bool bgen = false;
	for (int i=0; i<natt; ++i)
	{
		if (szicmp(att[i].szatt, "GENERATE")) bgen = true;
		else if (szicmp(att[i].szatt, "ELSET"))
		{
			strcpy(szname, att[i].szval);
		}
		else if (szicmp(att[i].szatt, "ELEMENTSET"))
		{
			strcpy(szname, att[i].szval);
		}
	}

	// find the element set or create a new one
	list<AbaqusModel::ELEMENT_SET>::iterator pset = part.FindElementSet(szname);
	if (pset == part.m_ElSet.end()) pset = part.AddElementSet(szname);

	pset->part = pg;

	// read the lines
	if (bgen)
	{
		int n1, n2, n;
		read_line(szline, fp);
		AbaqusModel::Telem_itr it;
		while (!feof(fp) && (szline[0] != '*'))
		{
			// parse the line
			int nread = sscanf(szline, "%d,%d,%d", &n1, &n2, &n);
			if (nread == 2) n = 1;
	
			// add the elements to the list
			for (int i=n1; i<=n2; i += n)
			{
				it = part.FindElement(i); 
				if (it != part.m_Elem.end()) pset->elem.push_back(it->id);
			}

			// read the next line
			read_line(szline, fp);
		}
	}
	else
	{
		int n[16], nr;
		read_line(szline, fp);
		AbaqusModel::Telem_itr it;
		while (!feof(fp) && (szline[0] != '*'))
		{
			nr = sscanf(szline, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &n[0],&n[1],&n[2],&n[3],&n[4],&n[5],&n[6],&n[7],&n[8],&n[9],&n[10],&n[11],&n[12],&n[13],&n[14],&n[15]);
			for (int i=0; i<nr; ++i)
			{
				it = part.FindElement(n[i]);
				if (it != part.m_Elem.end()) pset->elem.push_back(it->id);
			}
			// read the next line
			read_line(szline, fp);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_node_sets(char* szline, FILE* fp)
{
	// read the attributes
	ATTRIBUTE att[7];
	int natt = parse_line(szline, att);

	// find the instance part (if any)
	AbaqusModel::PART* pg = 0;
	const char* szinst = find_attribute(att, 7, "instance");
	if (szinst)
	{
		AbaqusModel::INSTANCE* pinst = m_inp.FindInstance(szinst);
		if (pinst) pg = pinst->GetPart();
	}
	else pg = m_inp.GetActivePart();

	// get the part
	if (pg == 0) return false;
	AbaqusModel::PART& part = *pg;

	char szname[256];
	
	// check the attributes
	bool bgen = false;
	for (int i=0; i<natt; ++i)
	{
		if (szicmp(att[i].szatt, "GENERATE")) bgen = true;
		else if (szicmp(att[i].szatt, "NSET"))
		{
			strcpy(szname, att[i].szval);
		}
	}

	// read the lines
	if (bgen)
	{
		map<string, AbaqusModel::NODE_SET>::iterator pset = part.FindNodeSet(szname);
		if (pset == part.m_NSet.end()) 
		{
			pset = part.AddNodeSet(szname);
			pset->second.part = pg;
		}

		int n1, n2, n;
		read_line(szline, fp);
		while (!feof(fp) && (szline[0] != '*'))
		{
			// parse the line
			int nread = sscanf(szline, "%d,%d,%d", &n1, &n2, &n);
			if (nread == 2) n = 1;
	
			// add the elements to the list
			AbaqusModel::Tnode_itr it;
			for (int i=n1; i<=n2; i += n)
			{
				it = part.FindNode(i);
				if (it == part.m_Node.end()) return false;
				pset->second.node.push_back(it);
			}

			// read the next line
			read_line(szline, fp);
		}
	}
	else
	{
		int i, nr, n[16];
		AbaqusModel::Tnode_itr it;

		// get/create the node set
//		list<AbaqusModel::NODE_SET>::iterator pset = part.FindNodeSet(szname);
//		if (pset == part.m_NSet.end()) pset = part.AddNodeSet(szname);
		map<string, AbaqusModel::NODE_SET>::iterator pset = part.AddNodeSet(szname);
		pset->second.part = pg;

		read_line(szline, fp);
		while (!feof(fp) && (szline[0] != '*'))
		{
			// read the nodes
			nr = sscanf(szline, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&n[0],&n[1],&n[2],&n[3],&n[4],&n[5],&n[6],&n[7],&n[8],&n[9],&n[10],&n[11],&n[12],&n[13],&n[14],&n[15]);

			// add the elements to the list
			for (i=0; i<nr; ++i)
			{
				it = part.FindNode(n[i]);
				if (it == part.m_Node.end()) return false;
				pset->second.node.push_back(it);
			}

			// read the next line
			read_line(szline, fp);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_surface(char* szline, FILE* fp)
{
	// read the attributes
	ATTRIBUTE att[7];
	int natt = parse_line(szline, att);

	char szname[256];
	int ntype = AbaqusModel::ST_ELEMENT;

	// find the instance part (if any)
	AbaqusModel::PART* pg = 0;
	const char* szinst = find_attribute(att, 7, "instance");
	if (szinst)
	{
		AbaqusModel::INSTANCE* pinst = m_inp.FindInstance(szinst);
		if (pinst) pg = pinst->GetPart();
	}
	else pg = m_inp.GetActivePart();

	// check the attributes
	for (int i=0; i<natt; ++i)
	{
		if (szicmp(att[i].szatt, "NAME"))
		{
			strcpy(szname, att[i].szval);
		}
		else if (szicmp(att[i].szatt, "TYPE"))
		{
			if (szicmp(att[i].szval, "ELEMENT")) ntype = AbaqusModel::ST_ELEMENT;
			else ntype = -1;
		}
	}

	if (ntype != AbaqusModel::ST_ELEMENT) return false;

	// find the surface
	AbaqusModel::SURFACE* ps = 0;
	if (pg)
	{
		list<AbaqusModel::SURFACE>::iterator psi = pg->FindSurface(szname);
		if (psi == pg->m_Surf.end()) psi = pg->AddSurface(szname);

		ps = &(*psi);

		// store the part this surface belongs to
		ps->part = pg;
	}

	// read the surface
	read_line(szline, fp);
	AbaqusModel::FACE f;
	char* ch;
	int ne;
	int nf;
	while (!feof(fp) && (szline[0] != '*'))
	{
		// find the comma
		ch = strchr(szline, ',');
		if (ch == 0) return false;
		*ch = 0;

		if (szline[0] == '"')
		{
			char* ch2 = strchr(szline+1, '"');
			if (ch2 == 0) return false;
			int l = (int)(ch2 - szline);
			for (int i=0; i<l-1; ++i) szline[i] = szline[i+1];
			szline[l-1] = 0;
		}

		// get the face id
		ch = strchr(ch+1, 'S');
		if (ch) nf = atoi(ch+1); else return false;

		// first see it the line refers to an element set
		// loop over all the parts
		AbaqusModel::ELEMENT_SET* pset = m_inp.FindElementSet(szline);

		if (pset)
		{
			if (ps == 0)
			{
				pg = pset->part; if (pg == 0) return false;
				list<AbaqusModel::SURFACE>::iterator psi = pg->AddSurface(szname);
				if (psi == pg->m_Surf.end()) return false;

				ps = &(*psi);
				ps->part = pg;
			}
			else if (ps->part != pset->part) return false;

			// add all elements from the element set to the surface
			vector<int>::iterator pe = pset->elem.begin();
			for (int i=0; i<(int) pset->elem.size(); ++i, ++pe)
			{
				f.eid = *pe;
				f.nf = nf;
				ps->face.push_back(f);
			}
		}
		else
		{
			// get the element number
			ne = atoi(szline);
			f.eid = ne;

			// add it to the list
			f.nf = nf;
			ps->face.push_back(f);
		}

		// read the next line
		read_line(szline, fp);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_materials(char *szline, FILE *fp)
{
	AbaqusModel::MATERIAL& mat = *m_inp.AddMaterial("");
	mat.dens = 1.0;

	ATTRIBUTE a[10];
	int natt = parse_line(szline, a);
	const char* szname = find_attribute(a, natt, "NAME");
	if (szname) strcpy(mat.szname, szname);

	read_line(szline, fp);
	while (!feof(fp))
	{
		if (szicmp(szline, "*DENSITY"))
		{
			read_line(szline, fp);
			sscanf(szline, "%lg", &mat.dens);
		}
		else if (szicmp(szline, "*ELASTIC"))
		{
			mat.mattype = AbaqusModel::ELASTIC;
			natt = parse_line(szline, a);
			const char* sztype = find_attribute(a, natt, "TYPE");
			if (sztype && szicmp(sztype, "ISOTROPIC")) mat.ntype = 1;
			else if (sztype == nullptr) mat.ntype = 1;

			read_line(szline, fp);
			char* sz = szline;
			char* ch = strchr(sz, ',');
			int nmax = 2;
			int np = 0;
			do
			{
				if (ch) *ch = 0; 
				sscanf(sz, "%lg", &mat.d[np]);
				if (ch)
				{
					++np;
					sz = ch+1;
					ch = strchr(sz, ',');
				}
			}
			while (ch && (np < nmax));
		}
		else if (szicmp(szline, "*HYPERELASTIC"))
		{
			mat.mattype = AbaqusModel::HYPERELASTIC;
			natt = parse_line(szline, a);
			const char* sztype = a[1].szatt;
			if (sztype && szicmp(sztype, "NEOHOOKE")) mat.ntype = 1;

			read_line(szline, fp);
			char* sz = szline;
			char* ch = strchr(sz, ',');
			int nmax = 2;
			int np = 0;
			do
			{
				if (ch) *ch = 0;
				sscanf(sz, "%lg", &mat.d[np]);
				if (ch)
				{
					++np;
					sz = ch + 1;
					ch = strchr(sz, ',');
				}
				else sz = 0;
			} while (sz && (np < nmax));
		}
		else if (szicmp(szline, "*ANISOTROPIC HYPERELASTIC"))
		{
			mat.mattype = AbaqusModel::ANI_HYPERELASTIC;
			natt = parse_line(szline, a);
			const char* sztype = a[1].szatt;
			if (sztype && szicmp(sztype, "HOLZAPFEL")) mat.ntype = 1;

			read_line(szline, fp);
			char* sz = szline;
			char* ch = strchr(sz, ',');
			int nmax = 5;
			int np = 0;
			do
			{
				if (ch) *ch = 0;
				sscanf(sz, "%lg", &mat.d[np]);
				if (ch)
				{
					++np;
					sz = ch + 1;
					ch = strchr(sz, ',');
				}
				else sz = 0;
			} while (sz && (np < nmax));
		}
		else break;
		read_line(szline, fp);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_part(char* szline, FILE* fp)
{
	if (m_inp.CurrentPart()) return errf("Error in file: new part was started before END PART was detected. (line %d)", m_nline);
	ATTRIBUTE att[2];
	int natt = parse_line(szline, att);
	if (natt == 0) return errf("Error: You need to add a NAME attribute when defining a part. (line %d)", m_nline);

	if (szicmp(att[1].szatt, "NAME"))
	{
		m_inp.SetCurrentPart(m_inp.CreatePart(att[1].szval));
	}
	else return errf("ERROR: invalid attribute for PART keyword. (line %d)", m_nline);

	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_end_part(char* szline, FILE* fp)
{
	// make sure we are in a part defintion
	if (m_inp.CurrentPart() == 0) return errf("ERROR in file: END PART detected but no part was defined. (line %d)", m_nline);

	// close part entry
	m_inp.SetCurrentPart(0);
	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_instance(char* szline, FILE* fp)
{
	ATTRIBUTE att[4];
	int natt = parse_line(szline, att);

	if (m_inp.CurrentPart()) return errf("Instance encountered while reading part.");
	if (m_inp.CurrentInstance()) return errf("Instance encountered when other instance not closed.");

	// create a new instance
	AbaqusModel::INSTANCE* pInst = m_inp.AddInstance();

	// set the name
	pInst->SetName(att[1].szval);

	// find the part symbol
	m_inp.SetCurrentPart(0);
	if (szicmp("part", att[2].szatt))
	{
		// find the part
		const char* szpart = att[2].szval;
		AbaqusModel::PART* pg = m_inp.FindPart(szpart);
		if (pg == 0) return false;

		m_inp.SetCurrentPart(pg);
		pInst->SetPart(pg);
	}
	else return errf("Instance needs a part attribute.");

	// read translation data
	read_line(szline, fp);
	if (szline[0] != '*')
	{
		double x[3];
		// parse the line
		char* ch = szline;
		sscanf(ch, "%lg", &x[0]);

		ch = strchr(ch, ',');
		if (ch == 0) return false;
		++ch;
		sscanf(ch, "%lg", &x[1]);

		ch = strchr(ch, ','); 
		if (ch==0) return false;
		++ch;
		sscanf(ch, "%lg", &x[2]);

		pInst->SetTranslation(x);

		// read the next line
		read_line(szline, fp);
	}

	// read rotation data
	if (szline[0] != '*')
	{
		double R[7];
		// parse the line
		char* ch = szline; sscanf(ch, "%lg", &R[0]);

		ch = strchr(ch, ','); if (ch == 0) return false;
		++ch; sscanf(ch, "%lg", &R[1]);

		ch = strchr(ch, ','); if (ch==0) return false;
		++ch; sscanf(ch, "%lg", &R[2]);

		ch = strchr(ch, ','); if (ch==0) return false;
		++ch; sscanf(ch, "%lg", &R[3]);

		ch = strchr(ch, ','); if (ch==0) return false;
		++ch; sscanf(ch, "%lg", &R[4]);

		ch = strchr(ch, ','); if (ch==0) return false;
		++ch; sscanf(ch, "%lg", &R[5]);

		ch = strchr(ch, ','); if (ch==0) return false;
		++ch; sscanf(ch, "%lg", &R[6]);

		pInst->SetRotation(R);

		// read the next line
		read_line(szline, fp);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_end_instance(char* szline, FILE* fp)
{
	if (m_inp.CurrentInstance() == 0) return errf("end instance encountered with no active instance.");
	m_inp.ClearCurrentInstance();
	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_surface_interaction(char* szline, FILE* fp)
{
	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		read_line(szline, fp);
	}
	return true;
}

//=============================================================================

//-----------------------------------------------------------------------------
bool AbaqusImport::build_model()
{
	// first, build the mesh
	if (build_mesh() == false) return false;

	// then, add all physics
	if (m_breadPhysics)
	{
		if (build_physics() == false) return false;
	}

	// all done
	return true;
}

//-----------------------------------------------------------------------------
// Build the model geometry
bool AbaqusImport::build_mesh()
{
	FEModel& fem = *m_pfem;

	// if instances are defined, we create the geometry based on the instances
	list<AbaqusModel::INSTANCE*>& Inst = m_inp.InstanceList();
	if (Inst.empty() == false)
	{
		// loop over all instances
		list<AbaqusModel::INSTANCE*>::iterator pi;
		for (pi=Inst.begin(); pi!=Inst.end(); ++pi)
		{
			AbaqusModel::INSTANCE* pinst = *pi;

			// build a part
			GObject* po = build_part(pinst->GetPart());
			assert(po);
			if (po == 0) return false;

			// apply translation
			double t[3];
			pinst->GetTranslation(t);
			po->GetTransform().SetPosition(vec3d(t[0], t[1], t[2]));

			// apply rotation
			double R[7];
			pinst->GetRotation(R);
			if (R[6] != 0.0)
			{
				vec3d a = vec3d(R[0], R[1], R[2]);
				vec3d b = vec3d(R[3], R[4], R[5]);
				po->GetTransform().Rotate(a, b, R[6]);
			}

			// if we get here we are good to go!
			po->SetName((*pi)->GetName());
			fem.GetModel().AddObject(po);
		}
	}
	else
	{
		// loop over all parts
		list<AbaqusModel::PART*>& Part = m_inp.PartList();
		list<AbaqusModel::PART*>::iterator pi;
		for (pi=Part.begin(); pi!=Part.end(); ++pi)
		{
			// build a part
			GObject* po = build_part(*pi);
			if (po == 0) return false;

			// if we get here we are good to go!
			po->SetName((*pi)->GetName());
			fem.GetModel().AddObject(po);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::build_physics()
{
	FEModel& fem = *m_pfem;

	// add the materials
	list<AbaqusModel::MATERIAL>& Mat = m_inp.MaterialList();
	list<AbaqusModel::MATERIAL>::iterator pm = Mat.begin();
	for (int i = 0; i<(int)Mat.size(); ++i, ++pm)
	{
		FEMaterial* pmat = 0;
		switch (pm->mattype)
		{
		case AbaqusModel::ELASTIC:
			if (pm->ntype == 1)
			{
				pmat = new FEIsotropicElastic;
				pmat->SetFloatValue(FEIsotropicElastic::MP_DENSITY, pm->dens);
				pmat->SetFloatValue(FEIsotropicElastic::MP_E, pm->d[0]);
				pmat->SetFloatValue(FEIsotropicElastic::MP_v, pm->d[1]);
			}
			break;
		case AbaqusModel::HYPERELASTIC:
			if (pm->ntype == 1)
			{
				pmat = new FEIncompNeoHookean;
				pmat->SetFloatValue(FEIncompNeoHookean::MP_DENSITY, pm->dens);
				pmat->SetFloatValue(FEIncompNeoHookean::MP_G, 2.0*pm->d[0]);
				pmat->SetFloatValue(FEIncompNeoHookean::MP_K, 1.0 / pm->d[1]);
			}
			break;
		case AbaqusModel::ANI_HYPERELASTIC:
			if (pm->ntype == 1)
			{
				pmat = new FEGasserOgdenHolzapfelUC;
				pmat->SetFloatValue(FEGasserOgdenHolzapfelUC::MP_DENSITY, pm->dens);
				pmat->SetFloatValue(FEGasserOgdenHolzapfelUC::MP_C10, pm->d[0]);
				pmat->SetFloatValue(FEGasserOgdenHolzapfelUC::MP_K, 1.0 / pm->d[1]);
				pmat->SetFloatValue(FEGasserOgdenHolzapfelUC::MP_K1, pm->d[2]);
				pmat->SetFloatValue(FEGasserOgdenHolzapfelUC::MP_K2, pm->d[3]);
				pmat->SetFloatValue(FEGasserOgdenHolzapfelUC::MP_KAPPA, pm->d[4]);
			}
			break;
		}

		if (pmat)
		{
			GMaterial* pgm = new GMaterial(pmat);
			pgm->SetName(pm->szname);
			fem.AddMaterial(pgm);
		}
	}

	// assign materials to parts
	list<AbaqusModel::PART*>& parts = m_inp.PartList();
	list<AbaqusModel::PART*>::iterator pgi = parts.begin();
	for (pgi; pgi != parts.end(); ++pgi)
	{
		AbaqusModel::PART* pg = *pgi;
		GObject* po = pg->m_po;
		if (po)
		{
			int ssections = (int)pg->m_Solid.size();
			if (po->Parts() == ssections)
			{
				list<AbaqusModel::SOLID_SECTION>::iterator ssi = pg->m_Solid.begin();
				for (int i = 0; i < ssections; ++ssi, ++i)
				{
					AbaqusModel::SOLID_SECTION& ss = *ssi;
					GMaterial* pmat = fem.FindMaterial(ss.szmat);
					if (pmat) po->Part(i)->SetMaterialID(pmat->GetID());
				}
			}
		}
	}

	// add the steps
	list<AbaqusModel::STEP>& Step = m_inp.StepList();
	for (list<AbaqusModel::STEP>::iterator it = Step.begin(); it != Step.end(); ++it)
	{
		AbaqusModel::STEP& stepi = *it;
		FENonLinearMechanics* festep = new FENonLinearMechanics(&fem);
		STEP_SETTINGS& set = festep->GetSettings();
		set.dt = stepi.dt0;
		set.ntime = (int) (stepi.time / stepi.dt0 + 0.5);
		festep->SetName(stepi.szname);
		fem.AddStep(festep);
	}

	// add all boundary conditions
	list<AbaqusModel::BOUNDARY>& BCs = m_inp.BoundaryConditionList();
	list<AbaqusModel::BOUNDARY>::iterator bci;
	int n = 0;
	for (bci = BCs.begin(); bci != BCs.end(); ++bci)
	{
		AbaqusModel::BOUNDARY& bc = *bci;

		int ns = (int)bc.m_nodeSet.size();
		for (int i=0; i<ns; ++i, ++n)
		{
			FENodeSet* nset = build_nodeset(bc.m_nodeSet[i].nodeSet);
			if (nset)
			{
				FEPrescribedDisplacement* pbc = new FEPrescribedDisplacement(&fem, nset, bc.m_nodeSet[i].ndof - 1, bc.m_nodeSet[i].load);
				char szname[256] = { 0 };
				sprintf(szname, "bc_%d", n);
				pbc->SetName(szname);
				fem.GetStep(0)->AddComponent(pbc);
			}
		}
	}

	// add all surface loads
	list<AbaqusModel::DSLOAD>& SLoads = m_inp.SurfaceLoadList();
	list<AbaqusModel::DSLOAD>::iterator sl;
	n = 0;
	for (sl = SLoads.begin(); sl != SLoads.end(); ++sl)
	{
		AbaqusModel::DSLOAD& p = *sl;

		int ns = (int)p.m_surf.size();
		for (int i=0; i<ns; ++i, ++n)
		{
			FESurface* surface = build_surface(p.m_surf[i].surf);
			if (surface)
			{
				FEPressureLoad* pl = new FEPressureLoad(&fem, surface);
				char szname[256] = {0};
				sprintf(szname, "dsload_%d", n);
				pl->SetName(szname);
				pl->SetLoad(p.m_surf[i].load);

				// add it to the initial step
				fem.GetStep(0)->AddComponent(pl);
			}
			else return false;
		}
	}

	// clean up
	Mat.clear();

	return true;
}

//-----------------------------------------------------------------------------
// Build a part
GObject* AbaqusImport::build_part(AbaqusModel::PART* pg)
{
	FEModel& fem = *m_pfem;

	AbaqusModel::PART& part = *pg;
	assert(part.m_po == 0);

	// count nodes
	int nodes = part.Nodes();

	// count elements
	int elems = 0;
	for (AbaqusModel::Telem_itr it = pg->m_Elem.begin(); it != pg->m_Elem.end(); ++it) if (it->id != -1) elems++;

	if ((nodes == 0) || (elems == 0)) return 0;

	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	// copy nodes
	int i, j, n;
	int imin = 0, imax = 0;
	AbaqusModel::Tnode_itr pn = part.m_Node.begin();
	for (i=0; i<nodes; ++i, ++pn)
	{
		FENode& node = pm->Node(i);
		pn->n = i;
		node.r.x = pn->x;
		node.r.y = pn->y;
		node.r.z = pn->z;

		if (i==0 || (pn->id < imin)) imin = pn->id;
		if (i==0 || (pn->id > imax)) imax = pn->id;
	}

	// copy elements
	int *en;
	i = 0;
	for (AbaqusModel::Telem_itr pe = part.m_Elem.begin(); pe != part.m_Elem.end(); ++pe)
	{
		if (pe->id != -1)
		{
			FEElement& el = pm->Element(i);
			pe->lid = i++;
			int etype = pe->type;
			el.SetType(etype);
			el.m_gid = 0;
			n = el.Nodes();
			en = pe->n;
			pn = part.m_Node.begin();
			for (j=0; j<n; ++j) 
			{
				AbaqusModel::Tnode_itr pn = part.FindNode(en[j]);
				el.m_node[j] = pn->n;
			}
		}
	}

	// reset nodal ID's
	pn = part.m_Node.begin();
	for (i=0; i<nodes; ++i, ++pn) pn->id = i;

	// auto-partition
	int elsets = (int)part.m_ElSet.size();
	vector<int> index; index.assign(elsets, 0);

	int ssections = (int) part.m_Solid.size();
	if (ssections > 0)
	{
		list<AbaqusModel::SOLID_SECTION>::iterator pss = part.m_Solid.begin();
		for (i = 0; i<ssections; ++i, ++pss)
		{
			AbaqusModel::SOLID_SECTION& ss = *pss;
			list<AbaqusModel::ELEMENT_SET>::iterator esi = part.FindElementSet(ss.szelset);
			if (esi != part.m_ElSet.end())
			{
				int n = (int)esi->elem.size();
				vector<int>::iterator pe = esi->elem.begin();
				for (j = 0; j<n; ++j, ++pe)
				{
					assert(*pe != -1);
					int eid = part.FindElement(*pe)->lid;
					FEElement& el = pm->Element(eid);
					el.m_gid = i;
				}
			}

			AbaqusModel::Orientation* orient = part.FindOrientation(ss.szorient);
			if (orient)
			{
				AbaqusModel::Distribution* dist = part.FindDistribution(orient->szdist);
				if (dist)
				{
					// skip the first entry
					for (int j=1; j<dist->m_data.size(); ++j)
					{
						AbaqusModel::Distribution::ENTRY& e = dist->m_data[j];
						int eid = part.FindElement(e.elem)->lid;

						vec3d a = vec3d(e.val[0], e.val[1], e.val[2]);
						vec3d b = vec3d(e.val[3], e.val[4], e.val[5]);
						FEElement& el = pm->Element(eid);

						vec3d e1 = a; e1.Normalize();
						vec3d e3 = a ^ b; e3.Normalize();
						vec3d e2 = e3 ^ e1;

						mat3d Q;
						Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
						Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
						Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

						el.m_Qactive = true;
						el.m_Q = Q;
					}
				}
			}
		}
	}
	else
	{
		if (m_bautopart && (elsets > 0))
		{
			list<AbaqusModel::ELEMENT_SET>::iterator pes = part.m_ElSet.begin();
			for (i=0; i<elsets; ++i, ++pes)
			{
				int n = (int)pes->elem.size();
				vector<int>::iterator pe = pes->elem.begin();
				for (j=0; j<n; ++j, ++pe)
				{
					assert(*pe != -1);
					int eid = part.FindElement(*pe)->lid;
					FEElement& el = pm->Element(eid);
					el.m_gid = i;
				}
			}

			// since elements can belong to multiple sets, 
			// some parts may become empty when all elements in the
			// set also belong to another set. 
			// Therefore we reindex the element GID's
			for (int i=0; i<elems; ++i)
			{
				FEElement& el = pm->Element(i);
				if (el.m_gid >= 0) index[el.m_gid]++;
			}

			int ngid = 0;
			for (int i=0; i<elsets; ++i)
			{
				if (index[i] > 0) index[i] = ngid++;
				else index[i] = -1;
			}

			for (int i = 0; i<elems; ++i)
			{
				FEElement& el = pm->Element(i);
				if (el.m_gid >= 0)
				{
					el.m_gid = index[el.m_gid];
					assert(el.m_gid != -1);
				}
			}
		}
	}

	pm->RebuildMesh(60.0, m_bautosurf);

	// next, we will add the mesh surfaces. Before we 
	// can do that we need to build and update the mesh.
	// This is simply done by attaching the mesh to an object
	GMeshObject* po = new GMeshObject(pm);

	// rename the parts to correspond to the element sets
	if (ssections > 0)
	{
		list<AbaqusModel::SOLID_SECTION>::iterator pss = part.m_Solid.begin();
		for (i = 0; i<ssections; ++i, ++pss)
		{
			AbaqusModel::SOLID_SECTION& ss = *pss;
			po->Part(i)->SetName(ss.szelset);
		}
	}
	else if (m_bautopart)
	{
		list<AbaqusModel::ELEMENT_SET>::iterator pes = part.m_ElSet.begin();
		int m = 0;
		int elsets = (int)part.m_ElSet.size();
		for (i = 0; i<elsets; ++i, ++pes)
		{
			if (index[i] >= 0)
			{
				po->Part(m++)->SetName(pes->szname);
			}
		}
	}

	// read element sets
	if (m_belemsets)
	{
		int elsets = (int)part.m_ElSet.size();
		if (elsets)
		{
			list<AbaqusModel::ELEMENT_SET>::iterator pes = part.m_ElSet.begin();
			for (i=0; i<elsets; ++i, ++pes)
			{
				int n = (int)pes->elem.size();
				FEPart* pg = new FEPart(po);
				pg->SetName(pes->szname);
				vector<int>::iterator pe = pes->elem.begin();
				for (j=0; j<n; ++j, ++pe) pg->add(part.FindElement(*pe)->lid);
				po->AddFEPart(pg);
			}
		}
	}

	// read node sets
	if (m_bnodesets)
	{
		int nsets = (int)part.m_NSet.size();
		if (nsets)
		{
			map<string, AbaqusModel::NODE_SET>::iterator ns = part.m_NSet.begin();
			int nn;
			for (i=0; i<nsets; ++i, ++ns)
			{
				FENodeSet* pg = new FENodeSet(po);
				pg->SetName(ns->second.szname);
				list<AbaqusModel::Tnode_itr>::iterator pn = ns->second.node.begin();
				nn = (int) ns->second.node.size();
				for (j=0; j<nn; ++j, ++pn) pg->add((*pn)->id);
				po->AddFENodeSet(pg);
			}
		}
	}

	// save the object on the part
	part.m_po = po;

	// now we are ready to define the surfaces
	if (m_bfacesets)
	{
		int surfs = (int)part.m_Surf.size();
		if (surfs)
		{
			list<AbaqusModel::SURFACE>::iterator si = part.m_Surf.begin();
			for (i=0; i<surfs; ++i, ++si)
			{
				FESurface* ps = build_surface(&(*si));
				if (ps)
				{
					ps->SetName(si->szname);
					po->AddFESurface(ps);
				}
			}
		}
	}

	// define the springs
	if (pg->Springs() > 0)
	{
		int NS = pg->Springs();
		list<AbaqusModel::SPRING>::iterator ps;
		int n = 1;
		for (ps = pg->m_Spring.begin(); ps != pg->m_Spring.end(); ++ps, ++n)
		{
			AbaqusModel::SPRING& s = *ps;
			AbaqusModel::Tnode_itr pn1 = part.FindNode(s.n[0]);
			AbaqusModel::Tnode_itr pn2 = part.FindNode(s.n[1]);

			int n0 = po->MakeGNode(pn1->n);
			int n1 = po->MakeGNode(pn2->n);

			GLinearSpring* ps = new GLinearSpring(n0, n1);
			char szname[256];
			sprintf(szname, "Spring%02d", n);
			ps->SetName(szname);
			ps->GetParam(GLinearSpring::MP_E).SetFloatValue(0);

			fem.GetModel().AddDiscreteObject(ps);
		}
	}

	return po;
}

//-----------------------------------------------------------------------------
FESurface* AbaqusImport::build_surface(AbaqusModel::SURFACE* si)
{
	AbaqusModel::PART* part = si->part;
	if (part == 0) return 0;

	// make sure the part has an object assigned to it
	GMeshObject* po = dynamic_cast<GMeshObject*>(part->m_po);
	if (po == 0) return 0;

	FEMesh* pm = part->m_po->GetFEMesh();

	int nf, n;
	FESurface* ps = new FESurface(part->m_po);
	nf = (int)si->face.size();
	list<AbaqusModel::FACE>::iterator pf = si->face.begin();
	AbaqusModel::Telem_itr pe;
	for (int j = 0; j<nf; ++j, ++pf)
	{
		pe = part->FindElement(pf->eid);
		FEElement& el = pm->Element(pe->lid);

		if (el.IsType(FE_HEX8))
		{
			n = -1;
			switch (pf->nf)
			{
			case 1: n = el.m_face[4]; break;
			case 2: n = el.m_face[5]; break;
			case 3: n = el.m_face[0]; break;
			case 4: n = el.m_face[1]; break;
			case 5: n = el.m_face[2]; break;
			case 6: n = el.m_face[3]; break;
			}
			assert(n >= 0);
			ps->add(n);
		}
		else if (el.IsType(FE_PENTA6))
		{
			n = -1;
			switch (pf->nf)
			{
			case 1: n = el.m_face[3]; break;
			case 2: n = el.m_face[4]; break;
			case 3: n = el.m_face[0]; break;
			case 4: n = el.m_face[1]; break;
			case 5: n = el.m_face[2]; break;
			}
			assert(n >= 0);
			ps->add(n);
		}
		else if (el.IsType(FE_TET4))
		{
			n = -1;
			switch (pf->nf)
			{
			case 1: n = el.m_face[3]; break;
			case 2: n = el.m_face[0]; break;
			case 3: n = el.m_face[1]; break;
			case 4: n = el.m_face[2]; break;
			}
			assert(n >= 0);
			ps->add(n);
		}
		else if (el.IsType(FE_TET10))
		{
			n = -1;
			switch (pf->nf)
			{
			case 1: n = el.m_face[3]; break;
			case 2: n = el.m_face[0]; break;
			case 3: n = el.m_face[1]; break;
			case 4: n = el.m_face[2]; break;
			}
			assert(n >= 0);
			ps->add(n);
		}
		else if (el.IsType(FE_QUAD4))
		{
			n = el.m_face[0];
			ps->add(n);
		}
		else if (el.IsType(FE_QUAD8))
		{
			n = el.m_face[0];
			ps->add(n);
		}
		else if (el.IsType(FE_QUAD9))
		{
			n = el.m_face[0];
			ps->add(n);
		}
		else if (el.IsType(FE_TRI3))
		{
			n = el.m_face[0];
			ps->add(n);
		}
		else if (el.IsType(FE_TRI6))
		{
			n = el.m_face[0];
			ps->add(n);
		}
	}
	
	return ps;
}

// build a nodeset
FENodeSet* AbaqusImport::build_nodeset(AbaqusModel::NODE_SET* ns)
{
	AbaqusModel::PART* part = ns->part;
	if (part == 0) return 0;

	// make sure the part has an object assigned to it
	GMeshObject* po = dynamic_cast<GMeshObject*>(part->m_po);
	if (po == 0) return 0;

	FEMesh* pm = part->m_po->GetFEMesh();

	FENodeSet* nset = new FENodeSet(po);
	list<AbaqusModel::Tnode_itr>::iterator it = ns->node.begin();
	for (it; it != ns->node.end(); ++it)
	{
		nset->add((*it)->n);
	}
	return nset;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_step(char* szline, FILE* fp)
{
	ATTRIBUTE att[5];
	parse_line(szline, att);

	const char* szname = find_attribute(att, 5, "name");
	AbaqusModel::STEP* step = m_inp.AddStep(szname);
	step->dt0 = 1.0;
	step->time = 1;

	// parse till END STEP
	while (!feof(fp))
	{
		if (szicmp(szline, "*STATIC"))
		{
			if (!read_static(szline, fp)) return false;
		}
		else if (szicmp(szline, "*DSLOAD"))
		{
			if (!read_dsload(szline, fp)) return false;
		}
		else if (szicmp(szline, "*BOUNDARY"))
		{
			if (!read_boundary(szline, fp)) return false;
		}
		else if (szicmp(szline, "*END STEP")) 
		{
			m_inp.SetCurrentStep(0);
			return true;
		}
		else read_line(szline, fp);
	}

	return false;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_boundary(char* szline, FILE* fp)
{
	AbaqusModel::BOUNDARY BC;
	ATTRIBUTE att[4];
	read_line(szline, fp);

	AbaqusModel::NODE_SET* dummy = nullptr;
	int ndof = -1;
	double val = 0.0;

	while (!feof(fp) && (szline[0] != '*'))
	{
		int n = parse_line(szline, att);
		if (n == 4)
		{
			const char* szset = att[0].szatt;
			AbaqusModel::NODE_SET* ns = m_inp.FindNodeSet(szset);
			ndof = atoi(att[1].szatt);
			val = atof(att[3].szatt);
			if (ns == nullptr)
			{
				int nid = atoi(szset);

				AbaqusModel::PART* part = m_inp.CurrentPart();
				if (part == nullptr) return false;

				if (dummy == nullptr)
				{
					dummy = &part->AddNodeSet("_unnamed")->second;
					dummy->part = part;
				}

				dummy->node.push_back(part->FindNode(nid));
			}
			else BC.add(ns, ndof, val);
		}
		read_line(szline, fp);
	}

	if (dummy) BC.add(dummy, ndof, val);

	m_inp.AddBoundaryCondition(BC);

	return true;	
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_dsload(char* szline, FILE* fp)
{
	AbaqusModel::DSLOAD P;
	ATTRIBUTE att[4];
	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		int n = parse_line(szline, att);
		if (n == 3)
		{
			const char* szsurf = att[0].szatt;
			AbaqusModel::SURFACE* s = m_inp.FindSurface(szsurf);
			double val = atof(att[2].szatt);

			if (s) P.add(s, val);
			else return false;
		}
		read_line(szline, fp);
	}

	m_inp.AddPressureLoad(P);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_solid_section(char* szline, FILE* fp)
{
	ATTRIBUTE att[5];
	int n = parse_line(szline, att);

	AbaqusModel::PART* pg = m_inp.GetActivePart(true);
	if (pg == 0) return false;

	const char* szelset = find_attribute(att, 5, "elset");
	if (szelset == 0) return false;

	const char* szmat    = find_attribute(att, 5, "material");
	const char* szorient = find_attribute(att, 5, "orientation");
	pg->AddSolidSection(szelset, szmat, szorient);

	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_static(char* szline, FILE* fp)
{
	// read the next line
	read_line(szline, fp);
	if (szline[0] == '*') return true;
	// parse it
	ATTRIBUTE att[5];
	parse_line(szline, att);

	AbaqusModel::STEP* step = m_inp.CurrentStep();
	if (step == 0) return false;

	step->dt0 = atof(att[0].szatt);
	step->time = atof(att[1].szatt);

	read_line(szline, fp);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_orientation(char* szline, FILE* fp)
{
	ATTRIBUTE att[5];
	parse_line(szline, att);
	const char* szname = find_attribute(att, 3, "name");
	read_line(szline, fp);

	AbaqusModel::PART* pg = m_inp.CurrentPart();
	if (pg == 0) return false;

	pg->AddOrientation(szname, szline);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_distribution(char* szline, FILE* fp)
{
	ATTRIBUTE att[8];
	parse_line(szline, att);
	const char* szname = find_attribute(att, 5, "name");
	if (szname == 0) return false;

	AbaqusModel::Distribution D;
	strcpy(D.m_szname, szname);

	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		int n = parse_line(szline, att);
		AbaqusModel::Distribution::ENTRY e;
		e.elem = atoi(att[0].szatt);
		e.val[0] = atof(att[1].szatt);
		e.val[1] = atof(att[2].szatt);
		e.val[2] = atof(att[3].szatt);
		e.val[3] = atof(att[4].szatt);
		e.val[4] = atof(att[5].szatt);
		e.val[5] = atof(att[6].szatt);
		D.m_data.push_back(e);
		read_line(szline, fp);
	}

	AbaqusModel::PART* pg = m_inp.CurrentPart();
	if (pg == 0) return false;

	pg->m_Distr.push_back(D);

	return true;
}
