#include "GLMeshRender.h"
#include <MeshLib/FECoreMesh.h>
#include <MeshLib/FEElement.h>
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/quad8.h>
#include <MeshTools/GLMesh.h>
#include <GLLib/glx.h>

GLMeshRender::GLMeshRender()
{
	m_bShell2Solid = false;
	m_nshellref = 0;
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
	vec3d n1, n2, n3, n4;
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
				assert(&pm->ElementRef(pf->m_elem[0]) == pe);
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
					n1 = n2 = n3 = n4 = n;
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = n;
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
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

inline void glxVertex(const vec3d& r)
{
	glVertex3d(r.x, r.y, r.z);
}

inline void glxNormal(const vec3d& n)
{
	glNormal3d(n.x, n.y, n.z);
}

inline void glxNormalVertex(const vec3d& n, const vec3d& r)
{
	glNormal3d(n.x, n.y, n.z);
	glVertex3d(r.x, r.y, r.z);
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
	vec3d n[9];
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
					n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = n[7] = n[8] = nn;
				}
			}
			else
			{
				vec3d nn = (r[1] - r[0]) ^ (r[2] - r[0]);
				nn.Normalize();
				n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = n[7] = n[8] = nn;
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				glxNormalVertex(n[0], r[0]);
				glxNormalVertex(n[4], r[4]);
				glxNormalVertex(n[7], r[7]);

				glxNormalVertex(n[4], r[4]);
				glxNormalVertex(n[1], r[1]);
				glxNormalVertex(n[5], r[5]);

				glxNormalVertex(n[5], r[5]);
				glxNormalVertex(n[2], r[2]);
				glxNormalVertex(n[6], r[6]);

				glxNormalVertex(n[6], r[6]);
				glxNormalVertex(n[3], r[3]);
				glxNormalVertex(n[7], r[7]);

				glxNormalVertex(n[8], r[8]);
				glxNormalVertex(n[7], r[7]);
				glxNormalVertex(n[4], r[4]);

				glxNormalVertex(n[8], r[8]);
				glxNormalVertex(n[4], r[4]);
				glxNormalVertex(n[5], r[5]);

				glxNormalVertex(n[8], r[8]);
				glxNormalVertex(n[5], r[5]);
				glxNormalVertex(n[6], r[6]);

				glxNormalVertex(n[8], r[8]);
				glxNormalVertex(n[6], r[6]);
				glxNormalVertex(n[7], r[7]);
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
	vec3d n1, n2, n3, n4, n5, n6, n7, n8;
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
					n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = n;
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = n;
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				GLX::vertex3d(r1, n1);
				GLX::vertex3d(r5, n5);
				GLX::vertex3d(r8, n8);

				GLX::vertex3d(r5, n5);
				GLX::vertex3d(r2, n2);
				GLX::vertex3d(r6, n6);

				GLX::vertex3d(r6, n6);
				GLX::vertex3d(r3, n3);
				GLX::vertex3d(r7, n7);

				GLX::vertex3d(r7, n7);
				GLX::vertex3d(r4, n4);
				GLX::vertex3d(r8, n8);

				GLX::vertex3d(r5, n5);
				GLX::vertex3d(r6, n6);
				GLX::vertex3d(r8, n8);

				GLX::vertex3d(r6, n6);
				GLX::vertex3d(r7, n7);
				GLX::vertex3d(r8, n8);
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
				assert(pm->ElementPtr(pf->m_elem[0]) == pe);
				n[0] = pf->m_nn[0];
				n[1] = pf->m_nn[1];
				n[2] = pf->m_nn[2];
				n[3] = pf->m_nn[3];
			}
			else
			{
				vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
				fn.Normalize();
				n[0] = n[1] = n[2] = n[3] = fn;
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				GLX::quad4(r, n);
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
				GLX::tri3(r, n);
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
	vec3d n1, n2, n3, n4, n5, n6, n7, n8;
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
					n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = n;
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = n;
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
					n1 = n2 = n3 = n4 = n5 = n6 = n;
				}
			}
			else
			{
				vec3d n = (r2 - r1) ^ (r3 - r1);
				n.Normalize();
				n1 = n2 = n3 = n4 = n5 = n6 = n;
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
	vec3d n1, n2, n3;
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
				n1 = n2 = n3 = n;

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
void GLMeshRender::RenderTET10(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TET10));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3d n1, n2, n3;
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
				n1 = n2 = n3 = n;
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
void GLMeshRender::RenderTET15(FEElement_ *pe, FECoreMesh *pm, bool bsel)
{
	assert(pe->IsType(FE_TET15));
	FEElement_& e = *pe;
	vec3d r1, r2, r3;
	vec3d n1, n2, n3;
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
				n1 = n2 = n3 = n;
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
	vec3d n1, n2, n3;
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
				n1 = n2 = n3 = n;
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

	n[0] = pf->m_nn[0];
	n[1] = pf->m_nn[1];
	n[2] = pf->m_nn[2];
	n[3] = pf->m_nn[3];

	glBegin(GL_TRIANGLES);
	{
		GLX::quad4(r, n);
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

	n[0] = pf->m_nn[0];
	n[1] = pf->m_nn[1];
	n[2] = pf->m_nn[2];
	n[3] = pf->m_nn[3];

	glBegin(GL_TRIANGLES);
	{
		GLX::quad4(r, n);
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

	n[0] = pf->m_nn[0];
	n[1] = pf->m_nn[1];
	n[2] = pf->m_nn[2];
	n[3] = pf->m_nn[3];

	glBegin(GL_TRIANGLES);
	{
		GLX::quad4(r, n);
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
		GLX::tri3(r, n);
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
		GLX::tri3(r, n);
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
				n[0] = pf->m_nn[0];
				n[1] = pf->m_nn[1];
				n[2] = pf->m_nn[2];
			}
			else
			{
				vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
				fn.Normalize();
				n[0] = n[1] = n[2] = fn;
			}

			if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
			{
				GLX::vertex3d(r[0], n[0]);
				GLX::vertex3d(r[1], n[1]);
				GLX::vertex3d(r[2], n[2]);
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
			assert(pm->ElementPtr(pf->m_elem[0]) == pe);
			n[0] = pf->m_nn[0];
			n[1] = pf->m_nn[1];
			n[2] = pf->m_nn[2];
			n[3] = pf->m_nn[3];
		}
		else
		{
			vec3d fn = (r[1] - r[0]) ^ (r[2] - r[0]);
			fn.Normalize();
			n[0] = n[1] = n[2] = n[3] = fn;
		}

		if ((pen == 0) || (!pen->IsVisible()) || (pen->IsSelected() && bsel))
		{
			GLX::quad4(r, n);
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

				glNormal3d(n0.x, n0.y, n0.z); glVertex3d(r0.x, r0.y, r0.z);
				glNormal3d(n1.x, n1.y, n1.z); glVertex3d(r1.x, r1.y, r1.z);
				glNormal3d(n2.x, n2.y, n2.z); glVertex3d(r2.x, r2.y, r2.z);
			}
		}
		glEnd();
	}
	else
	{
		assert(pm->m_FIL.size() > 0);
		glBegin(GL_TRIANGLES);
		{
			assert((nid >= 0) && (nid < (int)pm->m_FIL.size()));
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
	else
	{
		assert(pm->m_EIL.size() > 0);
		glBegin(GL_LINES);
		{
			assert((nid >= 0) && (nid < (int)pm->m_EIL.size()));
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

void GLMeshRender::RenderFace(FEFace& face, FECoreMesh* pm, int ndivs)
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0]).IsShell())
		{
			RenderThickShell(face, pm);
			return;
		}
	}

	glBegin(GL_TRIANGLES);
	if (ndivs == 1)
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
		case FE_FACE_QUAD4: RenderSmoothQUAD4(pm, face, ndivs); break;
		case FE_FACE_QUAD8: RenderSmoothQUAD8(pm, face, ndivs); break;
		case FE_FACE_QUAD9: RenderSmoothQUAD9(pm, face, ndivs); break;
		case FE_FACE_TRI3 : RenderSmoothTRI3(pm, face, ndivs); break;
		case FE_FACE_TRI6 : RenderSmoothTRI6(pm, face, ndivs); break;
		case FE_FACE_TRI7 : RenderSmoothTRI7(pm, face, ndivs); break;
		case FE_FACE_TRI10: RenderSmoothTRI10(pm, face, ndivs); break;
		default:
			assert(false);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFace(FEFace& face, FECoreMesh* pm, GLColor c[4], int ndivs)
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0]).IsShell())
		{
			RenderThickShell(face, pm);
			return;
		}
	}

	vec3d& r1 = pm->Node(face.n[0]).r;
	vec3d& r2 = pm->Node(face.n[1]).r;
	vec3d& r3 = pm->Node(face.n[2]).r;
	vec3d& r4 = pm->Node(face.n[3]).r;

	vec3f& n1 = face.m_nn[0];
	vec3f& n2 = face.m_nn[1];
	vec3f& n3 = face.m_nn[2];
	vec3f& n4 = face.m_nn[3];

	vec3f& fn = face.m_fn;

	float t[4];
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
		if (pm->ElementRef(face.m_elem[0]).IsShell())
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
		RenderThickTri(face, pm);
		break;
	}
}

