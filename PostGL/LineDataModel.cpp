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
#include "LineDataModel.h"
#include <MeshLib/FECurveMesh.h>
#include <MeshLib/FEFindElement.h>
#include <PostLib/FEPostModel.h>
using namespace std;
using namespace Post;

//-----------------------------------------------------------------------------
class OctreeBox
{
public:
	OctreeBox(BOX box, int levels) : m_box(box), m_level(levels)
	{
		if (levels == 0)
		{
			for (int i = 0; i < 8; ++i) m_child[i] = nullptr;
			return;
		}

		double R = box.Radius();

		double x0 = box.x0, x1 = box.x1;
		double y0 = box.y0, y1 = box.y1;
		double z0 = box.z0, z1 = box.z1;
		int n = 0;
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
				{
					double xa = x0 + i * (x1 - x0) * 0.5;
					double ya = y0 + j * (y1 - y0) * 0.5;
					double za = z0 + k * (z1 - z0) * 0.5;
					double xb = x0 + (i + 1.0) * (x1 - x0) * 0.5;
					double yb = y0 + (j + 1.0) * (y1 - y0) * 0.5;
					double zb = z0 + (k + 1.0) * (z1 - z0) * 0.5;
					BOX boxi(xa, ya, za, xb, yb, zb);
					boxi.Inflate(R * 1e-7);
					m_child[n++] = new OctreeBox(boxi, levels - 1);
				}
	}
	~OctreeBox() { for (int i = 0; i < 8; ++i) delete m_child[i]; }

	int addNode(std::vector<vec3d>& points, const vec3d& r)
	{
		if (m_box.IsInside(r) == false) return -1;

		if (m_level == 0)
		{
			const double eps = 1e-12;
			for (int i = 0; i < m_nodes.size(); ++i)
			{
				vec3d& ri = points[m_nodes[i]];
				if ((ri - r).SqrLength() <= eps)
				{
					// node is already in list
					return m_nodes[i];
				}
			}

			// if we get here, the node is in this box, 
			// but not in the points array yet, so add it
			points.push_back(r);
			m_nodes.push_back((int)points.size() - 1);
			return (int)points.size() - 1;
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				int n = m_child[i]->addNode(points, r);
				if (n >= 0) return n;
			}
			return -1;
		}
	}

private:
	int			m_level;
	BOX			m_box;
	OctreeBox* m_child[8];
	std::vector<int>	m_nodes;
};

class Segment
{
public:
	struct LINE
	{
		int	m_index;	// index into mesh' edge array
		int	m_ln[2];	// local node indices in segment's node array
		int m_orient;	// 1 or -1, depending on the orientation in the segment
	};

public:
	Segment(FECurveMesh* mesh) : m_mesh(mesh) {}
	Segment(const Segment& s) { m_mesh = s.m_mesh; m_pt = s.m_pt; m_seg = s.m_seg; }
	void operator = (const Segment& s) { m_mesh = s.m_mesh; m_pt = s.m_pt; m_seg = s.m_seg; }

	void Add(int n)
	{
		LINE l;
		l.m_index = n;
		l.m_orient = 0;
		l.m_ln[0] = l.m_ln[1] = -1;
		m_seg.push_back(l);
	}

	int Points() const { return (int) m_pt.size(); }
	vec3d& Point(int n) { return m_mesh->Node(m_pt[n]).r; }

	void Build()
	{
		m_pt.clear();
		for (int i = 0; i < m_seg.size(); ++i)
		{
			FSEdge& edge = m_mesh->Edge(m_seg[i].m_index);
			m_seg[i].m_ln[0] = addPoint(edge.n[0]); assert(m_seg[i].m_ln[0] >= 0);
			m_seg[i].m_ln[1] = addPoint(edge.n[1]); assert(m_seg[i].m_ln[1] >= 0);
		}
		assert((m_pt.size() == m_seg.size() + 1) || (m_pt.size() == m_seg.size()));

		Sort();
	}

	int LineSegments() const { return (int) m_seg.size(); }
	LINE& GetLineSegment(int n) { return m_seg[n]; }

private:
	int addPoint(int n)
	{
		for (int j = 0; j < m_pt.size(); ++j)
		{
			if (m_pt[j] == n) return j;
		}
		m_pt.push_back(n);
		return (int)m_pt.size() - 1;
	}

