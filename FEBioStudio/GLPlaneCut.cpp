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
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSModel.h>
#include <MeshLib/GMesh.h>
#include <GLLib/GLCamera.h>
#include <GLLib/GLContext.h>
#include <GLLib/GLViewSettings.h>
#include <GLLib/GLRenderEngine.h>
#include <PostLib/ColorMap.h>

const int HEX_NT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int PEN_NT[8] = { 0, 1, 2, 2, 3, 4, 5, 5 };
const int TET_NT[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
const int PYR_NT[8] = { 0, 1, 2, 3, 4, 4, 4, 4 };

// in MeshTools\lut.cpp
extern int LUT[256][15];
extern int ET_HEX[12][2];
extern int ET_TET[6][2];
extern int LUT2D_quad[16][9];
extern int ET2D[4][2];

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

void GLPlaneCut::Create(FSModel& fem, bool showMeshData, int mode)
{
	switch (mode)
	{
	case Planecut_Mode::PLANECUT     : CreatePlaneCut(fem, showMeshData); break;
	case Planecut_Mode::HIDE_ELEMENTS: CreateHideElements(fem, showMeshData); break;
	default:
		break;
	}
}

void GLPlaneCut::CreatePlaneCut(FSModel& fem, bool showMeshData)
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

			const Transform& T = po->GetRenderTransform();

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
							c = fem.GetMaterialFromID(mid)->GetColor();
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
							r[2-k] = to_vec3f(T.LocalToGlobal(rk));
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

								face.c[2-k] = c;
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

void GLPlaneCut::CreateHideElements(FSModel& fem, bool showMeshData)
{
	GModel& mdl = fem.GetModel();
	GObject* poa = mdl.GetActiveObject();
	double vmin, vmax;

	if (mdl.Objects() == 0) return;

	if (m_planeCut) delete m_planeCut;
	m_planeCut = new GMesh;
	GMesh* planeCut = m_planeCut;

	// TODO: swith to texture
	Post::CColorMap colormap;

	for (int n = 0; n < mdl.Objects(); ++n)
	{
		GObject* po = mdl.Object(n);
		if (po->GetFEMesh())
		{
			FSMesh* mesh = po->GetFEMesh();

			vec3d ex[8];
			int en[8];
			double ev[8] = { 0 };
			GLColor ec[8];

			const Transform& T = po->GetRenderTransform();

			// set the plane normal
			vec3d norm(m_plane[0], m_plane[1], m_plane[2]);
			double l = norm.Length(); if (l == 0) l = 1;
			norm.Normalize();
			double ref = -m_plane[3] / l;
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
			int NE = mesh->Elements();
			for (int i = 0; i < NE; ++i)
			{
				// render only when visible
				FSElement& el = mesh->Element(i);
				el.m_ntag = -1;
				if (el.IsVisible() && el.IsSolid())
				{
					// see if the plane intersects this element
					const int* nt = nullptr;
					switch (el.Type())
					{
					case FE_HEX8: nt = HEX_NT; break;
					case FE_HEX20: nt = HEX_NT; break;
					case FE_HEX27: nt = HEX_NT; break;
					case FE_PENTA6: nt = PEN_NT; break;
					case FE_PENTA15: nt = PEN_NT; break;
					case FE_TET4: nt = TET_NT; break;
					case FE_TET5: nt = TET_NT; break;
					case FE_TET10: nt = TET_NT; break;
					case FE_TET15: nt = TET_NT; break;
					case FE_TET20: nt = TET_NT; break;
					case FE_PYRA5: nt = PYR_NT; break;
					case FE_PYRA13: nt = PYR_NT; break;
					default:
						assert(false);
					}

					// get the nodal values
					for (int k = 0; k < 8; ++k)
					{
						FSNode& node = mesh->Node(el.m_node[nt[k]]);
						ev[k] = norm * node.r;
					}

					// calculate the case of the element
					int ncase = 0;
					for (int k = 0; k < 8; ++k)
						if (ev[k] > ref * 0.999999) ncase |= (1 << k);

					el.m_ntag = ncase;
				}
			}

			GLColor defaultColor(200, 200, 200);
			GLColor c(defaultColor);
			int matId = -1;
			for (int i = 0; i < NE; ++i)
			{
				FSElement& el = mesh->Element(i);
				int ncase = el.m_ntag;

				if ((ncase > 0) && (ncase < 255))
				{
					int mid = el.m_MatID;
					if (mid != matId)
					{
						GMaterial* pmat = fem.GetMaterialFromID(mid);
						if (pmat)
						{
							c = fem.GetMaterialFromID(mid)->GetColor();
							matId = mid;
						}
						else
						{
							matId = -1;
							c = po->GetColor();
						}
					}

					FSFace f;
					for (int j = 0; j < el.Faces(); ++j)
					{
						FSElement* pej = (el.m_nbr[j] >= 0 ? &mesh->Element(el.m_nbr[j]) : nullptr);
						if ((pej == nullptr) || (pej->m_ntag == 0) || (pej->m_ntag == 255))
						{
							f = el.GetFace(j);

							// get the nodal values
							for (int k = 0; k < f.Nodes(); ++k)
							{
								FSNode& node = mesh->Node(f.n[k]);
								ex[k] = node.r;
								ev[k] = norm * ex[k];
							}
							if (f.Nodes() == 3)
							{
								ex[3] = ex[2];
								ev[3] = ev[2];
							}

/*							if (showContour)
							{
								for (int k = 0; k < 8; ++k)
								{
									if (data.GetElementDataTag(i) > 0)
										ec[k] = colormap.map(data.GetElementValue(i, nt[k]));
									else
										ec[k] = defaultColor;
								}
							}
*/
							// calculate the case of the face
							int ncase = 0;
							for (int k = 0; k < 4; ++k)
							{
								if (ev[k] > ref * 0.999999) ncase |= (1 << k);
							}

							// TODO: why do we need to do this?
							ncase = 15 - ncase;

							// loop over faces
							int* pf = LUT2D_quad[ncase];
							for (int l = 0; l < 3; l++)
							{
								if (*pf == -1) break;

								// calculate nodal positions
								vec3f r[3];
								for (int m = 0; m < 3; m++)
								{
									int node = pf[m];
									if (node < 4)
									{
										r[m] = to_vec3f(ex[node]);
									}
									else
									{
										int n1 = ET2D[node - 4][0];
										int n2 = ET2D[node - 4][1];

										float v1 = ev[n1];
										float v2 = ev[n2];

										float w = (ref - (float)v1) / ((float)v2 - (float)v1);
										vec3d p = ex[n1] * (1.f - w) + ex[n2] * w;
										r[m] = to_vec3f(p);
									}
								}

								int nf = planeCut->Faces();
								planeCut->AddFace(r, (el.IsSelected() ? 1 : 0));
								GMesh::FACE& face = planeCut->Face(nf);
								if (po == poa)
								{
									face.eid = i;
								}
								face.c[0] = face.c[1] = face.c[2] = c;

								pf += 3;
							}

							vec3f r[2];
							int ne = f.Edges();
							for (int k = 0; k < ne; ++k)
							{
								int n0 = f.n[k];
								int n1 = f.n[(k+1)%ne];

								r[0] = to_vec3f(mesh->Node(n0).r);
								r[1] = to_vec3f(mesh->Node(n1).r);

								planeCut->AddEdge(r, 2, (el.IsSelected() ? 1 : 0));
							}
						}
					}
				}
			}
		}
	}

	planeCut->Update();
}

void GLPlaneCut::Render(GLRenderEngine& re, GLContext& rc)
{
	if (m_planeCut == nullptr) return;

	// render the unselected faces
	re.setMaterial(GLMaterial::PLASTIC, GLColor(200, 200, 200));
	re.renderGMesh(*m_planeCut, 0, false);

	// render the selected faces
	re.setColor(GLColor(255, 64, 0));
	re.renderGMesh(*m_planeCut, 1, false);

	if (rc.m_settings.m_bmesh)
	{
		GLCamera& cam = *rc.m_cam;
		cam.LineDrawMode(true);
		re.positionCamera(cam);

		GLColor c = rc.m_settings.m_meshColor;
		re.setMaterial(GLMaterial::CONSTANT, c);
		re.renderGMeshEdges(*m_planeCut, 0, false);

		// TODO: This used to be drawn with depthtest off
		re.setColor(GLColor::Yellow());
		re.renderGMeshEdges(*m_planeCut, 1, false);

		cam.LineDrawMode(false);
		re.positionCamera(cam);
	}
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
