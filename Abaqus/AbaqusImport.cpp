/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "AbaqusImport.h"
#include <stdlib.h>
#include <GeomLib/GMeshObject.h>
#include <vector>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/GDiscreteObject.h>
#include <GeomLib/GModel.h>
#include <FEBioLink/FEBioModule.h>
#include <FEBioLink/FEBioClass.h>
#include "ModelBuilder.h"
#include <sstream>

#ifdef LINUX // same for Linux and Mac OS X
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

// compare two strings, not considering case
inline bool szicmp(const char* sz1, const char* sz2)
{
	return (stricmp(sz1, sz2) == 0);
}

int parse_line(const char* szline, AbaqusImport::ATTRIBUTE* pa)
{
	int n = 0;
	int l = (int)strlen(szline);
	int m = 0;
	int k = 0;
	char* sz = pa[n].szatt;
	pa[0].szval[0] = 0;
	while ((m < l) && (n < AbaqusImport::MAX_ATTRIB))
	{
		char c = szline[m];
		switch (c)
		{
		case ',':	// go to the next attribute
			sz[k] = 0;
			// remove trailing whitespace
			while ((k > 0) && isspace(sz[k - 1])) { k--; sz[k] = 0; }
			k = 0;

			++n;
			sz = pa[n].szatt;
			pa[n].szval[0] = 0;
			break;
		case '"':
		{
			++m;
			while (m < l && ((c = szline[m]) != '"')) { sz[k++] = c; ++m; }
		}
		break;
		case '=':	// read the value
			sz[k] = 0;
			// remove trailing whitespace
			while ((k > 0) && isspace(sz[k - 1])) { k--; sz[k] = 0; }
			k = 0;
			sz = pa[n].szval;
			break;
		case '\r':
		case '\n':
			break;
		case ' ': // skip leading spaces
			if (k > 0) sz[k++] = c;
			break;
		default:	// copy text
			sz[k++] = c;
		}

		++m;
	}

	// remove trailing whitespace
	sz[k] = 0;
	while ((k > 0) && isspace(sz[k - 1])) { k--; sz[k] = 0; }
	++n;

	return n;
}

const char* find_attribute(AbaqusImport::ATTRIBUTE* pa, int nmax, const char* szatt)
{
	for (int i = 0; i < nmax; ++i)
	{
		if (szicmp(pa[i].szatt, szatt)) return pa[i].szval;
	}
	return nullptr;
}

AbaqusImport::AbaqusImport(FSProject& prj) : FSFileImport(prj)
{
	m_breadPhysics = false;
}

AbaqusImport::~AbaqusImport()
{
}

AbaqusModel::PART* AbaqusImport::GetActivePart()
{
	switch (m_scope)
	{
	case GLOBAL_SCOPE:
		// global part only if no assembly is defined yet
		if (m_inp.GetAssembly() == nullptr) return &m_inp.GlobalPart();
		else return nullptr;
		break;
	case ASSEMBLY_SCOPE:
		// global part
		return &m_inp.GlobalPart();
		break;
	case INSTANCE_SCOPE:
		// no active part
		return nullptr;
		break;
	case PART_SCOPE:
		assert(m_currentPart);
		return m_currentPart;
		break;
	}
	return nullptr;
}

bool AbaqusImport::read_line(char* szline, FILE* fp)
{
	// read a line but skip over comments (i.e.lines that start with **)
	do
	{
		if (fgets(szline, 255, fp) == nullptr) return false;
		++m_nline;
		if (feof(fp)) return false;
	}
	while ((szline[0] == '\n') || (szline[0] == '\r') || (strncmp(szline,"**", 2) == 0));

	// remove the eof line charachter
	char* ch = strrchr(szline, '\n');
	if (ch) *ch = 0;

	return true;
}

bool AbaqusImport::skip_keyword(char* szline, FILE* fp)
{
	do
	{
		if (read_line(szline, fp) == false) return false;
	}
	while (szline[0] != '*');
	return true;
}

// see if sz2 is contained in sz1, ignoring case
bool szicnt(const char* sz1, const char* sz2)
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

//! Load an Abaqus model file
bool AbaqusImport::Load(const char* szfile)
{
	m_prj.SetModule(FEBio::SetActiveModule("solid"));
	FSModel& fem = m_prj.GetFSModel();
	m_pprj = &m_prj;
	m_pfem = &fem;

	m_nline = 0;

	// set initial values
	m_scope = SCOPE::GLOBAL_SCOPE;
	m_currentPart = nullptr;
	m_currentStep = &m_inp.GetInitStep();

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
		Close();
		return false;
	}

	Close();

	// build the model
	if (build_model() == false) return false;

	return true;
}

