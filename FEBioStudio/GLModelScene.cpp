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
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FELoad.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <PostGL/GLVectorRender.h>
#include <ImageLib/ImageModel.h>
#include <FSCore/ColorMapManager.h>

const int HEX_NT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int PEN_NT[8] = { 0, 1, 2, 2, 3, 4, 5, 5 };
const int TET_NT[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
const int PYR_NT[8] = { 0, 1, 2, 3, 4, 4, 4, 4 };

// in MeshTools\lut.cpp
extern int LUT[256][15];
extern int ET_HEX[12][2];
extern int ET_TET[6][2];
extern int ET_PYR[8][2];

const int MAX_FIBER_COLORS = 16;
static GLColor fiberColorPalette[MAX_FIBER_COLORS] = {
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

void SetModelView(GLRenderEngine& re, GObject* po)
{
	// get transform data
	Transform& T = po->GetRenderTransform();
	vec3d r = T.GetPosition();
	vec3d s = T.GetScale();
	quatd q = T.GetRotation();

	// translate mesh
	re.translate(r);

	// orient mesh
	re.rotate(q);

	// scale the mesh
	re.scale(s.x, s.y, s.z);
}

class GLFiberRenderer : public GLVectorRenderer
{
public:
	GLFiberRenderer() {}

	void BuildFiberVectors(GObject* po, FSMaterial* pmat, FEElementRef& rel, const vec3d& c, mat3d Q);
	void BuildFiberVectors(GObject* po, FSMaterialProperty* pmat, FEElementRef& rel, const vec3d& c, mat3d Q);

public:
	int m_colorOption = 0;
	GLColor	m_defaultCol = GLColor(0, 0, 0);
};

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

const GLCamera& GLModelSceneItem::GetCamera() const { return m_scene->GetCamera(); }

CGLModelScene::CGLModelScene(CModelDocument* doc) : m_doc(doc)
{
	m_objectColor = OBJECT_COLOR_MODE::DEFAULT_COLOR;
	m_fiberViz = nullptr;
	m_buildScene = true;
}

BOX CGLModelScene::GetBoundingBox()
{
	BOX box;
	if (m_doc) box = m_doc->GetModelBox();
	return box;
}

void CGLModelScene::Render(GLRenderEngine& engine, GLContext& rc)
{
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return;
	FSModel* ps = m_doc->GetFSModel();
	if (ps == nullptr) return;
	GModel& model = ps->GetModel();

	GLViewSettings& view = rc.m_settings;

	// set the object's render transforms
	UpdateRenderTransforms(rc);

	int nitem = m_doc->GetItemMode();
	if ((view.m_nrender == RENDER_SOLID) || (nitem != ITEM_MESH))
	{
		if (ObjectColorMode() == OBJECT_COLOR_MODE::PHYSICS_TYPE)
		{
			// Tag all faces depending on how they are used in a model component
			FSModel* fsm = GetFSModel();
			TagFacesByPhysics(*fsm);
		}
	}

	// build the scene
	if (m_buildScene)
	{
		BuildScene(rc);
		m_buildScene = false;
	}

//	if (view.m_use_environment_map) ActivateEnvironmentMap(engine);

	// now render it
	GLScene::Render(engine, rc);

/*
	if (view.m_use_environment_map) DeactivateEnvironmentMap(engine);

	// Render 2D stuff
	ClearTags();

	// show the labels on rigid bodies
	if (view.m_showRigidLabels) RenderRigidLabels();

	// render the tags
	if (view.m_bTags) RenderTags(rc);
*/

	// see if we need to draw the legend bar for the mesh inspector
	m_showLegend = false;
	if (view.m_bcontour)
	{
		GObject* po = m_doc->GetActiveObject();
		FSMesh* pm = (po ? po->GetFEMesh() : nullptr);
		if (pm) m_showLegend = true;
	}
}

void CGLModelScene::BuildScene(GLContext& rc)
{
	clear(); // clear the scene

	if ((m_doc == nullptr) || !m_doc->IsValid()) return;

	GLViewSettings& vs = rc.m_settings;
	// add plane cut item
	GLPlaneCutItem* planeCut = new GLPlaneCutItem(this);
	addItem(planeCut);

	// add all objects for solid rendering
	GModel& model = *GetGModel();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		planeCut->addItem(new GLObjectItem(this, po));

		if ((po == GetActiveObject()) && (vs.m_bcontour))
		{
			ColorizeMesh(po);
		}
	}

	planeCut->addItem(new GLDiscreteItem(this));

	planeCut->addItem(new GLSelectionBox(this));

	planeCut->addItem(new GLFeatureEdgesItem(this));

	planeCut->addItem(new GLMeshLinesItem(this));
/*
	if (vs.m_bfiber == false)
	{
		if (m_fiberViz)
		{
			delete m_fiberViz;
			m_fiberViz = nullptr;
		}
	}
	else
	{
		if (m_fiberViz == nullptr)
		{
			BuildFiberViz(rc);
		}
	}
*/
	planeCut->addItem(new GLPhysicsItem(this));

	planeCut->addItem(new GLSelectionItem(this));

	planeCut->addItem(new GLHighlighterItem(this));

	addItem(new GLGridItem(this));

	if (m_doc->ImageModels())
	{
		for (int i = 0; i < m_doc->ImageModels(); ++i)
		{
			CImageModel* img = m_doc->GetImageModel(i);
			addItem(new GL3DImageItem(this, img));
		}
	}
}

void CGLModelScene::UpdateRenderTransforms(GLContext& rc)
{
	FSModel* ps = m_doc->GetFSModel();
	if (ps == nullptr) return;
	GModel& model = ps->GetModel();
	GLViewSettings& view = rc.m_settings;
	if (view.m_explode && (model.Objects() > 1))
	{
		vector<pair<GObject*, double>> obj;
		double R = 0, avg = 0;
		for (int i = 0; i < model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po && po->IsVisible() && po->IsValid())
			{
				double v = i;
				BOX bo = po->GetGlobalBox();
				vec3d c = bo.Center();
				double r = 0;
				switch (view.m_explode_direction)
				{
				case EXPLODE_X: v = c.x; r = bo.Width(); break;
				case EXPLODE_Y: v = c.y; r = bo.Height(); break;
				case EXPLODE_Z: v = c.z; r = bo.Depth(); break;
				}

				avg += v;

				if (r == 0) r = bo.GetMaxExtent();
				if (r == 0) r = 1;
				R += r * 1.5;

				obj.push_back({ po, v });
			}
		}
		if (obj.empty()) return;
		avg /= obj.size();

		std::sort(obj.begin(), obj.end(), [](pair<GObject*, double>& a, pair<GObject*, double>& b) {
			double va = a.second;
			double vb = b.second;
			return (va < vb);
			});

		double z = avg - R * 0.5;
		double s = view.m_explode_strength;
		for (int i = 0; i < obj.size(); ++i)
		{
			GObject* po = obj[i].first;
			double vi = obj[i].second;

			BOX bo = po->GetGlobalBox();

			vec3d n(0, 0, 0);
			double r = 0;
			switch (view.m_explode_direction)
			{
			case EXPLODE_X: n = vec3d(1, 0, 0); r = bo.Width(); break;
			case EXPLODE_Y: n = vec3d(0, 1, 0); r = bo.Height(); break;
			case EXPLODE_Z: n = vec3d(0, 0, 1); r = bo.Depth(); break;
			}
			if (r == 0) r = bo.GetMaxExtent();
			if (r == 0) r = 1;
			r *= 1.5;

			double d = s * (z + r * 0.5 - vi);
			vec3d t = n * d;
			z += r;

			Transform T = po->GetTransform();
			T.Translate(t);
			po->SetRenderTransform(T);
		}
	}
	else
	{
		for (int i = 0; i < model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			po->SetRenderTransform(po->GetTransform());
		}
	}

}

void GLFiberRenderer::BuildFiberVectors(
	GObject* po,
	FSMaterial* pmat, 
	FEElementRef& rel, 
	const vec3d& c, 
	mat3d Q)
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
			q = po->GetRenderTransform().LocalToGlobalNormal(q);
		}

		GLColor col = m_defaultCol;
		if (m_colorOption == 0)
		{
			uint8_t r = (uint8_t)(255 * fabs(q.x));
			uint8_t g = (uint8_t)(255 * fabs(q.y));
			uint8_t b = (uint8_t)(255 * fabs(q.z));
			col = GLColor(r, g, b);
		}

		AddVector({ col, c, q });
	}

	if (pmat->HasMaterialAxes())
	{
		Q = Q * pmat->GetMatAxes(rel);
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
				if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % MAX_FIBER_COLORS];
				BuildFiberVectors(po, matj, rel, c, Q);
			}
			else
			{
				FSMaterialProperty* matProp = dynamic_cast<FSMaterialProperty*>(pmat->GetProperty(i).GetComponent(j));
				if (matProp)
				{
					if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % MAX_FIBER_COLORS];
					BuildFiberVectors(po, matProp, rel, c, Q);
				}
			}
		}
	}
}


void GLFiberRenderer::BuildFiberVectors(
	GObject* po,
	FSMaterialProperty* pmat,
	FEElementRef& rel,
	const vec3d& c,
	mat3d Q)
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
			q = po->GetRenderTransform().LocalToGlobalNormal(q);
		}

		GLColor col = m_defaultCol;
		if (m_colorOption == 0)
		{
			uint8_t r = (uint8_t)(255 * fabs(q.x));
			uint8_t g = (uint8_t)(255 * fabs(q.y));
			uint8_t b = (uint8_t)(255 * fabs(q.z));
			col = GLColor(r, g, b);
		}

		AddVector({ col, c, q });
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
				if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % MAX_FIBER_COLORS];
				BuildFiberVectors(po, matj, rel, c, Q);
			}
			else
			{
				FSMaterialProperty* matProp = dynamic_cast<FSMaterialProperty*>(pmat->GetProperty(i).GetComponent(j));
				if (matProp)
				{
					if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % MAX_FIBER_COLORS];
					BuildFiberVectors(po, matProp, rel, c, Q);
				}
			}
		}
	}
}

void CGLModelScene::UpdateFiberViz()
{
	delete m_fiberViz;
	m_fiberViz = nullptr;
}

void CGLModelScene::BuildFiberViz(GLContext& rc)
{
	if (m_fiberViz == nullptr) m_fiberViz = new GLFiberRenderer();
	else m_fiberViz->Clear();

	// get the model
	FSModel* ps = m_doc->GetFSModel();
	GModel& model = ps->GetModel();

	FEElementRef rel;

	GLViewSettings& view = rc.m_settings;
	m_fiberViz->m_colorOption = view.m_fibColor;

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
							GLMaterial& glm = pgm->GetGLMaterial();
							m_fiberViz->m_defaultCol = glm.diffuse;
						}

						rel.m_nelem = j;
						if (pmat)
						{
							// element center
							vec3d c(0, 0, 0);
							for (int k = 0; k < el.Nodes(); ++k) c += pm->Node(el.m_node[k]).r;
							c /= el.Nodes();

							// to global coordinates
							c = po->GetRenderTransform().LocalToGlobal(c);

							// add it to the pile
							m_fiberViz->BuildFiberVectors(po, pmat, rel, c, mat3d::identity());
						}
					}
				}
			}
		}
	}
}

//=============================================================================
//					Rendering functions for FEMeshes
//=============================================================================

