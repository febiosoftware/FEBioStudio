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

#include "stdafx.h"
#include "GView.h"
#include <stdio.h>

CGView::CGView()
{ 
	SetName("View"); 
	m_nkey = -1;

	m_bortho = false;
	m_fnear = 1.f;
	m_ffar = 50000.f;
	m_fov = 45.f;
}

CGView::~CGView()
{ 
	DeleteAllKeys();
}

void CGView::Reset()
{
	m_cam.SetTargetDistance(5.f);
	m_cam.GetOrientation() = quatd(-1, vec3d(1,0,0));
	m_cam.Update(true);
	DeleteAllKeys();
}

void CGView::DeleteAllKeys()
{
	std::vector<GLCameraTransform*>::iterator it;
	for (it=m_key.begin(); it != m_key.end(); ++it) delete (*it);
	m_key.clear();
}

GLCameraTransform* CGView::AddCameraKey(GLCameraTransform& key)
{
	m_key.push_back(new GLCameraTransform(key));
	m_nkey = (int)m_key.size()-1;
	return m_key[m_nkey];
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
	std::vector<GLCameraTransform*>::iterator it;
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
