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
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include <ImageLib/3DGradientMap.h>
#include <MeshLib/FEMesh.h>
#include <sstream>
#include <algorithm>
#include <assert.h>
//using namespace std;

using std::stringstream;
using namespace Post;

extern int LUT[256][15];
extern int ET_HEX[12][2];
extern int LUT2D_quad[16][9];
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
	AddBoolParam(true, "close surface");
	AddBoolParam(true, "invert space");
	AddBoolParam(true, "allow clipping");
	AddColorParam(GLColor::White(), "surface color");
	AddColorParam(GLColor::White(), "specular color");
	AddDoubleParam(0, "shininess")->SetFloatRange(0.0, 1.0);

	m_val = 0.5;
	m_oldVal = -1.0;
	m_bsmooth = true;
	m_bcloseSurface = true;
	m_binvertSpace = false;
	m_col = GLColor(200, 185, 185);
	m_spc = GLColor(85, 85, 85);
	m_shininess = 0.25;

    m_del8BitImage = true;
    switch (GetImageModel()->Get3DImage()->PixelType())
    {
    case CImage::UINT_8:
        m_8bitImage = GetImageModel()->Get3DImage();
        m_del8BitImage = false;
        break;
    case CImage::INT_8:
        Create8BitImage<int8_t>();
        break;
    case CImage::UINT_16:
        Create8BitImage<uint16_t>();
        break;
    case CImage::INT_16:
        Create8BitImage<int16_t>();
        break;
    case CImage::UINT_32:
        Create8BitImage<uint32_t>();
        break;
    case CImage::INT_32:
        Create8BitImage<int32_t>();
        break;
    // case CImage::UINT_RGB8:
    //     Create8BitImage<uint8_t>();
    //     break;
    // case CImage::INT_RGB8:
    //     Create8BitImage<int8_t>();
    //     break;
    // case CImage::UINT_RGB16:
    //     Create8BitImage<uint16_t>();
    //     break;
    // case CImage::INT_RGB16:
    //     Create8BitImage<int16_t>();
    //     break;
    case CImage::REAL_32:
        Create8BitImage<float>();
        break;
    case CImage::REAL_64:
        Create8BitImage<double>();
        break;
    default:
        assert(false);
    }

	ProcessImage();

	UpdateData(false);

	// let's use VBOs
	m_mesh.SetRenderMode(GLMesh::VBOMode);
}

CMarchingCubes::~CMarchingCubes()
{
    if(m_del8BitImage)
    {
        delete m_8bitImage;
    }
}

bool CMarchingCubes::UpdateData(bool bsave)
{
	if (bsave)
	{
		bool update = false;

		if (m_val != GetFloatValue(ISO_VALUE)) { m_val = GetFloatValue(ISO_VALUE);  update = true; }
		if (m_bsmooth != GetBoolValue(SMOOTH)) { m_bsmooth = GetBoolValue(SMOOTH); update = true; }
		if (m_bcloseSurface != GetBoolValue(CLOSE_SURFACE)) { m_bcloseSurface = GetBoolValue(CLOSE_SURFACE); update = true; };
		if (m_binvertSpace != GetBoolValue(INVERT_SPACE)) { m_binvertSpace = GetBoolValue(INVERT_SPACE); update = true; }
		AllowClipping(GetBoolValue(CLIP));
		m_col = GetColorValue(COLOR);
		m_spc = GetColorValue(SPECULAR_COLOR);
		m_shininess = GetFloatValue(SHININESS);

		if (update) CreateSurface();
	}
	else
	{
		SetFloatValue(ISO_VALUE, m_val);
		SetBoolValue(SMOOTH, m_bsmooth);
		SetBoolValue(CLOSE_SURFACE, m_bcloseSurface);
		SetBoolValue(INVERT_SPACE, m_binvertSpace);
		SetBoolValue(CLIP, AllowClipping());
		SetColorValue(COLOR, m_col);
		SetColorValue(SPECULAR_COLOR, m_spc);
		SetFloatValue(SHININESS, m_shininess);
	}

	return false;
}

