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
#include "GLHighlighter.h"
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>
#include <GLLib/glx.h>
#include <GLLib/GLMeshRender.h>
#include <FEMLib/FEModelConstraint.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <FEMLib/FELoad.h>
#include <MeshLib/MeshMetrics.h>
#include <ImageLib/RGBImage.h>
#include <QImageReader>

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
	m_objectColor = OBJECT_COLOR_MODE::DEFAULT_COLOR;
}

GLMeshRender& CGLModelScene::GetMeshRenderer() { return m_renderer; }

BOX CGLModelScene::GetBoundingBox()
{
	BOX box;
	if (m_doc) box = m_doc->GetModelBox();
	return box;
}

BOX CGLModelScene::GetSelectionBox()
{
	BOX box;
	if (m_doc)
	{
		FESelection* ps = m_doc->GetCurrentSelection();
		if (ps && ps->Size() != 0)
		{
			box = ps->GetBoundingBox();
		}
	}
	return box;
}

void CGLModelScene::Render(CGLContext& rc)
{
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return;

	CGLView* glview = rc.m_view; assert(glview);
	if (glview == nullptr) return;

	// We don't need this for rendering model docs
	glDisable(GL_COLOR_MATERIAL);

	GLViewSettings& view = glview->GetViewSettings();
	int nitem = m_doc->GetItemMode();

	CGLCamera& cam = *rc.m_cam;
	cam.PositionInScene();

	if (glview->ShowPlaneCut())
	{
		if (glview->PlaneCutMode() == 0)
		{
			// render the plane cut first
			glview->RenderPlaneCut();

			// then turn on the clipping plane before rendering the other geometry
			glClipPlane(GL_CLIP_PLANE0, glview->PlaneCoordinates());
			glEnable(GL_CLIP_PLANE0);
		}
	}

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
	cam.PositionInScene();

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
	cam.PositionInScene();

	// render physics
	if (view.m_brigid) RenderRigidBodies(rc);
	if (view.m_bjoint) { RenderRigidJoints(rc); RenderRigidConnectors(rc); }
	if (view.m_bwall ) RenderRigidWalls(rc);
	if (view.m_bfiber) RenderMaterialFibers(rc);
	if (view.m_blma  ) RenderLocalMaterialAxes(rc);

	// render the selected parts
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

	glDisable(GL_CLIP_PLANE0);

	// show the labels on rigid bodies
	if (view.m_showRigidLabels) RenderRigidLabels(rc);

	// render the tags
	if (view.m_bTags) RenderTags(rc);

	// render the grid
	if (view.m_bgrid ) m_grid.Render(rc);

	// render the image data
	RenderImageData(rc);

	// render the decorations
	glview->RenderDecorations();

	// render the highlights
	GLHighlighter::draw();

	// render 3D cursor
	if (m_doc->GetItemMode() == ITEM_MESH)
	{
		glview->Render3DCursor();
	}

	// see if we need to draw the legend bar for the mesh inspector
	if (view.m_bcontour)
	{
		GObject* po = m_doc->GetActiveObject();
		FSMesh* pm = (po ? po->GetFEMesh() : nullptr);
		if (pm)
		{
			Mesh_Data& data = pm->GetMeshData();
			double vmin, vmax;
			data.GetValueRange(vmin, vmax);
			if (vmin == vmax) vmax++;
			if (rc.m_view)
			{
				rc.m_view->setLegendRange((float)vmin, (float)vmax);
			}
			m_doc->ShowLegend(true);
		}
	}
	else m_doc->ShowLegend(false);
}

void TagFaces(GFaceList& faceList, int tag)
{
	std::vector<GFace*> faces = faceList.GetFaceList();
	for (auto pf : faces) pf->m_ntag = tag;
}

// TODO: This is currently called every time we render the scene (with color mode set to physics)!
void TagFacesByPhysics(FSModel& fem)
{
	GModel& model = fem.GetModel();

	// Clear all the tags
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid())
		{
			for (int j = 0; j < po->Faces(); ++j)
			{
				GFace* pf = po->Face(j);
				pf->m_ntag = 0;
			}
		}
	}

	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j < step->BCs(); ++j)
		{
			FSBoundaryCondition* pbc = step->BC(j);
			GFaceList* faceList = dynamic_cast<GFaceList*>(pbc->GetItemList());
			if (faceList) TagFaces(*faceList, 1);
		}
		for (int j = 0; j < step->ICs(); ++j)
		{
			FSInitialCondition* pic = step->IC(j);
			GFaceList* faceList = dynamic_cast<GFaceList*>(pic->GetItemList());
			if (faceList) TagFaces(*faceList, 2);
		}
		for (int j = 0; j < step->Loads(); ++j)
		{
			FSLoad* pl = step->Load(j);
			GFaceList* faceList = dynamic_cast<GFaceList*>(pl->GetItemList());
			if (faceList) TagFaces(*faceList, 3);
		}

		for (int j = 0; j < step->Interfaces(); ++j)
		{
			FSPairedInterface* pi = dynamic_cast<FSPairedInterface*>(step->Interface(j));
			if (pi)
			{
				GFaceList* faceList = dynamic_cast<GFaceList*>(pi->GetItemList(0));
				if (faceList) TagFaces(*faceList, 4);

				faceList = dynamic_cast<GFaceList*>(pi->GetItemList(1));
				if (faceList) TagFaces(*faceList, 5);
			}
		}
	}
}

void CGLModelScene::RenderModel(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;
	FSModel* ps = pdoc->GetFSModel();
	if (ps == nullptr) return;
	GModel& model = ps->GetModel();

	// we don't use backface culling when drawing
	glDisable(GL_CULL_FACE);
	
	GLViewSettings& view = rc.m_settings;
	if (m_objectColor == OBJECT_COLOR_MODE::PHYSICS_TYPE)
	{
		// Tag all faces depending on how they are used in a model component
		TagFacesByPhysics(*ps);
	}

	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid())
		{
			glPushMatrix();
			SetModelView(po);
			RenderGObject(rc, po);
			glPopMatrix();
		}
	}
}

