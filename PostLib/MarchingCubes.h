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
#include "GLImageRenderer.h"
#include <vector>
#include <FSCore/math3d.h>
#include <FSCore/color.h>
#include <GLLib/GLMesh.h>

class FSMesh;
class CImageModel;
class C3DImage;

namespace Post {

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
	enum { ISO_VALUE, SMOOTH, CLOSE_SURFACE, INVERT_SPACE, CLIP, COLOR, SPECULAR_COLOR, SHININESS };

public:
	CMarchingCubes(CImageModel* img);
	virtual ~CMarchingCubes();

	float GetIsoValue() const { return (float) m_val; }
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

	void Render(GLRenderEngine& re, GLContext& rc) override;

	void Update() override;

	bool UpdateData(bool bsave = true) override;

	bool GetMesh(FSMesh& mesh);

private:
	void AddSurfaceTris(TriMesh& mesh, uint8_t val[4], vec3f r[4], const vec3f& faceNormal);

	void CreateSurface();

	void ProcessImage();

    template<class pType>
    void Create8BitImage();

private:
	double	m_val, m_oldVal;		// iso-surface value
	bool	m_bsmooth;
	bool	m_bcloseSurface;
	bool	m_binvertSpace;
	GLColor	m_col;
	GLColor	m_spc;
	double	m_shininess;
	uint8_t m_ref;

	GLMesh*	m_mesh = nullptr;

    C3DImage* m_8bitImage;
    bool m_del8BitImage;
};
}