void CMarchingCubes::SetSmooth(bool b)
{ 
	m_bsmooth = b; 
	m_oldVal = -1.f;
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

	CImageModel& im = *GetImageModel();
	if (im.Get3DImage() == nullptr) return;
	C3DImage& im3d = *m_8bitImage;

	BOX b = im.GetBoundingBox();
	vec3f r0 = to_vec3f(b.r0());
	vec3f r1 = to_vec3f(b.r1());

	int NX = im3d.Width();
	int NY = im3d.Height();
	int NZ = im3d.Depth();
	if ((NX == 1) || (NY == 1) || (NZ == 1)) return;

	float dxi = (float)((b.x1 - b.x0) / (NX - 1));
	float dyi = (float)((b.y1 - b.y0) / (NY - 1));
	float dzi = (float)((b.z1 - b.z0) / (NZ - 1));

	uint8_t ref = (uint8_t)(m_val * 255.0);
	float fref = (float)ref;
	m_ref = ref;

	C3DGradientMap grad(im3d, b);

	// construct a mesh object
	TriMesh mesh;

	#pragma omp parallel default(shared)
	{
		const int MAX_FACES = 1000000;
		TriMesh temp;
		temp.Resize(MAX_FACES);
		int nfaces = 0;
		uint8_t val[8];
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
						val[0] = im3d.GetByte(i, j, k);
						val[3] = im3d.GetByte(i, j + 1, k);
						val[4] = im3d.GetByte(i, j, k + 1);
						val[7] = im3d.GetByte(i, j + 1, k + 1);
					}

					val[1] = im3d.GetByte(i + 1, j, k);
					val[2] = im3d.GetByte(i + 1, j + 1, k);
					val[5] = im3d.GetByte(i + 1, j, k + 1);
					val[6] = im3d.GetByte(i + 1, j + 1, k + 1);

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
						r[0].x = r0.x + i      *dxi; r[0].y = r0.y + j      *dyi; r[0].z = r0.z + k      *dzi;
						r[1].x = r0.x + (i + 1)*dxi; r[1].y = r0.y + j      *dyi; r[1].z = r0.z + k      *dzi;
						r[2].x = r0.x + (i + 1)*dxi; r[2].y = r0.y + (j + 1)*dyi; r[2].z = r0.z + k      *dzi;
						r[3].x = r0.x + i      *dxi; r[3].y = r0.y + (j + 1)*dyi; r[3].z = r0.z + k      *dzi;
						r[4].x = r0.x + i      *dxi; r[4].y = r0.y + j      *dyi; r[4].z = r0.z + (k + 1)*dzi;
						r[5].x = r0.x + (i + 1)*dxi; r[5].y = r0.y + j      *dyi; r[5].z = r0.z + (k + 1)*dzi;
						r[6].x = r0.x + (i + 1)*dxi; r[6].y = r0.y + (j + 1)*dyi; r[6].z = r0.z + (k + 1)*dzi;
						r[7].x = r0.x + i      *dxi; r[7].y = r0.y + (j + 1)*dyi; r[7].z = r0.z + (k + 1)*dzi;

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

								tri.m_node[2 - m] = r[n1] * (1.f - w) + r[n2] * w;

								if (m_bsmooth)
								{
									vec3f normal = g[n1] * (1.f - w) + g[n2] * w;
									normal.Normalize();
									tri.m_norm[2 - m] = (m_binvertSpace ? normal : -normal);
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
								mesh.Merge(temp, nfaces);
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
		mesh.Merge(temp, nfaces);
	}

	// create surface meshes
	if (m_bcloseSurface)
	{
		uint8_t val[4];
		vec3f r[4];

		// X-planes
		for (int i = 0; i <= NX - 1; i += NX - 1)
		{
			vec3f faceNormal(1.f, 0.f, 0.f);

			float x = (i == 0 ? r0.x : r1.x);

			for (int k = 0; k < NZ - 1; k++)
			{
				for (int j = 0; j < NY - 1; ++j)
				{
					// get the pixel's values
					val[0] = im3d.GetByte(i, j, k);
					val[1] = im3d.GetByte(i, j + 1, k);
					val[2] = im3d.GetByte(i, j + 1, k + 1);
					val[3] = im3d.GetByte(i, j, k + 1);

					// get the corners
					r[0].x = x; r[0].y = r0.y + j      *dyi; r[0].z = r0.z + k*dzi;
					r[1].x = x; r[1].y = r0.y + (j + 1)*dyi; r[1].z = r0.z + k*dzi;
					r[2].x = x; r[2].y = r0.y + (j + 1)*dyi; r[2].z = r0.z + (k + 1)*dzi;
					r[3].x = x; r[3].y = r0.y + j      *dyi; r[3].z = r0.z + (k + 1)*dzi;

					// add the triangles
					AddSurfaceTris(mesh, val, r, faceNormal);
				}
			}
		}

		// Y-planes
		for (int j = 0; j <= NY - 1; j += NY - 1)
		{
			vec3f faceNormal(0.f, -1.f, 0.f);

			float y = (j == 0 ? r0.y : r1.y);

			for (int k = 0; k < NZ - 1; k++)
			{
				for (int i = 0; i < NX - 1; ++i)
				{
					// get the pixel's values
					val[0] = im3d.GetByte(i  , j, k);
					val[1] = im3d.GetByte(i+1, j, k);
					val[2] = im3d.GetByte(i+1, j, k + 1);
					val[3] = im3d.GetByte(i  , j, k + 1);

					// get the corners
					r[0].x = r0.x + i    *dxi; r[0].y = y; r[0].z = r0.z + k*dzi;
					r[1].x = r0.x + (i+1)*dxi; r[1].y = y; r[1].z = r0.z + k*dzi;
					r[2].x = r0.x + (i+1)*dxi; r[2].y = y; r[2].z = r0.z + (k + 1)*dzi;
					r[3].x = r0.x + i    *dxi; r[3].y = y; r[3].z = r0.z + (k + 1)*dzi;

					// add the triangles
					AddSurfaceTris(mesh, val, r, faceNormal);
				}
			}
		}

		// Z-planes
		for (int k = 0; k <= NZ - 1; k += NZ - 1)
		{
			vec3f faceNormal(0.f, 0.f, 1.f);

			float z = (k == 0 ? r0.z : r1.z);

			for (int j = 0; j < NY - 1; ++j)
			{
				for (int i = 0; i < NX - 1; ++i)
				{
					// get the pixel's values
					val[0] = im3d.GetByte(i    , j    , k);
					val[1] = im3d.GetByte(i + 1, j    , k);
					val[2] = im3d.GetByte(i + 1, j + 1, k);
					val[3] = im3d.GetByte(i    , j + 1, k);

					// get the corners
					r[0].x = r0.x + i      *dxi; r[0].y = r0.y + j      *dyi; r[0].z = z;
					r[1].x = r0.x + (i + 1)*dxi; r[1].y = r0.y + j      *dyi; r[1].z = z;
					r[2].x = r0.x + (i + 1)*dxi; r[2].y = r0.y + (j + 1)*dyi; r[2].z = z;
					r[3].x = r0.x + i      *dxi; r[3].y = r0.y + (j + 1)*dyi; r[3].z = z;

					// add the triangles
					AddSurfaceTris(mesh, val, r, faceNormal);
				}
			}
		}
	}

	// create vertex arrays from mesh
	int faces = mesh.Faces();
	m_mesh.Create(faces, GLMesh::FLAG_NORMAL);
	m_mesh.BeginMesh();
	for (int i = 0; i < faces; ++i)
	{
		Post::TriMesh::TRI& face = mesh.Face(i);
		for (int j = 0; j < 3; ++j) m_mesh.AddVertex(face.m_node[j], face.m_norm[j]);
	}
	m_mesh.EndMesh();
}

