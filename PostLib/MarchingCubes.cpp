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
#include "stdafx.h"
#ifdef WIN32
#include <Windows.h>
#include <gl/GL.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif
#ifdef LINUX
#include <GL/gl.h>
#endif
#include "MarchingCubes.h"
#include "ImageModel.h"
#include <ImageLib/3DImage.h>
#include <ImageLib/3DGradientMap.h>
#include <sstream>
#include <assert.h>
//using namespace std;

using std::stringstream;
using namespace Post;

extern int LUT[256][15];
extern int ET_HEX[12][2];
extern int LUT2D_tri[16][9];
extern int ET2D[4][2];

TriMesh::TriMesh()
{
}

void TriMesh::Clear()
{
	m_Face.clear();
}

void TriMesh::Reserve(size_t nsize)
{
	m_Face.reserve(nsize);
}

void TriMesh::Resize(size_t nsize)
{
	m_Face.resize(nsize);
}

void TriMesh::Merge(TriMesh& tri, int ncount)
{
	if (ncount < 0) ncount = tri.Faces();
	int N = (int)m_Face.size();
	m_Face.resize(m_Face.size() + ncount);
	for (size_t i = 0; i < ncount; ++i)
		m_Face[N + i] = tri.m_Face[i];
}

CMarchingCubes::CMarchingCubes(CImageModel* img) : CGLImageRenderer(img)
{
	static int n = 1;
	stringstream ss;
	ss << "ImageIsosurface" << n++;
	SetName(ss.str());

	AddDoubleParam(0, "isosurface value")->SetFloatRange(0.0, 1.0);
	AddBoolParam(true, "smooth surface");
	AddColorParam(GLColor::White(), "surface color");
	AddBoolParam(true, "close surface");
	AddBoolParam(true, "invert space");
	AddBoolParam(true, "allow clipping");

	m_val = 0.5f;
	m_oldVal = -1.f;
	m_bsmooth = true;
	m_bcloseSurface = true;
	m_binvertSpace = false;
	m_col = GLColor(200, 185, 185);

	UpdateData(false);
}

CMarchingCubes::~CMarchingCubes()
{

}

bool CMarchingCubes::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_val = GetFloatValue(ISO_VALUE);
		m_bsmooth = GetBoolValue(SMOOTH);
		m_col = GetColorValue(COLOR);
		m_bcloseSurface = GetBoolValue(CLOSE_SURFACE);
		m_binvertSpace = GetBoolValue(INVERT_SPACE);
		AllowClipping(GetBoolValue(CLIP));

		CreateSurface();
	}
	else
	{
		SetFloatValue(ISO_VALUE, m_val);
		SetBoolValue(SMOOTH, m_bsmooth);
		SetColorValue(COLOR, m_col);
		SetBoolValue(CLOSE_SURFACE, m_bcloseSurface);
		SetBoolValue(INVERT_SPACE, m_binvertSpace);
		SetBoolValue(CLIP, AllowClipping());
	}

	return false;
}

void CMarchingCubes::SetSmooth(bool b)
{ 
	m_bsmooth = b; 
	m_oldVal = -1.f;
//	Create();
}

void CMarchingCubes::SetInvertSpace(bool b)
{ 
	m_binvertSpace = b; 
	m_oldVal = -1.f;
}

void CMarchingCubes::SetCloseSurface(bool b)
{
	m_bcloseSurface = b;
	m_oldVal = -1.f;
}

void CMarchingCubes::Update()
{
	UpdateData();
	if (m_oldVal == m_val) return;
	Create();
}

void CMarchingCubes::Create()
{
	UpdateData();
	CreateSurface();
}

