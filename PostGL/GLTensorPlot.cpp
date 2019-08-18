#include "stdafx.h"
#include "GLTensorPlot.h"
#include "PostLib/PropertyList.h"
#include "PostLib/constants.h"
#include "GLModel.h"
#include <stdlib.h>
using namespace Post;

class CTensorPlotProps : public CPropertyList
{
public:
	CTensorPlotProps(GLTensorPlot* v) : m_tensor(v)
	{
		QStringList cols;

		for (int i = 0; i<ColorMapManager::ColorMaps(); ++i)
		{
			string name = ColorMapManager::GetColorMapName(i);
			cols << name.c_str();
		}

		addProperty("Data field"    , CProperty::DataMat3);
		addProperty("Calculate"		, CProperty::Enum)->setEnumValues(QStringList() << "Eigenvectors" << "Columns" << "Rows");
		addProperty("Color map"     , CProperty::Enum)->setEnumValues(cols);
		addProperty("Allow clipping", CProperty::Bool);
		addProperty("Show hidden"   , CProperty::Bool);
		addProperty("Scale"         , CProperty::Float);
		addProperty("Density"       , CProperty::Float)->setFloatRange(0.0, 1.0).setFloatStep(0.0001);
		addProperty("Glyph"         , CProperty::Enum)->setEnumValues(QStringList() << "Arrow" << "Line"<< "Sphere" << "Box");
		addProperty("Glyph Color"   , CProperty::Enum)->setEnumValues(QStringList() << "Solid" << "Norm");
		addProperty("Solid Color"   , CProperty::Color);
		addProperty("Auto-scale"    , CProperty::Bool);
		addProperty("Normalize"     , CProperty::Bool);
	}

	QVariant GetPropertyValue(int i)
	{
		switch (i)
		{
		case 0: return m_tensor->GetTensorField(); break;
		case 1: return m_tensor->GetVectorMethod(); break;
		case 2: return m_tensor->GetColorMap()->GetColorMap();
		case 3: return m_tensor->AllowClipping(); break;
		case 4: return m_tensor->ShowHidden(); break;
		case 5: return m_tensor->GetScaleFactor(); break;
		case 6: return m_tensor->GetDensity(); break;
		case 7: return m_tensor->GetGlyphType(); break;
		case 8: return m_tensor->GetColorType(); break;
		case 9: return toQColor(m_tensor->GetGlyphColor()); break;
		case 10: return m_tensor->GetAutoScale(); break;
		case 11: return m_tensor->GetNormalize(); break;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_tensor->SetTensorField(v.toInt()); break;
		case 1: m_tensor->SetVectorMethod(v.toInt()); break;
		case 2: m_tensor->GetColorMap()->SetColorMap(v.toInt()); break;
		case 3: m_tensor->AllowClipping(v.toBool()); break;
		case 4: m_tensor->ShowHidden(v.toBool()); break;
		case 5: m_tensor->SetScaleFactor(v.toFloat()); break;
		case 6: m_tensor->SetDensity(v.toFloat()); break;
		case 7: m_tensor->SetGlyphType(v.toInt()); break;
		case 8: m_tensor->SetColorType(v.toInt()); break;
		case 9: m_tensor->SetGlyphColor(toGLColor(v.value<QColor>())); break;
		case 10: m_tensor->SetAutoScale(v.toBool()); break;
		case 11: m_tensor->SetNormalize(v.toBool()); break;
		}
	}

private:
	GLTensorPlot*	m_tensor;
};

GLTensorPlot::GLTensorPlot(CGLModel* po) : CGLPlot(po)
{
	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "TensorPlot.%02d", n++);
	SetName(szname);

	m_scale = 1;
	m_dens = 1;

	m_ntime = -1;
	m_ntensor = 0;

	m_nglyph = Glyph_Arrow;

	m_ncol = Glyph_Col_Solid;

	m_gcl.r = 255;
	m_gcl.g = 255;
	m_gcl.b = 255;
	m_gcl.a = 255;

	m_bautoscale = true;
	m_bshowHidden = true;
	m_bnormalize = false;

	m_seed = rand();

	m_nmethod = VEC_EIGEN;
}

GLTensorPlot::~GLTensorPlot()
{
}

CPropertyList* GLTensorPlot::propertyList()
{
	return new CTensorPlotProps(this);
}

