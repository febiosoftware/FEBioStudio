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

#include "GLMeshRender.h"
#include <MeshLib/FECoreMesh.h>
#include <MeshLib/FEElement.h>
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/quad8.h>
#include <MeshTools/GLMesh.h>
#include <GLLib/glx.h>

//-----------------------------------------------------------------------------
extern int ET_HEX[12][2];
extern int ET_HEX20[12][3];
extern int ET_TET[6][2];
extern int ET_PENTA[9][2];
extern int ET_PENTA15[9][3];
extern int ET_TET10[6][3];
extern int ET_PYRA5[8][2];
extern int ET_PYRA13[8][3];

GLMeshRender::GLMeshRender()
{
	m_bShell2Solid = false;
	m_nshellref = 0;
	m_ndivs = 1;
	m_pointSize = 7.f;
}

//-----------------------------------------------------------------------------
// TODO: This may not always give the desired result: I render using both
//		 element and face data. But that cannot always be guaranteed to be consistent.
//		 What I need to do is only render using element or face data, but not both.
void GLMeshRender::RenderHEX8(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_HEX8));
	FEElement_& e = *pe;
	vec3d r1, r2, r3, r4;
	vec3f n1, n2, n3, n4;
	glBegin(GL_QUADS);
	{
		for (int i = 0; i<6; ++i)
		{
			r1 = pm->Node(e.m_node[FTHEX8[i][0]]).r;
			r2 = pm->Node(e.m_node[FTHEX8[i][1]]).r;
			r3 = pm->Node(e.m_node[FTHEX8[i][2]]).r;
			r4 = pm->Node(e.m_node[FTHEX8[i][3]]).r;

			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);
				assert(&pm->ElementRef(pf->m_elem[0].eid) == pe);
				if (pf)
				{
					n1 = pf->m_nn[0];
					n2 = pf->m_nn[1];
					n3 = pf->m_nn[2];
					n4 = pf->m_nn[3];
				}
				else
				{
					vec3d n = (r2 - r1) ^ (r3 - r1);
					n.Normalize();
					n1 = n2 = n3 = n4 = to_vec3f(n);
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = to_vec3f(n);
			}

			if ((pen == 0) || ((!pen->IsVisible()) || (pen->IsSelected() && pen->IsSolid() && bsel)))
			{
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glVertex3d(r3.x, r3.y, r3.z);
				glNormal3d(n4.x, n4.y, n4.z); glVertex3d(r4.x, r4.y, r4.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
inline void glxColor(const GLColor& c)
{
	glColor3ub(c.r, c.g, c.b);
}

//-----------------------------------------------------------------------------
// TODO: This may not always give the desired result: I render using both
//		 element and face data. But that cannot always be guaranteed to be consistent.
//		 What I need to do is only render using element or face data, but not both.
void GLMeshRender::RenderHEX8(FEElement_ *pe, FECoreMesh *pm, GLColor* col)
{
	assert(pe->IsType(FE_HEX8));
	FEElement_& e = *pe;
	vec3d r1, r2, r3, r4;
	vec3f n1, n2, n3, n4;
	GLColor c[4];
	glBegin(GL_QUADS);
	{
		for (int i = 0; i<6; ++i)
		{
			r1 = pm->Node(e.m_node[FTHEX8[i][0]]).r;
			r2 = pm->Node(e.m_node[FTHEX8[i][1]]).r;
			r3 = pm->Node(e.m_node[FTHEX8[i][2]]).r;
			r4 = pm->Node(e.m_node[FTHEX8[i][3]]).r;

			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);
				assert(&pm->ElementRef(pf->m_elem[0].eid) == pe);
				if (pf)
				{
					n1 = pf->m_nn[0];
					n2 = pf->m_nn[1];
					n3 = pf->m_nn[2];
					n4 = pf->m_nn[3];

					c[0] = col[FTHEX8[i][0]];
					c[1] = col[FTHEX8[i][1]];
					c[2] = col[FTHEX8[i][2]];
					c[3] = col[FTHEX8[i][3]];
				}
				else
				{
					vec3d n = (r2 - r1) ^ (r3 - r1);
					n.Normalize();
					n1 = n2 = n3 = n4 = to_vec3f(n);
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = to_vec3f(n);
			}

			if ((pen == 0) || (!pen->IsVisible()))
			{
				glNormal3d(n1.x, n1.y, n1.z); glxColor(c[0]); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glxColor(c[1]); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glxColor(c[2]); glVertex3d(r3.x, r3.y, r3.z);
				glNormal3d(n4.x, n4.y, n4.z); glxColor(c[3]); glVertex3d(r4.x, r4.y, r4.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// TODO: This may not always give the desired result: I render using both
//		 element and face data. But that cannot always be guaranteed to be consistent.
//		 What I need to do is only render using element or face data, but not both.
void GLMeshRender::RenderHEX20(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_HEX20));
	FEElement_& e = *pe;
	vec3d r[9];
	vec3f n[9];
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i<6; ++i)
		{
			r[0] = pm->Node(e.m_node[FTHEX20[i][0]]).r;
			r[1] = pm->Node(e.m_node[FTHEX20[i][1]]).r;
			r[2] = pm->Node(e.m_node[FTHEX20[i][2]]).r;
			r[3] = pm->Node(e.m_node[FTHEX20[i][3]]).r;
			r[4] = pm->Node(e.m_node[FTHEX20[i][4]]).r;
			r[5] = pm->Node(e.m_node[FTHEX20[i][5]]).r;
			r[6] = pm->Node(e.m_node[FTHEX20[i][6]]).r;
			r[7] = pm->Node(e.m_node[FTHEX20[i][7]]).r;
			r[8] = QUAD8::eval(r, 0.0, 0.0);

			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);
				if (pf)
				{
					n[0] = pf->m_nn[0];
					n[1] = pf->m_nn[1];
					n[2] = pf->m_nn[2];
					n[3] = pf->m_nn[3];
					n[4] = pf->m_nn[4];
					n[5] = pf->m_nn[5];
					n[6] = pf->m_nn[6];
					n[7] = pf->m_nn[7];
					n[8] = (n[0] + n[1] + n[2] + n[3]); n[8].Normalize();
				}
				else
				{
					vec3d nn = (r[1] - r[0]) ^ (r[2] - r[0]);
					nn.Normalize();
					n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = n[7] = n[8] = to_vec3f(nn);
				}
			}
			else
			{
				vec3d nn = (r[1] - r[0]) ^ (r[2] - r[0]);
				nn.Normalize();
				n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = n[7] = n[8] = to_vec3f(nn);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glx::vertex3d(r[0], n[0]);
				glx::vertex3d(r[4], n[4]);
				glx::vertex3d(r[7], n[7]);

				glx::vertex3d(r[4], n[4]);
				glx::vertex3d(r[1], n[1]);
				glx::vertex3d(r[5], n[5]);

				glx::vertex3d(r[5], n[5]);
				glx::vertex3d(r[2], n[2]);
				glx::vertex3d(r[6], n[6]);

				glx::vertex3d(r[6], n[6]);
				glx::vertex3d(r[3], n[3]);
				glx::vertex3d(r[7], n[7]);

				glx::vertex3d(r[8], n[8]);
				glx::vertex3d(r[7], n[7]);
				glx::vertex3d(r[4], n[4]);

				glx::vertex3d(r[8], n[8]);
				glx::vertex3d(r[4], n[4]);
				glx::vertex3d(r[5], n[5]);

				glx::vertex3d(r[8], n[8]);
				glx::vertex3d(r[5], n[5]);
				glx::vertex3d(r[6], n[6]);

				glx::vertex3d(r[8], n[8]);
				glx::vertex3d(r[6], n[6]);
				glx::vertex3d(r[7], n[7]);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// TODO: This may not always give the desired result: I render using both
//		 element and face data. But that cannot always be guaranteed to be consistent.
//		 What I need to do is only render using element or face data, but not both.
void GLMeshRender::RenderHEX27(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_HEX27));
	FEElement_& e = *pe;
	vec3d r1, r2, r3, r4, r5, r6, r7, r8;
	vec3f n1, n2, n3, n4, n5, n6, n7, n8;
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i<6; ++i)
		{
			r1 = pm->Node(e.m_node[FTHEX20[i][0]]).r;
			r2 = pm->Node(e.m_node[FTHEX20[i][1]]).r;
			r3 = pm->Node(e.m_node[FTHEX20[i][2]]).r;
			r4 = pm->Node(e.m_node[FTHEX20[i][3]]).r;
			r5 = pm->Node(e.m_node[FTHEX20[i][4]]).r;
			r6 = pm->Node(e.m_node[FTHEX20[i][5]]).r;
			r7 = pm->Node(e.m_node[FTHEX20[i][6]]).r;
			r8 = pm->Node(e.m_node[FTHEX20[i][7]]).r;

			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);
				if (pf)
				{
					n1 = pf->m_nn[0];
					n2 = pf->m_nn[1];
					n3 = pf->m_nn[2];
					n4 = pf->m_nn[3];
					n5 = pf->m_nn[4];
					n6 = pf->m_nn[5];
					n7 = pf->m_nn[6];
					n8 = pf->m_nn[7];
				}
				else
				{
					vec3d n = (r2 - r1) ^ (r3 - r1);
					n.Normalize();
					n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = to_vec3f(n);
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = to_vec3f(n);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glx::vertex3d(r1, n1);
				glx::vertex3d(r5, n5);
				glx::vertex3d(r8, n8);

				glx::vertex3d(r5, n5);
				glx::vertex3d(r2, n2);
				glx::vertex3d(r6, n6);

				glx::vertex3d(r6, n6);
				glx::vertex3d(r3, n3);
				glx::vertex3d(r7, n7);

				glx::vertex3d(r7, n7);
				glx::vertex3d(r4, n4);
				glx::vertex3d(r8, n8);

				glx::vertex3d(r5, n5);
				glx::vertex3d(r6, n6);
				glx::vertex3d(r8, n8);

				glx::vertex3d(r6, n6);
				glx::vertex3d(r7, n7);
				glx::vertex3d(r8, n8);
			}
		}
	}
	glEnd();
}


