#pragma once

#include <MathLib/math3d.h>

class CGLView;

class GLCanvas
{
public:
	GLCanvas(CGLView* pglview);
	~GLCanvas(void);

	CGLView* GetGLView() { return m_pGLView; }
	void SetDefaultMaterial();

public:
	int			m_x, m_y;	// screen coordinates of event

protected:
	CGLView*	m_pGLView;	// pointer to the GL view
};
