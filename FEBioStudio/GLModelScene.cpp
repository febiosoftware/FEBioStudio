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
#include "GLModelScene.h"
#include "GLView.h"
#include "ModelDocument.h"
#include <MeshTools/GModel.h>
#include <GeomLib/GObject.h>
#include <GLLib/glx.h>
#include <GLLib/GLMeshRender.h>
#include <FEMLib/FEModelConstraint.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshLib/MeshMetrics.h>

const int HEX_NT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int PEN_NT[8] = { 0, 1, 2, 2, 3, 4, 5, 5 };
const int TET_NT[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
const int PYR_NT[8] = { 0, 1, 2, 3, 4, 4, 4, 4 };

// in MeshTools\lut.cpp
extern int LUT[256][15];
extern int ET_HEX[12][2];
extern int ET_TET[6][2];
extern int ET_PYR[8][2];

static GLColor fiberColorPalette[GMaterial::MAX_COLORS] = {
	GLColor(255, 128, 128),
	GLColor(128, 255, 128),
	GLColor(128, 128, 255),
	GLColor(  0, 240, 240),
	GLColor(240, 180,   0),
	GLColor(240,   0, 180),
	GLColor(180, 240,   0),
	GLColor(  0, 240, 180),
	GLColor(180,   0, 240),
	GLColor(  0, 180, 240),
	GLColor(  0, 180,   0),
	GLColor(  0,   0, 180),
	GLColor(180, 180,   0),
	GLColor(  0, 180, 180),
	GLColor(180,   0, 180),
	GLColor(120,   0, 240)
};

CGLModelScene::CGLModelScene(CModelDocument* doc) : m_doc(doc)
{

}

void CGLModelScene::Render(CGLContext& rc)
{
	if (m_doc == nullptr) return;

	CGLView* glview = rc.m_view; assert(glview);
	if (glview == nullptr) return;

	// We don't need this for rendering model docs
	glDisable(GL_COLOR_MATERIAL);

	VIEW_SETTINGS& view = glview->GetViewSettings();
	int nitem = m_doc->GetItemMode();

	CGLCamera& cam = *rc.m_cam;

	if (glview->ShowPlaneCut())
	{
		GLMesh* planecut = glview->PlaneCutMesh();
		if (planecut == nullptr) glview->UpdatePlaneCut();
		if (glview->PlaneCutMode() == 0)
		{
			// render the plane cut first
			glview->RenderPlaneCut();

			// then turn on the clipping plane before rendering the other geometry
			glClipPlane(GL_CLIP_PLANE0, glview->PlaneCoordinates());
			glEnable(GL_CLIP_PLANE0);
		}
	}

	// render the model
	if (m_doc->IsValid())
	{
		// render the (solid) model
		if ((view.m_nrender == RENDER_SOLID) || (nitem != ITEM_MESH)) RenderModel(rc);

		// render discrete objects
		if (view.m_showDiscrete)
		{
			RenderDiscrete(rc);
		}

		// render selected box
		RenderSelectionBox(rc);

		cam.LineDrawMode(true);
		cam.Transform();

		// Render mesh lines
		//	if ((view.m_nrender == RENDER_SOLID) && (view.m_bmesh || (nitem != ITEM_MESH)))
		if (view.m_bmesh) RenderMeshLines(rc);

		if (view.m_bfeat || (view.m_nrender == RENDER_WIREFRAME))
		{
			// don't draw feature edges in edge mode, since the edges are the feature edges
			// (Don't draw feature edges when we are rendering FE edges)
			int nselect = m_doc->GetSelectionMode();
			if (((nitem != ITEM_MESH) || (nselect != SELECT_EDGE)) && (nitem != ITEM_EDGE)) RenderFeatureEdges(rc);
		}

		cam.LineDrawMode(false);
		cam.Transform();
	}

	//// render the temp object
	//CCreatePanel* cp = m_pWnd->GetCreatePanel();
	//if (cp)
	//{
	//	GObject* po = cp->GetTempObject();
	//	if (po)
	//	{
	//		RenderObject(po);
	//		RenderEdges(po);
	//	}
	//}

	// render physics
	if (m_doc->IsValid())
	{
		if (view.m_brigid) RenderRigidBodies(rc);
		if (view.m_bjoint) { RenderRigidJoints(rc); RenderRigidConnectors(rc); }
		if (view.m_bwall ) RenderRigidWalls(rc);
		if (view.m_bfiber) RenderMaterialFibers(rc);
		if (view.m_blma  ) RenderLocalMaterialAxes(rc);
	}

	// render the command window gizmo's
	/*	CCommandPanel* pcw = m_pWnd->GetCommandWindow()->GetActivePanel();
	if (pcw)
	{
	GLCanvas glc(this);
	pcw->Render(&glc);
	}
	*/
	// render the selected parts
	if (m_doc->IsValid())
	{
		GModel& model = *m_doc->GetGModel();
		int nsel = m_doc->GetSelectionMode();
		if (nitem == ITEM_MESH)
		{
			for (int i = 0; i < model.Objects(); ++i)
			{
				GObject* po = model.Object(i);
				if (po->IsVisible() && po->IsValid())
				{
					glPushMatrix();
					SetModelView(po);
					switch (nsel)
					{
					case SELECT_PART: RenderSelectedParts(rc, po); break;
					case SELECT_FACE: RenderSelectedSurfaces(rc, po); break;
					case SELECT_EDGE: RenderSelectedEdges(rc, po); break;
					case SELECT_NODE: RenderSelectedNodes(rc, po); break;
					}
					glPopMatrix();
				}
			}
		}
	}

	glDisable(GL_CLIP_PLANE0);

	// show the labels on rigid bodies
	if (view.m_showRigidLabels) RenderRigidLabels(rc);

	// render the tags
	if (view.m_bTags) glview->RenderTags();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderModel(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	VIEW_SETTINGS& view = glview->GetViewSettings();

	CGLCamera& cam = *rc.m_cam;

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// Get the item mode
	int item = pdoc->GetItemMode();

	// get the selection mode
	int nsel = pdoc->GetSelectionMode();

	GObject* poa = pdoc->GetActiveObject();

	bool bnorm = view.m_bnorm;
	double scale = view.m_scaleNormals;

	// we don't use backface culling when drawing
	//	if (view.m_bcull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	if (item == ITEM_MESH)
	{
		for (int i = 0; i < model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po->IsVisible() && po->IsValid())
			{
				glPushMatrix();
				SetModelView(po);
				switch (nsel)
				{
				case SELECT_OBJECT:
				{
					if (view.m_bcontour && (poa == po) && po->GetFEMesh()) RenderFEElements(rc, po);
					else if (glview->ShowPlaneCut() && (glview->PlaneCutMode() == 1))
					{
						RenderFEElements(rc, po);

						GLColor c = view.m_mcol;
						glColor3ub(c.r, c.g, c.b);
						RenderMeshLines(rc, po);
					}
					else RenderObject(rc, po);
				}
				break;
				case SELECT_PART: RenderParts(rc, po); break;
				case SELECT_FACE: RenderSurfaces(rc, po); break;
				case SELECT_EDGE:
				{
					RenderObject(rc, po);
					cam.LineDrawMode(true);
					cam.Transform();
					SetModelView(po);
					RenderEdges(rc, po);
					cam.LineDrawMode(false);
					cam.Transform();
					SetModelView(po);
				}
				break;
				case SELECT_NODE:
				{
					RenderObject(rc, po);
					cam.LineDrawMode(true);
					cam.Transform();
					SetModelView(po);
					RenderNodes(rc, po);
					cam.LineDrawMode(false);
					cam.Transform();
					SetModelView(po);
				}
				break;
				case SELECT_DISCRETE:
				{
					RenderObject(rc, po);
				}
				break;
				}
				if (bnorm) RenderNormals(rc, po, scale);
				glPopMatrix();
			}
		}
	}
	else
	{
		// get the mesh mode
		int meshMode = glview->GetMeshMode();

		for (int i = 0; i < model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po->IsVisible() && po->IsValid())
			{
				glPushMatrix();
				SetModelView(po);
				if (po == poa)
				{
					if (meshMode == MESH_MODE_VOLUME)
					{
						if (item == ITEM_ELEM)
						{
							RenderFEElements(rc, po);
						}
						else if (item == ITEM_FACE)
						{
							RenderFEFaces(rc, po);
						}
						else if (item == ITEM_EDGE)
						{
							RenderFEFaces(rc, po);
							cam.LineDrawMode(true);
							cam.Transform();
							SetModelView(po);
							RenderFEEdges(rc, po);
							cam.LineDrawMode(false);
							cam.Transform();
						}
						else if (item == ITEM_NODE)
						{
							RenderFEFaces(rc, po);
							RenderFENodes(rc, po);
						}
					}
					else
					{
						if (item == ITEM_FACE)
						{
							RenderSurfaceMeshFaces(rc, po);
						}
						else if (item == ITEM_EDGE)
						{
							RenderSurfaceMeshFaces(rc, po);
							cam.LineDrawMode(true);
							cam.Transform();
							SetModelView(po);
							RenderSurfaceMeshEdges(rc, po);
							cam.LineDrawMode(false);
							cam.Transform();
						}
						else if (item == ITEM_NODE)
						{
							RenderSurfaceMeshFaces(rc, po);
							RenderSurfaceMeshNodes(rc, po);
						}
					}
				}
				else RenderObject(rc, po);
				if (bnorm) RenderNormals(rc, po, scale);
				glPopMatrix();
			}
		}
	}
}

void CGLModelScene::RenderSelectionBox(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	VIEW_SETTINGS& view = glview->GetViewSettings();

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// Get the item mode
	int item = pdoc->GetItemMode();

	// get the selection mode
	int nsel = pdoc->GetSelectionMode();

	GObject* poa = pdoc->GetActiveObject();

	bool bnorm = view.m_bnorm;
	double scale = view.m_scaleNormals;

	if (item == ITEM_MESH)
	{
		for (int i = 0; i < model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po->IsVisible())
			{
				glPushMatrix();
				SetModelView(po);

				if (nsel == SELECT_OBJECT)
				{
					glColor3ub(255, 255, 255);
					if (po->IsSelected())
					{
						RenderBox(po->GetLocalBox(), true, 1.025);
					}
				}
				else if (po == poa)
				{
					glColor3ub(164, 0, 164);
					assert(po->IsSelected());
					RenderBox(po->GetLocalBox(), true, 1.025);
				}
				glPopMatrix();
			}
		}
	}
	else if (poa)
	{
		glPushMatrix();
		SetModelView(poa);
		glColor3ub(255, 255, 0);
		RenderBox(poa->GetLocalBox(), true, 1.025);
		glPopMatrix();
	}
}