	void Sort()
	{
		size_t pts = m_pt.size();
		vector<int> tag(pts, 0);
		for (int i = 0; i < m_seg.size(); ++i)
		{
			tag[m_seg[i].m_ln[0]]++;
			tag[m_seg[i].m_ln[1]]++;
		}

		// find one end
		vector<int> NLT; NLT.reserve(pts);
		for (int i = 0; i < pts; ++i)
		{
			assert((tag[i] == 1) || (tag[i] == 2));
			if (tag[i] == 1)
			{
				NLT.push_back(i);
				break;
			}
		}

		size_t nltsize = pts;
		if (NLT.empty())
		{
			// this is probably a loop, so just pick the first point
			NLT.push_back(0);
			nltsize++;
		}
		assert(NLT.size() == 1);

		size_t nsegs = m_seg.size();
		int seg0 = 0;
		while (NLT.size() != nltsize)
		{
			int n0 = NLT.back();

			// find a segment with this node
			for (int i = seg0; i < nsegs; ++i)
			{
				int m0 = m_seg[i].m_ln[0];
				int m1 = m_seg[i].m_ln[1];
				if ((m0 == n0) || (m1 == n0))
				{
					if (m0 == n0)
					{
						NLT.push_back(m1);
						m_seg[i].m_orient = 1;
					}
					else
					{
						NLT.push_back(m0);
						m_seg[i].m_orient = -1;
					}
					swapSegments(i, seg0);
					seg0++;
					break;
				}
			}
		}

		// reorder nodes
		vector<int> NLTi(pts);
		vector<int> newpt(pts);
		for (int i = 0; i < pts; ++i)
		{
			int ni = NLT[i];
			NLTi[ni] = i;
			newpt[i] = m_pt[ni];
		}
		m_pt = newpt;

		// reorder segments
		for (int i = 0; i < m_seg.size(); ++i)
		{
			LINE& li = m_seg[i];
			li.m_ln[0] = NLTi[li.m_ln[0]];
			li.m_ln[1] = NLTi[li.m_ln[1]];
		}
	}

	void swapSegments(int a, int b)
	{
		if (a == b) return;
		LINE tmp = m_seg[a];
		m_seg[a] = m_seg[b];
		m_seg[b] = tmp;
	}

private:
	FECurveMesh* m_mesh;
	vector<LINE>	m_seg;
	vector<int>		m_pt;
};

void LineData::processLines()
{
	int lines = Lines();
	if (lines == 0) return;

	// find the bounding box
	BOX box;
	for (int i = 0; i < lines; ++i)
	{
		Post::LINESEGMENT& line = Line(i);
		box += to_vec3d(line.m_r0);
		box += to_vec3d(line.m_r1);
	}

	// inflate a little
	double R = box.GetMaxExtent(); if (R == 0.0) R = 1.0;
	box.Inflate(R * 1e-5);

	// start adding points
	int l = (int)(log(lines) / log(8.0));
	if (l < 3) l = 3;
	if (l > 8) l = 8;
	OctreeBox ocb(box, l);
	std::vector<pair<int, int> > ptIndex; ptIndex.resize(lines, pair<int, int>(-1, -1));
	std::vector<vec3d> points; points.reserve(lines * 2);
	for (int i = 0; i < lines; ++i)
	{
		Post::LINESEGMENT& line = Line(i);
		ptIndex[i].first = ocb.addNode(points, to_vec3d(line.m_r0)); assert(ptIndex[i].first >= 0);
		ptIndex[i].second = ocb.addNode(points, to_vec3d(line.m_r1)); assert(ptIndex[i].second >= 0);
		assert(ptIndex[i].first != ptIndex[i].second);
	}

	// now build a curve mesh
	// (this is used to find the segments, i.e. the connected lines)
	FECurveMesh mesh;
	mesh.Create((int)points.size(), lines);
	for (int i = 0; i < points.size(); ++i)
	{
		FSNode& node = mesh.Node(i);
		node.r = points[i];
	}
	for (int i = 0; i < lines; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		edge.n[0] = ptIndex[i].first;
		edge.n[1] = ptIndex[i].second;
	}
	mesh.BuildMesh();

	// figure out the segments
	int nsegs = mesh.Segments();
	if (nsegs == 0) return;

	std::vector<Segment> segment(nsegs, Segment(&mesh));
	for (int i = 0; i < lines; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		segment[edge.m_gid].Add(i);
		m_Line[i].m_segId = edge.m_gid;
	}

	// process each segment
	for (int n = 0; n < nsegs; ++n)
	{
		Segment& segn = segment[n];
		segn.Build();

		int pts = segn.Points();
		vector<vec3d> t(pts, vec3d(0, 0, 0));
		for (int i = 0; i < pts - 1; ++i)
		{
			vec3d a = segn.Point(i);
			vec3d b = segn.Point(i + 1);

			vec3d ti = b - a;
			t[i] += ti;
			t[i + 1] += ti;
		}

		// normalize tangents
		for (int i = 0; i < pts; ++i) t[i].Normalize();

		// assign the tangents
		int nseg = segn.LineSegments();
		for (int i = 0; i < nseg; ++i)
		{
			Segment::LINE& seg = segn.GetLineSegment(i);

			Post::LINESEGMENT& line = Line(seg.m_index);

			if (i == 0)
			{
				if (seg.m_orient > 0) line.m_end[0] = 1;
				if (seg.m_orient < 0) line.m_end[1] = 1;
			}
			else if (i == nseg - 1)
			{
				if (seg.m_orient > 0) line.m_end[1] = 1;
				if (seg.m_orient < 0) line.m_end[0] = 1;
			}

			line.m_t0 = t[seg.m_ln[0]];
			line.m_t1 = t[seg.m_ln[1]];

			assert(seg.m_orient != 0);
			if (seg.m_orient < 0)
			{
				line.m_t0 = -line.m_t0;
				line.m_t1 = -line.m_t1;
			}
		}
	}
}

