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
#include "VRMLExporter.h"
#include "FEPostModel.h"
using namespace Post;
//-----------------------------------------------------------------------------

VRMLExporter::VRMLExporter(void)
{
	m_fp = 0;
	m_sztab[0] = 0;
}

VRMLExporter::~VRMLExporter(void)
{
}

void VRMLExporter::inctab()
{
	strcat(m_sztab, "\t");
}

void VRMLExporter::dectab()
{
	int l = (int)strlen(m_sztab);
	if (l > 0) m_sztab[l-1] = 0;
}

bool VRMLExporter::Save(FEPostModel* pscene, const char* szfile)
{
	// open the file
	m_fp = fopen(szfile, "wt");
	if (m_fp == 0) return false;

	// keep a pointer to the scene
	m_pscene = pscene;

	// write the header
	fprintf(m_fp, "#VRML V2.0 utf8\n\n");

	// write the group node
	WriteNode("Group");

	// write the children
	Write("children [\n");
	inctab();

	// write the mesh
	write_mesh();

	// write the different states
	write_states();

	// write the time sensor
	write_timer();

	// close children
	dectab();
	Write("]\n");

	CloseNode(); // Group

	// write event routing
	if (m_pscene->GetStates())
	{
		Write("ROUTE ts.fraction_changed TO ci.set_fraction\n");
		Write("ROUTE ci.value_changed TO co.set_point\n");
	}

	// close the file
	fclose(m_fp);
	m_fp = 0;

	return true;
}

void VRMLExporter::write_timer()
{
	if (m_pscene->GetStates() <= 1) return;

	WriteNode("DEF ts TimeSensor");

	Write("cycleInterval 2\n");
	Write("loop TRUE\n");

	CloseNode(); // TimeSensor
}

void VRMLExporter::write_mesh()
{
	int i, j, n;

	// write the shape group
	WriteNode("Shape");

	// write the appearance field
	Write("appearance Appearance { material Material {} }\n");

	// write the geometry field
	WriteNode("geometry IndexedFaceSet");


	// get the mesh
	FSMesh& mesh = *m_pscene->GetFEMesh(0);

	// we first need to count how many nodes we have that will be exported
	for (i=0; i<mesh.Nodes(); ++i) 
	{
		FSNode& node = mesh.Node(i);
		node.m_ntag = 0;
	}
	for (i=0; i<mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j=0; j<n; ++j) 
		{
			mesh.Node(face.n[j]).m_ntag = 1;
		}
	}
	n = 0;
	for (i=0; i<mesh.Nodes(); ++i)
	{
		FSNode& node = mesh.Node(i);
		node.m_ntag = (node.m_ntag? n++ : -1);
	}
	int nodes = n-1;

	// write faces
	Write("coordIndex [\n"); inctab();

	char szline[256];

	int* fn;
	for (i=0; i<mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		fn = face.n;
		int nf[4];
		nf[0] = mesh.Node(fn[0]).m_ntag;
		nf[1] = mesh.Node(fn[1]).m_ntag;
		nf[2] = mesh.Node(fn[2]).m_ntag;
		nf[3] = mesh.Node(fn[3]).m_ntag;
		switch (n)
		{
		case 3: sprintf(szline, "%d,%d,%d,-1"   , nf[0], nf[1], nf[2]);  break;
		case 4: sprintf(szline, "%d,%d,%d,%d,-1", nf[0], nf[1], nf[2], nf[3]);  break;
		default: assert(false);
		}
		Write(szline);
		if (i < mesh.Faces()-1) fprintf(m_fp,",\n"); else fprintf(m_fp, "\n");
	}

	dectab();
	Write("]\n");

	// write nodes
	WriteNode("coord DEF co Coordinate");

	Write("point [\n");
	inctab();

	for (i=0, n=0; i<mesh.Nodes(); ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.m_ntag >= 0)
		{
			sprintf(szline, "%lg %lg %lg", node.r.x, node.r.y, node.r.z);
			Write(szline);
			if (n < nodes-1) fprintf(m_fp, ",\n"); else fprintf(m_fp, "\n");
			++n;
		}
	}

	dectab();
	Write("]\n");

	CloseNode(); // Coordinate

	Write("creaseAngle 1\n");

	WriteNode("color Color");

	Write("color [\n"); inctab();

	float r, g, b;
	float f = 1.f / 255.f;

	for (i=0; i<mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int matid = mesh.ElementRef(face.m_elem[0].eid).m_MatID;
		GLColor& col = m_pscene->GetMaterial(matid)->diffuse;
		r = (float) col.r *f;
		g = (float) col.g *f;
		b = (float) col.b *f;
		sprintf(szline, "%g %g %g", r, g, b);
		if (i==mesh.Faces()-1) strcat(szline, "\n"); else strcat(szline, ",\n");
		Write(szline);
	}

	dectab();
	Write("]\n");

	CloseNode(); // Color

	Write("colorPerVertex FALSE\n");

	CloseNode(); // geometry

	CloseNode(); // Shape
}

void VRMLExporter::write_states()
{
	int i, j;

	// get the mesh
	FSMesh& mesh = *m_pscene->GetFEMesh(0);

	// get the nr of states
	int ntime = m_pscene->GetStates();

	if (ntime == 1) return;

	// get the last time value
	float time0 = m_pscene->GetState(0      )->m_time;
	float time1 = m_pscene->GetState(ntime-1)->m_time;
	float dt = (time1 - time0);
	if (dt == 0.f) dt = 1.f;

	WriteNode("DEF ci CoordinateInterpolator");

	// write the key
	Write("key [\n"); inctab();
	char szline[256];
	for (i=0; i<ntime; ++i) 
	{
		sprintf(szline, "%g\n", (m_pscene->GetState(i)->m_time - time0) / dt);
		Write(szline);
	}
	dectab();
	Write("]\n");

	// write the key-value
	Write("keyValue [\n"); inctab();

	int N = mesh.Nodes();

	for (i=0; i<ntime; ++i)
	{
		FEState& s = *m_pscene->GetState(i);

		for (j=0; j<N; ++j)
		{
			if (mesh.Node(j).m_ntag >= 0)
			{
				vec3f& r = s.m_NODE[j].m_rt;
				sprintf(szline, "%g %g %g", r.x, r.y, r.z);
				if ((i==ntime-1) && (j==N-1)) strcat(szline, "\n"); else strcat(szline, ",\n");
				Write(szline);
			}
		}
	}

	dectab();
	Write("]\n");

	CloseNode(); // CoordinateInterpolator
}

void VRMLExporter::WriteNode(const char* szname)
{
	fprintf(m_fp, "%s%s {\n", m_sztab, szname);
	inctab();
}

void VRMLExporter::CloseNode()
{
	dectab();
	fprintf(m_fp, "%s}\n", m_sztab);
}

void VRMLExporter::Write(const char* sz)
{
	fprintf(m_fp, "%s%s", m_sztab, sz);
}