void CGLModelScene::RenderRigidBodies(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	CGLCamera& cam = *rc.m_cam;

	FSModel* ps = pdoc->GetFSModel();

	double scale = 0.03 * (double)cam.GetTargetDistance();
	double R = 0.5 * scale;

	quatd qi = cam.GetOrientation().Inverse();

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	for (int i = 0; i < ps->Materials(); ++i)
	{
		GMaterial* pgm = ps->GetMaterial(i);
		FSMaterial* pm = pgm->GetMaterialProperties();
		if (pm && pm->IsRigid())
		{
			GLColor c = pgm->Diffuse();

			glColor3ub(c.r, c.g, c.b);

			vec3d r = pm->GetParamVec3d("center_of_mass");

			glPushMatrix();
			glTranslatef((float)r.x, (float)r.y, (float)r.z);

			glx::renderRigidBody(R);

			glPopMatrix();

			// get the parent
/*			if (pb->m_pid != -1)
			{
				FSRigidMaterial* pp = dynamic_cast<FSRigidMaterial*>(ps->GetMaterialFromID(pb->m_pid)->GetMaterialProperties());
				assert(pp);

				glColor3ub(50, 50, 255);
				vec3d r0 = pb->GetVecValue(FSRigidMaterial::MP_RC);
				vec3d r1 = pp->GetVecValue(FSRigidMaterial::MP_RC);

				double l = (r1 - r0).Length();
				vec3d el = r0 - r1; el.Normalize();

				quatd q(vec3d(0, 0, 1), el);
				glPushMatrix();
				{
					glTranslated(r1.x, r1.y, r1.z);
					glx::rotate(q);

					vec3d e2 = q*vec3d(0, 0, 1);

					double a = l*0.25;
					double b = a*0.25;
					glBegin(GL_LINES);
					{
						glVertex3d(0, 0, 0); glVertex3d(b, b, a); glVertex3d(b, b, a); glVertex3d(0, 0, l);
						glVertex3d(0, 0, 0); glVertex3d(-b, b, a); glVertex3d(-b, b, a); glVertex3d(0, 0, l);
						glVertex3d(0, 0, 0); glVertex3d(-b, -b, a); glVertex3d(-b, -b, a); glVertex3d(0, 0, l);
						glVertex3d(0, 0, 0); glVertex3d(b, -b, a); glVertex3d(b, -b, a); glVertex3d(0, 0, l);
						glVertex3d(b, b, a); glVertex3d(-b, b, a);
						glVertex3d(-b, b, a); glVertex3d(-b, -b, a);
						glVertex3d(-b, -b, a); glVertex3d(b, -b, a);
						glVertex3d(b, -b, a); glVertex3d(b, b, a);
					}
					glEnd();
				}
				glPopMatrix();
			}*/
		}
	}

	glPopAttrib();
}

void CGLModelScene::RenderRigidWalls(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	FSModel* ps = pdoc->GetFSModel();
	BOX box = ps->GetModel().GetBoundingBox();
	double R = box.GetMaxExtent();
	vec3d c = box.Center();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	for (int n = 0; n < ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i < s.Constraints(); ++i)
		{
			FSModelConstraint* pw = s.Constraint(i);
			if (pw->IsType("rigid_wall"))
			{
				// get the plane equation
				vector<double> a = pw->GetParamArrayDouble("plane");
				vec3d n(a[0], a[1], a[2]);
				double D = a[3];
				vec3d r0 = n * (D / (n * n));

				// project the center of the box onto the plane
				n.Normalize();
				vec3d p = c - n * (n * (c - r0));

				quatd q(vec3d(0, 0, 1), n);
				glPushMatrix();
				{
					glTranslated(p.x, p.y, p.z);
					glx::rotate(q);
					glColor4ub(128, 96, 0, 96);
					glRectd(-R, -R, R, R);

					glColor3ub(164, 128, 0);
					glBegin(GL_LINE_LOOP);
					{
						glVertex3d(-R, -R, 0);
						glVertex3d(R, -R, 0);
						glVertex3d(R, R, 0);
						glVertex3d(-R, R, 0);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(0, 0, 0); glVertex3d(0, 0, R / 2);
						glVertex3d(0, 0, R / 2); glVertex3d(-R * 0.1, 0, R * 0.4);
						glVertex3d(0, 0, R / 2); glVertex3d(R * 0.1, 0, R * 0.4);
					}
					glEnd();
				}
				glPopMatrix();
			}
		}
	}

	glPopAttrib();
}

void CGLModelScene::RenderRigidJoints(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	CGLCamera& cam = *rc.m_cam;

	FSModel* ps = pdoc->GetFSModel();

	double scale = 0.05 * (double)cam.GetTargetDistance();
	double R = 0.5 * scale;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255, 0, 0);

	for (int n = 0; n < ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i < s.Interfaces(); ++i)
		{
			FSRigidJoint* pj = dynamic_cast<FSRigidJoint*> (s.Interface(i));
			if (pj)
			{
				vec3d r = pj->GetVecValue(FSRigidJoint::RJ);

				glColor3ub(255, 0, 0);
				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glx::renderJoint(R);
				glPopMatrix();
			}
		}
	}

	glPopAttrib();
}

