#include "stdafx.h"
#include "4PointAngleTool.h"
#include "PostDoc.h"
#include <PostGL/GLModel.h>
using namespace Post;

//-----------------------------------------------------------------------------
class C4PointAngleDecoration : public GDecoration
{
public:
	C4PointAngleDecoration()
	{
		point[0] = new GPointDecoration(vec3f(0,0,0));
		point[1] = new GPointDecoration(vec3f(0,0,0));
		point[2] = new GPointDecoration(vec3f(0,0,0));
		point[3] = new GPointDecoration(vec3f(0,0,0));
		line[0] = new GLineDecoration(point[0], point[1]);
		line[1] = new GLineDecoration(point[2], point[3]);
		setVisible(false);
	}

	~C4PointAngleDecoration()
	{
		delete line[1]; 
		delete line[0];
		delete point[3];
		delete point[2];
		delete point[1];
		delete point[0];
	}

	void setPosition(const vec3f& a, const vec3f& b, const vec3f& c, const vec3f& d)
	{
		point[0]->setPosition(a);
		point[1]->setPosition(b);
		point[2]->setPosition(c);
		point[3]->setPosition(d);
	}

	void render()
	{
		point[0]->render();
		point[1]->render();
		point[2]->render();
		point[3]->render();
		line[0]->render();
		line[1]->render();
	}

private:
	GPointDecoration*	point[4];
	GLineDecoration*	line[2];
};

//-----------------------------------------------------------------------------
QVariant C4PointAngleTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_node[0]; break;
	case 1: return m_node[1]; break;
	case 2: return m_node[2]; break;
	case 3: return m_node[3]; break;
	case 4: return m_angle; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void C4PointAngleTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i==0) m_node[0] = v.toInt();
	if (i==1) m_node[1] = v.toInt();
	if (i==2) m_node[2] = v.toInt();
	if (i==3) m_node[3] = v.toInt();
	UpdateAngle();
}

//-----------------------------------------------------------------------------
C4PointAngleTool::C4PointAngleTool(CMainWindow* wnd) : CBasicTool(wnd, "4Point Angle")
{
	addProperty("node 1", CProperty::Int);
	addProperty("node 2", CProperty::Int);
	addProperty("node 3", CProperty::Int);
	addProperty("node 4", CProperty::Int);
	addProperty("angle", CProperty::Float)->setFlags(CProperty::Visible);

	m_node[0] = 0;
	m_node[1] = 0;
	m_node[2] = 0;
	m_node[3] = 0;
	m_angle = 0.0;
}

//-----------------------------------------------------------------------------
void C4PointAngleTool::UpdateAngle()
{
	m_angle = 0.0;
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		Post::FEPostModel& fem = *doc->GetFEModel();
		Post::CGLModel* mdl = doc->GetGLModel();
		Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
		int ntime = mdl->CurrentTimeIndex();
		int NN = mesh.Nodes();
		if ((m_node[0] >   0)&&(m_node[1] >   0)&&(m_node[2] >   0)&&(m_node[3] >  0)&&
			(m_node[0] <= NN)&&(m_node[1] <= NN)&&(m_node[2] <= NN)&&(m_node[3] <= NN))
		{
			vec3f a = fem.NodePosition(m_node[0]-1, ntime);
			vec3f b = fem.NodePosition(m_node[1]-1, ntime);
			vec3f c = fem.NodePosition(m_node[2]-1, ntime);
			vec3f d = fem.NodePosition(m_node[3]-1, ntime);
			
			vec3f e1 = b - a; e1.Normalize();
			vec3f e2 = d - c; e2.Normalize();

			m_angle = 180.0*acos(e1*e2)/PI;
		}
	}
	updateUi();
}

//-----------------------------------------------------------------------------
void C4PointAngleTool::Update()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		Post::FEPostModel& fem = *doc->GetFEModel();
		Post::CGLModel* mdl = doc->GetGLModel();
		Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
		const vector<FENode*> selectedNodes = doc->GetGLModel()->GetNodeSelection();
		int N = (int)selectedNodes.size();
		int nsel = 0;
		for (int i = 0; i<N; ++i)
		{
			int nid = selectedNodes[i]->GetID();
			if      (m_node[0] == 0) m_node[0] = nid;
			else if (m_node[1] == 0) m_node[1] = nid;
			else if (m_node[2] == 0) m_node[2] = nid;
			else if (m_node[3] == 0) m_node[3] = nid;
			else
			{
				m_node[0] = m_node[1];
				m_node[1] = m_node[2];
				m_node[2] = m_node[3];
				m_node[3] = nid;
			}
		}
		UpdateAngle();
	}
}
