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
#include "FEFEBioExport.h"
#include <FEBioXML/XMLWriter.h>
#include "FEPostModel.h"
#include <map>
using namespace Post;

// defined in FEBio/FEBioExport.h
template <> std::string type_to_string<vec3f>(const vec3f& v);

const char* elementTypeStr(int ntype)
{
	const char* szeltype = 0;
	switch (ntype)
	{
	case FE_HEX8    : szeltype = "hex8"; break;
	case FE_HEX20   : szeltype = "hex20"; break;
	case FE_HEX27   : szeltype = "hex27"; break;
	case FE_PENTA6  : szeltype = "penta6"; break;
	case FE_PENTA15 : szeltype = "penta15"; break;
	case FE_TET4    : szeltype = "tet4"; break;
	case FE_QUAD4   : szeltype = "quad4"; break;
	case FE_QUAD8   : szeltype = "quad8"; break;
	case FE_QUAD9   : szeltype = "quad9"; break;
	case FE_TRI3    : szeltype = "tri3"; break;
	case FE_TET10   : szeltype = "tet10"; break;
	case FE_TET15   : szeltype = "tet15"; break;
	case FE_TET20   : szeltype = "tet20"; break;
	case FE_TRI6    : szeltype = "tri6"; break;
	case FE_PYRA5   : szeltype = "pyra5"; break;
    case FE_PYRA13  : szeltype = "pyra13"; break;
	default:
		assert(false);
	}

	return szeltype;		
}

bool FEFEBioExport::Save(FEPostModel& fem, const char* szfile)
{
	int nmat = fem.Materials();

	FEState* pst = fem.CurrentState();
	FSMesh* pm = pst->GetFEMesh();
	int NE = pm->Elements();

	XMLWriter xml;
	if (xml.open(szfile) == false) return false;

	XMLElement el;

	el.name("febio_spec");
	el.add_attribute("version", "2.5");
	xml.add_branch(el);
	{
		xml.add_branch("Geometry");
		{
			xml.add_branch("Nodes");
			{
				el.clear();
				el.name("node");
				int n1 = el.add_attribute("id", "");
				for (int i=0; i<pm->Nodes(); ++i)
				{
					vec3f& r = pst->m_NODE[i].m_rt;
					el.set_attribute(n1, i+1);
					el.value(r);
					xml.add_leaf(el, false);
				}
			}
			xml.close_branch(); // Nodes

			// loop over all materials
			int np = 0;
			for (int m=0; m<nmat; ++m)
			{
				for (int i=0; i<NE; ++i) pm->ElementRef(i).m_ntag = 0;

				int i0 = 0;
				while (i0 < NE)
				{
					// find the first element of this part
					for (int i = i0; i<NE; ++i, ++i0)
					{
						FSElement_& elm = pm->ElementRef(i);
						if ((elm.m_ntag == 0) && 
							(elm.m_MatID == m)) break;
					}
					if (i0 >= NE) break;

					// create a part for this material+element type combo
					char sz[256] = { 0 };
					sprintf(sz, "Part%d", np + 1);
					XMLElement part("Elements");
					part.add_attribute("name", sz);

					// get the element type
					FSElement_& el0 = pm->ElementRef(i0);
					const char* szeltype = elementTypeStr(el0.Type());
					if (szeltype == 0) return false;

					part.add_attribute("type", szeltype);

					// now export all the elements of this material and type
					xml.add_branch(part);
					{
						int n[FSElement::MAX_NODES];
						XMLElement el("elem");
						int n1 = el.add_attribute("id", "");
						for (int i=i0; i<pm->Elements(); ++i)
						{
							FSElement_& elm = pm->ElementRef(i);
							if ((elm.m_MatID == m) && (elm.Type() == el0.Type()))
							{
								for (int j=0; j<elm.Nodes(); ++j) n[j] = elm.m_node[j]+1;

								el.set_attribute(n1, i+1);
								el.value(n, elm.Nodes());
								xml.add_leaf(el, false);

								elm.m_ntag = 1;
							}
						}
					}
					xml.close_branch();

					np++;
				}
			}
		}
		xml.close_branch(); // Geometry
	}
	xml.close_branch(); // febio_spec
	xml.close();

	return true;
}