void CGLModelScene::RenderGObject(CGLContext& rc, GObject* po)
{
	CModelDocument* pdoc = m_doc;
	GLViewSettings& view = rc.m_settings;

	CGLView* glview = rc.m_view;

	CGLCamera& cam = *rc.m_cam;
	
	// Get the item mode
	int item = pdoc->GetItemMode();

	// get the selection mode
	int nsel = pdoc->GetSelectionMode();

	GObject* poa = pdoc->GetActiveObject();

	if (item == ITEM_MESH)
	{
		switch (nsel)
		{
		case SELECT_OBJECT:
		{
			if (view.m_bcontour && (poa == po))
			{
				if (po->GetFEMesh()) RenderFEElements(rc, po);
				else if (po->GetEditableMesh()) RenderSurfaceMeshFaces(rc, po);
				else RenderObject(rc, po);
			}
			else if (m_objectColor == OBJECT_COLOR_MODE::FSELEMENT_TYPE)
			{
				if (po->GetFEMesh()) RenderFEElements(rc, po);
				else RenderObject(rc, po);
			}
			else if (glview->ShowPlaneCut() && (glview->PlaneCutMode() == Planecut_Mode::HIDE_ELEMENTS))
			{
				RenderFEElements(rc, po);

				GLColor c = view.m_meshColor;
				glColor4ub(c.r, c.g, c.b, c.a);
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
			cam.PositionInScene();
			SetModelView(po);
			RenderEdges(rc, po);
			cam.LineDrawMode(false);
			cam.PositionInScene();
			SetModelView(po);
		}
		break;
		case SELECT_NODE:
		{
			RenderObject(rc, po);
			cam.LineDrawMode(true);
			cam.PositionInScene();
			SetModelView(po);
			RenderNodes(rc, po);
			cam.LineDrawMode(false);
			cam.PositionInScene();
			SetModelView(po);
		}
		break;
		case SELECT_DISCRETE:
		{
			RenderObject(rc, po);
		}
		break;
		}
	}
	else
	{
		// get the mesh mode
		int meshMode = m_doc->GetMeshMode();

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
					GMesh* gm = po->GetFERenderMesh();
					if (gm) RenderFEFacesFromGMesh(rc, po);
					else RenderFEFaces(rc, po);
					cam.LineDrawMode(true);
					cam.PositionInScene();
					SetModelView(po);
					RenderFEEdges(rc, po);
					cam.LineDrawMode(false);
					cam.PositionInScene();
				}
				else if (item == ITEM_NODE)
				{
					GMesh* gm = po->GetFERenderMesh();
					if (gm) RenderFEFacesFromGMesh(rc, po);
					else RenderFEFaces(rc, po);
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
					cam.PositionInScene();
					SetModelView(po);
					RenderSurfaceMeshEdges(rc, po);
					cam.LineDrawMode(false);
					cam.PositionInScene();
				}
				else if (item == ITEM_NODE)
				{
					RenderSurfaceMeshFaces(rc, po);
					RenderSurfaceMeshNodes(rc, po);
				}
			}
		}
		else RenderObject(rc, po);
	}

	// render normals if requested
	if (view.m_bnorm) RenderNormals(rc, po, view.m_scaleNormals);
}

void CGLModelScene::RenderSelectionBox(CGLContext& rc)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	CGLView* glview = rc.m_view;
	if (glview == nullptr) return;

	GLViewSettings& view = glview->GetViewSettings();

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
						glx::renderBox(po->GetLocalBox(), true, 1.025);
					}
				}
				else if (po == poa)
				{
					glColor3ub(164, 0, 164);
					assert(po->IsSelected());
					glx::renderBox(po->GetLocalBox(), true, 1.025);
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
		glx::renderBox(poa->GetLocalBox(), true, 1.025);
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

			// We'll position the rigid body glyph, either in the center of rigid part,
			// or in the center_of_mass parameter if the override_com is true.
			vec3d r(0, 0, 0);
			bool b = pm->GetParamBool("override_com");
			if (b) r = pm->GetParamVec3d("center_of_mass");
			else r = pgm->GetPosition();

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
	void RenderFiber(GObject* po, FSMaterial* pmat, FEElementRef& rel, const vec3d& c, mat3d Q = mat3d::identity());
	void RenderFiber(GObject* po, FSMaterialProperty* pmat, FEElementRef& rel, const vec3d& c, mat3d Q = mat3d::identity());

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

void GLFiberRenderer::RenderFiber(GObject* po, FSMaterial* pmat, FEElementRef& rel, const vec3d& c, mat3d Q)
{
	if (pmat->HasFibers())
	{
		vec3d q0 = pmat->GetFiber(rel);

		vec3d q = Q * q0;

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
			uint8_t r = (uint8_t)(255 * fabs(q.x));
			uint8_t g = (uint8_t)(255 * fabs(q.y));
			uint8_t b = (uint8_t)(255 * fabs(q.z));
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

	if (pmat->HasMaterialAxes())
	{
		Q = Q*pmat->GetMatAxes(rel);
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
				RenderFiber(po, matj, rel, c, Q);
			}
			else
			{
				FSMaterialProperty* matProp = dynamic_cast<FSMaterialProperty*>(pmat->GetProperty(i).GetComponent(j));
				if (matProp)
				{
					if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
					RenderFiber(po, matProp, rel, c, Q);
				}
			}
		}
	}
}

