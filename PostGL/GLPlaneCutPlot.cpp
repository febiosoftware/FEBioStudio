#include "stdafx.h"
#include "GLPlaneCutPlot.h"
#include <GLLib/GLContext.h>
#include "GLModel.h"
using namespace Post;

extern int LUT[256][15];
extern int LUT2D[16][4];
extern int ET_HEX[12][2];
extern int ET2D[4][2];

const int HEX_NT[8] = {0, 1, 2, 3, 4, 5, 6, 7};
const int PEN_NT[8] = {0, 1, 2, 2, 3, 4, 5, 5};
const int TET_NT[8] = {0, 1, 2, 2, 3, 3, 3, 3};
const int PYR_NT[8] = {0, 1, 2, 3, 4, 4, 4, 4};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

vector<int> CGLPlaneCutPlot::m_clip;
vector<CGLPlaneCutPlot*> CGLPlaneCutPlot::m_pcp;


CGLPlaneCutPlot::CGLPlaneCutPlot(CGLModel* po) : CGLPlot(po)
{
	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "Planecut.%02d", n++);
	SetName(szname);

	AddBoolParam(true, "Show plane");
	AddBoolParam(true, "Cut hidden");
	AddBoolParam(true, "Show Mesh" );
	AddDoubleParam(0, "Transparency")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0, "X-normal")->SetFloatRange(-1.0, 1.0);
	AddDoubleParam(0, "Y-normal")->SetFloatRange(-1.0, 1.0);
	AddDoubleParam(0, "Z-normal")->SetFloatRange(-1.0, 1.0);
	AddDoubleParam(0, "offset")->SetFloatRange(-1.0, 1.0, 0.01);

	m_eq[0] = 1;
	m_eq[1] = 0;
	m_eq[2] = 0;
	m_eq[3] = 0;

	m_rot = 0.f;
	m_transparency = 0.25;
	m_bcut_hidden = false;
	m_bshowplane = true;
	m_bshow_mesh = false;

	m_nclip = GetFreePlane();
	if (m_nclip >= 0) m_pcp[m_nclip] = this;

	UpdateData(false);
}

CGLPlaneCutPlot::~CGLPlaneCutPlot()
{
	ReleasePlane();
}

void CGLPlaneCutPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_bshowplane  = GetBoolValue(SHOW_PLANE);
		m_bcut_hidden = GetBoolValue(CUT_HIDDEN);
		m_bshow_mesh  = GetBoolValue(SHOW_MESH);
		m_transparency = GetFloatValue(TRANSPARENCY);
		m_eq[0] = GetFloatValue(NORMAL_X);
		m_eq[1] = GetFloatValue(NORMAL_Y);
		m_eq[2] = GetFloatValue(NORMAL_Z);
		m_eq[3] = GetFloatValue(OFFSET);

		UpdateSlice();
	}
	else
	{
		SetBoolValue(SHOW_PLANE, m_bshowplane);
		SetBoolValue(CUT_HIDDEN, m_bcut_hidden);
		SetBoolValue(SHOW_MESH, m_bshow_mesh);
		SetFloatValue(TRANSPARENCY, m_transparency);
		SetFloatValue(NORMAL_X, m_eq[0]);
		SetFloatValue(NORMAL_Y, m_eq[1]);
		SetFloatValue(NORMAL_Z, m_eq[2]);
		SetFloatValue(OFFSET  , m_eq[3]);
	}
}

void CGLPlaneCutPlot::DisableClipPlanes()
{
	for (int i=0; i<(int) m_clip.size(); ++i)
	{
		if (m_clip[i] != 0) glDisable(GL_CLIP_PLANE0 + i);
	}
}

void CGLPlaneCutPlot::EnableClipPlanes()
{
	for (int i=0; i<(int) m_clip.size(); ++i)
	{
		if (m_clip[i] != 0) glEnable(GL_CLIP_PLANE0 + i);
	}
}

void CGLPlaneCutPlot::InitClipPlanes()
{
	// allocate the clip array
	if (m_clip.size() == 0)
	{
		int N = 0;
		glGetIntegerv(GL_MAX_CLIP_PLANES, &N);
		assert(N);
		m_clip.assign(N, 0);
		m_pcp.assign(N, 0);
	}
}