void CMarchingCubes::CreateSurface()
{
	m_oldVal = m_val;

	m_mesh.Clear();

	CImageModel& im = *GetImageModel();
	CImageSource* src = im.GetImageSource();
	if (src == nullptr) return;
	C3DImage& im3d = *src->Get3DImage();

	BOX b = im.GetBoundingBox();

	int NX = im3d.Width();
	int NY = im3d.Height();
	int NZ = im3d.Depth();
	if ((NX == 1) || (NY == 1) || (NZ == 1)) return;

	float dxi = (b.x1 - b.x0) / (NX - 1);
	float dyi = (b.y1 - b.y0) / (NY - 1);
	float dzi = (b.z1 - b.z0) / (NZ - 1);

	Byte ref = (Byte)(m_val * 255.f);
	float fref = (float)ref;
	m_ref = ref;

	C3DGradientMap grad(im3d, b);

	#pragma omp parallel default(shared)
	{
		const int MAX_FACES = 1000000;
		TriMesh temp;
		temp.Resize(MAX_FACES);
		int nfaces = 0;
		Byte val[8];
		vec3f r[8], g[8];

		#pragma omp for schedule(dynamic, 5)
		for (int k = 0; k < NZ - 1; ++k)
		{
			for (int j = 0; j < NY - 1; ++j)
			{
				for (int i = 0; i < NX - 1; ++i)
				{
					// get the voxel's values
					if (i == 0)
					{
						val[0] = im3d.value(i, j, k);
						val[3] = im3d.value(i, j + 1, k);
						val[4] = im3d.value(i, j, k + 1);
						val[7] = im3d.value(i, j + 1, k + 1);
					}

					val[1] = im3d.value(i + 1, j, k);
					val[2] = im3d.value(i + 1, j + 1, k);
					val[5] = im3d.value(i + 1, j, k + 1);
					val[6] = im3d.value(i + 1, j + 1, k + 1);

					// calculate the case of the voxel
					int ncase = 0;
					if (m_binvertSpace)
					{
						if (val[0] < ref) ncase |= 0x01;
						if (val[1] < ref) ncase |= 0x02;
						if (val[2] < ref) ncase |= 0x04;
						if (val[3] < ref) ncase |= 0x08;
						if (val[4] < ref) ncase |= 0x10;
						if (val[5] < ref) ncase |= 0x20;
						if (val[6] < ref) ncase |= 0x40;
						if (val[7] < ref) ncase |= 0x80;
					}
					else
					{
						if (val[0] > ref) ncase |= 0x01;
						if (val[1] > ref) ncase |= 0x02;
						if (val[2] > ref) ncase |= 0x04;
						if (val[3] > ref) ncase |= 0x08;
						if (val[4] > ref) ncase |= 0x10;
						if (val[5] > ref) ncase |= 0x20;
						if (val[6] > ref) ncase |= 0x40;
						if (val[7] > ref) ncase |= 0x80;
					}

					// cases 0 and 255 don't generate triangles, so don't waste time on these
					if ((ncase != 0) && (ncase != 255))
					{
						// get the corners
						r[0].x = b.x0 + i      *dxi; r[0].y = b.y0 + j      *dyi; r[0].z = b.z0 + k      *dzi;
						r[1].x = b.x0 + (i + 1)*dxi; r[1].y = b.y0 + j      *dyi; r[1].z = b.z0 + k      *dzi;
						r[2].x = b.x0 + (i + 1)*dxi; r[2].y = b.y0 + (j + 1)*dyi; r[2].z = b.z0 + k      *dzi;
						r[3].x = b.x0 + i      *dxi; r[3].y = b.y0 + (j + 1)*dyi; r[3].z = b.z0 + k      *dzi;
						r[4].x = b.x0 + i      *dxi; r[4].y = b.y0 + j      *dyi; r[4].z = b.z0 + (k + 1)*dzi;
						r[5].x = b.x0 + (i + 1)*dxi; r[5].y = b.y0 + j      *dyi; r[5].z = b.z0 + (k + 1)*dzi;
						r[6].x = b.x0 + (i + 1)*dxi; r[6].y = b.y0 + (j + 1)*dyi; r[6].z = b.z0 + (k + 1)*dzi;
						r[7].x = b.x0 + i      *dxi; r[7].y = b.y0 + (j + 1)*dyi; r[7].z = b.z0 + (k + 1)*dzi;

						// calculate gradients
						if (m_bsmooth)
						{
							g[0] = grad.Value(i, j, k);
							g[1] = grad.Value(i + 1, j, k);
							g[2] = grad.Value(i + 1, j + 1, k);
							g[3] = grad.Value(i, j + 1, k);
							g[4] = grad.Value(i, j, k + 1);
							g[5] = grad.Value(i + 1, j, k + 1);
							g[6] = grad.Value(i + 1, j + 1, k + 1);
							g[7] = grad.Value(i, j + 1, k + 1);
						}

						// loop over faces
						int* pf = LUT[ncase];
						for (int l = 0; l < 5; l++)
						{
							if (*pf == -1) break;

							// calculate nodal positions
							TriMesh::TRI& tri = temp.Face(nfaces++);
							for (int m = 0; m < 3; m++)
							{
								int n1 = ET_HEX[pf[m]][0];
								int n2 = ET_HEX[pf[m]][1];

								float w = (fref - (float)val[n1]) / ((float)val[n2] - (float)val[n1]);
								assert((w >= 0.f) && (w <= 1.f));

								tri.m_node[m] = r[n1] * (1.f - w) + r[n2] * w;

								if (m_bsmooth)
								{
									vec3f normal = g[n1] * (1.f - w) + g[n2] * w;
									normal.Normalize();
									tri.m_norm[m] = (m_binvertSpace ? -normal : normal);
								}
							}

							if (m_bsmooth == false)
							{
								vec3f normal = (tri.m_node[1] - tri.m_node[0]) ^ (tri.m_node[2] - tri.m_node[0]);
								normal.Normalize();
								tri.m_norm[0] = normal;
								tri.m_norm[1] = normal;
								tri.m_norm[2] = normal;
							}

							pf += 3;

							if (nfaces == MAX_FACES)
							{
								#pragma omp critical
								m_mesh.Merge(temp, nfaces);
								nfaces = 0;
							}
						}
					}

					// keep this for next i
					val[0] = val[1];
					val[4] = val[5];
					val[3] = val[2];
					val[7] = val[6];
				}
			}
		}

		#pragma omp critical
		m_mesh.Merge(temp, nfaces);
	}

	// create surface meshes
	if (m_bcloseSurface)
	{
		Byte val[4];
		vec3f r[4];

		// X-planes
		for (int i = 0; i <= NX - 1; i += NX - 1)
		{
			vec3f faceNormal(1.f, 0.f, 0.f);

			float x = (i == 0 ? b.x0 : b.x1);

			for (int k = 0; k < NZ - 1; k++)
			{
				for (int j = 0; j < NY - 1; ++j)
				{
					// get the pixel's values
					val[0] = im3d.value(i, j, k);
					val[1] = im3d.value(i, j + 1, k);
					val[2] = im3d.value(i, j + 1, k + 1);
					val[3] = im3d.value(i, j, k + 1);

					// get the corners
					r[0].x = x; r[0].y = b.y0 + j      *dyi; r[0].z = b.z0 + k*dzi;
					r[1].x = x; r[1].y = b.y0 + (j + 1)*dyi; r[1].z = b.z0 + k*dzi;
					r[2].x = x; r[2].y = b.y0 + (j + 1)*dyi; r[2].z = b.z0 + (k + 1)*dzi;
					r[3].x = x; r[3].y = b.y0 + j      *dyi; r[3].z = b.z0 + (k + 1)*dzi;

					// add the triangles
					AddSurfaceTris(val, r, faceNormal);
				}
			}
		}

		// Y-planes
		for (int j = 0; j <= NY - 1; j += NY - 1)
		{
			vec3f faceNormal(0.f, -1.f, 0.f);

			float y = (j == 0 ? b.y0 : b.y1);

			for (int k = 0; k < NZ - 1; k++)
			{
				for (int i = 0; i < NX - 1; ++i)
				{
					// get the pixel's values
					val[0] = im3d.value(i  , j, k);
					val[1] = im3d.value(i+1, j, k);
					val[2] = im3d.value(i+1, j, k + 1);
					val[3] = im3d.value(i  , j, k + 1);

					// get the corners
					r[0].x = b.x0 + i    *dxi; r[0].y = y; r[0].z = b.z0 + k*dzi;
					r[1].x = b.x0 + (i+1)*dxi; r[1].y = y; r[1].z = b.z0 + k*dzi;
					r[2].x = b.x0 + (i+1)*dxi; r[2].y = y; r[2].z = b.z0 + (k + 1)*dzi;
					r[3].x = b.x0 + i    *dxi; r[3].y = y; r[3].z = b.z0 + (k + 1)*dzi;

					// add the triangles
					AddSurfaceTris(val, r, faceNormal);
				}
			}
		}

		// Z-planes
		for (int k = 0; k <= NZ - 1; k += NZ - 1)
		{
			vec3f faceNormal(0.f, 0.f, 1.f);

			float z = (k == 0 ? b.z0 : b.z1);

			for (int j = 0; j < NY - 1; ++j)
			{
				for (int i = 0; i < NX - 1; ++i)
				{
					// get the pixel's values
					val[0] = im3d.value(i    , j    , k);
					val[1] = im3d.value(i + 1, j    , k);
					val[2] = im3d.value(i + 1, j + 1, k);
					val[3] = im3d.value(i    , j + 1, k);

					// get the corners
					r[0].x = b.x0 + i      *dxi; r[0].y = b.y0 + j      *dyi; r[0].z = z;
					r[1].x = b.x0 + (i + 1)*dxi; r[1].y = b.y0 + j      *dyi; r[1].z = z;
					r[2].x = b.x0 + (i + 1)*dxi; r[2].y = b.y0 + (j + 1)*dyi; r[2].z = z;
					r[3].x = b.x0 + i      *dxi; r[3].y = b.y0 + (j + 1)*dyi; r[3].z = z;

					// add the triangles
					AddSurfaceTris(val, r, faceNormal);
				}
			}
		}
	}
}

