#include "stdafx.h"
#include "GLCursor.h"
#include "GLView.h"

CGLView* GLCursor::m_view = 0;

GLCursor::GLCursor() {}

void GLCursor::AttachToView(CGLView* view)
{
	m_view = view;
}

vec3d GLCursor::Position()
{
	if (m_view)
	{
		return m_view->GetPickPosition();
	}
	else return vec3d(0,0,0);
}
