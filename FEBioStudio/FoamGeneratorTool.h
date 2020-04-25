#pragma once
#include "PropertyList.h"
#include "Tool.h"
#include <MeshTools/FoamMesh.h>

class GMeshObject;

class CFoamGeneratorTool : public CBasicTool
{
public:
	CFoamGeneratorTool(CMainWindow* wnd);

	bool OnApply();

private:
	FoamGen	m_foam;
	GMeshObject*	m_pfo;
};
