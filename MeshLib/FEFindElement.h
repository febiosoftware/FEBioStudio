#pragma once
#include <FSCore/box.h>

class FECoreMesh;

class FEFindElement
{
public:
	class OCTREE_BOX
	{
	public:
		BOX					m_box;
		vector<OCTREE_BOX*>	m_child;
		int					m_elem;
		int					m_level;

	public:
		OCTREE_BOX();
		~OCTREE_BOX();

		void split(int levels);

		OCTREE_BOX* Find(const vec3f& r);

		bool IsInside(const vec3f& r) const { return m_box.IsInside(r); }

		void Add(BOX& b, int nelem);
	};

public:
	FEFindElement(FECoreMesh& mesh);

	void Init(int nframe = 0);
	void Init(vector<bool>& flags, int nframe = 0);

	bool FindElement(const vec3f& x, int& nelem, double r[3]);

	BOX BoundingBox() const { return m_bound.m_box; }

private:
	void InitReferenceFrame(vector<bool>& flags);
	void InitCurrentFrame(vector<bool>& flags);

	bool FindInReferenceFrame(const vec3f& x, int& nelem, double r[3]);
	bool FindInCurrentFrame(const vec3f& x, int& nelem, double r[3]);

private:
	OCTREE_BOX* FindBox(const vec3f& r);

private:
	OCTREE_BOX	m_bound;
	FECoreMesh&	m_mesh;
	int			m_nframe;	// = 0 reference, 1 = current
};

inline bool FEFindElement::FindElement(const vec3f& x, int& nelem, double r[3])
{
	return (m_nframe == 0 ? FindInReferenceFrame(x, nelem, r) : FindInCurrentFrame(x, nelem, r));
}