void GLFiberRenderer::RenderFiber(GObject* po, FSMaterialProperty* pmat, FEElementRef& rel, const vec3d& c, mat3d Q)
{
	if (pmat->HasFibers())
	{
		vec3d q0 = pmat->GetFiber(rel);

		vec3d q = Q * q0;

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
			uint8_t r = (uint8_t)(255 * fabs(q.x));
			uint8_t g = (uint8_t)(255 * fabs(q.y));
			uint8_t b = (uint8_t)(255 * fabs(q.z));
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
			FSMaterial* matj = dynamic_cast<FSMaterial*>(pmat->GetProperty(i).GetComponent(j));
			if (matj)
			{
				if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
				RenderFiber(po, matj, rel, c, Q);
			}
			else
			{
				FSMaterialProperty* matProp = dynamic_cast<FSMaterialProperty*>(pmat->GetProperty(i).GetComponent(j));
				if (matProp)
				{
					if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
					RenderFiber(po, matProp, rel, c, Q);
				}
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

	GLViewSettings& view = glview->GetViewSettings();

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

	GLViewSettings& view = glview->GetViewSettings();
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
						else if (pmat && pmat->HasMaterialAxes())
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

	// build a lookup table for GNodes
	vector<GNode*> nodes; nodes.reserve(1024);
	int minId = -1, maxId = -1;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		for (int j = 0; j < po->Nodes(); ++j)
		{
			GNode* nj = po->Node(j);
			int nid = nj->GetID();
			if (nid != -1)
			{
				if ((minId == -1) || (nid < minId)) minId = nid;
				if ((maxId == -1) || (nid > maxId)) maxId = nid;
				nodes.push_back(nj);
			}
		}
	}

	int nsize = maxId - minId + 1;
	vector<GNode*> lut(nsize, nullptr);
	for (int i = 0; i < nodes.size(); ++i)
	{
		GNode* ni = nodes[i];
		int nid = ni->GetID();
		lut[nid - minId] = ni;
	}	

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
				GNode* pn0 = lut[ps->m_node[0] - minId];
				GNode* pn1 = lut[ps->m_node[1] - minId];
				if (pn0 && pn1) RenderLine(*pn0, *pn1);
			}

			GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(po);
			if (pg)
			{
				GNode* pn0 = lut[pg->m_node[0] - minId];
				GNode* pn1 = lut[pg->m_node[1] - minId];
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

					GNode* pn0 = lut[el.Node(0) - minId];
					GNode* pn1 = lut[el.Node(1) - minId];
					if (pn0 && pn1) RenderLine(*pn0, *pn1);
				}
			}

			GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(po);
			if (ds)
			{
				GNode* pn0 = lut[ds->NodeID(0) - minId];
				GNode* pn1 = lut[ds->NodeID(1) - minId];
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

	GLMeshRender& renderer = GetMeshRenderer();

	GModel& model = *pdoc->GetGModel();
	int nitem = pdoc->GetItemMode();

	GLViewSettings& vs = rc.m_settings;
	GLColor c = vs.m_meshColor;
	glEnable(GL_COLOR_MATERIAL);
	glColor4ub(c.r, c.g, c.b, c.a);

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
				{
					GMesh* lineMesh = po->GetFERenderMesh();
					if (lineMesh) renderer.RenderMeshLines(*lineMesh);
					else renderer.RenderMeshLines(pm);
				}
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

	GLMeshRender& renderer = GetMeshRenderer();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
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

			GMesh* m = po->GetRenderMesh();
			if (m)
			{
				renderer.RenderGLEdges(m);
				renderer.RenderOutline(rc, m, (rc.m_settings.m_nrender == RENDER_WIREFRAME));
			}

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

#ifndef NDEBUG
	// Draw FE nodes on top of GMesh nodes to make sure they match
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		glColor3ub(255, 0, 0);
		m_renderer.RenderFENodes(*pm, [&](const FSNode& node) {
			if (node.m_gid > -1)
			{
				GNode& gn = *po->Node(node.m_gid);
				if (gn.IsSelected()) return true;
			}
			return false;
			});
	}
#endif

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render non-selected edges
void CGLModelScene::RenderEdges(CGLContext& rc, GObject* po)
{
	GMesh* m = po->GetRenderMesh();
	if (m == nullptr) return;

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 255);

	GLMeshRender& renderer = GetMeshRenderer();

	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected() == false)
		{
			renderer.RenderGLEdges(m, i);
		}
	}
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// render selected edges
void CGLModelScene::RenderSelectedEdges(CGLContext& rc, GObject* po)
{
	GMesh* m = po->GetRenderMesh();
	if (m == nullptr) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	vec3d r1, r2;

	GLMeshRender& renderer = GetMeshRenderer();

	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected())
		{
			renderer.RenderGLEdges(m, i);

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

#ifndef NDEBUG
	// Render FE edges onto GMesh edges to make sure they are consistent
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		glColor3ub(255, 0, 0);
		m_renderer.RenderFEEdges(*pm, [&](const FSEdge& edge) {
			if (edge.m_gid > -1)
			{
				GEdge* ge = po->Edge(edge.m_gid);
				if (ge && ge->IsSelected()) return true;
			}
			return false;
			});
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

	GLMeshRender& renderer = GetMeshRenderer();

	GLViewSettings& vs = rc.m_settings;

	// get the GMesh
	FSModel& fem = *doc->GetFSModel();
	GMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == nullptr) return;

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
				if (m_objectColor == OBJECT_COLOR_MODE::PHYSICS_TYPE)
				{
					SetDefaultMatProps();
					GLfloat col[] = { 0.f, 0.f, 0.f, 1.f };
					switch (f.m_ntag)
					{
					case 0: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.9f; glEnable(GL_POLYGON_STIPPLE); break;
					case 1: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.0f; break;	// boundary conditions
					case 2: col[0] = 0.0f; col[1] = 0.4f; col[2] = 0.0f; break;	// initial conditions
					case 3: col[0] = 0.0f; col[1] = 0.9f; col[2] = 0.9f; break;	// loads
					case 4: col[0] = 0.9f; col[1] = 0.0f; col[2] = 0.9f; break;	// contact primary
					case 5: col[0] = 0.3f; col[1] = 0.0f; col[2] = 0.3f; break;	// contact secondary
					}
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
				}
				else SetMatProps(rc, pg);

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

				if ((vs.m_transparencyMode != 0) ||
					(m_objectColor == OBJECT_COLOR_MODE::PHYSICS_TYPE))
					glDisable(GL_POLYGON_STIPPLE);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Render selected surfaces
void CGLModelScene::RenderSelectedSurfaces(CGLContext& rc, GObject* po)
{
	if (!po->IsVisible()) return;

	GLMeshRender& renderer = GetMeshRenderer();

	GMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == nullptr) return;

	int NF = po->Faces();
	vector<int> selectedSurfaces; selectedSurfaces.reserve(NF);
	for (int i = 0; i < NF; ++i)
	{
		GFace& f = *po->Face(i);
		if (f.IsSelected())
		{
			selectedSurfaces.push_back(i);
		}
	}
	if (selectedSurfaces.empty()) return;

	// render the selected faces
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	{
		renderer.SetRenderMode(GLMeshRender::SelectionMode);
		glColor3ub(0, 0, 255);
		for (int surfId : selectedSurfaces)
		{
			renderer.RenderGLMesh(pm, surfId);
		}

#ifndef NDEBUG
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

			GLTriMesh mesh;
			mesh.Create(pm->Faces() * 6); // each face can have a max of 6 * 3 vertices 
			mesh.BeginMesh();
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
						case 10:
							mesh.AddTriangle(rf[0], rf[1], rf[2]);
							break;
						case 4:
							mesh.AddTriangle(rf[0], rf[1], rf[2]);
							mesh.AddTriangle(rf[2], rf[3], rf[0]);
							break;
						case 6:
						case 7:
							mesh.AddTriangle(rf[0], rf[3], rf[5]);
							mesh.AddTriangle(rf[3], rf[1], rf[4]);
							mesh.AddTriangle(rf[5], rf[4], rf[2]);
							mesh.AddTriangle(rf[3], rf[4], rf[5]);
							break;
						case 8:
						case 9:
							mesh.AddTriangle(rf[0], rf[4], rf[7]);
							mesh.AddTriangle(rf[1], rf[5], rf[4]);
							mesh.AddTriangle(rf[2], rf[6], rf[5]);
							mesh.AddTriangle(rf[3], rf[7], rf[6]);
							mesh.AddTriangle(rf[4], rf[6], rf[7]);
							mesh.AddTriangle(rf[4], rf[5], rf[6]);
							break;
						}
					}
				}
			}
			mesh.EndMesh();
			mesh.Render();
		}
