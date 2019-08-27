#include "stdafx.h"
#include "GLModel.h"
#include "GLPlaneCutPlot.h"
#include "PostLib/FEDataManager.h"
#include <GLLib/GLContext.h>
#include "PostLib/constants.h"
#include <GLLib/GLCamera.h>
#include "PostLib/FENodeEdgeList.h"
#include "GLWLib/GLWidgetManager.h"
#include <stack>
using namespace std;
using namespace Post;

//-----------------------------------------------------------------------------
extern int ET_HEX[12][2];
extern int ET_HEX20[12][3];
extern int ET_TET[6][2];
extern int ET_PENTA[9][2];
extern int ET_PENTA15[9][3];
extern int ET_TET10[6][3];
extern int ET_PYRA5[8][2];

//-----------------------------------------------------------------------------
// constructor
CGLModel::CGLModel(FEModel* ps)
{
	m_nTime = 0;
	m_fTime = 0.f;

	static int layer = 1;
	m_layer = layer++;

	m_stol = 60.*PI / 180.0;

	m_ps = ps;

	CGLWidgetManager::GetInstance()->SetActiveLayer(m_layer);

	// see if the mesh has any vector fields
	// which can be used for displacement maps
	FEDataManager* pdm = ps->GetDataManager();
	FEDataFieldPtr pd = pdm->FirstDataField();
	int ndisp = -1;
	for (int i=0; i<pdm->DataFields(); ++i, ++pd)
	{
		if ((*pd)->Type() == DATA_VEC3F)
		{
			std::string sname = (*pd)->GetName();
			if ((sname == "displacement") || (sname == "Displacement")) ndisp = i;
		}
	}

	m_pdis = 0;
	if (ndisp != -1)
	{
		ps->SetDisplacementField(BUILD_FIELD(1, ndisp, 0));
		m_pdis = new CGLDisplacementMap(this);
	}

	// add a default color map
	m_pcol = new CGLColorMap(this);

	SetName("Model");

	m_bnorm   = false;
	m_bsmooth = true;
	m_bghost = false;
	m_nDivs = 0; // this means "auto"
	m_brenderInteriorNodes = true;

	m_bshowMesh = true;

	m_bShell2Hex  = false;
	m_nshellref   = 0;

	m_line_col = GLColor(0,0,0);
	m_node_col = GLColor(0,0,255);
	m_sel_col  = GLColor(255,0,0);

	m_nrender = RENDER_MODE_SOLID;
    
    m_nconv = CONV_FR_XZ;

	m_selectMode = SELECT_ELEMS;
	m_selectStyle = SELECT_RECT;

	UpdateEdge();
	UpdateInternalSurfaces();
}

//-----------------------------------------------------------------------------
//! destructor
CGLModel::~CGLModel(void)
{
	delete m_pdis;
	delete m_pcol;
	ClearInternalSurfaces();
}

//-----------------------------------------------------------------------------
FEMeshBase* CGLModel::GetActiveMesh()
{
	FEModel* pfem = GetFEModel();
	if (pfem && (pfem->GetStates() > 0)) return pfem->GetState(m_nTime)->GetFEMesh();
	return pfem->GetFEMesh(0);
}

//-----------------------------------------------------------------------------
void CGLModel::ResetAllStates()
{
	FEModel* fem = GetFEModel();
	if ((fem == 0) || (fem->GetStates() == 0)) return;

	int N = fem->GetStates();
	for (int i=0; i<N; ++i)
	{
		FEState* ps = fem->GetState(i);
		ps->m_nField = -1;
	}
}

//-----------------------------------------------------------------------------
float CGLModel::GetTimeValue(int ntime)
{ 
	FEModel* pfem = GetFEModel();
	if (pfem && (pfem->GetStates() > 0)) return pfem->GetState(ntime)->m_time;
	return 0.f; 
}

//-----------------------------------------------------------------------------
void CGLModel::setCurrentTimeIndex(int ntime)
{
	m_nTime = ntime;
	m_fTime = GetTimeValue(m_nTime);
}

//-----------------------------------------------------------------------------
void CGLModel::SetTimeValue(float ftime)
{
	m_nTime = GetClosestTime(ftime);
	m_fTime = ftime;
}

//------------------------------------------------------------------------------------------
// This returns the time step whose time value is closest but less than t
//
int CGLModel::GetClosestTime(double t)
{
	FEModel* pfem = GetFEModel();
	if ((pfem == 0) || (pfem->GetStates() <= 1)) return 0;

	FEState& s = *pfem->GetState(0);
	if (s.m_time >= t) return 0;

	for (int i=1; i<pfem->GetStates(); ++i)
	{
		FEState& s = *pfem->GetState(i);
		if (s.m_time >= t) return i-1;
	}
	return pfem->GetStates()-1;
}

//-----------------------------------------------------------------------------
FEState* CGLModel::currentState()
{
	return GetFEModel()->GetState(m_nTime);
}

//-----------------------------------------------------------------------------
// Update the model data
bool CGLModel::Update(bool breset)
{
	// get the time inc value
	float dt = m_fTime - GetTimeValue(m_nTime);

	// update the state of the mesh
	GetFEModel()->UpdateMeshState(m_nTime);
	UpdateInternalSurfaces(false);

	// update displacement map
	if (m_pdis && m_pdis->IsActive()) m_pdis->Update(m_nTime, dt, breset);

	// update the colormap
	if (m_pcol && m_pcol->IsActive()) m_pcol->Update(m_nTime, dt, breset);

	GetFEModel()->UpdateDependants();

	// update the plot list
	for (int i = 0; i < (int)m_pPlot.Size(); ++i)
	{
		CGLPlot* pi = m_pPlot[i];
		if (pi->IsActive()) pi->Update(currentTimeIndex(), 0.0, breset);
	}

	return true;
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateDisplacements(int nstate, bool breset)
{
	if (m_pdis && m_pdis->IsActive()) m_pdis->Update(nstate, 0.f, breset);
}

//-----------------------------------------------------------------------------
void CGLModel::SetMaterialParams(FEMaterial* pm)
{
	GLfloat fv[4] = {0,0,0,1};
	const float f = 1.f / 255.f;

	GLubyte a = (GLubyte) (255.f*pm->transparency);

	glColor4ub(pm->diffuse.r, pm->diffuse.g, pm->diffuse.b, a);

/*	fv[0] = (float) pm->ambient.r*f;
	fv[1] = (float) pm->ambient.g*f;
	fv[2] = (float) pm->ambient.b*f;
	fv[3] = (float) a*f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fv);
*/

	fv[0] = (float) pm->specular.r*f;
	fv[1] = (float) pm->specular.g*f;
	fv[2] = (float) pm->specular.b*f;
	fv[3] = 1.f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fv);

	fv[0] = (float) pm->emission.r*f;
	fv[1] = (float) pm->emission.g*f;
	fv[2] = (float) pm->emission.b*f;
	fv[3] = 1.f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, fv);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pm->shininess*64.f);
}

//-----------------------------------------------------------------------------
void CGLModel::SetSmoothingAngle(double w)
{ 
	m_stol = w*PI / 180.0;

	FEModel* ps = GetFEModel();
	if (ps == 0) return;

	FEMeshBase* pm = ps->GetFEMesh(0);
	pm->AutoSmooth(m_stol);
}

//-----------------------------------------------------------------------------
bool CGLModel::AddDisplacementMap(const char* szvectorField)
{
	if (szvectorField == nullptr) szvectorField = "displacement";

	FEModel* ps = GetFEModel();

	// see if the mesh has any vector fields
	// which can be used for displacement maps
	FEDataManager* pdm = ps->GetDataManager();
	FEDataFieldPtr pd = pdm->FirstDataField();
	int nv = 0;
	int ndisp = -1;
	for (int i=0; i<pdm->DataFields(); ++i, ++pd)
	{
		if ((*pd)->Type() == DATA_VEC3F) ++nv;
		if ((*pd)->GetName() == szvectorField) ndisp = i;
	}

	if (nv == 0) return false;

	if (m_pdis) delete m_pdis;
	m_pdis = new CGLDisplacementMap(this);
	if (ndisp != -1)
	{
		ps->SetDisplacementField(BUILD_FIELD(1, ndisp, 0));
	}
	else ps->SetDisplacementField(0);

	ResetAllStates();

	return true;
}

//-----------------------------------------------------------------------------
bool CGLModel::HasDisplacementMap()
{
	if (m_pdis == 0) return false;
	return (GetFEModel()->GetDisplacementField() != 0);
}

//-----------------------------------------------------------------------------
void CGLModel::ResetMesh()
{
	FEModel& fem = *GetFEModel();
	FEMeshBase& mesh = *fem.GetFEMesh(0);

	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FENode& node = mesh.Node(i);
		node.m_rt = node.m_r0;
	}

	// reevaluate normals
	mesh.UpdateNormals(RenderSmooth());
}
//-----------------------------------------------------------------------------
void CGLModel::RemoveDisplacementMap()
{
	FEModel* ps = GetFEModel();
	ps->SetDisplacementField(0);
	delete m_pdis;
	m_pdis = 0;

	// reset the mesh
	ResetMesh();

	// just to be safe, let's reset all states to force them to reevaluate
	ResetAllStates();
}