void CMarchingCubes::AddSurfaceTris(TriMesh& mesh, uint8_t val[4], vec3f r[4], const vec3f& faceNormal)
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
	int* pf = LUT2D_quad[ncase];
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

		mesh.AddFace(tri);

		pf += 3;
	}
}

void CMarchingCubes::SetIsoValue(float v)
{
	m_val = v;
}

void CMarchingCubes::Render(CGLContext& rc)
{
	GLfloat spc[4] = { m_spc.r / 255.f, m_spc.g / 255.f, m_spc.b / 255.f, 1.f };
	glColor3ub(m_col.r, m_col.g, m_col.b);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, (GLint)(128*m_shininess));

	m_mesh.Render(GLMesh::FLAG_NORMAL);
}

bool CMarchingCubes::GetMesh(FSMesh& mesh)
{
	int nodes = (int) m_mesh.Vertices();
	int faces = nodes / 3; assert((nodes % 3) == 0);
	mesh.Create(nodes, 0, faces);
	for (int i = 0; i < nodes; ++i)
	{
		GLMesh::Vertex v = m_mesh.GetVertex(i);
		mesh.Node(i).r = to_vec3d(v.r);
	}

	for (int i = 0; i < faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		face.SetType(FE_FACE_TRI3);
		face.n[0] = 3 * i;
		face.n[1] = 3 * i + 1;
		face.n[2] = 3 * i + 2;
	}

	mesh.UpdateNormals();

	return true;
}