#endif
		glDisable(GL_POLYGON_STIPPLE);
	}
	glPopAttrib();

	// render the selected faces
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	{
		renderer.SetRenderMode(GLMeshRender::OutlineMode);
		glColor3ub(0, 0, 255);
		for (int surfId : selectedSurfaces)
		{
			renderer.RenderSurfaceOutline(rc, pm, po->GetTransform(), surfId);
		}
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

	GLMeshRender& renderer = GetMeshRenderer();

	GLViewSettings& vs = rc.m_settings;

	// get the GMesh
	FSModel& fem = *doc->GetFSModel();
	GMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == nullptr) return;

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
			SetMatProps(rc, pg);

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
	GMesh* m = po->GetRenderMesh();
	if (m == nullptr) return;

	int NF = po->Faces();
	vector<int> facesToRender; facesToRender.reserve(NF);
	for (int i = 0; i < NF; ++i)
	{
		GFace* pf = po->Face(i);
		GPart* p0 = po->Part(pf->m_nPID[0]);
		GPart* p1 = po->Part(pf->m_nPID[1]);
		GPart* p2 = po->Part(pf->m_nPID[2]);
		if ((p0 && p0->IsSelected()) || (p1 && p1->IsSelected()) || (p2 && p2->IsSelected()))
		{
			facesToRender.push_back(i);
		}
	}
	if (facesToRender.empty()) return;

	GLMeshRender& renderer = GetMeshRenderer();

	glPushAttrib(GL_ENABLE_BIT);
	{
		renderer.SetRenderMode(GLMeshRender::SelectionMode);
		SetMatProps(0);
		glColor3ub(0, 0, 255);
		for (int surfId : facesToRender)
		{
			renderer.RenderGLMesh(m, surfId);
		}
	}
	glPopAttrib();

	glPushAttrib(GL_ENABLE_BIT);
	{
		renderer.SetRenderMode(GLMeshRender::OutlineMode);
		SetMatProps(0);
		glColor3ub(0, 0, 200);
		for (int surfId : facesToRender)
		{
			renderer.RenderSurfaceOutline(rc, m, po->GetTransform(), surfId);
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

	GLViewSettings& vs = glview->GetViewSettings();

	// get the GMesh
	FSModel& fem = *doc->GetFSModel();
	GMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == 0) return;

	GLMeshRender& renderer = GetMeshRenderer();

	if (vs.m_use_environment_map) ActivateEnvironmentMap();

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
				if (m_objectColor == OBJECT_COLOR_MODE::PHYSICS_TYPE)
				{
					SetDefaultMatProps();
					GLfloat col[] = { 0.f, 0.f, 0.f, 1.f };
					switch (f.m_ntag)
					{ 
					case 0: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.9f; glEnable(GL_POLYGON_STIPPLE); break;
					case 1: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.0f; break;	// boundary conditions
					case 2: col[0] = 0.0f; col[1] = 0.4f; col[2] = 0.0f; break;	// initial conditions
					case 3: col[0] = 0.0f; col[1] = 0.9f; col[2] = 0.9f; break;	// loads
					case 4: col[0] = 0.9f; col[1] = 0.0f; col[2] = 0.9f; break;	// contact
					case 5: col[0] = 0.3f; col[1] = 0.0f; col[2] = 0.3f; break;	// contact secondary
					}
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
				}
				else SetMatProps(rc, pg);

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

				if ((vs.m_transparencyMode != 0) ||
					(m_objectColor == OBJECT_COLOR_MODE::PHYSICS_TYPE))
					glDisable(GL_POLYGON_STIPPLE);
			}
		}
	}

	if (vs.m_use_environment_map) DeactivateEnvironmentMap();

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

	GLViewSettings& vs = rc.m_settings;

	// get the GMesh
	FSModel& fem = *doc->GetFSModel();
	GMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == 0) return;

	GLMeshRender& renderer = GetMeshRenderer();

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
			SetMatProps(rc, pg);

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

	GLViewSettings& view = rc.m_settings;
	quatd q = rc.m_cam->GetOrientation();

	GLMeshRender& renderer = GetMeshRenderer();

	// set the point size
	float fsize = view.m_node_size;
	renderer.SetPointSize(fsize);

	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		int N = pm->Nodes();
		for (int i = 0; i < N; ++i)
		{
			FSNode& node = pm->Node(i);
			if (!node.IsVisible() ||
				(view.m_bext && !node.IsExterior())) node.m_ntag = 0;
			else node.m_ntag = 1;
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

void CGLModelScene::RenderFEFacesFromGMesh(CGLContext& rc, GObject* po)
{
	GMesh* gm = po->GetFERenderMesh(); assert(gm);
	if (gm == nullptr) return;

	GLViewSettings& vs = rc.m_settings;

	SetDefaultMatProps();
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	switch (m_objectColor)
	{
	case OBJECT_COLOR_MODE::DEFAULT_COLOR : RenderMeshByDefault     (rc, *po, *gm); break;
	case OBJECT_COLOR_MODE::OBJECT_COLOR  : RenderMeshByObjectColor (rc, *po, *gm); break;
	case OBJECT_COLOR_MODE::MATERIAL_TYPE : RenderMeshByMaterialType(rc, *po, *gm); break;
	case OBJECT_COLOR_MODE::PHYSICS_TYPE  : RenderMeshByPhysics     (rc, *po, *gm); break;
	case OBJECT_COLOR_MODE::FSELEMENT_TYPE: RenderMeshByElementType (rc, *po, *gm); break;
	default:
		m_renderer.RenderGLMesh(gm);
	}
}

void CGLModelScene::RenderMeshByDefault(CGLContext& rc, GObject& o, GMesh& mesh)
{
	if ((m_doc == nullptr) || !m_doc->IsValid()) return;
	FSModel* fem = m_doc->GetFSModel();

	glDisable(GL_COLOR_MATERIAL);
	SetDefaultMatProps();

	GLColor c;
	int NF = o.Faces();
	for (int i = 0; i < NF; ++i)
	{
		GFace* face = o.Face(i);
		if (face->IsVisible())
		{
			GPart* pg = o.Part(face->m_nPID[0]);
			if (!pg->IsVisible() && (face->m_nPID[1] >= 0))
				pg = o.Part(face->m_nPID[1]);

			GMaterial* gmat = fem->GetMaterialFromID(pg->GetMaterialID());
			SetMatProps(gmat);

			m_renderer.RenderGLMesh(&mesh, i);
		}
	}
}

void CGLModelScene::RenderMeshByObjectColor(CGLContext& rc, GObject& o, GMesh& mesh)
{
	m_renderer.RenderGLMesh(mesh, o.GetColor());
}

void CGLModelScene::RenderMeshByMaterialType(CGLContext& rc, GObject& o, GMesh& mesh)
{
	if ((m_doc == nullptr) || !m_doc->IsValid()) return;
	FSModel* fem = m_doc->GetFSModel();

	glDisable(GL_COLOR_MATERIAL);
	SetDefaultMatProps();

	GLColor c;
	int NF = o.Faces();
	for (int i = 0; i < NF; ++i)
	{
		GFace* face = o.Face(i);
		if (face->IsVisible())
		{
			GPart* pg = o.Part(face->m_nPID[0]);
			if (!pg->IsVisible() && (face->m_nPID[1] >= 0))
				pg = o.Part(face->m_nPID[1]);

			GMaterial* gmat = fem->GetMaterialFromID(pg->GetMaterialID());
			if (gmat == nullptr) c = GLColor(200, 200, 200);
			else
			{
				FSMaterial* pm = gmat->GetMaterialProperties();
				if (pm == nullptr) c = GLColor(0, 0, 0);
				else if (pm->IsRigid()) c = GLColor(210, 200, 164);
				else c = GLColor(200, 128, 128);
			}
			GLfloat col[] = { 0.f, 0.f, 0.f, 1.f };
			col[0] = (float)c.r / 255.f;
			col[1] = (float)c.g / 255.f;
			col[2] = (float)c.b / 255.f;
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);

			m_renderer.RenderGLMesh(&mesh, i);
		}
	}
}