//-----------------------------------------------------------------------------
// TODO: This may not always give the desired result: I render using both
//		 element and face data. But that cannot always be guaranteed to be consistent.
//		 What I need to do is only render using element or face data, but not both.
void GLMeshRender::RenderPENTA(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_PENTA6));
	FEElement_& e = *pe;
	glBegin(GL_TRIANGLES);
	{
		vec3d r[4];
		vec3d n[4];
		for (int j = 0; j<3; j++)
		{
			r[0] = pm->Node(e.m_node[FTPENTA[j][0]]).r;
			r[1] = pm->Node(e.m_node[FTPENTA[j][1]]).r;
			r[2] = pm->Node(e.m_node[FTPENTA[j][2]]).r;
			r[3] = pm->Node(e.m_node[FTPENTA[j][3]]).r;

			FEElement_* pen = (e.m_nbr[j] != -1 ? pm->ElementPtr(e.m_nbr[j]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[j]);
				assert(pm->ElementPtr(pf->m_elem[0].eid) == pe);
				n[0] = to_vec3d(pf->m_nn[0]);
				n[1] = to_vec3d(pf->m_nn[1]);
				n[2] = to_vec3d(pf->m_nn[2]);
				n[3] = to_vec3d(pf->m_nn[3]);
			}
			else
			{
				vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
				fn.Normalize();
				n[0] = n[1] = n[2] = n[3] = fn;
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glx::quad4(r, n);
			}
		}
	}
	glEnd();

	glBegin(GL_TRIANGLES);
	{
		vec3d r[3];
		vec3f n[3];
		for (int j = 3; j<5; j++)
		{
			r[0] = pm->Node(e.m_node[FTPENTA[j][0]]).r;
			r[1] = pm->Node(e.m_node[FTPENTA[j][1]]).r;
			r[2] = pm->Node(e.m_node[FTPENTA[j][2]]).r;

			FEElement_* pen = (e.m_nbr[j] != -1 ? pm->ElementPtr(e.m_nbr[j]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[j]);
				n[0] = pf->m_nn[0];
				n[1] = pf->m_nn[1];
				n[2] = pf->m_nn[2];
			}
			else
			{
				vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
				fn.Normalize();
				n[0] = n[1] = n[2] = to_vec3f(fn);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glx::tri3(r, n);
			}
		}
	}
	glEnd();
}