void CGLModelScene::RenderRigidConnectors(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLCamera& cam = *rc.m_cam;

	FSModel* ps = pdoc->GetFSModel();

	double scale = 0.05 * (double)cam.GetTargetDistance();
	double R = 0.5 * scale;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(0, 0, 255);

	for (int n = 0; n < ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i < s.RigidConnectors(); ++i)
		{
			FSRigidConnector* rci = s.RigidConnector(i);
			if (rci->IsType("rigid spherical joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");

				if (rci->IsActive())
					glColor3ub(255, 0, 0);
				else
					glColor3ub(64, 64, 64);

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glx::renderJoint(R);
				glPopMatrix();
			}
			else if (rci->IsType("rigid revolute joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("rotation_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("transverse_axis"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				if (rci->IsActive())
					glColor3ub(0, 0, 255);
				else
					glColor3ub(64, 64, 64);

				glx::renderRevoluteJoint(R);

				glPopMatrix();
			}
			else if (rci->IsType("rigid prismatic joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d a = rci->GetParamVec3d("translation_axis"); a.Normalize();
				vec3d b = rci->GetParamVec3d("transverse_axis"); b.Normalize();
				vec3d c = a ^ b; c.Normalize();
				b = c ^ a; b.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				if (rci->IsActive())
					glColor3ub(0, 255, 0);
				else
					glColor3ub(64, 64, 64);
				glx::renderPrismaticJoint(R);

				glPopMatrix();
			}
			else if (rci->IsType("rigid cylindrical joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("joint_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("transverse_axis"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				if (rci->IsActive())
					glColor3ub(255, 0, 255);
				else
					glColor3ub(64, 64, 64);

				glx::renderCylindricalJoint(R);

				glPopMatrix();
			}
			else if (rci->IsType("rigid planar joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("rotation_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("translation_axis_1"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				if (rci->IsActive())
					glColor3ub(0, 255, 255);
				else
					glColor3ub(64, 64, 64);

				glx::renderPlanarJoint(R);

				glPopMatrix();
			}
			else if (rci->IsType("rigid lock"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("first_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("second_axis"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				GLfloat Q4[16] = {
					(GLfloat)a.x, (GLfloat)a.y, (GLfloat)a.z, 0.f,
					(GLfloat)b.x, (GLfloat)b.y, (GLfloat)b.z, 0.f,
					(GLfloat)c.x, (GLfloat)c.y, (GLfloat)c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				glPushMatrix();
				glTranslatef((float)r.x, (float)r.y, (float)r.z);
				glMultMatrixf(Q4);

				if (rci->IsActive())
					glColor3ub(255, 127, 0);
				else
					glColor3ub(64, 64, 64);

				glx::renderRigidLock(R);

				glPopMatrix();
			}
			else if (rci->IsType("rigid spring"))
			{
				vec3d xa = rci->GetParamVec3d("insertion_a");
				vec3d xb = rci->GetParamVec3d("insertion_b");

				glPushMatrix();
				if (rci->IsActive())
					glColor3ub(255, 0, 0);
				else
					glColor3ub(64, 64, 64);

				glx::renderSpring(xa, xb, R);
				glPopMatrix();
			}
			else if (rci->IsType("rigid damper"))
			{
				vec3d xa = rci->GetParamVec3d("insertion_a");
				vec3d xb = rci->GetParamVec3d("insertion_b");

				glPushMatrix();

				if (rci->IsActive())
					glColor3ub(255, 0, 0);
				else
					glColor3ub(64, 64, 64);

				glx::renderDamper(xa, xb, R);

				glPopMatrix();
			}
			else if (rci->IsType("rigid contractile force"))
			{
				vec3d xa = rci->GetParamVec3d("insertion_a");
				vec3d xb = rci->GetParamVec3d("insertion_b");

				glPushMatrix();

				if (rci->IsActive())
					glColor3ub(255, 0, 0);
				else
					glColor3ub(64, 64, 64);

				glx::renderContractileForce(xa, xb, R);

				glPopMatrix();
			}
		}
	}

	glPopAttrib();
}

class GLFiberRenderer
{
public:
	GLFiberRenderer() {}
	void RenderFiber(GObject* po, FSMaterial* pmat, FEElementRef& rel, const vec3d& c);

	void Init();

	void Finish();

public:
	void SetColorOption(int n) { m_colorOption = n; }
	void SetDefaultColor(GLColor c) { m_defaultCol = c; }
	void SetScaleFactor(double s) { m_scale = s; }
	void SetLineStyle(int n) { m_lineStyle = n; }
	void SetLineWidth(double l) { m_lineWidth = l; }

private:
	int		m_colorOption = 0;
	int		m_lineStyle = 0;
	double	m_lineWidth = 1.0;
	GLColor	m_defaultCol;
	double	m_scale = 1.0;
	GLUquadricObj* m_glyph = nullptr;
};

void GLFiberRenderer::Init()
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	if (m_lineStyle == 0)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
	}
	else
	{
		m_glyph = gluNewQuadric();
		gluQuadricNormals(m_glyph, GLU_SMOOTH);
	}
}

void GLFiberRenderer::Finish()
{
	if (m_lineStyle == 0)
	{
		glEnd(); // GL_LINES
	}
	else
	{
		gluDeleteQuadric(m_glyph);
	}
	glPopAttrib();
}

void GLFiberRenderer::RenderFiber(GObject* po, FSMaterial* pmat, FEElementRef& rel, const vec3d& c)
{
	if (pmat->HasFibers())
	{
		vec3d q = pmat->GetFiber(rel);

		// This vector is defined in global coordinates, except for user-defined fibers, which
		// are assumed to be in local coordinates
		FSTransverselyIsotropic* ptiso = dynamic_cast<FSTransverselyIsotropic*>(pmat);
		if (ptiso && (ptiso->GetFiberMaterial()->m_naopt == FE_FIBER_USER))
		{
			q = po->GetTransform().LocalToGlobalNormal(q);
		}

		GLColor col = m_defaultCol;
		if (m_colorOption == 0)
		{
			Byte r = (Byte)(255 * fabs(q.x));
			Byte g = (Byte)(255 * fabs(q.y));
			Byte b = (Byte)(255 * fabs(q.z));
			col = GLColor(r, g, b);
		}

		vec3d p0 = c - q * (m_scale * 0.5);
		vec3d p1 = c + q * (m_scale * 0.5);

		glColor3ub(col.r, col.g, col.b);
		if (m_lineStyle == 0)
		{
			glVertex3d(p0.x, p0.y, p0.z);
			glVertex3d(p1.x, p1.y, p1.z);
		}
		else
		{
			glPushMatrix();

			glx::translate(p0);
			quatd Q(vec3d(0, 0, 1), q);
			glx::rotate(Q);

			gluCylinder(m_glyph, m_lineWidth, m_lineWidth, m_scale, 10, 1);

			glPopMatrix();
		}
	}

	int index = 0;
	for (int i = 0; i < pmat->Properties(); ++i)
	{
		FSProperty& prop = pmat->GetProperty(i);
		for (int j = 0; j < prop.Size(); ++j, ++index)
		{
			FSMaterial* matj = pmat->GetMaterialProperty(i, j);
			if (matj)
			{
				if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
				RenderFiber(po, matj, rel, c);
			}
		}
	}
}

void CGLModelScene::RenderMaterialFibers(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	VIEW_SETTINGS& view = glview->GetViewSettings();

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	FEElementRef rel;

	BOX box = model.GetBoundingBox();
	double h = 0.05 * box.GetMaxExtent();

	GLFiberRenderer fiberRender;
	fiberRender.SetScaleFactor(h * view.m_fiber_scale);
	fiberRender.SetLineWidth(h * view.m_fiber_width * 0.1);
	fiberRender.SetColorOption(view.m_fibColor);
	fiberRender.SetLineStyle(view.m_fibLineStyle);

	fiberRender.Init();

	GMaterial* pgm = nullptr;
	int matId = -1;
	int index = 0;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid() && (po->IsSelected() || (view.m_showSelectFibersOnly == false)))
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				rel.m_pmesh = pm;
				for (int j = 0; j < pm->Elements(); ++j)
				{
					FSElement& el = pm->Element(j);
					GPart* pg = po->Part(el.m_gid);

					bool showFiber = (pg->IsVisible() && el.IsVisible()) || view.m_showHiddenFibers;

					if (showFiber)
					{
						int partMatID = po->Part(el.m_gid)->GetMaterialID();
						if (partMatID != matId)
						{
							matId = partMatID;
							pgm = ps->GetMaterialFromID(matId);
						}
						FSMaterial* pmat = 0;
						if (pgm)
						{
							pmat = pgm->GetMaterialProperties();
							fiberRender.SetDefaultColor(pgm->Diffuse());
						}

						rel.m_nelem = j;
						if (pmat)
						{
							// element center
							vec3d c(0, 0, 0);
							for (int k = 0; k < el.Nodes(); ++k) c += pm->Node(el.m_node[k]).r;
							c /= el.Nodes();

							// to global coordinates
							c = po->GetTransform().LocalToGlobal(c);

							// render the fiber
							fiberRender.RenderFiber(po, pmat, rel, c);
						}
					}
				}
			}
		}
	}

	fiberRender.Finish();
}

void CGLModelScene::RenderLocalMaterialAxes(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	FEElementRef rel;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	VIEW_SETTINGS& view = glview->GetViewSettings();
	BOX box = model.GetBoundingBox();
	double h = 0.05 * box.GetMaxExtent() * view.m_fiber_scale;

	double rgb[3][3] = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };

	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				Transform& T = po->GetTransform();
				rel.m_pmesh = pm;
				for (int j = 0; j < pm->Elements(); ++j)
				{
					FSElement& el = pm->Element(j);

					GPart* pg = po->Part(el.m_gid);

					bool showAxes = (pg->IsVisible() && el.IsVisible()) || view.m_showHiddenFibers;

					if (showAxes)
					{
						GMaterial* pgm = ps->GetMaterialFromID(po->Part(el.m_gid)->GetMaterialID());
						FSMaterial* pmat = 0;
						if (pgm) pmat = pgm->GetMaterialProperties();

						rel.m_nelem = j;
						if (el.m_Qactive)
						{
							vec3d c(0, 0, 0);
							for (int k = 0; k < el.Nodes(); ++k) c += pm->NodePosition(el.m_node[k]);
							c /= el.Nodes();

							mat3d Q = el.m_Q;
							vec3d q;
							for (int k = 0; k < 3; ++k) {
								q = vec3d(Q[0][k], Q[1][k], Q[2][k]);

								q = T.LocalToGlobalNormal(q);

								glColor3ub(rgb[0][k], rgb[1][k], rgb[2][k]);

								glx::drawLine(c, c + q * h);
							}
						}
						else if (pmat)
						{
							vec3d c(0, 0, 0);
							for (int k = 0; k < el.Nodes(); ++k) c += pm->NodePosition(el.m_node[k]);
							c /= el.Nodes();

							mat3d Q = pmat->GetMatAxes(rel);
							vec3d q;
							for (int k = 0; k < 3; ++k) {
								q = vec3d(Q[0][k], Q[1][k], Q[2][k]);

								glColor3ub(rgb[0][k], rgb[1][k], rgb[2][k]);

								glx::drawLine(c, c + q * h);
							}
						}
					}
				}
			}
		}
	}

	glPopAttrib();
}

void RenderLine(GNode& n0, GNode& n1)
{
	vec3d r0 = n0.Position();
	vec3d r1 = n1.Position();

	glx::drawPoint(r0);
	glx::drawPoint(r1);

	glx::drawLine(r0, r1);
}

void CGLModelScene::RenderDiscrete(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	// get the selection mode
	int nsel = pdoc->GetSelectionMode();
	bool bsel = (nsel == SELECT_DISCRETE);

	// get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// render the discrete objects
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	int ND = model.DiscreteObjects();
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		if (po->IsVisible())
		{
			GLColor c = po->GetColor();

			if (bsel && po->IsSelected()) glColor3ub(255, 255, 0);
			else glColor3ub(c.r, c.g, c.b);

			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(po);
			if (ps)
			{
				GNode* pn0 = model.FindNode(ps->m_node[0]);
				GNode* pn1 = model.FindNode(ps->m_node[1]);
				if (pn0 && pn1) RenderLine(*pn0, *pn1);
			}

			GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(po);
			if (pg)
			{
				GNode* pn0 = model.FindNode(pg->m_node[0]);
				GNode* pn1 = model.FindNode(pg->m_node[1]);
				if (pn0 && pn1) RenderLine(*pn0, *pn1);
			}

			GDiscreteElementSet* pd = dynamic_cast<GDiscreteElementSet*>(po);
			if (pd)
			{
				int N = pd->size();
				for (int n = 0; n < N; ++n)
				{
					GDiscreteElement& el = pd->element(n);

					if (bsel && el.IsSelected()) glColor3ub(255, 255, 0);
					else glColor3ub(c.r, c.g, c.b);

					GNode* pn0 = model.FindNode(el.Node(0));
					GNode* pn1 = model.FindNode(el.Node(1));
					if (pn0 && pn1) RenderLine(*pn0, *pn1);
				}
			}

			GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(po);
			if (ds)
			{
				GNode* pn0 = model.FindNode(ds->NodeID(0));
				GNode* pn1 = model.FindNode(ds->NodeID(1));
				if (pn0 && pn1) RenderLine(*pn0, *pn1);
			}
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderMeshLines(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view; assert(glview);
	if (glview == nullptr) return;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	GModel& model = *pdoc->GetGModel();
	int nitem = pdoc->GetItemMode();

	VIEW_SETTINGS& vs = glview->GetViewSettings();
	GLColor c = vs.m_mcol;
	glColor3ub(c.r, c.g, c.b);

	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid())
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				glPushMatrix();
				SetModelView(po);
				if (nitem == ITEM_ELEM)
					RenderMeshLines(rc, po);
				else if (nitem == ITEM_MESH)
					renderer.RenderMeshLines(pm);
				else if (nitem != ITEM_EDGE)
					renderer.RenderMeshLines(po->GetEditableMesh());
				glPopMatrix();
			}
			else if (dynamic_cast<GSurfaceMeshObject*>(po))
			{
				FSSurfaceMesh* surfaceMesh = dynamic_cast<GSurfaceMeshObject*>(po)->GetSurfaceMesh();
				if (surfaceMesh && (nitem != ITEM_EDGE))
				{
					glPushMatrix();
					SetModelView(po);
					renderer.RenderMeshLines(surfaceMesh);
					glPopMatrix();
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderFeatureEdges(CGLContext& rc)
{
	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 0);

	FSModel* ps = doc->GetFSModel();
	GModel& model = ps->GetModel();

	for (int k = 0; k < model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		if (po->IsVisible())
		{
			glPushMatrix();
			SetModelView(po);

			GLMesh& m = *po->GetRenderMesh();
			renderer.RenderGLEdges(&m);

			glPopMatrix();
		}
	}
	glPopAttrib();
}

//=============================================================================
//					Rendering functions for GObjects
//=============================================================================

//-----------------------------------------------------------------------------
// Render non-selected nodes
void CGLModelScene::RenderNodes(CGLContext& rc, GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 255);
	for (int i = 0; i < po->Nodes(); ++i)
	{
		// only render nodes that are not selected
		// and are not shape-nodes
		GNode& n = *po->Node(i);
		if (!n.IsSelected() && (n.Type() != NODE_SHAPE))
		{
			vec3d r = n.LocalPosition();
			glBegin(GL_POINTS);
			{
				glVertex3d(r.x, r.y, r.z);
			}
			glEnd();
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render selected nodes
void CGLModelScene::RenderSelectedNodes(CGLContext& rc, GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255, 255, 0);
	for (int i = 0; i < po->Nodes(); ++i)
	{
		GNode& n = *po->Node(i);
		if (n.IsSelected())
		{
			assert(n.Type() != NODE_SHAPE);
			vec3d r = n.LocalPosition();
			glBegin(GL_POINTS);
			{
				glVertex3d(r.x, r.y, r.z);
			}
			glEnd();
		}
	}

#ifdef _DEBUG
	// Draw FE nodes on top of GMesh nodes to make sure they match
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		glColor3ub(255, 0, 0);
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& n = pm->Node(i);
			if (n.m_gid > -1)
			{
				GNode& gn = *po->Node(n.m_gid);
				if (gn.IsSelected())
				{
					vec3d r = n.r;
					glBegin(GL_POINTS);
					{
						glVertex3d(r.x, r.y, r.z);
					}
					glEnd();
				}
			}
		}
	}
#endif

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render non-selected edges
void CGLModelScene::RenderEdges(CGLContext& rc, GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 255);

	GLMeshRender& renderer = rc.m_view->GetMeshRenderer();

	GLMesh& m = *po->GetRenderMesh();
	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected() == false)
		{
			renderer.RenderGLEdges(&m, i);
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render selected edges
void CGLModelScene::RenderSelectedEdges(CGLContext& rc, GObject* po)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	vec3d r1, r2;

	GLMeshRender& renderer = rc.m_view->GetMeshRenderer();

	GLMesh& m = *po->GetRenderMesh();
	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected())
		{
			renderer.RenderGLEdges(&m, i);

			GNode* n0 = po->Node(e.m_node[0]);
			GNode* n1 = po->Node(e.m_node[1]);

			if (n0 && n1)
			{
				glBegin(GL_POINTS);
				{
					vec3d r0 = n0->LocalPosition();
					vec3d r1 = n1->LocalPosition();
					glVertex3d(r0.x, r0.y, r0.z);
					glVertex3d(r1.x, r1.y, r1.z);
				}
				glEnd();
			}
		}
	}