void GLMeshRender::RenderThickQuad(FEFace &face, FECoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0]);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;
	vec3d r4 = pm->Node(face.n[3]).r;

	vec3f n1 = face.m_nn[0];
	vec3f n2 = face.m_nn[1];
	vec3f n3 = face.m_nn[2];
	vec3f n4 = face.m_nn[3];

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

	glBegin(GL_QUADS);
	{
		glNormal3f(n1.x, n1.y, n1.z); glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
		glNormal3f(n2.x, n2.y, n2.z); glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
		glNormal3f(n3.x, n3.y, n3.z); glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
		glNormal3f(n4.x, n4.y, n4.z); glTexCoord1f(t4); glVertex3f(r4b.x, r4b.y, r4b.z);

		glNormal3f(-n4.x, -n4.y, -n4.z); glTexCoord1f(t4); glVertex3f(r4a.x, r4a.y, r4a.z);
		glNormal3f(-n3.x, -n3.y, -n3.z); glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
		glNormal3f(-n2.x, -n2.y, -n2.z); glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
		glNormal3f(-n1.x, -n1.y, -n1.z); glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);

		if (face.m_nbr[0] == -1)
		{
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
		}

		if (face.m_nbr[1] == -1)
		{
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
		}

		if (face.m_nbr[2] == -1)
		{
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t4); glVertex3f(r4a.x, r4a.y, r4a.z);
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t4); glVertex3f(r4b.x, r4b.y, r4b.z);
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
		}

		if (face.m_nbr[3] == -1)
		{
			glNormal3f(m4.x, m4.y, m4.z); glTexCoord1f(t4); glVertex3f(r4a.x, r4a.y, r4a.z);
			glNormal3f(m4.x, m4.y, m4.z); glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
			glNormal3f(m4.x, m4.y, m4.z); glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
			glNormal3f(m4.x, m4.y, m4.z); glTexCoord1f(t4); glVertex3f(r4b.x, r4b.y, r4b.z);
		}
	}
	glEnd();
}

