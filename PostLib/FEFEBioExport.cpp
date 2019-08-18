#include "stdafx.h"
#include "FEFEBioExport.h"
#include "XMLWriter.h"
using namespace Post;

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
	default:
		assert(false);
	}

	return szeltype;		
}

bool FEFEBioExport::Save(FEModel& fem, const char* szfile)
{
	int nmat = fem.Materials();

	FEMeshBase* pm = fem.GetFEMesh(0);
	FEState* pst = fem.GetActiveState();
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
				for (int i=0; i<NE; ++i) pm->Element(i).m_ntag = 0;

				int i0 = 0;
				while (i0 < NE)
				{
					// find the first element of this part
					for (int i = i0; i<NE; ++i, ++i0)
					{
						FEElement& elm = pm->Element(i);
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
					FEElement& el0 = pm->Element(i0);
					const char* szeltype = elementTypeStr(el0.Type());
					if (szeltype == 0) return false;

					part.add_attribute("type", szeltype);

					// now export all the elements of this material and type
					xml.add_branch(part);
					{
						int n[FEGenericElement::MAX_NODES];
						XMLElement el("elem");
						int n1 = el.add_attribute("id", "");
						for (int i=i0; i<pm->Elements(); ++i)
						{
							FEElement& elm = pm->Element(i);
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
