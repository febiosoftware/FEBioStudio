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
#include <GLLib/GLShader.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FELoad.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <PostGL/GLVectorRender.h>

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

CGLModelScene::CGLModelScene(CModelDocument* doc) : m_doc(doc)
{
	m_objectColor = OBJECT_COLOR_MODE::DEFAULT_COLOR;
	m_fiberViz = nullptr;
	m_buildScene = true;
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

void CGLModelScene::Render(GLRenderEngine& engine, CGLContext& rc)
{
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return;

	FSModel* ps = m_doc->GetFSModel();
	if (ps == nullptr) return;
	GModel& model = ps->GetModel();

	GLViewSettings& view = rc.m_settings;

	// set the object's render transforms
	UpdateRenderTransforms(rc);

	CGLCamera& cam = *rc.m_cam;
	cam.PositionInScene();

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

	if (view.m_use_environment_map) ActivateEnvironmentMap(engine);

	// now render it
	CGLScene::Render(engine, rc);

	if (view.m_use_environment_map) DeactivateEnvironmentMap(engine);

	// render the highlights
	GLHighlighter::draw();

	// Render 2D stuff
	ClearTags();

	// show the labels on rigid bodies
	if (view.m_showRigidLabels) RenderRigidLabels(rc);

	// render the tags
	if (view.m_bTags) RenderTags(rc);

	// render 3D cursor
	if (m_doc->GetItemMode() == ITEM_MESH)
	{
		view.m_show3DCursor = true;
	}
	else
		view.m_show3DCursor = false;

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
			CGLView* glview = dynamic_cast<CGLView*>(rc.m_view);
			if (glview)
			{
				glview->setLegendRange((float)vmin, (float)vmax);
			}
			m_doc->ShowLegend(true);
		}
	}
	else m_doc->ShowLegend(false);
}

void CGLModelScene::BuildScene(CGLContext& rc)
{
	clear(); // clear the scene

	if ((m_doc == nullptr) || !m_doc->IsValid()) return;

	GLViewSettings& vs = rc.m_settings;

	// add plane cut if requested
	addItem(new GLPlaneCutItem(this));

	// add all objects for solid rendering
	GModel& model = *GetGModel();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		addItem(new GLObjectItem(this, po));
	}

	addItem(new GLDiscreteItem(this));

	addItem(new GLSelectionBox(this));

	addItem(new GLMeshLinesItem(this));

	addItem(new GLFeatureEdgesItem(this));

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

	addItem(new GLPhysicsItem(this));

	addItem(new GLSelectionItem(this));

	// Why is this here? I mean, why do we need to turn off the clipping plane here?
	addItem(new GLDisableClipPlaneItem(this));

	addItem(new GLGridItem(this));

	if (m_doc->ImageModels())
	{
		for (int i = 0; i < m_doc->ImageModels(); ++i)
		{
			CImageModel* img = m_doc->GetImageModel(i);
			if (img->IsActive())
			{
				addItem(new GL3DImageItem(this, img));
			}
		}
	}
}

