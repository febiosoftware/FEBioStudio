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

// FELSDYNAimport.cpp: implementation of the FELSDYNAimport class.
//
//////////////////////////////////////////////////////////////////////

#include "FELSDYNAimport.h"
#include "GeomLib/GMeshObject.h"
#include <MeshTools/GModel.h>
#include <vector>
//using namespace std;

//////////////////////////////////////////////////////////////////////
// FELSDYNAimport::CARD
//////////////////////////////////////////////////////////////////////

FELSDYNAimport::CARD::CARD(int field)
{
	m_szline[0] = 0;
	m_ch = 0;
	m_bfree = false;
	m_nfield = field;
}

bool FELSDYNAimport::CARD::nextd(double& d, int nwidth)
{
	d = 0;
	if (m_ch == 0) return false;

	if (nwidth == -1) nwidth = m_nfield;

	if (m_bfree)
	{
		char* ch = strchr(m_ch, ',');
		if (ch) *ch = 0;
		d = atof(m_ch);
		if (ch) m_ch = ch+1; else m_ch = 0;
	}
	else
	{
        char sz[256] = {0};
        strncpy(sz,m_ch,nwidth);
        d = atof(sz);
		m_ch += nwidth;
		if ((*m_ch == 0) || (m_ch - m_szline >= m_l)) m_ch = 0;
	}

	return true;
}

