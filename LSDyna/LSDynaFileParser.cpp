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
#include "LSDynaFileParser.h"
#include <sstream>

void LSDynaFileParser::ClearError()
{
	m_err.clear();
}

bool LSDynaFileParser::Error(const std::string& err)
{
	size_t line = m_ls.CurrentLineNumber();
	std::stringstream ss;
	ss << err << " (line " << line << ")";
	m_err += ss.str() + "\n";
	return false;
}

bool LSDynaFileParser::ParseFile()
{
	LSDynaFile::CARD card;
	bool bdone = false;
	do
	{
		// get card at current line
		m_ls.GetCard(card);

		// process card
		if (card.IsKeyword())
		{
			if (card == "*KEYWORD") { m_ls.NextCard(card); }
			else if (card == "*TITLE")
			{
				m_ls.NextCard(card);
				m_ls.NextCard(card);
			}
			else if (card == "*PARAMETER")
			{
				if (Read_Parameter() == false) return Error("error while reading PARAMETER section.");
			}
			else if (card == "*PARAMETER_EXPRESSION")
			{
				if (Read_Parameter_Expression() == false) return Error("error while reading PARAMETER_EXPRESSION section.");
			}
			else if (card == "*DEFINE_CURVE_TITLE")
			{
				if (Read_Define_Curve_Title() == false) return Error("error while reading DEFINE_CURVE_TITLE section.");
			}
			else if (card == "*DEFINE_CURVE")
			{
				if (Read_Define_Curve() == false) return Error("error while reading DEFINE_CURVE section.");
			}
			else if (card == "*ELEMENT_SOLID")
			{
				if (Read_Element_Solid() == false) return Error("error while reading ELEMENT_SOLID section.");
			}
			else if (card == "*ELEMENT_SHELL_THICKNESS")
			{
				if (Read_Element_Shell_Thickness() == false) return Error("error while readin ELEMENT_SHELL_THICKNESS section.");
			}
			else if (card == "*ELEMENT_SHELL")
			{
				if (Read_Element_Shell() == false) return Error("error while reading ELEMENT_SHELL section.");
			}
			else if (card == "*SECTION_SHELL")
			{
				if (Read_Section_Shell() == false) return Error("error while reading SECTION_SHELL section.");
			}
			else if (card == "*SECTION_SOLID_TITLE")
			{
				if (Read_Section_Solid_Title() == false) return Error("error while reading SECTION_SOLID_TITLE section.");
			}
			else if (card == "*SECTION_SOLID")
			{
				if (Read_Section_Solid() == false) return Error("error while reading SECTION_SOLID section.");
			}
			else if (card == "*ELEMENT_DISCRETE")
			{
				if (Read_Element_Discrete() == false) return Error("error while reading ELEMENT_DISCRETE section.");
			}
			else if (card == "*NODE")
			{
				if (Read_Node() == false) return Error("error while reading NODE section.");
			}
			else if (card == "*NODAL_RESULTS")
			{
				if (Read_Nodal_Results() == false) return Error("error while reading NODAL_RESULTS section.");
			}
			else if (card == "*PART_CONTACT")
			{
				if (Read_Part_Contact() == false)
					return Error("error while reading PART_CONTACT section.");
			}
			else if (card == "*PART")
			{
				if (Read_Part() == false) return Error("error while reading PART section.");
			}
			else if (card.contains("*MAT_"))
			{
				if (Read_Material() == false) return Error("error while reading MATERIAL section.");
			}
			else if (card == "*SET_NODE_LIST_TITLE")
			{
				if (Read_Set_Node_List_Title() == false) return Error("error while reading SET_NODE_LIST_TITLE section.");
			}
			else if (card == "*SET_SEGMENT_TITLE")
			{
				if (Read_Set_Segment_Title() == false) return Error("error while reading SET_SEGMENT_TITLE section.");
			}
			else if (card == "*INCLUDE")
			{
				if (Read_Include() == false) return false;
			}
			else if (card == "*END")
			{
				bdone = true;
				break;
			}
			else
			{
				// unrecognized keyword. let's skip
				Error(std::string("Unrecognized keyword ") + std::string(card.m_szline));
				if (m_ls.NextCard(card) == false)
				{
					// oh oh, looks like we ran out of file
					break;
				}
			}
		}
		else
		{
			// we can get here if the keyword parser did not increment the last read line
			// or if the keyword is not known. 
			if (m_ls.NextCard(card) == false)
			{
				// oh oh, looks like we ran out of file
				break;
			}
		}
	} while (!bdone);

	return bdone;
}