GLColor GetMaterialTypeColor(GMaterial* mat)
{
	GLColor c;
	if (mat == nullptr) return GLColor(205, 205, 205);
	FSMaterial* pm = mat->GetMaterialProperties();
	if (pm == nullptr) c = GLColor::Black();
	else if (pm->IsRigid()) c = GLColor(210, 205, 200);
	else
	{
		const char* sztype = pm->GetTypeString();
		if (sztype)
		{
			if ((strcmp(sztype, "biphasic") == 0) ||
				(strcmp(sztype, "triphasic") == 0) ||
				(strcmp(sztype, "multiphasic") == 0))
			{
				c = GLColor(205, 164, 210);
			}
			else
			{
				c = GLColor(205, 102, 102);
			}
		}
		else c = GLColor::Black();
	}
	return c;
}

GLColor CGLModelScene::GetPartColor(GPart* pg)
{
	if (pg == nullptr) return GLColor(0, 0, 0);
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return GLColor(0,0,0);

	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSModel* fem = m_doc->GetFSModel();

	switch (m_objectColor)
	{
	case OBJECT_COLOR_MODE::DEFAULT_COLOR:
	{
		GLColor c = po->GetColor();
		if (!pg->IsActive())
		{
			c = GLColor(128, 128, 128);
		}
		else
		{
			GMaterial* pmat = fem->GetMaterialFromID(pg->GetMaterialID());
			GLMaterial& glm = pmat->GetGLMaterial();
			if (pmat) c = glm.diffuse;
		}
		return c;
	}
	break;
	case OBJECT_COLOR_MODE::OBJECT_COLOR:
	{
		return po->GetColor();
	}
	break;
	case OBJECT_COLOR_MODE::MATERIAL_TYPE:
	{
		GMaterial* gmat = fem->GetMaterialFromID(pg->GetMaterialID());
		GLColor c = GetMaterialTypeColor(gmat);
		return c;
	}
	break;
	case OBJECT_COLOR_MODE::FSELEMENT_TYPE:
	{
		return GLColor(255, 255, 255);
	}
	break;
	case OBJECT_COLOR_MODE::PHYSICS_TYPE:
	{
		return GLColor(200, 200, 200, 128);
	}
	break;
	}
	return GLColor(0, 0, 0);
}

GLColor CGLModelScene::GetFaceColor(GFace& f)
{
	GLColor c = GLColor::Black();

	GBaseObject* po = f.Object();
	if (po == nullptr) return c;

	// get the part (that is visible)
	int* pid = f.m_nPID;
	GPart* pg = po->Part(pid[0]);
	if (pg && pg->IsVisible() == false)
	{
		if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = nullptr;
		if (pg && (pg->IsVisible() == false)) pg = nullptr;
	}

	// make sure we have a part
	if (pg)
	{
		if (ObjectColorMode()  == OBJECT_COLOR_MODE::PHYSICS_TYPE)
		{
			switch (f.m_ntag)
			{
			case 0: c = GLColor(200, 200, 200, 128); break;
			case 1: c = GLColor(200, 200,   0); break;	// boundary conditions
			case 2: c = GLColor(  0, 100,   0); break;	// initial conditions
			case 3: c = GLColor(  0, 200, 200); break;	// loads
			case 4: c = GLColor(200,   0, 200); break;	// contact primary
			case 5: c = GLColor(100,   0, 100); break;	// contact secondary
			}
		}
		else c = GetPartColor(pg);
	}

	return c;
}

void CGLModelScene::RenderTags(GLContext& rc)
{
	GLViewSettings& view = rc.m_settings;

	GObject* po = m_doc->GetActiveObject();
	if (po == nullptr) return;

	// create the tag array.
	// We add a tag for each selected item
	GLTAG tag;

	int mode = m_doc->GetItemMode();

	GLColor extcol(255, 255, 0);
	GLColor intcol(255, 0, 0);

	// process elements
	FESelection* currentSelection = m_doc->GetCurrentSelection();
	if ((view.m_ntagInfo > TagInfoOption::NO_TAG_INFO) && currentSelection)
	{
		FSLineMesh* mesh = nullptr;
		if (mode == ITEM_ELEM)
		{
			FEElementSelection* selection = dynamic_cast<FEElementSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSMesh* pm = selection->GetMesh(); mesh = pm;
				GObject* po = pm->GetGObject(); assert(po);
				Transform& T = po->GetRenderTransform();
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NE = selection->Count();
				for (int i = 0; i < NE; i++)
				{
					FSElement_& el = *selection->Element(i); assert(el.IsSelected());
					tag.r = T.LocalToGlobal(pm->ElementCenter(el));
					tag.c = extcol;
					int nid = el.GetID();
					if (nid < 0) nid = selection->ElementIndex(i) + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "E%d", nid);
					AddTag(tag);

					if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
					{
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
					}
				}
			}
		}

		// process faces
		if (mode == ITEM_FACE)
		{
			FEFaceSelection* selection = dynamic_cast<FEFaceSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSMeshBase* pm = selection->GetMesh(); mesh = pm;
				GObject* po = pm->GetGObject(); assert(po);
				Transform& T = po->GetRenderTransform();
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NF = selection->Count();
				for (int i = 0; i < NF; ++i)
				{
					FSFace& f = *selection->Face(i); assert(f.IsSelected());
					tag.r = T.LocalToGlobal(pm->FaceCenter(f));
					tag.c = (f.IsExternal() ? extcol : intcol);
					int nid = f.GetID();
					if (nid < 0) nid = selection->FaceIndex(i) + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "F%d", nid);
					AddTag(tag);

					if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
					{
						int nf = f.Nodes();
						for (int j = 0; j < nf; ++j) pm->Node(f.n[j]).m_ntag = 1;
					}
				}
			}
		}

		// process edges
		if (mode == ITEM_EDGE)
		{
			FEEdgeSelection* selection = dynamic_cast<FEEdgeSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSLineMesh* pm = selection->GetMesh(); mesh = pm;
				GObject* po = pm->GetGObject(); assert(po);
				Transform& T = po->GetRenderTransform();
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NC = selection->Size();
				for (int i = 0; i < NC; i++)
				{
					FSEdge& edge = *selection->Edge(i);
					tag.r = T.LocalToGlobal(pm->EdgeCenter(edge));
					tag.c = extcol;
					int nid = edge.GetID();
					if (nid < 0) nid = selection->EdgeIndex(i) + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "L%d", nid);
					AddTag(tag);

					if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
					{
						int ne = edge.Nodes();
						for (int j = 0; j < ne; ++j) pm->Node(edge.n[j]).m_ntag = 1;
					}
				}
			}
		}

		// process nodes
		if (mode == ITEM_NODE)
		{
			FENodeSelection* selection = dynamic_cast<FENodeSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSLineMesh* pm = selection->GetMesh(); mesh = pm;
				GObject* po = pm->GetGObject(); assert(po);
				Transform& T = po->GetRenderTransform();
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NN = selection->Size();
				for (int i = 0; i < NN; i++)
				{
					FSNode& node = *selection->Node(i);
					tag.r = T.LocalToGlobal(node.r);
					tag.c = (node.IsExterior() ? extcol : intcol);
					int nid = node.GetID();
					if (nid < 0) nid = selection->NodeIndex(i) + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "N%d", nid);
					AddTag(tag);
				}
			}
		}

		// add additional nodes
		if ((view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) && mesh)
		{
			GObject* po = mesh->GetGObject(); assert(po);
			Transform& T = po->GetRenderTransform();
			int NN = mesh->Nodes();
			for (int i = 0; i < NN; i++)
			{
				FSNode& node = mesh->Node(i);
				if (node.m_ntag == 1)
				{
					tag.r = T.LocalToGlobal(node.r);
					tag.c = (node.IsExterior() ? extcol : intcol);
					int n = node.GetID();
					if (n < 0) n = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "N%d", n);
					AddTag(tag);
				}
			}
		}
	}
}

void CGLModelScene::RenderRigidLabels()
{
	FSModel* fem = m_doc->GetFSModel();
	if (fem == nullptr) return;

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
			size_t l = name.size(); if (l > 63) l = 63;
			if (l > 0)
			{
				strncpy(tag.sztag, name.c_str(), l);
				tag.sztag[l] = 0;
			}
			else snprintf(tag.sztag, sizeof tag.sztag, "_no_name");
			AddTag(tag);
		}
	}
}

void CGLModelScene::SetObjectColorMode(OBJECT_COLOR_MODE colorMode)
{
	m_objectColor = colorMode;
	Update();
}

OBJECT_COLOR_MODE CGLModelScene::ObjectColorMode() const
{
	return m_objectColor;
}

void CGLModelScene::Update()
{
	m_buildScene = true;
	GLScene::Update();
}

void GLPlaneCutItem::RenderBoxCut(GLRenderEngine& re, const BOX& box)
{
	vec3d a = box.r0();
	vec3d b = box.r1();
	vec3d ex[8];
	ex[0] = vec3d(a.x, a.y, a.z);
	ex[1] = vec3d(b.x, a.y, a.z);
	ex[2] = vec3d(b.x, b.y, a.z);
	ex[3] = vec3d(a.x, b.y, a.z);
	ex[4] = vec3d(a.x, a.y, b.z);
	ex[5] = vec3d(b.x, a.y, b.z);
	ex[6] = vec3d(b.x, b.y, b.z);
	ex[7] = vec3d(a.x, b.y, b.z);
	double R = box.GetMaxExtent();
	if (R == 0) R = 1;
	int ncase = 0;
	const double* plane = m_planeCut.GetPlaneCoordinates();
	vec3d norm(plane[0], plane[1], plane[2]);
	double ref = -(plane[3] - R * 0.001);
	for (int k = 0; k < 8; ++k)
		if (norm * ex[k] > ref) ncase |= (1 << k);
	if ((ncase > 0) && (ncase < 255))
	{
		int edge[15][2], edgeNode[15][2], etag[15];
		GLMesh plane;
		int* pf = LUT[ncase];
		int ne = 0;
		for (int l = 0; l < 5; l++)
		{
			if (*pf == -1) break;
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
				r[k] = to_vec3f(rk);
				plane.AddFace(r);
			}
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

					GLMesh::FACE& face = plane.Face(plane.Faces() - 1);
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
				plane.AddEdge(edgeNode[k], 2, 0);
			}
		}
		plane.Update();

		re.setMaterial(GLMaterial::CONSTANT, GLColor(255, 64, 255));
		re.renderGMeshEdges(plane, false);
	}
}