// The purpose of this function is to find an initial value for m_val that does 
// not generate too many triangles. 
void CMarchingCubes::ProcessImage()
{
	CImageModel& im = *GetImageModel();
	if (im.Get3DImage() == nullptr) return;
	C3DImage& im3d = *m_8bitImage;

	int NX = im3d.Width();
	int NY = im3d.Height();
	int NZ = im3d.Depth();
	if ((NX == 1) || (NY == 1) || (NZ == 1)) return;

	uint8_t val[8];

	std::vector<std::pair<unsigned int, unsigned int> > bin;
	bin.resize(256);
	for (int i = 0; i < 256; ++i) {
		bin[i].first = i; bin[i].second = 0;
	}

	for (int k = 0; k < NZ - 1; ++k)
	{
		for (int j = 0; j < NY - 1; ++j)
		{
			for (int i = 0; i < NX - 1; ++i)
			{
				// get the voxel's values
				if (i == 0)
				{
					val[0] = im3d.GetByte(i, j, k);
					val[3] = im3d.GetByte(i, j + 1, k);
					val[4] = im3d.GetByte(i, j, k + 1);
					val[7] = im3d.GetByte(i, j + 1, k + 1);
				}

				val[1] = im3d.GetByte(i + 1, j, k);
				val[2] = im3d.GetByte(i + 1, j + 1, k);
				val[5] = im3d.GetByte(i + 1, j, k + 1);
				val[6] = im3d.GetByte(i + 1, j + 1, k + 1);

				// find the min/max
				uint8_t min = val[0], max = val[0];
				for (int l = 1; l < 8; ++l)
				{
					if (val[l] < min) min = val[l];
					if (val[l] > max) max = val[l];
				}

				for (int l = min; l <= max; ++l) bin[l].second++;

				// keep this for next i
				val[0] = val[1];
				val[4] = val[5];
				val[3] = val[2];
				val[7] = val[6];
			}
		}
	}

	// sort the bins
	std::sort(bin.begin(), bin.end(), [](pair<unsigned int, unsigned int>& a, pair<unsigned int, unsigned int>& b) {
		return a.second < b.second;
		});

	// pick the 25%
	int ival = bin[64].first;

	// set the initial value
	m_val = ival / 255.0;
}

template<class pType> void CMarchingCubes::Create8BitImage()
{
    C3DImage* oldImg = GetImageModel()->Get3DImage();

    int nx = oldImg->Width();
    int ny = oldImg->Height();
    int nz = oldImg->Depth();
    int N = nx*ny*nz;

    m_8bitImage = new C3DImage;
    m_8bitImage->Create(nx, ny, nz);

    double min, max;
    oldImg->GetMinMax(min, max);
    double range = max - min;

    pType* oldData = (pType*)oldImg->GetBytes();
    uint8_t* newData = m_8bitImage->GetBytes();

    for(int i = 0; i < N; i++)
    {
        newData[i] = (uint8_t)(255 * (oldData[i] - min)/range);
    }
}
