#include "stdafx.h"
#include "GLVectorPlot.h"
#include "PostLib/ColorMap.h"
#include "PostLib/constants.h"
#include "GLWLib/GLWidgetManager.h"
#include <PostGL/GLModel.h>
using namespace Post;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGLVectorPlot::CGLVectorPlot(CGLModel* po) : CGLPlot(po)
{
	static int n = 1;
	char szname[128] = {0};
	sprintf(szname, "VectorPlot.%02d", n++);
	SetName(szname);

	AddIntParam(0, "Data field")->SetEnumNames("@data_vec3");
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddBoolParam(true, "Allow clipping");
	AddBoolParam(true, "Show hidden"   );
	AddDoubleParam(0., "Density")->SetFloatRange(0.0, 1.0, 0.0001);
	AddIntParam(0, "Glyph"         )->SetEnumNames("Arrow\0Cone\0Cylinder\0Sphere\0Box\0Line\0");
	AddIntParam(0, "Glyph Color"   )->SetEnumNames("Solid\0Length\0Orientation\0");
	AddColorParam(GLColor::White(), "Solid Color");
	AddBoolParam(true, "Normalize" );
	AddBoolParam(true, "Auto-scale");
	AddDoubleParam(0., "Scale"     );
	AddIntParam(0, "Range type")->SetEnumNames("Dynamic\0Static\0User\0");
	AddDoubleParam(1., "User Max"  );
	AddDoubleParam(0., "User Min"  );

	m_scale = 1;
	m_dens = 1;

	m_ntime = -1;
	m_nvec = -1;

	m_lastTime = -1;
	m_lastDt = 0.f;

	m_nglyph = GLYPH_ARROW;

	m_ncol = GLYPH_COL_SOLID;

	m_gcl.r = 255;
	m_gcl.g = 255;
	m_gcl.b = 255;
	m_gcl.a = 255;

	m_bnorm = false;
	m_bautoscale = false;
	m_bshowHidden = true;

	m_seed = rand();

	m_rngType = 0;
	m_usr[0] = 0.0;
	m_usr[1] = 1.0;

	m_pbar = new GLLegendBar(&m_Col, 0, 0, 120, 500);
	m_pbar->align(GLW_ALIGN_BOTTOM | GLW_ALIGN_HCENTER);
	m_pbar->SetOrientation(GLLegendBar::HORIZONTAL);
	m_pbar->copy_label(szname);
	CGLWidgetManager::GetInstance()->AddWidget(m_pbar);

	m_pbar->hide();
	m_pbar->ShowTitle(true);

	UpdateData(false);
}

CGLVectorPlot::~CGLVectorPlot()
{
	CGLWidgetManager::GetInstance()->RemoveWidget(m_pbar);
	delete m_pbar;
}

void CGLVectorPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_nvec = GetIntValue(DATA_FIELD);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		AllowClipping(GetBoolValue(CLIP));
		m_bshowHidden = GetBoolValue(SHOW_HIDDEN);
		m_dens = GetFloatValue(DENSITY);
		m_nglyph = GetIntValue(GLYPH);
		m_ncol = GetIntValue(GLYPH_COLOR);
		m_gcl = GetColorValue(SOLID_COLOR);
		m_bnorm = GetBoolValue(NORMALIZE);
		m_bautoscale = GetBoolValue(AUTO_SCALE);
		m_scale = GetFloatValue(SCALE);
		m_rngType = GetIntValue(RANGE_TYPE);
		m_usr[1] = GetFloatValue(USER_MAX);
		m_usr[0] = GetFloatValue(USER_MIN);

		if ((m_ncol == 0) || !IsActive()) m_pbar->hide();
		else
		{
			m_pbar->SetRange(m_crng.x, m_crng.y);
			m_pbar->show();
		}
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nvec);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(CLIP, AllowClipping());
		SetBoolValue(SHOW_HIDDEN, m_bshowHidden);
		SetFloatValue(DENSITY, m_dens);
		SetIntValue(GLYPH, m_nglyph);
		SetIntValue(GLYPH_COLOR, m_ncol);
		SetColorValue(SOLID_COLOR, m_gcl);
		SetBoolValue(NORMALIZE, m_bnorm);
		SetBoolValue(AUTO_SCALE, m_bautoscale);
		SetFloatValue(SCALE, m_scale);
		SetIntValue(RANGE_TYPE, m_rngType);
		SetFloatValue(USER_MAX, m_usr[1]);
		SetFloatValue(USER_MIN, m_usr[0]);
	}
}

static double frand() { return (double) rand() / (double) RAND_MAX; }

