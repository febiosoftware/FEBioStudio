#pragma once
#include "Tool.h"
#include <MathLib/math3d.h>

class CPointDistanceDecoration;

class CPointDistanceTool : public CBasicTool
{
public:
	CPointDistanceTool();

	void activate(CMainWindow* wnd) override;

	void deactivate() override;

	void updateLength();

	void updateUi();

	void update(bool reset);

private:
	QVariant GetPropertyValue(int i) override;
	void SetPropertyValue(int i, const QVariant& v) override;

private:
	bool		m_bvalid;			// true of node1 and node2 defined
	int			m_node1, m_node2;	// mesh nodes
	vec3f		m_d0;				// initial separation vector
	vec3f		m_d;				// separation vector
	CPointDistanceDecoration*	m_deco;

	friend class Props;
};