//! Parse an abaqus model file
bool AbaqusImport::parse_file(FILE* fp)
{
	// get the first line
	char szline[256];
	if (!read_line(szline, fp)) return errf("Error while reading file");

	// build function table
	using functionPtr = bool (AbaqusImport::*)(char*, FILE*);
	map<string, functionPtr> Map = {
		{"*AMPLITUDE"          , &AbaqusImport::read_amplitude},
		{"*ASSEMBLY"           , &AbaqusImport::read_assembly},
		{"*BOUNDARY"           , &AbaqusImport::read_boundary},
		{"*CLOAD"              , &AbaqusImport::read_cload},
		{"*CONTACT PAIR"       , &AbaqusImport::read_contact_pair},
		{"*DISTRIBUTION"       , &AbaqusImport::read_distribution},
		{"*DSLOAD"             , &AbaqusImport::read_dsload},
		{"*ELEMENT"            , &AbaqusImport::read_elements},
		{"*ELSET"              , &AbaqusImport::read_element_sets},
		{"*END ASSEMBLY"       , &AbaqusImport::read_end_assembly},
		{"*END INSTANCE"       , &AbaqusImport::read_end_instance},
		{"*END PART"           , &AbaqusImport::read_end_part},
		{"*END STEP"           , &AbaqusImport::read_end_step},
		{"*HEADING"            , &AbaqusImport::read_heading},
		{"*INCLUDE"            , &AbaqusImport::read_include},
		{"*INSTANCE"           , &AbaqusImport::read_instance},
		{"*MATERIAL"           , &AbaqusImport::read_materials},
		{"*NGEN"               , &AbaqusImport::read_ngen   },
		{"*NFILL"              , &AbaqusImport::read_nfill  },
		{"*NSET"               , &AbaqusImport::read_node_sets},
		{"*NODE"               , &AbaqusImport::read_nodes  },
		{"*ORIENTATION"        , &AbaqusImport::read_orientation},
		{"*PART"               , &AbaqusImport::read_part},
		{"*SHELL SECTION"      , &AbaqusImport::read_shell_section},
		{"*SOLID SECTION"      , &AbaqusImport::read_solid_section},
		{"*SPRING"             , &AbaqusImport::read_spring},
		{"*STATIC"             , &AbaqusImport::read_static},
		{"*STEP"               , &AbaqusImport::read_step},
		{"*SURFACE"            , &AbaqusImport::read_surface},
		{"*SURFACE INTERACTION", &AbaqusImport::read_surface_interaction},
		{"*TIE"                , &AbaqusImport::read_tie},
	};

	// parse the keywords
	while (!feof(fp))
	{
		// get the next keyword
		string key;
		if (szline[0] == '*')
		{
			ATTRIBUTE att[MAX_ATTRIB];
			parse_line(szline, att);

			key = att[0].szatt;

			// convert to all upper-case
			std::transform(key.begin(), key.end(), key.begin(),
				[](unsigned char c) { return std::toupper(c); });
		}

		if (!key.empty())
		{
			// call the correct method handler
			auto it = Map.find(key);
			if (it != Map.end()) {
				functionPtr fnc = it->second;
				bool b = ((*this).*fnc)(szline, fp);
				if (b == false)
				{
					return errf("Error while reading keyword %s (line %d)", key.c_str(), m_nline);
				}
			}
			else {
				errf("Skipping unrecognized keyword \"%s\" (line %d)", key.c_str(), m_nline);
				read_line(szline, fp);
			}
		}
		else
		{
			// we can get here if a keyword was skipped.
			// we'll just ignore lines until we hit the next keyword
			read_line(szline, fp);
		}
	}

	return true;
}

bool AbaqusImport::read_heading(char* szline, FILE* fp)
{
	int n = 0;
	do
	{
		read_line(szline, fp);
		if (feof(fp)) return false;

		if (n == 0) m_title = szline;
		n++;
	}
	while (szline[0] != '*');

	return true;
}


//-----------------------------------------------------------------------------