LineDataSource::LineDataSource(LineDataModel* mdl) : m_mdl(mdl)
{
	if (mdl) mdl->SetLineDataSource(this);
}

LineDataModel::LineDataModel(FEPostModel* fem) : m_fem(fem)
{
	m_src = nullptr;
	int ns = fem->GetStates();
	if (ns != 0) m_line.resize(ns);
}

LineDataModel::~LineDataModel()
{
	delete m_src;
}

void LineDataModel::Clear()
{
	m_line.clear();
	int ns = m_fem->GetStates();
	if (ns != 0) m_line.resize(ns);
}

//=======================================================================================
Ang2LineDataSource::Ang2LineDataSource(Post::LineDataModel* mdl) : Post::LineDataSource(mdl)
{
	SetTypeString("ang2");

	AddStringParam("", "fileName");
}

bool Ang2LineDataSource::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_fileName = GetStringValue(MP_FILENAME);
	}
	else
	{
		SetStringValue(MP_FILENAME, m_fileName);
	}

	return false; // only return true of the parameterization was modified!
}

bool Ang2LineDataSource::Load(const char* szfile)
{
	m_fileName = szfile;
	UpdateData(false);

	bool bsuccess = Reload();
	if (bsuccess == false) m_fileName.clear();

	return bsuccess;
}

bool Ang2LineDataSource::Reload()
{
	if (m_fileName.empty()) return false;
	const char* szfile = m_fileName.c_str();

	Post::LineDataModel* ldm = GetLineDataModel();
	ldm->Clear();
	bool bsuccess = false;
	const char* szext = strrchr(szfile, '.');
	if (szext && (strcmp(szext, ".ang2") == 0))
	{
		// Read AngioFE2 format
		int nret = ReadAng2Format(szfile, *ldm);
		bsuccess = (nret != 0);
		if (nret == 2)
		{
			//				QMessageBox::warning(0, "FEBio Studio", "End-of-file reached before all states were processed.");
		}
	}
	else
	{
		// read old format (this assumes this is a text file)
		bsuccess = ReadOldFormat(szfile, *ldm);
	}

	if (bsuccess)
	{
		// process the line data
		int states = ldm->States();
#pragma omp parallel for schedule(dynamic)
		for (int nstate = 0; nstate < states; ++nstate)
		{
			Post::LineData& lineData = ldm->GetLineData(nstate);
			lineData.processLines();
		}
	}

	return bsuccess;
}

bool Ang2LineDataSource::ReadOldFormat(const char* szfile, Post::LineDataModel& lineData)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	char szline[256] = { 0 };
	while (!feof(fp))
	{
		if (fgets(szline, 255, fp))
		{
			int nstate;
			float x0, y0, z0, x1, y1, z1;
			int n = sscanf(szline, "%d%*g%g%g%g%g%g%g", &nstate, &x0, &y0, &z0, &x1, &y1, &z1);
			if (n == 7)
			{
				if ((nstate >= 0) && (nstate < lineData.States()))
				{
					Post::LineData& lines = lineData.GetLineData(nstate);
					lines.AddLine(vec3f(x0, y0, z0), vec3f(x1, y1, z1));
				}
			}
		}
	}

	fclose(fp);

	return true;
}

// helper structure for finding position of vessel fragments
struct FRAG
{
	int		iel;	// element in which tip lies
	double	r[3];	// iso-coords of tip
	vec3f	r0;		// reference position of tip
	double	user_data;
};

vec3f GetCoordinatesFromFrag(Post::FEPostModel& fem, int nstate, FRAG& a)
{
	FSMesh& mesh = *fem.GetFEMesh(0);
	vec3f x[FSElement::MAX_NODES];

	vec3f r0 = a.r0;
	if (a.iel >= 0)
	{
		FEElement_& el = mesh.ElementRef(a.iel);
		for (int i = 0; i < el.Nodes(); ++i) x[i] = fem.NodePosition(el.m_node[i], nstate);
		r0 = el.eval(x, a.r[0], a.r[1], a.r[2]);
	}

	return r0;
}