void CMarchingCubes::AddSurfaceTris(Byte val[4], vec3f r[4], const vec3f& faceNormal)
{
	// calculate the case of the voxel
	int ncase = 0;
	if (m_binvertSpace)
	{
		if (val[0] < m_ref) ncase |= 0x01;
		if (val[1] < m_ref) ncase |= 0x02;
		if (val[2] < m_ref) ncase |= 0x04;
		if (val[3] < m_ref) ncase |= 0x08;
	}
	else
	{
		if (val[0] > m_ref) ncase |= 0x01;
		if (val[1] > m_ref) ncase |= 0x02;
		if (val[2] > m_ref) ncase |= 0x04;
		if (val[3] > m_ref) ncase |= 0x08;
	}

	float fref = (float)m_ref;

	// loop over faces
	int* pf = LUT2D_tri[ncase];
	for (int l = 0; l < 3; l++)
	{
		if (*pf == -1) break;

		// calculate nodal positions
		TriMesh::TRI tri;
		for (int m = 0; m < 3; m++)
		{
			int node = pf[m];
			if (node < 4)
			{
				tri.m_node[m] = r[node];
			}
			else
			{
				int n1 = ET2D[node - 4][0];
				int n2 = ET2D[node - 4][1];

				float w = (fref - (float)val[n1]) / ((float)val[n2] - (float)val[n1]);
				tri.m_node[m] = r[n1] * (1.f - w) + r[n2] * w;
			}

			tri.m_norm[m] = faceNormal;
		}

		m_mesh.AddFace(tri);

		pf += 3;
	}
}

void CMarchingCubes::SetIsoValue(float v)
{
	m_val = v;
}

void CMarchingCubes::Render(CGLContext& rc)
{
	glColor3ub(m_col.r, m_col.g, m_col.b);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < m_mesh.Faces(); ++i)
	{
		TriMesh::TRI& face = m_mesh.Face(i);
		glNormal3f(face.m_norm[0].x, face.m_norm[0].y, face.m_norm[0].z);
		glVertex3f(face.m_node[0].x, face.m_node[0].y, face.m_node[0].z);
		glNormal3f(face.m_norm[1].x, face.m_norm[1].y, face.m_norm[1].z);
		glVertex3f(face.m_node[1].x, face.m_node[1].y, face.m_node[1].z);
		glNormal3f(face.m_norm[2].x, face.m_norm[2].y, face.m_norm[2].z);
		glVertex3f(face.m_node[2].x, face.m_node[2].y, face.m_node[2].z);
	}
	glEnd();
}
