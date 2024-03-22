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
#include "FEAsciiExport.h"
#include "FEPostModel.h"
#include <stdio.h>
using namespace Post;

void print_format(char* szfmt, int id, float val, FILE* fp)
{
	if (szfmt[0] == 0)
	{
		fprintf(fp, "%8d, %15.7g\n", id, val);
	}
	else
	{
		char* sz = szfmt;
		do
		{
			char* ch = strchr(sz, '%');
			if (ch)
			{
				if (ch[1]=='i')
				{
					*ch = 0;
					fprintf(fp, "%s", sz);
					*ch = '%'; sz = ch+2;
					fprintf(fp, "%d", id);
				}
				else if (ch[1]=='g')
				{
					*ch = 0;
					fprintf(fp, "%s", sz);
					*ch = '%'; sz = ch+2;
					fprintf(fp, "%lg", val);
				}
				else if (ch[1]=='t')
				{
					*ch = 0;
					fprintf(fp, "%s", sz);
					*ch = '%'; sz = ch+2;
					fprintf(fp, "\t");
				}
				else if (ch[1]=='n')
				{
					*ch = 0;
					fprintf(fp, "%s", sz);
					*ch = '%'; sz = ch+2;
					fprintf(fp, "\n");
				}
				else
				{
					*ch = 0;
					fprintf(fp, "%s", sz);
					*ch = '%'; sz = ch+1;
				}
			}
			else { fprintf(fp, "%s", sz); break; }
		}
		while (*sz);
		fprintf(fp, "\n");
	}
}

FEASCIIExport::FEASCIIExport()
{
	m_bcoords = false;
	m_belem = false;
	m_bface = false;
	m_bfnormals = false;
	m_bndata = false;
	m_bedata = false;
	m_bselonly = false;
	m_alltimes = 0;

	AddBoolParam(m_bcoords  , "coords"      , "Nodal coordinates");
	AddBoolParam(m_bface    , "faces"       , "Facet connectivity");
	AddBoolParam(m_bfnormals, "face_normals", "Facet normals");
	AddBoolParam(m_belem    , "elements"    , "Element connectivity");
	AddBoolParam(m_bndata   , "node_data"   , "Nodal data values");
	AddBoolParam(m_bedata   , "elem_data"   , "Element data values");
	AddBoolParam(m_bselonly , "sel_only"    , "Selection only");
	AddChoiceParam(m_alltimes, "all_steps", "Time steps")->SetEnumNames("Current time step\0All time steps\0");
	AddStringParam("", "format", "Format");
}

bool FEASCIIExport::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_bcoords   = GetBoolValue(0);
		m_bface     = GetBoolValue(1);
		m_bfnormals = GetBoolValue(2);
		m_belem     = GetBoolValue(3);
		m_bndata    = GetBoolValue(4);
		m_bedata    = GetBoolValue(5);
		m_bselonly  = GetBoolValue(6);
		m_alltimes  = GetIntValue(7);
		m_fmt       = GetStringValue(8);
	}
	else
	{
		SetBoolValue(0, m_bcoords);
		SetBoolValue(1, m_bface);
		SetBoolValue(2, m_bfnormals);
		SetBoolValue(3, m_belem);
		SetBoolValue(4, m_bndata);
		SetBoolValue(5, m_bedata);
		SetBoolValue(6, m_bselonly);
		SetIntValue (7, m_alltimes);
		SetStringValue(8, m_fmt);
	}

	return false;
}