void CGLModelScene::UpdateRenderTransforms(CGLContext& rc)
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
				if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
				BuildFiberVectors(po, matj, rel, c, Q);
			}
			else
			{
				FSMaterialProperty* matProp = dynamic_cast<FSMaterialProperty*>(pmat->GetProperty(i).GetComponent(j));
				if (matProp)
				{
					if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
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
				if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
				BuildFiberVectors(po, matj, rel, c, Q);
			}
			else
			{
				FSMaterialProperty* matProp = dynamic_cast<FSMaterialProperty*>(pmat->GetProperty(i).GetComponent(j));
				if (matProp)
				{
					if (m_colorOption == 2) m_defaultCol = fiberColorPalette[index % GMaterial::MAX_COLORS];
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

void CGLModelScene::BuildFiberViz(CGLContext& rc)
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

std::vector<GLFacetShader*> BuildMaterialShaders(FSModel* fem)
{
	int nmat = 0;
	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* mat = fem->GetMaterial(i);
		if (mat->GetID() > nmat) nmat = mat->GetID();
	}
	nmat++;
	std::vector<GLFacetShader*> shaders(nmat, nullptr);

	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* mat = fem->GetMaterial(i);
		GLMaterial& glm = mat->GetGLMaterial();
		shaders[mat->GetID()] = new GLStandardModelShader(glm.diffuse);
	}
	return shaders;
}

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

class GLElementTypeShader : public GLFacetShader
{
public:
	GLElementTypeShader(FSMesh* pm) : m_pm(pm) {}

	void Activate() override
	{
		glEnable(GL_COLOR_MATERIAL);
		GLFacetShader::Activate();
	}

	void Render(const GMesh::FACE& face) override
	{
		GLColor col;
		if (face.eid >= 0)
		{
			FEElement_* pe = m_pm->ElementPtr(face.eid);
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
		glNormal3fv(&face.vn[0].x); glVertex3fv(&face.vr[0].x);
		glNormal3fv(&face.vn[1].x); glVertex3fv(&face.vr[1].x);
		glNormal3fv(&face.vn[2].x); glVertex3fv(&face.vr[2].x);
	}

private:
	FSMesh* m_pm;
};

GLColor CGLModelScene::GetPartColor(CGLContext& rc, GPart* pg)
{
	if (pg == nullptr) return GLColor(0, 0, 0);
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return GLColor(0,0,0);

	GLViewSettings& vs = rc.m_settings;
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSModel* fem = m_doc->GetFSModel();

	switch (m_objectColor)
	{
	case OBJECT_COLOR_MODE::DEFAULT_COLOR:
	{
		GMaterial* pmat = fem->GetMaterialFromID(pg->GetMaterialID());
		GLMaterial& glm = pmat->GetGLMaterial();
		GLColor c = po->GetColor();
		if (pmat) c = glm.diffuse;

		if (!pg->IsActive())
		{
			c = GLColor(128, 128, 128);
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
		return GLColor(255, 255, 255);
	}
	break;
	}
	return GLColor(0, 0, 0);
}

void CGLModelScene::RenderTags(CGLContext& rc)
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
					FEElement_& el = *selection->Element(i); assert(el.IsSelected());
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

void CGLModelScene::RenderRigidLabels(CGLContext& rc)
{
	FSModel* fem = m_doc->GetFSModel();
	if (fem == nullptr) return;

	GLViewSettings& view = rc.m_settings;

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
}

OBJECT_COLOR_MODE CGLModelScene::ObjectColorMode() const
{
	return m_objectColor;
}

void CGLModelScene::Update()
{
	m_buildScene = true;
	CGLScene::Update();
}

void RenderBoxCut(GLMeshRender& meshRender, CGLContext& rc, const BOX& box)
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
	CGLView* view = dynamic_cast<CGLView*>(rc.m_view);
	double* plane = view->PlaneCoordinates();
	vec3d norm(plane[0], plane[1], plane[2]);
	double ref = -(plane[3] - R * 0.001);
	for (int k = 0; k < 8; ++k)
		if (norm * ex[k] > ref) ncase |= (1 << k);
	if ((ncase > 0) && (ncase < 255))
	{
		int edge[15][2], edgeNode[15][2], etag[15];
		GMesh plane;
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

					GMesh::FACE& face = plane.Face(plane.Faces() - 1);
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

		meshRender.SetLineShader(new GLLineColorShader(GLColor(255, 64, 255)));
		meshRender.RenderEdges(plane);
	}
}

bool BuildSelectionMesh(FESelection* sel, GMesh& mesh)
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
			FEElement_& el = *esel->Element(i); assert(el.IsSelected());
			if (el.IsSolid())
			{
				int nf = el.Faces();
				for (int j = 0; j < nf; ++j)
				{
					int nj = el.m_nbr[j];
					FEElement_* pej = pm->ElementPtr(nj);
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

void GLPlaneCutItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	if (rc.m_settings.m_showPlaneCut == false) return;

	BOX box = m_scene->GetBoundingBox();
	glColor3ub(200, 0, 200);
	glx::renderBox(box, false);

	RenderBoxCut(m_scene->GetMeshRenderer(), rc, box);

	CGLView* view = dynamic_cast<CGLView*>(rc.m_view);
	if (view->PlaneCutMode() == 0)
	{
		// render the plane cut first
		view->RenderPlaneCut(rc);

		// then turn on the clipping plane before rendering the other geometry
		re.setClipPlane(0, view->PlaneCoordinates());
		re.enable(GLRenderEngine::CLIPPLANE);
	}
}

void GLObjectItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	GLViewSettings& vs = rc.m_settings;
	int nitem = m_scene->GetItemMode();
	if ((vs.m_nrender == RENDER_SOLID) || (nitem != ITEM_MESH))
	{
		if (m_po && m_po->IsValid())
		{
			glPushMatrix();
			SetModelView(m_po);
			RenderGObject(re, rc);
			glPopMatrix();
		}
	}
}

void GLObjectItem::RenderGObject(GLRenderEngine& re, CGLContext& rc) const
{
	GLViewSettings& view = rc.m_settings;

	CGLCamera& cam = *rc.m_cam;

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
				GMesh* gm = po->GetFERenderMesh();
				if (gm) RenderFEFacesFromGMesh(rc);
				else if (po->GetEditableMesh()) RenderSurfaceMeshFaces(re, rc);
				else RenderObject(re, rc);
			}
			else if (objectColor == OBJECT_COLOR_MODE::FSELEMENT_TYPE)
			{
				GMesh* gm = po->GetFERenderMesh();
				if (gm) RenderFEFacesFromGMesh(rc);
				else RenderObject(re, rc);
			}
			else if (view.m_showPlaneCut && (view.m_planeCutMode == Planecut_Mode::HIDE_ELEMENTS))
			{
				GMesh* gm = po->GetFERenderMesh();
				if (gm) RenderFEFacesFromGMesh(rc);
			}
			else RenderObject(re, rc);
		}
		break;
		case SELECT_PART: RenderParts(rc); break;
		case SELECT_FACE: RenderSurfaces(rc); break;
		case SELECT_EDGE:
		{
			RenderObject(re, rc);
			cam.LineDrawMode(true);
			cam.PositionInScene();
			SetModelView(po);
			RenderEdges(rc);
			cam.LineDrawMode(false);
			cam.PositionInScene();
			SetModelView(po);
		}
		break;
		case SELECT_NODE:
		{
			RenderObject(re, rc);
			cam.LineDrawMode(true);
			cam.PositionInScene();
			SetModelView(po);
			RenderNodes(rc);
			cam.LineDrawMode(false);
			cam.PositionInScene();
			SetModelView(po);
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
				RenderFEFacesFromGMesh(rc);
				RenderUnselectedBeamElements(rc);
				RenderSelectedFEElements(rc);
			}
			else if (item == ITEM_FACE)
			{
				GMesh* gm = po->GetFERenderMesh(); assert(gm);
				if (gm)
				{
					RenderFEFacesFromGMesh(rc);
					RenderAllBeamElements(rc);
					RenderSelectedFEFaces(rc);
				}
			}
			else if (item == ITEM_EDGE)
			{
				GMesh* gm = po->GetFERenderMesh(); assert(gm);
				if (gm) RenderFEFacesFromGMesh(rc);
				cam.LineDrawMode(true);
				cam.PositionInScene();
				SetModelView(po);
				RenderFEEdges(rc);
				cam.LineDrawMode(false);
				cam.PositionInScene();
			}
			else if (item == ITEM_NODE)
			{
				GMesh* gm = po->GetFERenderMesh(); assert(gm);
				if (gm) RenderFEFacesFromGMesh(rc);
				RenderFENodes(rc);
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
				cam.LineDrawMode(true);
				cam.PositionInScene();
				SetModelView(po);
				RenderSurfaceMeshEdges(rc);
				cam.LineDrawMode(false);
				cam.PositionInScene();
			}
			else if (item == ITEM_NODE)
			{
				RenderSurfaceMeshFaces(re, rc);
				RenderSurfaceMeshNodes(rc);
			}
		}
	}

	// render normals if requested
	if (view.m_bnorm) RenderNormals(rc, view.m_scaleNormals);
}