void CGLVectorPlot::Render(CGLContext& rc)
{
	if (m_nvec == -1) return;

	GLfloat ambient[] = {0.1f,0.1f,0.1f,1.f};
	GLfloat specular[] = {0.0f,0.0f,0.0f,1};
	GLfloat emission[] = {0,0,0,1};

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
//	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);

	// store attributes
	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);

	// create the cylinder object
	glEnable(GL_LIGHTING);
	GLUquadricObj* pglyph = gluNewQuadric();
	gluQuadricNormals(pglyph, GLU_SMOOTH);

	CGLModel* mdl = GetModel();
	FEModel* ps = mdl->GetFEModel();

	srand(m_seed);

	FEModel* pfem = mdl->GetFEModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	// calculate scale factor for rendering
	m_fscale = 0.02f*m_scale*pfem->GetBoundingBox().Radius();

	// calculate auto-scale factor
	if (m_bautoscale)
	{
		float autoscale = 1.f;
		float Lmax = 0.f;
		for (int i = 0; i<(int)m_val.size(); ++i)
		{
			float L = m_val[i].Length();
			if (L > Lmax) Lmax = L;
		}
		if (Lmax == 0.f) Lmax = 1.f;
		autoscale = 1.f / Lmax;

		m_fscale *= autoscale;
	}

	if (m_nglyph == GLYPH_LINE) glDisable(GL_LIGHTING);
	else
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		GLfloat dif[] = {1.f, 1.f, 1.f, 1.f};

		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		glLightfv(GL_LIGHT0, GL_AMBIENT, dif);
	}

	if (IS_ELEM_FIELD(m_nvec))
	{
		pm->TagAllElements(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement_& e = pm->ElementRef(i);
			FEMaterial* mat = ps->GetMaterial(e.m_MatID);
			if (mat->benable && (m_bshowHidden || mat->visible()))
			{
				e.m_ntag = 1;
			}
		}

		if (m_bshowHidden == false)
		{
			// make sure no vector is drawn for hidden elements
			for (int i = 0; i < pm->Elements(); ++i)
			{
				FEElement_& elem = pm->ElementRef(i);
				if (elem.IsVisible() == false) elem.m_ntag = 0;
			}
		}

		// render the vectors at the elements' centers
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement_& elem = pm->ElementRef(i);
			if ((frand() <= m_dens) && elem.m_ntag)
			{
				vec3f r = to_vec3f(pm->ElementCenter(elem));
				vec3f v = m_val[i];
				RenderVector(r, v, pglyph);
			}
		}
	}
	else
	{
		pm->TagAllNodes(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement_& e = pm->ElementRef(i);
			FEMaterial* mat = ps->GetMaterial(e.m_MatID);
			if (mat->benable && (m_bshowHidden || mat->visible()))
			{
				int n = e.Nodes();
				for (int j = 0; j < n; ++j) pm->Node(e.m_node[j]).m_ntag = 1;
			}
		}

		if (m_bshowHidden == false)
		{
			// make sure no vector is drawn for hidden nodes
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				FENode& node = pm->Node(i);
				if (node.IsVisible() == false) node.m_ntag = 0;
			}
		}

		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			if ((frand() <= m_dens) && node.m_ntag)
			{
				vec3f r = to_vec3f(node.r);

				vec3f v = m_val[i];

				RenderVector(r, v, pglyph);
			}
		}
	}

	gluDeleteQuadric(pglyph);

	// restore attributes
	glPopAttrib();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void CGLVectorPlot::RenderVector(const vec3f& r, vec3f v, GLUquadric* pglyph)
{
	float L = v.Length();
	if (L == 0.f) return;

	CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

	float fmin = m_crng.x;
	float fmax = m_crng.y;

	float f = (L - fmin) / (fmax - fmin);
	GLColor col = map.map(f);
	v.Normalize();

	switch (m_ncol)
	{
	case GLYPH_COL_LENGTH:
		glColor3ub(col.r, col.g, col.b);
		break;
	case GLYPH_COL_ORIENT:
	{
		GLdouble r = fabs(v.x);
		GLdouble g = fabs(v.y);
		GLdouble b = fabs(v.z);
		glColor3d(r, g, b);
	}
	break;
	case GLYPH_COL_SOLID:
	default:
		glColor3ub(m_gcl.r, m_gcl.g, m_gcl.b);
	}

	if (m_bnorm) L = 1;

	L *= m_fscale;
	float l0 = L*.9;
	float l1 = L*.2;
	float r0 = L*0.05;
	float r1 = L*0.15;

	glPushMatrix();

	glTranslatef(r.x, r.y, r.z);
	quatd q(vec3d(0,0,1), v);
	float w = q.GetAngle();
	if (fabs(w) > 1e-6)
	{
		vec3d p = q.GetVector();
		if (p.Length() > 1e-6) glRotated(w * 180 / PI, p.x, p.y, p.z);
		else glRotated(w * 180 / PI, 1, 0, 0);
	}

	switch (m_nglyph)
	{
	case GLYPH_ARROW:
		gluCylinder(pglyph, r0, r0, l0, 5, 1);
		glTranslatef(0.f, 0.f, (float)l0*0.9f);
		gluCylinder(pglyph, r1, 0, l1, 10, 1);
		break;
	case GLYPH_CONE:
		gluCylinder(pglyph, r1, 0, l0, 10, 1);
		break;
	case GLYPH_CYLINDER:
		gluCylinder(pglyph, r1, r1, l0, 10, 1);
		break;
	case GLYPH_SPHERE:
		gluSphere(pglyph, r1, 10, 5);
		break;
	case GLYPH_BOX:
		glBegin(GL_QUADS);
		{
			glNormal3d(1, 0, 0);
			glVertex3d(r0, -r0, -r0);
			glVertex3d(r0, r0, -r0);
			glVertex3d(r0, r0, r0);
			glVertex3d(r0, -r0, r0);

			glNormal3d(-1, 0, 0);
			glVertex3d(-r0, r0, -r0);
			glVertex3d(-r0, -r0, -r0);
			glVertex3d(-r0, -r0, r0);
			glVertex3d(-r0, r0, r0);

			glNormal3d(0, 1, 0);
			glVertex3d(r0, r0, -r0);
			glVertex3d(-r0, r0, -r0);
			glVertex3d(-r0, r0, r0);
			glVertex3d(r0, r0, r0);

			glNormal3d(0, -1, 0);
			glVertex3d(-r0, -r0, -r0);
			glVertex3d(r0, -r0, -r0);
			glVertex3d(r0, -r0, r0);
			glVertex3d(-r0, -r0, r0);

			glNormal3d(0, 0, 1);
			glVertex3d(-r0, r0, r0);
			glVertex3d(r0, r0, r0);
			glVertex3d(r0, -r0, r0);
			glVertex3d(-r0, -r0, r0);

			glNormal3d(0, 0, -1);
			glVertex3d(r0, r0, -r0);
			glVertex3d(-r0, r0, -r0);
			glVertex3d(-r0, -r0, -r0);
			glVertex3d(r0, -r0, -r0);
		}
		glEnd();
		break;
	case GLYPH_LINE:
		glBegin(GL_LINES);
		{
			glVertex3d(0, 0, 0);
			glVertex3d(0, 0, L);
		}
		glEnd();
	}

	glPopMatrix();
}