bool BuildSelectionMesh(FESelection* sel, GLMesh& mesh)
{
	mesh.Clear();
	if (sel == nullptr) return false;

	FEElementSelection* esel = dynamic_cast<FEElementSelection*>(sel);
	if (esel && esel->Count())
	{
		mesh.NewPartition();
		FSMesh* pm = esel->GetMesh();
		int NE = esel->Count();
		int n[FSFace::MAX_NODES];
		for (int i = 0; i < NE; ++i)
		{
			FSElement_& el = *esel->Element(i); assert(el.IsSelected());
			if (el.IsSolid())
			{
				int nf = el.Faces();
				for (int j = 0; j < nf; ++j)
				{
					int nj = el.m_nbr[j];
					FSElement_* pej = pm->ElementPtr(nj);
					if ((pej == nullptr) || (!pej->IsSelected()))
					{
						FSFace f = el.GetFace(j);
						for (int k = 0; k < f.Nodes(); ++k)
						{
							FSNode& nodek = pm->Node(f.n[k]);
							vec3f r = to_vec3f(nodek.r);
							n[k] = mesh.AddNode(r);
							nodek.m_ntag = n[k];
						}
						mesh.AddFace(n, f.Nodes(), 0, -1, true, -1, i);

						for (int k = 0; k < f.Edges(); ++k)
						{
							int en[FSEdge::MAX_NODES];
							FSEdge edge = f.GetEdge(k);
							for (int l = 0; l < edge.Nodes(); ++l)
								en[l] = pm->Node(edge.n[l]).m_ntag;
							mesh.AddEdge(en, edge.Nodes());
						}
					}
				}
			}
			else if (el.IsShell())
			{
				// add shells
				for (int k = 0; k < el.Nodes(); ++k)
				{
					FSNode& nodek = pm->Node(el.m_node[k]);
					vec3f r = to_vec3f(nodek.r);
					n[k] = mesh.AddNode(r);
					nodek.m_ntag = n[k];
				}
				mesh.AddFace(n, el.Nodes(), 0, -1, true, -1, i);

				for (int k = 0; k < el.Edges(); ++k)
				{
					int en[FSEdge::MAX_NODES];
					FSEdge edge = el.GetEdge(k);
					for (int l = 0; l < edge.Nodes(); ++l)
						en[l] = pm->Node(edge.n[l]).m_ntag;
					mesh.AddEdge(en, edge.Nodes());
				}
			}
			else if (el.IsBeam())
			{
				for (int k = 0; k < el.Nodes(); ++k)
				{
					FSNode& nodek = pm->Node(el.m_node[k]);
					vec3f r = to_vec3f(nodek.r);
					n[k] = mesh.AddNode(r);
					nodek.m_ntag = n[k];
				}
				mesh.AddEdge(n, el.Nodes());
			}
		}
		mesh.Update();
	}

	FEFaceSelection* fsel = dynamic_cast<FEFaceSelection*>(sel);
	if (fsel && fsel->Count())
	{
		mesh.NewPartition();
		FSMeshBase* pm = fsel->GetMesh();
		int NF = fsel->Count();
		int n[FSFace::MAX_NODES];
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = *fsel->Face(i); assert(face.IsSelected());
			for (int k = 0; k < face.Nodes(); ++k)
			{
				FSNode& nodek = pm->Node(face.n[k]);
				vec3f r = to_vec3f(nodek.r);
				n[k] = mesh.AddNode(r);
				nodek.m_ntag = n[k];
			}
			mesh.AddFace(n, face.Nodes(), 0, -1, true, i, -1);

			for (int k = 0; k < face.Edges(); ++k)
			{
				FSFace* pf = pm->FacePtr(face.m_nbr[k]);
				if ((pf == nullptr) || !pf->IsSelected() || !pf->IsVisible() || (face.GetID() < pf->GetID()))
				{
					int en[FSEdge::MAX_NODES];
					FSEdge edge = face.GetEdge(k);
					for (int l = 0; l < edge.Nodes(); ++l)
						en[l] = pm->Node(edge.n[l]).m_ntag;

					mesh.AddEdge(en, edge.Nodes());
				}
			}
		}
		mesh.Update();
	}

	FEEdgeSelection* csel = dynamic_cast<FEEdgeSelection*>(sel);
	if (csel)
	{
		FSLineMesh* pm = csel->GetMesh();
		int NE = csel->Count();
		int n[FSEdge::MAX_NODES];
		for (int i = 0; i < NE; ++i)
		{
			FSEdge* edge = csel->Edge(i); assert(edge->IsSelected());
			for (int l = 0; l < edge->Nodes(); ++l)
			{
				vec3f r = to_vec3f(pm->Node(edge->n[l]).r);
				n[l] = mesh.AddNode(r);
			}
			mesh.AddEdge(n, edge->Nodes());
		}
		mesh.Update();
	}

	FENodeSelection* nsel = dynamic_cast<FENodeSelection*>(sel);
	if (nsel)
	{
		FSLineMesh* pm = nsel->GetMesh();
		int NN = nsel->Count();
		for (int i = 0; i < NN; ++i)
		{
			FSNode* node = nsel->Node(i); assert(node->IsSelected());
			vec3f r = to_vec3f(node->r);
			mesh.AddNode(r);
		}
		mesh.Update();
	}

	return true;
}

int CGLModelScene::GetSelectionMode() const
{
	return m_doc->GetSelectionMode();
}

int CGLModelScene::GetItemMode() const
{
	return m_doc->GetItemMode();
}

int CGLModelScene::GetMeshMode() const
{
	return m_doc->GetMeshMode();
}

int CGLModelScene::GetObjectColorMode() const
{
	return m_objectColor;
}

GObject* CGLModelScene::GetActiveObject() const
{
	return m_doc->GetActiveObject();
}

GLFiberRenderer* CGLModelScene::GetFiberRenderer()
{
	return m_fiberViz;
}

FESelection* CGLModelScene::GetCurrentSelection()
{
	return (m_doc ? m_doc->GetCurrentSelection() : nullptr);
}

void CGLModelScene::UpdateSelectionMesh(FESelection* sel)
{
	BuildSelectionMesh(sel, m_selectionMesh);
}

GModel* CGLModelScene::GetGModel()
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return nullptr;
	FSModel* ps = pdoc->GetFSModel();
	if (ps == nullptr) return nullptr;
	return &ps->GetModel();
}

FSModel* CGLModelScene::GetFSModel()
{
	CModelDocument* pdoc = m_doc;
	if (pdoc == nullptr) return nullptr;
	return pdoc->GetFSModel();
}

GLPlaneCutItem::GLPlaneCutItem(CGLModelScene* scene) : m_scene(scene)
{

}

void GLPlaneCutItem::render(GLRenderEngine& re, GLContext& rc)
{
	if (rc.m_settings.m_showPlaneCut)
	{
		BOX box = m_scene->GetBoundingBox();
		glx::renderBox(re, box, GLColor(200, 0, 200), false);

		// render the plane cut first
		if (m_planeCut.IsValid() == false)
		{
			UpdatePlaneCut(rc, true);
		}

		RenderBoxCut(re, box);

		m_planeCut.RenderMesh(rc.m_settings.m_bmesh);
		m_planeCut.SetMeshColor(rc.m_settings.m_meshColor);
		m_planeCut.Render(re);

		// then turn on the clipping plane before rendering the other geometry
		re.setClipPlane(0, rc.m_settings.m_planeCut);
		re.enableClipPlane(0);
	}

	// render the children
	GLCompositeSceneItem::render(re, rc);

	if (rc.m_settings.m_showPlaneCut)
	{
		// turn off clipping plane
		re.disableClipPlane(0);
	}
}

void GLPlaneCutItem::UpdatePlaneCut(GLContext& rc, bool reset)
{
	m_planeCut.Clear();

	FSModel& fem = *m_scene->GetFSModel();

	GModel& mdl = *m_scene->GetGModel();
	if (mdl.Objects() == 0) return;

	double* d = rc.m_settings.m_planeCut;
	m_planeCut.SetPlaneCoordinates(d[0], d[1], d[2], d[3]);

	if (m_scene)
	{
		int nmap = m_scene->GetColorMap();
		m_planeCut.SetColorMap(ColorMapManager::GetColorMap(nmap));
	}

	GLViewSettings& vs = rc.m_settings;
	m_planeCut.Create(fem, vs.m_bcontour, vs.m_planeCutMode);
}

GLObjectItem::GLObjectItem(CGLModelScene* scene, GObject* po) : GLModelSceneItem(scene), m_po(po) 
{
	m_clearCache = true;
	if (m_po)
	{
		UpdateGMeshColor(*m_po->GetRenderMesh());
		UpdateGMeshColor(*m_po->GetFERenderMesh());
	}
}

void GLObjectItem::UpdateGMeshColor(GLMesh& msh)
{
	if (m_po == nullptr) return;
	GLMesh* gm = m_po->GetRenderMesh();

	// color the meshes
	int objColorMode = m_scene->GetObjectColorMode();
	switch (objColorMode)
	{
	case DEFAULT_COLOR : ColorByDefault(*gm); break;
	case OBJECT_COLOR  : ColorByObject(*gm); break;
	case MATERIAL_TYPE : ColorByMaterialType(*gm); break;
	case FSELEMENT_TYPE: ColorByElementType(*gm); break;
	case PHYSICS_TYPE  : ColorByPhysics(*gm); break;
	}
}

void GLObjectItem::ColorByDefault(GLMesh& msh)
{
	GLColor c = m_po->GetColor();
	for (int n = 0; n < msh.Partitions(); ++n)
	{
		const GLMesh::PARTITION& p = msh.Partition(n);
		GFace* pf = m_po->Face(n);
		GLColor c = m_scene->GetFaceColor(*pf);

		for (int i = 0; i < p.nf; ++i)
		{
			GLMesh::FACE& face = msh.Face(i + p.n0);
			face.c[0] = face.c[1] = face.c[2] = c;
		}
	}
}

void GLObjectItem::ColorByObject(GLMesh& msh)
{
	GLColor c = m_po->GetColor();
	for (int i = 0; i < msh.Faces(); ++i)
	{
		GLMesh::FACE& face = msh.Face(i);
		face.c[0] = face.c[1] = face.c[2] = c;
	}
}

void GLObjectItem::ColorByMaterialType(GLMesh& msh)
{
	GLColor c = m_po->GetColor();
	for (int n = 0; n < msh.Partitions(); ++n)
	{
		const GLMesh::PARTITION& p = msh.Partition(n);
		GFace* pf = m_po->Face(n);
		GLColor c = m_scene->GetFaceColor(*pf);

		for (int i = 0; i < p.nf; ++i)
		{
			GLMesh::FACE& face = msh.Face(i + p.n0);
			face.c[0] = face.c[1] = face.c[2] = c;
		}
	}
}

