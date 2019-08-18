#include "stdafx.h"
#include "GLContext.h"
using namespace Post;

CGLContext::CGLContext()
{
	m_cam = nullptr;

	m_showMesh = false;
	m_showOutline = false;
	m_bext = false;
	m_springThick = 1.f;
}

CGLContext::~CGLContext(void)
{
}
