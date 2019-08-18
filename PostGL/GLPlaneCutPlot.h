#pragma once
#include "GLPlot.h"
#include <vector>

namespace Post {

class CGLPlaneCutPlot : public CGLPlot  
{
	class GLSlice
	{
	public:
		struct FACE
		{
			int		mat;
			vec3f	norm;
			vec3f	r[3];
			float	tex[3];
			bool	bactive;
		};

		struct EDGE
		{
			vec3f r[2];
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

	void SetBoundingBox(BOUNDINGBOX box) { m_box = box; }

	void SetRotation(float rot) { m_rot = rot; }
	float GetRotation() { return m_rot; }

	void GetPlaneEqn(GLdouble* eqn)
	{
		eqn[0] = m_eq[0];
		eqn[1] = m_eq[1];
		eqn[2] = m_eq[2];
		eqn[3] = m_eq[3];
	}

	void SetPlaneEqn(GLdouble a[4]);

	void GetNormalizedEquations(double a[4]);
	vec3f GetPlaneNormal();
	float GetPlaneOffset();
	float GetPlaneReference() const { return m_ref; }

	void Render(CGLContext& rc);
	void RenderPlane();
	float Integrate(FEState* ps);

	static void InitClipPlanes();
	static void DisableClipPlanes();
	static void EnableClipPlanes();

	void Activate(bool bact);

	CPropertyList* propertyList();

	void Update(int ntime, float dt, bool breset);

protected:
	void RenderSlice();
	void RenderMesh();
	void RenderOutline();
	vec3f WorldToPlane(vec3f r);

	void ReleasePlane();
	static int GetFreePlane();
	void UpdateSlice();

public:
	static int ClipPlanes();
	static CGLPlaneCutPlot* GetClipPlane(int i);
	static bool IsInsideClipRegion(const vec3f& r);

public:
	bool	m_bshowplane;	// show the plane or not
	bool	m_bcut_hidden;	// cut hidden materials
	bool	m_bshow_mesh;
	float	m_transparency;

protected:
	GLdouble	m_eq[4];	// plane equation

	float		m_ref;	// reference = m_pos + m_off
	float		m_rot;	// rotation around z-axis
	BOUNDINGBOX	m_box;	// bounding box to cut

	struct EDGE
	{
		vec3f	m_r[2];	// position of nodes
		int		m_n[2];	// node numbers
		int		m_ntag;
	};

	GLSlice	m_slice;

	int		m_nclip;								// clip plane number
	static	std::vector<int>				m_clip;	// avaialabe clip planes
	static	std::vector<CGLPlaneCutPlot*>	m_pcp;
};
}
