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

#include "stdafx.h"
#include "FEBioImport.h"
#include "FEPostModel.h"
#include <MeshLib/FEMesh.h>
using namespace Post;

FEBioFileImport::FEBioFileImport(FEPostModel* fem) : FEFileReader(fem)
{
	m_pm = 0;
	m_nversion = 0;
}

FEBioFileImport::~FEBioFileImport(void)
{
}

//-----------------------------------------------------------------------------
bool FEBioFileImport::ParseVersion(XMLTag& tag)
{
	const char* szv = tag.AttributeValue("version");
	int n1, n2;
	int nr = sscanf(szv, "%d.%d", &n1, &n2);
	if (nr != 2) return false;
	if ((n1 < 1) || (n1 > 0xFF)) return false;
	if ((n2 < 0) || (n2 > 0xFF)) return false;
	m_nversion = (n1 << 8) + n2;

	// we only support version 1.0, 1.1, 1.2, 2.0
	if ((m_nversion != 0x0100) && 
		(m_nversion != 0x0101) &&
		(m_nversion != 0x0102) && 
		(m_nversion != 0x0200)) return false;
	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFileImport::Load(const char* szfile)
{
	m_fem->Clear();
	m_pm = new FSMesh;
	m_fem->AddMesh(m_pm);

    SetFileName(szfile);

	// Open thefile with the XML reader
	XMLReader xml;
	if (xml.Open(szfile) == false) return errf("This is not a valid FEBio input file");

    // Set the file stream
    SetFileStream(xml.GetFileStream());

	// loop over all child tags
	try
	{
		// Find the root element
		XMLTag tag;
		if (m_xml.FindTag("febio_spec", tag) == false) return false;

		// check the version number
		if (ParseVersion(tag) == false) return errf("Invalid version number");

		m_xml.NextTag(tag);
		do
		{
			if      (tag == "Material") ParseMaterialSection(*m_fem, tag);
			else if (tag == "Geometry") 
			{
				if (m_nversion >= 0x0200) ParseGeometrySection2(*m_fem, tag);
				else ParseGeometrySection(*m_fem, tag);
			}
			else m_xml.SkipTag(tag);
	
			// go to the next tag
			m_xml.NextTag(tag);
		}
		while (!tag.isend());
	}
	catch (XMLReader::Error e)
	{
		errf("FATAL ERROR: %s (line %d)\n", e.what(), m_xml.GetCurrentLine());
		return false;
	}
	catch (...)
	{
		errf("FATAL ERROR: unrecoverable error (line %d)\n", m_xml.GetCurrentLine());
		return false;
	}

	// update the mesh
	m_pm->RebuildMesh();
	m_fem->UpdateBoundingBox();

	// we need a single state
	FEState* ps = new FEState(0.f, m_fem, m_fem->GetFEMesh(0));
	m_fem->AddState(ps);

	return true;
}

void FEBioFileImport::ParseMaterialSection(FEPostModel& fem, XMLTag& tag)
{
	Material mat;

	// make sure the section is not empty
	if (tag.isleaf()) return;

	// count the number of materials
	m_nmat = 0;
	m_xml.NextTag(tag);
	do
	{
		m_nmat++;
		m_xml.SkipTag(tag);
		m_xml.NextTag(tag);
	}
	while (!tag.isend());

	// add the materials
	for (int i=0; i<m_nmat; ++i)
	{
		// add a material to the scene
		Material mat;
		m_fem->AddMaterial(mat);
	}
}

void FEBioFileImport::ParseGeometrySection(FEPostModel &fem, XMLTag &tag)
{
	int i, j;

	// make sure the section is not empty
	if (tag.isleaf()) return;

	m_xml.NextTag(tag);
	do
	{
		if (tag == "Nodes")
		{
			// first we need to figure out how many nodes there are
			XMLTag t(tag);
			int nn = 0;
			m_xml.NextTag(t);
			while (!t.isend()) { nn++; m_xml.NextTag(t); }

			// create nodes
			m_pm->Create(nn, 0);

			// read nodal coordinates
			m_xml.NextTag(tag);
			for (i=0; i<nn; ++i)
			{
				FSNode& node = m_pm->Node(i);
				tag.value(node.r);
				m_xml.NextTag(tag);
			}
		}
		else if (tag == "Elements")
		{
			// first we need to figure out how many elements there are
			XMLTag t(tag); m_xml.NextTag(t);
			int elems = 0;
			while (!t.isend()) { elems++; m_xml.NextTag(t); }

			// create elements
			m_pm->Create(0, elems);

			// read element data
			m_xml.NextTag(tag);
			int n[FSElement::MAX_NODES];
			int nbel = 0;
			int nsel = 0;
			for (i=0; i<elems; ++i)
			{
				FEElementType etype;
				FSElement& el = static_cast<FSElement&>(m_pm->ElementRef(i));
				if      (tag == "hex8"   ) { etype = FE_HEX8;    ++nbel; }
				else if (tag == "hex20"  ) { etype = FE_HEX20;   ++nbel; }
				else if (tag == "hex27"  ) { etype = FE_HEX27;   ++nbel; }
				else if (tag == "penta6" ) { etype = FE_PENTA6;  ++nbel; }
                else if (tag == "penta15") { etype = FE_PENTA15; ++nbel; }
                else if (tag == "tet4"   ) { etype = FE_TET4;    ++nbel; }
				else if (tag == "tet10"  ) { etype = FE_TET10;   ++nbel; }
				else if (tag == "tet15"  ) { etype = FE_TET15;   ++nbel; }
				else if (tag == "tet20"  ) { etype = FE_TET20;   ++nbel; }
				else if (tag == "quad4"  ) { etype = FE_QUAD4;   ++nsel; }
                else if (tag == "quad8"  ) { etype = FE_QUAD8;   ++nsel; }
                else if (tag == "quad9"  ) { etype = FE_QUAD9;   ++nsel; }
				else if (tag == "tri3"   ) { etype = FE_TRI3;    ++nsel; }
                else if (tag == "tri6"   ) { etype = FE_TRI6;    ++nsel; }
				else if (tag == "pyra5"  ) { etype = FE_PYRA5;   ++nsel; }
                else if (tag == "pyra13" ) { etype = FE_PYRA13;  ++nsel; }
				else throw XMLReader::InvalidTag(tag);
				el.SetType(etype);

				tag.value(n,el.Nodes());
				for (j=0; j<el.Nodes(); ++j) el.m_node[j] = n[j]-1;
				int nmat = atoi(tag.AttributeValue("mat"))-1;
				el.m_MatID = nmat;

				m_xml.NextTag(tag);
			}
		}
		else m_xml.SkipTag(tag);

		m_xml.NextTag(tag);
	}
	while (!tag.isend());
}

void FEBioFileImport::ParseGeometrySection2(FEPostModel &fem, XMLTag &tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return;

	m_xml.NextTag(tag);
	do
	{
		if (tag == "Nodes")
		{
			// first we need to figure out how many nodes there are
			XMLTag t(tag);
			int nn = 0;
			m_xml.NextTag(t);
			while (!t.isend()) { nn++; m_xml.NextTag(t); }

			// create nodes
			m_pm->Create(nn, 0);

			// read nodal coordinates
			m_xml.NextTag(tag);
			for (int i=0; i<nn; ++i)
			{
				FSNode& node = m_pm->Node(i);
				tag.value(node.r);
				m_xml.NextTag(tag);
			}
		}
		else if (tag == "Elements")
		{
			// get the element type
			const char* sztype = tag.AttributeValue("type");
			FEElementType etype;
			if      (strcmp(sztype, "hex8"   ) == 0) etype = FE_HEX8;
			else if (strcmp(sztype, "tet4"   ) == 0) etype = FE_TET4;
			else if (strcmp(sztype, "penta6" ) == 0) etype = FE_PENTA6;
            else if (strcmp(sztype, "penta15") == 0) etype = FE_PENTA15;
            else if (strcmp(sztype, "quad4"  ) == 0) etype = FE_QUAD4;
			else if (strcmp(sztype, "tri3"   ) == 0) etype = FE_TRI3;
			else if (strcmp(sztype, "hex20"  ) == 0) etype = FE_HEX20;
			else if (strcmp(sztype, "quad8"  ) == 0) etype = FE_QUAD8;
			else if (strcmp(sztype, "tet10"  ) == 0) etype = FE_TET10;
			else if (strcmp(sztype, "tet15"  ) == 0) etype = FE_TET15;
			else if (strcmp(sztype, "tet20"  ) == 0) etype = FE_TET20;
			else if (strcmp(sztype, "hex27"  ) == 0) etype = FE_HEX27;
			else if (strcmp(sztype, "tri6"   ) == 0) etype = FE_TRI6;
			else if (strcmp(sztype, "quad9"  ) == 0) etype = FE_QUAD9;
			else if (strcmp(sztype, "pyra5"  ) == 0) etype = FE_PYRA5;
            else if (strcmp(sztype, "pyra13" ) == 0) etype = FE_PYRA13;
			else throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

			// get the material ID
			int nmat = tag.AttributeValue<int>("mat", -1);
			if (nmat == -1) throw XMLReader::MissingAttribute(tag, "mat");
			nmat--;

			// first we need to figure out how many elementsthere are
			XMLTag t(tag);
			int ne = 0;
			m_xml.NextTag(t);
			while (!t.isend()) { ne++; m_xml.NextTag(t); }

			// create elements
			int NE = m_pm->Elements();
			m_pm->Create(0, NE + ne);

			// read element data
			m_xml.NextTag(tag);
			int n[FSElement::MAX_NODES];
			int nbel = 0;
			int nsel = 0;
			for (int i=0; i<ne; ++i)
			{
				FSElement& el = static_cast<FSElement&>(m_pm->ElementRef(NE + i));
				int nid = tag.AttributeValue<int>("id", 0);
				el.SetID(nid);
				el.SetType(etype);
				
				tag.value(n,el.Nodes());
				for (int j=0; j<el.Nodes(); ++j) el.m_node[j] = n[j]-1;
				el.m_MatID = nmat;

				m_xml.NextTag(tag);
			}
		}
		else m_xml.SkipTag(tag);

		m_xml.NextTag(tag);
	}
	while (!tag.isend());
}