void CGLPlaneCutPlot::Update(int ntime, float dt, bool breset)
{
	UpdateSlice();
}

///////////////////////////////////////////////////////////////////////////////

void CGLPlaneCutPlot::GetNormalizedEquations(double a[4])
{
	double L = sqrt(m_eq[0]*m_eq[0] + m_eq[1]*m_eq[1] + m_eq[2]*m_eq[2]);
	if (L == 0) L = 1;
	a[0] = m_eq[0]/L;
	a[1] = m_eq[1]/L;
	a[2] = m_eq[2]/L;
	a[3] = m_eq[3]/L;
}

//-----------------------------------------------------------------------------
// Return the plane normal
vec3d CGLPlaneCutPlot::GetPlaneNormal()
{
	double a[4];
	GetNormalizedEquations(a);
	return vec3d((float) a[0], (float) a[1], (float) a[2]);
}

float CGLPlaneCutPlot::GetPlaneOffset()
{
	return m_eq[3];
}

void CGLPlaneCutPlot::SetPlaneEqn(GLdouble a[4])
{
	m_eq[0] = a[0];
	m_eq[1] = a[1];
	m_eq[2] = a[2];
	m_eq[3] = a[3];
	UpdateSlice();
}

///////////////////////////////////////////////////////////////////////////////

void CGLPlaneCutPlot::Render(CGLContext& rc)
{
	// make sure we have a clip plane ID assigned
	if (m_nclip == -1) return;

	// get the plane equations
	GLdouble a[4];
	GetNormalizedEquations(a);

	// calculate the plane offset
	a[3] = -m_ref;

	// set the clip plane coefficients
	glClipPlane(GL_CLIP_PLANE0 + m_nclip, a);

	if (GetModel()->IsActive() == false) return;

	// make sure the current clip plane is not active
	glDisable(GL_CLIP_PLANE0 + m_nclip);

	// render the slice
	RenderSlice();

	// render the mesh
	if (m_bshow_mesh)
	{
		glDepthRange(0, 0.9999);
		RenderMesh();
		RenderOutline();
		glDepthRange(0, 1);
	}

	if (rc.m_showOutline)
	{
		RenderOutline();
	}

	// render the plane
	if (m_bshowplane)
	{
		glPushAttrib(GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);
		DisableClipPlanes();
		glDepthRange(0, 0.99999);
		RenderPlane();
		glDepthRange(0, 1);
		EnableClipPlanes();
		glPopAttrib();
	}

	// enable the clip plane
	glEnable(GL_CLIP_PLANE0 + m_nclip);
}

