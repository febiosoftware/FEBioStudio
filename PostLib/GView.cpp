#include "stdafx.h"
#include "GView.h"
#include <stdio.h>
using namespace Post;
CGView::CGView()
{ 
	SetName("View"); 
	m_nkey = -1;
}

CGView::~CGView()
{ 
	DeleteAllKeys();
}

void CGView::Reset()
{
	m_cam.SetTargetDistance(5.f);
	m_cam.GetOrientation() = quat4f(-1, vec3f(1,0,0));
	m_cam.UpdatePosition(true);
	DeleteAllKeys();
}

void CGView::DeleteAllKeys()
{
	vector<GLCameraTransform*>::iterator it;
	for (it=m_key.begin(); it != m_key.end(); ++it) delete (*it);
	m_key.clear();
}

void CGView::AddCameraKey(GLCameraTransform& key)
{
	m_key.push_back(new GLCameraTransform(key));
	m_nkey = (int)m_key.size()-1;
}

void CGView::SetCurrentKey(GLCameraTransform* pkey)
{
	if (m_key.empty()) return;
	int N = CameraKeys();
	for (int i=0; i<N; ++i)
	{
		if (m_key[i] == pkey)
		{
			SetCurrentKey(i);
			break;
		}
	}
}

void CGView::SetCurrentKey(int i)
{
	if (m_key.empty()) return;
	int N = CameraKeys();
	if ((i<0)||(i>=N)) return;
	m_nkey = i;
	m_cam.SetTransform(*m_key[m_nkey]);
}

void CGView::NextKey()
{
	if (m_nkey != -1)
	{
		m_nkey++;
		if (m_nkey >= (int)m_key.size()) m_nkey = 0;
	}
	m_cam.SetTransform(*m_key[m_nkey]);
}

void CGView::PrevKey()
{
	if (m_nkey != -1)
	{
		m_nkey--;
		if (m_nkey < 0) m_nkey = (int)m_key.size() - 1;
	}
	m_cam.SetTransform(*m_key[m_nkey]);
}

void CGView::DeleteKey(GLCameraTransform* pt)
{
	if (m_key.empty()) return;
	vector<GLCameraTransform*>::iterator it;
	for (it=m_key.begin(); it != m_key.end(); ++it)
	{
		if (*it == pt)
		{
			delete (*it);
			m_key.erase(it);
			break;
		}
	}
}
