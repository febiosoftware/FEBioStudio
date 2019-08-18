#include "stdafx.h"
#include "FEBioImport.h"
#include "FEModel.h"
#include "FEMesh.h"
using namespace Post;

FEBioImport::FEBioImport(void) : FEFileReader("FEBio input")
{
	m_pfem = 0;
	m_pm = 0;
	m_nversion = 0;
}

FEBioImport::~FEBioImport(void)
{
}

//-----------------------------------------------------------------------------
bool FEBioImport::ParseVersion(XMLTag& tag)
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
bool FEBioImport::Load(FEModel& fem, const char* szfile)
{
	if (Open(szfile, "rt") == false) return errf("Failed opening FEBio input file.");

	fem.Clear();
	m_pfem = &fem;
	m_pm = new FEMesh;
	fem.AddMesh(m_pm);

	// Attach the XML reader to the stream
	if (m_xml.Attach(m_fp) == false) return false;

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
			if      (tag == "Material") ParseMaterialSection(fem, tag);
			else if (tag == "Geometry") 
			{
				if (m_nversion >= 0x0200) ParseGeometrySection2(fem, tag);
				else ParseGeometrySection(fem, tag);
			}
			else m_xml.SkipTag(tag);
	
			// go to the next tag
			m_xml.NextTag(tag);
		}
		while (!tag.isend());
	}
	catch (XMLReader::XMLSyntaxError)
	{
		errf("FATAL ERROR: Syntax error (line %d)\n", m_xml.GetCurrentLine());
		return false;
	}
	catch (XMLReader::InvalidTag e)
	{
		errf("FATAL ERROR: unrecognized tag \"%s\" (line %d)\n", e.tag.m_sztag, e.tag.m_nstart_line);
		return false;
	}
	catch (XMLReader::InvalidAttributeValue e)
	{
		const char* szt = e.tag.m_sztag;
		const char* sza = e.szatt;
		const char* szv = e.szval;
		int l = e.tag.m_nstart_line;
		errf("FATAL ERROR: unrecognized value \"%s\" for attribute \"%s.%s\" (line %d)\n", szv, szt, sza, l);
		return false;
	}
	catch (XMLReader::MissingAttribute e)
	{
		errf("FATAL ERROR: Missing attribute \"%s\" of tag \"%s\" (line %d)\n", e.szatt, e.tag.m_sztag, e.tag.m_nstart_line);
		return false;
	}
	catch (XMLReader::UnmatchedEndTag e)
	{
		const char* sz = e.tag.m_szroot[e.tag.m_nlevel];
		errf("FATAL ERROR: Unmatched end tag for \"%s\" (line %d)\n", sz, e.tag.m_nstart_line);
		return false;
	}
	catch (...)
	{
		errf("FATAL ERROR: unrecoverable error (line %d)\n", m_xml.GetCurrentLine());
		return false;
	}

	// close the XML file
	Close();

	// update the mesh
	m_pm->Update();
	fem.UpdateBoundingBox();

	// we need a single state
	FEState* ps = new FEState(0.f, &fem, fem.GetFEMesh(0));
	fem.AddState(ps);

	return true;
}

void FEBioImport::ParseMaterialSection(FEModel& fem, XMLTag& tag)
{
	FEMaterial mat;

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
		FEMaterial mat;
		m_pfem->AddMaterial(mat);
	}
}

void FEBioImport::ParseGeometrySection(FEModel &fem, XMLTag &tag)
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
				FENode& node = m_pm->Node(i);
				tag.value(node.m_r0);
				node.m_rt = node.m_r0;
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
			int n[FEGenericElement::MAX_NODES];
			int nbel = 0;
			int nsel = 0;
			for (i=0; i<elems; ++i)
			{
				FEElemType etype;
				FEGenericElement& el = static_cast<FEGenericElement&>(m_pm->Element(i));
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

void FEBioImport::ParseGeometrySection2(FEModel &fem, XMLTag &tag)
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
				FENode& node = m_pm->Node(i);
				tag.value(node.m_r0);
				node.m_rt = node.m_r0;
				m_xml.NextTag(tag);
			}
		}
		else if (tag == "Elements")
		{
			// get the element type
			const char* sztype = tag.AttributeValue("type");
			FEElemType etype;
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
			int n[FEGenericElement::MAX_NODES];
			int nbel = 0;
			int nsel = 0;
			for (int i=0; i<ne; ++i)
			{
				FEGenericElement& el = static_cast<FEGenericElement&>(m_pm->Element(NE + i));
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