void CGLVectorPlot::SetVectorField(int ntype) 
{ 
	m_nvec = ntype; 
	Update(GetModel()->CurrentTimeIndex(), 0.0, true);
}

void CGLVectorPlot::Update()
{
	Update(m_lastTime, m_lastDt, false);
}

void CGLVectorPlot::Activate(bool b)
{
	CGLPlot::Activate(b);
	if ((m_ncol == 0) || !IsActive()) m_pbar->hide();
	else
	{
		m_pbar->SetRange(m_crng.x, m_crng.y);
		m_pbar->show();
	}
}

void CGLVectorPlot::Update(int ntime, float dt, bool breset)
{
	if (breset) { m_map.Clear(); m_rng.clear(); m_val.clear(); }

	m_lastTime = ntime;
	m_lastDt = dt;

	CGLModel* mdl = GetModel();
	FEPostMesh* pm = mdl->GetActiveMesh();
	FEModel* pfem = mdl->GetFEModel();

	if (m_map.States() == 0)
	{
		// nr of states
		int NS = pfem->GetStates();

		// pick the max of nodes and elements
		int NN = pm->Nodes();
		int NE = pm->Elements();
		int NM = (NN > NE ? NN : NE);

		// allocate buffers
		m_map.Create(NS, NM, vec3f(0,0,0), -1);
		m_rng.resize(NS);
		m_val.resize(NM);
	}

	 // check the tag
	int ntag = m_map.GetTag(ntime);

	// see if we need to update
	if (ntag != m_nvec)
	{
		m_map.SetTag(ntime, m_nvec);

		// get the state we are interested in
		vector<vec3f>& val = m_map.State(ntime);
		
		vec2f& rng = m_rng[ntime];
		rng.x = rng.y = 0;

		float L;

		if (IS_ELEM_FIELD(m_nvec))
		{
			for (int i = 0; i < pm->Elements(); ++i)
			{
				val[i] = pfem->EvaluateElemVector(i, ntime, m_nvec);
				L = val[i].Length();
				if (L > rng.y) rng.y = L;
			}
		}
		else
		{
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				val[i] = pfem->EvaluateNodeVector(i, ntime, m_nvec);
				L = val[i].Length();
				if (L > rng.y) rng.y = L;
			}
		}

		if (rng.y == rng.x) ++rng.y;
	}

	// update static range
	if (breset)
	{
		m_staticRange = m_rng[ntime];
	}
	else
	{
		vec2f& rng = m_rng[ntime];
		if (rng.x < m_staticRange.x) m_staticRange.x = rng.x;
		if (rng.y > m_staticRange.y) m_staticRange.y = rng.y;
	}

	// choose the range for rendering
	switch (m_rngType)
	{
	case 0: // dynamic
		m_crng = m_rng[ntime];
		break;
	case 1: // static
		m_crng = m_staticRange;
		break;
	case 2: // user
		m_crng.x = m_usr[0];
		m_crng.y = m_usr[1];
		break;
	}
	if (m_crng.x == m_crng.y) m_crng.y++;

	// update the color bar's range
	m_pbar->SetRange(m_crng.x, m_crng.y);

	// copy nodal values
	m_val = m_map.State(ntime);
}