//-----------------------------------------------------------------------------
// Render the plane cut slice 
void CGLPlaneCutPlot::RenderSlice()
{
	CGLModel* mdl = GetModel();

	FEModel* ps = mdl->GetFEModel();
	FEMeshBase* pm = mdl->GetActiveMesh();

	CGLColorMap* pcol = mdl->GetColorMap();

	GLTexture1D& tex = pcol->GetColorMap()->GetTexture();
	glDisable(GL_CULL_FACE);

	// loop over all enabled materials
	for (int n=0; n<ps->Materials(); ++n)
	{
		FEMaterial* pmat = ps->GetMaterial(n);
		if ((pmat->bvisible || m_bcut_hidden) && pmat->bclip)
		{
			if (pcol->IsActive() && pmat->benable)
			{
				glEnable(GL_TEXTURE_1D);
				tex.MakeCurrent();
				GLubyte a = (GLubyte) (255.f*pmat->transparency);
				glColor4ub(255,255,255,a);
			}
			else
			{
				glDisable(GL_TEXTURE_1D);
				mdl->SetMaterialParams(pmat);
			}

			// repeat over all active faces
			int NF = m_slice.Faces();
			for (int i=0; i<NF; ++i)
			{
				GLSlice::FACE& face = m_slice.Face(i);
				if ((face.mat == n) && (face.bactive))
				{
					vec3d& norm = face.norm;
					glNormal3f(norm.x,norm.y,norm.z);

					// render the face
					vec3d* r = face.r;
					float* tex = face.tex;
					glBegin(GL_TRIANGLES);
					{
						glTexCoord1f(tex[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
						glTexCoord1f(tex[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
						glTexCoord1f(tex[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
					}
					glEnd();
				}
			}

			// render inactive faces
			glDisable(GL_TEXTURE_1D);
			for (int i = 0; i<NF; ++i)
			{
				GLSlice::FACE& face = m_slice.Face(i);
				if ((face.mat == n) && (!face.bactive))
				{
					vec3d& norm = face.norm;
					glNormal3f(norm.x, norm.y, norm.z);

					// render the face
					vec3d* r = face.r;
					float* tex = face.tex;
					glBegin(GL_TRIANGLES);
					{
						glVertex3f(r[0].x, r[0].y, r[0].z);
						glVertex3f(r[1].x, r[1].y, r[1].z);
						glVertex3f(r[2].x, r[2].y, r[2].z);
					}
					glEnd();
				}
			}
		}
	}

	glDisable(GL_TEXTURE_1D);
}

//-----------------------------------------------------------------------------
// Render the mesh of the plane cut
void CGLPlaneCutPlot::RenderMesh()
{
	int i, k, l, m;
	int ncase;
	int *pf;

	CGLModel* mdl = GetModel();

	FEModel* ps = mdl->GetFEModel();
	FEMeshBase* pm = mdl->GetActiveMesh();

	glColor3ub(0,0,0);

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	// set attributes
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);

	EDGE edge[15];
	int en[8];
	int ne;

	const int* nt;

	float ev[8];
	vec3d ex[8];

	vec3d r[3];
	float w1, w2, w;
	int n1, n2, m1, m2;
	bool badd;

	// calculate plane normal
	vec3d norm = GetPlaneNormal();

	// repeat over all elements
	for (i=0; i<pm->Elements(); ++i)
	{
		// render only when visible
		FEElement_& el = pm->ElementRef(i);
		FEMaterial* pmat = ps->GetMaterial(el.m_MatID);
		if ((el.m_ntag > 0) && (pmat->bmesh) && (pmat->bvisible || m_bcut_hidden) && (pmat->bclip))
		{
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
			}

			// calculate the case of the element
			ncase = el.m_ntag;

			// get the nodal values
			for (k=0; k<8; ++k)
			{
				FENode& node = pm->Node(el.m_node[nt[k]]);
				en[k] = el.m_node[k];
				ev[k] = node.m_tex;
				ex[k] = node.r;
			}

			// loop over faces
			pf = LUT[ncase];
			ne = 0;
			for (l=0; l<5; l++)
			{
				if (*pf == -1) break;

				// calculate nodal positions
				for (k=0; k<3; k++)
				{
					n1 = ET_HEX[pf[k]][0];
					n2 = ET_HEX[pf[k]][1];

					w1 = norm*ex[n1];
					w2 = norm*ex[n2];
	
					if (w2 != w1)
						w = (m_ref - w1)/(w2 - w1);
					else 
						w = 0.f;

					r[k] = ex[n1]*(1-w) + ex[n2]*w;
				}

				// add all edges to the list
				for (k=0; k<3; ++k)
				{
					n1 = pf[k];
					n2 = pf[(k+1)%3];

					badd = true;
					for (m=0; m<ne; ++m)
					{
						m1 = edge[m].m_n[0];
						m2 = edge[m].m_n[1];
						if (((n1 == m1) && (n2 == m2)) ||
							((n1 == m2) && (n2 == m1)))
						{
							badd = false;
							edge[m].m_ntag++;
							break;
						}
					}

					if (badd)
					{
						edge[ne].m_n[0] = n1;
						edge[ne].m_n[1] = n2;
						edge[ne].m_r[0] = r[k];
						edge[ne].m_r[1] = r[(k+1)%3];
						edge[ne].m_ntag = 0;
						++ne;
					}
				}
	
				pf+=3;
			}

			// render the lines
			glBegin(GL_LINES);
			{
				for (k=0; k<ne; ++k)
					if (edge[k].m_ntag == 0)
					{
						vec3d& r0 = edge[k].m_r[0];
						vec3d& r1 = edge[k].m_r[1];
						glVertex3f(r0.x, r0.y, r0.z);
						glVertex3f(r1.x, r1.y, r1.z);
					}
			}
			glEnd();
		}
	}

	// restore attributes
	glPopAttrib();
}


//-----------------------------------------------------------------------------
// Render the outline of the mesh of the plane cut
// TODO: This algorithm fails for thin structures that are one element wide.
//		 In that case, all nodes are exterior and thus all the edges will be drawn.
void CGLPlaneCutPlot::RenderOutline()
{
	CGLModel* mdl = GetModel();

	FEModel* ps = mdl->GetFEModel();
	FEMeshBase* pm = mdl->GetActiveMesh();

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	// set attributes
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);

	// because plots are drawn before the mesh
	// we get visual artifacts from the background seeping through.
	// therefor we turn blending of
	glDisable(GL_BLEND);

	glColor3ub(0,0,0);

	// calculate plane normal
	vec3d norm = GetPlaneNormal();

	// repeat over all elements
	for (int i=0; i<m_slice.Edges(); ++i)
	{
		// render only when visible
		GLSlice::EDGE& edge = m_slice.Edge(i);

		// loop over faces
		glBegin(GL_LINES);
		{
			vec3d& r0 = edge.r[0];
			vec3d& r1 = edge.r[1];
			glVertex3f(r0.x, r0.y, r0.z);
			glVertex3f(r1.x, r1.y, r1.z);
		}
		glEnd();
	}

	// restore attributes
	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLPlaneCutPlot::UpdateSlice()
{
	float ev[8];
	vec3d ex[8];
	int	nf[8];
	EDGE edge[15];
	int en[8];
	int	rf[3];

	// get the plane equations
	GLdouble a[4];
	GetNormalizedEquations(a);

	// set the plane normal
	vec3d norm((float) a[0], (float) a[1], (float) a[2]);

	CGLModel* mdl = GetModel();

	// calculate the plane offset
	m_box = mdl->GetFEModel()->GetBoundingBox();
	float s = m_box.Radius();
	vec3d r = m_box.Center();

	m_ref = (float) a[3]*s + r*norm;
	a[3] = -m_ref;

	FEModel* ps = mdl->GetFEModel();
	FEMeshBase* pm = mdl->GetActiveMesh();

	CGLColorMap& colorMap = *mdl->GetColorMap();
	bool bnode = colorMap.DisplayNodalValues();

	m_slice.Clear();

	// loop over all domains
	for (int n=0; n<pm->Domains(); ++n)
	{
		FEDomain& dom = pm->Domain(n);
		int matId = dom.GetMatID();
		if ((matId >= 0) && (matId < ps->Materials()))
		{
			FEMaterial* pmat = ps->GetMaterial(matId);
			if ((pmat->bvisible || m_bcut_hidden) && pmat->bclip)
			{
				// repeat over all elements
				for (int i=0; i<dom.Elements(); ++i)
				{
					// render only when visible
					FEElement_& el = dom.Element(i);
					if (el.IsVisible() || m_bcut_hidden)
					{
						if (el.IsSolid())
						{
							const int *nt;
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
							}
	
							// get the nodal values
							for (int k=0; k<8; ++k)
							{
								FENode& node = pm->Node(el.m_node[nt[k]]);
								nf[k] = (node.m_bext?1:0);
								ex[k] = to_vec3f(node.r);
								en[k] = el.m_node[k];
								ev[k] = (bnode ? node.m_tex : el.m_tex);
							}

							// calculate the case of the element
							int ncase = 0;
							for (int k=0; k<8; ++k) 
							if (norm*ex[k] >= m_ref) ncase |= (1 << k);

							// store the case for this element
							// so we don't have to calculate it again when
							// we draw the mesh
							el.m_ntag = 0;
							if ((ncase > 0) && (ncase < 255)) el.m_ntag = ncase;

							// loop over faces
							int* pf = LUT[ncase];
							int ne = 0;
							for (int l=0; l<5; l++)
							{
								if (*pf == -1) break;

								// calculate nodal positions
								vec3d r[3];
								float tex[3], w1, w2, w;
								for (int k=0; k<3; k++)
								{
									int n1 = ET_HEX[pf[k]][0];
									int n2 = ET_HEX[pf[k]][1];

									w1 = norm*ex[n1];
									w2 = norm*ex[n2];
			
									if (w2 != w1)
										w = (m_ref - w1)/(w2 - w1);
									else 
										w = 0.f;

									r[k] = ex[n1]*(1-w) + ex[n2]*w;
									tex[k] = ev[n1]*(1-w) + ev[n2]*w;
									rf[k] = ((nf[n1]==1)&&(nf[n2]==1)?1:0);
								}

								GLSlice::FACE face;
								face.mat = n;
								face.norm = norm;
								face.r[0] = r[0];
								face.r[1] = r[1];
								face.r[2] = r[2];
								face.tex[0] = tex[0];
								face.tex[1] = tex[1];
								face.tex[2] = tex[2];
								face.bactive = el.IsActive();

								m_slice.AddFace(face);

								// add all edges to the list
								for (int k=0; k<3; ++k)
								{
									int n1 = pf[k];
									int n2 = pf[(k+1)%3];

									bool badd = true;
									// make sure this edge is on the surface
									if ((rf[k] != 1) || (rf[(k+1)%3] != 1)) badd = false;
									else
									{
										// make sure we don't have this edge yet
										for (int m=0; m<ne; ++m)
										{
											int m1 = edge[m].m_n[0];
											int m2 = edge[m].m_n[1];
											if (((n1 == m1) && (n2 == m2)) ||
												((n1 == m2) && (n2 == m1)))
											{
												badd = false;
												edge[m].m_ntag++;
												break;
											}
										}
									}

									if (badd)
									{
										edge[ne].m_n[0] = n1;
										edge[ne].m_n[1] = n2;
										edge[ne].m_r[0] = r[k];
										edge[ne].m_r[1] = r[(k+1)%3];
										edge[ne].m_ntag = 0;
										++ne;
									}
								}

								pf+=3;
							}

							// add the lines
							GLSlice::EDGE e;
							for (int k=0; k<ne; ++k)
								if (edge[k].m_ntag == 0)
								{
									e.r[0] = edge[k].m_r[0];
									e.r[1] = edge[k].m_r[1];
									m_slice.AddEdge(e);
								}
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Calculate the integral over the plane cut
float CGLPlaneCutPlot::Integrate(FEState* ps)
{
	int k, l;

	CGLModel* mdl = GetModel();

	FEModel* pfem = mdl->GetFEModel();
	FEMeshBase* pm = mdl->GetActiveMesh();

	float ev[8];
	vec3d ex[8];
	int   en[8];

	vec3d r[4];
	float v[4];

	// Integral
	float sum = 0.f;

	// calculate plane normal
	vec3d norm = GetPlaneNormal();

	// repeat over all elements
	for (int i=0; i<pm->Elements(); ++i)
	{
		// consider only solid elements that are visible
		FEElement_& el = pm->ElementRef(i);
		FEMaterial* pmat = pfem->GetMaterial(el.m_MatID);
		if (el.IsSolid() && el.IsVisible() && pmat->bvisible)
		{
			// we consider all elements degenerate hexes
			// so get the equivalent hex' node numbering
			const int* nt;
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
			}

			// get the nodal values
			for (k=0; k<8; ++k)
			{
				FENode& node = pm->Node(el.m_node[nt[k]]);
				en[k] = el.m_node[k];
				ev[k] = ps->m_NODE[en[k]].m_val;
				ex[k] = ps->m_NODE[en[k]].m_rt;
			}

			// calculate the case of the element
			int ncase = 0;
			for (k=0; k<8; ++k) 
			if (norm*ex[k] >= m_ref) ncase |= (1 << k);

			// loop over faces
			int* pf = LUT[ncase];
			for (l=0; l<5; l++)
			{
				if (*pf == -1) break;

				// calculate nodal positions
				for (k=0; k<3; k++)
				{
					int n1 = ET_HEX[pf[k]][0];
					int n2 = ET_HEX[pf[k]][1];

					float w1 = norm*ex[n1];
					float w2 = norm*ex[n2];

					float w = 0.f; 
					if (w2 != w1) w = (m_ref - w1)/(w2 - w1);

					r[k] = ex[n1]*(1-w) + ex[n2]*w;
					v[k] = ev[n1]*(1-w) + ev[n2]*w;
				}

				// the integration requires a quad
				r[3] = r[2];
				v[3] = v[2];

				// integrate
				sum += IntegrateQuad(r, v);

				// next face
				pf+=3;
			}
		}
	}

	return sum;
}

//-----------------------------------------------------------------------------
// Render the cutting plane
void CGLPlaneCutPlot::RenderPlane()
{
	GLdouble a[4];
	GetNormalizedEquations(a);
	vec3d norm((float) a[0], (float) a[1], (float) a[2]);

	CGLModel* mdl = GetModel();

	m_box = mdl->GetFEModel()->GetBoundingBox();
	vec3d rc = m_box.Center();

	// calculate reference value
	vec3d p0 = norm*(-m_ref);

	float N = norm*norm;
	if (N==0) N=1;

	float lam = (N*m_ref - rc*norm)/N;
	vec3d pc = rc + norm*lam;

	glPushMatrix();

	glTranslatef(pc.x, pc.y, pc.z);

	quatd q = quatd(vec3d(0,0,1), norm);
	float w = q.GetAngle();
	if (w != 0)
	{
		vec3d v = q.GetVector();
		glRotatef(w*180/PI, v.x, v.y, v.z);
	}

	glRotatef(m_rot, 0, 0, 1);

	float R = m_box.Radius();

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	GLdouble r = fabs(norm.x);
	GLdouble g = fabs(norm.y);
	GLdouble b = fabs(norm.z);

	glColor4d(r, g, b, m_transparency);
	glDepthMask(false);
	glNormal3f(0,0,1);
	glBegin(GL_QUADS);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f( R, -R, 0);
		glVertex3f( R,  R, 0);
		glVertex3f(-R,  R, 0);
	}
	glEnd();
	glDepthMask(true);

	glColor3ub(255, 255, 0);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f( R, -R, 0);
		glVertex3f( R,  R, 0);
		glVertex3f(-R,  R, 0);
	}
	glEnd();

	glPopMatrix();

	// restore attributes
	glPopAttrib();
}

void CGLPlaneCutPlot::Activate(bool bact)
{
	if (bact != IsActive())
	{
		CGLObject::Activate(bact);
		if (bact)
		{
			m_nclip = GetFreePlane();
			if (m_nclip >= 0) m_pcp[m_nclip] = this;
		}
		else
		{
			ReleasePlane();
		}
	}
}

int CGLPlaneCutPlot::GetFreePlane()
{
	// NOTE: This assumes that InitClipPlanes() has already been called
	int n = -1;
	if (m_clip.size() > 0)
	{
		// find an available clipping plane
		for (int i=0; i<(int) m_clip.size(); ++i)
		{
			if (m_clip[i] == 0)
			{
				n = i;
				m_clip[i] = 1;
				break;
			}
		}
	}

	assert(n >= 0);

	return n;
}

void CGLPlaneCutPlot::ReleasePlane()
{
	if ((m_clip.size() > 0) && (m_nclip != -1)) 
	{
		m_clip[m_nclip] = 0;
		m_pcp[m_nclip] = 0;
	}
	m_nclip = -1;
}

int CGLPlaneCutPlot::ClipPlanes()
{
	return (int)m_pcp.size();
}

CGLPlaneCutPlot* CGLPlaneCutPlot::GetClipPlane(int i)
{
	return m_pcp[i];
}

bool CGLPlaneCutPlot::IsInsideClipRegion(const vec3d& r)
{
	int N = CGLPlaneCutPlot::ClipPlanes();
	for (int i = 0; i<N; ++i)
	{
		CGLPlaneCutPlot* pcp = CGLPlaneCutPlot::GetClipPlane(i);
		if (pcp && pcp->IsActive())
		{
			double a[4];
			pcp->GetNormalizedEquations(a);
			a[3] = pcp->GetPlaneReference();

			double q = a[0] * r.x + a[1] * r.y + a[2] * r.z - a[3];
			if (q < 0) return false;
		}
	}
	return true;
}
