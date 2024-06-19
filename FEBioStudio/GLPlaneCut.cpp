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
#include "GLPlaneCut.h"
#include "GLView.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSModel.h>
#include <GLLib/GLMeshRender.h>
#include <MeshLib/GMesh.h>

const int HEX_NT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int PEN_NT[8] = { 0, 1, 2, 2, 3, 4, 5, 5 };
const int TET_NT[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
const int PYR_NT[8] = { 0, 1, 2, 3, 4, 4, 4, 4 };

// in MeshTools\lut.cpp
extern int LUT[256][15];
extern int ET_HEX[12][2];
extern int ET_TET[6][2];

GLPlaneCut::GLPlaneCut()
{

}

void GLPlaneCut::Clear() 
{ 
	delete m_planeCut; 
	m_planeCut = nullptr; 
}

bool GLPlaneCut::IsValid() const 
{ 
	return (m_planeCut != nullptr); 
}

void GLPlaneCut::BuildPlaneCut(FSModel& fem, bool showMeshData)
{
	GModel& mdl = fem.GetModel();
	GObject* poa = mdl.GetActiveObject();
	double vmin, vmax;

	if (mdl.Objects() == 0) return;

	int edge[15][2], edgeNode[15][2], etag[15];

	if (m_planeCut) delete m_planeCut;
	m_planeCut = new GMesh;
	GMesh* planeCut = m_planeCut;

	// TODO: swith to texture
	Post::CColorMap colormap;

	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		if (po->GetFEMesh())
		{
			FSMesh* mesh = po->GetFEMesh();

			vec3d ex[8];
			int en[8];
			GLColor ec[8];

			const Transform& T = po->GetTransform();

			// set the plane normal
			vec3d norm(m_plane[0], m_plane[1], m_plane[2]);
			double l = norm.Length(); if (l == 0) l = 1;
			norm.Normalize();
			double ref = -m_plane[3]/l;
			vec3d o = norm * ref;

			// convert to local
			norm = T.GlobalToLocalNormal(norm);
			o = T.GlobalToLocal(o);
			ref = norm * o;

			bool showContour = false;
			Mesh_Data& data = mesh->GetMeshData();
			if ((po == poa) && (showMeshData))
			{
				showContour = (showMeshData && data.IsValid());
				if (showContour) { data.GetValueRange(vmin, vmax); colormap.SetRange((float)vmin, (float)vmax); }
			}

			// repeat over all elements
			GLColor defaultColor(200, 200, 200);
			GLColor c(defaultColor);
			int matId = -1;
			int NE = mesh->Elements();
			for (int i = 0; i < NE; ++i)
			{
				// render only when visible
				FSElement& el = mesh->Element(i);
				if (el.IsVisible() && el.IsSolid())
				{
					int mid = el.m_MatID;
					if (mid != matId)
					{
						GMaterial* pmat = fem.GetMaterialFromID(mid);
						if (pmat)
						{
							c = fem.GetMaterialFromID(mid)->Diffuse();
							matId = mid;
						}
						else
						{
							matId = -1;
							c = po->GetColor();
						}
					}


					const int* nt = nullptr;
					switch (el.Type())
					{
					case FE_HEX8   : nt = HEX_NT; break;
					case FE_HEX20  : nt = HEX_NT; break;
					case FE_HEX27  : nt = HEX_NT; break;
					case FE_PENTA6 : nt = PEN_NT; break;
					case FE_PENTA15: nt = PEN_NT; break;
					case FE_TET4   : nt = TET_NT; break;
					case FE_TET5   : nt = TET_NT; break;
					case FE_TET10  : nt = TET_NT; break;
					case FE_TET15  : nt = TET_NT; break;
					case FE_TET20  : nt = TET_NT; break;
					case FE_PYRA5  : nt = PYR_NT; break;
					case FE_PYRA13 : nt = PYR_NT; break;
					default:
						assert(false);
					}

					// get the nodal values
					for (int k = 0; k < 8; ++k)
					{
						FSNode& node = mesh->Node(el.m_node[nt[k]]);
						ex[k] = node.r;
						en[k] = el.m_node[nt[k]];
					}

					if (showContour)
					{
						for (int k = 0; k < 8; ++k)
						{
							if (data.GetElementDataTag(i) > 0)
								ec[k] = colormap.map(data.GetElementValue(i, nt[k]));
							else
								ec[k] = defaultColor;
						}
					}

					// calculate the case of the element
					int ncase = 0;
					for (int k = 0; k < 8; ++k)
						if (norm * ex[k] > ref * 0.999999) ncase |= (1 << k);

					// loop over faces
					int* pf = LUT[ncase];
					int ne = 0;
					for (int l = 0; l < 5; l++)
					{
						if (*pf == -1) break;

						// calculate nodal positions
						vec3f r[3];
						float w1, w2, w;
						for (int k = 0; k < 3; k++)
						{
							int n1 = ET_HEX[pf[k]][0];
							int n2 = ET_HEX[pf[k]][1];

							w1 = norm * ex[n1];
							w2 = norm * ex[n2];

							if (w2 != w1)
								w = (ref - w1) / (w2 - w1);
							else
								w = 0.f;

							vec3d rk = ex[n1] * (1 - w) + ex[n2] * w;
							r[k] = to_vec3f(T.LocalToGlobal(rk));
						}

						int nf = planeCut->Faces();
						planeCut->AddFace(r, (el.IsSelected() ? 1 : 0));
						GMesh::FACE& face = planeCut->Face(nf);
						if (po == poa)
						{
							face.eid = i;
						}

						if (showContour)
						{
							GLColor c;
							for (int k = 0; k < 3; k++)
							{
								int n1 = ET_HEX[pf[k]][0];
								int n2 = ET_HEX[pf[k]][1];

								w1 = norm * ex[n1];
								w2 = norm * ex[n2];

								if (w2 != w1)
									w = (ref - w1) / (w2 - w1);
								else
									w = 0.f;

								c.r = (uint8_t)((double)ec[n1].r * (1.0 - w) + (double)ec[n2].r * w);
								c.g = (uint8_t)((double)ec[n1].g * (1.0 - w) + (double)ec[n2].g * w);
								c.b = (uint8_t)((double)ec[n1].b * (1.0 - w) + (double)ec[n2].b * w);

								face.c[k] = c;
							}
						}
						else
						{
							face.c[0] = face.c[1] = face.c[2] = c;
						}

						// add edges (for mesh rendering)
						for (int k = 0; k < 3; ++k)
						{
							int n1 = pf[k];
							int n2 = pf[(k + 1) % 3];

							bool badd = true;
							for (int m = 0; m < ne; ++m)
							{
								int m1 = edge[m][0];
								int m2 = edge[m][1];
								if (((n1 == m1) && (n2 == m2)) ||
									((n1 == m2) && (n2 == m1)))
								{
									badd = false;
									etag[m]++;
									break;
								}
							}

							if (badd)
							{
								edge[ne][0] = n1;
								edge[ne][1] = n2;
								etag[ne] = 0;

								GMesh::FACE& face = planeCut->Face(planeCut->Faces() - 1);
								edgeNode[ne][0] = face.n[k];
								edgeNode[ne][1] = face.n[(k + 1) % 3];
								++ne;
							}
						}
						pf += 3;
					}

					for (int k = 0; k < ne; ++k)
					{
						if (etag[k] == 0)
						{
							planeCut->AddEdge(edgeNode[k], 2, (el.IsSelected() ? 1 : 0));
						}
					}
				}
			}
		}
	}

	planeCut->Update();
}

