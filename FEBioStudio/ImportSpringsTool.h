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
	CImportSpringsTool();

	bool OnApply();

private:
	bool ReadFile();
	bool AddSprings(GModel* fem, GMeshObject* po);

private:
	QString	m_fileName;
	double	m_tol;

	std::vector<SPRING>	m_springs;
};