void GLObjectItem::ColorByElementType(GLMesh& msh)
{
	FSMesh* pm = m_po->GetFEMesh();
	if (pm == nullptr)
	{
		ColorByObject(msh);
		return;
	}

	for (int i = 0; i < msh.Faces(); ++i)
	{
		GLMesh::FACE& face = msh.Face(i);

		GLColor col(212, 212, 212);
		if (face.eid >= 0)
		{
			FSElement_* pe = pm->ElementPtr(face.eid);
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
		face.c[0] = face.c[1] = face.c[2] = col;
	}
}

void GLObjectItem::ColorByPhysics(GLMesh& msh)
{
	GLColor c = m_po->GetColor();
	for (int n = 0; n < msh.Partitions(); ++n)
	{
		const GLMesh::PARTITION& p = msh.Partition(n);
		GFace* pf = m_po->Face(n);
		GLColor c = m_scene->GetFaceColor(*pf);

		for (int i = 0; i < p.nf; ++i)
		{
			GLMesh::FACE& face = msh.Face(i + p.n0);
			face.c[0] = face.c[1] = face.c[2] = c;
		}
	}
}

void GLObjectItem::render(GLRenderEngine& re, GLContext& rc)
{
	GLViewSettings& vs = rc.m_settings;
	int nitem = m_scene->GetItemMode();
	if ((vs.m_nrender == RENDER_SOLID) || (nitem != ITEM_MESH))
	{
		if (m_po && m_po->IsValid())
		{
			GLMesh* gm = m_po->GetRenderMesh();
			if (gm && gm->IsModified())
			{
				re.deleteCachedMesh(gm);
				gm->setModified(false);
			}

			gm = m_po->GetFERenderMesh();
			if (gm && gm->IsModified())
			{
				re.deleteCachedMesh(gm);
				gm->setModified(false);
			}

			re.pushTransform();
			SetModelView(re, m_po);
			RenderGObject(re, rc);
			re.popTransform();
		}
	}
}

void GLObjectItem::RenderGObject(GLRenderEngine& re, GLContext& rc)
{
	GLViewSettings& view = rc.m_settings;

	int item = m_scene->GetItemMode();
	int objectColor = m_scene->GetObjectColorMode();

	GObject* po = m_po;
	GObject* poa = m_scene->GetActiveObject();

	if (po != poa)
	{
		RenderObject(re, rc);
		return;
	}

	// get the selection mode
	int nsel = m_scene->GetSelectionMode();

	if (item == ITEM_MESH)
	{
		switch (nsel)
		{
		case SELECT_OBJECT:
		{
			if (view.m_bcontour)
			{
				GLMesh* gm = po->GetFERenderMesh();
				if (gm) RenderFEFacesFromGMesh(re, rc);
				else if (po->GetEditableMesh()) RenderSurfaceMeshFaces(re, rc);
				else RenderObject(re, rc);
			}
			else if (objectColor == OBJECT_COLOR_MODE::FSELEMENT_TYPE)
			{
				GLMesh* gm = po->GetFERenderMesh();
				if (gm) RenderFEFacesFromGMesh(re, rc);
				else RenderObject(re, rc);
			}
			else if (view.m_showPlaneCut && (view.m_planeCutMode == Planecut_Mode::HIDE_ELEMENTS))
			{
				GLMesh* gm = po->GetFERenderMesh();
				if (gm) RenderFEFacesFromGMesh(re, rc);
			}
			else RenderObject(re, rc);
		}
		break;
		case SELECT_PART: RenderParts(re, rc); break;
		case SELECT_FACE: RenderSurfaces(re, rc); break;
		case SELECT_EDGE:
		{
			RenderObject(re, rc);
			SetModelView(re, po);
			RenderEdges(re);
			SetModelView(re, po);
		}
		break;
		case SELECT_NODE:
		{
			RenderObject(re, rc);
			SetModelView(re, po);
			RenderNodes(re);
			SetModelView(re, po);
		}
		break;
		case SELECT_DISCRETE:
		{
			RenderObject(re, rc);
		}
		break;
		}
	}
	else
	{
		// get the mesh mode
		int meshMode = m_scene->GetMeshMode();

		if (meshMode == MESH_MODE_VOLUME)
		{
			if (item == ITEM_ELEM)
			{
				RenderFEFacesFromGMesh(re, rc);
				RenderUnselectedBeamElements(re);
				RenderSelectedFEElements(re);
			}
			else if (item == ITEM_FACE)
			{
				GLMesh* gm = po->GetFERenderMesh(); assert(gm);
				if (gm)
				{
					RenderFEFacesFromGMesh(re, rc);
					RenderAllBeamElements(re);
					RenderSelectedFEFaces(re);
				}
			}
			else if (item == ITEM_EDGE)
			{
				GLMesh* gm = po->GetFERenderMesh(); assert(gm);
				if (gm) RenderFEFacesFromGMesh(re, rc);
				SetModelView(re, po);
				RenderFEEdges(re);
			}
			else if (item == ITEM_NODE)
			{
				GLMesh* gm = po->GetFERenderMesh(); assert(gm);
				if (gm) RenderFEFacesFromGMesh(re, rc);
				RenderFENodes(re, rc);
			}
		}
		else
		{
			if (item == ITEM_FACE)
			{
				RenderSurfaceMeshFaces(re, rc);
			}
			else if (item == ITEM_EDGE)
			{
				RenderSurfaceMeshFaces(re, rc);
				SetModelView(re, po);
				RenderSurfaceMeshEdges(re);
			}
			else if (item == ITEM_NODE)
			{
				RenderSurfaceMeshFaces(re, rc);
				RenderSurfaceMeshNodes(re, rc);
			}
		}
	}

	// render normals if requested
	if (view.m_bnorm) RenderNormals(re, view.m_scaleNormals);
}

// render non-selected parts
void GLObjectItem::RenderParts(GLRenderEngine& re, GLContext& rc)
{
	GLViewSettings& vs = rc.m_settings;

	GObject* po = m_po;

	// get the GLMesh
	FSModel& fem = *m_scene->GetFSModel();
	GLMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == nullptr) return;

	// render non-selected parts
	int NF = po->Faces();
	for (int n = 0; n < NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);
		if (f.IsVisible())
		{
			// get the part (that is visible)
			int* pid = f.m_nPID;
			GPart* pg = po->Part(pid[0]);
			if (pg && pg->IsVisible() == false)
			{
				if (pid[1] >= 0) pg = po->Part(pid[1]); else pg = nullptr;
				if (pg && (pg->IsVisible() == false)) pg = nullptr;
			}

			if (pg && !pg->IsSelected())
			{
				GLColor c = m_scene->GetFaceColor(f);

				bool useStipple = false;
				if (c.a != 255)
				{
					c.a = 255;
					useStipple = true;
				}

				// render the face
				bool frontOnly = vs.m_identifyBackfacing;
				if (useStipple) re.setMaterial(GLMaterial::GLASS, c, GLMaterial::NONE, frontOnly);
				else re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::NONE, frontOnly);
				re.renderGMesh(*pm, n);
			}
		}
	}

	RenderBeamParts(re);
}

// Render non-selected surfaces
void GLObjectItem::RenderSurfaces(GLRenderEngine& re, GLContext& rc)
{
	GLViewSettings& vs = rc.m_settings;

	GObject* po = m_po;

	// get the GLMesh
	FSModel& fem = *m_scene->GetFSModel();
	GLMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == nullptr) return;

	bool frontOnly = rc.m_settings.m_identifyBackfacing;

	// render non-selected faces
	int NF = po->Faces();
	for (int n = 0; n < NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		// make sure this face is not selected
		if (f.IsVisible() && (f.IsSelected() == false))
		{
			GLColor c = m_scene->GetFaceColor(f);

			bool useStipple = false;
			if (c.a != 255)
			{
				useStipple = true;
				c.a = 255;
			}

			// render the face
			if (useStipple) re.setMaterial(GLMaterial::GLASS, c, GLMaterial::NONE, frontOnly);
			else re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::NONE, frontOnly);
			re.renderGMesh(*pm, n);
		}
	}
}

// Render non-selected nodes
void GLObjectItem::RenderNodes(GLRenderEngine& re)
{
	GObject* po = m_po;
	if ((po == nullptr) || (po->Nodes() == 0)) return;

	GLMesh points;
	for (int i = 0; i < po->Nodes(); ++i)
	{
		// only render nodes that are not selected
		// and are not shape-nodes
		GNode& n = *po->Node(i);
		if (!n.IsSelected() && (n.Type() != NODE_SHAPE))
		{
			vec3f r = to_vec3f(n.LocalPosition());
			points.AddNode(r);
		}
	}
	if (points.Nodes() == 0) return;

	re.setMaterial(GLMaterial::CONSTANT, GLColor::Blue());
	re.renderGMeshNodes(points, false);
}

// render non-selected edges
void GLObjectItem::RenderEdges(GLRenderEngine& re)
{
	GObject* po = m_po;
	GLMesh* m = po->GetRenderMesh();
	if (m == nullptr) return;

	re.setMaterial(GLMaterial::CONSTANT, GLColor::Blue());

	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected() == false)
		{
			re.renderGMeshEdges(*m, i);
		}
	}
}

void GLObjectItem::RenderFEFacesFromGMesh(GLRenderEngine& re, GLContext& rc)
{
	GObject* po = m_po;

	GLMesh* gm = po->GetFERenderMesh();
	if (gm == nullptr) return;

	GLViewSettings& vs = rc.m_settings;

	if (vs.m_bcontour && (po == m_scene->GetActiveObject()))
	{
		// We don't use the cached mesh, since it probably won't have the 
		// vertex colors stored, which we need here. 
		re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::VERTEX_COLOR);
		re.renderGMesh(*gm, false);
	}
	else
	{
		int objectColor = m_scene->GetObjectColorMode();
		switch (objectColor)
		{
		case OBJECT_COLOR_MODE::DEFAULT_COLOR : RenderMeshByDefault(re, rc); break;
		case OBJECT_COLOR_MODE::OBJECT_COLOR  : RenderMeshByObjectColor(re, rc); break;
		case OBJECT_COLOR_MODE::MATERIAL_TYPE : RenderMeshByDefault(re, rc); break;
		case OBJECT_COLOR_MODE::PHYSICS_TYPE  : RenderMeshByDefault(re, rc); break;
		case OBJECT_COLOR_MODE::FSELEMENT_TYPE: RenderMeshByElementType(re, rc, *gm); break;
		default:
			assert(false);
		}
	}
}

void GLObjectItem::RenderMeshByDefault(GLRenderEngine& re, GLContext& rc)
{
	GObject* po = m_po;
	GLMesh* gm = po->GetFERenderMesh();
	if (gm == nullptr) return;

	GLViewSettings& vs = rc.m_settings;

	int objectColor = m_scene->GetObjectColorMode();

	GPart* pgmat = nullptr; // the part that defines the material
	int NF = po->Faces();
	for (int n = 0; n < NF; ++n)
	{
		// get the next face
		GFace& f = *po->Face(n);

		GLColor c = m_scene->GetFaceColor(f);

		bool useStipple = false;
		if (c.a != 255)
		{
			useStipple = true;
			c.a = 255;
		}

		// render the face
		if (useStipple) re.setMaterial(GLMaterial::GLASS, c, GLMaterial::NONE, rc.m_settings.m_identifyBackfacing);
		else re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::NONE, rc.m_settings.m_identifyBackfacing);
		re.renderGMesh(*gm, n);
	}

	// render internal surfaces
	int nitem = m_scene->GetItemMode();
	if (nitem == ITEM_ELEM)
	{
		int NP = po->Parts();
		for (int i = 0; i < NP; ++i)
		{
			GPart* pg = po->Part(i);

			GLColor c = m_scene->GetPartColor(pg);
			bool useStipple = false;
			if (c.a != 255)
			{
				c.a = 255;
				useStipple = true;
			}

			if (vs.m_transparencyMode != 0)
			{
				switch (vs.m_transparencyMode)
				{
				case 1: if (po->IsSelected()) useStipple = true; break;
				case 2: if (!po->IsSelected()) useStipple = true; break;
				}
			}

			// render the face
			if (useStipple) re.setMaterial(GLMaterial::GLASS, c, GLMaterial::NONE, rc.m_settings.m_identifyBackfacing);
			else re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::NONE, rc.m_settings.m_identifyBackfacing);
			re.renderGMesh(*gm, NF + i);
		}
	}
}

void GLObjectItem::RenderMeshByObjectColor(GLRenderEngine& re, GLContext& rc)
{
	GLMesh* gm = m_po->GetFERenderMesh();
	if (gm == nullptr) return;

	GLViewSettings& vs = rc.m_settings;
	bool useStipple = false;
	if (vs.m_transparencyMode != 0)
	{
		switch (vs.m_transparencyMode)
		{
		case 1: if (m_po->IsSelected()) useStipple = true; break;
		case 2: if (!m_po->IsSelected()) useStipple = true; break;
		}
	}

	GLColor c = m_po->GetColor();
	if (useStipple) re.setMaterial(GLMaterial::GLASS, c, GLMaterial::NONE, rc.m_settings.m_identifyBackfacing);
	else re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::NONE, rc.m_settings.m_identifyBackfacing);

	re.renderGMesh(*gm);
}

void GLObjectItem::RenderMeshByElementType(GLRenderEngine& re, GLContext& rc, GLMesh& mesh)
{
	GLViewSettings& vs = rc.m_settings;
	bool useStipple = false;
	if (vs.m_transparencyMode != 0)
	{
		switch (vs.m_transparencyMode)
		{
		case 1: if (m_po->IsSelected()) useStipple = true; break;
		case 2: if (!m_po->IsSelected()) useStipple = true; break;
		}
	}

	GLColor c = m_po->GetColor();
	if (useStipple) re.setMaterial(GLMaterial::GLASS, c, GLMaterial::VERTEX_COLOR, rc.m_settings.m_identifyBackfacing);
	else re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::VERTEX_COLOR, rc.m_settings.m_identifyBackfacing);

	re.renderGMesh(mesh);
}

