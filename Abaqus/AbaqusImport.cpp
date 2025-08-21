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
#include <sstream>

AbaqusImport::AbaqusImport(FSProject& prj) : FSFileImport(prj)
{
	// default options
	m_bnodesets = true;
	m_belemsets = true;
	m_bfacesets = true;
	m_bautopart = false;
	m_bautosurf = true;
	m_bssection = true;

	m_breadPhysics = false;

	AddBoolParam(true, "import_nodesets", "Import nodesets");
	AddBoolParam(true, "import_elemsets", "Import element sets");
	AddBoolParam(true, "import_surfaces", "Import surfaces");
	AddBoolParam(false, "auto_partition", "Auto-partition from element sets");
	AddBoolParam(true, "auto_partition_surface", "Auto-partition surface");
	AddBoolParam(true, "process_solid_sections", "Process solid sections");
}

AbaqusImport::~AbaqusImport()
{
}

bool AbaqusImport::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_bnodesets = GetBoolValue(0);
		m_belemsets = GetBoolValue(1);
		m_bfacesets = GetBoolValue(2);
		m_bautopart = GetBoolValue(3);
		m_bautosurf = GetBoolValue(4);
		m_bssection = GetBoolValue(5);
	}
	else
	{
		SetBoolValue(0, m_bnodesets);
		SetBoolValue(1, m_belemsets);
		SetBoolValue(2, m_bfacesets);
		SetBoolValue(3, m_bautopart);
		SetBoolValue(4, m_bautosurf);
		SetBoolValue(5, m_bssection);
	}

	return false;
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

