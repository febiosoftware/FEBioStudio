#include "stdafx.h"
#include "GLContext.h"

CGLContext::CGLContext()
{
	m_cam = nullptr;

	m_showMesh = false;
	m_showOutline = false;
	m_bext = false;
	m_springThick = 1.f;

	m_btrack = false;
}

CGLContext::~CGLContext(void)
{
}
