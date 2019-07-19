#include "stdafx.h"
#include "GLCanvas.h"
#include "GLView.h"
#include "GLCamera.h"

GLCanvas::GLCanvas(CGLView* pglview)
{
	m_pGLView = pglview;
	assert(pglview);
}

GLCanvas::~GLCanvas(void)
{

}

void GLCanvas::SetDefaultMaterial()
{
//	m_pGLView->SetDefaultMaterial();
}