// render non-selected parts
void GLObjectItem::RenderParts(CGLContext& rc) const
{
	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GLViewSettings& vs = rc.m_settings;

	GObject* po = m_po;

	// get the GMesh
	FSModel& fem = *m_scene->GetFSModel();
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
			bool useStipple = false;
			if (vs.m_transparencyMode != 0)
			{
				switch (vs.m_transparencyMode)
				{
				case 1: if (po->IsSelected()) useStipple = true; break;
				case 2: if (!po->IsSelected()) useStipple = true; break;
				}
			}

			// render the face
			GLColor c = m_scene->GetPartColor(rc, pg);
			GLStandardModelShader shader(c, useStipple);
			int nid = pg->GetLocalID();
			renderer.RenderGMesh(*pm, n, shader);
		}
	}

	RenderBeamParts(rc);
}

// Render non-selected surfaces
void GLObjectItem::RenderSurfaces(CGLContext& rc) const
{
	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GLViewSettings& vs = rc.m_settings;

	GObject* po = m_po;

	// get the GMesh
	FSModel& fem = *m_scene->GetFSModel();
	GMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == nullptr) return;

	int objectColor = m_scene->GetObjectColorMode();

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
				GLColor c;
				bool useStipple = false;
				if (objectColor == OBJECT_COLOR_MODE::PHYSICS_TYPE)
				{
					GLfloat col[] = { 0.f, 0.f, 0.f, 1.f };
					switch (f.m_ntag)
					{
					case 0: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.9f; useStipple = true; break;
					case 1: col[0] = 0.9f; col[1] = 0.9f; col[2] = 0.0f; break;	// boundary conditions
					case 2: col[0] = 0.0f; col[1] = 0.4f; col[2] = 0.0f; break;	// initial conditions
					case 3: col[0] = 0.0f; col[1] = 0.9f; col[2] = 0.9f; break;	// loads
					case 4: col[0] = 0.9f; col[1] = 0.0f; col[2] = 0.9f; break;	// contact primary
					case 5: col[0] = 0.3f; col[1] = 0.0f; col[2] = 0.3f; break;	// contact secondary
					}
					c = GLColor::FromRGBf(col[0], col[1], col[2]);
				}
				else c = m_scene->GetPartColor(rc, pg);

				if (vs.m_transparencyMode != 0)
				{
					switch (vs.m_transparencyMode)
					{
					case 1: if (po->IsSelected()) useStipple = true; break;
					case 2: if (!po->IsSelected()) useStipple = true; break;
					}
				}

				// render the face
				GLStandardModelShader shader(c, useStipple);
				renderer.RenderGMesh(*pm, n, shader);
			}
		}
	}
}

// Render non-selected nodes
void GLObjectItem::RenderNodes(CGLContext& rc) const
{
	GObject* po = m_po;
	if ((po == nullptr) || (po->Nodes() == 0)) return;

	GMesh points;
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

	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.SetPointShader(new GLPointColorShader(GLColor::Blue()));
	renderer.RenderPoints(points);
}

// render non-selected edges
void GLObjectItem::RenderEdges(CGLContext& rc) const
{
	GObject* po = m_po;
	GMesh* m = po->GetRenderMesh();
	if (m == nullptr) return;

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glColor3ub(0, 0, 255);

	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.SetLineShader(new GLLineColorShader(GLColor::Blue()));

	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected() == false)
		{
			renderer.RenderEdges(*m, i);
		}
	}
	glPopAttrib();
}