// Render the FE nodes
void GLObjectItem::RenderFENodes(GLRenderEngine& re, GLContext& rc)
{
	GObject* po = m_po;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	re.setPointSize(rc.m_settings.m_node_size);

	GLMesh* gm = po->GetFERenderMesh(); assert(gm);
	assert(gm->Nodes() == pm->Nodes());

	// render the visible nodes
	re.setMaterial(GLMaterial::CONSTANT, GLColor(0, 0, 255, 128));
	re.renderTaggedGMeshNodes(*gm, 1);

	// render selected nodes
	// TODO: Shouldn't this be done in the GLSelectionItem?
	FENodeSelection* sel = dynamic_cast<FENodeSelection*>(m_scene->GetCurrentSelection());
	if (sel && sel->Size())
	{
		GLMesh& selectionMesh = m_scene->GetSelectionMesh();
		if (selectionMesh.Nodes() > 0)
		{
			re.setMaterial(GLMaterial::OVERLAY, GLColor::Red());
			re.renderGMeshNodes(selectionMesh, false);
		}
	}
}

void GLObjectItem::RenderSelectedFEFaces(GLRenderEngine& re)
{
	GObject* po = m_po;

	FEFaceSelection* sel = dynamic_cast<FEFaceSelection*>(m_scene->GetCurrentSelection());
	if ((sel == nullptr) || (sel->Count() == 0)) return;
	if (sel->GetMesh() != po->GetFEMesh()) return;

	GLMesh& selMesh = m_scene->GetSelectionMesh();

	re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Red());
	re.renderGMesh(selMesh, false);

	re.setMaterial(GLMaterial::OVERLAY, GLColor::Yellow());
	re.renderGMeshEdges(selMesh, false);
}

void GLObjectItem::RenderSelectedFEElements(GLRenderEngine& re)
{
	FEElementSelection* sel = dynamic_cast<FEElementSelection*>(m_scene->GetCurrentSelection());
	if ((sel == nullptr) || (sel->Count() == 0)) return;
	if (sel->GetMesh() != m_po->GetFEMesh()) return;

	re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Red());
	re.renderGMesh(m_scene->GetSelectionMesh(), false);

	// render a yellow highlight around selected elements
	re.setMaterial(GLMaterial::OVERLAY, GLColor::Yellow());
	re.renderGMeshEdges(m_scene->GetSelectionMesh(), false);
}

void GLObjectItem::RenderSurfaceMeshNodes(GLRenderEngine& re, GLContext& rc)
{
	GLViewSettings& view = rc.m_settings;
	quatd q = GetCamera().GetOrientation();

	// set the point size
	float fsize = view.m_node_size;
	re.setPointSize(fsize);

	FSMeshBase* mesh = m_po->GetEditableMesh();
	if (mesh)
	{
		// reset all tags
		int NN = mesh->Nodes();
		GLMesh pointMesh;
		for (int i = 0; i < NN; ++i)
		{
			FSNode& node = mesh->Node(i);
			if (node.IsVisible()) pointMesh.AddNode(to_vec3f(node.r));
		}

		// check the cull
		if (view.m_bcull)
		{
			vec3d f;
			int NF = mesh->Faces();
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = mesh->Face(i);
				int n = face.Nodes();
				for (int j = 0; j < n; ++j)
				{
					vec3d nn = to_vec3d(face.m_nn[j]);
					f = q * nn;
					if (f.z < 0) pointMesh.AddNode(to_vec3f(mesh->Node(face.n[j]).r));
				}
			}
		}

		if (pointMesh.Nodes())
		{
			re.setMaterial(GLMaterial::CONSTANT, GLColor(0, 0, 255, 128));
			re.renderGMeshNodes(pointMesh, false);
		}
	}
}

//-----------------------------------------------------------------------------
// Render the FE Edges
void GLObjectItem::RenderFEEdges(GLRenderEngine& re)
{
	// render the unselected edges
	GLMesh* mesh = m_po->GetFERenderMesh();
	if (mesh)
	{
		re.setMaterial(GLMaterial::CONSTANT, GLColor(0, 0, 255, 128));
		re.renderGMeshEdges(*mesh);
	}

	// render the selected edges
	GLMesh& selectionMesh = m_scene->GetSelectionMesh();
	if (selectionMesh.Edges() > 0)
	{
		re.setMaterial(GLMaterial::OVERLAY, GLColor(255, 0, 0, 128));
		re.renderGMeshEdges(selectionMesh, false);
	}
}

void GLObjectItem::RenderAllBeamElements(GLRenderEngine& re)
{
	GObject* po = m_po;
	if (po == nullptr) return;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	GLMesh beamMesh;
	vec3f r[3];
	int NE = pm->Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.IsVisible() && (edge.m_elem >= 0))
		{
			FSElement& el = pm->Element(edge.m_elem);
			r[0] = to_vec3f(pm->Node(el.m_node[0]).r);
			r[1] = to_vec3f(pm->Node(el.m_node[1]).r);
			switch (el.Type())
			{
			case FE_BEAM2: beamMesh.AddEdge(r, 2); break;
			case FE_BEAM3:
				r[2] = to_vec3f(pm->Node(el.m_node[2]).r);
				beamMesh.AddEdge(r, 3);
				break;
			}
		}
	}
	if (beamMesh.Edges() == 0) return;

	re.setMaterial(GLMaterial::CONSTANT, po->GetColor());
	re.renderGMeshEdges(beamMesh, false);
}

void GLObjectItem::RenderUnselectedBeamElements(GLRenderEngine& re)
{
	GObject* po = m_po;
	if (po == nullptr) return;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	GLMesh beamMesh;
	vec3f r[3];
	int NE = pm->Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.IsVisible() && (!edge.IsSelected()) && (edge.m_elem >= 0))
		{
			FSElement& el = pm->Element(edge.m_elem);
			r[0] = to_vec3f(pm->Node(el.m_node[0]).r);
			r[1] = to_vec3f(pm->Node(el.m_node[1]).r);
			switch (el.Type())
			{
			case FE_BEAM2: beamMesh.AddEdge(r, 2); break;
			case FE_BEAM3:
				r[2] = to_vec3f(pm->Node(el.m_node[2]).r);
				beamMesh.AddEdge(r, 3);
				break;
			}
		}
	}
	if (beamMesh.Edges() == 0) return;

	re.setMaterial(GLMaterial::CONSTANT, po->GetColor());
	re.renderGMeshEdges(beamMesh, false);
}

void GLObjectItem::RenderNormals(GLRenderEngine& re, double scale)
{
	if (m_po->IsVisible() == false) return;

	FSMeshBase* pm = m_po->GetEditableMesh();
	if (pm == 0) return;
	double R = 0.05 * pm->GetBoundingBox().GetMaxExtent() * scale;

	GLMesh lineMesh;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		const FSFace& face = pm->Face(i);
		if (face.IsVisible())
		{
			vec3f p[2];
			p[0] = vec3f(0, 0, 0);
			vec3f fn = face.m_fn;

			vec3d c(0, 0, 0);
			int nf = face.Nodes();
			for (int j = 0; j < nf; ++j) c += pm->Node(face.n[j]).r;
			c /= nf;

			p[0] = to_vec3f(c);
			p[1] = p[0] + fn * R;

			lineMesh.AddEdge(p, 2);

			float r = fabs(fn.x);
			float g = fabs(fn.y);
			float b = fabs(fn.z);

			GLMesh::EDGE& edge = lineMesh.Edge(lineMesh.Edges() - 1);
			edge.c[0] = GLColor::White();
			edge.c[1] = GLColor::FromRGBf(r, g, b);
		}
	}

	re.setMaterial(GLMaterial::CONSTANT, GLColor::White(), GLMaterial::VERTEX_COLOR);
	re.renderGMeshEdges(lineMesh, false);
}

void GLObjectItem::RenderBeamParts(GLRenderEngine& re)
{
	GObject* po = m_po;
	if (!po->IsVisible()) return;

	int nitem = m_scene->GetItemMode();
	int nsel = m_scene->GetSelectionMode();

	// get the GLMesh
	FSModel& fem = *m_scene->GetFSModel();
	GLMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == 0) return;

	re.setMaterial(GLMaterial::CONSTANT, po->GetColor());
	for (int i = 0; i < po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		if (pg->IsVisible() && pg->IsBeam())
		{
			for (int j = 0; j < pg->m_edge.size(); ++j)
			{
				GEdge& e = *po->Edge(pg->m_edge[j]);
				if (e.IsVisible())
					re.renderGMeshEdges(*pm, e.GetLocalID());
			}
		}
	}
}

void GLObjectItem::RenderSurfaceMeshEdges(GLRenderEngine& re)
{
	GLMesh* mesh = m_po->GetRenderMesh();
	if (mesh == nullptr) return;

	// render the unselected edges
	re.setMaterial(GLMaterial::CONSTANT, GLColor::Blue());
	re.renderGMeshEdges(*mesh);

	// render the selected edges
	re.setMaterial(GLMaterial::OVERLAY, GLColor::Red());
	re.renderGMeshEdges(m_scene->GetSelectionMesh(), false);
}

void GLObjectItem::RenderSelection(GLRenderEngine& re)
{
	GLMesh& selectionMesh = m_scene->GetSelectionMesh();
	if (selectionMesh.Faces() > 0)
	{
		re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Red());
		re.renderGMesh(selectionMesh, false);

		re.setMaterial(GLMaterial::OVERLAY, GLColor::Yellow());
		re.renderGMeshEdges(selectionMesh, false);
	}
}

// This function renders the object by looping over all the parts and
// for each part render the external surfaces that belong to that part.
// NOTE: The reason why only external surfaces are rendered is because
//       it is possible for an external surface to coincide with an
//       internal surface. E.g., when a shell layer lies on top of a 
//       hex layer.
void GLObjectItem::RenderObject(GLRenderEngine& re, GLContext& rc)
{
	bool useStipple = false;
	if (rc.m_settings.m_transparencyMode != 0)
	{
		switch (rc.m_settings.m_transparencyMode)
		{
		case 1: if (m_po->IsSelected()) useStipple = true; break;
		case 2: if (!m_po->IsSelected()) useStipple = true; break;
		}
	}

	bool frontOnly = rc.m_settings.m_identifyBackfacing;

	if (useStipple) re.setMaterial(GLMaterial::GLASS, m_po->GetColor(), GLMaterial::VERTEX_COLOR, frontOnly);
	else re.setMaterial(GLMaterial::PLASTIC, m_po->GetColor(), GLMaterial::VERTEX_COLOR, frontOnly);

	GLMesh* mesh = m_po->GetRenderMesh();
	if (mesh == nullptr) return;

	if (m_clearCache)
	{
		re.deleteCachedMesh(mesh);
		m_clearCache = false;
	}

	int NF = m_po->Faces();
	for (int i = 0; i < NF; ++i)
	{
		GFace* pf = m_po->Face(i);
		if (pf->IsVisible())
		{
			re.renderGMesh(*mesh, i);
		}
	}
/*
	if (NF == 0)
	{
		// if there are no faces, render edges instead
		re.setMaterial(GLMaterial::CONSTANT, GLColor::Black());
		int NC = m_po->Edges();
		for (int n = 0; n < NC; ++n)
		{
			GEdge& e = *m_po->Edge(n);
			if (e.IsVisible())
			{
				re.renderGMeshEdges(*mesh, n);
			}
		}
	}

	// render beam sections if feature edges are not rendered. 
	if (rc.m_settings.m_bfeat == false)
	{
		RenderBeamParts(re);
	}
*/
}

