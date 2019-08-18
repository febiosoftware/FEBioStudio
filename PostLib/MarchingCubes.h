#pragma once
#include "GLImageRenderer.h"
#include <vector>
#include "math3d.h"
#include "color.h"

namespace Post {

class CImageModel;

class TriMesh
{
public:
	struct TRI
	{
		vec3f	m_node[3];
		vec3f	m_norm[3];
	};

public:
	TriMesh();

	void Clear();

	void Merge(TriMesh& tri, int ncount = -1);

	void Reserve(size_t nsize);

	void Resize(size_t nsize);

	TRI& Face(int i) { return m_Face[i]; }
	int Faces() const { return (int)m_Face.size(); }

	void AddFace(TRI& tri) { m_Face.push_back(tri); }

protected:
	std::vector<TRI>	m_Face;
};

class CMarchingCubes : public CGLImageRenderer
{
public:
	CMarchingCubes(CImageModel* img);
	virtual ~CMarchingCubes();

	float GetIsoValue() const { return m_val; }
	void SetIsoValue(float v);

	bool GetSmooth() const { return m_bsmooth; }
	void SetSmooth(bool b);

	GLColor GetColor() const { return m_col; }
	void SetColor(GLColor c) { m_col = c; }

	bool GetInvertSpace() const { return m_binvertSpace; }
	void SetInvertSpace(bool b);

	bool GetCloseSurface() const { return m_bcloseSurface; }
	void SetCloseSurface(bool b);

	void Create();

	void Render(CGLContext& rc) override;

	void Update() override;

	CPropertyList* propertyList() override;

private:
	void AddSurfaceTris(byte val[4], vec3f r[4], const vec3f& faceNormal);

private:
	float	m_val, m_oldVal;		// iso-surface value
	bool	m_bsmooth;
	bool	m_bcloseSurface;
	bool	m_binvertSpace;
	GLColor	m_col;
	TriMesh	m_mesh;

	byte m_ref;
};
}
