#pragma once
#include "GLPlot.h"
#include <MathLib/Transform.h>
#include <vector>

namespace Post {

	class FEState;

class CGLPlaneCutPlot : public CGLPlot  
{
	enum { SHOW_PLANE, CUT_HIDDEN, SHOW_MESH, TRANSPARENCY, NORMAL_X, NORMAL_Y, NORMAL_Z, OFFSET };

	class GLSlice
	{
	public:
		struct FACE
		{
			int		mat;
			vec3d	norm;
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
	CGLPlaneCutPlot(CGLModel* po);
	virtual ~CGLPlaneCutPlot();

	void SetTransform(Transform& T) { m_T = T; }

	void SetRotation(float rot) { m_rot = rot; }
	float GetRotation() { return m_rot; }

	void GetNormalizedEquations(double a[4]);
	vec3d GetPlaneNormal();
	float GetPlaneOffset();

	void Render(CGLContext& rc) override;
	void RenderPlane();
	float Integrate(FEState* ps);

	static void InitClipPlanes();
	static void DisableClipPlanes();
	static void ClearClipPlanes();
	static void EnableClipPlanes();

	void Activate(bool bact) override;

	void Update(int ntime, float dt, bool breset) override;

	void UpdateData(bool bsave = true) override;

protected:
	void RenderSlice();
	void RenderMesh();
	void RenderOutline();
	vec3d WorldToPlane(const vec3d& r);

	void ReleasePlane();
	static int GetFreePlane();
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
};
}