void GLObjectItem::RenderSurfaceMeshFaces(GLRenderEngine& re, GLContext& rc)
{
	GObject* po = m_po;
	GSurfaceMeshObject* surfaceObject = dynamic_cast<GSurfaceMeshObject*>(po);
	if (surfaceObject == 0)
	{
		// just render something, otherwise nothing will show up
		RenderObject(re, rc);
		return;
	}

	FSSurfaceMesh* surfaceMesh = surfaceObject->GetSurfaceMesh();
	assert(surfaceMesh);
	if (surfaceMesh == nullptr) return;

	GLViewSettings& view = rc.m_settings;

	Mesh_Data& data = surfaceMesh->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());

	// render the unselected faces
	if (showContour)
	{
		GLMesh* gmesh = surfaceObject->GetRenderMesh();

		// We don't use the cached mesh, since it probably won't have the 
		// vertex colors stored, which we need here. 
		re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::VERTEX_COLOR);
		re.renderGMesh(*gmesh, false);
	}
	else
	{
		RenderObject(re, rc);
	}

	RenderSelection(re);
}

void RenderLine(GLRenderEngine& re, GNode& n0, GNode& n1)
{
	vec3d r0 = n0.Position();
	vec3d r1 = n1.Position();

	re.renderPoint(r0);
	re.renderPoint(r1);
	re.renderLine(r0, r1);
}

GLDiscreteItem::GLDiscreteItem(CGLModelScene* scene) : GLModelSceneItem(scene)
{
	// get the selection mode
	int nsel = m_scene->GetSelectionMode();
	bool bsel = (nsel == SELECT_DISCRETE);

	// get the model
	FSModel* ps = m_scene->GetFSModel();
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
	Line line;
	int ND = model.DiscreteObjects();
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		if (po->IsVisible())
		{
			GLColor c = po->GetColor();
			if (bsel && po->IsSelected()) c = GLColor(255, 255, 0);
			else if (!po->IsActive()) c = GLColor(128, 128, 128);

			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(po);
			if (ps)
			{
				GNode* pn0 = lut[ps->m_node[0] - minId];
				GNode* pn1 = lut[ps->m_node[1] - minId];
				if (pn0 && pn1)
				{
					line.a = pn0->Position();
					line.b = pn1->Position();
					line.col = c;
					m_lines.push_back(line);
				}
			}

			GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(po);
			if (pg)
			{
				GNode* pn0 = lut[pg->m_node[0] - minId];
				GNode* pn1 = lut[pg->m_node[1] - minId];
				if (pn0 && pn1)
				{
					line.a = pn0->Position();
					line.b = pn1->Position();
					line.col = c;
					m_lines.push_back(line);
				}
			}

			GDiscreteElementSet* pd = dynamic_cast<GDiscreteElementSet*>(po);
			if (pd)
			{
				int N = pd->size();
				for (int n = 0; n < N; ++n)
				{
					GDiscreteElement& el = pd->element(n);

					if (bsel && el.IsSelected()) c = GLColor(255, 255, 0);
					else if (!po->IsActive()) c = GLColor(128, 128, 128);
					else c = GLColor(c.r, c.g, c.b);

					int n0 = el.Node(0) - minId;
					int n1 = el.Node(1) - minId;
					if ((n0 >= 0) && (n0 < lut.size()) &&
						(n1 >= 0) && (n1 < lut.size()))
					{
						GNode* pn0 = lut[n0];
						GNode* pn1 = lut[n1];
						if (pn0 && pn1)
						{
							line.a = pn0->Position();
							line.b = pn1->Position();
							line.col = c;
							m_lines.push_back(line);
						}
					}
				}
			}

			GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(po);
			if (ds)
			{
				GNode* pn0 = lut[ds->NodeID(0) - minId];
				GNode* pn1 = lut[ds->NodeID(1) - minId];
				if (pn0 && pn1)
				{
					line.a = pn0->Position();
					line.b = pn1->Position();
					line.col = c;
					m_lines.push_back(line);
				}
			}
		}
	}
}

void GLDiscreteItem::render(GLRenderEngine& re, GLContext& rc)
{
	if (rc.m_settings.m_showDiscrete == false) return;

	if (m_lines.empty()) return;

	re.pushState();
	re.disable(GLRenderEngine::LIGHTING);
	for (const Line& line : m_lines)
	{
		GLColor c = line.col;
		re.setColor(c);

		re.renderPoint(line.a);
		re.renderPoint(line.b);
		re.renderLine(line.a, line.b);
	}
	re.popState();
}

void GLSelectionBox::render(GLRenderEngine& re, GLContext& rc)
{
	// get the model
	FSModel* ps = m_scene->GetFSModel();
	GModel& model = ps->GetModel();

	// Get the item mode
	int item = m_scene->GetItemMode();

	// get the selection mode
	int nsel = m_scene->GetSelectionMode();

	GObject* poa = m_scene->GetActiveObject();

	re.enable(GLRenderEngine::DEPTHTEST);

	if (item == ITEM_MESH)
	{
		for (int i = 0; i < model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po->IsVisible())
			{
				re.pushTransform();
				SetModelView(re, po);

				if (nsel == SELECT_OBJECT)
				{
					if (po->IsSelected())
					{
						glx::renderBox(re, po->GetLocalBox(), GLColor(255, 255, 255), true, 1.025);
					}
				}
				else if (po == poa)
				{
					assert(po->IsSelected());
					glx::renderBox(re, po->GetLocalBox(), GLColor(164, 0, 164), true, 1.025);
				}
				re.popTransform();
			}
		}
	}
	else if (poa)
	{
		re.pushTransform();
		SetModelView(re, poa);
		glx::renderBox(re, poa->GetLocalBox(), GLColor(255, 255, 0), true, 1.025);
		re.popTransform();
	}
}

void GLMeshLinesItem::render(GLRenderEngine& re, GLContext& rc)
{
	if (rc.m_settings.m_bmesh == false) return;

	GModel& model = *m_scene->GetGModel();
	int nitem = m_scene->GetItemMode();

	GLViewSettings& vs = rc.m_settings;
	re.setMaterial(GLMaterial::CONSTANT, vs.m_meshColor);

	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid())
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				re.pushTransform();
				SetModelView(re, po);
				if (nitem != ITEM_EDGE)
				{
					GLMesh* lineMesh = po->GetFERenderMesh(); assert(lineMesh);
					if (lineMesh)
					{
						re.renderGMeshEdges(*lineMesh);
					}
				}
				re.popTransform();
			}
			else if (dynamic_cast<GSurfaceMeshObject*>(po))
			{
				GLMesh* gmesh = po->GetRenderMesh();
				if (gmesh && (nitem != ITEM_EDGE))
				{
					re.pushTransform();
					SetModelView(re, po);
					re.renderGMeshEdges(*gmesh);
					re.popTransform();
				}
			}
		}
	}
}

void GLFeatureEdgesItem::render(GLRenderEngine& re, GLContext& rc)
{
	GLViewSettings& vs = rc.m_settings;
	if (vs.m_bfeat || (vs.m_nrender == RENDER_WIREFRAME))
	{
		// don't draw feature edges in edge mode, since the edges are the feature edges
		// (Don't draw feature edges when we are rendering FE edges)
		int nselect = m_scene->GetSelectionMode();
		int nitem = m_scene->GetItemMode();
		if (((nitem != ITEM_MESH) || (nselect != SELECT_EDGE)) && (nitem != ITEM_EDGE))
		{
			FSModel* ps = m_scene->GetFSModel();
			GModel& model = ps->GetModel();

			re.setMaterial(GLMaterial::CONSTANT, GLColor::Black());

			for (int k = 0; k < model.Objects(); ++k)
			{
				GObject* po = model.Object(k);
				if (po->IsVisible())
				{
					re.pushTransform();
					SetModelView(re, po);

					GLMesh* m = po->GetRenderMesh();
					if (m)
					{
						re.setColor(GLColor::Black());
						re.renderGMeshEdges(*m);

						if (vs.m_nrender == RENDER_WIREFRAME)
						{
							re.setColor(GLColor(32, 0, 0));
							re.renderGMeshOutline(GetCamera(), *m, po->GetRenderTransform());
						}
					}
					re.popTransform();
				}
			}
		}
	}
}

void GLPhysicsItem::render(GLRenderEngine& re, GLContext& rc)
{
	GLViewSettings& vs = rc.m_settings;

	const GLCamera& cam = GetCamera();
	double scale = 0.05 * (double)cam.GetTargetDistance();

	// render physics
	if (vs.m_brigid) RenderRigidBodies(re, rc);
	if (vs.m_bjoint) { RenderRigidJoints(re, scale); RenderRigidConnectors(re, scale); }
	if (vs.m_bwall) RenderRigidWalls(re);
	if (vs.m_bfiber) RenderMaterialFibers(re, rc);
	if (vs.m_blma) RenderLocalMaterialAxes(re, rc);
}

void GLPhysicsItem::RenderRigidBodies(GLRenderEngine& re, GLContext& rc) const
{
	const GLCamera& cam = GetCamera();

	FSModel* ps = m_scene->GetFSModel();

	double scale = 0.03 * (double)cam.GetTargetDistance();
	double R = 0.5 * scale;

	quatd qi = cam.GetOrientation().Inverse();

	for (int i = 0; i < ps->Materials(); ++i)
	{
		GMaterial* pgm = ps->GetMaterial(i);
		GLMaterial& glm = pgm->GetGLMaterial();
		FSMaterial* pm = pgm->GetMaterialProperties();
		if (pm && pm->IsRigid())
		{
			GLColor c = glm.diffuse;

			// We'll position the rigid body glyph, either in the center of rigid part,
			// or in the center_of_mass parameter if the override_com is true.
			vec3d r(0, 0, 0);
			bool b = pm->GetParamBool("override_com");
			if (b) r = pm->GetParamVec3d("center_of_mass");
			else r = pgm->GetPosition();

			re.pushTransform();
			re.translate(r);

			glx::renderGlyph(re, glx::RIGID_BODY, R, c);

			re.popTransform();
		}
	}
}

void GLPhysicsItem::RenderRigidWalls(GLRenderEngine& re) const
{
	FSModel* ps = m_scene->GetFSModel();
	BOX box = ps->GetModel().GetBoundingBox();
	double R = box.GetMaxExtent();
	vec3d c = box.Center();

	for (int n = 0; n < ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i < s.Constraints(); ++i)
		{
			FSModelConstraint* pw = s.Constraint(i);
			if (pw->IsType("rigid_wall") && pw->IsActive())
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
				re.pushTransform();
				{
					re.transform(p, q);
					glx::renderGlyph(re, glx::RIGID_WALL, R, GLColor::Black());
				}
				re.popTransform();
			}
		}
	}
}

void GLPhysicsItem::RenderRigidJoints(GLRenderEngine& re, double scale) const
{
	FSModel* ps = m_scene->GetFSModel();

	double R = 0.5 * scale;

	for (int n = 0; n < ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i < s.Interfaces(); ++i)
		{
			FSRigidJoint* pj = dynamic_cast<FSRigidJoint*> (s.Interface(i));
			if (pj)
			{
				vec3d r = pj->GetVecValue(FSRigidJoint::RJ);
				re.pushTransform();
				re.translate(r);
				glx::renderGlyph(re, glx::RIGID_JOINT, R, GLColor::Red());
				re.popTransform();
			}
		}
	}
}

