#include "stdafx.h"
#include "3PointAngleTool.h"
#include "Document.h"
#include "PostDoc.h"
#include <PostGL/GLModel.h>
using namespace Post;

//-----------------------------------------------------------------------------
class C3PointAngleDecoration : public GDecoration
{
public:
	C3PointAngleDecoration()
	{
		point[0] = new GPointDecoration(vec3f(0,0,0));
		point[1] = new GPointDecoration(vec3f(0,0,0));
		point[2] = new GPointDecoration(vec3f(0,0,0));
		line[0] = new GLineDecoration(point[0], point[1]);
		line[1] = new GLineDecoration(point[1], point[2]);
		setVisible(false);
	}

	~C3PointAngleDecoration()
	{
		delete line[1]; 
		delete line[0];
		delete point[2];
		delete point[1];
		delete point[0];
	}

	void setPosition(const vec3f& a, const vec3f& b, const vec3f& c)
	{
		point[0]->setPosition(a);
		point[1]->setPosition(b);
		point[2]->setPosition(c);
	}

	void render()
	{
		point[0]->render();
		point[1]->render();
		point[2]->render();
		line[0]->render();
		line[1]->render();
	}

private:
	GPointDecoration*	point[3];
	GLineDecoration*	line[2];
};

//-----------------------------------------------------------------------------
C3PointAngleTool::C3PointAngleTool() : CBasicTool("3Point Angle")
{
	addProperty("node 1", CProperty::Int);
	addProperty("node 2", CProperty::Int);
	addProperty("node 3", CProperty::Int);
	addProperty("angle", CProperty::Float)->setFlags(CProperty::Visible);

	m_deco = 0;

	m_node[0] = 0;
	m_node[1] = 0;
	m_node[2] = 0;
	m_angle = 0.0;
}

//-----------------------------------------------------------------------------
QVariant C3PointAngleTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_node[0]; break;
	case 1: return m_node[1]; break;
	case 2: return m_node[2]; break;
	case 3: return m_angle; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 0) m_node[0] = v.toInt();
	if (i == 1) m_node[1] = v.toInt();
	if (i == 2) m_node[2] = v.toInt();
	UpdateAngle();
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::activate(CMainWindow* wnd)
{
	CBasicTool::activate(wnd);
	update(true);
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::deactivate()
{
	CBasicTool::deactivate();
	if (m_deco)
	{
		CPostDoc* doc = GetPostDoc();
		if (doc) doc->GetGLModel()->RemoveDecoration(m_deco);
		delete m_deco;
		m_deco = 0;
	}
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::update(bool breset)
{
	if (breset)
	{
		CPostDoc* doc = GetPostDoc();
		if (doc && doc->IsValid())
		{
			Post::FEModel& fem = *doc->GetFEModel();
			Post::CGLModel* mdl = doc->GetGLModel();
			Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
			const vector<FENode*> selectedNodes = doc->GetGLModel()->GetNodeSelection();
			int N = (int) selectedNodes.size();
			int nsel = 0;
			for (int i = 0; i<N; ++i)
			{
				int nid = selectedNodes[i]->GetID();
				if      (m_node[0] == 0) m_node[0] = nid;
				else if (m_node[1] == 0) m_node[1] = nid;
				else if (m_node[2] == 0) m_node[2] = nid;
				else
				{
					m_node[0] = m_node[1];
					m_node[1] = m_node[2];
					m_node[2] = nid;
				}
			}

			if (m_deco)
			{
				doc->GetGLModel()->RemoveDecoration(m_deco);
				delete m_deco;
				m_deco = 0;
			}
			m_deco = new C3PointAngleDecoration;
			doc->GetGLModel()->AddDecoration(m_deco);
			UpdateAngle();
		}
	}
	else UpdateAngle();
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::UpdateAngle()
{
	m_angle = 0.0;
	if (m_deco) m_deco->setVisible(false);
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		Post::FEModel& fem = *doc->GetFEModel();
		Post::CGLModel* mdl = doc->GetGLModel();
		Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
		int ntime = mdl->CurrentTimeIndex();
		int NN = mesh.Nodes();
		if ((m_node[0] >   0)&&(m_node[1] >   0)&&(m_node[2] >   0)&&
			(m_node[0] <= NN)&&(m_node[1] <= NN)&&(m_node[2] <= NN))
		{
			vec3f a = fem.NodePosition(m_node[0]-1, ntime);
			vec3f b = fem.NodePosition(m_node[1]-1, ntime);
			vec3f c = fem.NodePosition(m_node[2]-1, ntime);
			
			vec3f e1 = a - b; e1.Normalize();
			vec3f e2 = c - b; e2.Normalize();

			m_angle = 180.0*acos(e1*e2)/PI;

			if (m_deco) 
			{
				m_deco->setPosition(a, b, c);
				m_deco->setVisible(true);
			}
		}
	}
	updateUi();
}