#ifdef _DEBUG
	// Render FE edges onto of GMesh edges to make sure they are consistent
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		glColor3ub(255, 0, 0);
		for (int i = 0; i < pm->Edges(); ++i)
		{
			FSEdge& e = pm->Edge(i);
			if (e.m_gid > -1)
			{
				GEdge& ge = *po->Edge(e.m_gid);
				if (ge.IsSelected())
				{
					vec3d r0 = pm->Node(e.n[0]).r;
					vec3d r1 = pm->Node(e.n[1]).r;
					glBegin(GL_LINES);
					{
						glVertex3d(r0.x, r0.y, r0.z);
						glVertex3d(r1.x, r1.y, r1.z);
					}
					glEnd();
				}
			}
		}
	}
#endif
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render non-selected surfaces
void CGLModelScene::RenderSurfaces(CGLContext& rc, GObject* po)
{
	if (!po->IsVisible()) return;

	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	VIEW_SETTINGS& vs = glview->GetViewSettings();

	// get the GLMesh
	FSModel& fem = *doc->GetFSModel();
	GLMesh* pm = po->GetRenderMesh();
	assert(pm);

	// render non-selected faces
	GPart* pgmat = 0; // the part that defines the material
	int NF = po->Faces();
	for (int n = 0; n < NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		// make sure this face is not selected
		if (f.IsSelected() == false)
		{
			// get the part IDs
			int* pid = f.m_nPID;

			// get the part (that is visible)
			GPart* pg = po->Part(pid[0]);
			if (pg && pg->IsVisible() == false)
			{
				if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = 0;
				if (pg && (pg->IsVisible() == false)) pg = 0;
			}

			// make sure we have a part
			if (pg)
			{
				// if this part is not the current part defining the 
				// material, we need to change the mat props
				if (pg != pgmat)
				{
					if (vs.m_objectColor == 0)
					{
						GMaterial* pmat = fem.GetMaterialFromID(pg->GetMaterialID());
						SetMatProps(pmat);
						GLColor c = po->GetColor();
						if (pmat) c = pmat->Diffuse();
						glColor3ub(c.r, c.g, c.b);
						pgmat = pg;
					}
					else
					{
						SetMatProps(0);
						GLColor c = po->GetColor();
						glColor3ub(c.r, c.g, c.b);
					}
				}

				if (vs.m_transparencyMode != 0)
				{
					switch (vs.m_transparencyMode)
					{
					case 1: if (po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					case 2: if (!po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					}
				}

				// render the face
				renderer.RenderGLMesh(pm, n);

				if (vs.m_transparencyMode != 0) glDisable(GL_POLYGON_STIPPLE);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Render selected surfaces
void CGLModelScene::RenderSelectedSurfaces(CGLContext& rc, GObject* po)
{
	if (!po->IsVisible()) return;

	GLMeshRender& renderer = rc.m_view->GetMeshRenderer();

	GLMesh* pm = po->GetRenderMesh();
	assert(pm);

	// render the selected faces
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	{
		glColor3ub(0, 0, 255);
		glEnable(GL_POLYGON_STIPPLE);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		int NF = po->Faces();
		for (int i = 0; i < NF; ++i)
		{
			GFace& f = *po->Face(i);
			if (f.IsSelected())
			{
				renderer.RenderGLMesh(pm, i);
			}
		}

#ifdef _DEBUG
		// Render the GFace nodes and the FE surfaces to make sure the 
		// GMesh and the FE mesh are consisten

		// render GNodes
		// TODO: This causes a crash after a primitive was converted to editable mesh and auto partition was applied.
/*		for (int i = 0; i<NF; ++i)
		{
			GFace& f = *po->Face(i);
			if (f.IsSelected())
			{
				glBegin(GL_POINTS);
				{
					int nf = f.Nodes();
					for (int j = 0; j<nf; ++j)
					if (f.m_node[j] != -1)
					{
						vec3d r = po->Node(f.m_node[j])->LocalPosition();
						int c = 255 * j / (nf - 1);
						glColor3ub((GLubyte)c, (GLubyte)c, (GLubyte)c);
						glVertex3d(r.x, r.y, r.z);
					}
				}
				glEnd();
			}
		}
*/
// render FE surfaces
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			glColor3ub(255, 0, 0);
			vec3d rf[FSElement::MAX_NODES];
			for (int i = 0; i < pm->Faces(); ++i)
			{
				FSFace& f = pm->Face(i);
				if (f.m_gid > -1)
				{
					GFace& gf = *po->Face(f.m_gid);
					if (gf.IsSelected())
					{
						int nf = f.Nodes();
						for (int j = 0; j < nf; ++j) rf[j] = pm->Node(f.n[j]).r;
						switch (nf)
						{
						case 3:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
							}
							glEnd();
							break;
						case 4:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
							}
							glEnd();
							break;
						case 6:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);

								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);

								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[2].x, rf[2].y, rf[2].z);

								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
							}
							glEnd();
							break;
						case 8:
						case 9:
							glBegin(GL_TRIANGLES);
							{
								glVertex3d(rf[0].x, rf[0].y, rf[0].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[7].x, rf[7].y, rf[7].z);

								glVertex3d(rf[1].x, rf[1].y, rf[1].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
								glVertex3d(rf[4].x, rf[4].y, rf[4].z);

								glVertex3d(rf[2].x, rf[2].y, rf[2].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);

								glVertex3d(rf[3].x, rf[3].y, rf[3].z);
								glVertex3d(rf[7].x, rf[7].y, rf[7].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);

								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);
								glVertex3d(rf[7].x, rf[7].y, rf[7].z);

								glVertex3d(rf[4].x, rf[4].y, rf[4].z);
								glVertex3d(rf[5].x, rf[5].y, rf[5].z);
								glVertex3d(rf[6].x, rf[6].y, rf[6].z);
							}
							glEnd();
							break;
						}
					}
				}
			}
		}
#endif
		glDisable(GL_POLYGON_STIPPLE);
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render non-selected parts
void CGLModelScene::RenderParts(CGLContext& rc, GObject* po)
{
	if (!po->IsVisible()) return;

	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	VIEW_SETTINGS& vs = glview->GetViewSettings();

	// get the GLMesh
	FSModel& fem = *doc->GetFSModel();
	GLMesh* pm = po->GetRenderMesh();
	assert(pm);

	// render non-selected parts
	GPart* pgmat = 0; // the part that defines the material
	int NF = po->Faces();
	for (int n = 0; n < NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		// get the part IDs
		int* pid = f.m_nPID;

		// get the part (that is visible)
		GPart* pg = po->Part(pid[0]); assert(pg);
		if (pg && ((pg->IsVisible() == false) || (pg->IsSelected())))
		{
			if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = 0;
			if (pg && ((pg->IsVisible() == false) || pg->IsSelected())) pg = 0;

			if (pg == nullptr)
			{
				if (pid[2] >= 0) pg = po->Part(pid[2]); else pg = 0;
				if (pg && ((pg->IsVisible() == false) || pg->IsSelected())) pg = 0;
			}
		}

		// make sure we have a part
		if (pg)
		{
			// if this part is not the current part defining the 
			// material, we need to change the mat props
			if (vs.m_objectColor == 0)
			{
				if (pg != pgmat)
				{
					GMaterial* pmat = fem.GetMaterialFromID(pg->GetMaterialID());
					SetMatProps(pmat);
					GLColor c = po->GetColor();
					if (pmat) c = pmat->Diffuse();
					glColor3ub(c.r, c.g, c.b);
					pgmat = pg;
				}
			}
			else
			{
				SetMatProps(0);
				GLColor c = po->GetColor();
				glColor3ub(c.r, c.g, c.b);
			}

			if (vs.m_transparencyMode != 0)
			{
				switch (vs.m_transparencyMode)
				{
				case 1: if (po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
				case 2: if (!po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
				}
			}

			// render the face
			int nid = pg->GetLocalID();
			renderer.RenderGLMesh(pm, n);

			if (vs.m_transparencyMode != 0) glDisable(GL_POLYGON_STIPPLE);
		}
	}

	RenderBeamParts(rc, po);
}

//-----------------------------------------------------------------------------
// render selected parts
void CGLModelScene::RenderSelectedParts(CGLContext& rc, GObject* po)
{
	if (!po->IsVisible()) return;

	CGLView* glview = rc.m_view;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	glPushAttrib(GL_ENABLE_BIT);
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable(GL_POLYGON_STIPPLE);
		SetMatProps(0);
		glColor3ub(0, 0, 255);
		GLMesh& m = *po->GetRenderMesh();
		int NF = po->Faces();
		for (int i = 0; i < NF; ++i)
		{
			GFace* pf = po->Face(i);
			GPart* p0 = po->Part(pf->m_nPID[0]);
			GPart* p1 = po->Part(pf->m_nPID[1]);
			GPart* p2 = po->Part(pf->m_nPID[2]);
			if ((p0 && p0->IsSelected()) || (p1 && p1->IsSelected()) || (p2 && p2->IsSelected()))
			{
				renderer.RenderGLMesh(&m, i);
			}
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// This function renders the object by looping over all the parts and
// for each part render the external surfaces that belong to that part.
// NOTE: The reason why only external surfaces are rendered is because
//       it is possible for an external surface to coincide with an
//       internal surface. E.g., when a shell layer lies on top of a 
//       hex layer.
void CGLModelScene::RenderObject(CGLContext& rc, GObject* po)
{
	if (!po->IsVisible()) return;

	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;

	VIEW_SETTINGS& vs = glview->GetViewSettings();

	// get the GLMesh
	FSModel& fem = *doc->GetFSModel();
	GLMesh* pm = po->GetRenderMesh();
	if (pm == 0) return;
	assert(pm);

	GLMeshRender& renderer = glview->GetMeshRenderer();

	// render non-selected faces
	GPart* pgmat = 0; // the part that defines the material
	int NF = po->Faces();
	for (int n = 0; n < NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		// make sure the face is visible
		if (f.IsVisible())
		{
			// get the part IDs
			int* pid = f.m_nPID;

			// get the part (that is visible)
			GPart* pg = po->Part(pid[0]);
			if (pg && pg->IsVisible() == false)
			{
				if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = 0;
				if (pg && (pg->IsVisible() == false)) pg = 0;
			}

			// make sure we have a part
			if (pg)
			{
				// if this part is not the current part defining the 
				// material, we need to change the mat props
				if (vs.m_objectColor == 0)
				{
					if (pg != pgmat)
					{
						GMaterial* pmat = fem.GetMaterialFromID(pg->GetMaterialID());
						SetMatProps(pmat);
						GLColor c = po->GetColor();
						if (pmat) c = pmat->Diffuse();

						glColor3ub(c.r, c.g, c.b);
						pgmat = pg;
					}
				}
				else
				{
					SetMatProps(0);
					GLColor c = po->GetColor();
					glColor3ub(c.r, c.g, c.b);
				}

				if (vs.m_transparencyMode != 0)
				{
					switch (vs.m_transparencyMode)
					{
					case 1: if (po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					case 2: if (!po->IsSelected()) glEnable(GL_POLYGON_STIPPLE); break;
					}
				}

				// render the face
				renderer.RenderGLMesh(pm, n);

				if (vs.m_transparencyMode != 0) glDisable(GL_POLYGON_STIPPLE);
			}
		}
	}

	if (NF == 0)
	{
		// if there are no faces, render edges instead
		int NC = po->Edges();
		for (int n = 0; n < NC; ++n)
		{
			GEdge& e = *po->Edge(n);
			if (e.IsVisible())
				renderer.RenderGLEdges(pm, e.GetLocalID());
		}
	}

	// render beam sections if feature edges are not rendered. 
	if (vs.m_bfeat == false)
	{
		RenderBeamParts(rc, po);
	}
}

void CGLModelScene::RenderBeamParts(CGLContext& rc, GObject* po)
{
	if (!po->IsVisible()) return;

	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	int nitem = m_doc->GetItemMode();
	int nsel = m_doc->GetSelectionMode();

	CGLView* glview = rc.m_view;

	VIEW_SETTINGS& vs = glview->GetViewSettings();

	// get the GLMesh
	FSModel& fem = *doc->GetFSModel();
	GLMesh* pm = po->GetRenderMesh();
	if (pm == 0) return;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	GPart* pgmat = 0; // the part that defines the material
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	SetMatProps(0);
	GLColor c = po->GetColor();
	glColor3ub(c.r, c.g, c.b);
	for (int i = 0; i < po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		if (pg->IsVisible() && pg->IsBeam())
		{
			// if this part is not the current part defining the 
			// material, we need to change the mat props
			if (vs.m_objectColor == 0)
			{
				GMaterial* pmat = fem.GetMaterialFromID(pg->GetMaterialID());
				SetMatProps(pmat);
				GLColor c = po->GetColor();
				if (pmat) c = pmat->Diffuse();

				glColor3ub(c.r, c.g, c.b);
				pgmat = pg;
			}

			if ((nitem == ITEM_MESH) && (nsel == SELECT_PART) && pg->IsSelected())
			{
				SetMatProps(0);
				glColor3ub(0, 0, 255);
			}

			for (int j = 0; j < pg->m_edge.size(); ++j)
			{
				GEdge& e = *po->Edge(pg->m_edge[j]);
				if (e.IsVisible())
					renderer.RenderGLEdges(pm, e.GetLocalID());
			}
		}
	}
	glPopAttrib();
}

//=============================================================================
//					Rendering functions for FEMeshes
//=============================================================================

//-----------------------------------------------------------------------------
// Render the FE nodes
void CGLModelScene::RenderFENodes(CGLContext& rc, GObject* po)
{
	CGLView* glview = rc.m_view;

	VIEW_SETTINGS& view = glview->GetViewSettings();
	quatd q = rc.m_cam->GetOrientation();

	GLMeshRender& renderer = glview->GetMeshRenderer();

	// set the point size
	float fsize = view.m_node_size;
	renderer.SetPointSize(fsize);

	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		int N = pm->Nodes();
		int NF = pm->Faces();
		int NE = pm->Elements();

		// reset all tags
		for (int i = 0; i < N; ++i) pm->Node(i).m_ntag = 1;

		// make sure we render all isolated nodes
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = pm->Element(i);
			int n = el.Nodes();
			for (int j = 0; j < n; ++j) pm->Node(el.m_node[j]).m_ntag = 0;
		}

		// check visibility
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.IsVisible() && (po->Part(el.m_gid)->IsVisible()))
			{
				int n = el.Nodes();
				for (int j = 0; j < n; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
			}
		}

		// check the cull
		if (view.m_bcull)
		{
			vec3d f;
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = pm->Face(i);
				int n = face.Nodes();
				for (int j = 0; j < n; ++j)
				{
					vec3d nn = to_vec3d(face.m_nn[j]);
					f = q * nn;
					if (f.z < 0) pm->Node(face.n[j]).m_ntag = 0;
				}
			}
		}

		// check the ext criteria
		if (view.m_bext)
		{
			for (int i = 0; i < N; ++i)
			{
				FSNode& node = pm->Node(i);
				if (!node.IsExterior()) node.m_ntag = 0;
			}
		}

		renderer.RenderFENodes(pm);
	}
	else
	{
		FSMeshBase* mesh = po->GetEditableMesh();
		if (mesh)
		{
			// reset all tags
			mesh->TagAllNodes(1);

			// make sure we render all isolated nodes
			int NF = mesh->Faces();
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = mesh->Face(i);
				int n = face.Nodes();
				for (int j = 0; j < n; ++j) mesh->Node(face.n[j]).m_ntag = 0;
			}

			// check visibility
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = mesh->Face(i);
				if (face.IsVisible())
				{
					int n = face.Nodes();
					for (int j = 0; j < n; ++j) mesh->Node(face.n[j]).m_ntag = 1;
				}
			}

			// check the cull
			if (view.m_bcull)
			{
				vec3d f;
				for (int i = 0; i < NF; ++i)
				{
					FSFace& face = mesh->Face(i);
					int n = face.Nodes();
					for (int j = 0; j < n; ++j)
					{
						vec3d nn = to_vec3d(face.m_nn[j]);
						f = q * nn;
						if (f.z < 0) mesh->Node(face.n[j]).m_ntag = 0;
					}
				}
			}

			renderer.RenderFENodes(mesh);
		}
		else
		{
			FSLineMesh* pm = po->GetEditableLineMesh();
			if (pm)
			{
				pm->TagAllNodes(1);
				renderer.RenderFENodes(pm);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderFEFaces(CGLContext& rc, GObject* po)
{
	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	VIEW_SETTINGS& view = glview->GetViewSettings();
	FSModel& fem = *doc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		RenderObject(rc, po);
		return;
	}

	GLColor col = po->GetColor();
	GLColor dif = col;
	SetMatProps(0);
	glColor3ub(dif.r, dif.g, dif.b);
	int nmatid = -1;

	double vmin, vmax;
	Post::CColorMap map;
	Mesh_Data& data = pm->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());
	if (showContour) { data.GetValueRange(vmin, vmax); map.SetRange((float)vmin, (float)vmax); }

	// render the unselected faces
	for (int i = 0; i < pm->Faces(); i++)
	{
		FSFace& face = pm->Face(i);

		FSElement& el = pm->Element(face.m_elem[0].eid);
		GPart* pg = po->Part(el.m_gid);
		if ((pg->IsVisible() == false) && (face.m_elem[1].eid != -1))
		{
			FSElement& el1 = pm->Element(face.m_elem[1].eid);
			pg = po->Part(el1.m_gid);
		}

		if (!face.IsSelected() && face.IsVisible())
		{
			if (pg && pg->IsVisible())
			{
				if (showContour)
				{
					if (data.GetElementDataTag(face.m_elem[0].eid) > 0)
					{
						int fnl[FSElement::MAX_NODES];
						int nn = el.GetLocalFaceIndices(face.m_elem[0].lid, fnl);
						assert(nn == face.Nodes());

						GLColor c[FSFace::MAX_NODES];
						int nf = face.Nodes();
						for (int j = 0; j < nf; ++j)
							c[j] = map.map(data.GetElementValue(face.m_elem[0].eid, fnl[j]));

						// Render the face
						renderer.RenderFace(face, pm, c, 1);
					}
					else
					{
						dif = GLColor(212, 212, 212);
						glColor3ub(dif.r, dif.g, dif.b);

						// Render the face
						glBegin(GL_TRIANGLES);
						{
							renderer.RenderFEFace(face, pm);
						}
						glEnd();
					}
				}
				else
				{
					if (view.m_objectColor == 0)
					{
						if (pg->GetMaterialID() != nmatid)
						{
							nmatid = pg->GetMaterialID();
							GMaterial* pmat = fem.GetMaterialFromID(nmatid);
							SetMatProps(pmat);
							dif = (pmat ? pmat->Diffuse() : col);
							glColor3ub(dif.r, dif.g, dif.b);

							int glmode = 0;
							if (pmat && (pmat->m_nrender != 0))
							{
								GLint n[2];
								glGetIntegerv(GL_POLYGON_MODE, n);
								glmode = n[1];
								if (n[1] != GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							}
						}
					}

					// Render the face
					glBegin(GL_TRIANGLES);
					{
						renderer.RenderFEFace(face, pm);
					}
					glEnd();
				}
			}
		}
	}

	// render beam elements
	RenderAllBeamElements(rc, po);

	// render the selected faces
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_STIPPLE);
	glColor3ub(255, 0, 0);
	renderer.RenderSelectedFEFaces(pm);

	// render the selected face outline
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	renderer.RenderSelectedFEFacesOutline(pm);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderSurfaceMeshFaces(CGLContext& rc, GObject* po)
{
	GSurfaceMeshObject* surfaceObject = dynamic_cast<GSurfaceMeshObject*>(po);
	if (surfaceObject == 0)
	{
		// just render something, otherwise nothing will show up
		RenderObject(rc, po);
		return;
	}

	FSSurfaceMesh* surfaceMesh = surfaceObject->GetSurfaceMesh();
	assert(surfaceMesh);
	if (surfaceMesh == 0) return;

	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	VIEW_SETTINGS& view = glview->GetViewSettings();
	FSModel& fem = *doc->GetFSModel();

	GLColor col = po->GetColor();
	SetMatProps(0);
	glColor3ub(col.r, col.g, col.b);

	// render the unselected faces
	// Note that we do not render internal faces
	renderer.RenderUnselectedFEFaces(surfaceMesh);

	// render the selected faces
	// override some settings
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_STIPPLE);
	glColor3ub(255, 128, 0);
	renderer.RenderSelectedFEFaces(surfaceMesh);

	// render the selected face outline
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	renderer.RenderSelectedFEFacesOutline(surfaceMesh);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderSurfaceMeshEdges(CGLContext& rc, GObject* po)
{
	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;

	GLMeshRender& renderer = glview->GetMeshRenderer();

	VIEW_SETTINGS& view = glview->GetViewSettings();
	FSModel& fem = *doc->GetFSModel();
	FSLineMesh* pm = po->GetEditableLineMesh();
	assert(pm);
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	// render the unselected edges
	glColor3ub(0, 0, 255);
	renderer.RenderUnselectedFEEdges(pm);

	// render the selected edges
	// override some settings
	glDisable(GL_CULL_FACE);
	glColor3ub(255, 0, 0);
	renderer.RenderSelectedFEEdges(pm);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderSurfaceMeshNodes(CGLContext& rc, GObject* po)
{
	CGLDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	GLMeshRender& renderer = glview->GetMeshRenderer();

	VIEW_SETTINGS& view = glview->GetViewSettings();
	quatd q = pdoc->GetView()->GetCamera().GetOrientation();

	// set the point size
	float fsize = view.m_node_size;
	renderer.SetPointSize(fsize);

	FSMeshBase* mesh = po->GetEditableMesh();
	if (mesh)
	{
		// reset all tags
		mesh->TagAllNodes(1);

		// make sure we render all isolated nodes
		int NF = mesh->Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = mesh->Face(i);
			int n = face.Nodes();
			for (int j = 0; j < n; ++j) mesh->Node(face.n[j]).m_ntag = 0;
		}

		// check visibility
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = mesh->Face(i);
			if (face.IsVisible())
			{
				int n = face.Nodes();
				for (int j = 0; j < n; ++j) mesh->Node(face.n[j]).m_ntag = 1;
			}
		}

		// check the cull
		if (view.m_bcull)
		{
			vec3d f;
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = mesh->Face(i);
				int n = face.Nodes();
				for (int j = 0; j < n; ++j)
				{
					vec3d nn = to_vec3d(face.m_nn[j]);
					f = q * nn;
					if (f.z < 0) mesh->Node(face.n[j]).m_ntag = 0;
				}
			}
		}

		renderer.RenderFENodes(mesh);
	}
	else
	{
		FSLineMesh* pm = po->GetEditableLineMesh();
		if (pm)
		{
			pm->TagAllNodes(1);
			renderer.RenderFENodes(pm);
		}
	}
}

//-----------------------------------------------------------------------------
// Render the FE Edges
void CGLModelScene::RenderFEEdges(CGLContext& rc, GObject* po)
{
	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	CGLView* glview = rc.m_view;
	GLMeshRender& renderer = glview->GetMeshRenderer();

	VIEW_SETTINGS& view = glview->GetViewSettings();
	FSModel& fem = *doc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	// render the unselected edges
	glColor3ub(0, 0, 255);
	renderer.RenderUnselectedFEEdges(pm);

	// render the selected edges
	// override some settings
	glDisable(GL_CULL_FACE);
	glColor3ub(255, 0, 0);
	renderer.RenderSelectedFEEdges(pm);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render the FE elements
void CGLModelScene::RenderFEElements(CGLContext& rc, GObject* po)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	GLMeshRender& renderer = glview->GetMeshRenderer();

	FSModel& fem = *pdoc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	assert(pm);
	if (pm == 0) return;

	VIEW_SETTINGS& view = glview->GetViewSettings();
	GLColor dif;

	GLColor col = po->GetColor();

	int i;

	int nmatid = -1;
	dif = po->GetColor();
	glColor3ub(dif.r, dif.g, dif.b);
	SetMatProps(0);
	int glmode = 0;

	Post::CColorMap& colorMap = glview->GetColorMap();

	double vmin, vmax;
	Mesh_Data& data = pm->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());
	if (showContour)
	{
		data.GetValueRange(vmin, vmax); colorMap.SetRange((float)vmin, (float)vmax);

		glEnable(GL_COLOR_MATERIAL);
	}

	// render the unselected faces
	vector<int> selectedElements;
	int NE = pm->Elements();
	bool hasBeamElements = false;
	for (i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsVisible() && el.IsSelected()) selectedElements.push_back(i);

		if (!el.IsSelected() && el.IsVisible())
		{
			GPart* pg = po->Part(el.m_gid);
			if (pg->IsVisible())
			{
				if (showContour)
				{
					GLColor c[FSElement::MAX_NODES];
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j)
					{
						if (data.GetElementDataTag(i) > 0)
							c[j] = colorMap.map(data.GetElementValue(i, j));
						else
							c[j] = GLColor(212, 212, 212);
					}

					switch (el.Type())
					{
					case FE_HEX8   : renderer.RenderHEX8(&el, pm, c); break;
					case FE_HEX20  : renderer.RenderHEX20(&el, pm, true); break;
					case FE_HEX27  : renderer.RenderHEX27(&el, pm, true); break;
					case FE_PENTA6 : renderer.RenderPENTA6(&el, pm, c); break;
					case FE_PENTA15: renderer.RenderPENTA15(&el, pm, true); break;
					case FE_TET4   : renderer.RenderTET4(&el, pm, c); break;
					case FE_TET5   : renderer.RenderTET4(&el, pm, c); break;
					case FE_TET10  : renderer.RenderTET10(&el, pm, c); break;
					case FE_TET15  : renderer.RenderTET15(&el, pm, true); break;
					case FE_TET20  : renderer.RenderTET20(&el, pm, true); break;
					case FE_QUAD4  : renderer.RenderQUAD(&el, pm, c); break;
					case FE_QUAD8  : renderer.RenderQUAD8(&el, pm, true); break;
					case FE_QUAD9  : renderer.RenderQUAD9(&el, pm, true); break;
					case FE_TRI3   : renderer.RenderTRI3(&el, pm, c); break;
					case FE_TRI6   : renderer.RenderTRI6(&el, pm, true); break;
					case FE_PYRA5  : renderer.RenderPYRA5(&el, pm, true); break;
					case FE_PYRA13 : renderer.RenderPYRA13(&el, pm, true); break;
					case FE_BEAM2  : break;
					case FE_BEAM3  : break;
					default:
						assert(false);
					}

				}
				else
				{
					if (view.m_objectColor == 0)
					{
						if (pg->GetMaterialID() != nmatid)
						{
							GMaterial* pmat = 0;
							if (pg->GetMaterialID() != nmatid)
							{
								nmatid = pg->GetMaterialID();
								pmat = fem.GetMaterialFromID(nmatid);
								SetMatProps(pmat);
							}

							dif = (pmat != 0 ? pmat->Diffuse() : col);

							glColor3ub(dif.r, dif.g, dif.b);

							if (pmat && (pmat->m_nrender != 0))
							{
								GLint n[2];
								glGetIntegerv(GL_POLYGON_MODE, n);
								glmode = n[1];
								if (n[1] != GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							}
						}
					}

					switch (el.Type())
					{
					case FE_HEX8   : renderer.RenderHEX8(&el, pm, true); break;
					case FE_HEX20  : renderer.RenderHEX20(&el, pm, true); break;
					case FE_HEX27  : renderer.RenderHEX27(&el, pm, true); break;
					case FE_PENTA6 : renderer.RenderPENTA(&el, pm, true); break;
					case FE_PENTA15: renderer.RenderPENTA15(&el, pm, true); break;
					case FE_TET4   : renderer.RenderTET4(&el, pm, true); break;
					case FE_TET5   : renderer.RenderTET4(&el, pm, true); break;
					case FE_TET10  : renderer.RenderTET10(&el, pm, true); break;
					case FE_TET15  : renderer.RenderTET15(&el, pm, true); break;
					case FE_TET20  : renderer.RenderTET20(&el, pm, true); break;
					case FE_QUAD4  : renderer.RenderQUAD(&el, pm, true); break;
					case FE_QUAD8  : renderer.RenderQUAD8(&el, pm, true); break;
					case FE_QUAD9  : renderer.RenderQUAD9(&el, pm, true); break;
					case FE_TRI3   : renderer.RenderTRI3(&el, pm, true); break;
					case FE_TRI6   : renderer.RenderTRI6(&el, pm, true); break;
					case FE_PYRA5  : renderer.RenderPYRA5(&el, pm, true); break;
					case FE_PYRA13 : renderer.RenderPYRA13(&el, pm, true); break;
					case FE_BEAM2  : hasBeamElements = true; break;
					case FE_BEAM3  : break;
					default:
						assert(false);
					}
				}
			}
		}
	}

	if (hasBeamElements)
	{
		// render beam elements
		RenderUnselectedBeamElements(rc, po);
	}

	// override some settings
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glColor4ub(255, 0, 0, 128);
	glEnable(GL_POLYGON_STIPPLE);
	glDisable(GL_LIGHTING);

	// render the selected faces
	if (pdoc == nullptr) return;
	if (selectedElements.empty() == false)
	{
		hasBeamElements = false;
		int NE = (int)selectedElements.size();
		for (i = 0; i < NE; ++i)
		{
			FEElement_& el = pm->Element(selectedElements[i]);
			if (el.IsVisible())
			{
				switch (el.Type())
				{
				case FE_HEX8   : renderer.RenderHEX8(&el, pm, false); break;
				case FE_HEX20  : renderer.RenderHEX20(&el, pm, false); break;
				case FE_HEX27  : renderer.RenderHEX27(&el, pm, false); break;
				case FE_PENTA6 : renderer.RenderPENTA(&el, pm, false); break;
				case FE_PENTA15: renderer.RenderPENTA15(&el, pm, true); break;
				case FE_TET4   : renderer.RenderTET4(&el, pm, false); break;
				case FE_TET5   : renderer.RenderTET4(&el, pm, false); break;
				case FE_TET10  : renderer.RenderTET10(&el, pm, false); break;
				case FE_TET15  : renderer.RenderTET15(&el, pm, false); break;
				case FE_TET20  : renderer.RenderTET20(&el, pm, false); break;
				case FE_QUAD4  : renderer.RenderQUAD(&el, pm, false); break;
				case FE_QUAD8  : break;
				case FE_QUAD9  : break;
				case FE_TRI3   : renderer.RenderTRI3(&el, pm, false); break;
				case FE_TRI6   : renderer.RenderTRI6(&el, pm, false); break;
				case FE_PYRA5  : renderer.RenderPYRA5(&el, pm, false); break;
				case FE_PYRA13 : renderer.RenderPYRA13(&el, pm, false); break;
				case FE_BEAM2  : hasBeamElements = true;  break;
				case FE_BEAM3  : break;
				default:
					assert(false);
				}
			}
		}

		// render a yellow highlight around selected elements
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glColor3ub(255, 255, 0);

		glBegin(GL_LINES);
		for (i = 0; i < NE; ++i)
		{
			FEElement_& el = pm->Element(selectedElements[i]);
			int ne = el.Nodes();
			if (el.IsVisible())
			{
				switch (el.Type())
				{
				case FE_HEX8:
				case FE_HEX20:
				case FE_HEX27:
					for (int j = 0; j < 12; ++j)
					{
						int n0 = el.m_node[ET_HEX[j][0]];
						int n1 = el.m_node[ET_HEX[j][1]];

						vec3d r0 = pm->Node(n0).pos();
						vec3d r1 = pm->Node(n1).pos();

						glx::vertex3d(r0);
						glx::vertex3d(r1);
					}
					break;
				case FE_TET4:
				case FE_TET10:
				case FE_TET15:
					for (int j = 0; j < 6; ++j)
					{
						int n0 = el.m_node[ET_TET[j][0]];
						int n1 = el.m_node[ET_TET[j][1]];

						vec3d r0 = pm->Node(n0).pos();
						vec3d r1 = pm->Node(n1).pos();

						glx::vertex3d(r0);
						glx::vertex3d(r1);
					}
					break;
				case FE_PYRA5:
				case FE_PYRA13:
					for (int j = 0; j < 8; ++j)
					{
						int n0 = el.m_node[ET_PYR[j][0]];
						int n1 = el.m_node[ET_PYR[j][1]];

						vec3d r0 = pm->Node(n0).pos();
						vec3d r1 = pm->Node(n1).pos();

						glx::vertex3d(r0);
						glx::vertex3d(r1);
					}
					break;
				case FE_TRI3:
				case FE_QUAD4:
					for (int i = 0; i < ne; ++i)
					{
						int n0 = el.m_node[i];
						int n1 = el.m_node[(i + 1) % ne];

						vec3d r0 = pm->Node(n0).pos();
						vec3d r1 = pm->Node(n1).pos();

						glx::vertex3d(r0);
						glx::vertex3d(r1);
					}
					break;
				}
			}
		}
		glEnd();

		glPopAttrib();

		if (hasBeamElements)
		{
			// render beam elements
			RenderSelectedBeamElements(rc, po);
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderAllBeamElements(CGLContext& rc, GObject* po)
{
	if (po == nullptr) return;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	CGLView* glview = rc.m_view;
	GLMeshRender& renderer = glview->GetMeshRenderer();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsVisible())
		{
			GPart* pg = po->Part(el.m_gid);
			if (pg->IsVisible())
			{
				switch (el.Type())
				{
				case FE_BEAM2: renderer.RenderBEAM2(&el, pm, true); break;
				case FE_BEAM3: break;
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderUnselectedBeamElements(CGLContext& rc, GObject* po)
{
	CGLView* glview = rc.m_view;
	GLMeshRender& renderer = glview->GetMeshRenderer();

	if (po == nullptr) return;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (!el.IsSelected() && el.IsVisible())
		{
			GPart* pg = po->Part(el.m_gid);
			if (pg->IsVisible())
			{
				switch (el.Type())
				{
				case FE_BEAM2: renderer.RenderBEAM2(&el, pm, true); break;
				case FE_BEAM3: break;
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderSelectedBeamElements(CGLContext& rc, GObject* po)
{
	CGLView* glview = rc.m_view;
	GLMeshRender& renderer = glview->GetMeshRenderer();

	if (po == nullptr) return;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);

	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsSelected() && el.IsVisible())
		{
			GPart* pg = po->Part(el.m_gid);
			if (pg->IsVisible())
			{
				switch (el.Type())
				{
				case FE_BEAM2: renderer.RenderBEAM2(&el, pm, true); break;
				case FE_BEAM3: break;
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderMeshLines(CGLContext& rc, GObject* po)
{
	if ((po == 0) || !po->IsVisible()) return;

	CGLView* glview = rc.m_view;

	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// loop over all elements
	for (int i = 0; i < pm->Elements(); i++)
	{
		const FEElement_& e = pm->ElementRef(i);
		if (e.IsVisible() && (po->Part(e.m_gid)->IsVisible()))
		{
			switch (e.Type())
			{
			case FE_HEX8:
			{
				for (int j = 0; j < 6; j++)
				{
					FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

					if ((pen == 0) || (!pen->IsVisible()))
					{
						glBegin(GL_LINE_LOOP);
						{

							const vec3d& r1 = pm->Node(e.m_node[FTHEX8[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTHEX8[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTHEX8[j][2]]).r;
							const vec3d& r4 = pm->Node(e.m_node[FTHEX8[j][3]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
							glVertex3d(r4.x, r4.y, r4.z);
						}
						glEnd();
					}
				}
			}
			break;
			case FE_HEX20:
			case FE_HEX27:
			{
				for (int j = 0; j < 6; j++)
				{
					FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

					if ((pen == 0) || (!pen->IsVisible()))
					{
						glBegin(GL_LINE_LOOP);
						{

							const vec3d& r1 = pm->Node(e.m_node[FTHEX20[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTHEX20[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTHEX20[j][2]]).r;
							const vec3d& r4 = pm->Node(e.m_node[FTHEX20[j][3]]).r;
							const vec3d& r5 = pm->Node(e.m_node[FTHEX20[j][4]]).r;
							const vec3d& r6 = pm->Node(e.m_node[FTHEX20[j][5]]).r;
							const vec3d& r7 = pm->Node(e.m_node[FTHEX20[j][6]]).r;
							const vec3d& r8 = pm->Node(e.m_node[FTHEX20[j][7]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r5.x, r5.y, r5.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r6.x, r6.y, r6.z);
							glVertex3d(r3.x, r3.y, r3.z);
							glVertex3d(r7.x, r7.y, r7.z);
							glVertex3d(r4.x, r4.y, r4.z);
							glVertex3d(r8.x, r8.y, r8.z);
						}
						glEnd();
					}
				}
			}
			break;
			case FE_PENTA6:
			case FE_PENTA15:
			{
				for (int j = 0; j < 3; j++)
				{
					FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

					if ((pen == 0) || (!pen->IsVisible()))
					{
						glBegin(GL_LINE_LOOP);
						{
							const vec3d& r1 = pm->Node(e.m_node[FTPENTA[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTPENTA[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTPENTA[j][2]]).r;
							const vec3d& r4 = pm->Node(e.m_node[FTPENTA[j][3]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
							glVertex3d(r4.x, r4.y, r4.z);
						}
						glEnd();
					}
				}

				for (int j = 3; j < 5; j++)
				{
					FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));

					if ((pen == 0) || (!pen->IsVisible()))
					{
						glBegin(GL_LINE_LOOP);
						{
							const vec3d& r1 = pm->Node(e.m_node[FTPENTA[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTPENTA[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTPENTA[j][2]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
						}
						glEnd();
					}
				}
			}
			break;
			case FE_PYRA5:
			{
				for (int j = 0; j < 4; j++)
				{
					glBegin(GL_LINE_LOOP);
					{
						const vec3d& r1 = pm->Node(e.m_node[FTPYRA5[j][0]]).r;
						const vec3d& r2 = pm->Node(e.m_node[FTPYRA5[j][1]]).r;
						const vec3d& r3 = pm->Node(e.m_node[FTPYRA5[j][2]]).r;

						glVertex3d(r1.x, r1.y, r1.z);
						glVertex3d(r2.x, r2.y, r2.z);
						glVertex3d(r3.x, r3.y, r3.z);
					}
					glEnd();
				}

				glBegin(GL_LINE_LOOP);
				{
					const vec3d& r1 = pm->Node(e.m_node[FTPYRA5[4][0]]).r;
					const vec3d& r2 = pm->Node(e.m_node[FTPYRA5[4][1]]).r;
					const vec3d& r3 = pm->Node(e.m_node[FTPYRA5[4][2]]).r;
					const vec3d& r4 = pm->Node(e.m_node[FTPYRA5[4][3]]).r;

					glVertex3d(r1.x, r1.y, r1.z);
					glVertex3d(r2.x, r2.y, r2.z);
					glVertex3d(r3.x, r3.y, r3.z);
					glVertex3d(r4.x, r4.y, r4.z);
				}
				glEnd();

			}
			break;

			case FE_PYRA13:
			{
				for (int j = 0; j < 4; j++)
				{
					glBegin(GL_LINE_LOOP);
					{
						const vec3d& r1 = pm->Node(e.m_node[FTPYRA13[j][0]]).r;
						const vec3d& r2 = pm->Node(e.m_node[FTPYRA13[j][1]]).r;
						const vec3d& r3 = pm->Node(e.m_node[FTPYRA13[j][2]]).r;
						const vec3d& r4 = pm->Node(e.m_node[FTPYRA13[j][3]]).r;
						const vec3d& r5 = pm->Node(e.m_node[FTPYRA13[j][4]]).r;
						const vec3d& r6 = pm->Node(e.m_node[FTPYRA13[j][5]]).r;

						glVertex3d(r1.x, r1.y, r1.z);
						glVertex3d(r4.x, r4.y, r4.z);
						glVertex3d(r2.x, r2.y, r2.z);
						glVertex3d(r5.x, r5.y, r5.z);
						glVertex3d(r3.x, r3.y, r3.z);
						glVertex3d(r6.x, r6.y, r6.z);
					}
					glEnd();
				}

				glBegin(GL_LINE_LOOP);
				{
					const vec3d& r1 = pm->Node(e.m_node[FTPYRA13[4][0]]).r;
					const vec3d& r2 = pm->Node(e.m_node[FTPYRA13[4][1]]).r;
					const vec3d& r3 = pm->Node(e.m_node[FTPYRA13[4][2]]).r;
					const vec3d& r4 = pm->Node(e.m_node[FTPYRA13[4][3]]).r;
					const vec3d& r5 = pm->Node(e.m_node[FTPYRA13[4][4]]).r;
					const vec3d& r6 = pm->Node(e.m_node[FTPYRA13[4][5]]).r;
					const vec3d& r7 = pm->Node(e.m_node[FTPYRA13[4][6]]).r;
					const vec3d& r8 = pm->Node(e.m_node[FTPYRA13[4][7]]).r;

					glVertex3d(r1.x, r1.y, r1.z);
					glVertex3d(r5.x, r5.y, r5.z);
					glVertex3d(r2.x, r2.y, r2.z);
					glVertex3d(r6.x, r6.y, r6.z);
					glVertex3d(r3.x, r3.y, r3.z);
					glVertex3d(r7.x, r7.y, r7.z);
					glVertex3d(r4.x, r4.y, r4.z);
					glVertex3d(r8.x, r8.y, r8.z);
				}
				glEnd();

			}
			break;

			case FE_TET4:
			case FE_TET5:
			case FE_TET20:
			{
				for (int j = 0; j < 4; j++)
				{
					FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));
					if ((pen == 0) || (!pen->IsVisible()))
					{
						glBegin(GL_LINE_LOOP);
						{
							const vec3d& r1 = pm->Node(e.m_node[FTTET[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTTET[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTTET[j][2]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r3.x, r3.y, r3.z);
						}
						glEnd();
					}
				}
			}
			break;
			case FE_TET10:
			case FE_TET15:
			{
				for (int j = 0; j < 4; j++)
				{
					FEElement_* pen = (e.m_nbr[j] == -1 ? 0 : pm->ElementPtr(e.m_nbr[j]));
					if ((pen == 0) || (!pen->IsVisible()))
					{
						glBegin(GL_LINE_LOOP);
						{
							const vec3d& r1 = pm->Node(e.m_node[FTTET10[j][0]]).r;
							const vec3d& r2 = pm->Node(e.m_node[FTTET10[j][1]]).r;
							const vec3d& r3 = pm->Node(e.m_node[FTTET10[j][2]]).r;
							const vec3d& r4 = pm->Node(e.m_node[FTTET10[j][3]]).r;
							const vec3d& r5 = pm->Node(e.m_node[FTTET10[j][4]]).r;
							const vec3d& r6 = pm->Node(e.m_node[FTTET10[j][5]]).r;

							glVertex3d(r1.x, r1.y, r1.z);
							glVertex3d(r4.x, r4.y, r4.z);
							glVertex3d(r2.x, r2.y, r2.z);
							glVertex3d(r5.x, r5.y, r5.z);
							glVertex3d(r3.x, r3.y, r3.z);
							glVertex3d(r6.x, r6.y, r6.z);
						}
						glEnd();
					}
				}
			}
			break;
			case FE_QUAD4:
			case FE_QUAD8:
			case FE_QUAD9:
			{
				glBegin(GL_LINE_LOOP);
				{
					const vec3d& r1 = pm->Node(e.m_node[0]).r;
					const vec3d& r2 = pm->Node(e.m_node[1]).r;
					const vec3d& r3 = pm->Node(e.m_node[2]).r;
					const vec3d& r4 = pm->Node(e.m_node[3]).r;

					glVertex3d(r1.x, r1.y, r1.z);
					glVertex3d(r2.x, r2.y, r2.z);
					glVertex3d(r3.x, r3.y, r3.z);
					glVertex3d(r4.x, r4.y, r4.z);
				}
				glEnd();
			}
			break;
			case FE_TRI3:
			case FE_TRI6:
			{
				glBegin(GL_LINE_LOOP);
				{
					const vec3d& r1 = pm->Node(e.m_node[0]).r;
					const vec3d& r2 = pm->Node(e.m_node[1]).r;
					const vec3d& r3 = pm->Node(e.m_node[2]).r;

					glVertex3d(r1.x, r1.y, r1.z);
					glVertex3d(r2.x, r2.y, r2.z);
					glVertex3d(r3.x, r3.y, r3.z);
				}
				glEnd();
			}
			break;
			} // switch
		} // if
	} // for

	glPopAttrib();
}

void CGLModelScene::RenderNormals(CGLContext& rc, GObject* po, double scale)
{
	if (po->IsVisible() == false) return;

	FSMeshBase* pm = po->GetEditableMesh();
	if (pm == 0) return;

	double R = 0.05 * pm->GetBoundingBox().GetMaxExtent() * scale;

	glPushAttrib(GL_LIGHTING);
	glDisable(GL_LIGHTING);

	int NS = po->Faces();
	vector<bool> vis(NS);
	for (int n = 0; n < NS; ++n)
	{
		GFace* gface = po->Face(n);
		vis[n] = po->IsFaceVisible(gface);
	}

	// tag the faces we want to render
	int N = pm->Faces();
	for (int i = 0; i < N; ++i)
	{
		FSFace& face = pm->Face(i);
		bool bvis = ((face.m_gid >= 0) && (face.m_gid < NS) ? vis[face.m_gid] : true);
		if (face.IsVisible() && bvis)
		{
			face.m_ntag = 1;
		}
		else face.m_ntag = 0;
	}

	GLMeshRender render;
	render.RenderNormals(pm, R, 1);
}

//-----------------------------------------------------------------------------
void CGLModelScene::SetMatProps(GMaterial* pm)
{
	if (pm == 0) SetDefaultMatProps();
	else
	{
		GMaterial& m = *pm;
		GLfloat f = 1.f / 255.f;
		//			GLfloat dif[4] = {m.m_diffuse.r*f, m.m_diffuse.g*f, m.m_diffuse.b*f, 1.f}; 
		GLColor a = pm->Ambient();
		GLColor s = pm->Specular();
		GLColor e = pm->Emission();
		GLfloat amb[4] = { a.r * f, a.g * f, a.b * f, 1.f };
		GLfloat spc[4] = { s.r * f, s.g * f, s.b * f, 1.f };
		GLfloat emi[4] = { e.r * f, e.g * f, e.b * f, 1.f };
		//			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , dif);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128 * (GLfloat)m.m_shininess);
	}
}

//-----------------------------------------------------------------------------
void CGLModelScene::SetDefaultMatProps()
{
	//		GLfloat dif[] = {0.8f, 0.8f, 0.8f, 1.f};
	GLfloat amb[] = { 0.8f, 0.8f, 0.8f, 1.f };
	GLfloat rev[] = { 0.8f, 0.6f, 0.6f, 1.f };
	GLfloat spc[] = { 0.0f, 0.0f, 0.0f, 1.f };
	GLfloat emi[] = { 0.0f, 0.0f, 0.0f, 1.f };

	//		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , dif);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, amb);
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, rev);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
}

void CGLModelScene::RenderRigidLabels(CGLContext& rc)
{
	FSModel* fem = m_doc->GetFSModel();
	if (fem == nullptr) return;

	CGLView* glview = rc.m_view;
	VIEW_SETTINGS& view = glview->GetViewSettings();

	vector<GLTAG> vtag;

	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* mat = fem->GetMaterial(i);
		FSMaterial* pm = mat->GetMaterialProperties();
		if (pm && pm->IsRigid())
		{
			GLTAG tag;
			tag.r = pm->GetParamVec3d("center_of_mass");
			tag.ntag = 0;

			string name = mat->GetName();
			int l = name.size(); if (l > 63) l = 63;
			if (l > 0)
			{
				strncpy(tag.sztag, name.c_str(), l);
				tag.sztag[l] = 0;
			}
			else sprintf(tag.sztag, "_no_name");
			vtag.push_back(tag);
		}
	}
	int nsel = vtag.size();
	if (nsel == 0) return;

	glview->RenderTags(vtag);
}
