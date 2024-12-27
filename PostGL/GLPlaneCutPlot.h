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

#pragma once
#include "GLPlot.h"
#include <FECore/FETransform.h>
#include <GLLib/GLMesh.h>
#include <vector>

namespace Post {

	class FEState;

class CGLPlaneCutPlot : public CGLPlot  
{
	enum { SHOW_PLANE, CUT_HIDDEN, SHOW_MESH, MESH_COLOR, TRANSPARENCY, NORMAL_X, NORMAL_Y, NORMAL_Z, OFFSET };

	class GLSlice
	{
	public:
		struct FACE
		{
			int		mat;
			vec3d	r[3];
			float	tex[3];
			bool	bactive;
		};

		struct EDGE
		{
			vec3d r[2];
		};

	public:
		GLSlice(){}

		int Faces() const { return (int) m_Face.size(); }
		FACE& Face(int i) { return m_Face[i]; }

		void AddFace(FACE& f) { m_Face.push_back(f); }

		int Edges() const { return (int) m_Edge.size(); }
		EDGE& Edge(int i) { return m_Edge[i]; }
		void AddEdge(EDGE& e) { m_Edge.push_back(e); }

		void Clear() { m_Face.clear(); m_Edge.clear(); }

	private:
		std::vector<FACE>	m_Face;
		std::vector<EDGE>	m_Edge;
	};

public:
	CGLPlaneCutPlot();
	virtual ~CGLPlaneCutPlot();

	void SetTransform(Transform& T) { m_T = T; }

	void SetRotation(float rot) { m_rot = rot; }
	float GetRotation() { return m_rot; }

	void GetNormalizedEquations(double a[4]);
	vec3d GetPlaneNormal() const;
	void SetPlaneNormal(const vec3d& n);
	float GetPlaneOffset();
	void SetPlaneOffset(float a);
	float GetOffsetScale() const;
	vec3d GetPlanePosition() const;

	void Render(GLRenderEngine& re, CGLContext& rc) override;
	void RenderPlane();
	float Integrate(FEState* ps);

	static void InitClipPlanes();
	static void DisableClipPlanes();
	static void ClearClipPlanes();
	static void EnableClipPlanes();

	void Activate(bool bact) override;

	void Update(int ntime, float dt, bool breset) override;

	bool UpdateData(bool bsave = true) override;

	void UpdatePlaneCut();

public:
	bool Intersects(Ray& ray, Intersection& q) override;
	FESelection* SelectComponent(int index) override;
	void ClearSelection() override;

protected:
	void RenderSlice();
	void RenderMesh();
	void RenderOutline();
	vec3d WorldToPlane(const vec3d& r);

	void ReleasePlane();
	static int GetFreePlane();

	void AddDomain(FEPostMesh* pm, int n);
	void AddFaces(FEPostMesh* pm);

	void UpdateTriMesh();
	void UpdateLineMesh();
	void UpdateOutlineMesh();
	void UpdateSlice();

public:
	static int ClipPlanes();
	static CGLPlaneCutPlot* GetClipPlane(int i);
	static bool IsInsideClipRegion(const vec3d& r);

public:
	bool	m_bshowplane;	// show the plane or not
	bool	m_bcut_hidden;	// cut hidden materials
	bool	m_bshow_mesh;
	float	m_transparency;
	GLColor	m_meshColor;

protected:
	vec3d		m_normal;	// plane normal (not normalized yet!)
	double		m_offset;	// plane offset in normal direction
	double		m_scl;		// scale factor for offset

	float	m_rot;	// rotation around z-axis

	Transform	m_T;	// local transformation

	struct EDGE
	{
		vec3d	m_r[2];	// position of nodes
		int		m_n[2];	// node numbers
		int		m_ntag;
	};

	GLSlice	m_slice;

	int		m_nclip;								// clip plane number
	static	std::vector<int>				m_clip;	// avaialabe clip planes
	static	std::vector<CGLPlaneCutPlot*>	m_pcp;

	GLTriMesh	m_activeMesh;	// for rendering active faces (i.e. that need texture)
	GLTriMesh	m_inactiveMesh;	// for rendering inactive faces (i.e. that use material color)
	GLLineMesh	m_lineMesh;	// for rendering mesh lines
	GLLineMesh	m_outlineMesh;	// for rendering the outline

	bool	m_bupdateSlice; // update slice before rendering
};
}