void GLPlaneCut::Render(CGLContext& rc)
{
	if (m_planeCut == nullptr) return;

	GLMeshRender mr;

	// turn off specular lighting
	GLfloat spc[] = { 0.0f, 0.0f, 0.0f, 1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);

	// render the unselected faces
	glColor3ub(255, 255, 255);
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	mr.SetFaceColor(true);
	mr.RenderGLMesh(m_planeCut, 0);

	// render the selected faces
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	mr.SetRenderMode(GLMeshRender::SelectionMode);
	glColor3ub(255, 64, 0);
	mr.SetFaceColor(false);
	mr.RenderGLMesh(m_planeCut, 1);

	if (rc.m_settings.m_bmesh)
	{
		GLColor c = rc.m_settings.m_meshColor;
		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColor4ub(c.r, c.g, c.b, c.a);

		CGLCamera& cam = *rc.m_cam;
		cam.LineDrawMode(true);
		cam.PositionInScene();

		mr.RenderGLEdges(m_planeCut, 0);
		glDisable(GL_DEPTH_TEST);
		glColor3ub(255, 255, 0);
		mr.RenderGLEdges(m_planeCut, 1);

		cam.LineDrawMode(false);
		cam.PositionInScene();
	}
	glPopAttrib();
}

bool GLPlaneCut::Intersect(const vec3d& p, const Ray& ray, Intersection& q)
{
	if (m_planeCut == nullptr) return false;

	bool bfound = false;

	// see if the intersection lies behind the plane cut. 
	double* a = GetPlaneCoordinates();
	double d = p.x * a[0] + p.y * a[1] + p.z * a[2] + a[3];
	if (d < 0)
	{
		// find the intersection with the plane cut
		bfound = FindFaceIntersection(ray, *m_planeCut, q);

		if (bfound)
		{
			// convert the index from a face index into an element index
			int nface = q.m_index;
			if ((nface >= 0) && (nface < m_planeCut->Faces()))
			{
				GMesh::FACE& face = m_planeCut->Face(nface);
				q.m_index = face.eid;
				if (q.m_index < 0) bfound = false;
			}
			else bfound = false;
		}
	}

	return bfound;
}