bool FELSDYNAimport::CARD::nexti(int& n, int nwidth)
{
	n = 0;
	if (m_ch == 0) return false;

	if (nwidth == -1) nwidth = m_nfield;

	if (m_bfree)
	{
		char* ch = strchr(m_ch, ',');
		if (ch) *ch = 0;
		n = atoi(m_ch);
		if (ch) m_ch = ch+1; else m_ch = 0;
	}
	else
	{
        char sz[256] = {0};
        strncpy(sz,m_ch,nwidth);
        n = atoi(sz);
		m_ch += nwidth;
		if ((*m_ch == 0) || (m_ch - m_szline >= m_l)) m_ch = 0;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FELSDYNAimport::FELSDYNAimport(FSProject& prj) : FEFileImport(prj)
{
	m_pprj = 0;
    m_lineno = 0;
}

FELSDYNAimport::~FELSDYNAimport()
{

}

bool FELSDYNAimport::ReadCard(FELSDYNAimport::CARD& c)
{
	// get the next line
	if (get_line(c.m_szline) == 0) return false;

	// figure out which format is used
	if (strchr(c.m_szline, ',')) c.m_bfree = true; else c.m_bfree = false;

	// set the intitial pointer
	c.m_ch = c.m_szline;

	c.m_l = (int)strlen(c.m_szline);

	return true;
}

char* FELSDYNAimport::get_line(char* szline)
{
	do
	{
		fgets(szline, 255, m_fp);
		if (feof(m_fp)) return 0;
        ++m_lineno;
	}
	while (szline[0] == '$');

	char* ch = strrchr(szline, '\n');
	if (ch) *ch = 0;

	return szline;
}

bool FELSDYNAimport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// open the file
	if (Open(szfile, "rt") == false) return errf("Cannot open the file %s", szfile);

	// make sure the first line is a *KEYWORD
	if (get_line(m_szline) == 0) return errf("FATAL ERROR: Unexpected end of file.");
	if (szcmp(m_szline, "*KEYWORD") != 0) return errf("FATAL ERROR: This is not a LSDYNA Keyword file.");

	// clear all data
	m_dyna.clear();

	// get the next line
	if (get_line(m_szline) == 0) return errf("FATAL ERROR: Unexpected end of file.");

	// repeat until done
	bool bdone = false;
	do
	{
		if (szcmp(m_szline, "*ELEMENT_SOLID") == 0)
		{
			if (strstr(m_szline, "(ten nodes format)"))
			{
				if (Read_Element_Solid2() == false) return errf("FATAL ERROR: error while reading ELEMENT_SOLID section.");
			}
			else
			{
				if (Read_Element_Solid() == false) return errf("FATAL ERROR: error while reading ELEMENT_SOLID section.");
			}
		}
		else if (szcmp(m_szline, "*ELEMENT_SHELL_THICKNESS") == 0)
		{
			if (Read_Element_Shell_Thickness() == false) return errf("FATAL ERROR: error while readin ELEMENT_SHELL_THICKNESS section.");
		}
		else if (szcmp(m_szline, "*ELEMENT_SHELL") == 0)
		{
			if (Read_Element_Shell() == false) return errf("FATAL ERROR: error while reading ELEMENT_SHELL section.");
		}
        else if (szcmp(m_szline, "*SECTION_SHELL") == 0)
        {
            if (Read_Domain_Shell_Thickness() == false) return errf("FATAL ERROR: error while reading SECTION_SHELL section.");
        }
		else if (szcmp(m_szline, "*NODE") == 0)
		{
			if (Read_Node() == false) return errf("FATAL ERROR: error while reading NODE section.");
		}
		else if (szcmp(m_szline, "*NODAL_RESULTS") == 0)
		{
			if (Read_Nodal_Results() == false) return errf("FATAL ERROR: error while reading NODAL_RESULTS section.");
		}
        else if (szcmp(m_szline, "*PART_CONTACT") == 0)
        {
            if (Read_Part_Contact() == false)
                return errf("FATAL ERROR: error while reading PART_CONTACT section at line %lu.", m_lineno);
        }
		else if (szcmp(m_szline, "*PART") == 0)
		{
            if (Read_Part() == false)
                return errf("FATAL ERROR: error while reading PART section at line %lu.", m_lineno);
		}
        else if (szcmp(m_szline, "*MAT_") == 0)
        {
            if (Read_Material() == false) return errf("FATAL ERROR: error while reading %s section.", m_szline);
        }
		else if (szcmp(m_szline, "*SET_SEGMENT_TITLE") == 0)
		{
			if (Read_Set_Segment_Title() == false) return errf("FATAL ERROR: error while reading SET_SEGMENT_TITLE section.");
		}
		else if (szcmp(m_szline, "*END") == 0)
		{
			bdone = true;
		}
		else if (get_line(m_szline) == 0) return errf("FATAL ERROR: unexpected end of file.");
	}
	while (!bdone);

	// close the file
	Close();

	// build the model
	bool b = m_dyna.BuildModel(fem);
	if (b)
	{
		// if we get here we are good to go!
		GMeshObject* po = m_dyna.TakeObject();
		char szname[256];
		FileTitle(szname);
		po->SetName(szname);
		fem.GetModel().AddObject(po);
	}

	// clean up
	m_dyna.clear();

	return (b ? true : errf("Failed building model"));
}

bool FELSDYNAimport::Read_Element_Solid()
{
	CARD card(8);
	if (ReadCard(card) == false) return false;
	LSDYNAModel::ELEMENT_SOLID el;
	while (!card.IsKeyword())
	{
		if (card.nexti(el.eid) == false) return false;
		if (card.nexti(el.pid) == false) return false;
		if (card.nexti(el.n[0]) == false) return false;
		if (card.nexti(el.n[1]) == false) return false;
		if (card.nexti(el.n[2]) == false) return false;
		if (card.nexti(el.n[3]) == false) return false;
		if (card.nexti(el.n[4]) == false) return false;
		if (card.nexti(el.n[5]) == false) return false;
		if (card.nexti(el.n[6]) == false) return false;
		if (card.nexti(el.n[7]) == false) return false;

		m_dyna.addSolidElement(el);

		if (ReadCard(card) == false) return false;
	}

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Element_Solid2()
{
	CARD card(8);
	if (ReadCard(card) == false) return false;
	LSDYNAModel::ELEMENT_SOLID el;
	while (!card.IsKeyword())
	{
		if (card.nexti(el.eid) == false) return false;
		if (card.nexti(el.pid) == false) return false;

		if (ReadCard(card) == false) return false;
		if (card.nexti(el.n[0]) == false) return false;
		if (card.nexti(el.n[1]) == false) return false;
		if (card.nexti(el.n[2]) == false) return false;
		if (card.nexti(el.n[3]) == false) return false;
		if (card.nexti(el.n[4]) == false) return false;
		if (card.nexti(el.n[5]) == false) return false;
		if (card.nexti(el.n[6]) == false) return false;
		if (card.nexti(el.n[7]) == false) return false;

		m_dyna.addSolidElement(el);

		if (ReadCard(card) == false) return false;
	}

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Element_Shell()
{
	CARD card(8);
	if (ReadCard(card) == false) return false;
	LSDYNAModel::ELEMENT_SHELL el;
	while (!card.IsKeyword())
	{
		if (card.nexti(el.eid) == false) return false;
		if (card.nexti(el.pid) == false) return false;
		if (card.nexti(el.n[0]) == false) return false;
		if (card.nexti(el.n[1]) == false) return false;
		if (card.nexti(el.n[2]) == false) return false;
		if (card.nexti(el.n[3]) == false) return false;

		m_dyna.addShellElement(el);

		if (ReadCard(card) == false) return false;
	}

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Element_Shell_Thickness()
{
	CARD card(8);
	if (ReadCard(card) == false) return false;
	LSDYNAModel::ELEMENT_SHELL el;
	while (!card.IsKeyword())
	{
		if (card.nexti(el.eid) == false) return false;
		if (card.nexti(el.pid) == false) return false;
		if (card.nexti(el.n[0]) == false) return false;
		if (card.nexti(el.n[1]) == false) return false;
		if (card.nexti(el.n[2]) == false) return false;
		if (card.nexti(el.n[3]) == false) return false;

		if (ReadCard(card) == false) return false;

		if (card.nextd(el.h[0], 16) == false) return false;
		if (card.nextd(el.h[1], 16) == false) return false;
		if (card.nextd(el.h[2], 16) == false) return false;
		if (card.nextd(el.h[3], 16) == false) return false;

		m_dyna.addShellElement(el);

		if (ReadCard(card) == false) return false;
	}

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Domain_Shell_Thickness()
{
    CARD card;
    // read first set of domain properties
    if (ReadCard(card) == false) return false;
    LSDYNAModel::DOMAIN_SHELL ds;
    if (card.nexti(ds.pid) == false) return false;
    
    // read second set of domain properties
    if (ReadCard(card) == false) return false;
    if (card.nextd(ds.h[0]) == false) return false;
    if (card.nextd(ds.h[1]) == false) return false;
    if (card.nextd(ds.h[2]) == false) return false;
    if (card.nextd(ds.h[3]) == false) return false;

    m_dyna.addShellDomain(ds);

    // read next card
    if (ReadCard(card) == false) return false;
    while (!card.IsKeyword())
    {
        if (ReadCard(card) == false) return false;
    }
    
    strcpy(m_szline, card.m_szline);
    
    return true;
}


bool FELSDYNAimport::Read_Node()
{
	CARD card(8);
	if (ReadCard(card) == false) return false;
	LSDYNAModel::NODE n;
	while (!card.IsKeyword())
	{
		if (card.nexti(n.id) == false) return false;
		n.x = n.y = n.z = 0;

		card.nextd(n.x, 16);
		card.nextd(n.y, 16);
		card.nextd(n.z, 16);

		m_dyna.addNode(n);

		if (ReadCard(card) == false) return false;
	}

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Nodal_Results()
{
	// assign nodal data
	int N = m_dyna.nodes();
	if (N == 0) return false;
	m_dyna.allocData(N);

	// read nodal data
	CARD card(2);
	if (ReadCard(card) == false) return false;
	int n;
	double v;
	while (!card.IsKeyword())
	{
		if (card.nexti(n, 8) == false) return false;

		card.nextd(v, 16);
		m_dyna.NodeData(n-1, 0) = v;

		card.nextd(v, 16);
		m_dyna.NodeData(n-1, 1) = v;

		if (ReadCard(card) == false) return false;
	}

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Part()
{
	CARD card;
	if (ReadCard(card) == false) return false;
	LSDYNAModel::PART p;
	int m = (int)strlen(card.m_szline);
	if (m > 80) m = 80;
	strncpy(p.szname, card.m_szline, m);
	p.szname[m] = 0;
	int l = (int)strlen(p.szname);
	if (l > 0)
	{
		char* ch = p.szname + l-1;
		while ((*ch == ' ') && (ch != p.szname)) --ch;
		ch[1] = 0;
	}
	else sprintf(p.szname, "Part%02d", (int) (m_dyna.parts()+1));

	if (ReadCard(card) == false) return false;
	if (card.nexti(p.pid) == false) return false;
	if (card.nexti(p.sid) == false) return false;
	if (card.nexti(p.mid) == false) return false;

	m_dyna.addPart(p);

	if (ReadCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Part_Contact()
{
    CARD card;
    if (ReadCard(card) == false) return false;
    if (ReadCard(card) == false) return false;
    if (ReadCard(card) == false) return false;
    if (ReadCard(card) == false) return false;
    if (!card.IsKeyword()) return false;
    
    strcpy(m_szline, card.m_szline);
    
    return true;
}

bool FELSDYNAimport::Read_Material()
{
    if (szcmp(m_szline, "*MAT_ELASTIC_") == 0)
        return Read_Mat_Other();
    else if (szcmp(m_szline, "*MAT_ELASTIC") == 0)
        return Read_Mat_Elastic();
    else if (szcmp(m_szline, "*MAT_RIGID") == 0)
        return Read_Mat_Rigid();
    else if (szcmp(m_szline, "*MAT_VISCOELASTIC") == 0)
        return Read_Mat_Viscoelastic();
    else if (szcmp(m_szline, "*MAT_KELVIN-MAXWELL_VISCOELASTIC") == 0)
        return Read_Mat_Kelvin_Maxwell_Viscoelastic();
    else
        return Read_Mat_Other();
}

bool FELSDYNAimport::Read_Mat_Other()
{
    CARD card;
    if (ReadCard(card) == false) return false;
    LSDYNAModel::MATERIAL *mat;
    mat = new LSDYNAModel::MATERIAL;
    strcpy(mat->szname, m_szline);
    if (strstr(m_szline, "_TITLE") != 0) {
        if (ReadCard(card) == false) return false;
    }
    card.nexti(mat->mid);
	m_dyna.addMaterial(mat);
    while (!card.IsKeyword()) {
        if (ReadCard(card) == false) return false;
    }
    
    strcpy(m_szline, card.m_szline);
    
    return true;
}

bool FELSDYNAimport::Read_Mat_Elastic()
{
	CARD card;
	if (ReadCard(card) == false) return false;
	LSDYNAModel::MAT_ELASTIC *mat;
    mat = new LSDYNAModel::MAT_ELASTIC;
    if (strstr(m_szline, "_TITLE") != 0) {
        strcpy(mat->szname, card.m_szline);
        if (ReadCard(card) == false) return false;
    }
	card.nexti(mat->mid);
	card.nextd(mat->ro);
	card.nextd(mat->e);
	card.nextd(mat->pr);
    card.nextd(mat->da);
    card.nextd(mat->db);

	// TODO: I don't think this works
	m_dyna.addMaterial(mat);

	if (ReadCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	strcpy(m_szline, card.m_szline);

	return true;
}

bool FELSDYNAimport::Read_Mat_Viscoelastic()
{
    CARD card;
    if (ReadCard(card) == false) return false;
	LSDYNAModel::MAT_VISCOELASTIC *mat;
    mat = new LSDYNAModel::MAT_VISCOELASTIC;
    if (strstr(m_szline, "_TITLE") != 0) {
        strcpy(mat->szname, card.m_szline);
        if (ReadCard(card) == false) return false;
    }
    card.nexti(mat->mid);
    card.nextd(mat->ro);
    card.nextd(mat->bulk);
    card.nextd(mat->g0);
    card.nextd(mat->gi);
    card.nextd(mat->beta);

	// TODO: I don't think this works
	m_dyna.addMaterial(mat);

    if (ReadCard(card) == false) return false;
    if (!card.IsKeyword()) return false;
    
    strcpy(m_szline, card.m_szline);
    
    return true;
}

bool FELSDYNAimport::Read_Mat_Kelvin_Maxwell_Viscoelastic()
{
    CARD card;
    if (ReadCard(card) == false) return false;
    LSDYNAModel::MAT_KELVIN_MAXWELL_VISCOELASTIC *mat;
	mat = new LSDYNAModel::MAT_KELVIN_MAXWELL_VISCOELASTIC;
    if (strstr(m_szline, "_TITLE") != 0) {
        strcpy(mat->szname, card.m_szline);
        if (ReadCard(card) == false) return false;
    }
    card.nexti(mat->mid);
    card.nextd(mat->ro);
    card.nextd(mat->bulk);
    card.nextd(mat->g0);
    card.nextd(mat->gi);
    card.nextd(mat->dc);
    card.nextd(mat->fo);
    card.nextd(mat->so);

	// TODO: I don't think this works
	m_dyna.addMaterial(mat);

    if (ReadCard(card) == false) return false;
    if (!card.IsKeyword()) return false;
    
    strcpy(m_szline, card.m_szline);
    
    return true;
}

bool FELSDYNAimport::Read_Mat_Rigid()
{
    CARD card;
    if (ReadCard(card) == false) return false;
	LSDYNAModel::MAT_RIGID *mat;
    mat = new LSDYNAModel::MAT_RIGID;
    if (strstr(m_szline, "_TITLE") != 0) {
        strcpy(mat->szname, card.m_szline);
        if (ReadCard(card) == false) return false;
    }
    card.nexti(mat->mid);
    card.nextd(mat->ro);
    card.nextd(mat->e);
    card.nextd(mat->pr);
    card.nextd(mat->n);
    card.nextd(mat->couple);
    card.nextd(mat->m);
    card.nextd(mat->alias);

    if (ReadCard(card) == false) return false;
    card.nextd(mat->cmo);
    card.nexti(mat->con1);
    card.nexti(mat->con2);

    if (ReadCard(card) == false) return false;
    card.nextd(mat->a1);
    card.nextd(mat->a2);
    card.nextd(mat->a3);
    card.nextd(mat->v1);
    card.nextd(mat->v2);
    card.nextd(mat->v3);

	// TODO: I don't think this works
	m_dyna.addMaterial(mat);

    if (ReadCard(card) == false) return false;
    if (!card.IsKeyword()) return false;
    
    strcpy(m_szline, card.m_szline);
    
    return true;
}

bool FELSDYNAimport::Read_Mat_Elastic_Spring_Discrete_Beam()
{
    CARD card;
    if (ReadCard(card) == false) return false;
	LSDYNAModel::MATERIAL *mat;
    mat = new LSDYNAModel::MATERIAL;
    strcpy(mat->szname, m_szline);
    if (strstr(m_szline, "_TITLE") != 0) {
        if (ReadCard(card) == false) return false;
    }
    card.nexti(mat->mid);

	// TODO: I don't think this works
	m_dyna.addMaterial(mat);

    while (!card.IsKeyword()) {
        if (ReadCard(card) == false) return false;
    }
    
    strcpy(m_szline, card.m_szline);
    
    return true;
}

bool FELSDYNAimport::Read_Set_Segment_Title()
{
	CARD card;
	if (ReadCard(card) == false) return false;
	LSDYNAModel::SET_SEGMENT_TITLE s;
	strcpy(s.m_szname, card.szvalue());
	if (ReadCard(card) == false) return false;
	card.nexti(s.m_nsid);

	if (ReadCard(card) == false) return false;
	while (!card.IsKeyword())
	{
		LSDYNAModel::SET_SEGMENT_TITLE::FACE f;
		card.nexti(f.n[0]);
		card.nexti(f.n[1]);
		card.nexti(f.n[2]);
		card.nexti(f.n[3]);
		s.m_face.push_back(f);
		if (ReadCard(card) == false) return false;
	}

	m_dyna.addSetSegmentTitle(s);

	strcpy(m_szline, card.m_szline);

	return true;
}