void GLObjectItem::RenderFEFacesFromGMesh(CGLContext& rc) const
{
	GObject* po = m_po;

	GMesh* gm = po->GetFERenderMesh();
	if (gm == nullptr) return;

	GLViewSettings& vs = rc.m_settings;

	if (vs.m_bcontour && (po == m_scene->GetActiveObject()))
	{
		GLFaceColorShader shader;
		m_scene->GetMeshRenderer().RenderGMesh(*gm, shader);
	}
	else
	{
		int objectColor = m_scene->GetObjectColorMode();
		switch (objectColor)
		{
		case OBJECT_COLOR_MODE::DEFAULT_COLOR : RenderMeshByDefault(rc, *gm); break;
		case OBJECT_COLOR_MODE::OBJECT_COLOR  : RenderMeshByObjectColor(rc, *gm); break;
		case OBJECT_COLOR_MODE::MATERIAL_TYPE : RenderMeshByMaterialType(rc, *gm); break;
		case OBJECT_COLOR_MODE::PHYSICS_TYPE  : RenderMeshByPhysics(rc, *gm); break;
		case OBJECT_COLOR_MODE::FSELEMENT_TYPE: RenderMeshByElementType(rc, *gm); break;
		default:
			assert(false);
		}
	}
}

void GLObjectItem::RenderMeshByDefault(CGLContext& rc, GMesh& mesh) const
{
	FSModel* fem = m_scene->GetFSModel();

	FSMesh* pm = m_po->GetFEMesh();
	if (pm == nullptr) return;

	std::vector<GLFacetShader*> shaders = BuildMaterialShaders(fem);
	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.ClearShaders();
	for (GLFacetShader* s : shaders) renderer.AddShader(s);

	GLStandardModelShader defaultShader;
	renderer.SetDefaultShader(&defaultShader);

	renderer.SetUseShaders(true);
	renderer.RenderGMesh(mesh);
	renderer.SetUseShaders(false);
	renderer.ClearShaders();
}

void GLObjectItem::RenderMeshByObjectColor(CGLContext& rc, GMesh& mesh) const
{
	GLStandardModelShader shader(m_po->GetColor());
	m_scene->GetMeshRenderer().RenderGMesh(mesh, shader);
}

void GLObjectItem::RenderMeshByMaterialType(CGLContext& rc, GMesh& mesh) const
{
	FSModel* fem = m_scene->GetFSModel();

	int nmat = 0;
	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* mat = fem->GetMaterial(i);
		if (mat->GetID() > nmat) nmat = mat->GetID();
	}
	nmat++;
	std::vector<GLFacetShader*> shaders(nmat, nullptr);

	for (int i = 0; i < fem->Materials(); ++i)
	{
		GMaterial* mat = fem->GetMaterial(i);
		GLColor c = GetMaterialTypeColor(mat);
		shaders[mat->GetID()] = new GLStandardModelShader(c);
	}

	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.ClearShaders();
	for (GLFacetShader* s : shaders) renderer.AddShader(s);

	GLStandardModelShader defaultShader(GetMaterialTypeColor(nullptr));
	renderer.SetDefaultShader(&defaultShader);

	renderer.SetUseShaders(true);
	renderer.RenderGMesh(mesh);
	renderer.SetUseShaders(false);

	renderer.SetDefaultShader(nullptr);
}

void GLObjectItem::RenderMeshByPhysics(CGLContext& rc, GMesh& mesh) const
{
	const int MAX_COLORS = 6;
	GLColor CLT[MAX_COLORS] = {
		{230, 230, 230}, // free surface
		{230, 230,   0}, // boundary conditions
		{  0, 102,   0}, // initial conditions
		{  0, 230, 230}, // loads
		{230,   0, 230}, // contact primary
		{ 77,   0,  77}  // contact secondary
	};

	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GObject& o = *m_po;
	int NF = o.Faces();
	for (int i = 0; i < NF; ++i)
	{
		GFace* face = o.Face(i);
		if (face->IsVisible())
		{
			GPart* pg = o.Part(face->m_nPID[0]);
			if (!pg->IsVisible() && (face->m_nPID[1] >= 0))
				pg = o.Part(face->m_nPID[1]);

			bool useStipple = false;
			GLColor c(0, 0, 0);
			int n = face->m_ntag;
			if ((n >= 0) && (n < MAX_COLORS))
			{
				useStipple = true;
				c = CLT[n];
			}

			GLStandardModelShader shader(c, useStipple);
			renderer.RenderGMesh(mesh, i, shader);
		}
	}

	if (m_scene->GetItemMode() == ITEM_ELEM)
	{
		// exposed facets cannot be part of physics, so render them transparent
		GLStandardModelShader shader(CLT[0], true);
		renderer.RenderGMesh(mesh, NF, shader);
	}
}

void GLObjectItem::RenderMeshByElementType(CGLContext& rc, GMesh& mesh) const
{
	FSMesh* pm = m_po->GetFEMesh();
	if (pm == nullptr) return;

	GLElementTypeShader shader(pm);
	m_scene->GetMeshRenderer().RenderGMesh(mesh, shader);
}

// Render the FE nodes
void GLObjectItem::RenderFENodes(CGLContext& rc) const
{
	GObject* po = m_po;
	GLViewSettings& view = rc.m_settings;
	quatd q = rc.m_cam->GetOrientation();

	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	// set the point size
	float fsize = view.m_node_size;
	renderer.SetPointSize(fsize);

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	GMesh* gm = po->GetFERenderMesh(); assert(gm);
	assert(gm->Nodes() == pm->Nodes());
	int N = pm->Nodes();
	for (int i = 0; i < N; ++i)
	{
		FSNode& node = pm->Node(i);
		GMesh::NODE& v = gm->Node(i);
		if (node.IsSelected()) v.tag = 1;
		else
		{
			if (!node.IsVisible() ||
				(view.m_bext && !node.IsExterior())) v.tag = 0;
			else v.tag = 1;
		}
	}
	renderer.SetPointShader(new GLPointColorShader(GLColor(0, 0, 255, 128)));
	renderer.RenderPoints(*gm, [](const GMesh::NODE& v) {
		return (v.tag != 0);
		});

	FENodeSelection* sel = dynamic_cast<FENodeSelection*>(m_scene->GetCurrentSelection());
	if (sel && sel->Size())
	{
		std::vector<int> items = sel->Items();
		renderer.SetPointShader(new GLPointOverlayShader(GLColor::Red()));
		renderer.RenderPoints(*gm, items);
	}
}