void CGLModelScene::RenderMeshByPhysics(CGLContext& rc, GObject& o, GMesh& mesh)
{
	SetDefaultMatProps();
	glDisable(GL_COLOR_MATERIAL);

	int NF = o.Faces();
	for (int i = 0; i < NF; ++i)
	{
		GFace* face = o.Face(i);
		if (face->IsVisible())
		{
			GPart* pg = o.Part(face->m_nPID[0]);
			if (!pg->IsVisible() && (face->m_nPID[1] >= 0))
				pg = o.Part(face->m_nPID[1]);

			GLfloat col[] = { 0.f, 0.f, 0.f, 1.f };
			switch (face->m_ntag)
			{
			case 0: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.9f; glEnable(GL_POLYGON_STIPPLE); break;
			case 1: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.0f; break;	// boundary conditions
			case 2: col[0] = 0.0f; col[1] = 0.4f; col[2] = 0.0f; break;	// initial conditions
			case 3: col[0] = 0.0f; col[1] = 0.9f; col[2] = 0.9f; break;	// loads
			case 4: col[0] = 0.9f; col[1] = 0.0f; col[2] = 0.9f; break;	// contact primary
			case 5: col[0] = 0.3f; col[1] = 0.0f; col[2] = 0.3f; break;	// contact secondary
			}
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);

			m_renderer.RenderGLMesh(&mesh, i);

			if (face->m_ntag == 0)
				glDisable(GL_POLYGON_STIPPLE);
		}
	}
}

void CGLModelScene::RenderMeshByElementType(CGLContext& rc, GObject& o, GMesh& mesh)
{
	FSMesh* pm = o.GetFEMesh();
	if (pm == nullptr) return;

	m_renderer.RenderGLMesh(&mesh, [&](const GMesh::FACE& face) {
		GLColor col;
		if (face.fid >= 0)
		{
			FSFace& f = pm->Face(face.fid);
			FEElement_* pe = pm->ElementPtr(f.m_elem[0].eid);
			if (pe)
			{
				const int a = 212;
				const int b = 106;
				const int d =  53;
				switch (pe->Type())
				{
				case FE_INVALID_ELEMENT_TYPE: col = GLColor(0, 0, 0); break;
				case FE_TRI3   : col = GLColor(0, a, a); break;
				case FE_TRI6   : col = GLColor(0, b, b); break;
				case FE_TRI7   : col = GLColor(0, b, d); break;
				case FE_TRI10  : col = GLColor(0, d, d); break;
				case FE_QUAD4  : col = GLColor(a, a, 0); break;
				case FE_QUAD8  : col = GLColor(b, b, 0); break;
				case FE_QUAD9  : col = GLColor(d, d, 0); break;
				case FE_TET4   : col = GLColor(0, a, 0); break;
				case FE_TET5   : col = GLColor(0, a, 0); break;
				case FE_TET10  : col = GLColor(0, b, 0); break;
				case FE_TET15  : col = GLColor(0, b, 0); break;
				case FE_TET20  : col = GLColor(0, d, 0); break;
				case FE_HEX8   : col = GLColor(a, 0, 0); break;
				case FE_HEX20  : col = GLColor(b, 0, 0); break;
				case FE_HEX27  : col = GLColor(b, 0, 0); break;
				case FE_PENTA6 : col = GLColor(0, 0, a); break;
				case FE_PENTA15: col = GLColor(0, 0, b); break;
				case FE_PYRA5  : col = GLColor(0, 0, a); break;
				case FE_PYRA13 : col = GLColor(0, 0, b); break;
				case FE_BEAM2  : col = GLColor(a, a, a); break;
				case FE_BEAM3  : col = GLColor(b, b, b); break;
				default:
					col = GLColor(255, 255, 255); break;
				}
			}
		}
		glColor3ub(col.r, col.g, col.b);
	});
}

