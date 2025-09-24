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
#include "AbaqusExport.h"
using namespace Post;

AbaqusExport::AbaqusExport()
{

}

void AbaqusExport::SetHeading(const std::string& s)
{
	m_heading = s;
}

bool AbaqusExport::Save(FEPostModel &fem, int ntime, const char *szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == nullptr) return false;

	// write header
	fprintf(fp, "*HEADING\n");
	fprintf(fp, "%s\n", m_heading.c_str());

	// write nodes
	fprintf(fp, "*NODE\n");
	FSMesh* mesh = fem.GetFEMesh(0);
	int NN = mesh->Nodes();
	for (int i = 0; i < NN; ++i)
	{
		vec3f r = fem.NodePosition(i, ntime);
		fprintf(fp, "%d, %.7g, %.7g, %.7g\n", i + 1, r.x, r.y, r.z);
	}

	// write elements
	int ndom = mesh->MeshPartitions();
	int ne = 1;
	for (int i = 0; i < ndom; ++i)
	{
		FSMeshPartition& dom = mesh->MeshPartition(i);

		if (dom.Elements() > 0)
		{
			// TODO: Looks like the domain doesn't store its type correctly.
			//       For now, we take the type of the first element.
//			int ntype = dom.Type();
			int ntype = dom.Element(0).Type();

			const char* sztype = "???";
			switch (ntype)
			{
			case FE_TET4  : sztype = "C3D4"; break;
			case FE_HEX8  : sztype = "C3D8"; break;
			case FE_TET10 : sztype = "C3D10"; break;
			case FE_HEX20 : sztype = "C3D20"; break;
            case FE_HEX27 : sztype = "C3D27"; break;
            case FE_PYRA5 : sztype = "C3D5"; break;
			case FE_PENTA6: sztype = "C3D6"; break;
            case FE_PENTA15: sztype = "C3D15"; break;
			default:
				assert(false);
			}
			fprintf(fp, "*ELEMENT, TYPE=%s\n", sztype);

			for (int j = 0; j < dom.Elements(); ++j)
			{
				FSElement_& el = dom.Element(j);

				fprintf(fp, "%d", ne++);

				int nn = el.Nodes();
				for (int k = 0; k < nn; ++k)
				{
					fprintf(fp, ",%d", el.m_node[k] + 1);
				}
				fprintf(fp, "\n");
			}
		}
	}

	fprintf(fp, "*END STEP\n");

	fclose(fp);

	return true;
}