struct DomainInfo
{
	int	elemClass;
	int matId;
	std::string name;
};

bool FEFEBioExport4::Save(FEPostModel& fem, const char* szfile)
{
	int nmat = fem.Materials();

	FEState* pst = fem.CurrentState();
	FSMesh* pm = pst->GetFEMesh();
	int NE = pm->Elements();

	XMLWriter xml;
	if (xml.open(szfile) == false) return false;

	XMLElement el;

	el.name("febio_spec");
	el.add_attribute("version", "4.0");
	xml.add_branch(el);
	{
		std::vector<DomainInfo> domainInfo;
		xml.add_branch("Mesh");
		{
			XMLElement nodes("Nodes");
			nodes.add_attribute("name", "Object1");
			xml.add_branch(nodes);
			{
				el.clear();
				el.name("node");
				int n1 = el.add_attribute("id", "");
				for (int i = 0; i < pm->Nodes(); ++i)
				{
					vec3f& r = pst->m_NODE[i].m_rt;
					el.set_attribute(n1, i + 1);
					el.value(r);
					xml.add_leaf(el, false);
				}
			}
			xml.close_branch(); // Nodes

			// loop over all materials
			int np = 0;
			for (int m = 0; m < nmat; ++m)
			{
				for (int i = 0; i < NE; ++i) pm->ElementRef(i).m_ntag = 0;

				int i0 = 0;
				while (i0 < NE)
				{
					// find the first element of this part
					for (int i = i0; i < NE; ++i, ++i0)
					{
						FSElement_& elm = pm->ElementRef(i);
						if ((elm.m_ntag == 0) &&
							(elm.m_MatID == m)) break;
					}
					if (i0 >= NE) break;

					// create a part for this material+element type combo
					char sz[256] = { 0 };
					sprintf(sz, "Part%d", np + 1);
					XMLElement part("Elements");
					part.add_attribute("name", sz);

					// get the element type
					FSElement_& el0 = pm->ElementRef(i0);
					const char* szeltype = elementTypeStr(el0.Type());
					if (szeltype == 0) return false;

					part.add_attribute("type", szeltype);

					domainInfo.push_back({ el0.Class(), el0.m_MatID, sz });

					// now export all the elements of this material and type
					xml.add_branch(part);
					{
						int n[FSElement::MAX_NODES];
						XMLElement el("elem");
						int n1 = el.add_attribute("id", "");
						for (int i = i0; i < pm->Elements(); ++i)
						{
							FSElement_& elm = pm->ElementRef(i);
							if ((elm.m_MatID == m) && (elm.Type() == el0.Type()))
							{
								for (int j = 0; j < elm.Nodes(); ++j) n[j] = elm.m_node[j] + 1;

								el.set_attribute(n1, i + 1);
								el.value(n, elm.Nodes());
								xml.add_leaf(el, false);

								elm.m_ntag = 1;
							}
						}
					}
					xml.close_branch();

					np++;
				}
			}
		}
		xml.close_branch(); // Mesh

		xml.add_branch("MeshDomains");
		{
			for (DomainInfo& dom : domainInfo)
			{
				if (dom.elemClass == ELEM_SOLID)
				{
					XMLElement xmldom("SolidDomain");
					xmldom.add_attribute("name", dom.name);
					xml.add_empty(xmldom);
				}
				else if (dom.elemClass == ELEM_SHELL)
				{
					XMLElement xmldom("ShellDomain");
					xmldom.add_attribute("name", dom.name);
					xml.add_empty(xmldom);
				}
				else if (dom.elemClass == ELEM_BEAM)
				{
					XMLElement xmldom("BeamDomain");
					xmldom.add_attribute("name", dom.name);
					xml.add_empty(xmldom);
				}
			}
		}
		xml.close_branch(); // MeshDomains
	}
	xml.close_branch(); // febio_spec
	xml.close();

	return true;
}