void GLTensorPlot::SetTensorField(int nfield)
{
	m_ntensor = nfield;
	Update(GetModel()->currentTimeIndex(), 0.0, false);
}

void GLTensorPlot::SetVectorMethod(int m)
{ 
	m_nmethod = m; 
	for (int i=0; i<m_map.States(); ++i) m_map.SetTag(i, -1);
	Update(GetModel()->currentTimeIndex(), 0.0, false);
}

CColorTexture* GLTensorPlot::GetColorMap()
{
	return &m_Col;
}

void GLTensorPlot::Update(int ntime, float dt, bool breset)
{
	if (breset) { m_map.Clear(); m_val.clear(); }

	CGLModel* mdl = GetModel();
	FEMeshBase* pm = mdl->GetActiveMesh();
	FEModel* pfem = mdl->GetFEModel();

	if (m_map.States() == 0)
	{
		int NS = pfem->GetStates();

		// pick the max of nodes and elements
		int NN = pm->Nodes();
		int NE = pm->Elements();
		int NM = (NN > NE ? NN : NE);

		TENSOR dummy;
		m_map.Create(NS, NM, dummy, -1);
		m_val.resize(NM);
	}

	// check the tag
	int ntag = m_map.GetTag(ntime);

	// see if we need to update
	if (ntag != m_ntensor)
	{
		m_map.SetTag(ntime, m_ntensor);

		// get the state we are interested in
		vector<TENSOR>& val = m_map.State(ntime);

		TENSOR t;
		if (IS_ELEM_FIELD(m_ntensor))
		{
			for (int i = 0; i < pm->Elements(); ++i)
			{
				switch (m_nmethod)
				{
				case VEC_EIGEN:
				{
					mat3f m = pfem->EvaluateElemTensor(i, ntime, m_ntensor, DATA_MAT3FS);

					mat3fs s = m.sym();
					s.eigen(t.r, t.l);

					vec3f n = t.r[0] ^ t.r[1];
					if (n*t.r[2] < 0.f)
					{
						t.r[2] = -t.r[2];
						t.l[2] = -t.l[2];
					}
					t.f = s.von_mises();

					val[i] = t;
				}
				break;
				case VEC_COLUMN:
				{
					mat3f m = pfem->EvaluateElemTensor(i, ntime, m_ntensor);
					t.r[0] = m.col(0);
					t.r[1] = m.col(1);
					t.r[2] = m.col(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				case VEC_ROW:
				{
					mat3f m = pfem->EvaluateElemTensor(i, ntime, m_ntensor);
					t.r[0] = m.row(0);
					t.r[1] = m.row(1);
					t.r[2] = m.row(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				}
			}
		}
		else
		{
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				switch (m_nmethod)
				{
				case VEC_EIGEN:
				{
					mat3f m = pfem->EvaluateNodeTensor(i, ntime, m_ntensor, DATA_MAT3FS);

					mat3fs s = m.sym();
					s.eigen(t.r, t.l);

					vec3f n = t.r[0] ^ t.r[1];
					if (n*t.r[2] < 0.f)
					{
						t.r[2] = -t.r[2];
						t.l[2] = -t.l[2];
					}
					t.f = s.von_mises();

					val[i] = t;
				}
				break;
				case VEC_COLUMN:
				{
					mat3f m = pfem->EvaluateNodeTensor(i, ntime, m_ntensor);
					t.r[0] = m.col(0);
					t.r[1] = m.col(1);
					t.r[2] = m.col(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				case VEC_ROW:
				{
					mat3f m = pfem->EvaluateNodeTensor(i, ntime, m_ntensor);
					t.r[0] = m.row(0);
					t.r[1] = m.row(1);
					t.r[2] = m.row(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				}
			}
		}
	}

	// copy nodal values
	m_val = m_map.State(ntime);
}

static double frand() { return (double)rand() / (double)RAND_MAX; }

void GLTensorPlot::Render(CGLContext& rc)
{
	if (m_ntensor == 0) return;

	GLfloat ambient[] = { 0.1f,0.1f,0.1f,1.f };
	GLfloat specular[] = { 0.0f,0.0f,0.0f,1 };
	GLfloat emission[] = { 0,0,0,1 };

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

	float scale = 0.02f*m_scale*pfem->GetBoundingBox().Radius();

	if (m_nglyph == Glyph_Line) glDisable(GL_LIGHTING);
	else
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		GLfloat dif[] = { 1.f, 1.f, 1.f, 1.f };

		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		glLightfv(GL_LIGHT0, GL_AMBIENT, dif);
	}

	if (IS_ELEM_FIELD(m_ntensor))
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

		float auto_scale = 1.f;
		if (m_bautoscale)
		{
			float Lmax = 0.f;
			for (int i = 0; i < pm->Elements(); ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					float L = fabs(m_val[i].l[j]);
					if (L > Lmax) Lmax = L;
				}
			}
			if (Lmax == 0.f) Lmax = 1.f;
			auto_scale = 1.f / Lmax;
		}

		CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());
		float fmax = 1.f;
		if (m_ncol == Glyph_Col_Norm)
		{
			fmax = 0.f;
			for (int i = 0; i < pm->Elements(); ++i)
			{
				float f = m_val[i].f;
				if (f > fmax) fmax = f;
			}
			if (fmax == 0.f) fmax = 1.f;
		}

		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement& elem = pm->Element(i);
			if ((frand() <= m_dens) && elem.m_ntag)
			{
				vec3f r = pm->ElementCenter(elem);

				TENSOR& t = m_val[i];

				if (m_ncol == Glyph_Col_Norm)
				{
					float w = t.f / fmax;
					GLColor c = map.map(w);
					glColor3ub(c.r, c.g, c.b);
				}

				glTranslatef(r.x, r.y, r.z);
				RenderGlyphs(t, scale*auto_scale, pglyph);
				glTranslatef(-r.x, -r.y, -r.z);
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

		float auto_scale = 1.f;
		if (m_bautoscale)
		{
			float Lmax = 0.f;
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					float L = fabs(m_val[i].l[j]);
					if (L > Lmax) Lmax = L;
				}
			}
			if (Lmax == 0.f) Lmax = 1.f;
			auto_scale = 1.f / Lmax;
		}

		CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());
		float fmax = 1.f;
		if (m_ncol == Glyph_Col_Norm)
		{
			fmax = 0.f;
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				float f = m_val[i].f;
				if (f > fmax) fmax = f;
			}
			if (fmax == 0.f) fmax = 1.f;
		}

		if (m_nglyph == Glyph_Line) glDisable(GL_LIGHTING);

		glColor3ub(m_gcl.r, m_gcl.g, m_gcl.b);

		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			if ((frand() <= m_dens) && node.m_ntag)
			{
				vec3f r = node.m_rt;

				TENSOR& t = m_val[i];

				if (m_ncol == Glyph_Col_Norm)
				{
					float w = t.f / fmax;
					GLColor c = map.map(w);
					glColor3ub(c.r, c.g, c.b);
				}

				glTranslatef(r.x, r.y, r.z);
				RenderGlyphs(t, scale*auto_scale, pglyph);
				glTranslatef(-r.x, -r.y, -r.z);
			}
		}
	}

	gluDeleteQuadric(pglyph);

	// restore attributes
	glPopAttrib();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void GLTensorPlot::RenderGlyphs(TENSOR& t, float scale, GLUquadricObj* glyph)
{
	switch (m_nglyph)
	{
	case Glyph_Arrow : RenderArrows(t, scale, glyph); break;
	case Glyph_Line  : RenderLines (t, scale, glyph); break;
	case Glyph_Sphere: RenderSphere(t, scale, glyph); break;
	case Glyph_Box   : RenderBox   (t, scale, glyph); break;
	}
}