void CGLModelScene::RenderFEFaces(CGLContext& rc, GObject* po)
{
	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	GLMeshRender& renderer = GetMeshRenderer();

	GLViewSettings& view = rc.m_settings;
	FSModel& fem = *doc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		RenderObject(rc, po);
		return;
	}

	GLColor dif = po->GetColor();
	SetMatProps(0);
	glColor3ub(dif.r, dif.g, dif.b);

	Mesh_Data& data = pm->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());

	// render the unselected faces
	if (showContour)
	{
		Post::CColorMap map;
		double vmin, vmax;
		data.GetValueRange(vmin, vmax); 
		map.SetRange((float)vmin, (float)vmax);

		renderer.RenderFEFaces(pm, [&](const FSFace& face, GLColor* c) {

			if (!face.IsSelected() && face.IsVisible())
			{
				FSElement& el = pm->Element(face.m_elem[0].eid);
				GPart* pg = po->Part(el.m_gid);
				if ((pg->IsVisible() == false) && (face.m_elem[1].eid != -1))
				{
					FSElement& el1 = pm->Element(face.m_elem[1].eid);
					pg = po->Part(el1.m_gid);
				}

				if (pg && pg->IsVisible())
				{
					if (data.GetElementDataTag(face.m_elem[0].eid) > 0)
					{
						int fnl[FSElement::MAX_NODES];
						int nn = el.GetLocalFaceIndices(face.m_elem[0].lid, fnl);
						assert(nn == face.Nodes());

						int nf = face.Nodes();
						for (int j = 0; j < nf; ++j)
							c[j] = map.map(data.GetElementValue(face.m_elem[0].eid, fnl[j]));

						// Render the face
						return true;
					}
					else
					{
						GLColor col(212, 212, 212);
						int nf = face.Nodes();
						for (int j = 0; j < nf; ++j) c[j] = col;

						// Render the face
						return true;
					}
				}
			}
			return false;
			});
	}
	else
	{
		GPart* pgmat = nullptr;

		renderer.RenderFEFaces(pm, [&](const FSFace& face) {
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
					if (pg != pgmat)
					{
						SetMatProps(rc, pg);
						pgmat = pg;
					}

					// Render the face
					return true;
				}
			}

			return false;
			});
	}

	// render beam elements
	RenderAllBeamElements(rc, po);

	// render the selected faces
	renderer.PushState();
	{
		renderer.SetRenderMode(GLMeshRender::SelectionMode);
		glColor3ub(255, 0, 0);
		renderer.RenderFEFaces(pm, [](const FSFace& face) { return face.IsSelected(); });

		// render the selected face outline
		renderer.SetRenderMode(GLMeshRender::OutlineMode);
		glColor3ub(255, 255, 0);
		renderer.RenderFEFacesOutline(pm, [](const FSFace& face) { return face.IsSelected(); });
	}
	renderer.PopState();
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

	GLMeshRender& renderer = GetMeshRenderer();

	GLViewSettings& view = rc.m_settings;
	FSModel& fem = *doc->GetFSModel();

	Mesh_Data& data = surfaceMesh->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());

	// render the unselected faces
	if (showContour)
	{
		// Color is determined by data and colormap
		double vmin, vmax;
		data.GetValueRange(vmin, vmax);

		// Create a copy so we can change the range
		Post::CColorMap colorMap = rc.m_view->GetColorMap();
		colorMap.SetRange((float)vmin, (float)vmax);

		SetMatProps(0);
		glEnable(GL_COLOR_MATERIAL);

		renderer.RenderFESurfaceMeshFaces(surfaceMesh, [&](const FSFace& face, GLColor* c) {
			int i = face.m_ntag;

			if (face.IsVisible() && !face.IsSelected())
			{
				int ne = face.Nodes();
				for (int j = 0; j < ne; ++j)
				{
					if (data.GetElementDataTag(i) > 0)
						c[j] = colorMap.map(data.GetElementValue(i, j));
					else
						c[j] = GLColor(212, 212, 212);
				}

				// render the face
				return true;
			}
			return false;
			});
	}
	else
	{
		GLColor col = po->GetColor();
		SetMatProps(0);
		glColor3ub(col.r, col.g, col.b);

		// render the unselected faces
		// Note that we do not render internal faces
		renderer.RenderFEFaces(surfaceMesh, [](const FSFace& face) {
			return (!face.IsSelected() && face.IsVisible());
			});
	}

	// render the selected faces
	// override some settings
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	renderer.SetRenderMode(GLMeshRender::SelectionMode);
	glColor3ub(255, 64, 0);
	renderer.RenderFEFaces(surfaceMesh, [](const FSFace& face) { return face.IsSelected(); });

	// render the selected face outline
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3ub(255, 255, 0);
	renderer.RenderFEFacesOutline(surfaceMesh, [](const FSFace& face) { return face.IsSelected(); });

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderSurfaceMeshEdges(CGLContext& rc, GObject* po)
{
	CModelDocument* doc = m_doc;
	if (doc == nullptr) return;

	GLMeshRender& renderer = GetMeshRenderer();

	GLViewSettings& view = rc.m_settings;
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

	GLMeshRender& renderer = GetMeshRenderer();

	GLViewSettings& view = rc.m_settings;
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

	GLMeshRender& renderer = GetMeshRenderer();

	GLViewSettings& view = rc.m_settings;
	FSModel& fem = *doc->GetFSModel();
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	// render the unselected edges
	glColor4ub(0, 0, 255, 128);
	renderer.RenderUnselectedFEEdges(pm);

	// render the selected edges
	// override some settings
	glDisable(GL_CULL_FACE);
	glColor4ub(255, 0, 0, 128);
	renderer.RenderSelectedFEEdges(pm);

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render the FE elements
void CGLModelScene::RenderFEElements(CGLContext& rc, GObject* po)
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return;

	FSMesh* pm = po->GetFEMesh(); assert(pm);
	if (pm == 0) return;

	GLMeshRender& renderer = GetMeshRenderer();
	GLViewSettings& view = rc.m_settings;

	GLColor dif;

	GLColor col = po->GetColor();

	int nmatid = -1;
	dif = po->GetColor();
	glColor3ub(dif.r, dif.g, dif.b);
	SetMatProps(0);
	int glmode = 0;

	Mesh_Data& data = pm->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());
	
	for (int i = 0; i < pm->Elements(); ++i) pm->Element(i).m_ntag = i;
	vector<int> selectedElements;

	// render the unselected faces
	int NE = pm->Elements();
	bool hasBeamElements = false;
	if (showContour)
	{
		// Color is determined by data and colormap
		double vmin, vmax;
		data.GetValueRange(vmin, vmax);

		// Create a copy so we can change the range
		Post::CColorMap colorMap = rc.m_view->GetColorMap();
		colorMap.SetRange((float)vmin, (float)vmax);
		
		glEnable(GL_COLOR_MATERIAL);

		renderer.RenderFEElements(*pm, [&](const FEElement_& el, GLColor* c) {
				int i = el.m_ntag;
				if (el.IsVisible() && el.IsSelected()) selectedElements.push_back(i);
				if (el.IsBeam()) hasBeamElements = true;

				if (!el.IsSelected() && el.IsVisible())
				{
					GPart* pg = po->Part(el.m_gid);
					if (pg->IsVisible())
					{
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j)
						{
							if (data.GetElementDataTag(i) > 0)
								c[j] = colorMap.map(data.GetElementValue(i, j));
							else
								c[j] = GLColor(212, 212, 212);
						}

						// render the element
						return true;
					}
				}
				return false;
			});
	}
	else if (m_objectColor == OBJECT_COLOR_MODE::FSELEMENT_TYPE)
	{
		glEnable(GL_COLOR_MATERIAL);

		renderer.RenderFEElements(*pm, [&](const FEElement_& el, GLColor* c) {
			int i = el.m_ntag;
			if (el.IsVisible() && el.IsSelected()) selectedElements.push_back(i);
			if (el.IsBeam()) hasBeamElements = true;

			if (!el.IsSelected() && el.IsVisible())
			{
				GPart* pg = po->Part(el.m_gid);
				if (pg->IsVisible())
				{
					GLColor col;
					const int a = 212;
					const int b = 106;
					const int d =  53;
					switch (el.Type())
					{
					case FE_INVALID_ELEMENT_TYPE: col = GLColor(0, 0, 0); break;
					case FE_TRI3   : col = GLColor(0, a, a); break;
					case FE_TRI6   : col = GLColor(0, b, b); break;
					case FE_TRI7   : col = GLColor(0, b, d); break;
					case FE_TRI10  : col = GLColor(0, d, d); break;
					case FE_QUAD4  : col = GLColor(a, a, 0); break;
					case FE_QUAD8  : col = GLColor(b, b, 0); break;
					case FE_QUAD9  : col = GLColor(d, d, 0); break;
					case FE_TET4   : col = GLColor(0, a, 0); break;
					case FE_TET5   : col = GLColor(0, a, 0); break;
					case FE_TET10  : col = GLColor(0, b, 0); break;
					case FE_TET15  : col = GLColor(0, b, 0); break;
					case FE_TET20  : col = GLColor(0, d, 0); break;
					case FE_HEX8   : col = GLColor(a, 0, 0); break;
					case FE_HEX20  : col = GLColor(b, 0, 0); break;
					case FE_HEX27  : col = GLColor(b, 0, 0); break;
					case FE_PENTA6 : col = GLColor(0, 0, a); break;
					case FE_PENTA15: col = GLColor(0, 0, b); break;
					case FE_PYRA5  : col = GLColor(0, 0, a); break;
					case FE_PYRA13 : col = GLColor(0, 0, b); break;
					case FE_BEAM2  : col = GLColor(a, a, a); break;
					case FE_BEAM3  : col = GLColor(b, b, b); break;
					default:
						col = GLColor(255, 255, 255); break;
					}
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) c[j] = col;

					// render the element
					return true;
				}
			}
			return false;
			});

	}
	else
	{
		// color is determined by material
		glDisable(GL_COLOR_MATERIAL);
		GPart* pgmat = nullptr;

		renderer.RenderFEElements(*pm, [&](const FEElement_& el) {
			int i = el.m_ntag;
			if (el.IsVisible() && el.IsSelected()) selectedElements.push_back(i);
			if (el.IsBeam()) hasBeamElements = true;

			if (!el.IsSelected() && el.IsVisible())
			{
				GPart* pg = po->Part(el.m_gid);
				if (pg->IsVisible())
				{
					if (pg != pgmat)
					{
						SetMatProps(rc, pg);
						pgmat = pg;
					}

					// render the element
					return true;
				}
			}
			return false;
			});
	}

	if (hasBeamElements)
	{
		// render beam elements
		RenderUnselectedBeamElements(rc, po);
	}

	// render the selected elements
	if (pdoc == nullptr) return;
	if (selectedElements.empty() == false)
	{
		renderer.PushState();
		
		renderer.SetRenderMode(GLMeshRender::SelectionMode);
		glColor3f(1.f, 0, 0);

		hasBeamElements = false;
		renderer.RenderFEElements(*pm, selectedElements, [&](const FEElement_& el) {
				// check for beams
				if (el.IsBeam()) hasBeamElements = true;
				return true;
			});

		// render a yellow highlight around selected elements
		renderer.SetRenderMode(GLMeshRender::OutlineMode);
		glColor3f(1.f, 1.f, 0);
		renderer.RenderFEElementsOutline(*pm, selectedElements);

		if (hasBeamElements)
		{
			// render beam elements
			RenderSelectedBeamElements(rc, po);
		}
	
		renderer.PopState();
	}
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderAllBeamElements(CGLContext& rc, GObject* po)
{
	if (po == nullptr) return;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	GLMeshRender& renderer = GetMeshRenderer();

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
				case FE_BEAM3: renderer.RenderBEAM3(&el, pm, true); break;
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderUnselectedBeamElements(CGLContext& rc, GObject* po)
{
	GLMeshRender& renderer = GetMeshRenderer();

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
				case FE_BEAM3: renderer.RenderBEAM3(&el, pm, true); break;
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModelScene::RenderSelectedBeamElements(CGLContext& rc, GObject* po)
{
	GLMeshRender& renderer = GetMeshRenderer();

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
				case FE_BEAM3: renderer.RenderBEAM3(&el, pm, true); break;
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

	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	m_renderer.RenderMeshLines(*pm, [=](const FEElement_& el) {
		return (el.IsVisible() && (po->Part(el.m_gid)->IsVisible()));
		});
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

//-----------------------------------------------------------------------------
void CGLModelScene::SetMatProps(CGLContext& rc, GPart* pg)
{
	if (pg == nullptr) return;
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return;

	CGLView* glview = rc.m_view;
	GLViewSettings& vs = glview->GetViewSettings();
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSModel* fem = m_doc->GetFSModel();

	switch (m_objectColor)
	{
	case OBJECT_COLOR_MODE::DEFAULT_COLOR:
	{
		GMaterial* pmat = fem->GetMaterialFromID(pg->GetMaterialID());
		SetMatProps(pmat);
		GLColor c = po->GetColor();
		if (pmat) c = pmat->Diffuse();
		glColor3ub(c.r, c.g, c.b);

		/*		if (pmat && (pmat->m_nrender != 0))
				{
					GLint n[2];
					glmode = glGetIntegerv(GL_POLYGON_MODE, n);
					if (n[1] != GL_LINE) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
		*/
	}
	break;
	case OBJECT_COLOR_MODE::OBJECT_COLOR:
	{
		SetDefaultMatProps();
		GLColor c = po->GetColor();
		GLfloat col[] = { 0.f, 0.f, 0.f, 1.f };
		col[0] = (float)c.r / 255.f;
		col[1] = (float)c.g / 255.f;
		col[2] = (float)c.b / 255.f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
//		glColor3ub(c.r, c.g, c.b);
	}
	break;
	case OBJECT_COLOR_MODE::MATERIAL_TYPE:
	{
		GLColor c;
		GMaterial* gmat = fem->GetMaterialFromID(pg->GetMaterialID());
		if (gmat == nullptr) c = GLColor(200, 200, 200);
		else
		{
			FSMaterial* pm = gmat->GetMaterialProperties();
			if (pm == nullptr) c = GLColor(0, 0, 0);
			else if (pm->IsRigid()) c = GLColor(210, 200, 164);
			else c = GLColor(200, 128, 128);
		}
		GLfloat col[] = { 0.f, 0.f, 0.f, 1.f };
		col[0] = (float)c.r / 255.f;
		col[1] = (float)c.g / 255.f;
		col[2] = (float)c.b / 255.f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
	}
	break;
	case OBJECT_COLOR_MODE::FSELEMENT_TYPE:
	{
		// We should only get here if the object is not active, or it is not meshed
		SetDefaultMatProps();
		GLfloat col[] = { 1.f, 1.f, 1.f, 1.f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
	}
	break;
	case OBJECT_COLOR_MODE::PHYSICS_TYPE:
	{
		SetDefaultMatProps();
		GLfloat col[] = { 1.f, 1.f, 1.f, 1.f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
	}
	break;
	}
}

void CGLModelScene::RenderTags(CGLContext& rc)
{
	if (rc.m_view == nullptr) return;
	GLViewSettings& view = rc.m_settings;

	GObject* po = m_doc->GetActiveObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	FSMeshBase* pmb = pm;
	if (pm == nullptr)
	{
		GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
		if (pso) pmb = pso->GetSurfaceMesh();
		if (pmb == nullptr) return;
	}

	// create the tag array.
	// We add a tag for each selected item
	GLTAG tag;
	vector<GLTAG> vtag;

	// clear the node tags
	pmb->TagAllNodes(0);
	int NN = pmb->Nodes();

	int mode = m_doc->GetItemMode();

	GLColor extcol(255, 255, 0);
	GLColor intcol(255, 0, 0);

	// process elements
	if (view.m_ntagInfo > TagInfoOption::NO_TAG_INFO)
	{
		if ((mode == ITEM_ELEM) && pm)
		{
			int NE = pm->Elements();
			for (int i = 0; i < NE; i++)
			{
				FEElement_& el = pm->Element(i);
				if (el.IsSelected())
				{
					tag.r = pm->LocalToGlobal(pm->ElementCenter(el));
					tag.c = extcol;
					int nid = el.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "E%d", nid);
					vtag.push_back(tag);

					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
				}
			}
		}

		// process faces
		if (mode == ITEM_FACE)
		{
			int NF = pmb->Faces();
			for (int i = 0; i < NF; ++i)
			{
				FSFace& f = pmb->Face(i);
				if (f.IsSelected())
				{
					tag.r = pmb->LocalToGlobal(pmb->FaceCenter(f));
					tag.c = (f.IsExternal() ? extcol : intcol);
					int nid = f.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "F%d", nid);
					vtag.push_back(tag);

					int nf = f.Nodes();
					for (int j = 0; j < nf; ++j) pmb->Node(f.n[j]).m_ntag = 1;
				}
			}
		}

		// process edges
		if (mode == ITEM_EDGE)
		{
			int NC = pmb->Edges();
			for (int i = 0; i < NC; i++)
			{
				FSEdge& edge = pmb->Edge(i);
				if (edge.IsSelected())
				{
					tag.r = pmb->LocalToGlobal(pmb->EdgeCenter(edge));
					tag.c = extcol;
					int nid = edge.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "L%d", nid);
					vtag.push_back(tag);

					int ne = edge.Nodes();
					for (int j = 0; j < ne; ++j) pmb->Node(edge.n[j]).m_ntag = 1;
				}
			}
		}

		// process nodes
		if (mode == ITEM_NODE)
		{
			for (int i = 0; i < NN; i++)
			{
				FSNode& node = pmb->Node(i);
				if (node.IsSelected())
				{
					tag.r = pmb->LocalToGlobal(node.r);
					tag.c = (node.IsExterior() ? extcol : intcol);
					int nid = node.GetID();
					if (nid < 0) nid = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "N%d", nid);
					vtag.push_back(tag);
				}
			}
		}

		// add additional nodes
		if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
		{
			for (int i = 0; i < NN; i++)
			{
				FSNode& node = pmb->Node(i);
				if (node.m_ntag == 1)
				{
					tag.r = pmb->LocalToGlobal(node.r);
					tag.c = (node.IsExterior() ? extcol : intcol);
					int n = node.GetID();
					if (n < 0) n = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "N%d", n);
					vtag.push_back(tag);
				}
			}
		}
	}

	// if we don't have any tags, just return
	if (vtag.empty()) return;

	// limit the number of tags to render
	const int MAX_TAGS = 100;
	int nsel = (int)vtag.size();
	if (nsel > MAX_TAGS) return; // nsel = MAX_TAGS;

	rc.m_view->RenderTags(vtag);
}

