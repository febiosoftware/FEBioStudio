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
#include "AbaqusExport.h"
#include <FEMLib/FSProject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>

AbaqusExport::AbaqusExport(FSProject& prj) : FSFileExport(prj)
{

}

AbaqusExport::~AbaqusExport(void)
{

}

void AbaqusExport::SetHeading(const std::string& s)
{
	m_heading = s;
}

bool AbaqusExport::Write(const char* szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == nullptr) return false;

	// write header
	fprintf(fp, "*HEADING\n");
	fprintf(fp, "%s\n", m_heading.c_str());

	GModel& model = m_prj.GetFSModel().GetModel();

	// write nodes
	int nc = 1;
	fprintf(fp, "*NODE\n");
	for (int obs = 0; obs < model.Objects(); ++obs)
	{
		GObject* po = model.Object(obs);
		Transform T = po->GetTransform();
		FSMesh* mesh = po->GetFEMesh();
		if (mesh == nullptr) { fclose(fp); return errf("Not all objects are meshed."); }
		int NN = mesh->Nodes();
		for (int i = 0; i < NN; ++i)
		{
			FSNode& node = mesh->Node(i);
			vec3d r0 = node.pos();
			vec3d r = T.LocalToGlobal(r0);
			fprintf(fp, "%d, %.7lg, %.7lg, %.7lg\n", nc, r.x, r.y, r.z);
			node.m_ntag = nc++;
		}
	}

	// write elements
	int ne = 1;
	for (int obs = 0; obs < model.Objects(); ++obs)
	{
		GObject* po = model.Object(obs);
		FSMesh* mesh = po->GetFEMesh();
		int NE = mesh->Elements();
		mesh->TagAllElements(-1);
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = mesh->Element(i);

			// find an unprocessed element
			if (el.m_ntag == -1)
			{
				int ntype = el.Type();
				int gid = el.m_gid;
				const char* sztype = "???";
				switch (ntype)
				{
				case FE_TET4: sztype = "C3D4"; break;
				case FE_HEX8: sztype = "C3D8"; break;
				case FE_TET10: sztype = "C3D10"; break;
				case FE_HEX20: sztype = "C3D20"; break;
				case FE_PENTA6: sztype = "C3D6"; break;
                case FE_PYRA5: sztype = "C3D5"; break;
				default:
					assert(false);
				}
				fprintf(fp, "*ELEMENT, TYPE=%s\n", sztype);

				for (int j = i; j < NE; ++j)
				{
					FEElement_& elj = mesh->Element(j);

					if ((elj.m_gid == gid) && (elj.Type() == ntype))
					{
						fprintf(fp, "%d", ne);

						int nn = elj.Nodes();
						for (int k = 0; k < nn; ++k)
						{
							int nid = mesh->Node(elj.m_node[k]).m_ntag;
							fprintf(fp, ",%d", nid);
						}
						fprintf(fp, "\n");

						elj.m_ntag = ne++;
					}
				}
			}
		}
	}

	fprintf(fp, "*END STEP\n");

	fclose(fp);

	return true;
}