// return code:
// 0 = failed
// 1 = success
// 2 = EOF reached before all states were read in
int Ang2LineDataSource::ReadAng2Format(const char* szfile, Post::LineDataModel& lineData)
{
	FILE* fp = fopen(szfile, "rb");
	if (fp == 0) return 0;

	Post::FEPostModel& fem = *lineData.GetFEModel();
	FSMesh& mesh = *fem.GetFEMesh(0);

	// read the magic number
	unsigned int magic = 0;
	if (fread(&magic, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; };
	if (magic != 0xfdb97531) { fclose(fp); return 0; }

	// read the version number
	unsigned int version = 0;
	if (fread(&version, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }

	// the flags say if vessels can grow inside a material or not
	int mats = fem.Materials();
	vector<bool> flags(mats, true);

	// number of user-defined data fields in line file.
	int ndataFields = 0;

	switch (version)
	{
	case 0: break;	// nothing to do (all materials are candidates)
	case 1:
	{
		// read masks
		int n = 0;
		unsigned int masks = 0;
		if (fread(&masks, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }
		for (unsigned int i = 0; i < masks; ++i)
		{
			unsigned int mask = 0;
			if (fread(&mask, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); return 0; }
			for (int j = 0; j < 32; ++j)
			{
				bool b = ((mask & (1 << j)) != 0);
				flags[n++] = b;
				if (n == mats) break;
			}
			if (n == mats) break;
		}
	}
	break;
	default:
		fclose(fp); return 0;
	}

	// store the raw data
	vector<pair<FRAG, FRAG> > raw;

	// we need to make sure that the mesh' coordinates
	// are the initial coordinates
	const int N = mesh.Nodes();
	vector<vec3d> tmp(N);

	// copy the initial positions to this mesh
	for (int i = 0; i < N; ++i)
	{
		tmp[i] = mesh.Node(i).r;
		mesh.Node(i).r = to_vec3d(fem.NodePosition(i, 0));
	}

	// build search tool
	FEFindElement find(mesh);
	find.Init(flags);

	int nret = 1;

	int nstate = 0;
	while (!feof(fp) && !ferror(fp))
	{
		if (nstate >= fem.GetStates()) break;

		Post::LineData& lines = lineData.GetLineData(nstate);

		// this file format only stores incremental changes to the network
		// so we need to copy all the data from the previous state as well
		if (nstate > 0)
		{
			// copy line data
			for (int i = 0; i < raw.size(); ++i)
			{
				FRAG& a = raw[i].first;
				FRAG& b = raw[i].second;
				vec3f r0 = GetCoordinatesFromFrag(fem, nstate, a);
				vec3f r1 = GetCoordinatesFromFrag(fem, nstate, b);

				// add the line
				lines.AddLine(r0, r1, (float) a.user_data, (float) b.user_data, a.iel, b.iel);
			}
		}

		// read number of segments 
		unsigned int segs = 0;
		if (fread(&segs, sizeof(unsigned int), 1, fp) != 1) { fclose(fp); nret = 2; break; }

		// read time stamp (is not used right now)
		float ftime = 0.0f;
		if (fread(&ftime, sizeof(float), 1, fp) != 1) { fclose(fp); nret = 2; break; }

		// read the segments
		int nd = 6 + 2 * ndataFields;
		vector<float> d(nd, 0.f);
		for (unsigned int i = 0; i < segs; ++i)
		{
			if (fread(&d[0], sizeof(float), nd, fp) != nd) { fclose(fp); nret = 2; break; }

			// store the raw coordinates
			float* c = &d[0];
			vec3f a0 = vec3f(c[0], c[1], c[2]); c += 3 + ndataFields;
			vec3f b0 = vec3f(c[0], c[1], c[2]);

			float va = ftime, vb = ftime;
			if (ndataFields > 0)
			{
				va = d[3];
				vb = d[6 + ndataFields];
			}

			FRAG a, b;
			a.user_data = va;
			b.user_data = vb;
			if (find.FindElement(a0, a.iel, a.r) == false) a.iel = -1;
			if (find.FindElement(b0, b.iel, b.r) == false) b.iel = -1;
			raw.push_back(pair<FRAG, FRAG>(a, b));

			// convert them to global coordinates
			vec3f r0 = GetCoordinatesFromFrag(fem, nstate, a);
			vec3f r1 = GetCoordinatesFromFrag(fem, nstate, b);

			// add the line data
			lines.AddLine(r0, r1, va, vb, a.iel, b.iel);
		}
		if (nret != 1) break;

		// next state
		nstate++;
	}
	fclose(fp);

	// restore mesh' nodal positions
	for (int i = 0; i < N; ++i) mesh.Node(i).r = tmp[i];

	// all done
	return nret;
}