void GLPhysicsItem::RenderRigidConnectors(GLRenderEngine& re, double scale) const
{
	FSModel* ps = m_scene->GetFSModel();

	double R = 0.5 * scale;

	for (int n = 0; n < ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i < s.RigidConnectors(); ++i)
		{
			FSRigidConnector* rci = s.RigidConnector(i);
			if (rci->IsType("rigid spherical joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");

				GLColor c;
				if (rci->IsActive())
					c = GLColor(255, 0, 0);
				else
					c = GLColor(64, 64, 64);

				re.pushTransform();
				re.translate(r);
				glx::renderGlyph(re, glx::RIGID_JOINT, R, c);
				re.popTransform();
			}
			else if (rci->IsType("rigid revolute joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("rotation_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("transverse_axis"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				double Q4[16] = {
					a.x, a.y, a.z, 0.f,
					b.x, b.y, b.z, 0.f,
					c.x, c.y, c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				re.pushTransform();
				re.translate(r);
				re.multTransform(Q4);

				GLColor col;
				if (rci->IsActive())
					col = GLColor(0, 0, 255);
				else
					col = GLColor(64, 64, 64);
				glx::renderGlyph(re, glx::REVOLUTE_JOINT, R, col);

				re.popTransform();
			}
			else if (rci->IsType("rigid prismatic joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d a = rci->GetParamVec3d("translation_axis"); a.Normalize();
				vec3d b = rci->GetParamVec3d("transverse_axis"); b.Normalize();
				vec3d c = a ^ b; c.Normalize();
				b = c ^ a; b.Normalize();
				double Q4[16] = {
					a.x, a.y, a.z, 0.f,
					b.x, b.y, b.z, 0.f,
					c.x, c.y, c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				re.pushTransform();
				re.translate(r);
				re.multTransform(Q4);

				GLColor col;
				if (rci->IsActive())
					col = GLColor(0, 255, 0);
				else
					col = GLColor(64, 64, 64);
				glx::renderGlyph(re, glx::PRISMATIC_JOINT, R, col);

				re.popTransform();
			}
			else if (rci->IsType("rigid cylindrical joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("joint_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("transverse_axis"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				double Q4[16] = {
					a.x, a.y, a.z, 0.f,
					b.x, b.y, b.z, 0.f,
					c.x, c.y, c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				re.pushTransform();
				re.translate(r);
				re.multTransform(Q4);

				GLColor col;
				if (rci->IsActive())
					col = GLColor(255, 0, 255);
				else
					col = GLColor(64, 64, 64);

				glx::renderGlyph(re, glx::CYLINDRICAL_JOINT, R, col);

				re.popTransform();
			}
			else if (rci->IsType("rigid planar joint"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("rotation_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("translation_axis_1"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				double Q4[16] = {
					a.x, a.y, a.z, 0.f,
					b.x, b.y, b.z, 0.f,
					c.x, c.y, c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				re.pushTransform();
				re.translate(r);
				re.multTransform(Q4);

				GLColor col;
				if (rci->IsActive())
					col = GLColor(0, 255, 255);
				else
					col = GLColor(64, 64, 64);

				glx::renderGlyph(re, glx::PLANAR_JOINT, R, col);

				re.popTransform();
			}
			else if (rci->IsType("rigid lock"))
			{
				vec3d r = rci->GetParamVec3d("joint_origin");
				vec3d c = rci->GetParamVec3d("first_axis"); c.Normalize();
				vec3d a = rci->GetParamVec3d("second_axis"); a.Normalize();
				vec3d b = c ^ a; b.Normalize();
				a = b ^ c; a.Normalize();
				double Q4[16] = {
					a.x, a.y, a.z, 0.f,
					b.x, b.y, b.z, 0.f,
					c.x, c.y, c.z, 0.f,
					0.f, 0.f, 0.f, 1.f };

				re.pushTransform();
				re.translate(r);
				re.multTransform(Q4);

				GLColor col = (rci->IsActive() ? GLColor(255, 127, 0) : GLColor(64, 64, 64));
				glx::renderGlyph(re, glx::RIGID_LOCK, R, col);

				re.popTransform();
			}
			else if (rci->IsType("rigid spring"))
			{
				vec3d xa = rci->GetParamVec3d("insertion_a");
				vec3d xb = rci->GetParamVec3d("insertion_b");

				re.pushTransform();
				if (rci->IsActive())
					re.setColor(GLColor(255, 0, 0));
				else
					re.setColor(GLColor(64, 64, 64));

				glx::renderSpring(re, xa, xb, R);
				re.popTransform();
			}
			else if (rci->IsType("rigid damper"))
			{
				vec3d xa = rci->GetParamVec3d("insertion_a");
				vec3d xb = rci->GetParamVec3d("insertion_b");

				re.pushTransform();

				if (rci->IsActive())
					re.setColor(GLColor(255, 0, 0));
				else
					re.setColor(GLColor(64, 64, 64));

				glx::renderDamper(re, xa, xb, R);

				re.popTransform();
			}
			else if (rci->IsType("rigid contractile force"))
			{
				vec3d xa = rci->GetParamVec3d("insertion_a");
				vec3d xb = rci->GetParamVec3d("insertion_b");

				re.pushTransform();

				if (rci->IsActive())
					re.setColor(GLColor(255, 0, 0));
				else
					re.setColor(GLColor(64, 64, 64));

				glx::renderContractileForce(re, xa, xb, R);

				re.popTransform();
			}
		}
	}
}

void GLPhysicsItem::RenderMaterialFibers(GLRenderEngine& re, GLContext& rc) const
{
	GLFiberRenderer* fiberRender = m_scene->GetFiberRenderer();
	if (fiberRender == nullptr) return;

	FSModel* ps = m_scene->GetFSModel();
	GModel& model = ps->GetModel();

	BOX box = model.GetBoundingBox();
	double h = 0.05 * box.GetMaxExtent();

	GLViewSettings& vs = rc.m_settings;

	fiberRender->SetScaleFactor(h * vs.m_fiber_scale);
	fiberRender->SetLineWidth(h * vs.m_fiber_width * 0.1);
	fiberRender->SetLineStyle(vs.m_fibLineStyle);
	fiberRender->SetDensity(vs.m_fiber_density);

	fiberRender->RenderVectors(re);
}

void GLPhysicsItem::RenderLocalMaterialAxes(GLRenderEngine& re, GLContext& rc) const
{
	// get the model
	FSModel* ps = m_scene->GetFSModel();
	GModel& model = ps->GetModel();

	FEElementRef rel;

	GLViewSettings& view = rc.m_settings;
	BOX box = model.GetBoundingBox();
	double h = 0.05 * box.GetMaxExtent() * view.m_fiber_scale;

	re.setMaterial(GLMaterial::CONSTANT, GLColor::White());

	GLColor rgb[3] = { GLColor::Red(), GLColor::Green(), GLColor::Blue() };

	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				Transform& T = po->GetRenderTransform();
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

								re.setColor(rgb[k]);
								re.renderLine(c, c + q * h);
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

								re.setColor(rgb[k]);
								re.renderLine(c, c + q * h);
							}
						}
					}
				}
			}
		}
	}
}

void GLSelectionItem::render(GLRenderEngine& re, GLContext& rc)
{
	if (m_scene->GetItemMode() != ITEM_MESH) return;

	FSModel* ps = m_scene->GetFSModel();
	GModel& model = ps->GetModel();

	int nsel = m_scene->GetSelectionMode();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible() && po->IsValid())
		{
			re.pushTransform();
			SetModelView(re, po);
			switch (nsel)
			{
			case SELECT_PART: RenderSelectedParts(re, rc, po); break;
			case SELECT_FACE: RenderSelectedSurfaces(re, rc, po); break;
			case SELECT_EDGE: RenderSelectedEdges(re, rc, po); break;
			case SELECT_NODE: RenderSelectedNodes(re, rc, po); break;
			}
			re.popTransform();
		}
	}
}

// Render selected nodes
void GLSelectionItem::RenderSelectedNodes(GLRenderEngine& re, GLContext& rc, GObject* po) const
{
	if ((po == nullptr) || (po->Nodes() == 0)) return;

	GLMesh points;
	for (int i = 0; i < po->Nodes(); ++i)
	{
		GNode& n = *po->Node(i);
		if (n.IsSelected())
		{
			assert(n.Type() != NODE_SHAPE);
			vec3f r = to_vec3f(n.LocalPosition());
			points.AddNode(r);
		}
	}
	if (points.IsEmpty()) return;

	re.setMaterial(GLMaterial::OVERLAY, GLColor::Yellow());
	re.renderGMeshNodes(points, false);

#ifndef NDEBUG
	// Draw FE nodes on top of GLMesh nodes to make sure they match
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		GLMesh fenodes;
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			if (node.m_gid > -1)
			{
				GNode& gn = *po->Node(node.m_gid);
				if (gn.IsSelected())
				{
					fenodes.AddNode(to_vec3f(node.r));
				}
			}
		}
		if (fenodes.Nodes() != 0)
		{
			re.setColor(GLColor::Red());
			re.renderGMeshNodes(fenodes, false);
		}
	}
#endif
}

// render selected edges
void GLSelectionItem::RenderSelectedEdges(GLRenderEngine& re, GLContext& rc, GObject* po) const
{
	GLMesh* m = po->GetRenderMesh();
	if (m == nullptr) return;

	re.setMaterial(GLMaterial::OVERLAY, GLColor::Yellow());

	GLMesh pointMesh;
	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected())
		{
			re.renderGMeshEdges(*m, i);

			GNode* n0 = po->Node(e.m_node[0]);
			GNode* n1 = po->Node(e.m_node[1]);

			if (n0 && n1)
			{
				pointMesh.AddNode(to_vec3f(n0->LocalPosition()));
				pointMesh.AddNode(to_vec3f(n1->LocalPosition()));
			}
		}
	}

	if (pointMesh.Nodes() != 0)
	{
		re.renderGMeshNodes(pointMesh, false);
	}

#ifndef NDEBUG
	// Render FE edges onto GLMesh edges to make sure they are consistent
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		// TODO: Add the edges to the render mesh
		GLMesh edges;
		vec3f r[FSEdge::MAX_NODES];
		for (int i = 0; i < pm->Edges(); ++i)
		{
			FSEdge& e = pm->Edge(i);
			if (e.m_gid > -1)
			{
				GEdge* ge = po->Edge(e.m_gid);
				if (ge && ge->IsSelected())
				{
					for (int j = 0; j < e.Nodes(); ++j)
						r[j] = to_vec3f(pm->Node(e.n[j]).r);
					edges.AddEdge(r, e.Nodes(), e.m_gid);
				}
			}
		}
		if (edges.Edges() > 0)
		{
			re.setColor(GLColor::Red());
			re.renderGMeshEdges(edges, false);
		}
	}
#endif
}

// Render selected surfaces
void GLSelectionItem::RenderSelectedSurfaces(GLRenderEngine& re, GLContext& rc, GObject* po) const
{
	if (!po->IsVisible()) return;

	GLMesh* pm = po->GetRenderMesh(); assert(pm);
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
	re.pushState();
	re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Blue());

	for (int surfId : selectedSurfaces)
	{
		re.renderGMesh(*pm, surfId);
	}
	re.popState();


#ifndef NDEBUG
	{
		// render FE surfaces
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			GLMesh msh;
			vec3f rf[FSElement::MAX_NODES];
			for (int i = 0; i < pm->Faces(); ++i)
			{
				FSFace& f = pm->Face(i);
				if (f.m_gid > -1)
				{
					GFace& gf = *po->Face(f.m_gid);
					if (gf.IsSelected())
					{
						int nf = f.Nodes();
						for (int j = 0; j < nf; ++j) rf[j] = to_vec3f(pm->Node(f.n[j]).r);
						msh.AddFace(rf, nf);
					}
				}
			}
			if (msh.Faces() > 0)
			{
				msh.Update();

				re.setMaterial(GLMaterial::OVERLAY, GLColor::Red());
				re.renderGMesh(msh, false);
			}
		}
	}