// compare two strings, not considering case
bool szicmp(const char* sz1, const char* sz2)
{
	int l1 = (int)strlen(sz1);
	int l2 = (int)strlen(sz2);
	if (l1 != l2) return false;
	int n1 = 0, n2 = 0;

	char c1, c2;

	do
	{
		c1 = sz1[n1++];
		c2 = sz2[n2++];

		if ((c1 >= 'A') && (c1 <= 'Z')) c1 = 'a' + (c1 - 'A');
		if ((c2 >= 'A') && (c2 <= 'Z')) c2 = 'a' + (c2 - 'A');
		if (c1 != c2) return false;
	} while ((n1 < l1) && (n2 < l2));

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

	// parse the keywords
	while (!feof(fp))
	{
		// find what keyword this is
		if (szicnt(szline, "*HEADING"))	// read the heading
		{
			if (!read_heading(szline, fp)) return errf("Error while reading keyword HEADING (line %d)", m_nline);
		}
		else if (szicnt(szline, "*PREPRINT"))
		{
			// just skip this
			read_line(szline, fp);
		}
		else if (szicnt(szline, "*NODE PRINT"))
		{
			// we need to read this otherwise, the NODE reader gets messed up
			read_line(szline, fp);
		}
		else if (szicnt(szline, "*NODE OUTPUT"))
		{
			// we need to read this otherwise, the NODE reader gets messed up
			read_line(szline, fp);
		}
		else if (szicnt(szline, "*NODE")) // read nodes
		{
			if (!read_nodes(szline, fp)) return errf("Error while reading keyword NODE (line %d)", m_nline);
		}
		else if (szicnt(szline, "*NGEN")) // read node generation
		{
			if (!read_ngen(szline, fp)) return errf("Error while reading keyword NGEN (line %d)", m_nline);
		}
		else if (szicnt(szline, "*NFILL")) // read nfill
		{
			if (!read_nfill(szline, fp)) return errf("Error while reading keyword NFILL (line %d)", m_nline);
		}
		else if (szicnt(szline, "*ELEMENT OUTPUT"))
		{
			// we need to read this otherwise, the ELEMENT reader gets messed up
			read_line(szline, fp);
		}
		else if (szicnt(szline, "*SOLID SECTION"))
		{
			if (!read_solid_section(szline, fp)) return errf("Error while reading keyword SOLID SECTION (line %d)", m_nline);
		}
		else if (szicnt(szline, "*SHELL SECTION"))
		{
			if (!read_shell_section(szline, fp)) return errf("Error while reading keyword SHELL SECTION (line %d)", m_nline);
		}
		else if (szicnt(szline, "*ELEMENTSET")) // read element sets
		{
			if (!read_element_sets(szline, fp)) return errf("Error while reading keyword ELEMENTSET (line %d)", m_nline);			
		}
		else if (szicnt(szline, "*ELEMENT")) // read elements
		{
			if (!read_elements(szline, fp)) return errf("Error while reading keyword ELEMENT (line %d)", m_nline);
		}
		else if (szicnt(szline, "*ELSET")) // read element sets
		{
			if (!read_element_sets(szline, fp)) return errf("Error while reading keyword ELSET (line %d)", m_nline);			
		}
		else if (szicnt(szline, "*NSET")) // read element sets
		{
			if (!read_node_sets(szline, fp)) return errf("Error while reading keyword NSET (line %d)", m_nline);			
		}
		else if (szicnt(szline, "*SURFACE BEHAVIOR"))
		{
			// read the next line
			read_line(szline, fp);
		}
		else if (szicnt(szline, "*SURFACE INTERACTION"))
		{
			if (!read_surface_interaction(szline, fp)) return errf("Error while reading keyword SURFACE INTERACTION (line %d)", m_nline);
		}
		else if (szicnt(szline, "*SURFACE")) // read surfaces
		{
			if (m_bfacesets)
			{
				if (!read_surface(szline, fp)) return errf("Error while reading keyword SURFACE (line %d)", m_nline);
			}
			else read_line(szline, fp);
		}
		else if (szicnt(szline, "*MATERIAL")) // read materials
		{
			if (!read_materials(szline, fp)) return errf("Error while reading keyword MATERIAL (line %d)", m_nline);
		}
		else if (szicnt(szline, "*PART")) // read parts
		{
			if (!read_part(szline, fp)) return errf("Error while reading keyword PART (line %d)", m_nline);
		}
		else if (szicnt(szline,"*END PART") || szicnt(szline, "*ENDPART"))
		{
			if (!read_end_part(szline, fp)) return errf("Error while reading keyword END PART (line %d)", m_nline);
		}
		else if (szicnt(szline, "*INSTANCE"))
		{
			if (!read_instance(szline, fp)) return errf("Error while reading keyword INSTANCE (line %d)", m_nline);
		}
		else if (szicnt(szline, "*END INSTANCE"))
		{
			if (!read_end_instance(szline, fp)) return errf("Error while reading keyword END INSTANCE (line %d)", m_nline);
		}
		else if (szicnt(szline, "*ASSEMBLY"))
		{
			if (!read_assembly(szline, fp)) return errf("Error while reading keyword INSTANCE (line %d)", m_nline);
		}
		else if (szicnt(szline, "*END ASSEMBLY"))
		{
			if (!read_end_assembly(szline, fp)) return errf("Error while reading keyword END INSTANCE (line %d)", m_nline);
		}
		else if (szicnt(szline, "*STEP"))
		{
			if (!read_step(szline, fp)) 
			{
				return errf("Error while reading keyword STEP (line %d)", m_nline);
			}
		}
		else if (szicnt(szline, "*BOUNDARY"))
		{
			if (!read_boundary(szline, fp)) return errf("Error while reading keyword BOUNDARY (line %d)", m_nline);
		}
		else if (szicnt(szline, "*DSLOAD"))
		{
			if (!read_dsload(szline, fp)) return errf("Error while reading keyword DSLOAD (line %d)", m_nline);
		}
		else if (szicnt(szline, "*ORIENTATION"))
		{
			if (!read_orientation(szline, fp)) return errf("Error while reading keyword ORIENTATION (line %d)", m_nline);
		}
		else if (szicnt(szline, "*DISTRIBUTION TABLE"))
		{
			read_line(szline, fp);
		}
		else if (szicnt(szline, "*DISTRIBUTION"))
		{
			if (!read_distribution(szline, fp)) return errf("Error while reading keyword DISTRIBUTION (line %d)", m_nline);
		}
		else if (szicnt(szline, "*AMPLITUDE"))
		{
			if (!read_amplitude(szline, fp)) return errf("Error while reading keyword AMPLITUDE (line %d)", m_nline);
		}
		else if (szicnt(szline, "*END STEP"))
		{
			read_line(szline, fp);
		}
		else if (szicnt(szline, "*CONTACT PAIR")) // read contact pairs
		{
			if (!read_contact_pair(szline, fp)) return errf("Error while reading keyword CONTACT PAIR (line %d)", m_nline);
		}
		else if (szicnt(szline, "*SPRING"))
		{
			if (!read_spring(szline, fp)) return errf("Error while reading keyword SPRING (line %d)", m_nline);
		}
		else if (szicnt(szline, "*INCLUDE")) // include another file
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
			if (szline[0] == '*')
			{
				char* ch = strchr(szline, ',');
				if (ch) *ch = 0;
				errf("Skipping unrecognized keyword \"%s\" (line %d)", szline, m_nline);
			}
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
	while ((m<l) && (n < MAX_ATTRIB))
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
		case '\r':
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
		read_line(szline, fp);
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
	ATTRIBUTE att[MAX_ATTRIB];
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
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	// check the parameters
	int nline = 0;
	for (i=1; i<natt; ++i)
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
		AbaqusModel::NODE_SET* ns1 = part.FindNodeSet(szline);
		if (ns1 == nullptr) return false;
		char* ch2 = strchr(ch1+1, ',');
		if (ch2) *ch2 = 0; else return false;
		AbaqusModel::NODE_SET* ns2 = part.FindNodeSet(ch1 + 1);
		if (ns2 == nullptr) return false;

		int nl, ni;
		sscanf(ch2+1, "%d,%d", &nl, &ni);

		if (ns1->node.size() != ns2->node.size()) return false;

		int N = (int)ns1->node.size();
		AbaqusModel::NODE n;
		double t;
		for (int l=1; l<nl; ++l)
		{
			list<AbaqusModel::Tnode_itr>::iterator n1 = ns1->node.begin();
			list<AbaqusModel::Tnode_itr>::iterator n2 = ns2->node.begin();

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
	AbaqusModel::PART* pg = m_inp.GetActivePart();
	if (pg == 0) { skip_keyword(szline, m_fp); return true; }
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

	// get the active part
	AbaqusModel::PART* pg = m_inp.GetActivePart();
	if (pg)
	{
		AbaqusModel::PART& part = *m_inp.GetActivePart();
		read_line(szline, fp);
		AbaqusModel::SPRING_ELEMENT el;

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
		AbaqusModel::SPRING_ELEMENT el;

		int nc = 0;
		while (!feof(fp) && (szline[0] != '*'))
		{
			ATTRIBUTE att[MAX_ATTRIB];
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
			if (szset)
			{
				AbaqusModel::SpringSet& set = pg->m_SpringSet[szset];
				set.m_Elem.push_back(el);
			}
			else pg->AddSpring(el);

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
	ATTRIBUTE att[MAX_ATTRIB];
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
	if (pg == 0)
	{
		errf("Error reading ELSET (line %d)", m_nline);
		skip_keyword(szline, m_fp);
		return true;
	}
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
	AbaqusModel::ELEMENT_SET* pset = part.FindElementSet(szname);
	if (pset == nullptr) pset = part.AddElementSet(szname);

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
				if (it != part.m_Elem.end() && (it->id != -1)) pset->elem.push_back(it->id);
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
		AbaqusModel::NODE_SET* pset = part.FindNodeSet(szname);
		if (pset == nullptr) 
		{
			pset = part.AddNodeSet(szname);
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
				pset->node.push_back(it);
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
		AbaqusModel::NODE_SET* pset = part.AddNodeSet(szname);

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
				pset->node.push_back(it);
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

	if (ntype != AbaqusModel::ST_ELEMENT)
	{
		errf("Failed reading SURFACE keyword (line %d)", m_nline);
		skip_keyword(szline, fp);
		return true;
	}

	// find the surface
	AbaqusModel::SURFACE* ps = 0;
	if (pg)
	{
		ps = pg->FindSurface(szname);
		if (ps == nullptr) ps = pg->AddSurface(szname);
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
		AbaqusModel::ELEMENT_SET* pset = (pg ? pg->FindElementSet(szline) : m_inp.FindElementSet(szline));

		if (pset)
		{
			if (ps == 0)
			{
				pg = pset->part; if (pg == 0) return false;
				AbaqusModel::SURFACE* psi = pg->AddSurface(szname);
				if (psi == nullptr) return false;

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
		else if (ps)
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

	ATTRIBUTE a[MAX_ATTRIB];
	int natt = parse_line(szline, a);
	const char* szname = find_attribute(a, natt, "NAME");
	if (szname) strcpy(mat.szname, szname);

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
	if (m_inp.CurrentPart()) return errf("Error in file: new part was started before END PART was detected. (line %d)", m_nline);
	ATTRIBUTE att[MAX_ATTRIB];
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
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);

	// make sure we have an assembly
	AbaqusModel::ASSEMBLY* asmbly = m_inp.GetCurrentAssembly();
	if (asmbly == nullptr) return errf("Instance defined outside Assembly.");

	if (m_inp.CurrentPart()) return errf("Instance encountered while reading part.");
	if (asmbly->CurrentInstance()) return errf("Instance encountered when other instance not finished.");

	// create a new instance
	AbaqusModel::INSTANCE* pInst = asmbly->AddInstance();

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
	AbaqusModel::ASSEMBLY* asmbly = m_inp.GetCurrentAssembly();
	if (asmbly == nullptr) return errf("end instance encountered without active assembly.");

	if (asmbly->CurrentInstance() == nullptr) return errf("end instance encountered with no active instance.");
	asmbly->ClearCurrentInstance();
	m_inp.SetCurrentPart(nullptr);
	read_line(szline, fp);
	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_assembly(char* szline, FILE* fp)
{
	// make sure we don't have an assembly yet
	AbaqusModel::ASSEMBLY* asmbly = m_inp.GetAssembly();
	if (asmbly) return errf("Only one assembly can be defined in a model.");

	// also make sure we don't already read the assembly keyword
	asmbly = m_inp.GetCurrentAssembly();
	if (asmbly) return errf("Duplicate assembly keyword.");

	// create an assembly
	asmbly = m_inp.CreateAssembly();
	m_inp.SetCurrentAssembly(asmbly);

	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	const char* szname = find_attribute(att, 2, "name");
	if (szname == nullptr) return errf("name attribute of ASSEMBLY keyword missing");

	asmbly->m_name = szname;

	read_line(szline, fp);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_end_assembly(char* szline, FILE* fp)
{
	if (m_inp.GetCurrentAssembly() == nullptr) return errf("no assembly was active when END ASSEMBLY was found.");
	m_inp.SetCurrentAssembly(nullptr);
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
	FSModel& fem = *m_pfem;

	const char* szdefaultName = "Object";

	// if instances are defined, we create the geometry based on the instances
	AbaqusModel::ASSEMBLY* asmbly = m_inp.GetAssembly();
	if (asmbly)
	{
		list<AbaqusModel::INSTANCE*>& Inst = asmbly->InstanceList();

		// loop over all instances
		list<AbaqusModel::INSTANCE*>::iterator pi;
		for (pi=Inst.begin(); pi!=Inst.end(); ++pi)
		{
			AbaqusModel::INSTANCE* pinst = *pi;

			// build a part
			GObject* po = build_part(pinst->GetPart());
			if (po)
			{
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
				const char* szname = (*pi)->GetName();
				if ((szname == 0) || (strlen(szname) == 0)) szname = szdefaultName;
				po->SetName(szname);
				fem.GetModel().AddObject(po);
			}
		}
	}
	else
	{
		// loop over all parts
		list<AbaqusModel::PART*>& Part = m_inp.PartList();
		if (Part.empty())
		{
			return errf("This file contains no parts.");
		}

		list<AbaqusModel::PART*>::iterator pi;
		for (pi=Part.begin(); pi!=Part.end(); ++pi)
		{
			// build a part
			GObject* po = build_part(*pi);
			if (po == 0) return false;

			// if we get here we are good to go!
			const char* szname = (*pi)->GetName();
			if ((szname == 0) || (strlen(szname) == 0)) szname = szdefaultName;
			po->SetName(szname);
			fem.GetModel().AddObject(po);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::build_physics()
{
	FSModel& fem = *m_pfem;

	// add the materials
	list<AbaqusModel::MATERIAL>& Mat = m_inp.MaterialList();
	list<AbaqusModel::MATERIAL>::iterator pm = Mat.begin();
	for (int i = 0; i<(int)Mat.size(); ++i, ++pm)
	{
		FSMaterial* pmat = 0;
		switch (pm->mattype)
		{
		case AbaqusModel::ELASTIC:
			if (pm->ntype == 1)
			{
				pmat = new FSIsotropicElastic(&fem);
				pmat->SetFloatValue(FSIsotropicElastic::MP_DENSITY, pm->dens);
				pmat->SetFloatValue(FSIsotropicElastic::MP_E, pm->d[0]);
				pmat->SetFloatValue(FSIsotropicElastic::MP_v, pm->d[1]);
			}
			break;
		case AbaqusModel::ANI_HYPERELASTIC:
			pmat = new FSTransMooneyRivlin(&fem);
			pmat->SetFloatValue(FSTransMooneyRivlin::MP_DENSITY, pm->dens);
			break;
		case AbaqusModel::HYPERELASTIC:
			if (pm->ntype == 1)
			{
				pmat = new FSIncompNeoHookean(&fem);
				pmat->SetFloatValue(FSIncompNeoHookean::MP_DENSITY, pm->dens);
				pmat->SetFloatValue(FSIncompNeoHookean::MP_G, 2.0*pm->d[0]);
				pmat->SetFloatValue(FSIncompNeoHookean::MP_K, 1.0 / pm->d[1]);
			}
			else if (pm->ntype == 2)
			{
				pmat = new FSOgdenMaterial(&fem);
				if (pm->nparam == 3)
				{
					pmat->SetFloatValue(FSOgdenMaterial::MP_C1, pm->d[0]*2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M1, pm->d[1]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_K, 1.0 / pm->d[2]);
				}
				else if (pm->nparam == 6)
				{
					pmat->SetFloatValue(FSOgdenMaterial::MP_C1, pm->d[0]*2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M1, pm->d[1]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_C2, pm->d[2]*2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M2, pm->d[3]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_K, 1.0 / pm->d[4]);
				}
				else if (pm->nparam == 9)
				{
					pmat->SetFloatValue(FSOgdenMaterial::MP_C1, pm->d[0]*2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M1, pm->d[1]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_C2, pm->d[2]*2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M2, pm->d[3]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_C3, pm->d[4]*2.0);
					pmat->SetFloatValue(FSOgdenMaterial::MP_M3, pm->d[5]);
					pmat->SetFloatValue(FSOgdenMaterial::MP_K, 1.0 / pm->d[6]);
				}
			}
			break;
		default:
			assert(false);
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
			int sections = (int)pg->m_Solid.size() + (int)pg->m_Shell.size();
			if (po->Parts() == sections)
			{
				int n = 0;
				map<string, AbaqusModel::SOLID_SECTION>::iterator solidSection = pg->m_Solid.begin();
				for (int i = 0; i < pg->m_Solid.size(); ++solidSection, ++i)
				{
					AbaqusModel::SOLID_SECTION& ss = solidSection->second;
					GMaterial* pmat = fem.FindMaterial(ss.szmat);
					if (pmat) po->Part(n++)->SetMaterialID(pmat->GetID());
				}

				map<string, AbaqusModel::SHELL_SECTION>::iterator shellSection = pg->m_Shell.begin();
				for (int i = 0; i < pg->m_Shell.size(); ++shellSection, ++i)
				{
					AbaqusModel::SHELL_SECTION& ss = shellSection->second;
					GMaterial* pmat = fem.FindMaterial(ss.szmat);
					if (pmat) po->Part(n++)->SetMaterialID(pmat->GetID());
				}
			}
		}
	}
	fem.UpdateMaterialSelections();

	// add the steps
	list<AbaqusModel::STEP>& Step = m_inp.StepList();
	for (list<AbaqusModel::STEP>::iterator it = Step.begin(); it != Step.end(); ++it)
	{
		AbaqusModel::STEP& stepi = *it;
		FSNonLinearMechanics* festep = new FSNonLinearMechanics(&fem);
		STEP_SETTINGS& set = festep->GetSettings();
		set.dt = stepi.dt0;
		set.ntime = (int) (stepi.time / stepi.dt0 + 0.5);
		festep->SetName(stepi.szname);
		fem.AddStep(festep);
	}

	// add the amplitude curves
	for (int n = 0; n < m_inp.Amplitudes(); ++n)
	{
		const AbaqusModel::Amplitude& amp = m_inp.GetAmplitude(n);
		LoadCurve lc;
		lc.Clear();
		for (int i = 0; i < amp.m_points.size(); ++i)
		{
			vec2d p = amp.m_points[i];
			lc.Add(p.x(), p.y());
		}
		if (amp.m_type == AbaqusModel::Amplitude::AMP_TABULAR    ) lc.SetInterpolator(LoadCurve::LINEAR);
		if (amp.m_type == AbaqusModel::Amplitude::AMP_SMOOTH_STEP) lc.SetInterpolator(LoadCurve::SMOOTH_STEP);
		FSLoadController* plc = fem.AddLoadCurve(lc);
		plc->SetName(amp.m_name);
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
			FSNodeSet* nset = find_nodeset(bc.m_nodeSet[i].nodeSet);
			if (nset)
			{
				FSPrescribedDisplacement* pbc = new FSPrescribedDisplacement(&fem, nset, bc.m_nodeSet[i].ndof - 1, bc.m_nodeSet[i].load);
				char szname[256] = { 0 };
				sprintf(szname, "bc_%d", n);
				pbc->SetName(szname);

				if (bc.m_ampl >= 0)
				{
					FSLoadController* plc = fem.GetLoadController(bc.m_ampl);
					if (plc)
					{
						Param& p = pbc->GetParam(FSPrescribedDOF::SCALE);
						p.SetLoadCurveID(plc->GetID());
					}
				}

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
			FSSurface* surface = find_surface(p.m_surf[i].surf);
			FSPressureLoad* pl = new FSPressureLoad(&fem, surface);
			char szname[256] = {0};
			sprintf(szname, "dsload_%d", n);
			pl->SetName(szname);
			pl->SetLoad(p.m_surf[i].load);

			if (p.m_ampl >= 0)
			{
				FSLoadController* plc = fem.GetLoadController(p.m_ampl);
				if (plc)
				{
					Param& p = pl->GetParam(FSPressureLoad::LOAD);
					p.SetLoadCurveID(plc->GetID());
				}
			}

			// add it to the initial step
			fem.GetStep(0)->AddComponent(pl);
		}
	}

	// add all contacts
	for (int i = 0; i < m_inp.ContactPairs(); ++i)
	{
		const AbaqusModel::CONTACT_PAIR& cp = m_inp.GetContactPair(i);
		FSTensionCompressionInterface* ci = new FSTensionCompressionInterface(&fem);
		ci->SetName(cp.name);

		if (cp.surf1.empty() == false)
		{
			AbaqusModel::SURFACE* s1 = m_inp.FindSurface(cp.surf1.c_str());
			FSSurface* surf1 = find_surface(s1);
			ci->SetPrimarySurface(surf1);
		}

		if (cp.surf2.empty() == false)
		{
			AbaqusModel::SURFACE* s2 = m_inp.FindSurface(cp.surf2.c_str());
			FSSurface* surf2 = find_surface(s2);
			ci->SetSecondarySurface(surf2);
		}

		ci->SetFloatValue(FSTensionCompressionInterface::FRICCOEFF, cp.friction);

		fem.GetStep(0)->AddInterface(ci);
	}

	// clean up
	Mat.clear();

	// The abaqus reader currently still uses the old FE classes, so we need to convert. 
	std::ostringstream log;
	m_prj.ConvertToNewFormat(log);
	std::string s = log.str();
	if (s.empty() == false)
	{
		errf(s.c_str());
	}

	return true;
}

//-----------------------------------------------------------------------------
// Build a part
GObject* AbaqusImport::build_part(AbaqusModel::PART* pg)
{
	FSModel& fem = *m_pfem;
	GModel& gm = fem.GetModel();

	AbaqusModel::PART& part = *pg;
	assert(part.m_po == 0);

	// count nodes
	int nodes = part.Nodes();

	// count elements
	int elems = 0;
	for (AbaqusModel::Telem_itr it = pg->m_Elem.begin(); it != pg->m_Elem.end(); ++it) if (it->id != -1) elems++;

	if ((nodes == 0) || (elems == 0)) return 0;

	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// copy nodes
	int i, j, n;
	int imin = 0, imax = 0;
	AbaqusModel::Tnode_itr pn = part.m_Node.begin();
	for (i=0; i<nodes; ++i, ++pn)
	{
		FSNode& node = pm->Node(i);
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
			FSElement& el = pm->Element(i);
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

	// auto-partition
	int elsets = (int)part.m_ESet.size();
	vector<int> index; index.assign(elsets, 0);

	int solidSections = (int) part.m_Solid.size();
	int shellSections = (int) part.m_Shell.size();
	if (((solidSections > 0) || (shellSections > 0)) && m_bssection)
	{
		int gid = 0;
		map<string, AbaqusModel::SOLID_SECTION>::iterator solidSection = part.m_Solid.begin();
		for (i = 0; i< solidSections; ++i, ++solidSection)
		{
			AbaqusModel::SOLID_SECTION& ss = solidSection->second;
			AbaqusModel::ELEMENT_SET* esi = part.FindElementSet(ss.szelset);
			if (esi != nullptr)
			{
				int n = (int)esi->elem.size();
				vector<int>::iterator pe = esi->elem.begin();
				for (j = 0; j<n; ++j, ++pe)
				{
					assert(*pe != -1);
					int eid = part.FindElement(*pe)->lid;
					FSElement& el = pm->Element(eid);
					assert(el.m_gid == 0);
					el.m_gid = gid;
				}
				ss.m_pid = gid++;
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
						FSElement& el = pm->Element(eid);

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

		// now to the shells
		map<string, AbaqusModel::SHELL_SECTION>::iterator shellSection = part.m_Shell.begin();
		for (i = 0; i < shellSections; ++i, ++shellSection)
		{
			AbaqusModel::SHELL_SECTION& ss = shellSection->second;
			AbaqusModel::ELEMENT_SET* esi = part.FindElementSet(ss.szelset);
			if (esi != nullptr)
			{
				int n = (int)esi->elem.size();
				vector<int>::iterator pe = esi->elem.begin();
				for (j = 0; j < n; ++j, ++pe)
				{
					assert(*pe != -1);
					int eid = part.FindElement(*pe)->lid;
					FSElement& el = pm->Element(eid);
					el.m_gid = gid;
				}
				ss.m_pid = gid++;
			}

			AbaqusModel::Orientation* orient = part.FindOrientation(ss.szorient);
			if (orient)
			{
				AbaqusModel::Distribution* dist = part.FindDistribution(orient->szdist);
				if (dist)
				{
					// skip the first entry
					for (int j = 1; j < dist->m_data.size(); ++j)
					{
						AbaqusModel::Distribution::ENTRY& e = dist->m_data[j];
						int eid = part.FindElement(e.elem)->lid;

						vec3d a = vec3d(e.val[0], e.val[1], e.val[2]);
						vec3d b = vec3d(e.val[3], e.val[4], e.val[5]);
						FSElement& el = pm->Element(eid);

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

		pm->RebuildMesh(60.0, false);
	}
	else if (m_bautopart && (elsets > 0))
	{
		map<string, AbaqusModel::ELEMENT_SET*>::iterator pes = part.m_ESet.begin();
		for (i=0; i<elsets; ++i, ++pes)
		{
			int n = (int)pes->second->elem.size();
			vector<int>::iterator pe = pes->second->elem.begin();
			for (j=0; j<n; ++j, ++pe)
			{
				assert(*pe != -1);
				int eid = part.FindElement(*pe)->lid;
				FSElement& el = pm->Element(eid);
				el.m_gid = i;
			}
		}

		// since elements can belong to multiple sets, 
		// some parts may become empty when all elements in the
		// set also belong to another set. 
		// Therefore we reindex the element GID's
		for (int i=0; i<elems; ++i)
		{
			FSElement& el = pm->Element(i);
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
			FSElement& el = pm->Element(i);
			if (el.m_gid >= 0)
			{
				el.m_gid = index[el.m_gid];
				assert(el.m_gid != -1);
			}
		}
		pm->RebuildMesh(60.0, false);
	}
	else {
		pm->RebuildMesh(60.0, true);
	}

	// next, we will add the mesh surfaces. Before we 
	// can do that we need to build and update the mesh.
	// This is simply done by attaching the mesh to an object
	GMeshObject* po = new GMeshObject(pm);

	// rename the parts to correspond to the element sets
	if (((solidSections > 0) || (shellSections > 0)) && m_bssection)
	{
		map<string, AbaqusModel::SOLID_SECTION>::iterator solidSection = part.m_Solid.begin();
		for (i = 0; i<solidSections; ++i, ++solidSection)
		{
			AbaqusModel::SOLID_SECTION& ss = solidSection->second;
			if (ss.m_pid >= 0)
			{
				GPart* pg = po->Part(ss.m_pid); assert(pg);
				if (pg) pg->SetName(ss.szelset);
			}
		}

		map<string, AbaqusModel::SHELL_SECTION>::iterator shellSection = part.m_Shell.begin();
		for (i = 0; i < shellSections; ++i, ++shellSection)
		{
			AbaqusModel::SHELL_SECTION& ss = shellSection->second;
			if (ss.m_pid >= 0)
			{
				GPart* pg = po->Part(ss.m_pid); assert(pg);
				if (pg) pg->SetName(ss.szelset);
			}
		}
	}
	else if (m_bautopart)
	{
		map<string, AbaqusModel::ELEMENT_SET*>::iterator pes = part.m_ESet.begin();
		int m = 0;
		int elsets = (int)part.m_ESet.size();
		for (i = 0; i<elsets; ++i, ++pes)
		{
			if (index[i] >= 0)
			{
				po->Part(m++)->SetName(pes->second->szname);
			}
		}
	}

	// read element sets
	if (m_belemsets)
	{
		int elsets = (int)part.m_ESet.size();
		if (elsets)
		{
			map<string, AbaqusModel::ELEMENT_SET*>::iterator pes = part.m_ESet.begin();
			for (i=0; i<elsets; ++i, ++pes)
			{
				int n = (int)pes->second->elem.size();
				FSElemSet* pg = new FSElemSet(pm);
				pg->SetName(pes->second->szname);
				vector<int>::iterator pe = pes->second->elem.begin();
				for (j=0; j<n; ++j, ++pe) pg->add(part.FindElement(*pe)->lid);
				pm->AddFEElemSet(pg);
			}
		}
	}

	// read node sets
	if (m_bnodesets)
	{
		int nsets = (int)part.m_NSet.size();
		if (nsets)
		{
			map<string, AbaqusModel::NODE_SET*>::iterator ns = part.m_NSet.begin();
			int nn;
			for (i=0; i<nsets; ++i, ++ns)
			{
				FSNodeSet* pg = new FSNodeSet(pm);
				pg->SetName(ns->second->szname);
				list<AbaqusModel::Tnode_itr>::iterator pn = ns->second->node.begin();
				nn = (int) ns->second->node.size();
				for (j = 0; j < nn; ++j, ++pn)
				{
					int lid = pm->NodeIndexFromID((*pn)->id);
					pg->add(lid);
				}
				pm->AddFENodeSet(pg);
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
			auto si = part.m_Surf.begin();
			for (i=0; i<surfs; ++i, ++si)
			{
				FSSurface* ps = build_surface(si->second);
				if (ps)
				{
					po->AddFESurface(ps);
				}
			}
		}
	}

	// define the springs
	if (pg->Springs() > 0)
	{
		int NS = pg->Springs();
		list<AbaqusModel::SPRING_ELEMENT>::iterator ps;
		int n = 1;
		for (ps = pg->m_SpringElem.begin(); ps != pg->m_SpringElem.end(); ++ps, ++n)
		{
			AbaqusModel::SPRING_ELEMENT& s = *ps;
			AbaqusModel::Tnode_itr pn1 = part.FindNode(s.n[0]);
			AbaqusModel::Tnode_itr pn2 = part.FindNode(s.n[1]);

			int n0 = po->MakeGNode(pn1->n);
			int n1 = po->MakeGNode(pn2->n);

			GLinearSpring* ps = new GLinearSpring(&gm, n0, n1);
			char szname[256];
			sprintf(szname, "Spring%02d", n);
			ps->SetName(szname);
			ps->GetParam(GLinearSpring::MP_E).SetFloatValue(0);

			fem.GetModel().AddDiscreteObject(ps);
		}
	}
	
	if (!pg->m_Spring.empty())
	{
		int NS = pg->m_Spring.size();
		for (auto& s : pg->m_Spring)
		{
			AbaqusModel::SpringSet* set = m_inp.FindSpringSet(s.elset.c_str());
			if (set)
			{
				int nsize = set->m_Elem.size();
				GDiscreteSpringSet* dis = new GDiscreteSpringSet(&gm);
				dis->SetName(s.elset);

				auto& springs = set->m_Elem;
				for (int i = 0; i < springs.size(); ++i)
				{
					AbaqusModel::SPRING_ELEMENT& el = springs[i];

					AbaqusModel::Tnode_itr pn1 = part.FindNode(el.n[0]);
					AbaqusModel::Tnode_itr pn2 = part.FindNode(el.n[1]);

					int n0 = po->MakeGNode(pn1->n);
					int n1 = po->MakeGNode(pn2->n);

					GNode* gn0 = po->FindNode(n0);
					GNode* gn1 = po->FindNode(n1);

					dis->AddElement(gn0, gn1);
				}

				if (s.m_lc.Points() > 0)
				{
					FSModel& fem = m_prj.GetFSModel();
					FSDiscreteMaterial* dmat = FEBio::CreateDiscreteMaterial("nonlinear spring", &fem);
					dis->SetMaterial(dmat);

					FSProperty* prop = dmat->FindProperty("force");
					if (prop)
					{
						FSFunction1D* plc = FEBio::CreateFunction1D("point", &fem);
						prop->SetComponent(plc);

						Param* p = plc->GetParam("points");
						if (p)
						{
							LoadCurve& lc = s.m_lc;
							std::vector<vec2d> v = lc.GetPoints();
							p->SetVectorVec2dValue(v);
						}
					}
				}

				fem.GetModel().AddDiscreteObject(dis);
			}
		}
	}

	return po;
}

//-----------------------------------------------------------------------------
FSSurface* AbaqusImport::build_surface(AbaqusModel::SURFACE* si)
{
	if (si == nullptr) return nullptr;

	AbaqusModel::PART* part = si->part;
	if (part == 0) return 0;

	// make sure the part has an object assigned to it
	GMeshObject* po = dynamic_cast<GMeshObject*>(part->m_po);
	if (po == 0) return 0;

	FSMesh* pm = part->m_po->GetFEMesh();

	int nf, n;
	FSSurface* ps = new FSSurface(pm);
	ps->SetName(si->szname);
	nf = (int)si->face.size();
	list<AbaqusModel::FACE>::iterator pf = si->face.begin();
	AbaqusModel::Telem_itr pe;
	for (int j = 0; j<nf; ++j, ++pf)
	{
		pe = part->FindElement(pf->eid);
		FSElement& el = pm->Element(pe->lid);

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

//-----------------------------------------------------------------------------
FSSurface* AbaqusImport::find_surface(AbaqusModel::SURFACE* si)
{
	if (si == nullptr) return nullptr;

	AbaqusModel::PART* part = si->part;
	if (part == 0) return 0;

	// make sure the part has an object assigned to it
	GMeshObject* po = dynamic_cast<GMeshObject*>(part->m_po);
	if (po == 0) return 0;

	// let's see if we can find it
	return po->FindFESurface(si->szname);
}

// build a nodeset
FSNodeSet* AbaqusImport::build_nodeset(AbaqusModel::NODE_SET* ns)
{
	AbaqusModel::PART* part = ns->part;
	if (part == 0) return 0;

	// make sure the part has an object assigned to it
	GMeshObject* po = dynamic_cast<GMeshObject*>(part->m_po);
	if (po == 0) return 0;

	FSMesh* pm = part->m_po->GetFEMesh();

	FSNodeSet* nset = new FSNodeSet(pm);
	list<AbaqusModel::Tnode_itr>::iterator it = ns->node.begin();
	for (it; it != ns->node.end(); ++it)
	{
		nset->add((*it)->n);
	}
	return nset;
}

// find the nodeset
FSNodeSet* AbaqusImport::find_nodeset(AbaqusModel::NODE_SET* ns)
{
	if (ns == nullptr) return nullptr;

	AbaqusModel::PART* part = ns->part;
	if (part == 0) return 0;

	// make sure the part has an object assigned to it
	GMeshObject* po = dynamic_cast<GMeshObject*>(part->m_po);
	if (po == 0) return 0;

	// let's see if we can find it
	return po->FindFENodeSet(ns->szname);
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_step(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);

	const char* szname = find_attribute(att, 5, "name");
	AbaqusModel::STEP* step = m_inp.AddStep(szname);
	step->dt0 = 1.0;
	step->time = 1;

	// parse till END STEP
	while (!feof(fp))
	{
		if (szicnt(szline, "*STATIC"))
		{
			if (read_static(szline, fp) == false)
			{
				errf("Error reading *STATIC keyword (line %d)", m_nline);
			}
		}
		else if (szicnt(szline, "*DSLOAD"))
		{
			if (read_dsload(szline, fp) == false)
			{
				errf("Error reading *DSLOAD keyword (line %d)", m_nline);
			}
		}
		else if (szicnt(szline, "*BOUNDARY"))
		{
			if (read_boundary(szline, fp) == false)
			{
				errf("Error reading *BOUNDARY keyword (line %d)", m_nline);
			}
		}
		else if (szicnt(szline, "*END STEP"))
		{
			m_inp.SetCurrentStep(0);
			return true;
		}
		else read_line(szline, fp);
	}

	return false;
}

bool AbaqusImport::read_boundary(char* szline, FILE* fp)
{
	AbaqusModel::BOUNDARY BC;
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	const char* szampl = find_attribute(att, 4, "amplitude");
	if (szampl)
	{
		BC.m_ampl = m_inp.FindAmplitude(szampl);
	}
	else BC.m_ampl = -1;

	read_line(szline, fp);

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
				AbaqusModel::PART* part = nullptr;
				int nid = -1;
				const char* ch = strchr(szset, '.');
				if (ch)
				{
					char szbuf[256] = { 0 };
					strncpy(szbuf, szset, ch - szset);
					AbaqusModel::INSTANCE* inst = m_inp.FindInstance(szbuf);
					if (inst == nullptr) return false;

					const char* nset = ch + 1;
					ns = inst->GetPart()->FindNodeSet(nset);
					if (ns)
					{
						BC.add(ns, ndof, val);
					}
					else
					{
						part = inst->GetPart();
						nid = atoi(ch + 1);
					}
				}
				else
				{
					part = m_inp.CurrentPart(); 
					nid = atoi(szset);
				}

				
				if (part == nullptr) return false;

				AbaqusModel::NODE_SET* dummy = part->AddNodeSet(szset);
				dummy->node.push_back(part->FindNode(nid));
				BC.add(dummy, ndof, val);
			}
			else BC.add(ns, ndof, val);
		}
		read_line(szline, fp);
	}

	m_inp.AddBoundaryCondition(BC);

	return true;	
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_dsload(char* szline, FILE* fp)
{
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
			AbaqusModel::SURFACE* s = m_inp.FindSurface(szsurf);
			if (s == nullptr) errf("Warning: Failed to find surface \"%s\"", szsurf);
			double val = atof(att[2].szatt);
			P.add(s, val);
		}
		read_line(szline, fp);
	}

	m_inp.AddPressureLoad(P);

	return true;
}

//-----------------------------------------------------------------------------
bool AbaqusImport::read_solid_section(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
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
bool AbaqusImport::read_shell_section(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	int n = parse_line(szline, att);

	AbaqusModel::PART* pg = m_inp.GetActivePart(true);
	if (pg == 0) return false;

	const char* szelset = find_attribute(att, 5, "elset");
	if (szelset == 0) return false;

	const char* szmat = find_attribute(att, 5, "material");
	const char* szorient = find_attribute(att, 5, "orientation");
	AbaqusModel::SHELL_SECTION& ss = pg->AddShellSection(szelset, szmat, szorient);

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
	ATTRIBUTE att[MAX_ATTRIB];
	parse_line(szline, att);
	const char* szname = find_attribute(att, 3, "name");
	read_line(szline, fp);

	AbaqusModel::PART* pg = m_inp.CurrentPart();
	if (pg == 0)
	{
		errf("Failed reading ORIENTATION keyword (line %d)", m_nline);
		skip_keyword(szline, fp);
		return true;
	}

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

	m_inp.AddContactPair(cp);

	return true;
}

bool AbaqusImport::read_spring(char* szline, FILE* fp)
{
	ATTRIBUTE att[MAX_ATTRIB];
	int natt = parse_line(szline, att);
	bool nonlinear = false;

	AbaqusModel::SPRING spring;

	if (szicnt(att[1].szatt, "ELSET"))
	{
		const char* szset = att[1].szval;
		spring.elset = szset;
	}
	if (szicnt(att[2].szatt, "nonlinear"))
	{
		spring.nonlinear = true;
	}

	// skip a line
	read_line(szline, fp);
	read_line(szline, fp);
	LoadCurve& lc = spring.m_lc;
	lc.Clear();
	while (!feof(fp) && (szline[0] != '*'))
	{
		double x, y;
		sscanf(szline, "%lg,%lg", &y, &x);
		lc.Add(x, y);
		read_line(szline, fp);
	}

	return true;
}