void CGLModelScene::RenderRigidLabels(CGLContext& rc)
{
	FSModel* fem = m_doc->GetFSModel();
	if (fem == nullptr) return;

	CGLView* glview = rc.m_view;
	GLViewSettings& view = glview->GetViewSettings();

	vector<GLTAG> vtag;

	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* mat = fem->GetMaterial(i);
		FSMaterial* pm = mat->GetMaterialProperties();
		if (pm && pm->IsRigid())
		{
			GLTAG tag;
			tag.c = GLColor(255, 255, 0);

			// We'll position the rigid body glyph, either in the center of rigid part,
			// or in the center_of_mass parameter if the override_com is true.
			bool b = pm->GetParamBool("override_com");
			if (b) tag.r = pm->GetParamVec3d("center_of_mass");
			else tag.r = mat->GetPosition();

			string name = mat->GetName();
			int l = name.size(); if (l > 63) l = 63;
			if (l > 0)
			{
				strncpy(tag.sztag, name.c_str(), l);
				tag.sztag[l] = 0;
			}
			else snprintf(tag.sztag, sizeof tag.sztag, "_no_name");
			vtag.push_back(tag);
		}
	}
	int nsel = vtag.size();
	if (nsel == 0) return;

	glview->RenderTags(vtag);
}

void CGLModelScene::RenderImageData(CGLContext& rc)
{
	if (m_doc->IsValid() == false) return;

	for (int i = 0; i < m_doc->ImageModels(); ++i)
	{
		CImageModel* img = m_doc->GetImageModel(i);
		if (img->IsActive()) img->Render(rc);
	}
}

void CGLModelScene::SetObjectColorMode(OBJECT_COLOR_MODE colorMode)
{
	m_objectColor = colorMode;
}

OBJECT_COLOR_MODE CGLModelScene::ObjectColorMode() const
{
	return m_objectColor;
}