void GLMeshRender::RenderThickTri(FEFace &face, FECoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0]);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;

	//h[0] = (h[0] + h[1] + h[2])/3;
	//h[1] = h[0];
	//h[2] = h[0];
	vec3f n1 = face.m_nn[0];
	vec3f n2 = face.m_nn[1];
	vec3f n3 = face.m_nn[2];

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

	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n1.x, n1.y, n1.z); glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
		glNormal3f(n2.x, n2.y, n2.z); glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
		glNormal3f(n3.x, n3.y, n3.z); glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);

		glNormal3f(-n3.x, -n3.y, -n3.z); glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
		glNormal3f(-n2.x, -n2.y, -n2.z); glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
		glNormal3f(-n1.x, -n1.y, -n1.z); glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
	}
	glEnd();

	glBegin(GL_QUADS);
	{
		if (face.m_nbr[0] == -1)
		{
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
			glNormal3f(m1.x, m1.y, m1.z); glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
		}

		if (face.m_nbr[1] == -1)
		{
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
			glNormal3f(m2.x, m2.y, m2.z); glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
		}

		if (face.m_nbr[2] == -1)
		{
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
			glNormal3f(m3.x, m3.y, m3.z); glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
		}
	}
	glEnd();
}

void GLMeshRender::RenderThickShellOutline(FEFace &face, FECoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0]);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;
	vec3d r4 = pm->Node(face.n[3]).r;

	vec3f n1 = face.m_nn[0];
	vec3f n2 = face.m_nn[1];
	vec3f n3 = face.m_nn[2];
	vec3f n4 = face.m_nn[3];

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
void RenderQUAD4(FECoreMesh* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_QUAD4);

	// get the nodal data
	vec3d r[4]; pm->FaceNodePosition(f, r);

	vec3f n[4];
	pm->FaceNodeNormals(f, n);

	float t[4];
	pm->FaceNodeTexCoords(f, t);

	// render the quads
	GLX::quad4(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 8-noded quad
void RenderQUAD8(FECoreMesh* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_QUAD8);

	// get the nodal data
	vec3d r[8]; pm->FaceNodePosition(f, r);

	vec3f n[8];
	pm->FaceNodeNormals(f, n);

	float t[8];
	pm->FaceNodeTexCoords(f, t);

	GLX::quad8(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 9-noded quad
void RenderQUAD9(FECoreMesh* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_QUAD9);

	// get the nodal data
	vec3d r[9]; pm->FaceNodePosition(f, r);

	vec3f n[9];
	pm->FaceNodeNormals(f, n);

	float t[9];
	pm->FaceNodeTexCoords(f, t);

	GLX::quad9(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 3-noded tri
void RenderTRI3(FECoreMesh* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI3);

	// get the nodal data
	vec3d r[3]; pm->FaceNodePosition(f, r);

	vec3f n[3];
	pm->FaceNodeNormals(f, n);

	float t[3];
	pm->FaceNodeTexCoords(f, t);

	GLX::tri3(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 6-noded tri
void RenderTRI6(FECoreMesh* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI6);

	// get the nodal data
	vec3d r[6]; pm->FaceNodePosition(f, r);

	vec3f n[6];
	pm->FaceNodeNormals(f, n);

	float t[6];
	pm->FaceNodeTexCoords(f, t);

	GLX::tri6(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 7-noded tri
void RenderTRI7(FECoreMesh* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI7);

	// get the nodal data
	vec3d r[7]; pm->FaceNodePosition(f, r);

	vec3f n[7];
	pm->FaceNodeNormals(f, n);

	float t[7];
	pm->FaceNodeTexCoords(f, t);

	GLX::tri7(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 10-noded tri
void RenderTRI10(FECoreMesh* pm, FEFace& f)
{
	assert(f.m_type == FE_FACE_TRI10);

	// get the nodal data
	vec3d r[10]; pm->FaceNodePosition(f, r);

	vec3f n[10];
	pm->FaceNodeNormals(f, n);

	float t[10];
	pm->FaceNodeTexCoords(f, t);

	GLX::tri10(r, n, t);
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

	GLX::smoothQUAD4(r, n, t, ndivs);
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

	GLX::smoothQUAD8(r, n, t, ndivs);
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

	GLX::smoothQUAD9(r, n, t, ndivs);
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

	GLX::smoothTRI6(r, n, t, ndivs);
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

	GLX::smoothTRI7(r, n, t, ndivs);
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

	GLX::smoothTRI10(r, n, t, ndivs);
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
		const int M = 2 * ndivs;
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
		const int M = 2 * ndivs;
		for (int n = 0; n<M; ++n)
		{
			double t = n / (double)M;
			vec3d p = e.eval(a, t);
			glVertex3f(p.x, p.y, p.z);
		}
	}
}
