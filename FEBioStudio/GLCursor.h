#pragma once
#include <MathLib/math3d.h>

class CGLView;

class GLCursor
{
public:
	static void AttachToView(CGLView* view);

	static vec3d Position();

private:
	static CGLView*	m_view;

private:
	GLCursor();
};