bool LSDynaFileParser::Read_Element_Solid()
{
	LSDynaFile::CARD card(8);
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::ELEMENT_SOLID el;
	while (!card.IsKeyword())
	{
		if (card.nexti(el.eid) == false) return false;
		if (card.nexti(el.pid) == false) return false;

		// try to read the next card
		if (card.nexti(el.n[0]) == false)
		{
			// probably 10-node format
			if (m_ls.NextCard(card) == false) return false;
			if (card.nexti(el.n[0]) == false) return false;
			if (card.nexti(el.n[1]) == false) return false;
			if (card.nexti(el.n[2]) == false) return false;
			if (card.nexti(el.n[3]) == false) return false;
			if (card.nexti(el.n[4]) == false) return false;
			if (card.nexti(el.n[5]) == false) return false;
			if (card.nexti(el.n[6]) == false) return false;
			if (card.nexti(el.n[7]) == false) return false;
		}
		else
		{
			if (card.nexti(el.n[1]) == false) return false;
			if (card.nexti(el.n[2]) == false) return false;
			if (card.nexti(el.n[3]) == false) return false;
			if (card.nexti(el.n[4]) == false) return false;
			if (card.nexti(el.n[5]) == false) return false;
			if (card.nexti(el.n[6]) == false) return false;
			if (card.nexti(el.n[7]) == false) return false;
		}

		m_dyn.addSolidElement(el);

		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Element_Shell()
{
	LSDynaFile::CARD card(8);
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::ELEMENT_SHELL el;
	while (!card.IsKeyword())
	{
		if (card.nexti(el.eid) == false) return false;
		if (card.nexti(el.pid) == false) return false;
		if (card.nexti(el.n[0]) == false) return false;
		if (card.nexti(el.n[1]) == false) return false;
		if (card.nexti(el.n[2]) == false) return false;
		if (card.nexti(el.n[3]) == false) return false;

		m_dyn.addShellElement(el);

		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Element_Shell_Thickness()
{
	LSDynaFile::CARD card(8);
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::ELEMENT_SHELL el;
	while (!card.IsKeyword())
	{
		if (card.nexti(el.eid) == false) return false;
		if (card.nexti(el.pid) == false) return false;
		if (card.nexti(el.n[0]) == false) return false;
		if (card.nexti(el.n[1]) == false) return false;
		if (card.nexti(el.n[2]) == false) return false;
		if (card.nexti(el.n[3]) == false) return false;

		if (m_ls.NextCard(card) == false) return false;

		if (card.nextd(el.h[0], 16) == false) return false;
		if (card.nextd(el.h[1], 16) == false) return false;
		if (card.nextd(el.h[2], 16) == false) return false;
		if (card.nextd(el.h[3], 16) == false) return false;

		m_dyn.addShellElement(el);

		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Section_Shell()
{
	LSDynaFile::CARD card;
	// read first set of domain properties
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::SECTION_SHELL ds;
	if (card.nexti(ds.secid) == false) return false;

	// read second set of domain properties
	if (m_ls.NextCard(card) == false) return false;
	if (card.nextd(ds.h[0]) == false) return false;
	if (card.nextd(ds.h[1]) == false) return false;
	if (card.nextd(ds.h[2]) == false) return false;
	if (card.nextd(ds.h[3]) == false) return false;

	m_dyn.addShellSection(ds);

	// read next card
	if (m_ls.NextCard(card) == false) return false;
	while (!card.IsKeyword())
	{
		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Section_Solid()
{
	LSDYNAModel::SECTION_SOLID ss;
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return false;
	if (card.nexti(ss.secid) == false) return false;
	if (card.nexti(ss.elform) == false) return false;
	m_dyn.addSolidSection(ss);
	return true;
}

bool LSDynaFileParser::Read_Section_Solid_Title()
{
	LSDynaFile::CARD card;

	// read title
	if (m_ls.NextCard(card) == false) return false;

	LSDYNAModel::SECTION_SOLID ss;
	if (m_ls.NextCard(card) == false) return false;
	if (card.nexti(ss.secid) == false) return false;
	if (card.nexti(ss.elform) == false) return false;
	m_dyn.addSolidSection(ss);
	return true;
}

bool LSDynaFileParser::Read_Element_Discrete()
{
	LSDynaFile::CARD card(8);
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::ELEMENT_DISCRETE e;
	while (!card.IsKeyword())
	{
		card.nexti(e.eid);
		card.nexti(e.pid);
		card.nexti(e.n1);
		card.nexti(e.n2);

		m_dyn.addDiscrete(e);

		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Node()
{
	LSDynaFile::CARD card(8);
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::NODE n;
	while (!card.IsKeyword())
	{
		if (card.nexti(n.id) == false) return false;
		n.x = n.y = n.z = 0;

		card.nextd(n.x, 16);
		card.nextd(n.y, 16);
		card.nextd(n.z, 16);

		m_dyn.addNode(n);

		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Nodal_Results()
{
	// assign nodal data
	int N = m_dyn.nodes();
	if (N == 0) return false;
	m_dyn.allocData(N);

	// read nodal data
	LSDynaFile::CARD card(2);
	if (m_ls.NextCard(card) == false) return false;
	int n;
	double v;
	while (!card.IsKeyword())
	{
		if (card.nexti(n, 8) == false) return false;

		card.nextd(v, 16);
		m_dyn.NodeData(n - 1, 0) = v;

		card.nextd(v, 16);
		m_dyn.NodeData(n - 1, 1) = v;

		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Part()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::PART p;
	int m = (int)strlen(card.m_szline);
	if (m > 80) m = 80;
	strncpy(p.szname, card.m_szline, m);
	p.szname[m] = 0;
	int l = (int)strlen(p.szname);
	if (l > 0)
	{
		char* ch = p.szname + l - 1;
		while ((*ch == ' ') && (ch != p.szname)) --ch;
		ch[1] = 0;
	}
	else sprintf(p.szname, "Part%02d", (int)(m_dyn.parts() + 1));

	if (m_ls.NextCard(card) == false) return false;
	if (card.nexti(p.pid) == false) return false;
	if (card.nexti(p.sid) == false) return false;
	if (card.nexti(p.mid) == false) return false;

	m_dyn.addPart(p);

	if (m_ls.NextCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	return true;
}

bool LSDynaFileParser::Read_Part_Contact()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return false;
	if (m_ls.NextCard(card) == false) return false;
	if (m_ls.NextCard(card) == false) return false;
	if (m_ls.NextCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	return true;
}

bool LSDynaFileParser::Read_Material()
{
	LSDynaFile::CARD c;
	m_ls.GetCard(c);
	if (c.contains("*MAT_ELASTIC_"))
		return Read_Mat_Other();
	else if (c.contains("*MAT_ELASTIC"))
		return Read_Mat_Elastic();
	else if (c.contains("*MAT_RIGID"))
		return Read_Mat_Rigid();
	else if (c.contains("*MAT_VISCOELASTIC"))
		return Read_Mat_Viscoelastic();
	else if (c.contains("*MAT_KELVIN-MAXWELL_VISCOELASTIC"))
		return Read_Mat_Kelvin_Maxwell_Viscoelastic();
	else if (c.contains("*MAT_SPRING"))
		return Read_Mat_Spring();
	else
		return Read_Mat_Other();
}

bool LSDynaFileParser::Read_Mat_Other()
{
	LSDynaFile::CARD card;
	m_ls.GetCard(card);
	LSDYNAModel::MATERIAL* mat;
	mat = new LSDYNAModel::MATERIAL;
	if (strstr(card.m_szline, "_TITLE") != 0) {
		if (m_ls.NextCard(card) == false) return false;
		strcpy(mat->szname, card.m_szline);
	}
	if (m_ls.NextCard(card) == false) return false;
	card.nexti(mat->mid);
	m_dyn.addMaterial(mat);
	while (!card.IsKeyword()) {
		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Mat_Elastic()
{
	LSDynaFile::CARD card;
	m_ls.GetCard(card);
	LSDYNAModel::MAT_ELASTIC* mat;
	mat = new LSDYNAModel::MAT_ELASTIC;
	if (strstr(card.m_szline, "_TITLE") != 0) {
		if (m_ls.NextCard(card) == false) return false;
		strcpy(mat->szname, card.m_szline);
	}
	if (m_ls.NextCard(card) == false) return false;
	card.nexti(mat->mid);
	card.nextd(mat->ro);
	card.nextd(mat->e);
	card.nextd(mat->pr);
	card.nextd(mat->da);
	card.nextd(mat->db);

	// TODO: I don't think this works
	m_dyn.addMaterial(mat);

	if (m_ls.NextCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	return true;
}

bool LSDynaFileParser::Read_Mat_Viscoelastic()
{
	LSDynaFile::CARD card;
	m_ls.GetCard(card);
	LSDYNAModel::MAT_VISCOELASTIC* mat;
	mat = new LSDYNAModel::MAT_VISCOELASTIC;
	if (strstr(card.m_szline, "_TITLE") != 0) {
		if (m_ls.NextCard(card) == false) return false;
		strcpy(mat->szname, card.m_szline);
	}
	if (m_ls.NextCard(card) == false) return false;
	card.nexti(mat->mid);
	card.nextd(mat->ro);
	card.nextd(mat->bulk);
	card.nextd(mat->g0);
	card.nextd(mat->gi);
	card.nextd(mat->beta);

	// TODO: I don't think this works
	m_dyn.addMaterial(mat);

	if (m_ls.NextCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	return true;
}

bool LSDynaFileParser::Read_Mat_Kelvin_Maxwell_Viscoelastic()
{
	LSDynaFile::CARD card;
	m_ls.GetCard(card);
	LSDYNAModel::MAT_KELVIN_MAXWELL_VISCOELASTIC* mat;
	mat = new LSDYNAModel::MAT_KELVIN_MAXWELL_VISCOELASTIC;
	if (strstr(card.m_szline, "_TITLE") != 0) {
		if (m_ls.NextCard(card) == false) return false;
		strcpy(mat->szname, card.m_szline);
	}
	if (m_ls.NextCard(card) == false) return false;
	card.nexti(mat->mid);
	card.nextd(mat->ro);
	card.nextd(mat->bulk);
	card.nextd(mat->g0);
	card.nextd(mat->gi);
	card.nextd(mat->dc);
	card.nextd(mat->fo);
	card.nextd(mat->so);

	// TODO: I don't think this works
	m_dyn.addMaterial(mat);

	if (m_ls.NextCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	return true;
}

bool LSDynaFileParser::Read_Mat_Rigid()
{
	LSDynaFile::CARD card;
	m_ls.GetCard(card);
	LSDYNAModel::MAT_RIGID* mat;
	mat = new LSDYNAModel::MAT_RIGID;
	if (strstr(card.m_szline, "_TITLE") != 0) {
		if (m_ls.NextCard(card) == false) return false;
		strcpy(mat->szname, card.m_szline);
	}
	if (m_ls.NextCard(card) == false) return false;
	card.nexti(mat->mid);
	card.nextd(mat->ro);
	card.nextd(mat->e);
	card.nextd(mat->pr);
	card.nextd(mat->n);
	card.nextd(mat->couple);
	card.nextd(mat->m);
	card.nextd(mat->alias);

	if (m_ls.NextCard(card) == false) return false;
	card.nextd(mat->cmo);
	card.nexti(mat->con1);
	card.nexti(mat->con2);

	if (m_ls.NextCard(card) == false) return false;
	card.nextd(mat->a1);
	card.nextd(mat->a2);
	card.nextd(mat->a3);
	card.nextd(mat->v1);
	card.nextd(mat->v2);
	card.nextd(mat->v3);

	// TODO: I don't think this works
	m_dyn.addMaterial(mat);

	if (m_ls.NextCard(card) == false) return false;
	if (!card.IsKeyword()) return false;

	return true;
}

bool LSDynaFileParser::Read_Mat_Elastic_Spring_Discrete_Beam()
{
	LSDynaFile::CARD card;
	m_ls.GetCard(card);
	LSDYNAModel::MATERIAL* mat;
	mat = new LSDYNAModel::MATERIAL;
	if (strstr(card.m_szline, "_TITLE") != 0) {
		if (m_ls.NextCard(card) == false) return false;
		strcpy(mat->szname, card.m_szline);
	}

	if (m_ls.NextCard(card) == false) return false;
	card.nexti(mat->mid);

	// TODO: I don't think this works
	m_dyn.addMaterial(mat);

	while (!card.IsKeyword()) {
		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Mat_Spring()
{
	LSDynaFile::CARD card;
	m_ls.GetCard(card);
	LSDYNAModel::MAT_SPRING_NONLINEAR_ELASTIC* mat = new LSDYNAModel::MAT_SPRING_NONLINEAR_ELASTIC;
	if (strstr(card.m_szline, "_TITLE") != 0) {
		if (m_ls.NextCard(card) == false) return false;
		strcpy(mat->szname, card.m_szline);
	}

	if (m_ls.NextCard(card) == false) return false;
	card.nexti(mat->mid);
	card.nexti(mat->lcd);

	m_dyn.addMaterial(mat);

	while (!card.IsKeyword()) {
		if (m_ls.NextCard(card) == false) return false;
	}

	return true;
}

bool LSDynaFileParser::Read_Set_Segment_Title()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::SET_SEGMENT_TITLE s;
	strcpy(s.m_szname, card.szvalue());
	if (m_ls.NextCard(card) == false) return false;
	card.nexti(s.m_nsid);

	if (m_ls.NextCard(card) == false) return false;
	while (!card.IsKeyword())
	{
		LSDYNAModel::SET_SEGMENT_TITLE::FACE f;
		card.nexti(f.n[0]);
		card.nexti(f.n[1]);
		card.nexti(f.n[2]);
		card.nexti(f.n[3]);
		s.m_face.push_back(f);
		if (m_ls.NextCard(card) == false) return false;
	}

	m_dyn.addSetSegmentTitle(s);

	return true;
}

bool LSDynaFileParser::Read_Set_Node_List_Title()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return false;
	LSDYNAModel::SET_NODE_LIST_TITLE nl;
	nl.m_name = card.szvalue();
	if (m_ls.NextCard(card) == false) return false;
	card.nexti(nl.m_nid);

	if (m_ls.NextCard(card) == false) return false;
	while (!card.IsKeyword())
	{
		int nid;
		while (card.nexti(nid))
		{
			if (nid != 0) nl.m_nodelist.push_back(nid);
		}
		if (m_ls.NextCard(card) == false) return false;
	}

	m_dyn.addNodeList(nl);

	return true;
}

bool LSDynaFileParser::Read_Define_Curve_Title()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");

	LSDYNAModel::LOAD_CURVE lc;
	lc.m_name = card.m_szline;

	if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
	card.nexti(lc.m_lcid);
	card.nexti(lc.m_sidr);
	card.nextf(lc.m_sfa);
	card.nextf(lc.m_sfo);
	card.nextf(lc.m_offa);
	card.nextf(lc.m_offo);
	card.nexti(lc.m_dattyp);

	do
	{
		if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
		if (card.IsKeyword() == false)
		{
			float a, o;
			card.nextf(a, 20);
			card.nextf(o, 20);
			lc.m_pt.push_back(std::pair<float, float>(a, o));
		}

	} while (card.IsKeyword() == false);

	m_dyn.addLoadCurve(lc);

	return true;
}

bool LSDynaFileParser::Read_Define_Curve()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");

	LSDYNAModel::LOAD_CURVE lc;
	card.nexti(lc.m_lcid);
	card.nexti(lc.m_sidr);
	card.nextf(lc.m_sfa);
	card.nextf(lc.m_sfo);
	card.nextf(lc.m_offa);
	card.nextf(lc.m_offo);
	card.nexti(lc.m_dattyp);

	do
	{
		if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
		if (card.IsKeyword() == false)
		{
			float a, o;
			card.nextf(a, 20);
			card.nextf(o, 20);
			lc.m_pt.push_back(std::pair<float, float>(a, o));
		}

	} while (card.IsKeyword() == false);

	m_dyn.addLoadCurve(lc);

	return true;
}

bool LSDynaFileParser::Read_Include()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
	std::string fileName = card.m_szline;

	// we assume that the file name is a relative path to the main file
	// so we'll need to find the file path first
	std::string parentFile = m_ls.FileName();
	size_t pbs = parentFile.rfind('\\');
	size_t pfs = parentFile.rfind('/');
	size_t ps = std::string::npos;
	if      (pfs == std::string::npos) ps = pbs;
	else if (pbs == std::string::npos) ps = pfs;
	else ps = (pfs > pbs ? pfs : pbs);
	if (ps != std::string::npos)
	{
		std::string path = parentFile.substr(0, ps + 1);
		fileName = path + fileName;
	}

	LSDynaFile lsfile;
	if (lsfile.Open(fileName.c_str()) == false) return false;

	LSDynaFileParser lsparse(lsfile, m_dyn);
	bool b = lsparse.ParseFile();

	lsfile.Close();

	// if the parsing failed, make sure to copy the error string
	if (lsparse.GetErrorString())
		Error(lsparse.GetErrorString());

	return b;
}

bool LSDynaFileParser::Read_Parameter()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
	while (card.IsKeyword() == false)
	{
		char ch[12] = { 0 };
		card.nexts(ch, 12);
		double f = 0.0;
		card.nextd(f);

		m_dyn.addParameter(ch+1, f);

		if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
	};

	return true;
}

bool LSDynaFileParser::Read_Parameter_Expression()
{
	LSDynaFile::CARD card;
	if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
	while (card.IsKeyword() == false)
	{
		// TODO: process card!
		if (m_ls.NextCard(card) == false) return Error("Unexpected end of file.");
	};

	return true;
}