//-----------------------------------------------------------------------------
// TODO: This may not always give the desired result: I render using both
//		 element and face data. But that cannot always be guaranteed to be consistent.
//		 What I need to do is only render using element or face data, but not both.
void GLMeshRender::RenderPENTA15(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_PENTA15));
	FEElement_& e = *pe;
	vec3d r1, r2, r3, r4, r5, r6, r7, r8;
	vec3f n1, n2, n3, n4, n5, n6, n7, n8;
	glBegin(GL_TRIANGLES);
	{
		for (int j = 0; j<3; j++)
		{
			r1 = pm->Node(e.m_node[FTPENTA15[j][0]]).r;
			r2 = pm->Node(e.m_node[FTPENTA15[j][1]]).r;
			r3 = pm->Node(e.m_node[FTPENTA15[j][2]]).r;
			r4 = pm->Node(e.m_node[FTPENTA15[j][3]]).r;
			r5 = pm->Node(e.m_node[FTPENTA15[j][4]]).r;
			r6 = pm->Node(e.m_node[FTPENTA15[j][5]]).r;
			r7 = pm->Node(e.m_node[FTPENTA15[j][6]]).r;
			r8 = pm->Node(e.m_node[FTPENTA15[j][7]]).r;

			FEElement_* pen = (e.m_nbr[j] != -1 ? pm->ElementPtr(e.m_nbr[j]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[j]);
				assert(pf);
				if (pf)
				{
					n1 = pf->m_nn[0];
					n2 = pf->m_nn[1];
					n3 = pf->m_nn[2];
					n4 = pf->m_nn[3];
					n5 = pf->m_nn[4];
					n6 = pf->m_nn[5];
					n7 = pf->m_nn[6];
					n8 = pf->m_nn[7];
				}
				else
				{
					vec3d n = (r2 - r1) ^ (r3 - r1);
					n.Normalize();
					n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = to_vec3f(n);
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = to_vec3f(n);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n5.x, n5.y, n5.z); glVertex3d(r5.x, r5.y, r5.z);
				glNormal3d(n8.x, n8.y, n8.z); glVertex3d(r8.x, r8.y, r8.z);

				glNormal3d(n5.x, n5.y, n5.z); glVertex3d(r5.x, r5.y, r5.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n6.x, n6.y, n6.z); glVertex3d(r6.x, r6.y, r6.z);

				glNormal3d(n6.x, n6.y, n6.z); glVertex3d(r6.x, r6.y, r6.z);
				glNormal3d(n3.x, n3.y, n3.z); glVertex3d(r3.x, r3.y, r3.z);
				glNormal3d(n7.x, n7.y, n7.z); glVertex3d(r7.x, r7.y, r7.z);

				glNormal3d(n7.x, n7.y, n7.z); glVertex3d(r7.x, r7.y, r7.z);
				glNormal3d(n4.x, n4.y, n4.z); glVertex3d(r4.x, r4.y, r4.z);
				glNormal3d(n8.x, n8.y, n8.z); glVertex3d(r8.x, r8.y, r8.z);

				glNormal3d(n5.x, n5.y, n5.z); glVertex3d(r5.x, r5.y, r5.z);
				glNormal3d(n6.x, n6.y, n6.z); glVertex3d(r6.x, r6.y, r6.z);
				glNormal3d(n8.x, n8.y, n8.z); glVertex3d(r8.x, r8.y, r8.z);

				glNormal3d(n6.x, n6.y, n6.z); glVertex3d(r6.x, r6.y, r6.z);
				glNormal3d(n7.x, n7.y, n7.z); glVertex3d(r7.x, r7.y, r7.z);
				glNormal3d(n8.x, n8.y, n8.z); glVertex3d(r8.x, r8.y, r8.z);
			}
		}
	}
	glEnd();

	glBegin(GL_TRIANGLES);
	{
		for (int j = 3; j<5; j++)
		{
			r1 = pm->Node(e.m_node[FTPENTA15[j][0]]).r;
			r2 = pm->Node(e.m_node[FTPENTA15[j][1]]).r;
			r3 = pm->Node(e.m_node[FTPENTA15[j][2]]).r;
			r4 = pm->Node(e.m_node[FTPENTA15[j][3]]).r;
			r5 = pm->Node(e.m_node[FTPENTA15[j][4]]).r;
			r6 = pm->Node(e.m_node[FTPENTA15[j][5]]).r;

			FEElement_* pen = (e.m_nbr[j] != -1 ? pm->ElementPtr(e.m_nbr[j]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[j]);
				assert(pf);
				if (pf)
				{
					n1 = pf->m_nn[0];
					n2 = pf->m_nn[1];
					n3 = pf->m_nn[2];
					n4 = pf->m_nn[3];
					n5 = pf->m_nn[4];
					n6 = pf->m_nn[5];
				}
				else
				{
					vec3d n = (r2 - r1) ^ (r3 - r1);
					n.Normalize();
					n1 = n2 = n3 = n4 = n5 = n6 = to_vec3f(n);
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = n5 = n6 = to_vec3f(n);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n4.x, n4.y, n4.z); glVertex3d(r4.x, r4.y, r4.z);
				glNormal3d(n6.x, n6.y, n6.z); glVertex3d(r6.x, r6.y, r6.z);

				glNormal3d(n4.x, n4.y, n4.z); glVertex3d(r4.x, r4.y, r4.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n5.x, n5.y, n5.z); glVertex3d(r5.x, r5.y, r5.z);

				glNormal3d(n5.x, n5.y, n5.z); glVertex3d(r5.x, r5.y, r5.z);
				glNormal3d(n3.x, n3.y, n3.z); glVertex3d(r3.x, r3.y, r3.z);
				glNormal3d(n6.x, n6.y, n6.z); glVertex3d(r6.x, r6.y, r6.z);

				glNormal3d(n4.x, n4.y, n4.z); glVertex3d(r4.x, r4.y, r4.z);
				glNormal3d(n5.x, n5.y, n5.z); glVertex3d(r5.x, r5.y, r5.z);
				glNormal3d(n6.x, n6.y, n6.z); glVertex3d(r6.x, r6.y, r6.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTET4(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TET4) || pe->IsType(FE_TET5));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3f n1, n2, n3;
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i<4; ++i)
		{
			bool bdraw = true;
			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				if (pf)
				{
					r1 = pm->Node(pf->n[0]).r;
					r2 = pm->Node(pf->n[1]).r;
					r3 = pm->Node(pf->n[2]).r;

					n1 = pf->m_nn[0];
					n2 = pf->m_nn[1];
					n3 = pf->m_nn[2];
				}
				else bdraw = false;
			}
			else
			{
				r1 = pm->Node(e.m_node[FTTET[i][0]]).r;
				r2 = pm->Node(e.m_node[FTTET[i][1]]).r;
				r3 = pm->Node(e.m_node[FTTET[i][2]]).r;

				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = to_vec3f(n);

				bdraw = (!pen->IsVisible()) || (pen->IsSelected() && bsel);
			}

			if (bdraw)
			{
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glVertex3d(r3.x, r3.y, r3.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTET4(FEElement_ *pe, FECoreMesh *pm, GLColor* col)
{
	assert(pe->IsType(FE_TET4) || pe->IsType(FE_TET5));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3f n1, n2, n3;
	GLColor c[3];
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i<4; ++i)
		{
			bool bdraw = true;
			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if ((pen == 0) || (!pen->IsVisible()))
			{
				if (e.m_face[i] >= 0)
				{
					FEFace* pf = pm->FacePtr(e.m_face[i]);
					r1 = pm->Node(pf->n[0]).r;
					r2 = pm->Node(pf->n[1]).r;
					r3 = pm->Node(pf->n[2]).r;

					n1 = pf->m_nn[0];
					n2 = pf->m_nn[1];
					n3 = pf->m_nn[2];
				}
				else
				{
					r1 = pm->Node(e.m_node[FTTET[i][0]]).r;
					r2 = pm->Node(e.m_node[FTTET[i][1]]).r;
					r3 = pm->Node(e.m_node[FTTET[i][2]]).r;

					vec3d n = (r2 - r1) ^ (r3 - r1);
					n.Normalize();
					n1 = n2 = n3 = to_vec3f(n);
				}

				c[0] = col[FTTET[i][0]];
				c[1] = col[FTTET[i][1]];
				c[2] = col[FTTET[i][2]];

				glNormal3d(n1.x, n1.y, n1.z); glxColor(c[0]); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glxColor(c[1]); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glxColor(c[2]); glVertex3d(r3.x, r3.y, r3.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTRI3(FEElement_ *pe, FECoreMesh *pm, GLColor* col)
{
	assert(pe->IsType(FE_TRI3));
	FEElement_& e = *pe;
	FEFace* pf = pm->FacePtr(e.m_face[0]);
	if (pf == 0) return;
	vec3d r[3];
	vec3f n[3];

	r[0] = pm->Node(e.m_node[0]).r;
	r[1] = pm->Node(e.m_node[1]).r;
	r[2] = pm->Node(e.m_node[2]).r;

	n[0] = pf->m_nn[0];
	n[1] = pf->m_nn[1];
	n[2] = pf->m_nn[2];

	glBegin(GL_TRIANGLES);
	{
		glx::vertex3d(r[0], n[0], col[0]);
		glx::vertex3d(r[1], n[1], col[1]);
		glx::vertex3d(r[2], n[2], col[2]);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderQUAD(FEElement_ *pe, FECoreMesh *pm, GLColor* col)
{
	assert(pe->IsType(FE_QUAD4));
	FEElement_& e = *pe;
	FEFace* pf = pm->FacePtr(e.m_face[0]);
	if (pf == 0) return;
	vec3d r[4];
	vec3f n[4];

	r[0] = pm->Node(e.m_node[0]).r;
	r[1] = pm->Node(e.m_node[1]).r;
	r[2] = pm->Node(e.m_node[2]).r;
	r[3] = pm->Node(e.m_node[3]).r;

	n[0] = pf->m_nn[0];
	n[1] = pf->m_nn[1];
	n[2] = pf->m_nn[2];
	n[3] = pf->m_nn[3];

	glBegin(GL_TRIANGLES);
	{
		glx::vertex3d(r[0], n[0], col[0]);
		glx::vertex3d(r[1], n[1], col[1]);
		glx::vertex3d(r[2], n[2], col[2]);

		glx::vertex3d(r[2], n[2], col[2]);
		glx::vertex3d(r[3], n[3], col[3]);
		glx::vertex3d(r[0], n[0], col[0]);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTET10(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TET10));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3f n1, n2, n3;
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i<4; ++i)
		{
			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);

				r1 = pm->Node(pf->n[0]).r;
				r2 = pm->Node(pf->n[1]).r;
				r3 = pm->Node(pf->n[2]).r;

				n1 = pf->m_nn[0];
				n2 = pf->m_nn[1];
				n3 = pf->m_nn[2];
			}
			else
			{
				r1 = pm->Node(e.m_node[FTTET10[i][0]]).r;
				r2 = pm->Node(e.m_node[FTTET10[i][1]]).r;
				r3 = pm->Node(e.m_node[FTTET10[i][2]]).r;

				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = to_vec3f(n);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glVertex3d(r3.x, r3.y, r3.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTET10(FEElement_ *pe, FECoreMesh *pm, GLColor* c)
{
	assert(pe->IsType(FE_TET10));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3f n1, n2, n3;
	GLColor c1, c2, c3;
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < 4; ++i)
		{
			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);

			r1 = pm->Node(e.m_node[FTTET10[i][0]]).r;
			r2 = pm->Node(e.m_node[FTTET10[i][1]]).r;
			r3 = pm->Node(e.m_node[FTTET10[i][2]]).r;

			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);
				n1 = pf->m_nn[0];
				n2 = pf->m_nn[1];
				n3 = pf->m_nn[2];
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = to_vec3f(n);
			}

			c1 = c[FTTET10[i][0]];
			c2 = c[FTTET10[i][1]];
			c3 = c[FTTET10[i][2]];

			if ((pen == 0) || (!pen->IsVisible()))
			{
				glNormal3d(n1.x, n1.y, n1.z); glColor3ub(c1.r, c1.g, c1.b); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glColor3ub(c2.r, c2.g, c2.b); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glColor3ub(c3.r, c3.g, c3.b); glVertex3d(r3.x, r3.y, r3.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTET15(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TET15));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3f n1, n2, n3;
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i<4; ++i)
		{
			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);

				r1 = pm->Node(pf->n[0]).r;
				r2 = pm->Node(pf->n[1]).r;
				r3 = pm->Node(pf->n[2]).r;

				n1 = pf->m_nn[0];
				n2 = pf->m_nn[1];
				n3 = pf->m_nn[2];
			}
			else
			{
				r1 = pm->Node(e.m_node[FTTET15[i][0]]).r;
				r2 = pm->Node(e.m_node[FTTET15[i][1]]).r;
				r3 = pm->Node(e.m_node[FTTET15[i][2]]).r;

				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = to_vec3f(n);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glVertex3d(r3.x, r3.y, r3.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTET20(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TET20));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3f n1, n2, n3;
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i<4; ++i)
		{
			FEElement_* pen = (e.m_nbr[i] != -1 ? pm->ElementPtr(e.m_nbr[i]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[i]);
				assert(pf);

				r1 = pm->Node(pf->n[0]).r;
				r2 = pm->Node(pf->n[1]).r;
				r3 = pm->Node(pf->n[2]).r;

				n1 = pf->m_nn[0];
				n2 = pf->m_nn[1];
				n3 = pf->m_nn[2];
			}
			else
			{
				r1 = pm->Node(e.m_node[FTTET20[i][0]]).r;
				r2 = pm->Node(e.m_node[FTTET20[i][1]]).r;
				r3 = pm->Node(e.m_node[FTTET20[i][2]]).r;

				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = to_vec3f(n);
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
				glNormal3d(n3.x, n3.y, n3.z); glVertex3d(r3.x, r3.y, r3.z);
			}
		}
	}
	glEnd();
}


//-----------------------------------------------------------------------------
void GLMeshRender::RenderQUAD(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_QUAD4));
	FEElement_& e = *pe;
	FEFace* pf = pm->FacePtr(e.m_face[0]);
	if (pf == 0) return;
	vec3d r[4];
	vec3d n[4];

	r[0] = pm->Node(e.m_node[0]).r;
	r[1] = pm->Node(e.m_node[1]).r;
	r[2] = pm->Node(e.m_node[2]).r;
	r[3] = pm->Node(e.m_node[3]).r;

	n[0] = to_vec3d(pf->m_nn[0]);
	n[1] = to_vec3d(pf->m_nn[1]);
	n[2] = to_vec3d(pf->m_nn[2]);
	n[3] = to_vec3d(pf->m_nn[3]);

	glBegin(GL_TRIANGLES);
	{
		glx::quad4(r, n);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderQUAD8(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_QUAD8));
	FEElement_& e = *pe;
	FEFace* pf = pm->FacePtr(e.m_face[0]);
	if (pf == 0) return;
	vec3d r[4];
	vec3d n[4];

	r[0] = pm->Node(e.m_node[0]).r;
	r[1] = pm->Node(e.m_node[1]).r;
	r[2] = pm->Node(e.m_node[2]).r;
	r[3] = pm->Node(e.m_node[3]).r;

	n[0] = to_vec3d(pf->m_nn[0]);
	n[1] = to_vec3d(pf->m_nn[1]);
	n[2] = to_vec3d(pf->m_nn[2]);
	n[3] = to_vec3d(pf->m_nn[3]);

	glBegin(GL_TRIANGLES);
	{
		glx::quad4(r, n);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderQUAD9(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_QUAD9));
	FEElement_& e = *pe;
	FEFace* pf = pm->FacePtr(e.m_face[0]);
	if (pf == 0) return;
	vec3d r[4];
	vec3d n[4];

	r[0] = pm->Node(e.m_node[0]).r;
	r[1] = pm->Node(e.m_node[1]).r;
	r[2] = pm->Node(e.m_node[2]).r;
	r[3] = pm->Node(e.m_node[3]).r;

	n[0] = to_vec3d(pf->m_nn[0]);
	n[1] = to_vec3d(pf->m_nn[1]);
	n[2] = to_vec3d(pf->m_nn[2]);
	n[3] = to_vec3d(pf->m_nn[3]);

	glBegin(GL_TRIANGLES);
	{
		glx::quad4(r, n);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTRI3(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TRI3));
	FEElement_& e = *pe;
	FEFace* pf = pm->FacePtr(e.m_face[0]); assert(pf);
	if (pf == 0) return;
	vec3d r[3];
	vec3f n[3];

	r[0] = pm->Node(e.m_node[0]).r;
	r[1] = pm->Node(e.m_node[1]).r;
	r[2] = pm->Node(e.m_node[2]).r;

	n[0] = pf->m_nn[0];
	n[1] = pf->m_nn[1];
	n[2] = pf->m_nn[2];

	glBegin(GL_TRIANGLES);
	{
		glx::tri3(r, n);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderTRI6(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TRI6));
	FEElement_& e = *pe;
	FEFace* pf = pm->FacePtr(e.m_face[0]); assert(pf);
	if (pf == 0) return;
	vec3d r[3];
	vec3f n[3];

	r[0] = pm->Node(e.m_node[0]).r;
	r[1] = pm->Node(e.m_node[1]).r;
	r[2] = pm->Node(e.m_node[2]).r;

	n[0] = pf->m_nn[0];
	n[1] = pf->m_nn[1];
	n[2] = pf->m_nn[2];

	glBegin(GL_TRIANGLES);
	{
		glx::tri3(r, n);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderPYRA5(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_PYRA5));
	FEElement_& e = *pe;
	vec3d r[4];
	vec3d n[4];

	glBegin(GL_TRIANGLES);
	{
		for (int j = 0; j<4; j++)
		{
			r[0] = pm->Node(e.m_node[FTPYRA5[j][0]]).r;
			r[1] = pm->Node(e.m_node[FTPYRA5[j][1]]).r;
			r[2] = pm->Node(e.m_node[FTPYRA5[j][2]]).r;

			FEElement_* pen = (e.m_nbr[j] != -1 ? pm->ElementPtr(e.m_nbr[j]) : 0);
			if (pen == 0)
			{
				FEFace* pf = pm->FacePtr(e.m_face[j]);
				n[0] = to_vec3d(pf->m_nn[0]);
				n[1] = to_vec3d(pf->m_nn[1]);
				n[2] = to_vec3d(pf->m_nn[2]);
			}
			else
			{
				vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
				fn.Normalize();
				n[0] = n[1] = n[2] = fn;
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glx::vertex3d(r[0], n[0]);
				glx::vertex3d(r[1], n[1]);
				glx::vertex3d(r[2], n[2]);
			}
		}
	}
	glEnd();

	glBegin(GL_TRIANGLES);
	{
		r[0] = pm->Node(e.m_node[FTPYRA5[4][0]]).r;
		r[1] = pm->Node(e.m_node[FTPYRA5[4][1]]).r;
		r[2] = pm->Node(e.m_node[FTPYRA5[4][2]]).r;
		r[3] = pm->Node(e.m_node[FTPYRA5[4][3]]).r;

		FEElement_* pen = (e.m_nbr[4] != -1 ? pm->ElementPtr(e.m_nbr[4]) : 0);
		if (pen == 0)
		{
			FEFace* pf = pm->FacePtr(e.m_face[4]);
			assert(pm->ElementPtr(pf->m_elem[0].eid) == pe);
			n[0] = to_vec3d(pf->m_nn[0]);
			n[1] = to_vec3d(pf->m_nn[1]);
			n[2] = to_vec3d(pf->m_nn[2]);
			n[3] = to_vec3d(pf->m_nn[3]);
		}
		else
		{
			vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
			fn.Normalize();
			n[0] = n[1] = n[2] = n[3] = fn;
		}

		if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
		{
			glx::quad4(r, n);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderPYRA13(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
    assert(pe->IsType(FE_PYRA13));
    FEElement_& e = *pe;
    vec3d r[4];
    vec3d n[4];
    
    glBegin(GL_TRIANGLES);
    {
        for (int j = 0; j<4; j++)
        {
            r[0] = pm->Node(e.m_node[FTPYRA13[j][0]]).r;
            r[1] = pm->Node(e.m_node[FTPYRA13[j][1]]).r;
            r[2] = pm->Node(e.m_node[FTPYRA13[j][2]]).r;
            
            FEElement_* pen = (e.m_nbr[j] != -1 ? pm->ElementPtr(e.m_nbr[j]) : 0);
            if (pen == 0)
            {
                FEFace* pf = pm->FacePtr(e.m_face[j]);
                n[0] = to_vec3d(pf->m_nn[0]);
                n[1] = to_vec3d(pf->m_nn[1]);
                n[2] = to_vec3d(pf->m_nn[2]);
            }
            else
            {
                vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
                fn.Normalize();
                n[0] = n[1] = n[2] = fn;
            }
            
            if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
            {
                glx::vertex3d(r[0], n[0]);
                glx::vertex3d(r[1], n[1]);
                glx::vertex3d(r[2], n[2]);
            }
        }
    }
    glEnd();
    
    glBegin(GL_TRIANGLES);
    {
        r[0] = pm->Node(e.m_node[FTPYRA13[4][0]]).r;
        r[1] = pm->Node(e.m_node[FTPYRA13[4][1]]).r;
        r[2] = pm->Node(e.m_node[FTPYRA13[4][2]]).r;
        r[3] = pm->Node(e.m_node[FTPYRA13[4][3]]).r;
        
        FEElement_* pen = (e.m_nbr[4] != -1 ? pm->ElementPtr(e.m_nbr[4]) : 0);
        if (pen == 0)
        {
            FEFace* pf = pm->FacePtr(e.m_face[4]);
            assert(pm->ElementPtr(pf->m_elem[0].eid) == pe);
            n[0] = to_vec3d(pf->m_nn[0]);
            n[1] = to_vec3d(pf->m_nn[1]);
            n[2] = to_vec3d(pf->m_nn[2]);
            n[3] = to_vec3d(pf->m_nn[3]);
        }
        else
        {
            vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
            fn.Normalize();
            n[0] = n[1] = n[2] = n[3] = fn;
        }
        
        if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
        {
            glx::quad4(r, n);
        }
    }
    glEnd();
}


//-----------------------------------------------------------------------------
void GLMeshRender::RenderGLMesh(GLMesh* pm, int nid)
{
	vec3d r0, r1, r2;
	vec3d n0, n1, n2;
	if (nid == -1)
	{
		int N = pm->Faces();
		glBegin(GL_TRIANGLES);
		{
			for (int i = 0; i<N; ++i)
			{
				GMesh::FACE& f = pm->Face(i);
				r0 = pm->Node(f.n[0]).r;
				r1 = pm->Node(f.n[1]).r;
				r2 = pm->Node(f.n[2]).r;

				n0 = f.nn[0];
				n1 = f.nn[1];
				n2 = f.nn[2];

				glxColor(f.c[0]); glNormal3d(n0.x, n0.y, n0.z); glVertex3d(r0.x, r0.y, r0.z);
				glxColor(f.c[1]); glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glxColor(f.c[2]); glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
			}
		}
		glEnd();
	}
	else if (nid < (int) pm->m_FIL.size())
	{
		assert(pm->m_FIL.size() > 0);
		glBegin(GL_TRIANGLES);
		{
			pair<int, int> fil = pm->m_FIL[nid];
			for (int i = 0; i<fil.second; ++i)
			{
				GMesh::FACE& f = pm->Face(i + fil.first);
				assert(f.pid == nid);
				r0 = pm->Node(f.n[0]).r;
				r1 = pm->Node(f.n[1]).r;
				r2 = pm->Node(f.n[2]).r;

				n0 = f.nn[0];
				n1 = f.nn[1];
				n2 = f.nn[2];

				glNormal3d(n0.x, n0.y, n0.z); glVertex3d(r0.x, r0.y, r0.z);
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
			}
		}
		glEnd();
	}
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderGLEdges(GLMesh* pm, int nid)
{
	vec3d r0, r1;
	if (pm == 0) return;
	int N = (int)pm->Edges();
	if (N == 0) return;
	if (nid == -1)
	{
		glBegin(GL_LINES);
		{
			for (int i = 0; i<N; ++i)
			{
				GMesh::EDGE& e = pm->Edge(i);
				if ((e.pid >= 0) && (e.n[0] != -1) && (e.n[1] != -1))
				{
					r0 = pm->Node(e.n[0]).r;
					r1 = pm->Node(e.n[1]).r;
					glVertex3d(r0.x, r0.y, r0.z);
					glVertex3d(r1.x, r1.y, r1.z);
				}
			}
		}
		glEnd();
	}
	else if (nid < (int)pm->m_EIL.size())
	{
		assert(pm->m_EIL.size() > 0);
		glBegin(GL_LINES);
		{
			pair<int, int> eil = pm->m_EIL[nid];

			for (int i = 0; i<eil.second; ++i)
			{
				GMesh::EDGE& e = pm->Edge(i + eil.first);
				assert(e.pid == nid);
				if ((e.n[0] != -1) && (e.n[1] != -1))
				{
					r0 = pm->Node(e.n[0]).r;
					r1 = pm->Node(e.n[1]).r;
					glVertex3d(r0.x, r0.y, r0.z);
					glVertex3d(r1.x, r1.y, r1.z);
				}
			}
		}
		glEnd();
	}
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFENodes(FELineMesh* mesh)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);

	glBegin(GL_POINTS);
	{
		// render unselected nodes first
		glColor3ub(0, 0, 255);
		for (int i = 0; i < mesh->Nodes(); ++i)
		{
			FENode& node = mesh->Node(i);
			if (node.m_ntag)
			{
				if (node.IsSelected() == false)
					glx::vertex3d(node.r);
			}
		}
	}
	glEnd();

	// render selected nodes next
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_POINTS);
	{
		glColor3ub(255, 0, 0);
		for (int i = 0; i < mesh->Nodes(); ++i)
		{
			FENode& node = mesh->Node(i);
			if (node.m_ntag)
			{
				if (node.IsSelected())
					glx::vertex3d(node.r);
			}
		}
	}
	glEnd();

	glPointSize(old_size);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderSelectedFEEdges(FELineMesh* pm)
{
	glBegin(GL_LINES);
	for (int i = 0; i<pm->Edges(); i++)
	{
		FEEdge& edge = pm->Edge(i);
		if (edge.IsSelected() && edge.IsVisible())
		{
			RenderFEEdge(edge, pm);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderUnselectedFEEdges(FELineMesh* pm)
{
	glBegin(GL_LINES);
	for (int i = 0; i<pm->Edges(); i++)
	{
		FEEdge& edge = pm->Edge(i);

		if (!edge.IsSelected() && edge.IsVisible())
		{
			RenderFEEdge(edge, pm);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderMeshLines(FEMeshBase* pm)
{
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// loop over all faces
	glBegin(GL_LINES);
	for (int i = 0; i<pm->Faces(); i++)
	{
		const FEFace& face = pm->Face(i);
		if (face.IsVisible())
		{
			switch (face.Type())
			{
			case FE_FACE_QUAD4:
			{
				const vec3d& r1 = pm->Node(face.n[0]).r;
				const vec3d& r2 = pm->Node(face.n[1]).r;
				const vec3d& r3 = pm->Node(face.n[2]).r;
				const vec3d& r4 = pm->Node(face.n[3]).r;
				glx::lineLoop(r1, r2, r3, r4);
			}
			break;
			case FE_FACE_QUAD8:
			case FE_FACE_QUAD9:
			{
				const vec3d& r1 = pm->Node(face.n[0]).r;
				const vec3d& r2 = pm->Node(face.n[1]).r;
				const vec3d& r3 = pm->Node(face.n[2]).r;
				const vec3d& r4 = pm->Node(face.n[3]).r;
				const vec3d& r5 = pm->Node(face.n[4]).r;
				const vec3d& r6 = pm->Node(face.n[5]).r;
				const vec3d& r7 = pm->Node(face.n[6]).r;
				const vec3d& r8 = pm->Node(face.n[7]).r;
				glx::lineLoop(r1, r5, r2, r6, r3, r7, r4, r8);
			}
			break;
			case FE_FACE_TRI3:
			{
				const vec3d& r1 = pm->Node(face.n[0]).r;
				const vec3d& r2 = pm->Node(face.n[1]).r;
				const vec3d& r3 = pm->Node(face.n[2]).r;
				glx::lineLoop(r1, r2, r3);
			}
			break;
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			{
				const vec3d& r1 = pm->Node(face.n[0]).r;
				const vec3d& r2 = pm->Node(face.n[1]).r;
				const vec3d& r3 = pm->Node(face.n[2]).r;
				const vec3d& r4 = pm->Node(face.n[3]).r;
				const vec3d& r5 = pm->Node(face.n[4]).r;
				const vec3d& r6 = pm->Node(face.n[5]).r;
				glx::lineLoop(r1, r4, r2, r5, r3, r6);
			}
			break;
			case FE_FACE_TRI10:
			{
				vec3d r[9];
				r[0] = pm->Node(face.n[0]).r;
				r[1] = pm->Node(face.n[3]).r;
				r[2] = pm->Node(face.n[4]).r;
				r[3] = pm->Node(face.n[1]).r;
				r[4] = pm->Node(face.n[5]).r;
				r[5] = pm->Node(face.n[6]).r;
				r[6] = pm->Node(face.n[2]).r;
				r[7] = pm->Node(face.n[8]).r;
				r[8] = pm->Node(face.n[7]).r;
				glx::lineLoop(r);
			}
			break;
			} // switch
		} // if
	} // for
	glEnd();

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderSelectedFEFaces(FEMeshBase* pm)
{
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < pm->Faces(); i++)
		{
			FEFace& face = pm->Face(i);
			if (face.IsSelected())
			{
				RenderFEFace(face, pm);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderUnselectedFEFaces(FEMeshBase* pm)
{
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < pm->Faces(); i++)
		{
			FEFace& face = pm->Face(i);
			if (!face.IsSelected() && face.IsVisible())
			{
				RenderFEFace(face, pm);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderSelectedFEFacesOutline(FEMeshBase* pm)
{
	glBegin(GL_LINES);
	for (int i = 0; i < pm->Faces(); i++)
	{
		FEFace& face = pm->Face(i);
		if (face.IsSelected())
		{
			int fn = face.Nodes();
			int ne = face.Edges();
			for (int j = 0; j < ne; ++j)
			{
				FEFace* pf = (face.m_nbr[j] >= 0 ? pm->FacePtr(face.m_nbr[j]) : 0);
				if ((pf == 0) || !pf->IsSelected() || !pf->IsVisible())
				{
					int jp1 = (j + 1) % ne;
					const vec3d& r1 = pm->Node(face.n[j]).r;
					const vec3d& r2 = pm->Node(face.n[jp1]).r;

					glVertex3d(r1.x, r1.y, r1.z);
					glVertex3d(r2.x, r2.y, r2.z);
				}
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFace(FEFace& face, FECoreMesh* pm)
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0].eid).IsShell())
		{
			RenderThickShell(face, pm);
			return;
		}
	}
	if (m_ndivs == 1)
	{
		// Render the facet
		switch (face.m_type)
		{
		case FE_FACE_QUAD4: ::RenderQUAD4(pm, face); break;
		case FE_FACE_QUAD8: ::RenderQUAD8(pm, face); break;
		case FE_FACE_QUAD9: ::RenderQUAD9(pm, face); break;
		case FE_FACE_TRI3 : ::RenderTRI3 (pm, face); break;
		case FE_FACE_TRI6 : ::RenderTRI6 (pm, face); break;
		case FE_FACE_TRI7 : ::RenderTRI7 (pm, face); break;
		case FE_FACE_TRI10: ::RenderTRI10(pm, face); break;
		default:
			assert(false);
		}
	}
	else
	{
		// Render the facet
		switch (face.m_type)
		{
		case FE_FACE_QUAD4: RenderSmoothQUAD4(pm, face, m_ndivs); break;
		case FE_FACE_QUAD8: RenderSmoothQUAD8(pm, face, m_ndivs); break;
		case FE_FACE_QUAD9: RenderSmoothQUAD9(pm, face, m_ndivs); break;
		case FE_FACE_TRI3 : RenderSmoothTRI3 (pm, face, m_ndivs); break;
		case FE_FACE_TRI6 : RenderSmoothTRI6 (pm, face, m_ndivs); break;
		case FE_FACE_TRI7 : RenderSmoothTRI7 (pm, face, m_ndivs); break;
		case FE_FACE_TRI10: RenderSmoothTRI10(pm, face, m_ndivs); break;
		default:
			assert(false);
		}
	}
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFace(FEFace& face, FECoreMesh* pm, GLColor c[4], int ndivs)
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0].eid).IsShell())
		{
			glBegin(GL_TRIANGLES);
			RenderThickShell(face, pm);
			glEnd();
			return;
		}
	}

	vec3d& r1 = pm->Node(face.n[0]).r;
	vec3d& r2 = pm->Node(face.n[1]).r;
	vec3d& r3 = pm->Node(face.n[2]).r;
	vec3d r4 = (face.n[3] >= 0 ? pm->Node(face.n[3]).r : r3);

	vec3f& n1 = face.m_nn[0];
	vec3f& n2 = face.m_nn[1];
	vec3f& n3 = face.m_nn[2];
	vec3f& n4 = face.m_nn[3];

	vec3f& fn = face.m_fn;

	float t[FEFace::MAX_NODES];
	pm->FaceNodeTexCoords(face, t);

	switch (face.m_type)
	{
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		if (ndivs <= 1)
		{
			glBegin(GL_QUADS);
			{
				glNormal3f(n1.x, n1.y, n1.z); glColor4ub(c[0].r, c[0].g, c[0].b, c[0].a); glTexCoord1f(t[0]); glVertex3f(r1.x, r1.y, r1.z);
				glNormal3f(n2.x, n2.y, n2.z); glColor4ub(c[1].r, c[1].g, c[1].b, c[1].a); glTexCoord1f(t[1]); glVertex3f(r2.x, r2.y, r2.z);
				glNormal3f(n3.x, n3.y, n3.z); glColor4ub(c[2].r, c[2].g, c[2].b, c[2].a); glTexCoord1f(t[2]); glVertex3f(r3.x, r3.y, r3.z);
				glNormal3f(n4.x, n4.y, n4.z); glColor4ub(c[3].r, c[3].g, c[3].b, c[3].a); glTexCoord1f(t[3]); glVertex3f(r4.x, r4.y, r4.z);
			}
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
			RenderSmoothQUAD4(pm, face, ndivs);
			glEnd();
		}
		break;
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		glBegin(GL_TRIANGLES);
		{
			glNormal3f(n1.x, n1.y, n1.z); glColor4ub(c[0].r, c[0].g, c[0].b, c[0].a); glTexCoord1f(t[0]); glVertex3f(r1.x, r1.y, r1.z);
			glNormal3f(n2.x, n2.y, n2.z); glColor4ub(c[1].r, c[1].g, c[1].b, c[1].a); glTexCoord1f(t[1]); glVertex3f(r2.x, r2.y, r2.z);
			glNormal3f(n3.x, n3.y, n3.z); glColor4ub(c[2].r, c[2].g, c[2].b, c[2].a); glTexCoord1f(t[2]); glVertex3f(r3.x, r3.y, r3.z);
		}
		glEnd();
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFaceOutline(FEFace& face, FECoreMesh* pm, int ndivs)
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0].eid).IsShell())
		{
			RenderThickShellOutline(face, pm);
			return;
		}
	}

	GLboolean btex;
	glGetBooleanv(GL_TEXTURE_1D, &btex);
	glDisable(GL_TEXTURE_1D);

	glBegin(GL_LINE_LOOP);
	{
		// render the edges of the face
		switch (face.m_type)
		{
		case FE_FACE_TRI3:
		case FE_FACE_QUAD4: RenderFace1Outline(pm, face); break;
		case FE_FACE_TRI6:
		case FE_FACE_TRI7:
		case FE_FACE_QUAD8:
		case FE_FACE_QUAD9: RenderFace2Outline(pm, face, ndivs); break;
		case FE_FACE_TRI10: RenderFace3Outline(pm, face, ndivs); break;
		default:
			assert(false);
		}
	}
	glEnd();

	if (btex) glEnable(GL_TEXTURE_1D);
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFEFace(FEFace& face, FEMeshBase* pm)
{
	switch (face.Type())
	{
	case FE_FACE_TRI3 : ::RenderTRI3 (pm, face); break;
	case FE_FACE_TRI6 : ::RenderTRI6 (pm, face); break;
	case FE_FACE_TRI7 : ::RenderTRI7 (pm, face); break;
	case FE_FACE_TRI10: ::RenderTRI10(pm, face); break;
	case FE_FACE_QUAD4: ::RenderQUAD4(pm, face); break;
	case FE_FACE_QUAD8: ::RenderQUAD8(pm, face); break;
	case FE_FACE_QUAD9: ::RenderQUAD9(pm, face); break;
	default:
		assert(false);
	}
}

///////////////////////////////////////////////////////////////////////////////
void GLMeshRender::RenderThickShell(FEFace &face, FECoreMesh* pm)
{
	switch (face.m_type)
	{
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		RenderThickQuad(face, pm);
		break;
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		RenderThickTri(face, pm);
		break;
	}
}

void GLMeshRender::RenderThickQuad(FEFace &face, FECoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;
	vec3d r4 = pm->Node(face.n[3]).r;

	vec3d n1 = to_vec3d(face.m_nn[0]);
	vec3d n2 = to_vec3d(face.m_nn[1]);
	vec3d n3 = to_vec3d(face.m_nn[2]);
	vec3d n4 = to_vec3d(face.m_nn[3]);

	vec3d r1a, r2a, r3a, r4a;
	vec3d r1b, r2b, r3b, r4b;

	switch (m_nshellref)
	{
	case 0:	// midsurface
		r1a = r1 - n1*(0.5f*h[0]); r1b = r1 + n1*(0.5f*h[0]);
		r2a = r2 - n2*(0.5f*h[1]); r2b = r2 + n2*(0.5f*h[1]);
		r3a = r3 - n3*(0.5f*h[2]); r3b = r3 + n3*(0.5f*h[2]);
		r4a = r4 - n4*(0.5f*h[3]); r4b = r4 + n4*(0.5f*h[3]);
		break;
	case 1: // top surface
		r1a = r1; r1b = r1 + n1*h[0];
		r2a = r2; r2b = r2 + n2*h[1];
		r3a = r3; r3b = r3 + n3*h[2];
		r4a = r4; r4b = r4 + n4*h[3];
		break;
	case 2: // bottom surface
		r1a = r1 - n1*h[0]; r1b = r1;
		r2a = r2 - n2*h[1]; r2b = r2;
		r3a = r3 - n3*h[2]; r3b = r3;
		r4a = r4 - n4*h[3]; r4b = r4;
		break;
	}

	vec3d m1 = (r2 - r1) ^ n1; m1.Normalize();
	vec3d m2 = (r3 - r2) ^ n1; m2.Normalize();
	vec3d m3 = (r4 - r3) ^ n1; m3.Normalize();
	vec3d m4 = (r1 - r4) ^ n1; m4.Normalize();

	vec3f fn = face.m_fn;

	float t1 = face.m_tex[0];
	float t2 = face.m_tex[1];
	float t3 = face.m_tex[2];
	float t4 = face.m_tex[3];

	// top face
	glx::vertex3d(r1b, n1, t1); glx::vertex3d(r2b, n2, t2); glx::vertex3d(r3b, n3, t3);
	glx::vertex3d(r3b, n3, t3); glx::vertex3d(r4b, n4, t4); glx::vertex3d(r1b, n1, t1);

	// bottom face
	glx::vertex3d(r4a, -n4, t4); glx::vertex3d(r3a, -n3, t3); glx::vertex3d(r2a, -n2, t2);
	glx::vertex3d(r2a, -n2, t2); glx::vertex3d(r1a, -n1, t1); glx::vertex3d(r4a, -n4, t4);

	// side faces
	if (face.m_nbr[0] == -1)
	{
		glx::vertex3d(r1a, m1, t1); glx::vertex3d(r2a, m1, t2); glx::vertex3d(r2b, m1, t2);
		glx::vertex3d(r2b, m1, t2); glx::vertex3d(r1b, m1, t1); glx::vertex3d(r1a, m1, t1);
	}

	if (face.m_nbr[1] == -1)
	{
		glx::vertex3d(r2a, m2, t2); glx::vertex3d(r3a, m2, t3); glx::vertex3d(r3b, m2, t3);
		glx::vertex3d(r3b, m2, t3); glx::vertex3d(r2b, m2, t2); glx::vertex3d(r2a, m2, t2);
	}

	if (face.m_nbr[2] == -1)
	{
		glx::vertex3d(r3a, m3, t3); glx::vertex3d(r4a, m3, t4); glx::vertex3d(r4b, m3, t4);
		glx::vertex3d(r4b, m3, t4); glx::vertex3d(r3b, m3, t3); glx::vertex3d(r3a, m3, t3);
	}

	if (face.m_nbr[3] == -1)
	{
		glx::vertex3d(r4a, m4, t4); glx::vertex3d(r1a, m4, t1); glx::vertex3d(r1b, m4, t1);
		glx::vertex3d(r1b, m4, t1); glx::vertex3d(r4b, m4, t4); glx::vertex3d(r4a, m4, t4);
	}
}

void GLMeshRender::RenderThickTri(FEFace &face, FECoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;

	//h[0] = (h[0] + h[1] + h[2])/3;
	//h[1] = h[0];
	//h[2] = h[0];
	vec3d n1 = to_vec3d(face.m_nn[0]);
	vec3d n2 = to_vec3d(face.m_nn[1]);
	vec3d n3 = to_vec3d(face.m_nn[2]);

	vec3d r1a, r2a, r3a;
	vec3d r1b, r2b, r3b;

	switch (m_nshellref)
	{
	case 0:	// midsurface
		r1a = r1 - n1*(0.5f*h[0]); r1b = r1 + n1*(0.5f*h[0]);
		r2a = r2 - n2*(0.5f*h[1]); r2b = r2 + n2*(0.5f*h[1]);
		r3a = r3 - n3*(0.5f*h[2]); r3b = r3 + n3*(0.5f*h[2]);
		break;
	case 1: // top surface
		r1a = r1; r1b = r1 + n1*h[0];
		r2a = r2; r2b = r2 + n2*h[1];
		r3a = r3; r3b = r3 + n3*h[2];
		break;
	case 2: // bottom surface
		r1a = r1 - n1*h[0]; r1b = r1;
		r2a = r2 - n2*h[1]; r2b = r2;
		r3a = r3 - n3*h[2]; r3b = r3;
		break;
	}

	vec3d m1 = (r2 - r1) ^ n1; m1.Normalize();
	vec3d m2 = (r3 - r2) ^ n1; m2.Normalize();
	vec3d m3 = (r1 - r3) ^ n1; m3.Normalize();

	vec3f fn = face.m_fn;

	float t1 = face.m_tex[0];
	float t2 = face.m_tex[1];
	float t3 = face.m_tex[2];

	// top face
	glx::vertex3d(r1b, n1, t1);
	glx::vertex3d(r2b, n2, t2);
	glx::vertex3d(r3b, n3, t3);

	// bottom face
	glx::vertex3d(r3a, -n3, t3);
	glx::vertex3d(r2a, -n2, t2);
	glx::vertex3d(r1a, -n1, t1);

	// side faces
	if (face.m_nbr[0] == -1)
	{
		glx::vertex3d(r1a, m1, t1); glx::vertex3d(r2a, m1, t2); glx::vertex3d(r2b, m1, t2);
		glx::vertex3d(r2b, m1, t2); glx::vertex3d(r1b, m1, t1); glx::vertex3d(r1a, m1, t1);
	}

	if (face.m_nbr[1] == -1)
	{
		glx::vertex3d(r2a, m2, t2); glx::vertex3d(r3a, m2, t3); glx::vertex3d(r3b, m2, t3);
		glx::vertex3d(r3b, m2, t3); glx::vertex3d(r2b, m2, t2); glx::vertex3d(r2a, m2, t2);
	}

	if (face.m_nbr[2] == -1)
	{
		glx::vertex3d(r3a, m3, t3); glx::vertex3d(r1a, m3, t1); glx::vertex3d(r1b, m3, t1);
		glx::vertex3d(r1b, m3, t1); glx::vertex3d(r3b, m3, t3); glx::vertex3d(r3a, m3, t3);
	}
}

void GLMeshRender::RenderThickShellOutline(FEFace &face, FECoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;
	vec3d r4 = pm->Node(face.n[3]).r;

	vec3d n1 = to_vec3d(face.m_nn[0]);
	vec3d n2 = to_vec3d(face.m_nn[1]);
	vec3d n3 = to_vec3d(face.m_nn[2]);
	vec3d n4 = to_vec3d(face.m_nn[3]);

	vec3d r1a, r2a, r3a, r4a;
	vec3d r1b, r2b, r3b, r4b;

	switch (m_nshellref)
	{
	case 0:	// midsurface
		r1a = r1 - n1*(0.5f*h[0]); r1b = r1 + n1*(0.5f*h[0]);
		r2a = r2 - n2*(0.5f*h[1]); r2b = r2 + n2*(0.5f*h[1]);
		r3a = r3 - n3*(0.5f*h[2]); r3b = r3 + n3*(0.5f*h[2]);
		r4a = r4 - n4*(0.5f*h[3]); r4b = r4 + n4*(0.5f*h[3]);
		break;
	case 1: // top surface
		r1a = r1; r1b = r1 + n1*h[0];
		r2a = r2; r2b = r2 + n2*h[1];
		r3a = r3; r3b = r3 + n3*h[2];
		r4a = r4; r4b = r4 + n4*h[3];
		break;
	case 2: // bottom surface
		r1a = r1 - n1*h[0]; r1b = r1;
		r2a = r2 - n2*h[1]; r2b = r2;
		r3a = r3 - n3*h[2]; r3b = r3;
		r4a = r4 - n4*h[3]; r4b = r4;
		break;
	}

	GLboolean btex;
	glGetBooleanv(GL_TEXTURE_1D, &btex);
	glDisable(GL_TEXTURE_1D);

	switch (face.m_type)
	{
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		glBegin(GL_LINES);
		{
			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r2a.x, r2a.y, r2a.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r3a.x, r3a.y, r3a.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r4a.x, r4a.y, r4a.z);
			glVertex3f(r4a.x, r4a.y, r4a.z); glVertex3f(r1a.x, r1a.y, r1a.z);

			glVertex3f(r1b.x, r1b.y, r1b.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r2b.x, r2b.y, r2b.z); glVertex3f(r3b.x, r3b.y, r3b.z);
			glVertex3f(r3b.x, r3b.y, r3b.z); glVertex3f(r4b.x, r4b.y, r4b.z);
			glVertex3f(r4b.x, r4b.y, r4b.z); glVertex3f(r1b.x, r1b.y, r1b.z);

			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r1b.x, r1b.y, r1b.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r3b.x, r3b.y, r3b.z);
			glVertex3f(r4a.x, r4a.y, r4a.z); glVertex3f(r4b.x, r4b.y, r4b.z);
		}
		glEnd();
		break;
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		glBegin(GL_LINES);
		{
			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r2a.x, r2a.y, r2a.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r3a.x, r3a.y, r3a.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r1a.x, r1a.y, r1a.z);

			glVertex3f(r1b.x, r1b.y, r1b.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r2b.x, r2b.y, r2b.z); glVertex3f(r3b.x, r3b.y, r3b.z);
			glVertex3f(r3b.x, r3b.y, r3b.z); glVertex3f(r1b.x, r1b.y, r1b.z);

			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r1b.x, r1b.y, r1b.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r3b.x, r3b.y, r3b.z);
		}
		glEnd();
		break;
	default:
		assert(false);
	}

	if (btex) glEnable(GL_TEXTURE_1D);
}

//-----------------------------------------------------------------------------
// Assumes that we are inside glBegin(GL_LINES)\glEnd()
void RenderFEEdge(FEEdge& edge, FELineMesh* pm)
{
	const vec3d& r1 = pm->Node(edge.n[0]).r;
	const vec3d& r2 = pm->Node(edge.n[1]).r;

	switch (edge.Nodes())
	{
	case 2:
	{
		glx::vertex3d(r1);
		glx::vertex3d(r2);
	}
	break;
	case 3:
	{
		const vec3d& r3 = pm->Node(edge.n[2]).r;
		glx::vertex3d(r1); glx::vertex3d(r3);
		glx::vertex3d(r3); glx::vertex3d(r2);
	}
	break;
	case 4:
	{
		const vec3d& r3 = pm->Node(edge.n[2]).r;
		const vec3d& r4 = pm->Node(edge.n[3]).r;
		glx::vertex3d(r1); glx::vertex3d(r3);
		glx::vertex3d(r3); glx::vertex3d(r4);
		glx::vertex3d(r4); glx::vertex3d(r2);
	}
	break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void RenderQUAD4(FEMeshBase* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_QUAD4);

	// get the nodal data
	vec3d r[4]; pm->FaceNodePosition(f, r);

	vec3f n[4];
	pm->FaceNodeNormals(f, n);

	float t[4];
	pm->FaceNodeTexCoords(f, t);

	// render the quads
	glx::quad4(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 8-noded quad
void RenderQUAD8(FEMeshBase* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_QUAD8);

	// get the nodal data
	vec3d r[8]; pm->FaceNodePosition(f, r);

	vec3f n[8];
	pm->FaceNodeNormals(f, n);

	float t[8];
	pm->FaceNodeTexCoords(f, t);

	glx::quad8(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 9-noded quad
void RenderQUAD9(FEMeshBase* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_QUAD9);

	// get the nodal data
	vec3d r[9]; pm->FaceNodePosition(f, r);

	vec3f n[9];
	pm->FaceNodeNormals(f, n);

	float t[9];
	pm->FaceNodeTexCoords(f, t);

	glx::quad9(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 3-noded tri
void RenderTRI3(FEMeshBase* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI3);

	// get the nodal data
	vec3d r[3]; pm->FaceNodePosition(f, r);

	vec3f n[3];
	pm->FaceNodeNormals(f, n);

	float t[3];
	pm->FaceNodeTexCoords(f, t);

	glx::tri3(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 6-noded tri
void RenderTRI6(FEMeshBase* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI6);

	// get the nodal data
	vec3d r[6]; pm->FaceNodePosition(f, r);

	vec3f n[6];
	pm->FaceNodeNormals(f, n);

	float t[6];
	pm->FaceNodeTexCoords(f, t);

	glx::tri6(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 7-noded tri
void RenderTRI7(FEMeshBase* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI7);

	// get the nodal data
	vec3d r[7]; pm->FaceNodePosition(f, r);

	vec3f n[7];
	pm->FaceNodeNormals(f, n);

	float t[7];
	pm->FaceNodeTexCoords(f, t);

	glx::tri7(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 10-noded tri
void RenderTRI10(FEMeshBase* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI10);

	// get the nodal data
	vec3d r[10]; pm->FaceNodePosition(f, r);

	vec3f n[10];
	pm->FaceNodeNormals(f, n);

	float t[10];
	pm->FaceNodeTexCoords(f, t);

	glx::tri10(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 4-noded quadrilateral
void RenderSmoothQUAD4(FECoreMesh* pm, FEFace& face, int ndivs)
{
	vec3d r[4] = {
		pm->Node(face.n[0]).r,
		pm->Node(face.n[1]).r,
		pm->Node(face.n[2]).r,
		pm->Node(face.n[3]).r
	};

	vec3f n[4] = { face.m_nn[0], face.m_nn[1], face.m_nn[2], face.m_nn[3] };

	float t[4];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothQUAD4(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 8-noded quadrilateral
void RenderSmoothQUAD8(FECoreMesh* pm, FEFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_QUAD8);

	vec3d r[8];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;
	r[7] = pm->Node(face.n[7]).r;

	vec3f n[8];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];

	float t[8];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothQUAD8(r, n, t, ndivs);
}


//-----------------------------------------------------------------------------
// Render a sub-divided 9-noded quadrilateral
void RenderSmoothQUAD9(FECoreMesh* pm, FEFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_QUAD9);

	vec3d r[9];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;
	r[7] = pm->Node(face.n[7]).r;
	r[8] = pm->Node(face.n[8]).r;

	vec3f n[9];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];
	n[8] = face.m_nn[8];

	float t[9];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothQUAD9(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 6-noded triangle
void RenderSmoothTRI3(FECoreMesh* pm, FEFace& face, int ndivs)
{
	RenderTRI3(pm, face);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 6-noded triangle
void RenderSmoothTRI6(FECoreMesh* pm, FEFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_TRI6);

	vec3d r[6];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;

	vec3f n[6];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];

	float t[6];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothTRI6(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 7-noded triangle
void RenderSmoothTRI7(FECoreMesh* pm, FEFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_TRI7);

	vec3d r[7];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;

	vec3f n[7];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];

	float t[7];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothTRI7(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 10-noded triangle
void RenderSmoothTRI10(FECoreMesh* pm, FEFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_TRI10);

	vec3d r[10];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;
	r[7] = pm->Node(face.n[7]).r;
	r[8] = pm->Node(face.n[8]).r;
	r[9] = pm->Node(face.n[9]).r;

	vec3f n[10];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];
	n[8] = face.m_nn[8];
	n[9] = face.m_nn[9];

	float t[10];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothTRI10(r, n, t, ndivs);
}

void RenderFace1Outline(FECoreMesh* pm, FEFace& face)
{
	int N = face.Nodes();
	for (int i = 0; i < N; ++i)
	{
		vec3d r = pm->Node(face.n[i]).r; glVertex3f(r.x, r.y, r.z);
	}
}

void RenderFace2Outline(FECoreMesh* pm, FEFace& face, int ndivs)
{
	vec3f a[3];
	int NE = face.Edges();
	for (int i = 0; i<NE; ++i)
	{
		FEEdge e = face.GetEdge(i);
		a[0] = to_vec3f(pm->Node(e.n[0]).r);
		a[1] = to_vec3f(pm->Node(e.n[1]).r);
		a[2] = to_vec3f(pm->Node(e.n[2]).r);
		const int M = (ndivs < 2 ? 2 : ndivs);
		for (int n = 0; n<M; ++n)
		{
			double t = n / (double)M;
			vec3f p = e.eval(a, t);
			glVertex3d(p.x, p.y, p.z);
		}
	}
}

void RenderFace3Outline(FECoreMesh* pm, FEFace& face, int ndivs)
{
	vec3f a[4];
	int NE = face.Edges();
	for (int i = 0; i<NE; ++i)
	{
		FEEdge e = face.GetEdge(i);
		a[0] = to_vec3f(pm->Node(e.n[0]).r);
		a[1] = to_vec3f(pm->Node(e.n[1]).r);
		a[2] = to_vec3f(pm->Node(e.n[2]).r);
		a[3] = to_vec3f(pm->Node(e.n[3]).r);
		const int M = (ndivs < 2 ? 3 : ndivs);
		for (int n = 0; n<M; ++n)
		{
			double t = n / (double)M;
			vec3f p = e.eval(a, t);
			glVertex3f(p.x, p.y, p.z);
		}
	}
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderElementOutline(FEElement_& el, FECoreMesh* pm, int ndivs)
{
	glBegin(GL_LINES);
	switch (el.Type())
	{
	case FE_HEX8:
	{
		int(*et)[2] = ET_HEX;
		for (int i = 0; i<12; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			glx::vertex3d(r0);
			glx::vertex3d(r1);
		}
	}
	break;
	case FE_PYRA5:
	{
		int(*et)[2] = ET_PYRA5;
		for (int i = 0; i<8; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			glx::vertex3d(r0);
			glx::vertex3d(r1);
		}
	}
	break;
    case FE_PYRA13:
        {
            int(*et)[3] = ET_PYRA13;
            for (int i = 0; i<8; ++i)
            {
                vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
                vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
                vec3d& r2 = pm->Node(el.m_node[et[i][2]]).r;
                glx::vertex3d(r0); glx::vertex3d(r2);
                glx::vertex3d(r2); glx::vertex3d(r1);
            }
        }
            break;
	case FE_HEX20:
	{
		int(*et)[3] = ET_HEX20;
		for (int i = 0; i<12; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			vec3d& r2 = pm->Node(el.m_node[et[i][2]]).r;
			glx::vertex3d(r0); glx::vertex3d(r2);
			glx::vertex3d(r2); glx::vertex3d(r1);
		}
	}
	break;
	case FE_HEX27:
	{
		int(*et)[3] = ET_HEX20;
		for (int i = 0; i<12; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			vec3d& r2 = pm->Node(el.m_node[et[i][2]]).r;
			glx::vertex3d(r0); glx::vertex3d(r2);
			glx::vertex3d(r2); glx::vertex3d(r1);
		}
	}
	break;
	case FE_PENTA6:
	{
		int(*et)[2] = ET_PENTA;
		for (int i = 0; i<9; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			glx::vertex3d(r0);
			glx::vertex3d(r1);
		}
	};
	break;
	case FE_PENTA15:
	{
		int(*et)[3] = ET_PENTA15;
		for (int i = 0; i<9; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			vec3d& r2 = pm->Node(el.m_node[et[i][2]]).r;
			glx::vertex3d(r0); glx::vertex3d(r2);
			glx::vertex3d(r2); glx::vertex3d(r1);
		}
	};
	break;
	case FE_TET4:
	case FE_TET5:
	case FE_TET20:
	{
		int(*et)[2] = ET_TET;
		for (int i = 0; i<6; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			glx::vertex3d(r0);
			glx::vertex3d(r1);
		}
	}
	break;
	case FE_TET10:
	case FE_TET15:
	{
		FEEdge edge;
		edge.SetType(FE_EDGE3);
		vec3f a[3];
		int(*et)[3] = ET_TET10;
		for (int i = 0; i<6; ++i)
		{
			vec3d& r0 = pm->Node(el.m_node[et[i][0]]).r;
			vec3d& r1 = pm->Node(el.m_node[et[i][1]]).r;
			vec3d& r2 = pm->Node(el.m_node[et[i][2]]).r;

			vec3f p = to_vec3f(r0);
			a[0] = to_vec3f(r0);
			a[1] = to_vec3f(r1);
			a[2] = to_vec3f(r2);
			const int M = (ndivs < 2 ? 2 : ndivs);
			for (int n = 1; n<=M; ++n)
			{
				double t = n / (double)M;
				vec3f q = edge.eval(a, t);
				glVertex3d(p.x, p.y, p.z);
				glVertex3d(q.x, q.y, q.z);
				p = q;
			}
		}
	}
	break;
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFaceEdge(FEFace& f, int j, FEMeshBase* pm, int ndivs)
{
	int n = f.Edges();
	int a = f.n[j];
	int b = f.n[(j + 1) % n];
	if (a > b) { a ^= b; b ^= a; a ^= b; }

	glBegin(GL_LINES);
	switch (f.m_type)
	{
	case FE_FACE_TRI3:
	case FE_FACE_QUAD4:
	{
		vec3d r1 = pm->Node(a).r;
		vec3d r2 = pm->Node(b).r;
		glVertex3d(r1.x, r1.y, r1.z);
		glVertex3d(r2.x, r2.y, r2.z);
	}
	break;
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
	{
		vec3d r1 = pm->Node(a).r;
		vec3d r2 = pm->Node(b).r;
		vec3d r3 = pm->Node(f.n[j + 4]).r;

		float r, H[3];
		vec3d p;
		int n = (ndivs <= 1 ? 2 : ndivs);
		for (int i = 0; i<n; ++i)
		{
			r = -1.f + 2.f*i / n;
			H[0] = 0.5f*r*(r - 1.f);
			H[1] = 0.5f*r*(r + 1.f);
			H[2] = 1.f - r*r;
			p = r1*H[0] + r2*H[1] + r3*H[2];
			glVertex3f(p.x, p.y, p.z);

			r = -1.f + 2.f*(i + 1) / n;
			H[0] = 0.5f*r*(r - 1.f);
			H[1] = 0.5f*r*(r + 1.f);
			H[2] = 1.f - r*r;
			p = r1*H[0] + r2*H[1] + r3*H[2];
			glVertex3f(p.x, p.y, p.z);
		}
	}
	break;
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	{
		vec3d r1 = pm->Node(a).r;
		vec3d r2 = pm->Node(b).r;
		vec3d r3 = pm->Node(f.n[j + 3]).r;

		float r, H[3];
		vec3d p;
		int n = (ndivs <= 1 ? 2 : ndivs);
		for (int i = 0; i<n; ++i)
		{
			r = -1.f + 2.f*i / n;
			H[0] = 0.5f*r*(r - 1.f);
			H[1] = 0.5f*r*(r + 1.f);
			H[2] = 1.f - r*r;
			p = r1*H[0] + r2*H[1] + r3*H[2];
			glVertex3f(p.x, p.y, p.z);

			r = -1.f + 2.f*(i + 1) / n;
			H[0] = 0.5f*r*(r - 1.f);
			H[1] = 0.5f*r*(r + 1.f);
			H[2] = 1.f - r*r;
			p = r1*H[0] + r2*H[1] + r3*H[2];
			glVertex3f(p.x, p.y, p.z);
		}
	}
	break;
	case FE_FACE_TRI10:
	{
		FEEdge e = f.GetEdge(j);

		vec3f r[4];
		r[0] = to_vec3f(pm->Node(e.n[0]).r);
		r[1] = to_vec3f(pm->Node(e.n[1]).r);
		r[2] = to_vec3f(pm->Node(e.n[2]).r);
		r[3] = to_vec3f(pm->Node(e.n[3]).r);

		vec3f p = r[0];
		int n = (ndivs < 3 ? 3 : ndivs);
		for (int i = 1; i<=n; ++i)
		{
			float w = (float) i / n;
			vec3f q = e.eval(r, w);
			glVertex3f(p.x, p.y, p.z);
			glVertex3f(q.x, q.y, q.z);
			p = q;
		}	
	}
	break;
	default:
		assert(false);
	}
	glEnd();
}
