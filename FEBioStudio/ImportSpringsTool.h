#pragma once
#include "Tool.h"

class GModel;
class GMeshObject;

class CImportSpringsTool : public CBasicTool
{
	struct SPRING
	{
		vec3d	r0;
		vec3d	r1;
	};

public:
	CImportSpringsTool(CMainWindow* wnd);

	bool OnApply();

private:
	bool ReadFile();
	bool AddSprings(GModel* fem, GMeshObject* po);
	void Intersect(GMeshObject* po, SPRING& s);

private:
	QString	m_fileName;
	double	m_tol;
	bool	m_bintersect;

	std::vector<SPRING>	m_springs;
};
