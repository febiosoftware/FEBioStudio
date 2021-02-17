/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "FELSDYNAexport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>
#include <memory>

using std::unique_ptr;

//-----------------------------------------------------------------------------
FELSDYNAexport::FELSDYNAexport(FEProject& prj) : FEFileExport(prj)
{
	m_fp = 0;

	// set default options
	m_ops.bselonly = false;
	m_ops.bshellthick = false;
}

//-----------------------------------------------------------------------------
FELSDYNAexport::~FELSDYNAexport(void)
{
}

//-----------------------------------------------------------------------------
bool FELSDYNAexport::Write(const char *szfile)
{
	m_fp = fopen(szfile, "wt");
	if (m_fp == 0) return errf("Failed creating LSDYNA file %s", szfile);

	FEModel* ps = &m_prj.GetFEModel();
	GModel& model = ps->GetModel();

	// reset part counter
	m_npart = 1;

	// write the header
	fprintf(m_fp, "*KEYWORD\n");

	// write the NODE section
	if (!write_NODE()) { fclose(m_fp); return false; }

	// write the ELEMENT_SOLID section
	int nsol = model.SolidElements();
	if (nsol)
	{
		if (!write_ELEMENT_SOLID()) { fclose(m_fp); return false; }
	}

	// write the shell section
	int nshl = model.ShellElements();
	if (nshl)
	{
		bool bret = false;
		if (m_ops.bshellthick) bret = write_ELEMENT_SHELL_THICKNESS();
		else bret = write_ELEMENT_SHELL();
		if (bret == false) { fclose(m_fp); return false; }
	}

	// write the parts
	if (!write_SET_SHELL_LIST()) { fclose(m_fp); return false; }

	// write the end tag
	fprintf(m_fp, "*END\n");

	fclose(m_fp);

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAexport::write_NODE()
{
	FEModel* ps = &(m_prj.GetFEModel());
	GModel& model = ps->GetModel();

	fprintf(m_fp, "*NODE\n");
	int n = 1;
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (!m_ops.bselonly || (po->IsSelected()))
		{
			FEMesh* pm = po->GetFEMesh();
			if (pm == 0) return false;
			for (int j=0; j<pm->Nodes(); ++j)
			{
				vec3d r = pm->Node(j).r;
				r = po->GetTransform().LocalToGlobal(r);

				fprintf(m_fp,"%8d%16.7e%16.7e%16.7e\n", n, r.x, r.y, r.z);
				pm->Node(j).m_ntag = n++;
			}
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAexport::write_ELEMENT_SOLID()
{
	int nn[8];
	int n = 1;

	FEModel* ps = &(m_prj.GetFEModel());
	GModel& model = ps->GetModel();

	fprintf(m_fp, "*ELEMENT_SOLID\n");
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (!m_ops.bselonly || (po->IsSelected()))
		{
			FEMesh& m = *po->GetFEMesh();
			for (int j=0; j<m.Elements(); ++j)
			{
				FEElement& el = m.Element(j);
				for (int k=0; k<el.Nodes(); ++k) nn[k] = m.Node(el.m_node[k]).m_ntag;
				el.m_ntag = n;

				int npart = m_npart + el.m_gid;

				switch (el.Type())
				{
				case FE_HEX8:
					fprintf(m_fp, "%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d\n", n, npart, nn[0], nn[1], nn[2], nn[3], nn[4], nn[5], nn[6], nn[7]);
					break;
				case FE_PENTA6:
					fprintf(m_fp, "%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d\n", n, npart, nn[0], nn[2], nn[5], nn[3], nn[1], nn[1], nn[4], nn[4]);
					break;
				case FE_TET4:
					fprintf(m_fp, "%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d\n", n, npart, nn[0], nn[1], nn[2], nn[3], nn[3], nn[3], nn[3], nn[3]);
					break;
				}
				++n;
			}
			m_npart += po->Parts();
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAexport::write_ELEMENT_SHELL()
{
	int nn[8];
	int n = 1;

	FEModel* ps = &(m_prj.GetFEModel());
	GModel& model = ps->GetModel();
		
	fprintf(m_fp, "*ELEMENT_SHELL\n");

	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (!m_ops.bselonly || (po->IsSelected()))
		{
			FEMesh& m = *po->GetFEMesh();
			for (int j=0; j<m.Elements(); ++j)
			{
				FEElement& el = m.Element(j);
				el.m_ntag = n;

				for (int k=0; k<el.Nodes(); ++k) nn[k] = m.Node(el.m_node[k]).m_ntag;

				switch (el.Type())
				{
				case FE_TRI3:
					fprintf(m_fp, "%8d%8d%8d%8d%8d%8d\n", n, m_npart, nn[0], nn[1], nn[2], nn[2]);
					break;
				case FE_QUAD4:
					fprintf(m_fp, "%8d%8d%8d%8d%8d%8d\n", n, m_npart, nn[0], nn[1], nn[2], nn[3]);
					break;
				}
				++n;
			}
		}
		++m_npart;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAexport::write_ELEMENT_SHELL_THICKNESS()
{
	int nn[8];
	int n = 1;

	FEModel* ps = &(m_prj.GetFEModel());
	GModel& model = ps->GetModel();
		
	fprintf(m_fp, "*ELEMENT_SHELL_THICKNESS\n");

	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (!m_ops.bselonly || (po->IsSelected()))
		{
			FEMesh& m = *po->GetFEMesh();
			for (int j=0; j<m.Elements(); ++j)
			{
				FEElement& el = m.Element(j);
				el.m_ntag = n;

				for (int k=0; k<el.Nodes(); ++k) nn[k] = m.Node(el.m_node[k]).m_ntag;
				double* h = el.m_h;

				switch (el.Type())
				{
				case FE_TRI3:
					fprintf(m_fp, "%8d%8d%8d%8d%8d%8d\n", n, m_npart, nn[0], nn[1], nn[2], nn[2]);
					fprintf(m_fp, "%15.10lg %15.10lg %15.10lg %15.10lg\n", h[0], h[1], h[2], h[2]);
					break;
				case FE_QUAD4:
					fprintf(m_fp, "%8d%8d%8d%8d%8d%8d\n", n, m_npart, nn[0], nn[1], nn[2], nn[3]);
					fprintf(m_fp, "%15.10lg %15.10lg %15.10lg %15.10lg\n", h[0], h[1], h[2], h[3]);
					break;
				}
				++n;
			}
		}
		++m_npart;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAexport::write_SET_SHELL_LIST()
{
	FEModel* ps = &(m_prj.GetFEModel());
	GModel& model = ps->GetModel();

	// export the parts
	int n = 1;
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		if (!m_ops.bselonly || po->IsSelected())
		{
			FEMesh* pm = po->GetFEMesh();
			assert(pm);

			for (int j=0; j<po->FEParts(); ++j)
			{
				// write the keyword
				fprintf(m_fp, "*SET_SHELL_LIST\n");
				fprintf(m_fp, "%10d%15.10lg%15.10lg%15.10lg%15.10lg\n", n++, 0., 0., 0., 0.);

				FEPart* pg = po->GetFEPart(j);
				unique_ptr<FEElemList> pl(pg->BuildElemList());
				int N = pl->Size();
				FEElemList::Iterator pi = pl->First();
				int ne[8], nc = 0;
				for (int k=0; k<N; ++k, ++pi)
				{
					FEElement_* pe = pi->m_pi;
					switch (pe->Type())
					{
					case FE_TRI3 : ne[nc++] = pe->m_ntag; break;
					case FE_QUAD4: ne[nc++] = pe->m_ntag; break;
						break;
					}

					if ((nc == 8) || (k==N-1))
					{
						for (int l=0; l<nc; ++l) fprintf(m_fp,"%10d", ne[l]);
						fprintf(m_fp, "\n");
						nc = 0;
					}
				}
			}
		}
	}

	return true;
}
