#include "stdafx.h"
#include "GLVectorPlot.h"
#include "PostLib/ColorMap.h"
#include "PostLib/constants.h"
#include "PostLib/PropertyList.h"
#include <PostGL/GLModel.h>
using namespace Post;

class CVectorPlotProps : public CPropertyList
{
public:
	CVectorPlotProps(CGLVectorPlot* v) : m_vec(v)
	{
		QStringList cols;

		for (int i = 0; i<ColorMapManager::ColorMaps(); ++i)
		{
			string name = ColorMapManager::GetColorMapName(i);
			cols << name.c_str();
		}

		addProperty("Data field"    , CProperty::DataVec3);
		addProperty("Color map"     , CProperty::Enum)->setEnumValues(cols);
		addProperty("Allow clipping", CProperty::Bool);
		addProperty("Show hidden"   , CProperty::Bool );
		addProperty("Density"       , CProperty::Float)->setFloatRange(0.0, 1.0).setFloatStep(0.0001);
		addProperty("Glyph"         , CProperty::Enum )->setEnumValues(QStringList() << "Arrow" << "Cone" << "Cylinder" << "Sphere" << "Box" << "Line");
		addProperty("Glyph Color"   , CProperty::Enum )->setEnumValues(QStringList() << "Solid" << "Length" << "Orientation");
		addProperty("Solid Color"   , CProperty::Color);
		addProperty("Normalize"     , CProperty::Bool );
		addProperty("Auto-scale"    , CProperty::Bool );
		addProperty("Scale"         , CProperty::Float);
	}

	QVariant GetPropertyValue(int i)
	{
		switch (i)
		{
		case 0: return m_vec->GetVectorType(); break;
		case 1: return m_vec->GetColorMap()->GetColorMap();
		case 2: return m_vec->AllowClipping(); break;
		case 3: return m_vec->ShowHidden(); break;
		case 4: return m_vec->GetDensity(); break;
		case 5: return m_vec->GetGlyphType(); break;
		case 6: return m_vec->GetColorType(); break;
		case 7: return toQColor(m_vec->GetGlyphColor()); break;
		case 8: return m_vec->NormalizeVectors(); break;
		case 9: return m_vec->GetAutoScale(); break;
		case 10: return m_vec->GetScaleFactor(); break;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_vec->SetVectorType(v.toInt()); break;
		case 1: m_vec->GetColorMap()->SetColorMap(v.toInt()); break;
		case 2: m_vec->AllowClipping(v.toBool()); break;
		case 3: m_vec->ShowHidden(v.toBool()); break;
		case 4: m_vec->SetDensity(v.toFloat()); break;
		case 5: m_vec->SetGlyphType(v.toInt()); break;
		case 6: m_vec->SetColorType(v.toInt()); break;
		case 7: m_vec->SetGlyphColor(toGLColor(v.value<QColor>())); break;
		case 8: m_vec->NormalizeVectors(v.toBool()); break;
		case 9: m_vec->SetAutoScale(v.toBool()); break;
		case 10: m_vec->SetScaleFactor(v.toFloat()); break;
		}
	}

private:
	CGLVectorPlot*	m_vec;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGLVectorPlot::CGLVectorPlot(CGLModel* po) : CGLPlot(po)
{
	static int n = 1;
	char szname[128] = {0};
	sprintf(szname, "VectorPlot.%02d", n++);
	SetName(szname);

	m_scale = 1;
	m_dens = 1;

	m_ntime = -1;
	m_nvec = -1;

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
}

CGLVectorPlot::~CGLVectorPlot()
{

}

CPropertyList* CGLVectorPlot::propertyList()
{
	return new CVectorPlotProps(this);
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
	glPushAttrib(GL_LIGHTING_BIT);

	// create the cylinder object
	glEnable(GL_LIGHTING);
	GLUquadricObj* pglyph = gluNewQuadric();
	gluQuadricNormals(pglyph, GLU_SMOOTH);

	CGLModel* mdl = GetModel();
	FEModel* ps = mdl->GetFEModel();

	srand(m_seed);

	FEModel* pfem = mdl->GetFEModel();
	FEMeshBase* pm = mdl->GetActiveMesh();

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
		pm->SetElementTags(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement& e = pm->Element(i);
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
				FEElement& elem = pm->Element(i);
				if (elem.IsVisible() == false) elem.m_ntag = 0;
			}
		}

		// render the vectors at the elements' centers
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement& elem = pm->Element(i);
			if ((frand() <= m_dens) && elem.m_ntag)
			{
				vec3f r = pm->ElementCenter(elem);
				vec3f v = m_val[i];
				RenderVector(r, v, pglyph);
			}
		}
	}
	else
	{
		pm->SetNodeTags(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement& e = pm->Element(i);
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
				vec3f r = node.m_rt;

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

void CGLVectorPlot::SetVectorType(int ntype) 
{ 
	m_nvec = ntype; 
	Update(GetModel()->currentTimeIndex(), 0.0, false);
}

void CGLVectorPlot::Update(int ntime, float dt, bool breset)
{
	if (breset) { m_map.Clear(); m_rng.clear(); m_val.clear(); }

	CGLModel* mdl = GetModel();
	FEMeshBase* pm = mdl->GetActiveMesh();
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

	// copy nodal values
	m_val = m_map.State(ntime);
	m_crng = m_rng[ntime];
	if (m_crng.x == m_crng.y) m_crng.y++;
}