void GLObjectItem::RenderSelectedFEFaces(CGLContext& rc) const
{
	GObject* po = m_po;

	FEFaceSelection* sel = dynamic_cast<FEFaceSelection*>(m_scene->GetCurrentSelection());
	if ((sel == nullptr) || (sel->Count() == 0)) return;
	if (sel->GetMesh() != po->GetFEMesh()) return;

	GLSelectionShader shader(GLColor::Red());
	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.RenderGMesh(m_scene->GetSelectionMesh(), shader);

	renderer.SetLineShader(new GLOutlineShader(GLColor::Yellow()));
	renderer.RenderEdges(m_scene->GetSelectionMesh());
}

void GLObjectItem::RenderSelectedFEElements(CGLContext& rc) const
{
	GObject* po = m_po;
		
	FEElementSelection* sel = dynamic_cast<FEElementSelection*>(m_scene->GetCurrentSelection());
	if ((sel == nullptr) || (sel->Count() == 0)) return;
	if (sel->GetMesh() != po->GetFEMesh()) return;

	GLSelectionShader shader;
	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.RenderGMesh(m_scene->GetSelectionMesh(), shader);

	// render a yellow highlight around selected elements
	renderer.SetLineShader(new GLOutlineShader(GLColor::Yellow()));
	renderer.RenderEdges(m_scene->GetSelectionMesh());
}

void GLObjectItem::RenderSurfaceMeshNodes(CGLContext& rc) const
{
	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GLViewSettings& view = rc.m_settings;
	quatd q = rc.m_cam->GetOrientation();

	// set the point size
	float fsize = view.m_node_size;
	renderer.SetPointSize(fsize);

	FSMeshBase* mesh = m_po->GetEditableMesh();
	if (mesh)
	{
		GMesh* gm = m_po->GetRenderMesh(); assert(gm);
		if (gm == nullptr) return;
		assert(gm->Nodes() == mesh->Nodes());

		// reset all tags
		int NN = mesh->Nodes();
		for (int i = 0; i < NN; ++i)
		{
			FSNode& node = mesh->Node(i);
			GMesh::NODE& gn = gm->Node(i);
			gn.tag = (node.IsVisible() ? 1 : 0);
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
					if (f.z < 0) gm->Node(face.n[j]).tag = 0;
				}
			}
		}

		renderer.SetPointShader(new GLPointColorShader(GLColor(0, 0, 255, 128)));
		renderer.RenderPoints(*gm, [](const GMesh::NODE& v) {
			return (v.tag != 0);
			});
	}
}

//-----------------------------------------------------------------------------
// Render the FE Edges
void GLObjectItem::RenderFEEdges(CGLContext& rc) const
{
	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GLLineColorShader* shader = new GLLineColorShader();
	renderer.SetLineShader(shader);

	// render the unselected edges
	GMesh* mesh = m_po->GetFERenderMesh();
	if (mesh)
	{
		shader->SetColor(GLColor(0, 0, 255, 128));
		renderer.RenderEdges(*mesh);
	}

	// render the selected edges
	shader->SetColor(GLColor(255, 0, 0, 128));
	renderer.RenderEdges(m_scene->GetSelectionMesh());
}

void GLObjectItem::RenderAllBeamElements(CGLContext& rc) const
{
	GObject* po = m_po;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	GMesh beamMesh;
	vec3f r[3];
	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsVisible())
		{
			GPart* pg = po->Part(el.m_gid);
			if (pg->IsVisible())
			{
				r[0] = to_vec3f(pm->Node(el.m_node[0]).r);
				r[1] = to_vec3f(pm->Node(el.m_node[1]).r);
				switch (el.Type())
				{
				case FE_BEAM2:
					beamMesh.AddEdge(r, 2);
					break;
				case FE_BEAM3:
					r[2] = to_vec3f(pm->Node(el.m_node[2]).r);
					beamMesh.AddEdge(r, 3);
					break;
				}
			}
		}
	}
	if (beamMesh.Edges() == 0) return;

	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.SetLineShader(new GLLineColorShader());
	renderer.RenderEdges(beamMesh);
}

void GLObjectItem::RenderUnselectedBeamElements(CGLContext& rc) const
{
	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GObject* po = m_po;
	if (po == nullptr) return;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	GMesh beamMesh;
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

	renderer.SetLineShader(new GLLineColorShader(po->GetColor()));
	renderer.RenderEdges(beamMesh);
}

