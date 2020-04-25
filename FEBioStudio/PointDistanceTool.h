#pragma once
#include "Tool.h"
#include <MathLib/math3d.h>

class CPointDistanceTool : public CBasicTool
{
public:
	CPointDistanceTool(CMainWindow* wnd);

	void Activate() override;

	void Update() override;

private:
	QVariant GetPropertyValue(int i) override;
	void SetPropertyValue(int i, const QVariant& v) override;

private:
	int			m_node1, m_node2;	// mesh nodes
	vec3d		m_d;				// separation vector

	friend class Props;
};