//-----------------------------------------------------------------------------
void CGLModel::Render(CGLContext& rc)
{
	if (rc.m_showOutline)
	{
		RenderOutline(rc);
	}

	// first we render all the plots
	RenderPlots(rc);

	// activate all clipping planes
	CGLPlaneCutPlot::EnableClipPlanes();

	// set the render interior nodes flag
	RenderInteriorNodes(rc.m_bext == false);

	// get the FE model
	FEModel* fem = GetFEModel();

	m_bshowMesh = rc.m_showMesh;

	// Render discrete elements
	float lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(rc.m_springThick);
	RenderDiscrete(rc);
	glLineWidth(lineWidth);

	int mode = GetSelectionMode();

	// render the faces
	if (mode == SELECT_FACES)
	{
		RenderFaces(m_ps, rc);
	}
	else if (mode == SELECT_ELEMS)
	{
		RenderElems(m_ps, rc);
	}
	else
	{
		// for nodes, edges, draw the faces as well
		RenderSurface(m_ps, rc);
	}

	// render the selected elements and faces
	RenderSelection(rc);

	// render the normals
	if (m_bnorm) RenderNormals(rc);

	// render the ghost
	if (m_bghost) RenderGhost(rc);

	// render the edges
	if (mode == SELECT_EDGES)
	{
		glDepthRange(0, 0.999999);
		RenderEdges(fem, rc);
		glDepthRange(0, 1);
	}

	// render the nodes
	if (mode == SELECT_NODES)
	{
		glDepthRange(0, 0.999985);
		RenderNodes(fem, rc);
		glDepthRange(0, 1);
	}

	// render decorations
	RenderDecorations();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderPlots(CGLContext& rc)
{
	GPlotList& PL = m_pPlot;
	for (int i = 0; i<(int)PL.Size(); ++i)
	{
		CGLPlot* pl = m_pPlot[i];

		if (pl->AllowClipping()) CGLPlaneCutPlot::EnableClipPlanes();
		else CGLPlaneCutPlot::DisableClipPlanes();

		if (pl->IsActive()) pl->Render(rc);
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderDiscrete(CGLContext& rc)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	FEMeshBase& mesh = *GetActiveMesh();
	int curMat = -1;
	bool bvisible = true;
	glBegin(GL_LINES);
	for (int i=0; i<m_edge.Edges(); ++i)
	{
		GLEdge::EDGE& edge = m_edge.Edge(i);
		int mat = edge.mat;
		if (mat != curMat)
		{
			FEMaterial* pmat = m_ps->GetMaterial(mat);
			GLColor c = pmat->diffuse;
			curMat = mat;
			bvisible = pmat->bvisible;
			glColor3ub(c.r, c.g, c.b);
		}

		if (bvisible)
		{
			vec3f r0 = mesh.Node(edge.n0).m_rt;
			vec3f r1 = mesh.Node(edge.n1).m_rt;

			glVertex3f(r0.x, r0.y, r0.z);
			glVertex3f(r1.x, r1.y, r1.z);
		}
	}
	glEnd();
	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderFaces(FEModel* ps, CGLContext& rc)
{
	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// we render the mesh by looping over the materials
	// first we render the opaque meshes
	for (int m=0; m<ps->Materials(); ++m)
	{
		// get the material
		FEMaterial* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency>.99f)) 
		{
			RenderSolidPart(ps, rc, m);
		}
	}

	// next, we render the transparent meshes
	for (int m=0; m<ps->Materials(); ++m)
	{
		// get the material
		FEMaterial* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency<=.99f) && (pmat->transparency>0.001f)) 
		{
			RenderSolidPart(ps, rc, m);
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderElems(FEModel* ps, CGLContext& rc)
{
	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// we render the mesh by looping over the materials
	// first we render the opaque meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		FEMaterial* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency>.99f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}

	// next, we render the transparent meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		FEMaterial* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency <= .99f) && (pmat->transparency>0.001f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSurface(FEModel* ps, CGLContext& rc)
{
	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// we render the mesh by looping over the materials
	// first we render the opaque meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		FEMaterial* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency>.99f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}

	// next, we render the transparent meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		FEMaterial* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency <= .99f) && (pmat->transparency>0.001f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}
}

//-----------------------------------------------------------------------------

void CGLModel::RenderSelection(CGLContext &rc)
{
	bool bnode = m_pcol->DisplayNodalValues();

	int mode = GetSelectionMode();

	// get the mesh
	FEModel* ps = m_ps;
	FEMeshBase* pm = GetActiveMesh();

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);

	// now render the selected faces
	glDisable(GL_TEXTURE_1D);
	GLColor c = m_sel_col;
	glColor4ub(c.r,c.g,c.g,128);
	glDisable(GL_LIGHTING);

	int ndivs = GetSubDivisions();

	// render the selected faces
	if (mode == SELECT_FACES)
	{
		for (int i=0; i<pm->Faces(); ++i)
		{
			FEFace& face = pm->Face(i);
			if (face.IsSelected())
			{
				// okay, we got one, so let's render it
				RenderFace(face, pm, ndivs, bnode);
			}
		}
	}

	// render the selected elements
	if (mode == SELECT_ELEMS)
	{
		for (int i = 0; i<pm->Faces(); ++i)
		{
			FEFace& face = pm->Face(i);
			FEElement_& el = pm->Element(face.m_elem[0]);
			if (el.IsSelected())
			{
				// okay, we got one, so let's render it
				RenderFace(face, pm, ndivs, bnode);
			}
		}
	}

	// render the outline of the selected elements
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255,255,0);
	glLineWidth(1.5f);

	// do the selected elements first
	if (mode == SELECT_ELEMS)
	{
		const vector<FEElement_*> elemSelection = GetElementSelection();
		for (int i = 0; i<(int)elemSelection.size(); ++i)
		{
			FEElement_& el = *elemSelection[i]; assert(el.IsSelected());
			RenderElementOutline(el, pm);
		}
	}

	// now do the selected faces
	if (mode == SELECT_FACES)
	{
		vec3f r[FEFace::MAX_NODES];
		const vector<FEFace*> faceSelection = GetFaceSelection();
		for (int i = 0; i<(int)faceSelection.size(); ++i)
		{
			FEFace& f = *faceSelection[i]; 
			if (f.IsSelected() == false) continue;

			int n = f.Nodes();
			for (int j=0; j<n; ++j) r[j] = pm->Node(f.n[j]).m_rt;
			switch (f.m_type)
			{
			case FE_FACE_TRI3:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
				}
				glEnd();
				break;
			case FE_FACE_QUAD4:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
				}
				glEnd();
				break;
			case FE_FACE_TRI6:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
				}
				glEnd();
				break;
			case FE_FACE_TRI7:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
				}
				glEnd();
				break;
			case FE_FACE_QUAD8:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[6].x, r[6].y, r[6].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[7].x, r[7].y, r[7].z);
				}
				glEnd();
				break;
			case FE_FACE_QUAD9:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[6].x, r[6].y, r[6].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[7].x, r[7].y, r[7].z);
				}
				glEnd();
				break;
			default:
				assert(false);
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------