void GLObjectItem::RenderNormals(CGLContext& rc, double scale) const
{
	if (m_po->IsVisible() == false) return;

	FSMeshBase* pm = m_po->GetEditableMesh();
	if (pm == 0) return;
	double R = 0.05 * pm->GetBoundingBox().GetMaxExtent() * scale;

	GMesh* mesh = m_po->GetFERenderMesh();
	if (mesh == nullptr) mesh = m_po->GetRenderMesh();
	if (mesh == nullptr) return;

	GLNormalShader shader;
	shader.SetScale(R);
	GLMeshRender& render = m_scene->GetMeshRenderer();
	render.RenderNormals(*mesh, shader);
}

void GLObjectItem::RenderBeamParts(CGLContext& rc) const
{
	GObject* po = m_po;
	if (!po->IsVisible()) return;

	int nitem = m_scene->GetItemMode();
	int nsel = m_scene->GetSelectionMode();

	GLViewSettings& vs = rc.m_settings;

	// get the GMesh
	FSModel& fem = *m_scene->GetFSModel();
	GMesh* pm = po->GetRenderMesh(); assert(pm);
	if (pm == 0) return;

	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GPart* pgmat = 0; // the part that defines the material
	GLLineColorShader* shader = new GLLineColorShader(po->GetColor());
	renderer.SetLineShader(shader);
	for (int i = 0; i < po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		if (pg->IsVisible() && pg->IsBeam())
		{
			if ((nitem == ITEM_MESH) && (nsel == SELECT_PART) && pg->IsSelected())
			{
				shader->SetColor(GLColor::Red());
			}
			else shader->SetColor(po->GetColor());

			for (int j = 0; j < pg->m_edge.size(); ++j)
			{
				GEdge& e = *po->Edge(pg->m_edge[j]);
				if (e.IsVisible())
					renderer.RenderEdges(*pm, e.GetLocalID());
			}
		}
	}
	glPopAttrib();
}

void GLObjectItem::RenderSurfaceMeshEdges(CGLContext& rc) const
{
	GMesh* mesh = m_po->GetRenderMesh();
	if (mesh == nullptr) return;

	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	// render the unselected edges
	GLLineColorShader* lineShader = new GLLineColorShader(GLColor::Blue());
	renderer.SetLineShader(lineShader);
	renderer.RenderEdges(*mesh);

	// render the selected edges
	lineShader->SetColor(GLColor::Red());
	renderer.RenderEdges(m_scene->GetSelectionMesh());
}

void GLObjectItem::RenderSelection(CGLContext& rc) const
{
	GMesh& selectionMesh = m_scene->GetSelectionMesh();
	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	GLSelectionShader shader;
	renderer.RenderGMesh(selectionMesh, shader);

	renderer.SetLineShader(new GLOutlineShader(GLColor::Yellow()));
	renderer.RenderEdges(selectionMesh);
}

// This function renders the object by looping over all the parts and
// for each part render the external surfaces that belong to that part.
// NOTE: The reason why only external surfaces are rendered is because
//       it is possible for an external surface to coincide with an
//       internal surface. E.g., when a shell layer lies on top of a 
//       hex layer.
void GLObjectItem::RenderObject(GLRenderEngine& re, CGLContext& rc) const
{
	re.setMaterial(GLMaterial::PLASTIC, m_po->GetColor());
	GMesh* mesh = m_po->GetRenderMesh();
	if (mesh == nullptr) return;
	
	int NF = m_po->Faces();
	for (int i = 0; i < NF; ++i)
	{
		GFace* pf = m_po->Face(i);
		if (pf->IsVisible())
		{
			GPart* pg = m_po->Part(pf->m_nPID[0]);
			if ((pg == nullptr) || !pg->IsVisible())
			{
				pg = m_po->Part(pf->m_nPID[1]);
			}

			if (pg && pg->IsVisible())
			{
				GLColor c = m_scene->GetPartColor(rc, pg);
				re.setColor(c);
				re.renderGMesh(*mesh, i);
			}
		}
	}

	if (NF == 0)
	{
		// if there are no faces, render edges instead
		GLMeshRender& renderer = m_scene->GetMeshRenderer();
		renderer.SetLineShader(new GLLineColorShader());
		int NC = m_po->Edges();
		for (int n = 0; n < NC; ++n)
		{
			GEdge& e = *m_po->Edge(n);
			if (e.IsVisible())
			{
				renderer.RenderEdges(*mesh, n);
			}
		}
	}

	// render beam sections if feature edges are not rendered. 
	if (rc.m_settings.m_bfeat == false)
	{
		RenderBeamParts(rc);
	}
}

void GLObjectItem::RenderSurfaceMeshFaces(GLRenderEngine& re, CGLContext& rc) const
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

	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GLViewSettings& view = rc.m_settings;

	Mesh_Data& data = surfaceMesh->GetMeshData();
	bool showContour = (view.m_bcontour && data.IsValid());

	// render the unselected faces
	if (showContour)
	{
		GMesh* gmesh = surfaceObject->GetRenderMesh();
		GLFaceColorShader shader;
		renderer.RenderGMesh(*gmesh, shader);
	}
	else
	{
		RenderObject(re, rc);
	}

	RenderSelection(rc);
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

			if (bsel && po->IsSelected()) glColor3ub(255, 255, 0);
			else if (!po->IsActive()) glColor3ub(128, 128, 128);
			else glColor3ub(c.r, c.g, c.b);

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

					if (bsel && el.IsSelected()) glColor3ub(255, 255, 0);
					else if (!po->IsActive()) glColor3ub(128, 128, 128);
					else glColor3ub(c.r, c.g, c.b);

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

void GLDiscreteItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	if (rc.m_settings.m_showDiscrete == false) return;

	if (m_lines.empty()) return;

	re.pushState();
	re.disable(GLRenderEngine::LIGHTING);
	for (const Line& line : m_lines)
	{
		GLColor c = line.col;
		glColor3ub(c.r, c.g, c.b);

		re.renderPoint(line.a);
		re.renderPoint(line.b);
		re.renderLine(line.a, line.b);
	}
	re.popState();
}

void GLSelectionBox::render(GLRenderEngine& re, CGLContext& rc) const
{
	GLViewSettings& view = rc.m_settings;

	// get the model
	FSModel* ps = m_scene->GetFSModel();
	GModel& model = ps->GetModel();

	// Get the item mode
	int item = m_scene->GetItemMode();

	// get the selection mode
	int nsel = m_scene->GetSelectionMode();

	GObject* poa = m_scene->GetActiveObject();

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

void GLMeshLinesItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	if (rc.m_settings.m_bmesh == false) return;

	CGLCamera& cam = *rc.m_cam;

	cam.LineDrawMode(true);
	cam.PositionInScene();

	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GModel& model = *m_scene->GetGModel();
	int nitem = m_scene->GetItemMode();

	GLViewSettings& vs = rc.m_settings;
	renderer.SetLineShader(new GLLineColorShader(vs.m_meshColor));
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
				if (nitem != ITEM_EDGE)
				{
					GMesh* lineMesh = po->GetFERenderMesh(); assert(lineMesh);
					if (lineMesh) renderer.RenderEdges(*lineMesh);
				}
				glPopMatrix();
			}
			else if (dynamic_cast<GSurfaceMeshObject*>(po))
			{
				GMesh* gmesh = po->GetRenderMesh();
				if (gmesh && (nitem != ITEM_EDGE))
				{
					glPushMatrix();
					SetModelView(po);
					renderer.RenderEdges(*gmesh);
					glPopMatrix();
				}
			}
		}
	}

	cam.LineDrawMode(false);
	cam.PositionInScene();
}

void GLFeatureEdgesItem::render(GLRenderEngine& re, CGLContext& rc) const
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

			CGLCamera& cam = *rc.m_cam;

			cam.LineDrawMode(true);
			cam.PositionInScene();

			GLMeshRender& renderer = m_scene->GetMeshRenderer();

			GLLineColorShader* shader = new GLLineColorShader();
			renderer.SetLineShader(shader);

			FSModel* ps = m_scene->GetFSModel();
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
						shader->SetColor(GLColor::Black());
						renderer.RenderEdges(*m, [](const GMesh::EDGE& e) {
							return (e.pid >= 0);
							});

						shader->SetColor(GLColor(64, 0, 16));
						renderer.RenderOutline(*rc.m_cam, m, po->GetRenderTransform(), (rc.m_settings.m_nrender == RENDER_WIREFRAME));
					}

					glPopMatrix();
				}
			}

			cam.LineDrawMode(false);
			cam.PositionInScene();
		}
	}
}

void GLPhysicsItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	GLViewSettings& vs = rc.m_settings;

	// render physics
	if (vs.m_brigid) RenderRigidBodies(rc);
	if (vs.m_bjoint) { RenderRigidJoints(rc); RenderRigidConnectors(rc); }
	if (vs.m_bwall) RenderRigidWalls(rc);
	if (vs.m_bfiber) RenderMaterialFibers(rc);
	if (vs.m_blma) RenderLocalMaterialAxes(rc);
}

void GLPhysicsItem::RenderRigidBodies(CGLContext& rc) const
{
	CGLCamera& cam = *rc.m_cam;

	FSModel* ps = m_scene->GetFSModel();

	double scale = 0.03 * (double)cam.GetTargetDistance();
	double R = 0.5 * scale;

	quatd qi = cam.GetOrientation().Inverse();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	for (int i = 0; i < ps->Materials(); ++i)
	{
		GMaterial* pgm = ps->GetMaterial(i);
		GLMaterial& glm = pgm->GetGLMaterial();
		FSMaterial* pm = pgm->GetMaterialProperties();
		if (pm && pm->IsRigid())
		{
			GLColor c = glm.diffuse;

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
		}
	}
	glPopAttrib();
}

void GLPhysicsItem::RenderRigidWalls(CGLContext& rc) const
{
	FSModel* ps = m_scene->GetFSModel();
	BOX box = ps->GetModel().GetBoundingBox();
	double R = box.GetMaxExtent();
	vec3d c = box.Center();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

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
					glx::renderRigidWall(R);
				}
				glPopMatrix();
			}
		}
	}

	glPopAttrib();
}

void GLPhysicsItem::RenderRigidJoints(CGLContext& rc) const
{
	CGLCamera& cam = *rc.m_cam;

	FSModel* ps = m_scene->GetFSModel();

	double scale = 0.05 * (double)cam.GetTargetDistance();
	double R = 0.5 * scale;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	for (int n = 0; n < ps->Steps(); ++n)
	{
		FSStep& s = *ps->GetStep(n);
		for (int i = 0; i < s.Interfaces(); ++i)
		{
			FSRigidJoint* pj = dynamic_cast<FSRigidJoint*> (s.Interface(i));
			if (pj)
			{
				vec3d r = pj->GetVecValue(FSRigidJoint::RJ);
				glx::renderJoint(r, R, GLColor::Red());
			}
		}
	}
	glPopAttrib();
}