void GLTensorPlot::RenderArrows(GLTensorPlot::TENSOR& t, float scale, GLUquadricObj* pglyph)
{
	GLColor c[3];
	c[0] = GLColor(255, 0, 0);
	c[1] = GLColor(0, 255, 0);
	c[2] = GLColor(0, 0, 255);

	for (int i = 0; i<3; ++i)
	{
		glPushMatrix();

		float L = (m_bnormalize ? scale : scale*t.l[i]);
		float l0 = L*.9;
		float l1 = L*.2;
		float r0 = L*0.05;
		float r1 = L*0.15;

		vec3f v = t.r[i];
		quat4f q = quat4f(vec3f(0,0,1), v);
		float w = q.GetAngle();
		if (fabs(w) > 1e-6)
		{
			vec3f p = q.GetVector();
			if (p.Length() > 1e-6) glRotatef(w * 180 / PI, p.x, p.y, p.z);
		}

		glColor3ub(c[i].r, c[i].g, c[i].b);
		gluCylinder(pglyph, r0, r0, l0, 5, 1);
		glTranslatef(0.f, 0.f, (float)l0*0.9f);
		gluCylinder(pglyph, r1, 0, l1, 10, 1);

		glPopMatrix();
	}
}

void GLTensorPlot::RenderLines(GLTensorPlot::TENSOR& t, float scale, GLUquadricObj* pglyph)
{
	GLColor c[3];
	c[0] = GLColor(255, 0, 0);
	c[1] = GLColor(0, 255, 0);
	c[2] = GLColor(0, 0, 255);

	for (int i = 0; i<3; ++i)
	{
		glPushMatrix();

		float L = (m_bnormalize ? scale : scale*t.l[i]);

		vec3f v = t.r[i];
		quat4f q = quat4f(vec3f(0, 0, 1), v);
		float w = q.GetAngle();
		if (fabs(w) > 1e-6)
		{
			vec3f p = q.GetVector();
			if (p.Length() > 1e-6) glRotatef(w * 180 / PI, p.x, p.y, p.z);
		}

		glBegin(GL_LINES);
		glColor3ub(c[i].r, c[i].g, c[i].b);
		glVertex3f(0.f, 0.f, 0.f);
		glVertex3f(0.f, 0.f, L);
		glEnd();

		glPopMatrix();
	}
}