#endif

	re.setMaterial(GLMaterial::OVERLAY, GLColor::Blue());
	for (int surfId : selectedSurfaces)
	{
		re.renderGMeshOutline(GetCamera(), *pm, po->GetRenderTransform(), surfId);
	}
}

// render selected parts
void GLSelectionItem::RenderSelectedParts(GLRenderEngine& re, GLContext& rc, GObject* po) const
{
	if (!po->IsVisible()) return;
	GLMesh* m = po->GetRenderMesh();
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

	re.pushState();
	re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Blue());

	for (int surfId : facesToRender)
	{
		re.renderGMesh(*m, surfId);
	}
	re.popState();

	re.setMaterial(GLMaterial::OVERLAY, GLColor::Blue());
	for (int surfId : facesToRender)
	{
		re.renderGMeshOutline(GetCamera(), *m, po->GetRenderTransform(), surfId);
	}
}

GLHighlighterItem::GLHighlighterItem(CGLModelScene* scene) : GLModelSceneItem(scene)
{
	m_activeColor = GLColor(100, 255, 255);
	m_pickColor[0] = GLColor(0, 200, 200);
	m_pickColor[1] = GLColor(200, 0, 200);
}

void GLHighlighterItem::render(GLRenderEngine& re, GLContext& rc)
{
	std::vector<GLHighlighter::Item> items = GLHighlighter::GetItems();
	GItem* activeItem = GLHighlighter::GetActiveItem();
	if (items.empty() && (activeItem == nullptr)) return;

	for (auto& item : items)
	{
		FSObject* it = item.item;

		GLColor c = m_pickColor[item.color];

		GEdge* edge = dynamic_cast<GEdge*>(it);
		if (edge) drawEdge(re, edge, c);

		GNode* node = dynamic_cast<GNode*>(it);
		if (node) drawNode(re, rc, node, c);

		GFace* face = dynamic_cast<GFace*>(it);
		if (face) drawFace(re, rc, face, c);

		GPart* part = dynamic_cast<GPart*>(it);
		if (part) drawPart(re, rc, part, c);

		FSNodeSet* nodeSet = dynamic_cast<FSNodeSet*>(it);
		if (nodeSet) drawFENodeSet(re, nodeSet, c);

		FSSurface* surf = dynamic_cast<FSSurface*>(it);
		if (surf) drawFESurface(re, surf, c);
	}

	if (activeItem)
	{
		GLColor c = m_activeColor;
		GEdge* edge = dynamic_cast<GEdge*>(activeItem);
		if (edge) drawEdge(re, edge, c);

		GNode* node = dynamic_cast<GNode*>(activeItem);
		if (node) drawNode(re, rc, node, c);

		GFace* face = dynamic_cast<GFace*>(activeItem);
		if (face) drawFace(re, rc, face, c);

		GPart* part = dynamic_cast<GPart*>(activeItem);
		if (part) drawPart(re, rc, part, c);
	}
}

void GLHighlighterItem::drawEdge(GLRenderEngine& re, GEdge* edge, GLColor c)
{
	GObject* po = dynamic_cast<GObject*>(edge->Object());
	if (po == 0) return;

	re.pushTransform();
	SetModelView(re, po);

	GLMesh& m = *po->GetRenderMesh();

	re.setMaterial(GLMaterial::OVERLAY, c);
	re.renderGMeshEdges(m, edge->GetLocalID());

	GNode* n0 = po->Node(edge->m_node[0]);
	GNode* n1 = po->Node(edge->m_node[1]);

	if (n0 && n1)
	{
		GLMesh endPoints;
		vec3d r0 = n0->LocalPosition();
		vec3d r1 = n1->LocalPosition();
		endPoints.AddNode(to_vec3f(r0));
		endPoints.AddNode(to_vec3f(r1));

		re.renderGMeshNodes(endPoints, false);
	}
	re.popTransform();
}

void GLHighlighterItem::drawNode(GLRenderEngine& re, GLContext& rc, GNode* node, GLColor c)
{
	if (node == nullptr) return;
	GObject* po = dynamic_cast<GObject*>(node->Object());
	if (po == nullptr) return;

	re.pushTransform();
	SetModelView(re, po);
	GLMesh pt;
	pt.AddNode(to_vec3f(node->LocalPosition()));
	re.setMaterial(GLMaterial::OVERLAY, c);
	re.renderGMeshNodes(pt, false);
	re.popTransform();
}

void GLHighlighterItem::drawFace(GLRenderEngine& re, GLContext& rc, GFace* face, GLColor c)
{
	GObject* po = dynamic_cast<GObject*>(face->Object());
	if (po == 0) return;
	GLMesh* mesh = po->GetRenderMesh();
	if (mesh == nullptr) return;

	re.pushTransform();
	SetModelView(re, po);

	re.setMaterial(GLMaterial::HIGHLIGHT, c);
	re.renderGMesh(*mesh, face->GetLocalID());

	re.setMaterial(GLMaterial::OVERLAY, c);
	re.renderGMeshOutline(GetCamera(), *mesh, po->GetRenderTransform(), face->GetLocalID());

	re.popTransform();
}

void GLHighlighterItem::drawPart(GLRenderEngine& re, GLContext& rc, GPart* part, GLColor c)
{
	GObject* po = dynamic_cast<GObject*>(part->Object());
	if (po == 0) return;
	GLMesh* mesh = po->GetRenderMesh();
	if (mesh == nullptr) return;

	int pid = part->GetLocalID();

	// TODO: Make sure that the part's face list is populated!
//	if (part->Faces() == 0) return;
	vector<int> faceList; faceList.reserve(po->Faces());
	for (int i = 0; i < po->Faces(); ++i)
	{
		GFace* face = po->Face(i);
		if ((face->m_nPID[0] == pid) || (face->m_nPID[1] == pid)) faceList.push_back(i);
	}
	if (faceList.empty()) return;

	re.pushTransform();
	SetModelView(re, po);

	re.setMaterial(GLMaterial::HIGHLIGHT, c);
	for (int surfID : faceList)
	{
		re.renderGMesh(*mesh, surfID);
	}

	re.setMaterial(GLMaterial::OVERLAY, c);
	for (int surfID : faceList)
	{
		re.renderGMeshOutline(GetCamera(), *mesh, po->GetRenderTransform(), surfID);
	}

	re.popTransform();
}

void GLHighlighterItem::drawFENodeSet(GLRenderEngine& re, FSNodeSet* nodeSet, GLColor c)
{
	if ((nodeSet == nullptr) || (nodeSet->size() == 0)) return;

	FSMesh* mesh = nodeSet->GetMesh();
	if (mesh == nullptr) return;
	GObject* po = mesh->GetGObject();
	if (po == nullptr) return;

	GLMesh* gm = po->GetFERenderMesh();
	if (gm == nullptr) return;
	assert(gm->Nodes() == mesh->Nodes());

	std::vector<int> nodeList = nodeSet->CopyItems();
	GLMesh pointMesh;
	pointMesh.Create((int)nodeList.size(), 0, 0);
	for (int i = 0; i<nodeList.size(); ++i)
	{
		pointMesh.Node(i).r = gm->Node(nodeList[i]).r;
	}

	re.pushTransform();
	SetModelView(re, po);
	re.setMaterial(GLMaterial::OVERLAY, c);
	re.renderGMeshNodes(pointMesh, false);
	re.popTransform();
}

void GLHighlighterItem::drawFESurface(GLRenderEngine& re, FSSurface* surf, GLColor c)
{
	FSMesh* mesh = surf->GetMesh();
	if (mesh == nullptr) return;
	GObject* po = mesh->GetGObject();
	if (po == nullptr) return;

	int NF = surf->size();
	if (NF == 0) return;

	std::vector<int> faceList = surf->CopyItems();

	re.pushTransform();
	{
		SetModelView(re, po);

		GLMesh gmesh;
		gmesh.NewPartition();
		int m[FSFace::MAX_NODES];
		for (int n : faceList)
		{
			const FSFace& face = mesh->Face(n);
			int nf = face.Nodes();
			for (int i = 0; i < nf; ++i)
			{
				vec3f r = to_vec3f(mesh->Node(face.n[i]).r);
				m[i] = gmesh.AddNode(r);
			}
			gmesh.AddFace(m, nf);
		}
		gmesh.Update();

		re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Red());
		re.renderGMesh(gmesh, false);
	}
	re.popTransform();
}

void GLGridItem::render(GLRenderEngine& re, GLContext& rc)
{
	if (rc.m_settings.m_bgrid)
	{
		GLGrid& grid = m_scene->GetGrid();
		grid.Render(re, m_scene->GetCamera());
	}
}

void GL3DImageItem::render(GLRenderEngine& re, GLContext& rc)
{
	m_img->Render(re, rc);
}

LegendData CGLModelScene::GetLegendData(int n)
{
	LegendData l;

	if ((n == 0) && m_showLegend)
	{
		l.colormap = (m_colormap == -1 ? ColorMapManager::JET : m_colormap);
		l.ndivs = 10;
		l.smooth = true;
		l.discrete = false;

		GObject* po = GetActiveObject();
		FSMesh* pm = (po ? po->GetFEMesh() : nullptr);
		if (pm)
		{
			Mesh_Data& data = pm->GetMeshData();
			double vmin, vmax;
			data.GetValueRange(vmin, vmax);
			if (vmin == vmax) vmax++;

			l.vmin = vmin;
			l.vmax = vmax;
		}
	}

	return l;
}

void CGLModelScene::ColorizeMesh(GObject* po)
{
	GLMesh* gmsh = po->GetFERenderMesh();
	if (gmsh == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	Mesh_Data& data = pm->GetMeshData();
	if (!data.IsValid()) return;

	double vmin, vmax;
	data.GetValueRange(vmin, vmax);
	if (vmax == vmin) vmax++;

	int NN = pm->Nodes();
	vector<double> val(NN, 0);

	int cmap = m_colormap;
	if (cmap == -1) cmap = ColorMapManager::JET;

	CColorMap map = ColorMapManager::GetColorMap(cmap);
	map.SetRange((float)vmin, (float)vmax);

	int NF = gmsh->Faces();
	for (int i = 0; i < NF; ++i)
	{
		GLMesh::FACE& fi = gmsh->Face(i);
		int fid = fi.fid;
		FSFace* pf = pm->FacePtr(fid);
		if (pf)
		{
			FSFace& face = *pf;
			FSElement& el = pm->Element(face.m_elem[0].eid);
			GPart* pg = po->Part(el.m_gid);
			if (pg && (pg->IsVisible() == false) && (face.m_elem[1].eid != -1))
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
					{
						double vj = data.GetElementValue(face.m_elem[0].eid, fnl[j]);
						val[face.n[j]] = vj;
					}

					for (int j = 0; j < 3; ++j)
					{
						double vj = val[fi.n[j]];
						fi.c[j] = map.map(vj);
					}
				}
				else
				{
					GLColor col(212, 212, 212);
					for (int j = 0; j < 3; ++j) fi.c[j] = col;
				}
			}
		}
		else if (fi.eid >= 0)
		{
			FSElement& el = pm->Element(fi.eid);
			if (data.GetElementDataTag(fi.eid) > 0)
			{
				int ne = el.Nodes();
				for (int j = 0; j < ne; ++j)
				{
					double vj = data.GetElementValue(fi.eid, j);
					val[el.m_node[j]] = vj;
				}

				for (int j = 0; j < 3; ++j)
				{
					double vj = val[fi.n[j]];
					fi.c[j] = map.map(vj);
				}
			}
			else
			{
				GLColor col(212, 212, 212);
				for (int j = 0; j < 3; ++j) fi.c[j] = col;
			}
		}
	}
}