void GLPhysicsItem::RenderRigidConnectors(CGLContext& rc) const
{
	CGLCamera& cam = *rc.m_cam;

	FSModel* ps = m_scene->GetFSModel();

	double scale = 0.05 * (double)cam.GetTargetDistance();
	double R = 0.5 * scale;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

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

				glx::renderJoint(r, R, c);
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

void GLPhysicsItem::RenderMaterialFibers(CGLContext& rc) const
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

	fiberRender->Init();
	fiberRender->RenderVectors();
	fiberRender->Finish();
}

void GLPhysicsItem::RenderLocalMaterialAxes(CGLContext& rc) const
{
	// get the model
	FSModel* ps = m_scene->GetFSModel();
	GModel& model = ps->GetModel();

	GLMeshRender& render = m_scene->GetMeshRenderer();

	FEElementRef rel;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	GLViewSettings& view = rc.m_settings;
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

								glColor3ub(rgb[0][k], rgb[1][k], rgb[2][k]);

								render.RenderLine(c, c + q * h);
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
								render.RenderLine(c, c + q * h);
							}
						}
					}
				}
			}
		}
	}

	glPopAttrib();
}

void GLSelectionItem::render(GLRenderEngine& re, CGLContext& rc) const
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
			glPushMatrix();
			SetModelView(po);
			switch (nsel)
			{
			case SELECT_PART: RenderSelectedParts(re, rc, po); break;
			case SELECT_FACE: RenderSelectedSurfaces(re, rc, po); break;
			case SELECT_EDGE: RenderSelectedEdges(re, rc, po); break;
			case SELECT_NODE: RenderSelectedNodes(re, rc, po); break;
			}
			glPopMatrix();
		}
	}
}

// Render selected nodes
void GLSelectionItem::RenderSelectedNodes(GLRenderEngine& re, CGLContext& rc, GObject* po) const
{
	if ((po == nullptr) || (po->Nodes() == 0)) return;

	GMesh points;
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

	GLMeshRender& renderer = m_scene->GetMeshRenderer();

	GLPointOverlayShader* shader = new GLPointOverlayShader(GLColor::Yellow());
	renderer.SetPointShader(shader);
	renderer.RenderPoints(points);

#ifndef NDEBUG
	// Draw FE nodes on top of GMesh nodes to make sure they match
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		GMesh fenodes;
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
			shader->SetColor(GLColor::Red());
			renderer.RenderPoints(fenodes);
		}
	}
#endif
}

// render selected edges
void GLSelectionItem::RenderSelectedEdges(GLRenderEngine& re, CGLContext& rc, GObject* po) const
{
	GMesh* m = po->GetRenderMesh();
	if (m == nullptr) return;

	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.SetLineShader(new GLOutlineShader(GLColor::Yellow()));

	GMesh pointMesh;
	int N = po->Edges();
	for (int i = 0; i < N; ++i)
	{
		GEdge& e = *po->Edge(i);
		if (e.IsSelected())
		{
			renderer.RenderEdges(*m, i);

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
		renderer.SetPointShader(new GLPointOverlayShader(GLColor::Yellow()));
		renderer.RenderPoints(pointMesh);
	}

#ifndef NDEBUG
	// Render FE edges onto GMesh edges to make sure they are consistent
	FSMesh* pm = po->GetFEMesh();
	if (pm)
	{
		// TODO: Add the edges to the render mesh
		GMesh edges;
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
			renderer.SetLineShader(new GLOutlineShader(GLColor::Red()));
			renderer.RenderEdges(edges);
		}
	}
#endif
}

// Render selected surfaces
void GLSelectionItem::RenderSelectedSurfaces(GLRenderEngine& re, CGLContext& rc, GObject* po) const
{
	if (!po->IsVisible()) return;

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
	re.pushState();
	re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Blue());

	for (int surfId : selectedSurfaces)
	{
		re.renderGMesh(*pm, surfId);
	}
	re.popState();


#ifndef NDEBUG
	GLSelectionShader shader(GLColor::Red());
	shader.Activate();
	{
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
	}
	shader.Deactivate();
#endif

	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.SetLineShader(new GLOutlineShader(GLColor::Blue()));
	for (int surfId : selectedSurfaces)
	{
		renderer.RenderSurfaceOutline(*rc.m_cam, pm, po->GetRenderTransform(), surfId);
	}
}

// render selected parts
void GLSelectionItem::RenderSelectedParts(GLRenderEngine& re, CGLContext& rc, GObject* po) const
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

	re.pushState();
	re.setMaterial(GLMaterial::HIGHLIGHT, GLColor::Blue());

	for (int surfId : facesToRender)
	{
		re.renderGMesh(*m, surfId);
	}
	re.popState();

	GLMeshRender& renderer = m_scene->GetMeshRenderer();
	renderer.SetLineShader(new GLOutlineShader(GLColor::Blue()));
	for (int surfId : facesToRender)
	{
		renderer.RenderSurfaceOutline(*rc.m_cam, m, po->GetRenderTransform(), surfId);
	}
}

void GLDisableClipPlaneItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	glDisable(GL_CLIP_PLANE0);
}

void GLGridItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	if (rc.m_settings.m_bgrid)
	{
		GGrid& grid = m_scene->GetGrid();
		grid.Render(rc);
	}
}

void GL3DImageItem::render(GLRenderEngine& re, CGLContext& rc) const
{
	m_img->Render(rc);
}
