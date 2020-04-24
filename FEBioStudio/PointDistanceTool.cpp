#include "stdafx.h"
#include "PointDistanceTool.h"
#include <GLLib/GDecoration.h>
#include "Document.h"
#include <PostGL/GLModel.h>
#include "PostDoc.h"
using namespace Post;

class CPointDistanceDecoration : public GDecoration
{
public:
	CPointDistanceDecoration()
	{
		point[0] = new GPointDecoration(vec3f(0,0,0));
		point[1] = new GPointDecoration(vec3f(1,1,1));
		line = new GLineDecoration(point[0], point[1]);
		setVisible(false);
	}

	void setPosition(const vec3f& a, const vec3f& b)
	{
		point[0]->setPosition(a);
		point[1]->setPosition(b);
	}

	~CPointDistanceDecoration()
	{
		delete line;
		delete point[0];
		delete point[1];
	}

	void render()
	{
		point[0]->render();
		point[1]->render();
		line->render();
	}

private:
	GPointDecoration*	point[2];
	GLineDecoration*	line;
};

QVariant CPointDistanceTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_node1; break;
	case 1: return m_node2; break;
	case 2: return fabs(m_d.x); break;
	case 3: return fabs(m_d.y); break;
	case 4: return fabs(m_d.z); break;
	case 5: return m_d.Length(); break;
	case 6: 
		{
			double s = 0.0;
			if (m_bvalid)
			{
				double L = m_d.Length();
				double L0 = m_d0.Length();
				if (L0 != 0.0) s = L / L0;
			}
			return s;
		}
		break;
	}
	return QVariant();
}

void CPointDistanceTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i==0) m_node1 = v.toInt();
	if (i==1) m_node2 = v.toInt();
	updateLength();
}

CPointDistanceTool::CPointDistanceTool() : CBasicTool("Pt.Distance")
{ 
	m_node1 = 0; 
	m_node2 = 0; 
	m_d = vec3f(0,0,0); 
	m_deco = 0;
	m_bvalid = false;

	addProperty("node 1", CProperty::Int);
	addProperty("node 2", CProperty::Int);
	addProperty("Dx", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Dy", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Dz", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Length", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Stretch", CProperty::Float)->setFlags(CProperty::Visible);

}

void CPointDistanceTool::activate(CMainWindow* wnd)
{
	CBasicTool::activate(wnd);
	update(true);
}

void CPointDistanceTool::deactivate()
{
	if (m_deco)
	{
		CPostDoc* doc = GetPostDoc();
		if (doc) doc->GetGLModel()->RemoveDecoration(m_deco);
		delete m_deco;
		m_deco = 0;
	}
}

void CPointDistanceTool::updateUi()
{
	CBasicTool::updateUi();
}

void CPointDistanceTool::updateLength()
{
	m_bvalid = false;
	m_d = vec3f(0.f,0.f,0.f);
	if (m_deco) m_deco->setVisible(false);
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		Post::FEPostModel& fem = *doc->GetFEModel();
		Post::CGLModel* mdl = doc->GetGLModel();
		Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
		int ntime = mdl->CurrentTimeIndex();
		int NN = mesh.Nodes();
		if ((m_node1 > 0)&&(m_node2 > 0)&&(m_node1 <= NN)&&(m_node2 <= NN))
		{
			vec3f a0 = fem.NodePosition(m_node1 - 1, 0);
			vec3f b0 = fem.NodePosition(m_node2 - 1, 0);
			m_d0 = b0 - a0;
			vec3f at = fem.NodePosition(m_node1 - 1, ntime);
			vec3f bt = fem.NodePosition(m_node2 - 1, ntime);
			m_d = bt - at;
			m_bvalid = true;
			if (m_deco) 
			{
				m_deco->setPosition(at, bt);
				m_deco->setVisible(true);
			}
		}
	}
}

void CPointDistanceTool::update(bool reset)
{
	if (reset)
	{
		CPostDoc* doc = GetPostDoc();
		if (doc && doc->IsValid())
		{
			Post::FEPostModel& fem = *doc->GetFEModel();
			Post::CGLModel* mdl = doc->GetGLModel();
			Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
			const vector<FENode*> selectedNodes = doc->GetGLModel()->GetNodeSelection();
			int N = (int) selectedNodes.size();
			for (int i = 0; i<N; ++i)
			{
				int nid = selectedNodes[i]->GetID();
				if (m_node1 == 0) m_node1 = nid;
				else if (m_node2 == 0) m_node2 = nid;
				else
				{
					m_node1 = m_node2;
					m_node2 = nid;
				}
			}

			if (m_deco)
			{
				doc->GetGLModel()->RemoveDecoration(m_deco);
				delete m_deco;
				m_deco = 0;
			}
			m_deco = new CPointDistanceDecoration;
			doc->GetGLModel()->AddDecoration(m_deco);
			updateLength();
		}
	}
	else updateLength();

	updateUi();
}