void GLTensorPlot::RenderSphere(TENSOR& t, float scale, GLUquadricObj* glyph)
{
	if (scale <= 0.f) return;

	float smax = 0.f;
	float sx = fabs(t.l[0]); if (sx > smax) smax = sx;
	float sy = fabs(t.l[1]); if (sy > smax) smax = sy;
	float sz = fabs(t.l[2]); if (sz > smax) smax = sz;
	if (smax < 1e-7f) return;

	if (sx < 0.1*smax) sx = 0.1f*smax;
	if (sy < 0.1*smax) sy = 0.1f*smax;
	if (sz < 0.1*smax) sz = 0.1f*smax;

	glPushMatrix();
	vec3f* e = t.r;
	GLfloat m[4][4] = {0};
	m[3][3] = 1.f;
	m[0][0] = e[0].x; m[0][1] = e[0].y; m[0][2] = e[0].z;
	m[1][0] = e[1].x; m[1][1] = e[1].y; m[1][2] = e[1].z;
	m[2][0] = e[2].x; m[2][1] = e[2].y; m[2][2] = e[2].z;
	glMultMatrixf(&m[0][0]);

	glScalef(scale*sx, scale*sy, scale*sz);
	gluSphere(glyph, 1.f, 16, 16);
	glPopMatrix();
}

void GLTensorPlot::RenderBox(TENSOR& t, float scale, GLUquadricObj* glyph)
{
	if (scale <= 0.f) return;

	float smax = 0.f;
	float sx = fabs(t.l[0]); if (sx > smax) smax = sx;
	float sy = fabs(t.l[1]); if (sy > smax) smax = sy;
	float sz = fabs(t.l[2]); if (sz > smax) smax = sz;
	if (smax < 1e-7f) return;

	if (sx < 0.1*smax) sx = 0.1f*smax;
	if (sy < 0.1*smax) sy = 0.1f*smax;
	if (sz < 0.1*smax) sz = 0.1f*smax;

	glPushMatrix();
	vec3f* e = t.r;
	GLfloat m[4][4] = { 0 };
	m[3][3] = 1.f;
	m[0][0] = e[0].x; m[0][1] = e[0].y; m[0][2] = e[0].z;
	m[1][0] = e[1].x; m[1][1] = e[1].y; m[1][2] = e[1].z;
	m[2][0] = e[2].x; m[2][1] = e[2].y; m[2][2] = e[2].z;
	glMultMatrixf(&m[0][0]);

	glScalef(scale*sx, scale*sy, scale*sz);
	glBegin(GL_QUADS);
	{
		float r0 = 0.5f;
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
	glPopMatrix();

}