bool AbaqusImport::read_nodes(char* szline, FILE* fp)
{
	// parse the szline for optional parameters
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);

	// get the active part
	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;
	AbaqusModel::PART& part = *pg;

	// read the nodes
	AbaqusModel::NODE n;
	n.x = n.y = n.z = 0;
	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		// parse the line
		int nr = sscanf(szline, "%d,%lg,%lg,%lg", &n.id, &n.x, &n.y, &n.z);
		if (nr == 4)
		{
			// add the node to the list
			part.AddNode(n);
		}

		// read the next line
		read_line(szline, fp);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_ngen(char* szline, FILE* fp)
{
	// get the active part
	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;

	AbaqusModel::PART& part = *pg;

	// scan the element line for optional parameters
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	// check the parameters
	int nline = 0;
	for (int i=1; i<natt; ++i)
	{
		if (szicnt(att[i].szatt, "LINE"))
		{
			const char* sz = att[i].szval;
			if      (szicnt(sz, "C")) nline = 1;
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
			for (int i=l1+linc; i<l2; i += linc)
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

			for (int i=l1+linc; i<l2; i += linc)
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
	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;

	AbaqusModel::PART& part = *pg;

	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		char* ch1 = strchr(szline, ',');
		if (ch1) *ch1 = 0; else return false;
		AbaqusModel::NODE_SET* ns1 = part.FindNodeSet(szline);
		if (ns1 == nullptr) return false;
		char* ch2 = strchr(ch1+1, ',');
		if (ch2) *ch2 = 0; else return false;
		AbaqusModel::NODE_SET* ns2 = part.FindNodeSet(ch1 + 1);
		if (ns2 == nullptr) return false;

		int nl, ni;
		sscanf(ch2+1, "%d,%d", &nl, &ni);

		if (ns1->node.size() != ns2->node.size()) return false;
/*
		int N = (int)ns1->node.size();
		AbaqusModel::NODE n;
		double t;
		for (int l=1; l<nl; ++l)
		{
			auto n1 = ns1->node.begin();
			auto n2 = ns2->node.begin();

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
*/
		read_line(szline, fp);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_elements(char* szline, FILE* fp)
{
	// scan the element line for optional parameters
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	// check the parameters
	int ntype = -1;
	bool bsprings = false;
	const char* szset = nullptr;
	for (int i=1; i<natt; ++i)
	{
		if (szicmp(att[i].szatt, "TYPE"))
		{
			const char* sz = att[i].szval;
			if      (szicmp(sz, "C3D8"  )) ntype = FE_HEX8;
            else if (szicmp(sz, "C3D8H" )) ntype = FE_HEX8;
			else if (szicmp(sz, "C3D8I" )) ntype = FE_HEX8;
			else if (szicmp(sz, "C3D8R" )) ntype = FE_HEX8;
			else if (szicmp(sz, "C3D8P" )) ntype = FE_HEX8;
            else if (szicmp(sz, "C3D5"  )) ntype = FE_PYRA5;
			else if (szicmp(sz, "C3D6"  )) ntype = FE_PENTA6;
			else if (szicmp(sz, "C3D4"  )) ntype = FE_TET4;
			else if (szicmp(sz, "C3D10" )) ntype = FE_TET10;
            else if (szicmp(sz, "C3D13" )) ntype = FE_PYRA13;  // invented Abaqus code (no Abaqus element of this type)
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
            else if (szicmp(sz, "S6"    )) ntype = FE_TRI6;
            else if (szicmp(sz, "S8R"   )) ntype = FE_QUAD8;
            else if (szicmp(sz, "S9R5"  )) ntype = FE_QUAD9;
            else if (szicmp(sz, "M3D3"  )) ntype = FE_TRI3;
            else if (szicmp(sz, "T3D2"  )) ntype = FE_BEAM2;
            else if (szicmp(sz, "CPS3"  )) ntype = FE_TRI3;
            else if (szicmp(sz, "CPS4"  )) ntype = FE_QUAD4;
			else if (szicmp(sz, "SPRINGA"))
			{
				ntype = -1;
				bsprings = true;
			}
			else {
				errf("Element type %s not supported (line %d)", sz, m_nline); 
				skip_keyword(szline, m_fp); 
				return true;
			}
		}
		else if (szicmp(att[i].szatt, "ELSET"))
		{
			szset = att[i].szval;
		}
	}

	// spring elements will be handled differently.
	if (bsprings)
	{
		return read_spring_elements(szline, fp);
	}

	// get the active part
	AbaqusModel::PART* pg = GetActivePart();
	if (pg == 0) return false;
	AbaqusModel::PART& part = *pg;

	// find the element set
	AbaqusModel::ELEMENT_SET* ps = nullptr;
	if (szset)
	{
		ps = part.FindElementSet(szset);
		if (ps == nullptr) ps = part.AddElementSet(szset);
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
    case FE_PYRA5 : N = 5; break;
	case FE_TET10 : N = 10; break;
    case FE_PYRA13: N = 13; break;
	case FE_HEX20 : N = 20; break;
	case FE_QUAD4 : N = 4; break;
	case FE_TRI3  : N = 3; break;
    case FE_TRI6  : N = 6; break;
    case FE_QUAD8 : N = 8; break;
    case FE_QUAD9 : N = 9; break;
    case FE_BEAM2 : N = 2; break;
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
				if (strlen(ch) == 1 || strcmp(ch,", ") == 0)
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

        // check for pyramid elements
        if (ntype == FE_HEX20)
        {
            if ((el.n[7] == el.n[4]) &&
                (el.n[6] == el.n[4]) &&
                (el.n[5] == el.n[4])) el.type = FE_PYRA13;
        }
        
		// add the element to the list
		part.AddElement(el);

		// add the element to the elementset
		if (ps != nullptr) ps->elem.push_back(el.id);

		// read the next line
		read_line(szline, fp);
	}
	
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_spring_elements(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	const char* szset = nullptr;
	if ((natt >= 3) && (szicmp(att[2].szatt, "ELSET")))
	{
		szset = att[2].szval;
	}

	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;

	AbaqusModel::PART& part = *pg;

	read_line(szline, fp);
	AbaqusModel::SpringSet set;

	if (szset) set.name = szset;

	int nc = 0;
	while (!feof(fp) && (szline[0] != '*'))
	{
		// parse the line
		char* ch = szline;

		int n = parse_line(szline, att);
		if (n == 3)
		{
			AbaqusModel::SPRING_ELEMENT el;
			el.id = atoi(att[0].szatt);
			el.n[0] = att[1].szatt;
			el.n[1] = att[2].szatt;
			set.m_Elem.push_back(el);
		}

		// read the next line
		read_line(szline, fp);
	}

	// add the spring element to the list
	part.AddSpringSet(set);

	return true;
}

//-----------------------------------------------------------------------------

bool AbaqusImport::read_element_sets(char* szline, FILE* fp)
{
	// read the attributes
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	// check the attributes
	bool bgen = false;
	bool internal = false;
	string instance;
	string name;
	for (int i=0; i<natt; ++i)
	{
		if      (szicmp(att[i].szatt, "GENERATE")) bgen = true;
		else if (szicmp(att[i].szatt, "ELSET"   )) name = att[i].szval;
		else if (szicmp(att[i].szatt, "INSTANCE")) instance = att[i].szval;
		else if (szicmp(att[i].szatt, "INTERNAL")) internal = true;
	}

	// get the current part
	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;

	// find the element set or create a new one
	AbaqusModel::ELEMENT_SET* pset = pg->FindElementSet(name);
	if (pset)
	{
		// see if we can merge or if we need to create a new set (with the same name)
		if (pset->instance != instance)
		{
			pset = pg->AddElementSet(name);
			pset->name = name;
			pset->instance = instance;
			pset->binternal = internal;
		}
	}
	else
	{
		pset = pg->AddElementSet(name);
		pset->name = name;
		pset->instance = instance;
		pset->binternal = internal;
	}

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
			for (int i = n1; i <= n2; i += n) pset->elem.push_back(i);

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
				pset->elem.push_back(n[i]);
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
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	string name;
	string instance;
	bool bgen = false;
	bool internal = false;
	
	// check the attributes
	for (int i=0; i<natt; ++i)
	{
		if      (szicmp(att[i].szatt, "GENERATE")) bgen = true;
		else if (szicmp(att[i].szatt, "NSET"    )) name = att[i].szval;
		else if (szicmp(att[i].szatt, "INSTANCE")) instance = att[i].szval;
		else if (szicmp(att[i].szatt, "INTERNAL")) internal = true;
	}

	// find the part
	AbaqusModel::PART* pg = GetActivePart();
	if (pg == 0) return false;
	AbaqusModel::PART& part = *pg;

	// create the nodeset
	AbaqusModel::NODE_SET* pset = part.FindNodeSet(name);
	if (pset) return errf("duplicate nodeset defined.");
	pset = part.AddNodeSet(name);
	pset->instance = instance;
	pset->binternal = internal;

	// read the lines
	if (bgen)
	{
		int n1, n2, n;
		read_line(szline, fp);
		while (!feof(fp) && (szline[0] != '*'))
		{
			// parse the line
			int nread = sscanf(szline, "%d,%d,%d", &n1, &n2, &n);
			if (nread == 2) n = 1;
	
			for (int i = n1; i <= n2; i += n) pset->node.push_back(i);

			// read the next line
			read_line(szline, fp);
		}
	}
	else
	{
		int n[16];
		read_line(szline, fp);
		while (!feof(fp) && (szline[0] != '*'))
		{
			// read the nodes
			int nr = sscanf(szline, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&n[0],&n[1],&n[2],&n[3],&n[4],&n[5],&n[6],&n[7],&n[8],&n[9],&n[10],&n[11],&n[12],&n[13],&n[14],&n[15]);

			// add the elements to the list
			for (int i=0; i<nr; ++i)
			{
				pset->node.push_back(n[i]);
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
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	int ntype = AbaqusModel::ST_ELEMENT;
	string name;
	string instance;

	// check the attributes
	for (int i=0; i<natt; ++i)
	{
		if      (szicmp(att[i].szatt, "NAME"    )) name = att[i].szval;
		else if (szicmp(att[i].szatt, "INSTANCE")) instance = att[i].szval;
		else if (szicmp(att[i].szatt, "TYPE"    ))
		{
			if      (szicmp(att[i].szval, "ELEMENT")) ntype = AbaqusModel::ST_ELEMENT;
			else if (szicmp(att[i].szval, "NODE"   )) ntype = AbaqusModel::ST_NODE;
			else ntype = -1;
		}
	}

	// find the surface
	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;

	AbaqusModel::SURFACE* ps = pg->FindSurface(name);
	if (ps) return errf("Duplicate surface found.");
	ps = pg->AddSurface(name);
	ps->instance = instance;
	ps->type = ntype;
	
	// read the surface definition
	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		// find the comma
		parse_line(szline, att);

		// strip "'s
		char* szset = att[0].szatt;
		if (szset[0] == '"')
		{
			char* ch2 = strchr(szset + 1, '"');
			if (ch2 == 0) return false;
			int l = (int)(ch2 - szset);
			for (int i = 0; i < l - 1; ++i) szset[i] = szset[i + 1];
			szset[l - 1] = 0;
		}

		char* ch = att[1].szatt;
		if (ntype == AbaqusModel::ST_ELEMENT)
		{
			// get the face id
			int nf = 0;
			if (ch)
			{
				if      (szicmp(ch, "SPOS")) nf = 1;
				else if (szicmp(ch, "SNEG")) nf = -1;
				else if (ch[0] == 'S')
				{
					if (ch) nf = atoi(ch + 1); else return false;
					if ((nf < 1) || (nf > 6)) return errf("Invalid surface definiton");
				}
				else return errf("Invalid surface definition");;
			}
			else errf("Invalid surface definition");
			ps->set.push_back({ szset, nf });
		}
		else if (ntype == AbaqusModel::ST_NODE)
		{
			ps->set.push_back({ szset, -1 });
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

	ATTRIBUTE a[MAX_ATTRIB];
	int natt = parse_line(szline, a);
	const char* szname = find_attribute(a, natt, "NAME");
	if (szname) mat.name = szname;

	read_line(szline, fp);
	while (!feof(fp))
	{
		if (szicnt(szline, "*DENSITY"))
		{
			read_line(szline, fp);
			sscanf(szline, "%lg", &mat.dens);
		}
		else if (szicnt(szline, "*ELASTIC"))
		{
			mat.mattype = AbaqusModel::ELASTIC;
			natt = parse_line(szline, a);
			const char* sztype = find_attribute(a, natt, "TYPE");
			if (sztype && szicmp(sztype, "ISOTROPIC")) mat.ntype = 1;
			else if (sztype == nullptr) mat.ntype = 1;

			read_line(szline, fp);
			char* sz = szline;
			int nmax = 2;
			int np = 0;
			char* ch = 0;
			do
			{
				ch = strchr(sz, ',');
				sscanf(sz, "%lg", &mat.d[np++]);
				if (ch) sz = ch + 1;
			}
			while (ch && (np < nmax));
		}
		else if (szicnt(szline, "*HYPERELASTIC"))
		{
			mat.mattype = AbaqusModel::HYPERELASTIC;
			mat.ntype = -1;
			natt = parse_line(szline, a);
			const char* sztype = a[1].szatt;
			int lines = 1;
			int nmax = 2;
			mat.nparam = 0;
			if (sztype && szicmp(sztype, "NEOHOOKE"))
			{
				mat.ntype = 1;
				mat.nparam = 2;
			}
			if (sztype && szicmp(sztype, "OGDEN"))
			{
				if (strcmp(a[2].szatt, "N") == 0)
				{
					int N = atoi(a[2].szval);
					mat.ntype = 2;
					lines = (N > 2 ? 2 : 1);
					nmax = N*3;
					mat.nparam = nmax;
				}
			}

			int np = 0;
			for (int l = 0; l < lines; ++l)
			{
				read_line(szline, fp);
				char* sz = szline;
				char* ch = strchr(sz, ',');
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
		}
		else if (szicnt(szline, "*ANISOTROPIC HYPERELASTIC"))
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
		else if (szicnt(szline, "*TRANSVERSE SHEAR"))
		{
			// TODO: do something with this
			read_line(szline, fp);
		}
		else break;
		read_line(szline, fp);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_part(char* szline, FILE* fp)
{
	if (m_currentPart) return errf("Error in file: new part was started before END PART was detected. (line %d)", m_nline);
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	if (natt == 0) return errf("Error: You need to add a NAME attribute when defining a part. (line %d)", m_nline);

	if (szicmp(att[1].szatt, "NAME"))
	{
		m_currentPart = m_inp.CreatePart(att[1].szval);
	}
	else return errf("ERROR: invalid attribute for PART keyword. (line %d)", m_nline);

	m_scope = SCOPE::PART_SCOPE;

	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_end_part(char* szline, FILE* fp)
{
	// make sure we are in a part defintion
	if (m_currentPart == 0) return errf("ERROR in file: END PART detected but no part was defined. (line %d)", m_nline);

	// close part entry
	m_currentPart = nullptr;
	m_scope = SCOPE::GLOBAL_SCOPE;
	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_instance(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	if (m_scope != SCOPE::ASSEMBLY_SCOPE) return errf("INSTANCE must be inside ASSEMBLY");

	// make sure we have an assembly
	AbaqusModel::ASSEMBLY* asmbly = m_inp.GetAssembly();
	if (asmbly == nullptr) return errf("Instance defined outside Assembly.");

	// create a new instance
	AbaqusModel::INSTANCE* pInst = asmbly->AddInstance();

	// set the name
	pInst->SetName(att[1].szval);

	// find the part symbol
	if (szicmp("part", att[2].szatt))
	{
		pInst->SetPartName(att[2].szval);
	}
	else return errf("Instance needs a part attribute.");

	m_scope = SCOPE::INSTANCE_SCOPE;

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
	if (m_scope != SCOPE::INSTANCE_SCOPE) return errf("Unexpected END INSTANCE keyword encountered.");
	m_scope = SCOPE::ASSEMBLY_SCOPE;
	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_assembly(char* szline, FILE* fp)
{
	if (m_scope != SCOPE::GLOBAL_SCOPE) return errf("Unexpected ASSEMBLY keyword encountered.");

	// make sure we don't have an assembly yet
	AbaqusModel::ASSEMBLY* asmbly = m_inp.GetAssembly();
	if (asmbly) return errf("Only one assembly can be defined in a model.");

	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	const char* szname = find_attribute(att, 2, "name");
	if (szname == nullptr) return errf("name attribute of ASSEMBLY keyword missing");

	// create an assembly
	asmbly = m_inp.CreateAssembly();
	asmbly->m_name = szname;

	m_scope = SCOPE::ASSEMBLY_SCOPE;

	read_line(szline, fp);

	return true;
}

bool AbaqusImport::read_end_assembly(char* szline, FILE* fp)
{
	if (m_scope != SCOPE::ASSEMBLY_SCOPE) return errf("Unexpected END ASSEMBLY keyword encountered.");
	m_scope = SCOPE::GLOBAL_SCOPE;
	read_line(szline, fp);
	return true;
}

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

bool AbaqusImport::build_model()
{
	// give the model a name
	char filename[256] = { 0 };
	FileTitle(filename);
	m_inp.SetName(filename);

	// build the geometry first
	ModelBuilder builder;
	if (builder.BuildGeometry(m_inp, m_pfem->GetModel()) == false)
	{
		return errf("Failed building geometry.");
	}

	if (m_breadPhysics)
	{
		// build all the physics components
		if (builder.BuildPhysics(m_inp, *m_pfem) == false)
		{
			return errf("Failed building physics.");
		}

		// The abaqus reader currently still uses the old FE classes, so we need to convert. 
		std::ostringstream log;
		m_prj.ConvertToNewFormat(log);
		std::string s = log.str();
		if (s.empty() == false)
		{
			errf(s.c_str());
		}
	}

	// all done
	return true;
}

bool AbaqusImport::read_step(char* szline, FILE* fp)
{
	if (m_scope != SCOPE::GLOBAL_SCOPE) return errf("Unexpected STEP keyword encountered.");

	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);

	string name;
	const char* szname = find_attribute(att, 5, "name");
	if (szname) name = szname;
	AbaqusModel::STEP* step = m_inp.AddStep(name);
	step->dt0 = 1.0;
	step->time = 1;

	m_currentStep = step;

	read_line(szline, fp);

	return true;
}

bool AbaqusImport::read_end_step(char* szline, FILE* fp)
{
	if ((m_currentStep == nullptr) || (m_scope != SCOPE::GLOBAL_SCOPE)) return errf("Unexpected END STEP keyword encountered.");
	m_currentStep = nullptr;
	read_line(szline, fp);
	return true;
}

bool AbaqusImport::read_boundary(char* szline, FILE* fp)
{
	if ((m_currentStep == nullptr) || (m_scope != SCOPE::GLOBAL_SCOPE)) return errf("Unexpected BOUNDARY keyword encountered.");

	AbaqusModel::BOUNDARY BC;
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	const char* szampl = find_attribute(att, 4, "amplitude");
	if (szampl) BC.m_ampl = szampl;

	read_line(szline, fp);

	while (!feof(fp) && (szline[0] != '*'))
	{
		int n = parse_line(szline, att);
		if (n > 1)
		{
			AbaqusModel::BOUNDARY::NSET nset;
			nset.nset = att[0].szatt;
			nset.ndof[0] = nset.ndof[1] = atoi(att[1].szatt);
			if (n > 2) nset.ndof[1] = atoi(att[2].szatt);
			if (n > 3) nset.val = atof(att[3].szatt);

			BC.add(nset);
		}
		read_line(szline, fp);
	}

	if (m_currentStep) m_currentStep->AddBoundaryCondition(BC);

	return true;	
}

bool AbaqusImport::read_dsload(char* szline, FILE* fp)
{
	if ((m_currentStep == nullptr) || (m_scope != SCOPE::GLOBAL_SCOPE)) return errf("Unexpected DSLOAD keyword encountered.");

	AbaqusModel::DSLOAD P;
	ATTRIBUTE att[MAX_ATTRIB];

	int natt = parse_line(szline, att);
	const char* sza = find_attribute(att, 4, "amplitude");
	if (sza)
	{
		P.m_ampl = m_inp.FindAmplitude(sza);
	}

	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		int n = parse_line(szline, att);
		if (n == 3)
		{
			const char* szsurf = att[0].szatt;
			double val = atof(att[2].szatt);
			P.add(szsurf, val);
		}
		read_line(szline, fp);
	}

	if (m_currentStep) m_currentStep->AddPressureLoad(P);

	return true;
}

bool AbaqusImport::read_cload(char* szline, FILE* fp)
{
	AbaqusModel::CLOAD load;
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	for (int i = 0; i < natt; ++i)
	{
		ATTRIBUTE& a = att[i];
		if (szicmp(a.szatt, "amplitude")) load.ampl = att[i].szval;
	}

	read_line(szline, fp);
	while (!feof(fp) && (szline[0] != '*'))
	{
		AbaqusModel::CLOAD::NSET ns;

		int n = parse_line(szline, att);
		if (n == 3)
		{
			ns.nset = att[0].szatt;
			ns.ndof = atoi(att[1].szatt);
			ns.val  = atof(att[2].szatt);
		}
		load.nset.push_back(ns);

		read_line(szline, fp);
	}

	if (m_currentStep) m_currentStep->AddCLoad(load);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_solid_section(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	int n = parse_line(szline, att);

	AbaqusModel::PART* pg = GetActivePart();
	if (pg == 0) return false;

	string elset;
	string mat;
	string orient;
	for (int i = 0; i < n; ++i)
	{
		ATTRIBUTE& a = att[i];
		if      (szicmp(a.szatt, "elset"      )) elset = a.szval;
		else if (szicmp(a.szatt, "material"   )) mat = a.szval;
		else if (szicmp(a.szatt, "orientation")) orient = a.szval;
	}

	if (elset.empty()) return false;

	pg->AddSolidSection(elset, mat, orient);

	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_shell_section(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	int n = parse_line(szline, att);

	AbaqusModel::PART* pg = GetActivePart();
	if (pg == 0) return false;

	string elset;
	string mat;
	string orient;
	for (int i = 0; i < n; ++i)
	{
		ATTRIBUTE& a = att[i];
		if      (szicmp(a.szatt, "elset"      )) elset  = a.szval;
		else if (szicmp(a.szatt, "material"   )) mat    = a.szval;
		else if (szicmp(a.szatt, "orientation")) orient = a.szval;
	}

	if (elset.empty()) return false;

	AbaqusModel::SHELL_SECTION& ss = pg->AddShellSection(elset, mat, orient);

	read_line(szline, fp);
	n = parse_line(szline, att);
	ss.m_shellThickness = atof(att[0].szatt);

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
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);

	if (m_currentStep == nullptr) return false;

	m_currentStep->dt0 = atof(att[0].szatt);
	m_currentStep->time = atof(att[1].szatt);

	read_line(szline, fp);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_orientation(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);
	const char* szname = find_attribute(att, 3, "name");
	read_line(szline, fp);

	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;

	pg->AddOrientation(szname, szline);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_distribution(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);
	const char* szname = find_attribute(att, 5, "name");
	if (szname == 0) return false;

	AbaqusModel::Distribution D;
	D.name = szname;

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

	AbaqusModel::PART* pg = GetActivePart();
	if (pg == 0) return false;

	pg->m_Distr.push_back(D);

	return true;
}

bool AbaqusImport::read_amplitude(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);

	const char* szname = find_attribute(att, 5, "name");
	if (szname == 0) return false;

	AbaqusModel::Amplitude amp; 
	amp.m_name = szname;

	const char* szdef = find_attribute(att, 10, "definition");
	if (szdef)
	{
		if (szicmp(szdef, "TABULAR"   )) amp.m_type = AbaqusModel::Amplitude::AMP_TABULAR;
		if (szicmp(szdef, "SMOOTHSTEP")) amp.m_type = AbaqusModel::Amplitude::AMP_SMOOTH_STEP;
	}
	else amp.m_type = AbaqusModel::Amplitude::AMP_TABULAR;

	if (amp.m_type == AbaqusModel::Amplitude::AMP_SMOOTH_STEP)
	{
		if (read_line(szline, fp) == false) return false;
		int count = parse_line(szline, att);
		for (int n = 0; n < count; n += 2)
		{
			double x = atof(att[n  ].szatt);
			double y = atof(att[n+1].szatt);
			amp.m_points.push_back(vec2d(x, y));
		}
	}
	else
	{
		do
		{
			if (read_line(szline, fp) == false) break;
			if (szline[0] == '*') break;

			int count = parse_line(szline, att);
			if (count >= 2)
			{
				double x = atof(att[0].szatt);
				double y = atof(att[1].szatt);
				amp.m_points.push_back(vec2d(x, y));
			}
		} while (true);
	}

	m_inp.AddAmplitude(amp);

	return true;
}

bool AbaqusImport::read_contact_pair(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);

	AbaqusModel::CONTACT_PAIR cp;
	cp.name = att[1].szval;

	read_line(szline, fp);
	int n = parse_line(szline, att);
	if (n == 1)
	{
		cp.surf1 = cp.surf2 = att[0].szatt;
	}
	else if (n == 2)
	{
		cp.surf1 = att[0].szatt;
		cp.surf2 = att[1].szatt;
	}

	// see if there are any surface properties
	read_line(szline, fp);
	if (szicnt(szline, "*SURFACE INTERACTION"))
	{
		read_line(szline, fp);
		if (szicnt(szline, "*FRICTION"))
		{
			read_line(szline, fp);
			cp.friction = atof(szline);
		}
	}

	if (m_currentStep) m_currentStep->AddContactPair(cp);

	return true;
}

bool AbaqusImport::read_tie(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);

	AbaqusModel::TIE tie;
	tie.name = att[1].szval;

	read_line(szline, fp);
	int n = parse_line(szline, att);
	if (n == 1)
	{
		tie.surf1 = tie.surf2 = att[0].szatt;
	}
	else if (n == 2)
	{
		tie.surf1 = att[0].szatt;
		tie.surf2 = att[1].szatt;
	}

	read_line(szline, fp);

	if (m_currentStep) m_currentStep->AddTie(tie);

	return true;
}

bool AbaqusImport::read_spring(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	bool nonlinear = false;

	AbaqusModel::SPRING spring;

	for (int i = 0; i < natt; ++i)
	{
		ATTRIBUTE& a = att[i];
		if (szicnt(a.szatt, "ELSET"    )) spring.elset = a.szval;
		if (szicnt(a.szatt, "nonlinear")) spring.nonlinear = true;
	}

	// skip a line
	read_line(szline, fp);
	read_line(szline, fp);
	LoadCurve& lc = spring.m_lc;
	lc.Clear();
	int line = 0;
	while (!feof(fp) && (szline[0] != '*'))
	{
		double x, y;
		sscanf(szline, "%lg,%lg", &y, &x);
		if ((line == 0) && !spring.nonlinear) spring.k = y;
		else lc.Add(x, y);
		read_line(szline, fp);
		line++;
	}

	AbaqusModel::PART* pg = GetActivePart();
	if (pg == nullptr) return false;

	pg->AddSpring(spring);

	return true;
}

bool AbaqusImport::read_include(char* szline, FILE* fp)
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

	return true;
}