bool FEASCIIExport::Save(FEPostModel* pfem, int n0, int n1, const char* szfile)
{
	FEPostMesh& m = *pfem->GetFEMesh(0);

	int NN = m.Nodes();
	int NE = m.Elements();

	// open the file
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	// tag all selected items
	for (int i = 0; i<NN; ++i)
	{
		FSNode& n = m.Node(i);
		n.m_ntag = 1;
		if (m_bselonly && !n.IsSelected()) n.m_ntag = 0;
	}

	// tag all selected elements
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& el = m.ElementRef(i);
		if ((m_bselonly == false) || el.IsSelected()) el.m_ntag = 1;
		else el.m_ntag = 0;
	}

	fprintf(fp, "*ASCII EXPORT\n");

	// export initial nodal coordinates
	if (m_bcoords)
	{
		fprintf(fp, "*NODES\n");
		for (int i = 0; i<NN; ++i)
		{
			if (m.Node(i).m_ntag == 1)
			{
				vec3f r = to_vec3f(m.Node(i).r);
				fprintf(fp, "%8d,%15.7lg,%15.7lg,%15.7lg\n", i + 1, r.x, r.y, r.z);
			}
		}
	}

	// export facet connectivity
	if (m_bface)
	{
		fprintf(fp, "*FACES\n");
		for (int i=0; i<m.Faces(); ++i)
		{
			FSFace& f = m.Face(i);
			if (f.Nodes() == 3) fprintf(fp, "%8d,%d,%d,%d\n"   , i + 1, f.n[0], f.n[1], f.n[2]);
			if (f.Nodes() == 4) fprintf(fp, "%8d,%d,%d,%d,%d\n", i + 1, f.n[0], f.n[1], f.n[2], f.n[3]);
		}
	}

	// export facet normals
	if (m_bfnormals)
	{
		fprintf(fp, "*FACE_NORMALS\n");
		for (int i = 0; i<m.Faces(); ++i)
		{
			vec3f& r = m.Face(i).m_fn;
			fprintf(fp, "%8d,%15.7lg,%15.7lg,%15.7lg\n", i + 1, r.x, r.y, r.z);
		}
	}

	// export element connectivity
	if (m_belem)
	{
		fprintf(fp, "*ELEMENTS\n");
		int i, n[FSElement::MAX_NODES];
		for (i = 0; i<NE; ++i)
		{
			FEElement_& e = m.ElementRef(i);
			if (e.m_ntag == 1)
			{
				int ne = e.Nodes();
				for (int j = 0; j < ne; ++j) n[j] = e.m_node[j] + 1;

				switch (e.Type())
				{
				case FE_HEX8:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7]);
					break;
				case FE_PYRA5:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4]);
					break;
				case FE_HEX20:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11], n[12], n[13], n[14], n[15], n[16], n[17], n[18], n[19]);
					break;
				case FE_HEX27:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11], n[12], n[13], n[14], n[15], n[16], n[17], n[18], n[19], n[20], n[21], n[22], n[23], n[24], n[25], n[26]);
					break;
				case FE_PENTA6:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5]);
					break;
				case FE_TET4:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3]);
					break;
				case FE_TET10:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9]);
					break;
				case FE_TET15:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11], n[12], n[13], n[14]);
					break;
				case FE_PENTA15:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11], n[12], n[13], n[14]);
					break;
				case FE_PYRA13:
					fprintf(fp, "%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d,%8d\n", i + 1, n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11], n[12]);
					break;
				}
			}
		}
	}

	// check input
	if (m_bndata || m_bedata)
	{
		if (n1 < n0) n1 = n0;
		for (int ntime = n0; ntime <= n1; ++ntime)
		{
			// get the next state
			FEState* ps = pfem->GetState(ntime);

			// get the time value
			float ftime = ps->m_time;

			fprintf(fp, "*STATE %d\n", ntime + 1);
			fprintf(fp, "*TIME_VALUE %g\n", ftime);
			//	fprintf(fp, "*FIELD_STRING %s\n", GetFieldString());

			// export nodal coordinates
			if (m_bcoords)
			{
				fprintf(fp, "*NODES\n");
				for (int i = 0; i<NN; ++i)
				{
					if (m.Node(i).m_ntag == 1)
					{
						vec3f& r = ps->m_NODE[i].m_rt;
						fprintf(fp, "%8d,%15.7lg,%15.7lg,%15.7lg\n", i + 1, r.x, r.y, r.z);
					}
				}
			}

			// export nodal values
			if (m_bndata)
			{
				fprintf(fp, "*NODAL_DATA\n");
				for (int i = 0; i<NN; ++i)
				{
					if (m.Node(i).m_ntag == 1)
					{
						float& d = ps->m_NODE[i].m_val;
						fprintf(fp, "%8d,%15.7g\n", i + 1, d);
					}
				}
			}

			// export element values
			if (m_bedata)
			{
				size_t n = m_fmt.size();
				char* szfmt = new char[n + 1];
				strcpy(szfmt, m_fmt.c_str());
				szfmt[n] = 0;
				fprintf(fp, "*ELEMENT_DATA\n");
				for (int i = 0; i<NE; ++i)
				{
					FEElement_& e = m.ElementRef(i);
					if (e.m_ntag == 1)
					{
						float& d = ps->m_ELEM[i].m_val;
						int id = m.ElementRef(i).m_nid;
						print_format(szfmt, id, d, fp);
					}
				}
				delete[] szfmt;
			}
		}
	}

	fprintf(fp, "*END\n");

	fclose(fp);

	return true;
}