void CGLModel::RenderTransparentMaterial(CGLContext& rc, FEModel* ps, int m)
{
	FEMaterial* pmat = ps->GetMaterial(m);
	FEMeshBase* pm = GetActiveMesh();

	// get the camera's orientation
	quatd q = rc.m_cam->GetOrientation();

	// make sure a part with this material exists
	if (m >= pm->Parts()) return;

	// set the material properties
	bool benable = false;
	if (pmat->benable && m_pcol->IsActive())
	{
		benable = true;
		glEnable(GL_TEXTURE_1D);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		GLubyte a = (GLubyte) (255.f*pmat->transparency);
		glColor4ub(255,255,255,a);
		m_pcol->GetColorMap()->GetTexture().MakeCurrent();
	}
	else
	{
		glDisable(GL_TEXTURE_1D);
		SetMaterialParams(pmat);
	}

	// see if we allow the model to be clipped
	glPushAttrib(GL_ENABLE_BIT);
	if (pmat->bclip == false) CGLPlaneCutPlot::DisableClipPlanes();

	bool bnode = m_pcol->DisplayNodalValues();

	// render the unselected faces
	FEDomain& dom = pm->Domain(m);
	int NF = dom.Faces();

	// for better transparency we first draw all the backfacing polygons.
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_CULL_FACE);

	GLColor d = pmat->diffuse;
	GLColor c[4];
	double tm = pmat->transparency;

	int ndivs = GetSubDivisions();

	int mode = GetSelectionMode();

	glCullFace(GL_FRONT);
	for (int i=0; i<NF; ++i)
	{
		FEFace& face = dom.Face(i);
		FEElement_& el = pm->Element(face.m_elem[0]);

		if (((mode != SELECT_ELEMS) || !el.IsSelected()) && face.IsVisible())
		{
			GLubyte a[4];
			if (m_bsmooth)
			{
				for (int j=0; j<face.Nodes(); ++j)
				{
					vec3d r = face.m_nn[j];
					q.RotateVector(r);
					double z = 1-fabs(r.z);
					a[j] = (GLubyte)(255*(tm + 0.5*(1-tm)*(z*z)));
				}
			}
			else
			{
				vec3d r = face.m_fn;
				q.RotateVector(r);
				double z = 1-fabs(r.z);
				a[0] = a[1] = a[2] = a[3] = (GLubyte)(255*(tm + 0.5*(1-tm)*(z*z)));
			}

			if (benable)
			{
				c[0] = GLColor(255, 255, 255, a[0]);
				c[1] = GLColor(255, 255, 255, a[1]);
				c[2] = GLColor(255, 255, 255, a[2]);
				c[3] = GLColor(255, 255, 255, a[3]);
			}
			else
			{
				c[0] = GLColor(d.r, d.g, d.b, a[0]);
				c[1] = GLColor(d.r, d.g, d.b, a[1]);
				c[2] = GLColor(d.r, d.g, d.b, a[2]);
				c[3] = GLColor(d.r, d.g, d.b, a[3]);
			}

			// okay, we got one, so let's render it
			RenderFace(face, pm, c, ndivs, bnode);
		}
	}

	// and then we draw the front-facing ones.
	glCullFace(GL_BACK);
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = dom.Face(i);
		FEElement_& el = pm->Element(face.m_elem[0]);

		if (((mode != SELECT_ELEMS) || !el.IsSelected()) && face.IsVisible())
		{
			GLubyte a[4];
			if (m_bsmooth)
			{
				for (int j=0; j<face.Nodes(); ++j)
				{
					vec3d r = face.m_nn[j];
					q.RotateVector(r);
					double z = 1-fabs(r.z);
					a[j] = (GLubyte)(255*(tm + 0.5*(1-tm)*(z*z)));
				}
			}
			else
			{
				vec3d r = face.m_fn;
				q.RotateVector(r);
				double z = 1-fabs(r.z);
				a[0] = a[1] = a[2] = a[3] = (GLubyte)(255*(tm + 0.5*(1-tm)*(z*z)));
			}

			if (benable)
			{
				c[0] = GLColor(255, 255, 255, a[0]);
				c[1] = GLColor(255, 255, 255, a[1]);
				c[2] = GLColor(255, 255, 255, a[2]);
				c[3] = GLColor(255, 255, 255, a[3]);
			}
			else
			{
				c[0] = GLColor(d.r, d.g, d.b, a[0]);
				c[1] = GLColor(d.r, d.g, d.b, a[1]);
				c[2] = GLColor(d.r, d.g, d.b, a[2]);
				c[3] = GLColor(d.r, d.g, d.b, a[3]);
			}

			// okay, we got one, so let's render it
			RenderFace(face, pm, c, ndivs, bnode);
		}
	}
	glPopAttrib();

	// reset the polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// render the internal faces
	RenderInnerSurface(m);

	if (pmat->benable && m_pcol->IsActive())
	{
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderInnerSurface(int m)
{
	bool bnode = m_pcol->DisplayNodalValues();
	bool old_smooth = m_bsmooth;
	m_bsmooth = false;
	FEMeshBase* pm = GetActiveMesh();
	GLSurface& surf = *m_innerSurface[m];
	for (int i = 0; i<surf.Faces(); ++i)
	{
		FEFace& face = surf.Face(i);
		RenderFace(face, pm, 1, bnode);
	}
	m_bsmooth = old_smooth;
}

//-----------------------------------------------------------------------------
void CGLModel::RenderInnerSurfaceOutline(int m, int ndivs)
{
	FEMeshBase* pm = GetActiveMesh();
	GLSurface& inSurf = *m_innerSurface[m];
	for (int i = 0; i<inSurf.Faces(); ++i)
	{
		FEFace& facet = inSurf.Face(i);
		RenderFaceOutline(facet, pm, ndivs);
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSolidDomain(FEDomain& dom, bool btex, bool benable)
{
	FEMeshBase* pm = GetActiveMesh();
	int ndivs = GetSubDivisions();
	bool bnode = m_pcol->DisplayNodalValues();

	if (btex) glEnable(GL_TEXTURE_1D);

	// render active faces
	int NF = dom.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = dom.Face(i);
		if (face.m_ntag == 1)
		{
			// okay, we got one, so let's render it
			RenderFace(face, pm, ndivs, bnode);
		}
	}

	// render inactive faces
	if (btex) glDisable(GL_TEXTURE_1D);
	if (m_pcol->IsActive() && benable) glColor4ub(m_col_inactive.r, m_col_inactive.g, m_col_inactive.b, m_col_inactive.a);
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = dom.Face(i);
		if (face.m_ntag == 2)
		{
			// okay, we got one, so let's render it
			RenderFace(face, pm, ndivs, bnode);
		}
	}
	if (btex) glEnable(GL_TEXTURE_1D);
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSolidPart(FEModel* ps, CGLContext& rc, int mat)
{
	// get the material
	FEMaterial* pmat = ps->GetMaterial(mat);

	glPolygonOffset(1.0, 1.0);

	// set the rendering mode
	int nmode = m_nrender;
	if (pmat->m_nrender != RENDER_MODE_DEFAULT) nmode = pmat->m_nrender;

	if (nmode == RENDER_MODE_SOLID)
	{
		if ((pmat->transparency >= 0.99f) || (pmat->m_ntransmode == RENDER_TRANS_CONSTANT)) RenderSolidMaterial(ps, mat);
		else RenderTransparentMaterial(rc, ps, mat);

		RenderSolidMaterial(ps, mat);
	}
	else
	{
		RenderOutline(rc, mat);
	}

	glPolygonOffset(0.0, 0.0);

	// Render the mesh lines
	if (m_bshowMesh && (GetSelectionMode() != SELECT_EDGES))
	{
		// store attributes
		glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);

		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		if (pmat->bclip == false) CGLPlaneCutPlot::DisableClipPlanes();

		// make sure the material is visible
		if (pmat->bvisible && pmat->bmesh)
		{
//			glDepthRange(0, 0.999999);

			// set the material properties
			GLColor c = pmat->meshcol;
			glColor3ub(c.r, c.g, c.b);
			RenderMeshLines(ps, mat);

//			glDepthRange(0, 1.0);
		}
		CGLPlaneCutPlot::EnableClipPlanes();

		// restore attributes
		glPopAttrib();
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSolidMaterial(FEModel* ps, int m)
{
	// make sure a part with this material exists
	FEMeshBase* pm = GetActiveMesh();
	if (m >= pm->Domains()) return;

	// get the material
	FEMaterial* pmat = ps->GetMaterial(m);

	// get the transparency value
	GLubyte alpha = (GLubyte)(255.f*pmat->transparency);

	// set the color for inactive materials
	m_col_inactive = GLColor(200, 200, 200);
	m_col_inactive.a = alpha;

	// set the material properties
	SetMaterialParams(pmat);
	bool btex = false;
	if (pmat->benable && m_pcol->IsActive())
	{
		btex = true;
		glColor4ub(255,255,255, alpha);
		m_pcol->GetColorMap()->GetTexture().MakeCurrent();
	}
	else
	{
		btex = false;
	}

	// see if we allow the model to be clipped
	glPushAttrib(GL_ENABLE_BIT);
	if (pmat->bclip == false) CGLPlaneCutPlot::DisableClipPlanes();

	// get selection mode
	int mode = GetSelectionMode();

	// determine which faces to draw 
	// tag == 0 : no-draw
	// tag == 1 : draw active
	// tag == 2 : draw inactive
	// TODO: It seems that this can be precomputed and stored somewhere in the domains
	FEDomain& dom = pm->Domain(m);
	int NF = dom.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = dom.Face(i);
		FEElement_& el = pm->Element(face.m_elem[0]);
		// assume no-draw
		face.m_ntag = 0;

		// check render state
		if (el.IsVisible())
		{
			if (((mode != SELECT_ELEMS) || !el.IsSelected()) && ((mode != SELECT_FACES) || !face.IsSelected()) && face.IsVisible())
			{
				face.m_ntag = (face.IsActive() ? 1 : 2);
			}
		}
	}

	// do the rendering
	if (pmat->transparency > .999f)
	{
		RenderSolidDomain(dom, btex, pmat->benable);
	}
	else
	{
		// for better transparency we first draw all the backfacing polygons.
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_CULL_FACE);

		glCullFace(GL_FRONT);
		RenderSolidDomain(dom, btex, pmat->benable);

		// and then we draw the front-facing ones.
		glCullFace(GL_BACK);
		if (btex) glColor4ub(255, 255, 255, alpha);
		RenderSolidDomain(dom, btex, pmat->benable);

		glPopAttrib();
	}

	// render the internal surfaces
	if (mode != SELECT_FACES)
	{
		if (btex) glColor3ub(255,255,255);
		RenderInnerSurface(m);
	}

	if (pmat->benable && m_pcol->IsActive())
	{
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// This algorithm is identical to the RenderOutline, except that it uses the
// original coordinates instead of the current ones
void CGLModel::RenderGhost(CGLContext &rc)
{
	int i, j, n;
	int a, b;
	vec3f r1, r2;

	FEModel* ps = m_ps;
	FEMeshBase* pm = GetActiveMesh();

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glColor3ub(96,96,96);

	quatd q = rc.m_cam->GetOrientation();

	double eps = cos(GetSmoothingAngleRadians());

	for (i=0; i<pm->Faces(); ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.IsVisible())
		{
			n = f.Edges();
			for (j=0; j<n; ++j)
			{
				bool bdraw = false;

				if (f.m_nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					FEFace& f2 = pm->Face(f.m_nbr[j]);
					if (f.m_mat != f2.m_mat)
					{
						bdraw = true;
					}
					else if (f.m_fn*f2.m_fn <= eps)
					{
						bdraw = true;
					}
					else
					{
						vec3d n1 = f.m_fn;
						vec3d n2 = f2.m_fn;
						q.RotateVector(n1);
						q.RotateVector(n2);
						if (n1.z*n2.z <= 0) 
						{
							bdraw = true;
						}
					}
				}

				if (bdraw)
				{
					a = f.n[j];
					b = f.n[(j+1)%n];

					if (a > b) { a ^= b; b ^= a; a ^= b; }

					r1 = pm->Node(a).m_r0;
					r2 = pm->Node(b).m_r0;

					glBegin(GL_LINES);
					{
						glVertex3f(r1.x, r1.y, r1.z);
						glVertex3f(r2.x, r2.y, r2.z);
					}
					glEnd();
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderFaceEdge(FEFace& f, int j, FEMeshBase* pm, int ndivs)
{
	int n = f.Edges();
	int a = f.n[j];
	int b = f.n[(j+1) % n];
	if (a > b) { a ^= b; b ^= a; a ^= b; }

	switch (f.m_type)
	{
	case FE_FACE_TRI3:
	case FE_FACE_QUAD4:
		{
			vec3f r1 = pm->Node(a).m_rt;
			vec3f r2 = pm->Node(b).m_rt;

			glBegin(GL_LINES);
			{
				glVertex3f(r1.x, r1.y, r1.z);
				glVertex3f(r2.x, r2.y, r2.z);
			}
			glEnd();
		}
		break;
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		{
			vec3f r1 = pm->Node(a).m_rt;
			vec3f r2 = pm->Node(b).m_rt;
			vec3f r3 = pm->Node(f.n[j+4]).m_rt;

			glBegin(GL_LINES);
			{
				float r, H[3];
				vec3f p;
				int n = (ndivs<=1?2:ndivs);
				for (int i=0; i<n; ++i)
				{
					r = -1.f + 2.f*i/n;
					H[0] = 0.5f*r*(r - 1.f);
					H[1] = 0.5f*r*(r + 1.f);
					H[2] = 1.f - r*r;
					p = r1*H[0] + r2*H[1] + r3*H[2];
					glVertex3f(p.x, p.y, p.z); 

					r = -1.f + 2.f*(i+1)/n;
					H[0] = 0.5f*r*(r - 1.f);
					H[1] = 0.5f*r*(r + 1.f);
					H[2] = 1.f - r*r;
					p = r1*H[0] + r2*H[1] + r3*H[2];
					glVertex3f(p.x, p.y, p.z); 
				}
			}
			glEnd();						
		}
		break;
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
		{
			vec3f r1 = pm->Node(a).m_rt;
			vec3f r2 = pm->Node(b).m_rt;
			vec3f r3 = pm->Node(f.n[j+3]).m_rt;

			glBegin(GL_LINES);
			{
				float r, H[3];
				vec3f p;
				int n = (ndivs<=1?2:ndivs);
				for (int i=0; i<n; ++i)
				{
					r = -1.f + 2.f*i/n;
					H[0] = 0.5f*r*(r - 1.f);
					H[1] = 0.5f*r*(r + 1.f);
					H[2] = 1.f - r*r;
					p = r1*H[0] + r2*H[1] + r3*H[2];
					glVertex3f(p.x, p.y, p.z); 

					r = -1.f + 2.f*(i+1)/n;
					H[0] = 0.5f*r*(r - 1.f);
					H[1] = 0.5f*r*(r + 1.f);
					H[2] = 1.f - r*r;
					p = r1*H[0] + r2*H[1] + r3*H[2];
					glVertex3f(p.x, p.y, p.z); 
				}
			}
			glEnd();
		}
		break;
	case FE_FACE_TRI10:
		{
			// implement this
		}
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
// NOTE: This algorithm does not always give satisfactory results. 
// In the case of perspective projection, the normal product should 
// be less than some value depending on the location of the edge, 
// in stead of zero (which is the correct value for ortho projection).

void CGLModel::RenderOutline(CGLContext& rc, int nmat)
{
	FEModel* ps = m_ps;
	FEMeshBase* pm = GetActiveMesh();

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);

	GLColor c = m_line_col;
	glColor3ub(c.r,c.g,c.b);

	quatd q = rc.m_cam->GetOrientation();

	int ndivs = GetSubDivisions();

	for (int i=0; i<pm->Faces(); ++i)
	{
		FEFace& f = pm->Face(i);
		FEElement_& el = pm->Element(f.m_elem[0]);
		if (f.IsVisible() && ((nmat == -1) || (el.m_MatID == nmat)))
		{
			int n = f.Edges();
			for (int j=0; j<n; ++j)
			{
				bool bdraw = false;

				if (f.m_nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					FEFace& f2 = pm->Face(f.m_nbr[j]);
					if (f.m_mat != f2.m_mat)
					{
						bdraw = true;
					}
					else if (f.m_sid != f2.m_sid)
					{
						bdraw = true;
					}
/*					else
					{
						vec3f n1 = f.m_fn;
						vec3f n2 = f2.m_fn;
						q.RotateVector(n1);
						q.RotateVector(n2);
						if (n1.z*n2.z <= 0) 
						{
							bdraw = true;
						}
					}
*/				}

				if (bdraw) RenderFaceEdge(f, j, pm, ndivs);
			}
		}
	}

	glPopAttrib();
}


///////////////////////////////////////////////////////////////////////////////

void CGLModel::RenderNormals(CGLContext& rc)
{
	// get the mesh
	FEModel* ps = m_ps;
	FEMeshBase* pm = GetActiveMesh();

	BOX box = ps->GetBoundingBox();

	float scale = 0.05f*box.Radius();

	// store the attributes
	glPushAttrib(GL_ENABLE_BIT);

	// disable lighting
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);
	{
		// render the normals
		for (int i=0; i<pm->Faces(); ++i)
		{	
			FEFace& face = pm->Face(i);

			// see if it is visible
			if (face.IsVisible())
			{
				vec3f r1(0,0,0);

				int n = face.Nodes();
				for (int j = 0; j<n; ++j) r1 += pm->Node(face.n[j]).m_rt;
				r1 /= (float) n;

				GLfloat r = (GLfloat)fabs(face.m_fn.x);
				GLfloat g = (GLfloat)fabs(face.m_fn.y);
				GLfloat b = (GLfloat)fabs(face.m_fn.z);

				vec3f r2 = r1 + face.m_fn*scale;

				glColor3ub(255,255,255); glVertex3f(r1.x, r1.y, r1.z);
				glColor3f(r, g, b); glVertex3f(r2.x, r2.y, r2.z);
			}
		}
	}
	glEnd();

	// restore attributes
	glPopAttrib();
}


//-----------------------------------------------------------------------------
// Render a textured face.
void CGLModel::RenderTexFace(FEFace& face, FEMeshBase* pm)
{
	switch (face.m_type)
	{
	case FE_FACE_QUAD4: RenderTexQUAD4(face, pm); break;
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9: RenderTexQUAD8(face, pm); break;
	case FE_FACE_TRI3 : RenderTexTRI3(face, pm); break;
	case FE_FACE_TRI6 : RenderTexTRI6(face, pm); break;
	case FE_FACE_TRI7 : RenderTexTRI7(face, pm); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
// Render the mesh lines for a specific material
//
void CGLModel::RenderMeshLines(FEModel* ps, int nmat)
{
	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	int ndivs = GetSubDivisions();

	// now loop over all faces and see which face belongs to this material
	if (nmat < pm->Domains())
	{
		FEDomain& dom = pm->Domain(nmat);
		for (int i=0; i<dom.Faces(); ++i)
		{
			FEFace& face = dom.Face(i);
			FEElement_& el = pm->Element(face.m_elem[0]);
			if (face.IsVisible() && el.IsVisible())
			{
				// okay, we got one, so let's render it
				RenderFaceOutline(face, pm, ndivs);
			}
		}
	}

	int mode = GetSelectionMode();
	if (mode != SELECT_FACES)
	{
		// draw elements
		RenderInnerSurfaceOutline(nmat, ndivs);
	}
}

//-----------------------------------------------------------------------------
// Render the mesh lines of the model.

///////////////////////////////////////////////////////////////////////////////

void CGLModel::RenderShadows(FEModel* ps, const vec3d& lp, float inf)
{
	FEMeshBase* pm = GetActiveMesh();

	// find all silhouette edges
	vec3d fn, fn2;
	vec3d n(lp); n.Normalize();
	int m;
	bool bvalid;
	for (int i=0; i<pm->Faces(); i++)
	{
		FEFace& f = pm->Face(i);

		m = f.Edges();
		bvalid = true;
		if      (f.n[0] == f.n[1]) bvalid = false;
		else if (f.n[0] == f.n[2]) bvalid = false;
		else if (f.n[1] == f.n[2]) bvalid = false;

		FEElement_& el = pm->Element(f.m_elem[0]);
		FEMaterial* pmat = ps->GetMaterial(el.m_MatID);

		// see it this face is visible
		if (!f.IsVisible() || !pmat->bvisible) bvalid = false;

		// make sure this material casts shadows
		if (pmat->bcast_shadows == false) bvalid = false;

		if (bvalid)
		{
			// only look at front facing faces
			fn = f.m_fn;
			if (fn*n > 0)
			{
				for (int j=0; j<m; j++)
				{
					FEFace* pf2 = 0;
					if (f.m_nbr[j] >= 0) pf2 = &pm->Face(f.m_nbr[j]);

					if (pf2)
					{
						fn2 = pf2->m_fn;
					}

					// we got one!
					if ((pf2 == 0) || (fn2*n < 0))
					{
						vec3d a, b, c, d;
						a = pm->Node(f.n[j]).m_rt;
						b = pm->Node(f.n[(j+1)%m]).m_rt;
	
						c = a - n*inf;
						d = b - n*inf;

						glBegin(GL_QUADS);
						{
							vec3d n = (c-a)^(d-a);
							n.Normalize();

							glNormal3d(n.x, n.y, n.z);
							glVertex3d(a.x, a.y, a.z);
							glVertex3d(c.x, c.y, c.z);
							glVertex3d(d.x, d.y, d.z);
							glVertex3d(b.x, b.y, b.z);
						}
						glEnd();
					}
				}
			}
			else 
			{
				vec3f r1 = pm->Node(f.n[0]).m_rt;
				vec3f r2 = pm->Node(f.n[1]).m_rt;
				vec3f r3 = pm->Node(f.n[2]).m_rt;
				vec3f r4 = pm->Node(f.n[3]).m_rt;
	
				glNormal3d(-fn.x, -fn.y, -fn.z);

				switch (f.m_type)
				{
				case FE_FACE_QUAD4:
				case FE_FACE_QUAD8:
				case FE_FACE_QUAD9:
					glBegin(GL_QUADS);
					{
						glVertex3f(r4.x, r4.y, r4.z);
						glVertex3f(r3.x, r3.y, r3.z);
						glVertex3f(r2.x, r2.y, r2.z);
						glVertex3f(r1.x, r1.y, r1.z);
					}
					glEnd();
					break;
				case FE_FACE_TRI3:
				case FE_FACE_TRI6:
				case FE_FACE_TRI7:
					glBegin(GL_TRIANGLES);
					{
						glVertex3f(r3.x, r3.y, r3.z);
						glVertex3f(r2.x, r2.y, r2.z);
						glVertex3f(r1.x, r1.y, r1.z);
					}
					glEnd();
					break;
				default:
					assert(false);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CGLModel::RenderNodes(FEModel* ps, CGLContext& rc)
{
	FEMeshBase* pm = GetActiveMesh();

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);

	// reset tags and check visibility
	for (int i=0; i<pm->Nodes(); ++i)
	{
		FENode& n = pm->Node(i);
		n.m_ntag = (n.IsVisible() ? 1 : 0);
	}

	// see if backface-culling is enabled or not
	GLboolean bcull;
	glGetBooleanv(GL_CULL_FACE, &bcull);
	if (bcull)
	{
		quatd q = rc.m_cam->GetOrientation();
		vec3f f;
		int NF = pm->Faces();
		for (int i = 0; i<NF; ++i)
		{
			FEFace& face = pm->Face(i);
			int n = face.Nodes();
			for (int j = 0; j<n; ++j)
			{
				vec3d f = face.m_nn[j];
				q.RotateVector(f);
				if (f.z < 0) pm->Node(face.n[j]).m_ntag = 0;
			}
		}
	}

	// render all unselected tagged nodes
	glColor3ub(m_node_col.r, m_node_col.g, m_node_col.b);
	glBegin(GL_POINTS);
	for (int i = 0; i<pm->Nodes(); ++i)
	{
		FENode& node = pm->Node(i);
		if (node.m_bext && node.m_ntag && (node.IsSelected() == false))
		{
			// get the nodal coordinate
			vec3f& r = node.m_rt;

			// render the point
			glVertex3f(r.x, r.y, r.z);
		}
	}
	if (m_brenderInteriorNodes)
	{
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			if ((node.m_bext == false) && node.m_ntag && (node.IsSelected() == false))
			{
				// get the nodal coordinate
				vec3f& r = node.m_rt;

				// render the point
				glVertex3f(r.x, r.y, r.z);
			}
		}
	}
	glEnd();

	// render selected tagged nodes
	if (GetSelectionMode() == SELECT_NODES)
	{
		glDisable(GL_DEPTH_TEST);

		// render exterior selected nodes first
		glColor3ub(255, 255, 0);
		glBegin(GL_POINTS);
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			if (node.m_bext && node.m_ntag && node.IsSelected())
			{
				// get the nodal coordinate
				vec3f r = node.m_rt;

				// render the point
				glVertex3f(r.x, r.y, r.z);
			}
		}
		glEnd();

		// render interior nodes
		if (m_brenderInteriorNodes)
		{
			glColor3ub(255, 0, 0);
			glBegin(GL_POINTS);
			for (int i = 0; i<pm->Nodes(); ++i)
			{
				FENode& node = pm->Node(i);
				if ((node.m_bext == false) && node.m_ntag && node.IsSelected())
				{
					// get the nodal coordinate
					vec3f r = node.m_rt;

					// render the point
					glVertex3f(r.x, r.y, r.z);
				}
			}
			glEnd();
		}
	}

	// restore attributes
	glPopAttrib();
}

//-----------------------------------------------------------------------------

void CGLModel::RenderFace(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode)
{
	if (m_bShell2Hex)
	{
		int ntype = pm->Element(face.m_elem[0]).Type();
		if ((ntype == FE_QUAD4) || (ntype == FE_QUAD8) || (ntype == FE_QUAD9) || (ntype == FE_TRI3) || (ntype == FE_TRI6))
		{
			RenderThickShell(face, pm);
			return;
		}
	}

	// Render the facet
	switch (face.m_type)
	{
	case FE_FACE_QUAD4:
		if (ndivs == 1) RenderQUAD4(face, m_bsmooth, bnode);
		else RenderSmoothQUAD4(face, pm, ndivs, bnode);
		break;
	case FE_FACE_QUAD8:
		if (ndivs == 1) RenderQUAD8(face, m_bsmooth, bnode);
		else RenderSmoothQUAD8(face, pm, ndivs, bnode);
		break;
	case FE_FACE_QUAD9:
		if (ndivs == 1) RenderQUAD9(face, m_bsmooth, bnode);
		else RenderSmoothQUAD9(face, pm, ndivs, bnode);
		break;
	case FE_FACE_TRI3:
		RenderTRI3(face, m_bsmooth, bnode);
		break;
	case FE_FACE_TRI6:
		if (ndivs == 1) RenderTRI6(face, m_bsmooth, bnode);
		else RenderSmoothTRI6(face, pm, ndivs, bnode);
		break;
	case FE_FACE_TRI7:
		if (ndivs == 1) RenderTRI7(face, m_bsmooth, bnode);
		else RenderSmoothTRI7(face, pm, ndivs, bnode);
		break;
	case FE_FACE_TRI10:
		if (ndivs == 1) RenderTRI10(face, m_bsmooth, bnode);
		else RenderSmoothTRI10(face, pm, ndivs, bnode);
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------

void CGLModel::RenderFace(FEFace& face, FEMeshBase* pm, GLColor c[4], int ndivs, bool bnode)
{
	if (m_bShell2Hex)
	{
		int ntype = pm->Element(face.m_elem[0]).Type();
		if ((ntype == FE_QUAD4) || (ntype == FE_QUAD8) || (ntype == FE_QUAD9) || (ntype == FE_TRI3) || (ntype == FE_TRI6))
		{
			RenderThickShell(face, pm);
			return;
		}
	}

	vec3f& r1 = pm->Node(face.n[0]).m_rt;
	vec3f& r2 = pm->Node(face.n[1]).m_rt;
	vec3f& r3 = pm->Node(face.n[2]).m_rt;
	vec3f& r4 = pm->Node(face.n[3]).m_rt;

	vec3f& n1 = face.m_nn[0];
	vec3f& n2 = face.m_nn[1];
	vec3f& n3 = face.m_nn[2];
	vec3f& n4 = face.m_nn[3];

	vec3f& fn = face.m_fn;

	float t[4];
	pm->FaceNodeTexCoords(face, t, bnode);

	if (m_bsmooth)
	{
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
			else RenderSmoothQUAD4(face, pm, ndivs, bnode);
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
	else
	{
		glNormal3f(fn.x, fn.y, fn.z);

		switch (face.m_type)
		{
		case FE_FACE_QUAD4:
		case FE_FACE_QUAD8:
		case FE_FACE_QUAD9:
			glBegin(GL_QUADS);
			{
				glColor4ub(c[0].r, c[0].g, c[0].b, c[0].a); glTexCoord1f(t[0]); glVertex3f(r1.x, r1.y, r1.z);
				glColor4ub(c[1].r, c[1].g, c[1].b, c[1].a); glTexCoord1f(t[1]); glVertex3f(r2.x, r2.y, r2.z);
				glColor4ub(c[2].r, c[2].g, c[2].b, c[2].a); glTexCoord1f(t[2]); glVertex3f(r3.x, r3.y, r3.z);
				glColor4ub(c[3].r, c[3].g, c[3].b, c[3].a); glTexCoord1f(t[3]); glVertex3f(r4.x, r4.y, r4.z);
			}
			glEnd();
			break;
		case FE_FACE_TRI3:
		case FE_FACE_TRI6:
		case FE_FACE_TRI7:
			glBegin(GL_TRIANGLES);
			{
				glColor4ub(c[0].r, c[0].g, c[0].b, c[0].a); glTexCoord1f(t[0]); glVertex3f(r1.x, r1.y, r1.z);
				glColor4ub(c[1].r, c[1].g, c[1].b, c[1].a); glTexCoord1f(t[1]); glVertex3f(r2.x, r2.y, r2.z);
				glColor4ub(c[2].r, c[2].g, c[2].b, c[2].a); glTexCoord1f(t[2]); glVertex3f(r3.x, r3.y, r3.z);
			}
			glEnd();
			break;
		default:
			assert(false);
		}
	}
}


//-----------------------------------------------------------------------------

void CGLModel::RenderElementOutline(FEElement_& el, FEMeshBase* pm)
{
	glBegin(GL_LINES);
	{
		switch (el.Type())
		{
		case FE_HEX8:
			{
				int (*et)[2] = ET_HEX;
				for (int i=0; i<12; ++i)
				{
					vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
					vec3f& r1 = pm->Node(el.m_node[et[i][1]]).m_rt;

					glVertex3f(r0.x, r0.y, r0.z);
					glVertex3f(r1.x, r1.y, r1.z);
				}
			}
			break;
		case FE_PYRA5:
			{
				int(*et)[2] = ET_PYRA5;
				for (int i = 0; i<8; ++i)
				{
					vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
					vec3f& r1 = pm->Node(el.m_node[et[i][1]]).m_rt;

					glVertex3f(r0.x, r0.y, r0.z);
					glVertex3f(r1.x, r1.y, r1.z);
				}
			}
			break;
		case FE_HEX20:
			{
				int (*et)[3] = ET_HEX20;
				for (int i=0; i<12; ++i)
				{
					vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
					vec3f& r1 = pm->Node(el.m_node[et[i][1]]).m_rt;
					vec3f& r2 = pm->Node(el.m_node[et[i][2]]).m_rt;

					glVertex3f(r0.x, r0.y, r0.z); glVertex3f(r2.x, r2.y, r2.z);
					glVertex3f(r2.x, r2.y, r2.z); glVertex3f(r1.x, r1.y, r1.z);
				}
			}
			break;
		case FE_HEX27:
			{
				int (*et)[3] = ET_HEX20;
				for (int i=0; i<12; ++i)
				{
					vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
					vec3f& r1 = pm->Node(el.m_node[et[i][1]]).m_rt;
					vec3f& r2 = pm->Node(el.m_node[et[i][2]]).m_rt;

					glVertex3f(r0.x, r0.y, r0.z); glVertex3f(r2.x, r2.y, r2.z);
					glVertex3f(r2.x, r2.y, r2.z); glVertex3f(r1.x, r1.y, r1.z);
				}
			}
			break;
		case FE_PENTA6:
			{
				int (*et)[2] = ET_PENTA;
				for (int i=0; i<9; ++i)
				{
					vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
					vec3f& r1 = pm->Node(el.m_node[et[i][1]]).m_rt;

					glVertex3f(r0.x, r0.y, r0.z);
					glVertex3f(r1.x, r1.y, r1.z);
				}
			};
			break;
        case FE_PENTA15:
            {
                int (*et)[3] = ET_PENTA15;
                for (int i=0; i<9; ++i)
                {
                    vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
                    vec3f& r1 = pm->Node(el.m_node[et[i][1]]).m_rt;
                    vec3f& r2 = pm->Node(el.m_node[et[i][2]]).m_rt;
                    
                    glVertex3f(r0.x, r0.y, r0.z); glVertex3f(r2.x, r2.y, r2.z);
                    glVertex3f(r2.x, r2.y, r2.z); glVertex3f(r1.x, r1.y, r1.z);
                }
            };
            break;
        case FE_TET4:
		case FE_TET5:
		case FE_TET20:
			{
				int (*et)[2] = ET_TET;
				for (int i=0; i<6; ++i)
				{
					vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
					vec3f& r1 = pm->Node(el.m_node[et[i][1]]).m_rt;

					glVertex3f(r0.x, r0.y, r0.z);
					glVertex3f(r1.x, r1.y, r1.z);
				}
			}
			break;
		case FE_TET10:
		case FE_TET15:
			{
				int (*et)[3] = ET_TET10;
				for (int i=0; i<6; ++i)
				{
					vec3f& r0 = pm->Node(el.m_node[et[i][0]]).m_rt;
					vec3f& r1 = pm->Node(el.m_node[et[i][2]]).m_rt;
					vec3f& r2 = pm->Node(el.m_node[et[i][1]]).m_rt;

					glVertex3f(r0.x, r0.y, r0.z); glVertex3f(r1.x, r1.y, r1.z);
					glVertex3f(r1.x, r1.y, r1.z); glVertex3f(r2.x, r2.y, r2.z);
				}
			}
			break;
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------

void CGLModel::RenderFaceOutline(FEFace& face, FEMeshBase* pm, int ndivs)
{
	if (m_bShell2Hex)
	{
		int ntype = pm->Element(face.m_elem[0]).Type();
		if ((ntype == FE_QUAD4) || (ntype == FE_QUAD8) || (ntype == FE_QUAD9) || (ntype == FE_TRI3) || (ntype == FE_TRI6))
		{
			RenderThickShellOutline(face, pm);
			return;
		}
	}

	GLboolean btex;
	glGetBooleanv(GL_TEXTURE_1D, &btex);
	glDisable(GL_TEXTURE_1D);

	// render the edges of the fae
	switch (face.m_type)
	{
	case FE_FACE_TRI3 :
	case FE_FACE_QUAD4: RenderFace1Outline(face, pm); break;
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9: RenderFace2Outline(face, pm, ndivs); break;
	case FE_FACE_TRI10: RenderFace3Outline(face, pm, ndivs); break;
	default:
		assert(false);
	}

	if (btex) glEnable(GL_TEXTURE_1D);
}

///////////////////////////////////////////////////////////////////////////////
void CGLModel::RenderThickShell(FEFace &face, FEMeshBase* pm)
{
	switch(face.m_type)
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

void CGLModel::RenderThickQuad(FEFace &face, FEMeshBase* pm)
{
	FEElement_& el = pm->Element(face.m_elem[0]);
	FEState* ps = m_ps->GetState(0);
	float* h = ps->m_ELEM[face.m_elem[0]].m_h;

	vec3f r1 = pm->Node(face.n[0]).m_rt;
	vec3f r2 = pm->Node(face.n[1]).m_rt;
	vec3f r3 = pm->Node(face.n[2]).m_rt;
	vec3f r4 = pm->Node(face.n[3]).m_rt;

	vec3f n1 = face.m_nn[0];
	vec3f n2 = face.m_nn[1];
	vec3f n3 = face.m_nn[2];
	vec3f n4 = face.m_nn[3];

	vec3f r1a, r2a, r3a, r4a;
	vec3f r1b, r2b, r3b, r4b;
	
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

	vec3f m1 = (r2 - r1)^n1; m1.Normalize();
	vec3f m2 = (r3 - r2)^n1; m2.Normalize();
	vec3f m3 = (r4 - r3)^n1; m3.Normalize();
	vec3f m4 = (r1 - r4)^n1; m4.Normalize();

	vec3f fn = face.m_fn;

	float t1 = face.m_tex[0];
	float t2 = face.m_tex[1];
	float t3 = face.m_tex[2];
	float t4 = face.m_tex[3];

	if (m_bsmooth)
	{
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
	else
	{
		glNormal3f(fn.x, fn.y, fn.z);

		glBegin(GL_QUADS);
		{
			glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
			glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
			glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
			glTexCoord1f(t4); glVertex3f(r4b.x, r4b.y, r4b.z);

			glTexCoord1f(t4); glVertex3f(r4a.x, r4a.y, r4a.z);
			glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
			glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
			glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);

			if (face.m_nbr[0] == -1)
			{
				glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
				glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
				glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
				glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
			}

			if (face.m_nbr[1] == -1)
			{
				glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
				glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
				glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
				glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
			}

			if (face.m_nbr[2] == -1)
			{
				glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
				glTexCoord1f(t4); glVertex3f(r4a.x, r4a.y, r4a.z);
				glTexCoord1f(t4); glVertex3f(r4b.x, r4b.y, r4b.z);
				glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
			}

			if (face.m_nbr[3] == -1)
			{
				glTexCoord1f(t4); glVertex3f(r4a.x, r4a.y, r4a.z);
				glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
				glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
				glTexCoord1f(t4); glVertex3f(r4b.x, r4b.y, r4b.z);
			}
		}
		glEnd();
	}
}

void CGLModel::RenderThickTri(FEFace &face, FEMeshBase* pm)
{
	FEElement_& el = pm->Element(face.m_elem[0]);
	FEState* ps = m_ps->GetState(0);
	float* h = ps->m_ELEM[face.m_elem[0]].m_h;

	vec3f r1 = pm->Node(face.n[0]).m_rt;
	vec3f r2 = pm->Node(face.n[1]).m_rt;
	vec3f r3 = pm->Node(face.n[2]).m_rt;

	//h[0] = (h[0] + h[1] + h[2])/3;
	//h[1] = h[0];
	//h[2] = h[0];
	vec3f n1 = face.m_nn[0];
	vec3f n2 = face.m_nn[1];
	vec3f n3 = face.m_nn[2];

	vec3f r1a, r2a, r3a;
	vec3f r1b, r2b, r3b;

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

	vec3f m1 = (r2 - r1) ^ n1; m1.Normalize();
	vec3f m2 = (r3 - r2) ^ n1; m2.Normalize();
	vec3f m3 = (r1 - r3) ^ n1; m3.Normalize();

	vec3f fn = face.m_fn;

//	float t1 = face.m_tex[0];
//	float t2 = face.m_tex[1];
//	float t3 = face.m_tex[2];
	float t1 = pm->Node(face.n[0]).m_tex;
	float t2 = pm->Node(face.n[1]).m_tex;
	float t3 = pm->Node(face.n[2]).m_tex;

	if (m_bsmooth)
	{
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
	else
	{
		glNormal3f(fn.x, fn.y, fn.z);

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
				glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
				glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
				glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
				glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
			}

			if (face.m_nbr[1] == -1)
			{
				glTexCoord1f(t2); glVertex3f(r2a.x, r2a.y, r2a.z);
				glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
				glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
				glTexCoord1f(t2); glVertex3f(r2b.x, r2b.y, r2b.z);
			}

			if (face.m_nbr[2] == -1)
			{
				glTexCoord1f(t3); glVertex3f(r3a.x, r3a.y, r3a.z);
				glTexCoord1f(t1); glVertex3f(r1a.x, r1a.y, r1a.z);
				glTexCoord1f(t1); glVertex3f(r1b.x, r1b.y, r1b.z);
				glTexCoord1f(t3); glVertex3f(r3b.x, r3b.y, r3b.z);
			}
		}
		glEnd();
	}
}


///////////////////////////////////////////////////////////////////////////////

void CGLModel::RenderThickShellOutline(FEFace &face, FEMeshBase* pm)
{
	FEElement_& el = pm->Element(face.m_elem[0]);
	FEState* ps = m_ps->GetState(0);
	float* h = ps->m_ELEM[face.m_elem[0]].m_h;

	vec3f r1 = pm->Node(face.n[0]).m_rt;
	vec3f r2 = pm->Node(face.n[1]).m_rt;
	vec3f r3 = pm->Node(face.n[2]).m_rt;
	vec3f r4 = pm->Node(face.n[3]).m_rt;

	vec3f n1 = face.m_nn[0];
	vec3f n2 = face.m_nn[1];
	vec3f n3 = face.m_nn[2];
	vec3f n4 = face.m_nn[3];

	vec3f r1a, r2a, r3a, r4a;
	vec3f r1b, r2b, r3b, r4b;
	
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
// Render the edges of the model.
void CGLModel::RenderEdges(FEModel* ps, CGLContext& rc)
{
	// store attributes
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	vec3f r[3];

	FEMeshBase& mesh = *GetActiveMesh();
	int NE = mesh.Edges();

	// render unselected edges
	glColor3ub(0, 0, 255);
	glBegin(GL_LINES);
	{
		for (int i = 0; i<NE; ++i)
		{
			FEEdge& edge = mesh.Edge(i);
			if (edge.IsVisible() && (edge.IsSelected() == false))
			{
				switch (edge.Type())
				{
				case FE_EDGE2:
					r[0] = mesh.Node(edge.n[0]).m_rt;
					r[1] = mesh.Node(edge.n[1]).m_rt;
					glVertex3d(r[0].x, r[0].y, r[0].z);
					glVertex3d(r[1].x, r[1].y, r[1].z);
					break;
				case FE_EDGE3:
					r[0] = mesh.Node(edge.n[0]).m_rt;
					r[1] = mesh.Node(edge.n[1]).m_rt;
					r[2] = mesh.Node(edge.n[2]).m_rt;
					glVertex3d(r[0].x, r[0].y, r[0].z);
					glVertex3d(r[1].x, r[1].y, r[1].z);
					glVertex3d(r[1].x, r[1].y, r[1].z);
					glVertex3d(r[2].x, r[2].y, r[2].z);
					break;
				}
			}
		}
	}
	glEnd();

	// render selected edges
	if (GetSelectionMode() == SELECT_EDGES)
	{
		glDisable(GL_DEPTH_TEST);
		glColor3ub(255, 255, 0);
		glBegin(GL_LINES);
		{
			for (int i = 0; i<NE; ++i)
			{
				FEEdge& edge = mesh.Edge(i);
				if (edge.IsVisible() && edge.IsSelected())
				{
					switch (edge.Type())
					{
					case FE_EDGE2:
						r[0] = mesh.Node(edge.n[0]).m_rt;
						r[1] = mesh.Node(edge.n[1]).m_rt;
						glVertex3d(r[0].x, r[0].y, r[0].z);
						glVertex3d(r[1].x, r[1].y, r[1].z);
						break;
					case FE_EDGE3:
						r[0] = mesh.Node(edge.n[0]).m_rt;
						r[1] = mesh.Node(edge.n[1]).m_rt;
						r[2] = mesh.Node(edge.n[2]).m_rt;
						glVertex3d(r[0].x, r[0].y, r[0].z);
						glVertex3d(r[1].x, r[1].y, r[1].z);
						glVertex3d(r[1].x, r[1].y, r[1].z);
						glVertex3d(r[2].x, r[2].y, r[2].z);
						break;
					}
				}
			}
		}
		glEnd();
	}

	// restore attributes
	glPopAttrib();
}

void CGLModel::RenderDecorations()
{
	if (m_decor.empty() == false)
	{
		glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glColor3ub(255, 255, 0);

		std::list<GDecoration*>::iterator it;
		for (it=m_decor.begin(); it != m_decor.end(); ++it)
		{
			GDecoration* pd = *it;
			if (pd->isVisible()) pd->render();
		}
		glPopAttrib();
	}
}

void CGLModel::AddDecoration(GDecoration* pd)
{
	m_decor.push_back(pd);
}

void CGLModel::RemoveDecoration(GDecoration* pd)
{
	std::list<GDecoration*>::iterator it;
	for (it=m_decor.begin(); it != m_decor.end(); ++it)
	{
		if (*it==pd)
		{
			m_decor.erase(it);
			return;
		}
	}
}

int CGLModel::GetSubDivisions()
{
	if (m_nDivs < 1)
	{
		FEMeshBase& mesh = *GetActiveMesh();
		int NE = mesh.Elements();
		if (NE == 0) return 1;

		const int max_elem = 10000;
		int ndivs = max_elem / NE;
		if (ndivs > 10) ndivs = 10;
		if (ndivs <  1) ndivs = 1;

#ifdef _DEBUG
		if (ndivs > 2) ndivs = 2;
#endif

		return ndivs;
	}
	else return m_nDivs;
}

//-----------------------------------------------------------------------------
void CGLModel::ClearSelectionLists()
{
	m_nodeSelection.clear();
	m_edgeSelection.clear();
	m_faceSelection.clear();
	m_elemSelection.clear();
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateSelectionLists(int mode)
{
	FEMeshBase& m = *GetActiveMesh();
	if ((mode == -1) || (mode == SELECT_NODES))
	{
		m_nodeSelection.clear();
		for (int i=0; i<m.Nodes(); ++i) if (m.Node(i).IsSelected()) m_nodeSelection.push_back(&m.Node(i));
	}

	if ((mode == -1) || (mode == SELECT_EDGES))
	{
		m_edgeSelection.clear();
		for (int i=0; i<m.Edges(); ++i) if (m.Edge(i).IsSelected()) m_edgeSelection.push_back(&m.Edge(i));
	}
	
	if ((mode == -1) || (mode == SELECT_FACES))
	{
		m_faceSelection.clear();
		for (int i=0; i<m.Faces(); ++i) if (m.Face(i).IsSelected()) m_faceSelection.push_back(&m.Face(i));
	}

	if ((mode == -1) || (mode == SELECT_ELEMS))
	{
		m_elemSelection.clear();
		for (int i=0; i<m.Elements(); ++i) if (m.Element(i).IsSelected()) m_elemSelection.push_back(&m.Element(i));
		UpdateInternalSurfaces();
	}
}

//-----------------------------------------------------------------------------
void CGLModel::SelectNodes(vector<int>& items, bool bclear)
{
	FEMeshBase& m = *GetActiveMesh();
	int N = m.Nodes();
	if (bclear) for (int i=0; i<N; ++i) m.Node(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i)
	{
		int nid = items[i];
		if ((nid >= 0) && (nid < N)) m.Node(nid).Select();
	}
	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectEdges(vector<int>& items, bool bclear)
{
	FEMeshBase& m = *GetActiveMesh();
	int N = m.Edges();
	if (bclear) for (int i = 0; i<N; ++i) m.Edge(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i)
	{
		int eid = items[i];
		if ((eid >= 0) && (eid < N)) m.Edge(eid).Select();
	}
	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectFaces(vector<int>& items, bool bclear)
{
	FEMeshBase& m = *GetActiveMesh();
	int N = m.Faces();
	if (bclear) for (int i=0; i<N; ++i) m.Face(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i) 
	{
		int fid = items[i];
		if ((fid >= 0) && (fid < N)) m.Face(fid).Select();
	}
	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectElements(vector<int>& items, bool bclear)
{
	FEMeshBase& m = *GetActiveMesh();
	int N = m.Elements();
	if (bclear) for (int i=0; i<N; ++i) m.Element(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i)
	{
		int eid = items[i];
		if ((eid >= 0) && (eid < N)) m.Element(eid).Select();
	}
	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
//! unhide all items
void CGLModel::UnhideAll()
{
	FEMeshBase& mesh = *GetActiveMesh();
	for (int i = 0; i<mesh.Elements(); ++i) mesh.Element(i).Unhide();
	for (int i = 0; i<mesh.Faces(); ++i) mesh.Face(i).Unhide();
	for (int i = 0; i<mesh.Edges(); ++i) mesh.Edge(i).Unhide();
	for (int i = 0; i<mesh.Nodes(); ++i) mesh.Node(i).Unhide();
	UpdateInternalSurfaces();
}

//-----------------------------------------------------------------------------
// Clear all selection
void CGLModel::ClearSelection()
{
	FEMeshBase& mesh = *GetActiveMesh();
	for (int i=0; i<mesh.Elements(); i++) mesh.Element(i).Unselect();
	for (int i=0; i<mesh.Faces   (); i++) mesh.Face(i).Unselect();
	for (int i=0; i<mesh.Edges   (); i++) mesh.Edge(i).Unselect();
	for (int i=0; i<mesh.Nodes   (); i++) mesh.Node(i).Unselect();
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Hide elements with a particular material ID
void CGLModel::HideMaterial(int nmat)
{
	FEMeshBase& mesh = *GetActiveMesh();

	// Hide the elements with the material ID
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.m_MatID == nmat)
		{
			el.Show(false);
		}
	}

	// hide faces
	// Faces are hidden if the adjacent element is hidden
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (mesh.Element(f.m_elem[0]).IsInvisible()) f.Show(false);
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsInvisible() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Show(false);

	// hide edges
	// edges are hidden if both nodes are hidden
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		if ((mesh.Node(edge.n[0]).IsInvisible()) &&
			(mesh.Node(edge.n[1]).IsInvisible())) edge.Show(false);
	}

	// selected items are unselected when hidden so
	// we need to update the selection lists.
	// This also updates the internal surfaces
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Show elements with a certain material ID
void CGLModel::ShowMaterial(int nmat)
{
	FEMeshBase& mesh = *GetActiveMesh();

	// unhide the elements with mat ID nmat
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i) 
	{
		FEElement_& e = mesh.Element(i);
		if (e.m_MatID == nmat) mesh.Element(i).Show(true);
	}

	// show faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (mesh.Element(f.m_elem[0]).IsInvisible() == false) f.Show(true);
	}

	// show nodes
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsInvisible() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 1) mesh.Node(i).Show(true);

	// show edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		if ((mesh.Node(edge.n[0]).IsInvisible() == false) &&
			(mesh.Node(edge.n[1]).IsInvisible()) == false) edge.Show(true);
	}

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Enable elements with a certain mat ID
void CGLModel::UpdateMeshState()
{
	FEMeshBase& mesh = *GetActiveMesh();
	FEModel& fem = *GetFEModel();

	// update the elements
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		int nmat = el.m_MatID;
		if (fem.GetMaterial(nmat)->enabled()) el.Enable();
		else el.Disable();
	}

	// now we update the nodes
	for (int i=0; i<mesh.Nodes(); ++i) mesh.Node(i).Disable();
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsEnabled())
		{
			int n = el.Nodes();
			for (int j=0; j<n; ++j) mesh.Node(el.m_node[j]).Enable();
		}
	}

	// enable the faces
	for (int i=0; i<mesh.Faces(); ++i) 
	{
		FEFace& f = mesh.Face(i);
		f.Disable();
		if (mesh.Element(f.m_elem[0]).IsEnabled()) f.Enable();
	}
}

//-----------------------------------------------------------------------------
// Select elements that are connected through the surface
void CGLModel::SelectConnectedSurfaceElements(FEElement_ &el)
{
	if (!el.IsVisible()) return;

	FEMeshBase& mesh = *GetActiveMesh();
	// tag all faces
	for (int i=0; i<mesh.Faces(); ++i) mesh.Face(i).m_ntag = 0;

	// find the face that this element belongs to
	for (int i=0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		if (f.m_elem[0] == el.m_lid)
		{
			// propagate through all neighbors
			stack<FEFace*> S;
			S.push(&f);
			while (!S.empty())
			{
				FEFace* pf = S.top(); S.pop();
				pf->m_ntag = 1;
				FEElement_& e2 = mesh.Element(pf->m_elem[0]);
				if (e2.IsVisible())
				{
					e2.Select();
					for (int j=0; j<pf->Edges(); ++j)
					{
						FEFace* pf2 = (pf->m_nbr[j] >= 0? &mesh.Face(pf->m_nbr[j]) : 0);
						if (pf2 && (pf2->m_ntag == 0)) S.push(pf2);
					}
				}
			}
		}
	}
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Select elements that are connected through the volume
void CGLModel::SelectConnectedVolumeElements(FEElement_ &el)
{
	if (!el.IsVisible()) return;

	FEMeshBase& mesh = *GetActiveMesh();
	// tag all elements
	for (int i=0; i<mesh.Elements(); ++i) mesh.Element(i).m_ntag = 0;

	// propagate through all neighbors
	stack<FEElement_*> S;
	S.push(&el);
	while (!S.empty())
	{
		FEElement_* pe = S.top(); S.pop();
		pe->m_ntag = 1;
		pe->Select();
		for (int j=0; j<pe->Faces(); ++j)
		{
			FEElement_* pe2 = mesh.ElementPtr(pe->m_nbr[j]);
			if (pe2 && pe2->IsVisible() && (pe2->m_ntag == 0)) S.push(pe2);
		}
	}

	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
// Select faces that are connected
void CGLModel::SelectConnectedEdges(FEEdge& e)
{
	FEMeshBase& mesh = *GetActiveMesh();

	// clear tags on all edge
	int NE = mesh.Edges();
	for (int i=0; i<NE; ++i) mesh.Edge(i).m_ntag = 0;

	// build the node-edge table
	FENodeEdgeList NEL;
	NEL.Build(&mesh);

	if (NEL.Empty()) return;

	// find a face that has both nodes connects to
	stack<FEEdge*> Stack;
	Stack.push(&e);
	while (Stack.empty() == false)
	{
		FEEdge* pe = Stack.top(); Stack.pop();
		pe->m_ntag = 1;

		// get the edge vector
		vec3f n = mesh.Node(pe->n[1]).m_rt - mesh.Node(pe->n[0]).m_rt; n.Normalize();

		// find the neighbor edges whose vector is closely aligned to the edge vector n
		for (int i=0; i<2; ++i)
		{
			int m = pe->n[i];
			int ne = NEL.Valence(m);
			int* EL = NEL.EdgeList(m);
			for (int j=0; j<ne; ++j)
			{
				FEEdge& ej = mesh.Edge(EL[j]);

				if (ej.IsVisible() && (&ej != pe) && (ej.m_ntag == 0))
				{
					vec3f nj = mesh.Node(ej.n[1]).m_rt - mesh.Node(ej.n[0]).m_rt; nj.Normalize();
					if (n*nj > 0.866) Stack.push(&ej);
				}
			}
		}
	}

	// select all the tagged edges
	for (int i=0; i<NE; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		if (edge.IsVisible() && (edge.m_ntag == 1)) edge.Select();
	}

	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
// Select faces that are connected
void CGLModel::SelectConnectedFaces(FEFace &f, double angleTol)
{
	FEMeshBase& mesh = *GetActiveMesh();

	// clear tags on all faces
	for (int i=0; i<mesh.Faces(); ++i) mesh.Face(i).m_ntag = 0;

	double tol = cos(DEG2RAD*angleTol);

	// propagate through all neighbors
	stack<FEFace*> S;
	f.m_ntag = 1;
	S.push(&f);
	while (!S.empty())
	{
		FEFace* pf = S.top(); S.pop();
		FEElement_& el = mesh.Element(pf->m_elem[0]);
		if (el.IsVisible())
		{
			pf->Select();
			for (int j=0; j<pf->Edges(); ++j)
			{
				FEFace* pf2 = (pf->m_nbr[j] >= 0? &mesh.Face(pf->m_nbr[j]) : 0);
				if (pf2 && (pf2->m_ntag == 0) && (pf2->m_sid == pf->m_sid) && (pf2->m_mat == pf->m_mat) && (f.m_fn*pf2->m_fn > tol))
				{
					pf2->m_ntag = 1;
					S.push(pf2);
				}
			}
		}
	}

	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
// Select nodes that are connected on a surface
void CGLModel::SelectConnectedSurfaceNodes(int n)
{
	FEMeshBase& mesh = *GetActiveMesh();

	// clear tags on all faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i) mesh.Face(i).m_ntag = 0;

	// find a face that has this node connects to
	FEFace* pf = 0;
	for (int i=0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		if (f.IsVisible() && f.HasNode(n))
		{
			pf = &f;
			break;
		}
	}
	if (pf == 0) return;

	// propagate through all neighbors
	stack<FEFace*> S;
	pf->m_ntag = 1;
	S.push(pf);
	while (!S.empty())
	{
		FEFace* pf = S.top(); S.pop();
		if (pf->IsVisible())
		{
			for (int j=0; j<pf->Edges(); ++j)
			{
				FEFace* pf2 = (pf->m_nbr[j] >= 0? &mesh.Face(pf->m_nbr[j]) : 0);
				if (pf2 && (pf2->m_ntag == 0) && (pf2->m_sid == pf->m_sid)) 
				{
					pf2->m_ntag = 1;
					S.push(pf2);
				}
			}
		}
	}

	// select all the nodes of tagged faces
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (f.m_ntag == 1)
		{
			int nf = f.Nodes();
			for (int j=0; j<nf; ++j) mesh.Node(f.n[j]).Select();
		}
	}

	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
// Select nodes that are connected on a volume
void CGLModel::SelectConnectedVolumeNodes(int n)
{
	FEMeshBase& mesh = *GetActiveMesh();

	// clear tags on all elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i) mesh.Element(i).m_ntag = 0;

	// find a visible element that has this node connects to
	FEElement_* pe = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& e = mesh.Element(i);
		if (e.IsVisible() && e.HasNode(n))
		{
			pe = &e;
			break;
		}
	}
	if (pe == 0) return;

	// propagate through all neighbors
	stack<FEElement_*> S;
	pe->m_ntag = 1;
	S.push(pe);
	while (!S.empty())
	{
		FEElement_* pe = S.top(); S.pop();
		if (pe->IsVisible())
		{
			for (int j=0; j<pe->Faces(); ++j)
			{
				FEElement_* pe2 = mesh.ElementPtr(pe->m_nbr[j]);
				if (pe2 && (pe2->m_ntag == 0)) 
				{
					pe2->m_ntag = 1;
					S.push(pe2);
				}
			}
		}
	}

	// select all the nodes of tagged elements
	for (int i=0; i<NE; ++i)
	{
		FEElement_& e = mesh.Element(i);
		if (e.m_ntag == 1)
		{
			int nf = e.Nodes();
			for (int j=0; j<nf; ++j) mesh.Node(e.m_node[j]).Select();
		}
	}

	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
// Hide selected elements
void CGLModel::HideSelectedElements()
{
	FEMeshBase& mesh = *GetActiveMesh();

	// hide selected elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; i++)
	{
		FEElement_& e = mesh.Element(i);
		if (e.IsSelected()) e.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (mesh.Element(f.m_elem[0]).IsHidden()) f.Hide();
	}

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		FENode& node0 = mesh.Node(edge.n[0]);
		FENode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Hide selected elements
void CGLModel::HideUnselectedElements()
{
	FEMeshBase& mesh = *GetActiveMesh();

	// hide unselected elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; i++)
	{
		FEElement_& e = mesh.Element(i);
		if (!e.IsSelected()) e.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (mesh.Element(f.m_elem[0]).IsHidden()) f.Hide();
	}

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		FENode& node0 = mesh.Node(edge.n[0]);
		FENode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Hide selected faces
void CGLModel::HideSelectedFaces()
{
	FEMeshBase& mesh = *GetActiveMesh();
	// hide the faces and the elements that they are attached to
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i) 
	{
		FEFace& f = mesh.Face(i);
		if (f.IsSelected())
		{
			f.Hide();
			mesh.Element(f.m_elem[0]).Hide();
		}
	}

	// hide faces that were hidden by hiding the elements
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (mesh.Element(f.m_elem[0]).IsVisible() == false) f.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		FENode& node0 = mesh.Node(edge.n[0]);
		FENode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// hide selected edges
void CGLModel::HideSelectedEdges()
{
	FEMeshBase& mesh = *GetActiveMesh();

	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = -1;

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		if (edge.IsSelected()) 
		{
			edge.Hide();
			mesh.Node(edge.n[0]).m_ntag = 1;
			mesh.Node(edge.n[1]).m_ntag = 1;
		}
	}

	// hide surfaces
	FEEdge edge;
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& face = mesh.Face(i);
		int ne = face.Edges();
		for (int j=0; j<ne; ++j)
		{
			edge = face.GetEdge(j);
			if ((mesh.Node(edge.n[0]).m_ntag == 1)&&
				(mesh.Node(edge.n[1]).m_ntag == 1)) 
			{
				// hide the face
				face.Hide();

				// hide the adjacent element
				mesh.Element(face.m_elem[0]).Hide();
				break;
			}
		}
	}

	// hide faces that were hidden by hiding the elements
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (mesh.Element(f.m_elem[0]).IsVisible() == false) f.Hide();
	}

	// hide nodes that were hidden by hiding elements
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i)
		if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide edges
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		FENode& node0 = mesh.Node(edge.n[0]);
		FENode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// hide selected nodes
void CGLModel::HideSelectedNodes()
{
	FEMeshBase& mesh = *GetActiveMesh();

	// hide nodes and all elements they attach to
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i)
	{
		FENode& n = mesh.Node(i);
		n.m_ntag = 0;
		if (n.IsSelected()) { n.Hide(); n.m_ntag = 1; }
	}

	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = mesh.Element(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j) 
		{
			FENode& node = mesh.Node(el.m_node[j]);
			if (node.IsHidden() && (node.m_ntag == 1)) el.Hide();
		}
	}

	// hide nodes that were hidden by hiding elements
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = mesh.Element(i);
		if (el.IsHidden())
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).Hide();
		}
	}

	// hide faces that were hidden by hiding the elements
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (mesh.Element(f.m_elem[0]).IsVisible() == false) f.Hide();
	}

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		FENode& node0 = mesh.Node(edge.n[0]);
		FENode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllNodes()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Nodes(); i++)
	{
		FENode& n = mesh->Node(i);
		if (n.IsVisible()) n.Select();
	}

	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllEdges()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Edges(); i++)
	{
		FEEdge& e = mesh->Edge(i);
		if (e.IsVisible()) e.Select();
	}

	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllFaces()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Faces(); i++)
	{
		FEFace& f = mesh->Face(i);
		if (f.IsVisible()) f.Select();
	}

	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllElements()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Elements(); i++)
	{
		FEElement_& e = mesh->Element(i);
		if (e.IsVisible()) e.Select();
	}

	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedNodes()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Nodes(); i++)
		{
			FENode& n = mesh->Node(i);
			if (n.IsVisible())
				if (n.IsSelected()) n.Unselect(); else n.Select();
		}
	}
	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedEdges()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Edges(); i++)
		{
			FEEdge& e = mesh->Edge(i);
			if (e.IsVisible())
				if (e.IsSelected()) e.Unselect(); else e.Select();
		}
	}
	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedFaces()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Faces(); i++)
		{
			FEFace& f = mesh->Face(i);
			if (f.IsVisible())
				if (f.IsSelected()) f.Unselect(); else f.Select();
		}
	}
	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedElements()
{
	FEMeshBase* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Elements(); i++)
		{
			FEElement_& e = mesh->Element(i);
			if (e.IsVisible())
				if (e.IsSelected()) e.Unselect(); else e.Select();
		}
	}
	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateEdge()
{
	m_edge.Clear();
	FEMeshBase& mesh = *GetActiveMesh();
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.Element(i);
		if ((el.Type() == FE_BEAM2)||(el.Type() == FE_BEAM3))
		{
			GLEdge::EDGE edge;
			edge.n0 = el.m_node[0];
			edge.n1 = el.m_node[1];
			edge.mat = el.m_MatID;
			m_edge.AddEdge(edge);
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::ClearInternalSurfaces()
{
	for (int i=0; i<(int)m_innerSurface.size(); ++i) delete m_innerSurface[i];
	m_innerSurface.clear();
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateInternalSurfaces(bool eval)
{
	ClearInternalSurfaces();

	int nmat = m_ps->Materials();
	for (int i=0; i<nmat; ++i) m_innerSurface.push_back(new GLSurface);

	FEMeshBase& mesh = *GetActiveMesh();
	FEFace face;
	int ndom = mesh.Domains();
	for (int m = 0; m<ndom; ++m)
	{
		FEDomain& dom = mesh.Domain(m);
		int NE = dom.Elements();

		float f1 = m_ps->GetMaterial(m)->transparency;

		for (int i=0; i<NE; ++i)
		{
			FEElement_& el = dom.Element(i);
			if (el.IsVisible())
			{
				for (int j=0; j<el.Faces(); ++j)
				{
					FEElement_* pen = mesh.ElementPtr(el.m_nbr[j]);
					if (pen)
					{
						bool badd = (pen->IsSelected() != el.IsSelected()) || !pen->IsVisible();

						if ((badd == false) && (el.m_MatID != pen->m_MatID))
						{
							FEMaterial* pm2 = m_ps->GetMaterial(pen->m_MatID);
							float f2 = pm2->transparency;
							if ((f1 > f2) || (pm2->m_nrender == RENDER_MODE_WIRE)) badd = true;
						}

						if (badd)
						{
							el.GetFace(j, face);
							face.m_elem[0] = el.m_lid; // store the element ID. This is used for selection ???
							face.m_elem[1] = pen->m_lid;

							// calculate the face normals
							vec3f& r0 = mesh.Node(face.n[0]).m_r0;
							vec3f& r1 = mesh.Node(face.n[1]).m_r0;
							vec3f& r2 = mesh.Node(face.n[2]).m_r0;

							face.m_fn = (r1 - r0) ^ (r2 - r0);
							face.m_fn.Normalize();
							face.m_sid = 0;

							m_innerSurface[m]->add(face);
						}
					}
				}
			}
		}
	}

	// reevaluate model
	if (eval) Update(false);
}

//-----------------------------------------------------------------------------
void CGLModel::GetSelectionList(vector<int>& L, int mode)
{
	L.clear();
	FEMeshBase& m = *GetActiveMesh();
	switch (mode)
	{
	case SELECT_NODES:
	{
		for (int i = 0; i<m.Nodes(); ++i) if (m.Node(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_EDGES:
	{
		for (int i = 0; i<m.Edges(); ++i) if (m.Edge(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FACES:
	{
		for (int i = 0; i<m.Faces(); ++i) if (m.Face(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_ELEMS:
	{
		for (int i = 0; i<m.Elements(); ++i) if (m.Element(i).IsSelected()) L.push_back(i);
	}
	break;
	}
}

void CGLModel::ConvertSelection(int oldMode, int newMode)
{
	if (newMode == SELECT_NODES)
	{
		FEMeshBase& mesh = *GetFEModel()->GetFEMesh(0);

		if (oldMode == SELECT_EDGES)
		{
			int NE = mesh.Edges();
			for (int i = 0; i<NE; ++i)
			{
				FEEdge& e = mesh.Edge(i);
				if (e.IsSelected())
				{
					e.Unselect();
					int ne = e.Nodes();
					for (int j = 0; j<ne; ++j)
						mesh.Node(e.n[j]).Select();
				}
			}
		}
		if (oldMode == SELECT_FACES)
		{
			int NF = mesh.Faces();
			for (int i = 0; i<NF; ++i)
			{
				FEFace& f = mesh.Face(i);
				if (f.IsSelected())
				{
					f.Unselect();
					int nf = f.Nodes();
					for (int j = 0; j<nf; ++j)
						mesh.Node(f.n[j]).Select();
				}
			}
		}
		if (oldMode == SELECT_ELEMS)
		{
			int NE = mesh.Elements();
			for (int i = 0; i<NE; ++i)
			{
				FEElement_& e = mesh.Element(i);
				if (e.IsSelected())
				{
					e.Unselect();
					int ne = e.Nodes();
					for (int j = 0; j<ne; ++j)
						mesh.Node(e.m_node[j]).Select();
				}
			}
		}
	}

	UpdateSelectionLists();
}

void CGLModel::AddPlot(CGLPlot* pplot)
{
	m_pPlot.Add(pplot);
	pplot->Update(currentTime(), 0.f, true);
}

void CGLModel::ClearPlots()
{
	m_pPlot.Clear();
}

void CGLModel::UpdateColorMaps()
{
	int N = (int)m_pPlot.Size();
	for (int i = 0; i<N; ++i)
	{
		CGLPlot* p = m_pPlot[i];
		p->UpdateTexture();
	}
}
